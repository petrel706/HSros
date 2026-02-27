/************************************************************************/
/* 
* @brief 演示：读取当前笛卡尔位置，并循环做Z轴-1mm直线运动（10次）
*/
/************************************************************************/

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <unistd.h>

#include "CommApi.h"
#include "proxy/ProxyMotion.h"
#include "proxy/ProxySys.h"

namespace {

class HRCError : public std::runtime_error {
public:
    explicit HRCError(const std::string & msg) : std::runtime_error(msg) {}
};

class Logger {
public:
    explicit Logger(std::string name) : m_name(std::move(name)) {}

    void info(const std::string & message) const {
        log("INFO", message);
    }

    void error(const std::string & message) const {
        log("ERROR", message);
    }

private:
    void log(const std::string & level, const std::string & message) const {
        // 输出格式严格对齐：%(asctime)s - %(name)s - %(levelname)s - %(message)s
        std::cout << nowAsString() << " - " << m_name << " - " << level << " - " << message << std::endl;
    }

    static std::string nowAsString() {
        using namespace std::chrono;
        const auto tp = system_clock::now();
        const auto t = system_clock::to_time_t(tp);

        std::tm tm{};
        localtime_r(&t, &tm);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

private:
    std::string m_name;
};

inline void throwOnErr(Hsc3::Comm::HMCErrCode ret, const std::string & what) {
    if (ret != 0) {
        std::ostringstream oss;
        oss << what << " failed, ret=" << static_cast<unsigned long long>(ret);
        throw HRCError(oss.str());
    }
}

inline std::string stripQuotes(std::string s) {
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
        return s.substr(1, s.size() - 2);
    }
    return s;
}

inline bool containsEstopKeyword(const std::string & s) {
    return (s.find("急停") != std::string::npos) || (s.find("estop") != std::string::npos) || (s.find("E-Stop") != std::string::npos);
}

inline std::string vecToString(const std::vector<double> & v, size_t n) {
    std::ostringstream oss;
    oss << "[";
    const size_t cnt = std::min(n, v.size());
    for (size_t i = 0; i < cnt; ++i) {
        if (i) oss << ", ";
        oss << v[i];
    }
    oss << "]";
    return oss.str();
}

struct ConnectionConfig {
    std::string ip;
    uint16_t port;
};

// C++ 里的“上下文管理器”：构造->连接，析构->断开
class HRCController {
public:
    HRCController(ConnectionConfig cfg, std::string motion_mode, const Logger & logger)
        : m_cfg(std::move(cfg)), m_motionMode(std::move(motion_mode)), m_logger(logger), m_cmApi("./"), m_pMot(&m_cmApi), m_pSys(&m_cmApi) {}

    HRCController(const HRCController &) = delete;
    HRCController & operator=(const HRCController &) = delete;

    void connect() {
        m_logger.info("Connecting to robot: " + m_cfg.ip + ":" + std::to_string(m_cfg.port));

        // 非自动重连更符合示例程序行为：失败就直接抛异常
        m_cmApi.setAutoConn(false);

        const auto ret = m_cmApi.connect(m_cfg.ip, m_cfg.port);
        throwOnErr(ret, "CommApi.connect");

        if (!m_cmApi.isConnected()) {
            throw HRCError("CommApi.isConnected returned false");
        }

        // motion_mode="cartesian"：设置工作坐标系为世界坐标（笛卡尔）
        // 如果你希望在 TOOL/BASE 下运动，可改为 FRAME_TOOL / FRAME_BASE。
        if (m_motionMode == "cartesian") {
            throwOnErr(m_pMot.setWorkFrame(0, FRAME_WORLD), "ProxyMotion.setWorkFrame(FRAME_WORLD)");
        }

        // 尝试解除“软件急停”。注意：若为物理急停/安全回路触发，软件接口通常无法强行解除。
        {
            const auto r = m_pMot.setEstop(false);
            if (r == 0) {
                m_logger.info("setEstop(false) ok");
            } else {
                m_logger.error("setEstop(false) failed, ret=" + std::to_string(static_cast<unsigned long long>(r)));
            }
        }

        // 示教模式 + 上使能（与你现有示例 TrainingMove.cpp 保持一致）
        throwOnErr(m_pMot.setOpMode(OP_T1), "ProxyMotion.setOpMode(OP_T1)");
        throwOnErr(m_pMot.setGpEn(0, true), "ProxyMotion.setGpEn(true)");

        // 打印一些关键状态，帮助定位“返回成功但不动”问题
        {
            bool estop = false;
            const auto r1 = m_pMot.getEstop(estop);
            if (r1 == 0) {
                m_logger.info(std::string("Estop=") + (estop ? "true" : "false"));
            }

            OpMode mode{};
            const auto r2 = m_pMot.getOpMode(mode);
            if (r2 == 0) {
                m_logger.info("OpMode=" + std::to_string(static_cast<int>(mode)));
            }
        }

        sleep(1);
        m_logger.info("Connected and enabled");
    }

    void disconnect() noexcept {
        try {
            // 下使能
            (void)m_pMot.setGpEn(0, false);
            (void)m_cmApi.disconnect();
        } catch (...) {
            // 析构/清理阶段不再抛异常
        }
    }

    ~HRCController() {
        disconnect();
    }

    // 获取当前笛卡尔位置：XYZABC（单位：mm/度）
    LocData get_current_cartesian_position() {
        LocData data;
        const auto ret = m_pMot.getLocData(0, data);
        throwOnErr(ret, "ProxyMotion.getLocData");

        if (data.size() < 6) {
            throw HRCError("LocData size < 6, cannot parse XYZABC");
        }
        return data;
    }

    // 获取当前关节角：J1..J6（单位：度）
    JntData get_current_joint_position() {
        JntData data;
        const auto ret = m_pMot.getJntData(0, data);
        throwOnErr(ret, "ProxyMotion.getJntData");
        if (data.size() < 6) {
            throw HRCError("JntData size < 6");
        }
        return data;
    }

    // 直线运动到目标笛卡尔位置
    void move_to_cartesian_position(const LocData & xyzabc) {
        if (xyzabc.size() < 6) {
            throw HRCError("Target LocData size < 6");
        }

        int32_t config = 1048576;
        (void)m_pMot.getConfig(0, config);

        GeneralPos target{};
        target.isJoint = false;
        target.ufNum = -1;
        target.utNum = -1;
        target.config = config;

        // GeneralPos.vecPos 在你之前示例里是 9 个元素，这里按 XYZABC + 3个0 补齐
        target.vecPos.clear();
        target.vecPos.reserve(9);
        target.vecPos.push_back(xyzabc[0]);
        target.vecPos.push_back(xyzabc[1]);
        target.vecPos.push_back(xyzabc[2]);
        target.vecPos.push_back(xyzabc[3]);
        target.vecPos.push_back(xyzabc[4]);
        target.vecPos.push_back(xyzabc[5]);
        target.vecPos.push_back(0);
        target.vecPos.push_back(0);
        target.vecPos.push_back(0);

        const auto ret = m_pMot.moveTo(0, target, true /*isLinear*/);
        throwOnErr(ret, "ProxyMotion.moveTo(linear)");
    }

    struct MotionWaitInfo {
        bool done;
        std::string group_status_raw;
        std::string async_msg;
    };

    // 等待运动停止：通过业务层状态查询（和你之前 TrainingMove.cpp 的思路一致）
    MotionWaitInfo wait_motion_done(int timeout_ms = 8000) {
        const int sleep_ms = 50;
        int elapsed = 0;

        MotionWaitInfo info{false, "", ""};

        while (elapsed < timeout_ms) {
            std::string reStr{};
            const auto ret = m_cmApi.execCmd("?group[0].status", reStr, 3);
            if (ret != 0) {
                // execCmd 失败：把错误抛出，方便你看到根因
                throwOnErr(ret, "CommApi.execCmd(?group[0].status)");
            }

            info.group_status_raw = reStr;

            // 控制器返回值通常是带引号的字符串，例如 "2"
            const std::string stat = stripQuotes(reStr);

            // 常见：2=停止/静止（你原先代码也是按"2"判断）
            if (stat == "2") {
                info.done = true;
                return info;
            }

            usleep(sleep_ms * 1000);
            elapsed += sleep_ms;
        }

        m_logger.error("wait_motion_done timeout");

        // 超时后记录一次 group 状态，便于分析（原始值包含引号）
        if (!info.group_status_raw.empty()) {
            m_logger.error("group[0].status(raw)=" + info.group_status_raw);
        }

        // 超时后尝试读一次异步信息（如果控制器有报警/拦截原因，可能会在异步信息里）
        std::string msg;
        (void)m_cmApi.getAsyncMsg(msg);
        if (!msg.empty()) {
            m_logger.error("asyncMsg=" + msg);
            info.async_msg = msg;
        }

        return info;
    }

    // 尝试解除急停并复位组（不保证对物理急停有效）
    void try_clear_estop_and_reset() {
        const auto r1 = m_pMot.setEstop(false);
        if (r1 == 0) {
            m_logger.info("setEstop(false) ok");
        } else {
            m_logger.error("setEstop(false) failed, ret=" + std::to_string(static_cast<unsigned long long>(r1)));
        }

        const auto r2 = m_pMot.gpReset(0);
        if (r2 == 0) {
            m_logger.info("gpReset(0) ok");
        } else {
            m_logger.error("gpReset(0) failed, ret=" + std::to_string(static_cast<unsigned long long>(r2)));
        }

        // 读一次异步信息，帮助确认是否仍是急停/安全链路
        std::string msg;
        (void)m_cmApi.getAsyncMsg(msg);
        if (!msg.empty()) {
            m_logger.error("asyncMsg(after reset)=" + msg);
        }
    }

    void dump_diagnostics_on_estop() {
        // 1) 再打印一次 estop/en 状态
        bool estop = false;
        if (m_pMot.getEstop(estop) == 0) {
            m_logger.error(std::string("Diag Estop=") + (estop ? "true" : "false"));
        }
        bool en = false;
        if (m_pMot.getGpEn(0, en) == 0) {
            m_logger.error(std::string("Diag GpEn=") + (en ? "true" : "false"));
        }

        // 2) 尝试读取系统消息（若有报警/安全拦截，可能在这里）
        //    ulWaitTime 单位 ms：这里用短等待避免卡住
        ErrLevel level{};
        uint64_t code = 0;
        std::string strMsg;
        const auto r = m_pSys.getMessage(level, code, strMsg, 200);
        if (r == 0) {
            m_logger.error("SysMessage level=" + std::to_string(static_cast<int>(level)) + ", code=" + std::to_string(static_cast<unsigned long long>(code)) + ", msg=" + strMsg);

            // 3) 对错误码做进一步查询
            std::string reason, elim;
            const auto r2 = m_pSys.queryError(code, reason, elim);
            if (r2 == 0) {
                m_logger.error("SysMessage reason=" + reason);
                m_logger.error("SysMessage elim=" + elim);
            }
        }

        // 4) 尽可能多地把异步信息读干净（有时多条）
        for (int i = 0; i < 5; ++i) {
            std::string msg;
            (void)m_cmApi.getAsyncMsg(msg);
            if (msg.empty()) {
                break;
            }
            m_logger.error("Drain asyncMsg=" + msg);
        }
    }

private:
    ConnectionConfig m_cfg;
    std::string m_motionMode;
    const Logger & m_logger;

    Hsc3::Comm::CommApi m_cmApi;
    Hsc3::Proxy::ProxyMotion m_pMot;
    Hsc3::Proxy::ProxySys m_pSys;
};

void run_demo() {
    // 1. 初始化日志（INFO级别，格式：%(asctime)s - %(name)s - %(levelname)s - %(message)s）
    Logger logger("demo");

    // 2. 配置机器人IP与端口
    ConnectionConfig cfg{.ip = "10.10.57.213", .port = 23234};

    // 3. 使用“上下文管理器”创建控制实例，并连接（motion_mode="cartesian"）
    HRCController controller(cfg, "cartesian", logger);
    controller.connect();

    // 输出开始运动时的笛卡尔与关节角度
    {
        const auto cart0 = controller.get_current_cartesian_position();
        const auto jnt0 = controller.get_current_joint_position();
        logger.info("Start Cartesian XYZABC=" + vecToString(cart0, 6));
        logger.info("Start Joint J1..J6=" + vecToString(jnt0, 6));
    }

    // 4. 获取当前笛卡尔位置，并循环10次：Z轴每次-1mm，延时0.02秒后运动
    auto pose = controller.get_current_cartesian_position();

    for (int i = 0; i < 10; ++i) {
        pose[2] -= 1.0; // Z轴减1mm

        usleep(20 * 1000); // 0.02秒

        // 如果遇到急停导致不动：尝试解除急停并重试（有限次数），然后继续执行
        const int max_retry = 3;
        bool ok = false;
        for (int retry = 0; retry < max_retry; ++retry) {
            controller.move_to_cartesian_position(pose);
            const auto waitInfo = controller.wait_motion_done();
            ok = waitInfo.done;
            if (ok) {
                break;
            }

            // 超时后：如果 waitInfo.async_msg 包含“急停”，则尝试解除并复位后重试
            if (!waitInfo.async_msg.empty() && containsEstopKeyword(waitInfo.async_msg)) {
                controller.dump_diagnostics_on_estop();
                controller.try_clear_estop_and_reset();
                usleep(200 * 1000);
            } else {
                logger.error("Move timeout but no estop keyword in async msg; stop retrying this step");
                break;
            }
        }

        if (!ok) {
            logger.error("Move failed after retries, continue to next step");
        }

        // 仅打印X轴值（仅打印数值）
        std::cout << pose[0] << std::endl;
    }

    // 输出运动结束后的笛卡尔与关节角度
    {
        const auto cart1 = controller.get_current_cartesian_position();
        const auto jnt1 = controller.get_current_joint_position();
        logger.info("End Cartesian XYZABC=" + vecToString(cart1, 6));
        logger.info("End Joint J1..J6=" + vecToString(jnt1, 6));
    }
}

} // namespace

int main() {
    try {
        run_demo();
        return 0;
    } catch (const HRCError & e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
        return 2;
    } catch (...) {
        std::cerr << "Unknown error" << std::endl;
        return 3;
    }
}
