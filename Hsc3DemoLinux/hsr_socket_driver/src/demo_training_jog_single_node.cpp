#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

#include "control_msgs/action/follow_joint_trajectory.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "trajectory_msgs/msg/joint_trajectory_point.hpp"

namespace {

class TrainingJogSingleNode : public rclcpp::Node
{
public:
  TrainingJogSingleNode()
  : Node("demo_training_jog_single")
  {
    axis_ = this->declare_parameter<int>("axis", 2);          // 1..6 (默认=2，对应原例 Axis=1(0-based))
    delta_deg_ = this->declare_parameter<double>("delta_deg", 13.0);

    action_name_ =
      this->declare_parameter<std::string>("action_name", "hsr_arm_controller/follow_joint_trajectory");
    joint_states_topic_ = this->declare_parameter<std::string>("joint_states_topic", "joint_states");
    move_time_sec_ = this->declare_parameter<double>("move_time_sec", 3.0);
    wait_action_server_sec_ = this->declare_parameter<double>("wait_action_server_sec", 5.0);
    wait_joint_states_sec_ = this->declare_parameter<double>("wait_joint_states_sec", 2.0);

    joint_state_sub_ = this->create_subscription<sensor_msgs::msg::JointState>(
      joint_states_topic_, rclcpp::SystemDefaultsQoS(),
      [this](const sensor_msgs::msg::JointState::SharedPtr msg) { this->on_joint_state(msg); });

    action_client_ = rclcpp_action::create_client<FollowJT>(this, action_name_);

    timer_ = this->create_wall_timer(std::chrono::milliseconds(100), [this]() { this->run_once(); });
  }

private:
  void run_once()
  {
    timer_->cancel();

    int axis0 = axis_;
    if (axis0 >= 1 && axis0 <= 6) {
      axis0 -= 1;  // 转成 0-based
    }
    if (axis0 < 0 || axis0 >= 6) {
      RCLCPP_ERROR(this->get_logger(), "axis 参数无效: %d（期望 1..6）", axis_);
      rclcpp::shutdown();
      return;
    }

    if (!action_client_->wait_for_action_server(std::chrono::duration<double>(wait_action_server_sec_))) {
      RCLCPP_ERROR(this->get_logger(), "等待 action server 超时: %s", action_name_.c_str());
      rclcpp::shutdown();
      return;
    }

    auto cur_opt = wait_current_joints_(std::chrono::duration<double>(wait_joint_states_sec_));
    if (!cur_opt.has_value()) {
      RCLCPP_ERROR(this->get_logger(), "等待 joint_states 超时: %s", joint_states_topic_.c_str());
      rclcpp::shutdown();
      return;
    }

    auto target = cur_opt.value();
    target[static_cast<size_t>(axis0)] += deg2rad_(delta_deg_);

    FollowJT::Goal goal;
    goal.trajectory.joint_names.assign(kArmJointNames_.begin(), kArmJointNames_.end());

    trajectory_msgs::msg::JointTrajectoryPoint pt;
    pt.positions.assign(target.begin(), target.end());
    pt.time_from_start = rclcpp::Duration::from_seconds(std::max(0.1, move_time_sec_));
    goal.trajectory.points.push_back(pt);

    auto send_goal_options = rclcpp_action::Client<FollowJT>::SendGoalOptions();
    send_goal_options.result_callback =
      [this](const rclcpp_action::ClientGoalHandle<FollowJT>::WrappedResult & result) {
        if (result.code != rclcpp_action::ResultCode::SUCCEEDED) {
          RCLCPP_ERROR(this->get_logger(), "执行失败，code=%d", static_cast<int>(result.code));
        }
        rclcpp::shutdown();
      };

    RCLCPP_INFO(this->get_logger(), "Axis %d relative move %.3f deg via action %s", axis0 + 1, delta_deg_,
      action_name_.c_str());

    (void)action_client_->async_send_goal(goal, send_goal_options);
  }

  static double deg2rad_(double deg) { return deg * M_PI / 180.0; }

  void on_joint_state(const sensor_msgs::msg::JointState::SharedPtr msg)
  {
    if (!msg) {
      return;
    }

    std::array<double, 6> joints{};
    if (!extract_arm_joints_(*msg, joints)) {
      return;
    }
    last_arm_joints_ = joints;
    last_arm_joints_time_ = this->now();
  }

  static bool extract_arm_joints_(const sensor_msgs::msg::JointState & msg, std::array<double, 6> & out)
  {
    if (msg.name.size() != msg.position.size()) {
      return false;
    }

    bool found_any = false;
    for (size_t i = 0; i < msg.name.size(); ++i) {
      for (size_t j = 0; j < kArmJointNames_.size(); ++j) {
        if (msg.name[i] == kArmJointNames_[j]) {
          out[j] = msg.position[i];
          found_any = true;
          break;
        }
      }
    }

    for (size_t j = 0; j < kArmJointNames_.size(); ++j) {
      const bool exists = std::find(msg.name.begin(), msg.name.end(), kArmJointNames_[j]) != msg.name.end();
      if (!exists) {
        return false;
      }
    }
    return found_any;
  }

  std::optional<std::array<double, 6>> wait_current_joints_(std::chrono::duration<double> timeout)
  {
    const auto start = this->now();
    while (rclcpp::ok() && (this->now() - start) < rclcpp::Duration(timeout)) {
      if (last_arm_joints_.has_value()) {
        return last_arm_joints_;
      }
      rclcpp::sleep_for(std::chrono::milliseconds(10));
    }
    return std::nullopt;
  }

private:
  int axis_{2};
  double delta_deg_{13.0};
  std::string action_name_;
  std::string joint_states_topic_;
  double move_time_sec_{3.0};
  double wait_action_server_sec_{5.0};
  double wait_joint_states_sec_{2.0};

  using FollowJT = control_msgs::action::FollowJointTrajectory;
  rclcpp_action::Client<FollowJT>::SharedPtr action_client_;
  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joint_state_sub_;

  std::optional<std::array<double, 6>> last_arm_joints_;
  rclcpp::Time last_arm_joints_time_;

  static inline const std::array<std::string, 6> kArmJointNames_ = {
    "joint_1", "joint_2", "joint_3", "joint_4", "joint_5", "joint_6"};

  rclcpp::TimerBase::SharedPtr timer_;
};

}  // namespace

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TrainingJogSingleNode>());
  rclcpp::shutdown();
  return 0;
}

