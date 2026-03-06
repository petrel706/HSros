#include <iostream>
#include <unistd.h>
#include <algorithm>

#include "CommApi.h"
#include "proxy/ProxyMotion.h"
#include "proxy/ProxySys.h"
#include "proxy/ProxyIO.h"

bool connectIPC(Hsc3::Comm::CommApi &cmApi, std::string strIP, uint16_t uPort);
bool disconnectIPC(Hsc3::Comm::CommApi & cmApi);
bool setEn(Hsc3::Proxy::ProxyMotion & pMot, bool en);
void waitDone(Hsc3::Proxy::ProxyMotion & pMot);

static bool tryClearEstopAndEnable(Hsc3::Proxy::ProxyMotion & pMot, Hsc3::Proxy::ProxySys & pSys);
static void forceDin0Ready(Hsc3::Proxy::ProxyIO & pIo);
static bool moveJointRelativeDeg(Hsc3::Proxy::ProxyMotion & pMot, int8_t gpId, int8_t axis, double deltaDeg);

static const int8_t kGpId = 0;
static const int8_t kAxis = 5;
static const double kDeltaDeg = -15.0;
static const int32_t kJogVord = 20;

static const int32_t kForceDinPort = 0;
static const bool kForceDinValue = true;
static const int32_t kForceDinHoldMs = 2000;

static int run_training_jog_single()
{
    Hsc3::Comm::CommApi cmApi("");
    Hsc3::Proxy::ProxyMotion pMot(&cmApi);
    Hsc3::Proxy::ProxySys pSys(&cmApi);
    Hsc3::Proxy::ProxyIO pIo(&cmApi);

    if (!connectIPC(cmApi, "10.10.57.213", 23234))
    {
        return 1;
    }

    forceDin0Ready(pIo);

    (void)pMot.setOpMode(OP_T1);
    (void)pMot.setManualMode(MANUAL_CONTINUE);
    (void)pMot.setWorkFrame(kGpId, FRAME_JOINT);
    (void)pMot.setJogVord(kJogVord);

    if (!(setEn(pMot, true) || tryClearEstopAndEnable(pMot, pSys)))
    {
        std::cout << "使能失败" << std::endl;
        (void)setEn(pMot, false);
        (void)disconnectIPC(cmApi);
        return 2;
    }

    std::cout << "Axis " << static_cast<int>(kAxis) << " move " << kDeltaDeg << " deg, JogVord=" << kJogVord << std::endl;
    if (!moveJointRelativeDeg(pMot, kGpId, kAxis, kDeltaDeg))
    {
        std::cout << "move failed" << std::endl;
    }

    (void)setEn(pMot, false);
    (void)disconnectIPC(cmApi);
    return 0;
}

static bool tryClearEstopAndEnable(Hsc3::Proxy::ProxyMotion & pMot, Hsc3::Proxy::ProxySys & pSys)
{
    for (int attempt = 0; attempt < 3; ++attempt)
    {
        (void)pSys.reset();
        (void)pMot.setEstop(false);
        (void)pMot.gpReset(kGpId);
        usleep(300 * 1000);
        if (setEn(pMot, true))
        {
            return true;
        }
        usleep(300 * 1000);
    }
    return false;
}

static void forceDin0Ready(Hsc3::Proxy::ProxyIO & pIo)
{
    (void)pIo.setDinMaskBit(kForceDinPort, true);

    const int32_t loops = (kForceDinHoldMs > 0) ? std::max<int32_t>(1, kForceDinHoldMs / 50) : 1;
    for (int32_t i = 0; i < loops; ++i)
    {
        (void)pIo.setDin(kForceDinPort, kForceDinValue);
        if (kForceDinHoldMs > 0)
        {
            usleep(50 * 1000);
        }
    }
}

static bool moveJointRelativeDeg(Hsc3::Proxy::ProxyMotion & pMot, int8_t gpId, int8_t axis, double deltaDeg)
{
    // Get current position
    JntData cur;
    const auto r1 = pMot.getJntData(gpId, cur);
    if (r1 != 0)
    {
        std::cout << "getJntData failed, ret=" << static_cast<unsigned long long>(r1) << std::endl;
        return false;
    }
    if (axis < 0 || static_cast<size_t>(axis) >= cur.size())
    {
        std::cout << "invalid axis=" << static_cast<int>(axis) << ", cur.size=" << cur.size() << std::endl;
        return false;
    }

    const double startPos = cur[static_cast<size_t>(axis)];
    const double targetPos = startPos + deltaDeg;
    
    std::cout << "Current axis " << static_cast<int>(axis) << " pos: " << startPos << " deg" << std::endl;
    std::cout << "Target pos: " << targetPos << " deg" << std::endl;

    // Determine direction
    DirectType direction = (deltaDeg > 0) ? POSITIVE : NEGATIVE;
    const double absDelta = (deltaDeg > 0) ? deltaDeg : -deltaDeg;

    // Start jogging
    std::cout << "Starting jog in " << (direction == POSITIVE ? "POSITIVE" : "NEGATIVE") << " direction..." << std::endl;
    const auto r2 = pMot.startJog(gpId, axis, direction);
    if (r2 != 0)
    {
        std::cout << "startJog failed, ret=" << static_cast<unsigned long long>(r2) << std::endl;
        return false;
    }

    // Monitor position and stop when target reached
    const int maxLoops = 1000; // 50 seconds max
    bool reached = false;
    
    for (int i = 0; i < maxLoops; ++i)
    {
        usleep(50 * 1000); // 50ms
        
        JntData current;
        if (pMot.getJntData(gpId, current) == 0 && static_cast<size_t>(axis) < current.size())
        {
            const double currentPos = current[static_cast<size_t>(axis)];
            const double moved = (direction == POSITIVE) ? (currentPos - startPos) : (startPos - currentPos);
            
            // Print progress every 1 second
            if ((i % 20) == 0)
            {
                std::cout << "Progress: " << currentPos << " deg (moved " << moved << " deg)" << std::endl;
            }
            
            // Check if target reached
            if (moved >= absDelta - 0.5) // 0.5 degree tolerance
            {
                reached = true;
                std::cout << "Target reached at " << currentPos << " deg" << std::endl;
                break;
            }
        }
    }

    // Stop jogging
    const auto r3 = pMot.stopJog(gpId);
    std::cout << "stopJog ret=" << static_cast<unsigned long long>(r3) << std::endl;
    
    if (!reached)
    {
        std::cout << "Warning: Target not reached within timeout" << std::endl;
    }

    return reached;
}

bool connectIPC(Hsc3::Comm::CommApi &cmApi, std::string strIP, uint16_t uPort)
{
    cmApi.setAutoConn(false);
    Hsc3::Comm::HMCErrCode ret = cmApi.connect(strIP, uPort);
    if (ret != 0)
    {
        std::cout << "CommApi::connect : ret = " << ret << std::endl;
    }
    if (cmApi.isConnected())
    {
        std::cout << "连接成功" << std::endl;
        return true;
    }
    std::cout << "连接失败" << std::endl;
    return false;
}

bool disconnectIPC(Hsc3::Comm::CommApi & cmApi)
{
    (void)cmApi.disconnect();
    sleep(1);
    return !cmApi.isConnected();
}

void waitDone(Hsc3::Proxy::ProxyMotion & pMot)
{
    ManState manualState = MAN_STATE_MAX;
    while (1)
    {
        (void)pMot.getManualStat(manualState);
        if (manualState == MAN_STATE_WAIT || manualState == MAN_STATE_ERROR)
        {
            break;
        }
    }
}

bool setEn(Hsc3::Proxy::ProxyMotion & pMot, bool en)
{
    bool gpEn = false;
    (void)pMot.getGpEn(kGpId, gpEn);
    if (gpEn == en)
    {
        return true;
    }

    const auto r = pMot.setGpEn(kGpId, en);
    std::cout << "setGpEn(" << (en ? "true" : "false") << ") ret=" << static_cast<unsigned long long>(r) << std::endl;
    sleep(2);

    (void)pMot.getGpEn(kGpId, gpEn);
    return gpEn == en;
}

int main()
{
    return run_training_jog_single();
}
