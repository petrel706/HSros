#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"

#include "CommApi.h"

namespace {

class CommApiTrainingNode : public rclcpp::Node
{
public:
  CommApiTrainingNode()
  : Node("demo_comm_api_training")
  {
    ip_ = this->declare_parameter<std::string>("ip", "10.10.57.213");
    port_ = static_cast<uint16_t>(this->declare_parameter<int>("port", 23234));
    log_dir_ = this->declare_parameter<std::string>("comm_log_dir", "./log/test");

    timer_ = this->create_wall_timer(std::chrono::milliseconds(100), [this]() { this->run_once(); });
  }

private:
  void run_once()
  {
    timer_->cancel();

    Hsc3::Comm::CommApi cmApi(log_dir_);
    cmApi.setAutoConn(false);

    const auto ret = cmApi.connect(ip_, port_);
    if (ret != 0) {
      RCLCPP_ERROR(this->get_logger(), "CommApi.connect failed, ret=%llu",
        static_cast<unsigned long long>(ret));
      rclcpp::shutdown();
      return;
    }

    if (!cmApi.isConnected()) {
      RCLCPP_ERROR(this->get_logger(), "CommApi.isConnected returned false");
      rclcpp::shutdown();
      return;
    }

    RCLCPP_INFO(this->get_logger(), "连接成功");
    RCLCPP_INFO(this->get_logger(), "版本信息: %s", cmApi.getVersionStr().c_str());
    (void)cmApi.disconnect();

    rclcpp::shutdown();
  }

private:
  std::string ip_;
  uint16_t port_{23234};
  std::string log_dir_;
  rclcpp::TimerBase::SharedPtr timer_;
};

}  // namespace

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<CommApiTrainingNode>());
  rclcpp::shutdown();
  return 0;
}

