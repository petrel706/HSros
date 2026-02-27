使用前请先阅读：
（1）使用时需要添加 _LINUX_ 宏
（2）Hsc3Api.chm为接口文档，有详细接口以及参数介绍,打开后，左边栏中可分别选择Hsc3 Comm Proxy等命名空间，即可看到包含的类成员。
（3）《通用接口命令手册_A02_CN.pdf》中的命令可以通过CommApi::execCmd接口进行调用，如:
	CommApi comm("./log"); //在当前目录下生成接口日志
	uint64_t ret = comm.connect("10.10.56.214", 23234)；//返回为0则表示连接成功
	std::string respone{};
	ret = execCmd("mot.getGpEn(0)", respone, 3); //获取系统使能状态，respone用于接受调用命令的返回值，3表示命令执行优先级，默认为3就行

