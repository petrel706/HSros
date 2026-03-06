#!/bin/bash

# Co602 机械臂 ROS 2 驱动启动脚本
# 使用 hsr_socket_driver 连接实体机械臂

echo "=========================================="
echo "  Co602 机械臂 ROS 2 驱动启动"
echo "=========================================="
echo ""

# 检查是否在正确的目录
if [ ! -f "install/setup.bash" ]; then
    echo "错误: 请在 Hsc3DemoLinux 目录下运行此脚本"
    echo "当前目录: $(pwd)"
    exit 1
fi

# 检查 ROS 2 环境
if [ -z "$ROS_DISTRO" ]; then
    echo "正在加载 ROS 2 Humble 环境..."
    source /opt/ros/humble/setup.bash
fi

echo "ROS 发行版: $ROS_DISTRO"
echo ""

# Source 工作空间
echo "正在加载工作空间..."
source install/setup.bash

# 设置 Hsc3Api 库路径
export LD_LIBRARY_PATH=/home/huashu/co_605_arm/Hsc3DemoLinux/Hsc3Api/lib:$LD_LIBRARY_PATH
echo "已设置 Hsc3Api 库路径"
echo ""

# 检查机械臂连接
echo "检查机械臂连接 (10.10.57.213:23234)..."
if ping -c 1 -W 2 10.10.57.213 &> /dev/null; then
    echo "✓ 机械臂网络连接正常"
else
    echo "✗ 警告: 无法 ping 通机械臂 IP"
    echo "  请确保机械臂已开机并连接到网络"
    read -p "是否继续? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

echo ""
echo "=========================================="
echo "  启动 hsr_socket_driver"
echo "=========================================="
echo ""
echo "驱动配置:"
echo "  - IP: 10.10.57.213"
echo "  - Port: 23234"
echo "  - 模式: EXT (外部模式)"
echo "  - 自动使能: 是"
echo "  - 发布频率: 50 Hz"
echo ""
echo "启动后可用接口:"
echo "  - 话题: /joint_states"
echo "  - 动作: /hsr_arm_controller/follow_joint_trajectory"
echo ""
echo "按 Ctrl+C 停止驱动"
echo ""

# 启动驱动
ros2 launch hsr_socket_driver driver.launch.py
