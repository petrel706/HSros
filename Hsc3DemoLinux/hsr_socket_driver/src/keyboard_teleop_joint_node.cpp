#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include <termios.h>
#include <unistd.h>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

#include "control_msgs/action/follow_joint_trajectory.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "trajectory_msgs/msg/joint_trajectory_point.hpp"

namespace {

class ScopedTermiosRaw
{
public:
  ScopedTermiosRaw()
  {
    if (tcgetattr(STDIN_FILENO, &orig_) != 0) {
      enabled_ = false;
      return;
    }

    termios raw = orig_;
    raw.c_lflag &= static_cast<unsigned long>(~(ICANON | ECHO));
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0) {
      enabled_ = false;
      return;
    }

    enabled_ = true;
  }

  ~ScopedTermiosRaw()
  {
    if (enabled_) {
      (void)tcsetattr(STDIN_FILENO, TCSANOW, &orig_);
    }
  }

  ScopedTermiosRaw(const ScopedTermiosRaw &) = delete;
  ScopedTermiosRaw & operator=(const ScopedTermiosRaw &) = delete;

  bool ok() const { return enabled_; }

private:
  termios orig_{};
  bool enabled_{false};
};

class KeyboardTeleopJointNode : public rclcpp::Node
{
public:
  using FollowJT = control_msgs::action::FollowJointTrajectory;

  KeyboardTeleopJointNode()
  : Node("keyboard_teleop_joint")
  {
    action_name_ =
      this->declare_parameter<std::string>("action_name", "hsr_arm_controller/follow_joint_trajectory");
    joint_states_topic_ = this->declare_parameter<std::string>("joint_states_topic", "joint_states");

    step_deg_ = this->declare_parameter<double>("step_deg", 2.0);
    move_time_sec_ = this->declare_parameter<double>("move_time_sec", 0.8);
    speed_scale_ = this->declare_parameter<double>("speed_scale", 1.0);
    speed_scale_step_ = this->declare_parameter<double>("speed_scale_step", 0.1);
    speed_scale_max_ = this->declare_parameter<double>("speed_scale_max", 10.0);
    wait_action_server_sec_ = this->declare_parameter<double>("wait_action_server_sec", 5.0);

    speed_scale_ = std::max(0.1, std::min(speed_scale_max_, speed_scale_));

    joint_state_sub_ = this->create_subscription<sensor_msgs::msg::JointState>(
      joint_states_topic_, rclcpp::SystemDefaultsQoS(),
      [this](const sensor_msgs::msg::JointState::SharedPtr msg) { this->on_joint_state(msg); });

    action_client_ = rclcpp_action::create_client<FollowJT>(this, action_name_);

    print_help_();

    timer_ = this->create_wall_timer(std::chrono::milliseconds(50), [this]() { this->poll_keyboard_(); });
  }

private:
  static double deg2rad_(double deg) { return deg * M_PI / 180.0; }

  void print_help_()
  {
    RCLCPP_INFO(this->get_logger(), "Keyboard teleop started.");
    RCLCPP_INFO(this->get_logger(), "Keys:");
    RCLCPP_INFO(this->get_logger(), "  1..6 : select axis (current=%d)", selected_axis_ + 1);
    RCLCPP_INFO(this->get_logger(), "  + / -: move selected axis by step_deg (current step=%.3f deg)", step_deg_);
    RCLCPP_INFO(this->get_logger(), "  [ / ]: decrease/increase step_deg by 0.5 deg");
    RCLCPP_INFO(this->get_logger(), "  < / >: decrease/increase speed_scale by speed_scale_step (current=%.2f, max=%.2f)",
      speed_scale_, speed_scale_max_);
    RCLCPP_INFO(this->get_logger(), "  h    : print this help");
    RCLCPP_INFO(this->get_logger(), "  q    : quit");
    RCLCPP_INFO(this->get_logger(), "Action: %s ; joint_states: %s", action_name_.c_str(), joint_states_topic_.c_str());
  }

  void on_joint_state(const sensor_msgs::msg::JointState::SharedPtr msg)
  {
    if (!msg) {
      return;
    }

    if (msg->name.size() != msg->position.size()) {
      return;
    }

    std::array<double, 6> out{};
    for (size_t j = 0; j < kArmJointNames_.size(); ++j) {
      bool found = false;
      for (size_t i = 0; i < msg->name.size(); ++i) {
        if (msg->name[i] == kArmJointNames_[j]) {
          out[j] = msg->position[i];
          found = true;
          break;
        }
      }
      if (!found) {
        return;
      }
    }

    last_arm_joints_ = out;
    last_arm_joints_time_ = this->now();
  }

  void poll_keyboard_()
  {
    if (!raw_mode_.has_value()) {
      raw_mode_.emplace();
      if (!raw_mode_->ok()) {
        RCLCPP_ERROR(this->get_logger(), "Failed to set terminal to raw mode");
        rclcpp::shutdown();
        return;
      }
    }

    char c = 0;
    const ssize_t n = ::read(STDIN_FILENO, &c, 1);
    if (n <= 0) {
      return;
    }

    if (c >= '1' && c <= '6') {
      selected_axis_ = static_cast<int>(c - '1');
      RCLCPP_INFO(this->get_logger(), "Selected axis %d", selected_axis_ + 1);
      return;
    }

    if (c == 'h' || c == 'H') {
      print_help_();
      return;
    }

    if (c == 'q' || c == 'Q') {
      rclcpp::shutdown();
      return;
    }

    if (c == '[') {
      step_deg_ = std::max(0.1, step_deg_ - 0.5);
      RCLCPP_INFO(this->get_logger(), "step_deg=%.3f", step_deg_);
      return;
    }

    if (c == ']') {
      step_deg_ = step_deg_ + 0.5;
      RCLCPP_INFO(this->get_logger(), "step_deg=%.3f", step_deg_);
      return;
    }

    if (c == '<' || c == ',') {
      speed_scale_ = std::max(0.1, speed_scale_ - speed_scale_step_);
      RCLCPP_INFO(this->get_logger(), "speed_scale=%.2f", speed_scale_);
      return;
    }

    if (c == '>' || c == '.') {
      speed_scale_ = std::min(speed_scale_max_, speed_scale_ + speed_scale_step_);
      RCLCPP_INFO(this->get_logger(), "speed_scale=%.2f", speed_scale_);
      return;
    }

    if (c == '+' || c == '=') {
      send_relative_move_(+step_deg_);
      return;
    }

    if (c == '-' || c == '_') {
      send_relative_move_(-step_deg_);
      return;
    }
  }

  void send_relative_move_(double delta_deg)
  {
    if (goal_in_flight_) {
      return;
    }

    if (!action_client_->wait_for_action_server(std::chrono::duration<double>(wait_action_server_sec_))) {
      RCLCPP_ERROR(this->get_logger(), "Waiting action server timeout: %s", action_name_.c_str());
      return;
    }

    if (!last_arm_joints_.has_value()) {
      RCLCPP_ERROR(this->get_logger(), "No joint_states received yet on %s", joint_states_topic_.c_str());
      return;
    }

    auto target = last_arm_joints_.value();
    target[static_cast<size_t>(selected_axis_)] += deg2rad_(delta_deg);

    FollowJT::Goal goal;
    goal.trajectory.joint_names.assign(kArmJointNames_.begin(), kArmJointNames_.end());

    trajectory_msgs::msg::JointTrajectoryPoint pt;
    pt.positions.assign(target.begin(), target.end());
    pt.time_from_start = rclcpp::Duration::from_seconds(std::max(0.1, move_time_sec_));

    // 将 speed_scale 映射到 vord(1~100%)，通过 velocities[0] 传给 driver
    const int32_t vord = static_cast<int32_t>(std::lround(std::max(0.1, speed_scale_) * 10.0));
    pt.velocities.resize(1);
    pt.velocities[0] = static_cast<double>(std::max<int32_t>(1, std::min<int32_t>(100, vord)));
    goal.trajectory.points.push_back(pt);

    auto send_goal_options = rclcpp_action::Client<FollowJT>::SendGoalOptions();
    send_goal_options.result_callback =
      [this](const rclcpp_action::ClientGoalHandle<FollowJT>::WrappedResult & result) {
        goal_in_flight_ = false;
        if (result.code != rclcpp_action::ResultCode::SUCCEEDED) {
          RCLCPP_ERROR(this->get_logger(), "Action failed, code=%d", static_cast<int>(result.code));
          return;
        }
      };

    goal_in_flight_ = true;
    RCLCPP_INFO(this->get_logger(), "Axis %d %+0.3f deg", selected_axis_ + 1, delta_deg);
    (void)action_client_->async_send_goal(goal, send_goal_options);
  }

private:
  std::string action_name_;
  std::string joint_states_topic_;

  double step_deg_{2.0};
  double move_time_sec_{0.8};
  double speed_scale_{1.0};
  double speed_scale_step_{0.1};
  double speed_scale_max_{10.0};
  double wait_action_server_sec_{5.0};

  int selected_axis_{0};

  rclcpp_action::Client<FollowJT>::SharedPtr action_client_;
  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joint_state_sub_;
  rclcpp::TimerBase::SharedPtr timer_;

  bool goal_in_flight_{false};

  std::optional<ScopedTermiosRaw> raw_mode_;

  std::optional<std::array<double, 6>> last_arm_joints_;
  rclcpp::Time last_arm_joints_time_;

  static inline const std::array<std::string, 6> kArmJointNames_ = {
    "joint_1", "joint_2", "joint_3", "joint_4", "joint_5", "joint_6"};
};

}  // namespace

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<KeyboardTeleopJointNode>());
  rclcpp::shutdown();
  return 0;
}
