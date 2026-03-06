#include <memory>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"

#include "hsr_socket_driver/hsc3_client.hpp"

namespace {

class TrainingJogNode : public rclcpp::Node
{
public:
  TrainingJogNode()
  : Node("demo_training_jog"), client_("")
  {
    cfg_.ip = this->declare_parameter<std::string>("ip", "10.10.57.213");
    cfg_.port = static_cast<uint16_t>(this->declare_parameter<int>("port", 23234));
    cfg_.gp_id = static_cast<int8_t>(this->declare_parameter<int>("gp_id", 0));

    // 该 demo 不负责安全链路/DI 控制：避免退出时把 DI 写回 0。
    // 如需 DI0=1 才能上使能，请在 driver.launch.py 中配置 force_virtual_din + reset_din_on_exit。
    cfg_.force_virtual_din = false;
    cfg_.reset_din_on_disconnect = false;

    delta_deg_ = this->declare_parameter<double>("delta_deg", 10.0);

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
      client_.set_work_frame_joint();

      if (!(client_.set_gp_enable(true) || client_.try_clear_estop_and_enable())) {
        RCLCPP_ERROR(this->get_logger(), "使能失败（可能急停/安全链路未满足）");
        rclcpp::shutdown();
        return;
      }

      for (int axis = 0; axis < 6; ++axis) {
        auto cur = client_.get_joint_deg();
        auto target = cur;
        target[static_cast<size_t>(axis)] += delta_deg_;

        RCLCPP_INFO(this->get_logger(), "Axis %d +%.3f deg", axis + 1, delta_deg_);
        client_.move_joint_deg(target);
        if (!client_.wait_group_done(15000)) {
          RCLCPP_ERROR(this->get_logger(), "Axis %d (+) 等待超时，asyncMsg='%s'", axis + 1, client_.get_async_msg().c_str());
          break;
        }

        cur = client_.get_joint_deg();
        target = cur;
        target[static_cast<size_t>(axis)] -= delta_deg_;

        RCLCPP_INFO(this->get_logger(), "Axis %d -%.3f deg", axis + 1, delta_deg_);
        client_.move_joint_deg(target);
        if (!client_.wait_group_done(15000)) {
          RCLCPP_ERROR(this->get_logger(), "Axis %d (-) 等待超时，asyncMsg='%s'", axis + 1, client_.get_async_msg().c_str());
          break;
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
  double delta_deg_{10.0};
  rclcpp::TimerBase::SharedPtr timer_;
};

}  // namespace

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TrainingJogNode>());
  rclcpp::shutdown();
  return 0;
}

