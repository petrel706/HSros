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

class TrainingJogHomeNode : public rclcpp::Node
{
public:
  TrainingJogHomeNode()
  : Node("demo_training_jog_home")
  {
    home_deg_ = this->declare_parameter<std::vector<double>>(
      "home_deg", {-5.44e-05, -90.0001, 89.9999, -89.9999, 90.0001, -0.0001632});

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

    if (home_deg_.size() < 6) {
      RCLCPP_ERROR(this->get_logger(), "home_deg 参数无效：需要至少 6 个关节角（deg）");
      rclcpp::shutdown();
      return;
    }

    if (!action_client_->wait_for_action_server(std::chrono::duration<double>(wait_action_server_sec_))) {
      RCLCPP_ERROR(this->get_logger(), "等待 action server 超时: %s", action_name_.c_str());
      rclcpp::shutdown();
      return;
    }

    FollowJT::Goal goal;
    goal.trajectory.joint_names.assign(kArmJointNames_.begin(), kArmJointNames_.end());

    trajectory_msgs::msg::JointTrajectoryPoint pt;
    pt.positions.resize(6);
    for (size_t i = 0; i < 6; ++i) {
      pt.positions[i] = deg2rad_(home_deg_[i]);
    }
    pt.time_from_start = rclcpp::Duration::from_seconds(std::max(0.1, move_time_sec_));
    goal.trajectory.points.push_back(pt);

    auto send_goal_options = rclcpp_action::Client<FollowJT>::SendGoalOptions();
    send_goal_options.result_callback =
      [this](const rclcpp_action::ClientGoalHandle<FollowJT>::WrappedResult & result) {
        if (result.code != rclcpp_action::ResultCode::SUCCEEDED) {
          RCLCPP_ERROR(this->get_logger(), "HOME 执行失败，code=%d", static_cast<int>(result.code));
        }
        rclcpp::shutdown();
      };

    RCLCPP_INFO(this->get_logger(), "Send HOME via action %s", action_name_.c_str());
    (void)action_client_->async_send_goal(goal, send_goal_options);
  }

  static double deg2rad_(double deg) { return deg * M_PI / 180.0; }

private:
  std::vector<double> home_deg_;
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
  rclcpp::spin(std::make_shared<TrainingJogHomeNode>());
  rclcpp::shutdown();
  return 0;
}

