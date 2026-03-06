#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "CommApi.h"
#include "proxy/ProxyIO.h"
#include "proxy/ProxyMotion.h"
#include "proxy/ProxySys.h"

namespace hsr_socket_driver {

class Hsc3Client
{
public:
  struct Config
  {
    std::string ip = "10.10.57.213";
    uint16_t port = 23234;
    int8_t gp_id = 0;

    // 运行时安全相关：默认不强行覆盖真实 DI
    bool force_virtual_din = false;
    int32_t force_din_port = 0;
    bool force_din_value = true;
    int32_t force_din_hold_ms = 2000;
    bool allow_override_real_din = false;

    // 若启用了 force_virtual_din：断开连接前是否把该 DI 写回 0
    bool reset_din_on_disconnect = false;
  };

  explicit Hsc3Client(std::string comm_log_dir = "");
  ~Hsc3Client();

  Hsc3Client(const Hsc3Client &) = delete;
  Hsc3Client & operator=(const Hsc3Client &) = delete;

  void set_config(const Config & cfg) { cfg_ = cfg; }
  const Config & config() const { return cfg_; }

  void connect();
  void disconnect() noexcept;
  bool is_connected() const;

  // 基础模式/使能
  void set_op_mode(OpMode mode);
  OpMode get_op_mode() const;
  void set_op_mode_t1();
  void set_manual_continue();
  void set_auto_continue();
  void set_work_frame_joint();
  void set_work_frame_world();

  // 速度倍率（1~100，单位：%）
  void set_vord(int32_t vord_percent);
  void set_jog_vord(int32_t vord_percent);

  bool set_gp_enable(bool en);
  bool get_gp_enable() const;
  bool try_clear_estop_and_enable(int attempts = 3);

  // 位置读取：关节单位（度）/（弧度）
  std::vector<double> get_joint_deg() const;
  std::vector<double> get_joint_rad() const;

  // 位置读取：笛卡尔（mm/deg），返回至少 6 个元素（XYZABC）
  std::vector<double> get_loc_xyzabc() const;

  // 运动
  void move_joint_deg(const std::vector<double> & joint_deg);
  void move_cartesian_linear_xyzabc(const std::vector<double> & xyzabc);

  // 等待运动结束（group.status==2）
  bool wait_group_done(int timeout_ms = 8000) const;

  std::string get_async_msg() const;

private:
  void maybe_force_virtual_din_value(bool value);
  void maybe_force_virtual_din();

  Config cfg_;

  // SDK objects (must outlive proxies)
  mutable Hsc3::Comm::CommApi cmApi_;
  mutable Hsc3::Proxy::ProxyMotion pMot_;
  mutable Hsc3::Proxy::ProxySys pSys_;
  mutable Hsc3::Proxy::ProxyIO pIo_;
};

}  // namespace hsr_socket_driver

