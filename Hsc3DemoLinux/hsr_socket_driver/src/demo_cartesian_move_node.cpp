#include <memory>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"

#include "hsr_socket_driver/hsc3_client.hpp"

namespace {

class DemoCartesianMoveNode : public rclcpp::Node
{
public:
  DemoCartesianMoveNode()
  : Node("demo_cartesian_move"), client_("")
  {
    cfg_.ip = this->declare_parameter<std::string>("ip", "10.10.57.213");
    cfg_.port = static_cast<uint16_t>(this->declare_parameter<int>("port", 23234));
    cfg_.gp_id = static_cast<int8_t>(this->declare_parameter<int>("gp_id", 0));

    cfg_.force_virtual_din = this->declare_parameter<bool>("force_virtual_din", false);
    cfg_.force_din_port = this->declare_parameter<int>("force_din_port", 0);
    cfg_.force_din_value = this->declare_parameter<bool>("force_din_value", true);
    cfg_.force_din_hold_ms = this->declare_parameter<int>("force_din_hold_ms", 2000);
    cfg_.allow_override_real_din = this->declare_parameter<bool>("allow_override_real_din", false);

    mode_ = this->declare_parameter<std::string>("mode", "relative_z");  // relative_z / goto
    steps_ = this->declare_parameter<int>("steps", 10);
    dz_mm_ = this->declare_parameter<double>("dz_mm", -1.0);
    target_xyzabc_ = this->declare_parameter<std::vector<double>>(
      "target_xyzabc", {0, 0, 0, 0, 0, 0});

    client_.set_config(cfg_);

    timer_ = this->create_wall_timer(std::chrono::milliseconds(100), [this]() { this->run_once(); });
  }

private:
  void run_once()
  {
    timer_->cancel();

    try {
      client_.connect();
      client_.set_op_mode_t1();
      client_.set_manual_continue();
      client_.set_work_frame_world();

      if (!(client_.set_gp_enable(true) || client_.try_clear_estop_and_enable())) {
        RCLCPP_ERROR(this->get_logger(), "使能失败（可能急停/安全链路未满足）");
        rclcpp::shutdown();
        return;
      }

      if (mode_ == "goto") {
        RCLCPP_INFO(this->get_logger(), "Linear move to target XYZABC (mm/deg)");
        client_.move_cartesian_linear_xyzabc(target_xyzabc_);
        if (!client_.wait_group_done(20000)) {
          RCLCPP_ERROR(this->get_logger(), "等待超时，asyncMsg='%s'", client_.get_async_msg().c_str());
        }
      } else {
        auto pose = client_.get_loc_xyzabc();
        RCLCPP_INFO(this->get_logger(), "Start XYZABC=[%.3f, %.3f, %.3f, %.3f, %.3f, %.3f]",
          pose[0], pose[1], pose[2], pose[3], pose[4], pose[5]);

        const int steps = std::max(1, steps_);
        for (int i = 0; i < steps; ++i) {
          pose[2] += dz_mm_;
          client_.move_cartesian_linear_xyzabc(pose);
          if (!client_.wait_group_done(20000)) {
            RCLCPP_ERROR(this->get_logger(), "第 %d 步等待超时，asyncMsg='%s'", i, client_.get_async_msg().c_str());
            break;
          }
        }
      }

      (void)client_.set_gp_enable(false);
      client_.disconnect();
    } catch (const std::exception & e) {
      RCLCPP_ERROR(this->get_logger(), "运行异常: %s", e.what());
    }

    rclcpp::shutdown();
  }

private:
  hsr_socket_driver::Hsc3Client::Config cfg_;
  hsr_socket_driver::Hsc3Client client_;
  std::string mode_;
  int steps_{10};
  double dz_mm_{-1.0};
  std::vector<double> target_xyzabc_;
  rclcpp::TimerBase::SharedPtr timer_;
};

}  // namespace

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<DemoCartesianMoveNode>());
  rclcpp::shutdown();
  return 0;
}

