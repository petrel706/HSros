#!/bin/bash

# Co602 机械臂键盘控制启动脚本
# 此脚本会在两个终端窗口中分别启动驱动和键盘控制节点

echo "=========================================="
echo "  Co602 机械臂键盘控制启动"
echo "=========================================="
echo ""

# 检查是否在正确的目录
if [ ! -f "install/setup.bash" ]; then
    echo "错误: 请在 Hsc3DemoLinux 目录下运行此脚本"
    exit 1
fi

# 检查 ROS 2 环境
if [ -z "$ROS_DISTRO" ]; then
    echo "正在加载 ROS 2 Humble 环境..."
    source /opt/ros/humble/setup.bash
fi

# Source 工作空间
source install/setup.bash

# 设置库路径
export LD_LIBRARY_PATH=/home/huashu/co_605_arm/Hsc3DemoLinux/Hsc3Api/lib:$LD_LIBRARY_PATH

echo "准备启动两个节点："
echo "  1. hsr_socket_driver (驱动)"
echo "  2. keyboard_teleop_joint_node (键盘控制)"
echo ""
echo "请在新打开的终端窗口中使用键盘控制机械臂"
echo ""
echo "键盘控制说明："
echo "  - 1-6: 选择关节"
echo "  - +/-: 正向/负向运动"
echo "  - 空格: 停止"
echo "  - q: 退出"
echo ""
echo "详细说明请查看: KEYBOARD_TELEOP_GUIDE.md"
echo ""

# 在新终端中启动键盘控制节点
gnome-terminal -- bash -c "
    cd ~/co_605_arm/Hsc3DemoLinux
    source /opt/ros/humble/setup.bash
    source install/setup.bash
    echo '=========================================='
    echo '  键盘控制节点'
    echo '=========================================='
    echo ''
    echo '键盘控制说明：'
    echo '  - 1-6: 选择关节'
    echo '  - +/-: 正向/负向运动'
    echo '  - 空格: 停止'
    echo '  - [/]: 降低/提高速度'
    echo '  - h: 帮助'
    echo '  - q: 退出'
    echo ''
    echo '等待驱动启动...'
    sleep 3
    ros2 run hsr_socket_driver keyboard_teleop_joint_node
    exec bash
" &

# 在当前终端启动驱动
echo "在当前终端启动驱动..."
echo ""
ros2 launch hsr_socket_driver driver.launch.py
