#include <atomic>
#include <memory>
#include <string>
#include <thread>

#include "rclcpp/rclcpp.hpp"

#include "CommApi.h"
#include "proxy/ProxySys.h"

namespace {

class SysAlarmNode : public rclcpp::Node
{
public:
  SysAlarmNode()
  : Node("demo_sys_alarm")
  {
    ip_ = this->declare_parameter<std::string>("ip", "10.10.57.213");
    port_ = static_cast<uint16_t>(this->declare_parameter<int>("port", 23234));

    running_.store(true);
    th_ = std::thread([this]() { this->thread_main(); });
  }

  ~SysAlarmNode() override
  {
    running_.store(false);
    if (th_.joinable()) {
      th_.join();
    }
  }

private:
  void thread_main()
  {
    Hsc3::Comm::CommApi cmApi("");
    Hsc3::Proxy::ProxySys pSys(&cmApi);

    cmApi.setAutoConn(false);
    const auto ret = cmApi.connect(ip_, port_);
    if (ret != 0 || !cmApi.isConnected()) {
      RCLCPP_ERROR(this->get_logger(), "连接失败 ret=%llu",
        static_cast<unsigned long long>(ret));
      return;
    }

    RCLCPP_INFO(this->get_logger(), "连接成功，开始监听系统报警...");

    while (rclcpp::ok() && running_.load()) {
      ErrLevel level = ERR_LEVEL_UNKNOWN;
      uint64_t code = 0;
      std::string msg;

      // 阻塞等待最多 2000ms
      const auto r = pSys.getMessage(level, code, msg, 2000);
      if (r == 0) {
        RCLCPP_WARN(this->get_logger(), "SysMessage level=%d code=%llu msg=%s",
          static_cast<int>(level),
          static_cast<unsigned long long>(code),
          msg.c_str());
      }
    }

    (void)cmApi.disconnect();
  }

private:
  std::string ip_;
  uint16_t port_{23234};
  std::atomic<bool> running_{false};
  std::thread th_;
};

}  // namespace

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<SysAlarmNode>());
  rclcpp::shutdown();
  return 0;
}

