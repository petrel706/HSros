#!/bin/bash

# Co602 机械臂 MoveIt2 配置测试脚本

echo "=========================================="
echo "  Co602 MoveIt2 配置测试"
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
echo "正在加载工作空间..."
source install/setup.bash

echo ""
echo "=========================================="
echo "  检查包和文件"
echo "=========================================="
echo ""

# 检查 hsr_description 包
if ros2 pkg prefix hsr_description &> /dev/null; then
    echo "✓ hsr_description 包已安装"
    HSR_DESC_PATH=$(ros2 pkg prefix hsr_description)
    
    # 检查 URDF 文件
    if [ -f "$HSR_DESC_PATH/share/hsr_description/urdf/co602.urdf" ]; then
        echo "  ✓ URDF 文件存在"
    else
        echo "  ✗ URDF 文件不存在"
    fi
    
    # 检查 mesh 文件
    if [ -d "$HSR_DESC_PATH/share/hsr_description/meshes/co602_490" ]; then
        MESH_COUNT=$(ls -1 "$HSR_DESC_PATH/share/hsr_description/meshes/co602_490"/*.STL 2>/dev/null | wc -l)
        echo "  ✓ Mesh 目录存在 ($MESH_COUNT 个文件)"
    else
        echo "  ✗ Mesh 目录不存在"
    fi
else
    echo "✗ hsr_description 包未找到"
fi

echo ""

# 检查 hsr_moveit_config 包
if ros2 pkg prefix hsr_moveit_config &> /dev/null; then
    echo "✓ hsr_moveit_config 包已安装"
    HSR_MOVEIT_PATH=$(ros2 pkg prefix hsr_moveit_config)
    
    # 检查配置文件
    if [ -f "$HSR_MOVEIT_PATH/share/hsr_moveit_config/config/co602.srdf" ]; then
        echo "  ✓ SRDF 文件存在"
    else
        echo "  ✗ SRDF 文件不存在"
    fi
    
    if [ -f "$HSR_MOVEIT_PATH/share/hsr_moveit_config/config/kinematics.yaml" ]; then
        echo "  ✓ Kinematics 配置存在"
    else
        echo "  ✗ Kinematics 配置不存在"
    fi
    
    if [ -f "$HSR_MOVEIT_PATH/share/hsr_moveit_config/config/moveit_controllers.yaml" ]; then
        echo "  ✓ Controllers 配置存在"
    else
        echo "  ✗ Controllers 配置不存在"
    fi
    
    # 检查启动文件
    if [ -f "$HSR_MOVEIT_PATH/share/hsr_moveit_config/launch/demo.launch.py" ]; then
        echo "  ✓ demo.launch.py 存在"
    else
        echo "  ✗ demo.launch.py 不存在"
    fi
else
    echo "✗ hsr_moveit_config 包未找到"
fi

echo ""
echo "=========================================="
echo "  测试完成"
echo "=========================================="
echo ""
echo "如果所有检查都通过，可以运行："
echo "  ros2 launch hsr_moveit_config demo.launch.py"
echo ""
echo "详细使用说明请查看: MOVEIT_SETUP_GUIDE.md"
echo ""
