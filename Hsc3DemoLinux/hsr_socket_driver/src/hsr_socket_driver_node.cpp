#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "control_msgs/action/follow_joint_trajectory.hpp"

#include "hsr_socket_driver/hsc3_client.hpp"

namespace hsr_socket_driver {
namespace {

inline double rad2deg(double rad) { return rad * 180.0 / M_PI; }

using FollowJT = control_msgs::action::FollowJointTrajectory;

class HsrSocketDriverNode : public rclcpp::Node
{
public:
  HsrSocketDriverNode()
  : Node("hsr_socket_driver"), client_("")
  {
    // 连接参数
    cfg_.ip = this->declare_parameter<std::string>("ip", "10.10.57.213");
    cfg_.port = static_cast<uint16_t>(this->declare_parameter<int>("port", 23234));
    cfg_.gp_id = static_cast<int8_t>(this->declare_parameter<int>("gp_id", 0));

    // 是否尝试强制虚拟 DI（默认关闭，避免安全风险）
    cfg_.force_virtual_din = this->declare_parameter<bool>("force_virtual_din", false);
    cfg_.force_din_port = this->declare_parameter<int>("force_din_port", 0);
    cfg_.force_din_value = this->declare_parameter<bool>("force_din_value", true);
    cfg_.force_din_hold_ms = this->declare_parameter<int>("force_din_hold_ms", 2000);
    cfg_.allow_override_real_din = this->declare_parameter<bool>("allow_override_real_din", false);
    cfg_.reset_din_on_disconnect = this->declare_parameter<bool>("reset_din_on_exit", false);

    publish_rate_hz_ = this->declare_parameter<double>("publish_rate_hz", 50.0);
    enable_on_start_ = this->declare_parameter<bool>("enable_on_start", true);
    op_mode_str_ = this->declare_parameter<std::string>("op_mode", "EXT");
    set_op_mode_ = this->declare_parameter<bool>("set_op_mode", false);
    trajectory_action_name_ =
      this->declare_parameter<std::string>("trajectory_action_name", "follow_joint_trajectory");

    joint_names_ = this->declare_parameter<std::vector<std::string>>(
      "joint_names", {"joint_1", "joint_2", "joint_3", "joint_4", "joint_5", "joint_6"});

    client_.set_config(cfg_);

    joint_pub_ = this->create_publisher<sensor_msgs::msg::JointState>("joint_states", 10);

    // 连接并设置模式
    try {
      client_.connect();

      OpMode mode = OP_NONE;
      if (set_op_mode_) {
        mode = parse_op_mode_(op_mode_str_);
        client_.set_op_mode(mode);
      } else {
        mode = client_.get_op_mode();
      }

      if (mode == OP_T1 || mode == OP_T2) {
        client_.set_manual_continue();
      } else {
        client_.set_auto_continue();
      }
      client_.set_work_frame_joint();

      if (enable_on_start_) {
        if (!(client_.set_gp_enable(true) || client_.try_clear_estop_and_enable())) {
          RCLCPP_ERROR(this->get_logger(), "上使能失败（可能是急停/安全链路未满足）");
        }
      }
      RCLCPP_INFO(this->get_logger(), "已连接到控制器 %s:%u", cfg_.ip.c_str(), cfg_.port);
    } catch (const std::exception & e) {
      RCLCPP_ERROR(this->get_logger(), "初始化失败: %s", e.what());
    }

    // joint_states 定时发布
    const auto period = std::chrono::duration<double>(1.0 / std::max(1e-3, publish_rate_hz_));
    timer_ = this->create_wall_timer(
      std::chrono::duration_cast<std::chrono::milliseconds>(period),
      [this]() { this->publish_joint_states(); });

    // FollowJointTrajectory action server（给 MoveIt2 / ros2_control 侧对接用）
    action_server_ = rclcpp_action::create_server<FollowJT>(
      this->get_node_base_interface(),
      this->get_node_clock_interface(),
      this->get_node_logging_interface(),
      this->get_node_waitables_interface(),
      trajectory_action_name_,
      [this](
        const rclcpp_action::GoalUUID &,
        std::shared_ptr<const FollowJT::Goal> goal) { return this->handle_goal(goal); },
      [this](std::shared_ptr<rclcpp_action::ServerGoalHandle<FollowJT>> goal_handle) {
        return this->handle_cancel(goal_handle);
      },
      [this](std::shared_ptr<rclcpp_action::ServerGoalHandle<FollowJT>> goal_handle) {
        std::thread{[this, goal_handle]() { this->execute(goal_handle); }}.detach();
      });
  }

  ~HsrSocketDriverNode() override
  {
    try {
      (void)client_.set_gp_enable(false);
      client_.disconnect();
    } catch (...) {
    }
  }

private:
  static OpMode parse_op_mode_(const std::string & s)
  {
    if (s == "T1") {
      return OP_T1;
    }
    if (s == "T2") {
      return OP_T2;
    }
    if (s == "AUT" || s == "AUTO") {
      return OP_AUT;
    }
    if (s == "EXT" || s == "EXTERNAL") {
      return OP_EXT;
    }
    if (s == "DRAG") {
      return OP_DRAG;
    }
    return OP_EXT;
  }

  rclcpp_action::GoalResponse handle_goal(std::shared_ptr<const FollowJT::Goal> goal)
  {
    if (!goal) {
      return rclcpp_action::GoalResponse::REJECT;
    }
    if (goal->trajectory.joint_names.size() < 6) {
      RCLCPP_ERROR(this->get_logger(), "trajectory.joint_names 数量不足（需要>=6）");
      return rclcpp_action::GoalResponse::REJECT;
    }
    if (goal->trajectory.points.empty()) {
      RCLCPP_ERROR(this->get_logger(), "trajectory.points 为空");
      return rclcpp_action::GoalResponse::REJECT;
    }
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
  }

  rclcpp_action::CancelResponse handle_cancel(
    const std::shared_ptr<rclcpp_action::ServerGoalHandle<FollowJT>>)
  {
    // 这里先接受取消请求；若你需要“真正停止”，后续可补充调用 SDK 的 stop/abort 接口
    return rclcpp_action::CancelResponse::ACCEPT;
  }

  void execute(const std::shared_ptr<rclcpp_action::ServerGoalHandle<FollowJT>> goal_handle)
  {
    const auto goal = goal_handle->get_goal();

    auto result = std::make_shared<FollowJT::Result>();

    try {
      // 逐点执行（最小可用版：每个点用 moveTo 走到位，并等待停止）
      for (size_t i = 0; i < goal->trajectory.points.size(); ++i) {
        if (goal_handle->is_canceling()) {
          result->error_code = FollowJT::Result::SUCCESSFUL;
          goal_handle->canceled(result);
          return;
        }

        const auto & pt = goal->trajectory.points[i];
        if (pt.positions.size() < 6) {
          result->error_code = FollowJT::Result::INVALID_GOAL;
          goal_handle->abort(result);
          return;
        }

        // 可选：使用 velocities[0] 作为速度倍率（vord，1~100%）
        if (!pt.velocities.empty()) {
          const double v = pt.velocities[0];
          if (std::isfinite(v)) {
            const int32_t vord = static_cast<int32_t>(std::lround(v));
            if (vord >= 1 && vord <= 100) {
              client_.set_vord(vord);
            }
          }
        }

        std::vector<double> joint_deg(6);
        for (size_t j = 0; j < 6; ++j) {
          joint_deg[j] = rad2deg(pt.positions[j]);
        }

        client_.move_joint_deg(joint_deg);
        const bool ok = client_.wait_group_done(10000);
        if (!ok) {
          const auto async = client_.get_async_msg();
          RCLCPP_ERROR(this->get_logger(), "等待运动完成超时，asyncMsg='%s'", async.c_str());
          result->error_code = FollowJT::Result::PATH_TOLERANCE_VIOLATED;
          goal_handle->abort(result);
          return;
        }

        auto fb = std::make_shared<FollowJT::Feedback>();
        fb->desired = pt;
        goal_handle->publish_feedback(fb);
      }

      result->error_code = FollowJT::Result::SUCCESSFUL;
      goal_handle->succeed(result);
    } catch (const std::exception & e) {
      RCLCPP_ERROR(this->get_logger(), "执行轨迹异常: %s", e.what());
      result->error_code = FollowJT::Result::INVALID_GOAL;
      goal_handle->abort(result);
    }
  }

  void publish_joint_states()
  {
    if (!client_.is_connected()) {
      return;
    }

    sensor_msgs::msg::JointState msg;
    msg.header.stamp = this->now();
    msg.name = joint_names_;

    try {
      const auto pos = client_.get_joint_rad();
      if (pos.size() < joint_names_.size()) {
        RCLCPP_WARN_THROTTLE(
          this->get_logger(), *this->get_clock(), 2000,
          "JointState 无效：joint_names=%zu 但 position=%zu（不足），已忽略发布",
          joint_names_.size(), pos.size());
        return;
      }

      msg.position.assign(pos.begin(), pos.begin() + static_cast<long>(joint_names_.size()));

      const auto append_joint = [&msg](const std::string & name, double position) {
        if (std::find(msg.name.begin(), msg.name.end(), name) != msg.name.end()) {
          return;
        }
        msg.name.push_back(name);
        msg.position.push_back(position);
      };

      append_joint("lf_wheel_joint", 0.0);
      append_joint("rf_wheel_joint", 0.0);

      joint_pub_->publish(msg);
    } catch (const std::exception & e) {
      RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 2000, "读关节失败: %s", e.what());
    }
  }

private:
  Hsc3Client::Config cfg_;
  double publish_rate_hz_{50.0};
  bool enable_on_start_{true};
  std::string op_mode_str_;
  bool set_op_mode_{false};
  std::string trajectory_action_name_;
  std::vector<std::string> joint_names_;

  Hsc3Client client_;

  rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr joint_pub_;
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp_action::Server<FollowJT>::SharedPtr action_server_;
};

}  // namespace
}  // namespace hsr_socket_driver

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<hsr_socket_driver::HsrSocketDriverNode>());
  rclcpp::shutdown();
  return 0;
}

