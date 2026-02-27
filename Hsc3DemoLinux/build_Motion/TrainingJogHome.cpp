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
bool waitDone(Hsc3::Proxy::ProxyMotion & pMot, int8_t gpId);

static bool tryClearEstopAndEnable(Hsc3::Proxy::ProxyMotion & pMot, Hsc3::Proxy::ProxySys & pSys);
static void forceDin0Ready(Hsc3::Proxy::ProxyIO & pIo);
static bool moveToHomeJoint(Hsc3::Proxy::ProxyMotion & pMot, int8_t gpId);
static bool ensureJointLimitAllowsHome(Hsc3::Proxy::ProxyMotion & pMot, int8_t gpId, const JntData & home);
static void dumpSysMessages(Hsc3::Proxy::ProxySys & pSys);

static const int8_t kGpId = 0;
static const int32_t kJogVord = 20;

static const int32_t kForceDinPort = 0;
static const bool kForceDinValue = true;
static const int32_t kForceDinHoldMs = 2000;

static int run_training_jog_home()
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

    std::cout << "Move to HOME (Joint) J1..J6=[-5.44e-05, -90.0001, 89.9999, -89.9999, 90.0001, -0.0001632]" << std::endl;
    if (!moveToHomeJoint(pMot, kGpId))
    {
        std::cout << "move home failed" << std::endl;
        dumpSysMessages(pSys);
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

static bool moveToHomeJoint(Hsc3::Proxy::ProxyMotion & pMot, int8_t gpId)
{
    JntData target;
    target.resize(6);
    target[0] = -5.44e-05;
    target[1] = -90.0001;
    target[2] = 89.9999;
    target[3] = -89.9999;
    target[4] = 90.0001;
    target[5] = -0.0001632;

    (void)ensureJointLimitAllowsHome(pMot, gpId, target);

    GeneralPos p;
    p.isJoint = true;
    p.ufNum = -1;
    p.utNum = -1;
    p.config = 0;
    p.vecPos = target;

    const auto r = pMot.moveTo(gpId, p, false);
    if (r != 0)
    {
        std::cout << "moveTo failed, ret=" << static_cast<unsigned long long>(r) << std::endl;
        return false;
    }

    return waitDone(pMot, gpId);
}

static bool ensureJointLimitAllowsHome(Hsc3::Proxy::ProxyMotion & pMot, int8_t gpId, const JntData & home)
{
    JntData pos;
    JntData neg;
    int32_t mask = 0;
    const auto r1 = pMot.getJointLimit(gpId, pos, neg, mask);
    std::cout << "getJointLimit ret=" << static_cast<unsigned long long>(r1) << ", mask=" << mask << std::endl;
    if (r1 != 0)
    {
        return false;
    }

    const size_t n = std::min(home.size(), std::min(pos.size(), neg.size()));
    for (size_t i = 0; i < n; ++i)
    {
        std::cout << "limit axis " << i << ": neg=" << neg[i] << ", pos=" << pos[i] << ", home=" << home[i] << std::endl;
    }

    int32_t needMask = 0;
    for (size_t i = 0; i < n && i < 9; ++i)
    {
        const bool enabled = ((mask >> static_cast<int32_t>(i)) & 0x1) != 0;
        if (!enabled)
        {
            continue;
        }

        if (home[i] > pos[i] || home[i] < neg[i])
        {
            needMask |= (1 << static_cast<int32_t>(i));
        }
    }

    if (needMask == 0)
    {
        return true;
    }

    JntData newPos = pos;
    JntData newNeg = neg;
    for (size_t i = 0; i < n && i < 9; ++i)
    {
        if (((needMask >> static_cast<int32_t>(i)) & 0x1) == 0)
        {
            continue;
        }
        newPos[i] = 360.0;
        newNeg[i] = -360.0;
    }

    const auto r2 = pMot.setJointLimit(gpId, newPos, newNeg, needMask);
    std::cout << "setJointLimit ret=" << static_cast<unsigned long long>(r2) << ", mask=" << needMask << std::endl;
    return r2 == 0;
}

static void dumpSysMessages(Hsc3::Proxy::ProxySys & pSys)
{
    for (int i = 0; i < 5; ++i)
    {
        ErrLevel level = ERR_LEVEL_UNKNOWN;
        uint64_t code = 0;
        std::string msg;
        const auto r = pSys.getMessage(level, code, msg, 0);
        if (r != 0)
        {
            break;
        }

        std::cout << "SysMsg level=" << static_cast<int>(level) << ", code=" << code << ", msg=" << msg << std::endl;
        if (code != 0)
        {
            std::string reason;
            std::string elim;
            const auto r2 = pSys.queryError(code, reason, elim);
            if (r2 == 0)
            {
                std::cout << "SysMsg detail reason=" << reason << std::endl;
                std::cout << "SysMsg detail elim=" << elim << std::endl;
            }
        }
    }
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

bool waitDone(Hsc3::Proxy::ProxyMotion & pMot, int8_t gpId)
{
    ManState manualState = MAN_STATE_MAX;
    const int32_t maxLoops = 600;
    JntData last;
    (void)pMot.getJntData(gpId, last);

    for (int32_t i = 0; i < maxLoops; ++i)
    {
        (void)pMot.getManualStat(manualState);
        if (manualState == MAN_STATE_WAIT || manualState == MAN_STATE_ERROR)
        {
            break;
        }

        usleep(50 * 1000);

        if ((i % 20) == 0)
        {
            JntData cur;
            const auto r = pMot.getJntData(gpId, cur);
            if (r == 0 && cur.size() >= 6)
            {
                std::cout << "manualState=" << static_cast<int>(manualState)
                          << " jnt=[" << cur[0] << ", " << cur[1] << ", " << cur[2] << ", "
                          << cur[3] << ", " << cur[4] << ", " << cur[5] << "]" << std::endl;
            }
            else
            {
                std::cout << "manualState=" << static_cast<int>(manualState)
                          << " getJntData ret=" << static_cast<unsigned long long>(r) << std::endl;
            }
            last = cur;
        }
    }

    if (manualState != MAN_STATE_WAIT && manualState != MAN_STATE_ERROR)
    {
        std::cout << "waitDone timeout, manualState=" << static_cast<int>(manualState) << std::endl;
        const auto r = pMot.stopJog(gpId);
        std::cout << "stopJog ret=" << static_cast<unsigned long long>(r) << std::endl;
        return false;
    }

    return manualState == MAN_STATE_WAIT;
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
    return run_training_jog_home();
}
