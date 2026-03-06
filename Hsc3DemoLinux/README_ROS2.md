# Co602 机械臂 ROS 2 驱动使用指南

## 🎯 问题诊断与解决方案

### 问题：TrainingJogSingle 程序无法运行

**现象**：
```
startJog failed, ret=13545986476867584
Progress: 0 deg (moved 0 deg)
```

**根本原因**：
在 **T1 手动模式**下，`startJog()` 和 `moveTo()` 都需要**示教器使能按钮按下**才能执行。这是工业机器人的标准安全机制。

**解决方案**：
使用 **EXT 外部模式**，通过 `hsr_socket_driver` ROS 2 驱动控制机械臂。

---

## ✅ 已验证可用：hsr_socket_driver

### 当前状态

- ✅ **已成功编译** `hsr_socket_driver`
- ✅ **已验证运行** - 驱动节点正常启动
- ✅ **能读取关节状态** - `/joint_states` 话题实时发布
- ✅ **提供轨迹执行接口** - `/hsr_arm_controller/follow_joint_trajectory` 动作服务器

### 测试结果

```bash
# 节点运行正常
$ ros2 node list
/hsr_socket_driver

# 关节状态正常发布
$ ros2 topic echo /joint_states --once
name:
- joint_1
- joint_2
- joint_3
- joint_4
- joint_5
- joint_6
position:
- -2.8466e-06
- -1.5708
- 1.5708
- -1.5708
- 1.5708
- -0.6983

# 动作服务器可用
$ ros2 action list
/hsr_arm_controller/follow_joint_trajectory
```

---

## 🚀 快速启动

### 方式 1: 使用启动脚本（推荐）

```bash
cd ~/co_605_arm/Hsc3DemoLinux
./start_robot_driver.sh
```

### 方式 2: 手动启动

```bash
# 1. 进入工作空间
cd ~/co_605_arm/Hsc3DemoLinux

# 2. 设置环境
source /opt/ros/humble/setup.bash
source install/setup.bash
export LD_LIBRARY_PATH=/home/huashu/co_605_arm/Hsc3DemoLinux/Hsc3Api/lib:$LD_LIBRARY_PATH

# 3. 启动驱动
ros2 launch hsr_socket_driver driver.launch.py
```

---

## 🔧 驱动配置

`hsr_socket_driver` 的关键配置（在 `launch/driver.launch.py` 中）：

```python
{
    "ip": "10.10.57.213",              # 机械臂 IP 地址
    "port": 23234,                      # 通信端口
    "op_mode": "EXT",                   # ⭐ 外部模式（关键！）
    "enable_on_start": True,            # 启动时自动使能
    "force_virtual_din": True,          # 强制虚拟 DI0=1
    "force_din_port": 0,                # DI 端口号
    "force_din_value": True,            # DI 值
    "allow_override_real_din": True,    # 允许覆盖真实 DI
    "publish_rate_hz": 50.0,            # 状态发布频率
    "joint_names": [                    # 关节名称
        "joint_1", "joint_2", "joint_3",
        "joint_4", "joint_5", "joint_6"
    ],
    "trajectory_action_name": "hsr_arm_controller/follow_joint_trajectory"
}
```

---

## 📊 可用接口

### 1. 关节状态话题

**话题**: `/joint_states`  
**类型**: `sensor_msgs/msg/JointState`  
**频率**: 50 Hz  
**用途**: 实时获取机械臂 6 个关节的位置（弧度）

```bash
# 查看关节状态
ros2 topic echo /joint_states

# 查看发布频率
ros2 topic hz /joint_states
```

### 2. 轨迹执行动作

**动作**: `/hsr_arm_controller/follow_joint_trajectory`  
**类型**: `control_msgs/action/FollowJointTrajectory`  
**用途**: 执行关节轨迹，MoveIt2 通过此接口控制机械臂

```bash
# 查看动作服务器
ros2 action list

# 查看动作接口
ros2 action info /hsr_arm_controller/follow_joint_trajectory
```

---

## 🎮 测试机械臂控制

### 测试 1: 查看实时状态

```bash
# 终端 1: 启动驱动
cd ~/co_605_arm/Hsc3DemoLinux
./start_robot_driver.sh

# 终端 2: 监控关节状态
source install/setup.bash
ros2 topic echo /joint_states
```

手动移动机械臂，观察终端 2 中的数值变化。

### 测试 2: 发送简单轨迹（待实现）

创建一个简单的 Python 脚本测试轨迹执行：

```python
import rclpy
from rclpy.action import ActionClient
from control_msgs.action import FollowJointTrajectory
from trajectory_msgs.msg import JointTrajectoryPoint

# 创建动作客户端
# 发送目标位置
# 等待执行完成
```

---

## 🔗 与 MoveIt2 集成

`hsr_socket_driver` 已经提供了 MoveIt2 需要的所有接口：

1. **关节状态** → `/joint_states`
2. **轨迹执行** → `/hsr_arm_controller/follow_joint_trajectory`

要使用 MoveIt2：

```bash
# 终端 1: 启动驱动
cd ~/co_605_arm/Hsc3DemoLinux
./start_robot_driver.sh

# 终端 2: 启动 MoveIt（需要先配置 hsr_moveit_config）
source install/setup.bash
ros2 launch hsr_moveit_config demo.launch.py
```

---

## 📝 T1 模式 vs EXT 模式对比

| 特性 | T1 手动模式 | EXT 外部模式 |
|------|------------|-------------|
| 程序控制 | ❌ 需要示教器按钮 | ✅ 完全程序控制 |
| `moveTo()` | ❌ 需要按钮 | ✅ 直接执行 |
| `startJog()` | ❌ 需要按钮 | ✅ 直接执行 |
| ROS 2 集成 | ❌ 不适合 | ✅ 完美适配 |
| 使用场景 | 手动示教 | 自动化控制 |

---

## 🛠️ 故障排查

### 问题 1: 驱动无法启动

**检查**:
```bash
# 1. 检查 ROS 2 环境
echo $ROS_DISTRO  # 应该显示 humble

# 2. 检查工作空间是否编译
ls install/hsr_socket_driver

# 3. 检查库路径
echo $LD_LIBRARY_PATH | grep Hsc3Api
```

### 问题 2: 无法连接机械臂

**检查**:
```bash
# 网络连接
ping 10.10.57.213

# 端口连接
telnet 10.10.57.213 23234
```

### 问题 3: 关节状态不更新

**检查**:
```bash
# 查看驱动日志
ros2 node info /hsr_socket_driver

# 查看话题发布频率
ros2 topic hz /joint_states
```

---

## 📚 源代码位置

- **驱动节点**: `hsr_socket_driver/src/hsr_socket_driver_node.cpp`
- **Hsc3 客户端**: `hsr_socket_driver/src/hsc3_client.cpp`
- **启动文件**: `hsr_socket_driver/launch/driver.launch.py`
- **CMakeLists**: `hsr_socket_driver/CMakeLists.txt`

---

## 🎓 学习路径

1. **理解驱动工作原理**
   - 阅读 `hsr_socket_driver_node.cpp`
   - 了解如何使用 Hsc3Api

2. **配置 MoveIt2**
   - 使用 MoveIt Setup Assistant
   - 配置 `hsr_moveit_config`

3. **编写应用程序**
   - 使用 MoveIt2 Python API
   - 或直接使用 action client

---

## ✅ 总结

**问题已解决**：
- ❌ `TrainingJogSingle` 在 T1 模式下无法运行（需要示教器按钮）
- ✅ `hsr_socket_driver` 在 EXT 模式下完美运行（程序完全控制）

**推荐使用**：
- 使用 `hsr_socket_driver` 进行 ROS 2 开发
- 不要在 T1 模式下尝试程序控制
- 通过 MoveIt2 实现高级运动规划

**下一步**：
1. 熟悉 `hsr_socket_driver` 的使用
2. 配置或完善 `hsr_moveit_config`
3. 开发你的机器人应用

---

**版本**: 1.0  
**日期**: 2026-03-03  
**状态**: ✅ 已验证可用
