/************************************************************************/
/* 
* @brief 演示控制机器人单轴点动。
*/
/************************************************************************/
#include <iostream>
//#include <windows.h>
#include <unistd.h>
#include <algorithm>
#include <cstdlib>
#include "CommApi.h"
#include "proxy/ProxyMotion.h"
#include "proxy/ProxySys.h"
#include "proxy/ProxyIO.h"

bool connectIPC(Hsc3::Comm::CommApi &cmApi,std::string strIP,uint16_t uPort);
bool disconnectIPC(Hsc3::Comm::CommApi & cmApi);
bool setEn(Hsc3::Proxy::ProxyMotion & pMot, bool en);
void waitDone(Hsc3::Proxy::ProxyMotion & pMot);

static bool tryClearEstopAndEnable(Hsc3::Comm::CommApi & cmApi, Hsc3::Proxy::ProxyMotion & pMot, Hsc3::Proxy::ProxySys & pSys, Hsc3::Proxy::ProxyIO & pIo);

static bool getEnvBool(const char * name, bool defVal);
static int32_t getEnvInt32(const char * name, int32_t defVal);
static double getEnvDouble(const char * name, double defVal);

static const int32_t kForceDinPort = 0;
static const bool kForceDinValue = true;
static const int32_t kForceDinHoldMs = 2000;
static const bool kAllowOverrideRealDinToVirtual = true;
static void tryForceVirtualDin(Hsc3::Proxy::ProxyIO & pIo);

static bool moveJointRelativeDeg(Hsc3::Proxy::ProxyMotion & pMot, int8_t gpId, int8_t axis, double deltaDeg);

static int run_training_jog()
{
    Hsc3::Comm::CommApi cmApi("");
    Hsc3::Proxy::ProxyMotion pMot(&cmApi);
    Hsc3::Proxy::ProxySys pSys(&cmApi);
    Hsc3::Proxy::ProxyIO pIo(&cmApi);

    if (connectIPC(cmApi,"10.10.57.213",23234))
    {
        tryForceVirtualDin(pIo);

        //设置手动模式
        pMot.setOpMode(OP_T1);

        //设置手动增量模式
        pMot.setManualMode(MANUAL_CONTINUE);
        
        //设置坐标系
        pMot.setWorkFrame(0,FRAME_JOINT);
        
        //机器人上使能
        if (setEn(pMot,true) || tryClearEstopAndEnable(cmApi, pMot, pSys, pIo))
        {
            const char * envDelta = std::getenv("HSC3_JOINT_DELTA_DEG");
            const double deltaDeg = getEnvDouble("HSC3_JOINT_DELTA_DEG", 10.0);
            if (envDelta && *envDelta)
            {
                std::cout << "JointDeltaDeg from env: " << envDelta << std::endl;
            }
            else
            {
                std::cout << "JointDeltaDeg from default: " << deltaDeg << std::endl;
            }
            for (int8_t axis = 0; axis < 6; ++axis)
            {
                std::cout << "Axis " << static_cast<int>(axis) << " +" << deltaDeg << " deg" << std::endl;
                if (!moveJointRelativeDeg(pMot, 0, axis, +deltaDeg))
                {
                    std::cout << "move failed on axis " << static_cast<int>(axis) << " (+)" << std::endl;
                    break;
                }
                sleep(1);

                std::cout << "Axis " << static_cast<int>(axis) << " -" << deltaDeg << " deg" << std::endl;
                if (!moveJointRelativeDeg(pMot, 0, axis, -deltaDeg))
                {
                    std::cout << "move failed on axis " << static_cast<int>(axis) << " (-)" << std::endl;
                    break;
                }
                sleep(1);
            }
        }
        else
        {
            std::cout<<"使能失败"<<std::endl;
        }
    }

    //机器人使能
    setEn(pMot,false);

    //断连
    disconnectIPC(cmApi);
    return 0;
}

static bool tryClearEstopAndEnable(Hsc3::Comm::CommApi & cmApi, Hsc3::Proxy::ProxyMotion & pMot, Hsc3::Proxy::ProxySys & pSys, Hsc3::Proxy::ProxyIO & pIo)
{
    for (int attempt = 0; attempt < 3; ++attempt)
    {
        tryForceVirtualDin(pIo);
        (void)pSys.reset();

        (void)pMot.setEstop(false);
        (void)pMot.gpReset(0);
        usleep(300 * 1000);
        if (setEn(pMot, true))
        {
            return true;
        }
        usleep(300 * 1000);
    }
    return false;
}

static void tryForceVirtualDin(Hsc3::Proxy::ProxyIO & pIo)
{
    // 默认行为（为减少运行命令）：强制 DI0=1，保持 2000ms
    // 环境变量可覆盖：
    //   HSC3_FORCE_DI_DISABLE=1        完全禁用强制 DI
    //   HSC3_FORCE_DI_PORT=0
    //   HSC3_FORCE_DI_VALUE=1
    //   HSC3_FORCE_DI_OVERRIDE_REAL=1
    //   HSC3_FORCE_DI_HOLD_MS=2000
    if (getEnvBool("HSC3_FORCE_DI_DISABLE", false))
    {
        return;
    }

    const char * envPort = std::getenv("HSC3_FORCE_DI_PORT");
    const int32_t port = getEnvInt32("HSC3_FORCE_DI_PORT", 0);
    const bool value = getEnvBool("HSC3_FORCE_DI_VALUE", true);
    const bool allowOverride = getEnvBool("HSC3_FORCE_DI_OVERRIDE_REAL", true);
    const int32_t holdMs = getEnvInt32("HSC3_FORCE_DI_HOLD_MS", 2000);

    if (envPort && *envPort)
    {
        std::cout << "ForceDin port from env: " << envPort << std::endl;
    }
    else
    {
        std::cout << "ForceDin port from default: " << port << std::endl;
    }

    // 读取该端口当前是否为“虚拟输入”
    const int32_t grp = port / 32;
    const int32_t bit = port % 32;
    uint32_t maskGrp = 0;
    if (pIo.getDinMaskGrp(grp, maskGrp) == 0)
    {
        const bool isVirtual = ((maskGrp >> bit) & 0x1u) != 0;
        std::cout << "ForceDin port=" << port << ", maskGrp=" << static_cast<unsigned int>(maskGrp) << ", isVirtual=" << (isVirtual ? "true" : "false") << std::endl;

        // 默认不允许把真实输入改成虚拟输入（这等同于覆盖硬件输入，存在安全风险）
        if (!isVirtual && !allowOverride)
        {
            std::cout << "ForceDin skipped: DI" << port << " is real input. Set HSC3_FORCE_DI_OVERRIDE_REAL=1 to override (NOT RECOMMENDED)." << std::endl;
            return;
        }
    }

    // 把该端口设置为虚拟输入（若本身已是虚拟输入，这一步不会改变行为）
    const auto r1 = pIo.setDinMaskBit(port, true);
    std::cout << "setDinMaskBit(" << port << ",true) ret=" << static_cast<unsigned long long>(r1) << std::endl;

    // 写入并读回（必要时持续写入）
    const int32_t loops = (holdMs > 0) ? std::max<int32_t>(1, holdMs / 50) : 1;
    for (int32_t i = 0; i < loops; ++i)
    {
        const auto r2 = pIo.setDin(port, value);
        if (i == 0)
        {
            std::cout << "setDin(" << port << "," << (value ? "true" : "false") << ") ret=" << static_cast<unsigned long long>(r2) << std::endl;
        }

        bool readBack = false;
        const auto r3 = pIo.getDin(port, readBack);
        if (i == 0)
        {
            if (r3 == 0)
            {
                std::cout << "getDin(" << port << ") => " << (readBack ? "true" : "false") << std::endl;
            }
            else
            {
                std::cout << "getDin(" << port << ") failed, ret=" << static_cast<unsigned long long>(r3) << std::endl;
            }
        }

        if (holdMs > 0)
        {
            usleep(50 * 1000);
        }
    }
}

static bool getEnvBool(const char * name, bool defVal)
{
    const char * v = std::getenv(name);
    if (!v || !*v)
    {
        return defVal;
    }
    // 兼容: 1/0, true/false, yes/no
    if (v[0] == '1' || v[0] == 'y' || v[0] == 'Y' || v[0] == 't' || v[0] == 'T')
    {
        return true;
    }
    if (v[0] == '0' || v[0] == 'n' || v[0] == 'N' || v[0] == 'f' || v[0] == 'F')
    {
        return false;
    }
    return defVal;
}

static int32_t getEnvInt32(const char * name, int32_t defVal)
{
    const char * v = std::getenv(name);
    if (!v || !*v)
    {
        return defVal;
    }
    return static_cast<int32_t>(std::strtol(v, nullptr, 10));
}

static double getEnvDouble(const char * name, double defVal)
{
    const char * v = std::getenv(name);
    if (!v || !*v)
    {
        return defVal;
    }
    return std::strtod(v, nullptr);
}

static bool moveJointRelativeDeg(Hsc3::Proxy::ProxyMotion & pMot, int8_t gpId, int8_t axis, double deltaDeg)
{
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

    JntData target = cur;
    target[static_cast<size_t>(axis)] += deltaDeg;

    GeneralPos p;
    p.isJoint = true;
    p.ufNum = -1;
    p.utNum = -1;
    p.config = 0;
    p.vecPos = target;

    const auto r2 = pMot.moveTo(gpId, p, false);
    if (r2 != 0)
    {
        std::cout << "moveTo failed, ret=" << static_cast<unsigned long long>(r2) << std::endl;
        return false;
    }

    waitDone(pMot);
    return true;
}

/************************************************************************/
/*            
* @brief 连接IPC
* @param cmApi:通信客户端对象，建议在自定义的函数中如果需要传递客户端对象，都使用引用传递。
* @param strIP:IP，控制器默认IP是"10.10.56.214"
* @param uPort:端口号,固定端口号:23234
*/
/************************************************************************/
bool connectIPC(Hsc3::Comm::CommApi &cmApi,std::string strIP,uint16_t uPort)
{
    //1.设置非自动重连模式,连接前调用。
    cmApi.setAutoConn(false);
    //2.连接
    Hsc3::Comm::HMCErrCode ret=cmApi.connect(strIP,uPort);
    if (ret!=0)
    {
        std::cout << "CommApi::connect : ret = " << ret << std::endl;
        // printf("CommApi::connect :ret=%lld\n",ret);
    }
    //3.查询是否连接
    if(cmApi.isConnected())
    {
        std::cout<<"连接成功"<<std::endl;
        return true;
    }
    else
    {
        std::cout<<"连接失败"<<std::endl;
        return false;
    }
}

/************************************************************************/
/* 
* @brief 断开与IPC的连接
* @param cmApi:通信客户端对象
*/
/************************************************************************/
bool disconnectIPC(Hsc3::Comm::CommApi & cmApi)
{
    Hsc3::Comm::HMCErrCode ret=cmApi.disconnect();
    sleep(1);
    if (cmApi.isConnected())
    {
        return false;
    }
    else
    {
        return true;
    }
}

/************************************************************************/
/* 
* @brief 等待运动停止，只能用于手动模式下用于检测是否处于静止状态或错误状态。
* @param pMot:运动功能代理
*/
/************************************************************************/
void waitDone(Hsc3::Proxy::ProxyMotion & pMot)
{
    ManState manualState=MAN_STATE_MAX;
    while(1)
    {
        pMot.getManualStat(manualState);
        if (manualState == MAN_STATE_WAIT || manualState == MAN_STATE_ERROR)
        {
            break;
        }
    }
}

/************************************************************************/
/*      
* @brief 设置使能
* @param pMot:运动功能代理
* @param en:使能状态
*/
/************************************************************************/
bool setEn(Hsc3::Proxy::ProxyMotion & pMot, bool en)
{
    bool gpEn=false;
    //先获取使能状态，如果使能状态是要设置的状态，则无需再此设置。
    pMot.getGpEn(0,gpEn);
    if (gpEn == en)
    {
        return true;
    }
    else
    {
        //1.使能
        const auto r = pMot.setGpEn(0,en);
        std::cout << "setGpEn(" << (en ? "true" : "false") << ") ret=" << static_cast<unsigned long long>(r) << std::endl;
        sleep(2);
        pMot.getGpEn(0,gpEn);

        if (gpEn==en)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

int main()
{
    return run_training_jog();
}