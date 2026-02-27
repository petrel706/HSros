/************************************************************************/
/* 
* @brief 演示如何通过接口与IPC建立通信连接。并获取到系统版本信息，和调用基本的执行命令接口。
*/
/************************************************************************/
#include <iostream>
#include <unistd.h>
#include <vector>
#include "CommApi.h"
#include "proxy/ProxyVar.h"

static int run_comm_api_training()
{
    //1.构造通讯客户端
    Hsc3::Comm::CommApi cmApi("./log/test");
    //2.设置非自动重连模式，此接口应该在连接前调用。
    cmApi.setAutoConn(false);
    //3.连接，控制器默认连接IP是"10.10.56.214"  - (std::string)。端口号是固定值：23234 -（int）
    Hsc3::Comm::HMCErrCode ret=cmApi.connect("10.10.57.213",23234);
    if (ret!=0)
    {
        printf("CommApi::connect :ret=%lld\n",ret);
    }
    //4.查询是否连接,连接后通过此接口确认连接状态。
    if(cmApi.isConnected())
    {
        std::cout<<"连接成功"<<std::endl;
        std::cout<<"版本信息:"<<cmApi.getVersionStr()<<std::endl;  //获取版本信息。
        (void)cmApi.disconnect();
        return 0;
    }
    else
    {
        std::cout<<"连接不成功"<<std::endl;
        return 1;
    }
}

int main()
{
    return run_comm_api_training();
}
