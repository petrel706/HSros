#include "hsr_socket_driver/hsc3_client.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <stdexcept>
#include <thread>

namespace hsr_socket_driver {
namespace {

inline double deg2rad(double deg) { return deg * M_PI / 180.0; }
inline double rad2deg(double rad) { return rad * 180.0 / M_PI; }

inline std::string strip_quotes(const std::string & s)
{
  if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
    return s.substr(1, s.size() - 2);
  }
  return s;
}

inline void throw_on_err(Hsc3::Comm::HMCErrCode ret, const std::string & what)
{
  if (ret != 0) {
    throw std::runtime_error(what + " failed, ret=" + std::to_string(static_cast<unsigned long long>(ret)));
  }
}

}  // namespace

Hsc3Client::Hsc3Client(std::string comm_log_dir)
: cmApi_(std::move(comm_log_dir)), pMot_(&cmApi_), pSys_(&cmApi_), pIo_(&cmApi_)
{
}

Hsc3Client::~Hsc3Client()
{
  disconnect();
}

void Hsc3Client::connect()
{
  cmApi_.setAutoConn(false);
  const auto ret = cmApi_.connect(cfg_.ip, cfg_.port);
  throw_on_err(ret, "CommApi.connect");
  if (!cmApi_.isConnected()) {
    throw std::runtime_error("CommApi.isConnected returned false");
  }

  maybe_force_virtual_din();
}

void Hsc3Client::disconnect() noexcept
{
  try {
    if (cmApi_.isConnected() && cfg_.force_virtual_din && cfg_.reset_din_on_disconnect) {
      // 退出前把强制 DI 写回 0（若该 port 是真实 DI 且不允许覆盖，会被保护逻辑忽略）
      maybe_force_virtual_din_value(false);
    }
    (void)cmApi_.disconnect();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  } catch (...) {
  }
}

bool Hsc3Client::is_connected() const
{
  return cmApi_.isConnected();
}

void Hsc3Client::set_op_mode(OpMode mode)
{
  throw_on_err(pMot_.setOpMode(mode), "ProxyMotion.setOpMode");
}

OpMode Hsc3Client::get_op_mode() const
{
  OpMode mode = OP_NONE;
  throw_on_err(pMot_.getOpMode(mode), "ProxyMotion.getOpMode");
  return mode;
}

void Hsc3Client::set_op_mode_t1()
{
  throw_on_err(pMot_.setOpMode(OP_T1), "ProxyMotion.setOpMode(OP_T1)");
}

void Hsc3Client::set_manual_continue()
{
  throw_on_err(pMot_.setManualMode(MANUAL_CONTINUE), "ProxyMotion.setManualMode(MANUAL_CONTINUE)");
}

void Hsc3Client::set_auto_continue()
{
  throw_on_err(pMot_.setAutoMode(AUTO_MODE_CONTINUE), "ProxyMotion.setAutoMode(AUTO_MODE_CONTINUE)");
}

void Hsc3Client::set_work_frame_joint()
{
  throw_on_err(pMot_.setWorkFrame(cfg_.gp_id, FRAME_JOINT), "ProxyMotion.setWorkFrame(FRAME_JOINT)");
}

void Hsc3Client::set_work_frame_world()
{
  throw_on_err(pMot_.setWorkFrame(cfg_.gp_id, FRAME_WORLD), "ProxyMotion.setWorkFrame(FRAME_WORLD)");
}

void Hsc3Client::set_vord(int32_t vord_percent)
{
  vord_percent = std::max<int32_t>(1, std::min<int32_t>(100, vord_percent));
  throw_on_err(pMot_.setVord(vord_percent), "ProxyMotion.setVord");
}

void Hsc3Client::set_jog_vord(int32_t vord_percent)
{
  vord_percent = std::max<int32_t>(1, std::min<int32_t>(100, vord_percent));
  throw_on_err(pMot_.setJogVord(vord_percent), "ProxyMotion.setJogVord");
}

bool Hsc3Client::set_gp_enable(bool en)
{
  const auto r = pMot_.setGpEn(cfg_.gp_id, en);
  if (r != 0) {
    return false;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  bool gpEn = false;
  if (pMot_.getGpEn(cfg_.gp_id, gpEn) != 0) {
    return false;
  }
  return gpEn == en;
}

bool Hsc3Client::get_gp_enable() const
{
  bool gpEn = false;
  throw_on_err(pMot_.getGpEn(cfg_.gp_id, gpEn), "ProxyMotion.getGpEn");
  return gpEn;
}

bool Hsc3Client::try_clear_estop_and_enable(int attempts)
{
  for (int i = 0; i < attempts; ++i) {
    maybe_force_virtual_din();
    (void)pSys_.reset();
    (void)pMot_.setEstop(false);
    (void)pMot_.gpReset(cfg_.gp_id);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    if (set_gp_enable(true)) {
      return true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
  }
  return false;
}

std::vector<double> Hsc3Client::get_joint_deg() const
{
  JntData data;
  throw_on_err(pMot_.getJntData(cfg_.gp_id, data), "ProxyMotion.getJntData");
  return std::vector<double>(data.begin(), data.end());
}

std::vector<double> Hsc3Client::get_joint_rad() const
{
  auto deg = get_joint_deg();
  for (auto & v : deg) {
    v = deg2rad(v);
  }
  return deg;
}

std::vector<double> Hsc3Client::get_loc_xyzabc() const
{
  LocData data;
  throw_on_err(pMot_.getLocData(cfg_.gp_id, data), "ProxyMotion.getLocData");
  if (data.size() < 6) {
    throw std::runtime_error("LocData size < 6");
  }
  return std::vector<double>(data.begin(), data.end());
}

void Hsc3Client::move_joint_deg(const std::vector<double> & joint_deg)
{
  if (joint_deg.size() < 6) {
    throw std::runtime_error("move_joint_deg expects at least 6 joints (deg)");
  }

  GeneralPos p{};
  p.isJoint = true;
  p.ufNum = -1;
  p.utNum = -1;
  p.config = 0;
  p.vecPos.assign(joint_deg.begin(), joint_deg.end());

  throw_on_err(pMot_.moveTo(cfg_.gp_id, p, false), "ProxyMotion.moveTo(joint)");
}

void Hsc3Client::move_cartesian_linear_xyzabc(const std::vector<double> & xyzabc)
{
  if (xyzabc.size() < 6) {
    throw std::runtime_error("move_cartesian_linear_xyzabc expects at least 6 elements (XYZABC)");
  }

  int32_t config = 1048576;
  (void)pMot_.getConfig(cfg_.gp_id, config);

  GeneralPos p{};
  p.isJoint = false;
  p.ufNum = -1;
  p.utNum = -1;
  p.config = config;
  p.vecPos.clear();
  p.vecPos.reserve(9);
  for (size_t i = 0; i < 6; ++i) {
    p.vecPos.push_back(xyzabc[i]);
  }
  p.vecPos.push_back(0);
  p.vecPos.push_back(0);
  p.vecPos.push_back(0);

  throw_on_err(pMot_.moveTo(cfg_.gp_id, p, true), "ProxyMotion.moveTo(cartesian linear)");
}

bool Hsc3Client::wait_group_done(int timeout_ms) const
{
  const int sleep_ms = 50;
  int elapsed = 0;

  while (elapsed < timeout_ms) {
    std::string reStr;
    const auto ret = cmApi_.execCmd("?group[" + std::to_string(static_cast<int>(cfg_.gp_id)) + "].status", reStr, 3);
    if (ret != 0) {
      return false;
    }

    if (strip_quotes(reStr) == "2") {
      return true;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    elapsed += sleep_ms;
  }
  return false;
}

std::string Hsc3Client::get_async_msg() const
{
  std::string msg;
  (void)cmApi_.getAsyncMsg(msg);
  return msg;
}

void Hsc3Client::maybe_force_virtual_din()
{
  if (!cfg_.force_virtual_din) {
    return;
  }

  maybe_force_virtual_din_value(cfg_.force_din_value);
}

void Hsc3Client::maybe_force_virtual_din_value(bool value)
{
  if (!cfg_.force_virtual_din) {
    return;
  }

  const int32_t port = cfg_.force_din_port;
  const int32_t holdMs = cfg_.force_din_hold_ms;

  const int32_t grp = port / 32;
  const int32_t bit = port % 32;
  uint32_t maskGrp = 0;
  if (pIo_.getDinMaskGrp(grp, maskGrp) == 0) {
    const bool isVirtual = ((maskGrp >> bit) & 0x1u) != 0;
    if (!isVirtual && !cfg_.allow_override_real_din) {
      // 不覆盖真实 DI：直接返回
      return;
    }
  }

  (void)pIo_.setDinMaskBit(port, true);

  const int32_t loops = (holdMs > 0) ? std::max<int32_t>(1, holdMs / 50) : 1;
  for (int32_t i = 0; i < loops; ++i) {
    (void)pIo_.setDin(port, value);
    if (holdMs > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  }
}

}  // namespace hsr_socket_driver

