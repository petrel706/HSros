#include <array>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

#include "control_msgs/action/follow_joint_trajectory.hpp"
#include "trajectory_msgs/msg/joint_trajectory_point.hpp"

namespace {

class TrainingMoveJointNode : public rclcpp::Node
{
public:
  TrainingMoveJointNode()
  : Node("demo_training_move_joint")
  {
    target1_ = this->declare_parameter<std::vector<double>>(
      "target1_deg", {11.60971, -90.49248, 182.82715, 6.721339, 93.89419, 0.0});
    target2_ = this->declare_parameter<std::vector<double>>(
      "target2_deg", {0.0, -90.0, 180.0, 0.0, 90.0, 0.0});

    action_name_ =
      this->declare_parameter<std::string>("action_name", "hsr_arm_controller/follow_joint_trajectory");
    move_time_sec_ = this->declare_parameter<double>("move_time_sec", 5.0);
    wait_action_server_sec_ = this->declare_parameter<double>("wait_action_server_sec", 5.0);

    action_client_ = rclcpp_action::create_client<FollowJT>(this, action_name_);

    timer_ = this->create_wall_timer(std::chrono::milliseconds(100), [this]() { this->run_once(); });
  }

private:
  void run_once()
  {
    timer_->cancel();

    if (target1_.size() < 6 || target2_.size() < 6) {
      RCLCPP_ERROR(this->get_logger(), "target1/target2 参数无效：需要至少 6 个关节角（deg）");
      rclcpp::shutdown();
      return;
    }

    if (!action_client_->wait_for_action_server(std::chrono::duration<double>(wait_action_server_sec_))) {
      RCLCPP_ERROR(this->get_logger(), "等待 action server 超时: %s", action_name_.c_str());
      rclcpp::shutdown();
      return;
    }

    send_target_("target1", target1_, [this]() { this->send_target_("target2", target2_, []() { rclcpp::shutdown(); }); });
  }

  static double deg2rad_(double deg) { return deg * M_PI / 180.0; }

  void send_target_(
    const std::string & label, const std::vector<double> & target_deg, std::function<void()> next)
  {
    FollowJT::Goal goal;
    goal.trajectory.joint_names.assign(kArmJointNames_.begin(), kArmJointNames_.end());

    trajectory_msgs::msg::JointTrajectoryPoint pt;
    pt.positions.resize(6);
    for (size_t i = 0; i < 6; ++i) {
      pt.positions[i] = deg2rad_(target_deg[i]);
    }
    pt.time_from_start = rclcpp::Duration::from_seconds(std::max(0.1, move_time_sec_));
    goal.trajectory.points.push_back(pt);

    auto send_goal_options = rclcpp_action::Client<FollowJT>::SendGoalOptions();
    send_goal_options.result_callback =
      [this, label, next](const rclcpp_action::ClientGoalHandle<FollowJT>::WrappedResult & result) {
        if (result.code != rclcpp_action::ResultCode::SUCCEEDED) {
          RCLCPP_ERROR(this->get_logger(), "%s 执行失败，code=%d", label.c_str(), static_cast<int>(result.code));
          rclcpp::shutdown();
          return;
        }
        RCLCPP_INFO(this->get_logger(), "%s 执行完成", label.c_str());
        next();
      };

    RCLCPP_INFO(this->get_logger(), "Send %s via action %s", label.c_str(), action_name_.c_str());
    (void)action_client_->async_send_goal(goal, send_goal_options);
  }

private:
  std::vector<double> target1_;
  std::vector<double> target2_;
  std::string action_name_;
  double move_time_sec_{5.0};
  double wait_action_server_sec_{5.0};

  using FollowJT = control_msgs::action::FollowJointTrajectory;
  rclcpp_action::Client<FollowJT>::SharedPtr action_client_;

  static inline const std::array<std::string, 6> kArmJointNames_ = {
    "joint_1", "joint_2", "joint_3", "joint_4", "joint_5", "joint_6"};

  rclcpp::TimerBase::SharedPtr timer_;
};

}  // namespace

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TrainingMoveJointNode>());
  rclcpp::shutdown();
  return 0;
}

