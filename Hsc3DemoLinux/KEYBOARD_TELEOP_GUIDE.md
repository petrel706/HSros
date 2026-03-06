# 键盘控制机械臂使用指南

## 🎮 keyboard_teleop_joint_node 使用方法

这个节点允许你通过键盘实时控制机械臂的关节运动。

## 🚀 启动步骤

### 步骤 1: 启动机械臂驱动

在**终端 1** 中启动驱动：

```bash
cd ~/co_605_arm/Hsc3DemoLinux
./start_robot_driver.sh
```

等待看到：
```
[INFO] [hsr_socket_driver]: 已连接到控制器 10.10.57.213:23234
```

### 步骤 2: 启动键盘控制节点

在**终端 2** 中启动键盘控制：

```bash
cd ~/co_605_arm/Hsc3DemoLinux
source install/setup.bash
ros2 run hsr_socket_driver keyboard_teleop_joint_node
```

## ⌨️ 键盘控制说明

启动后，你可以使用以下按键控制机械臂：

### 关节选择
- **1-6**: 选择要控制的关节（joint_1 到 joint_6）

### 运动控制
- **+** 或 **=**: 正向运动（增加关节角度）
- **-**: 负向运动（减小关节角度）
- **空格**: 停止当前运动

### 速度调节
- **[**: 降低运动速度
- **]**: 提高运动速度

### 其他
- **h**: 显示帮助信息
- **q** 或 **ESC**: 退出程序

## 📝 使用示例

1. **控制 joint_1 正向运动**：
   - 按 `1` 选择 joint_1
   - 按 `+` 开始正向运动
   - 按 `空格` 停止

2. **控制 joint_3 负向运动**：
   - 按 `3` 选择 joint_3
   - 按 `-` 开始负向运动
   - 按 `空格` 停止

3. **调整速度**：
   - 按 `[` 降低速度
   - 按 `]` 提高速度

## ⚠️ 安全注意事项

1. **首次使用时请小心**：
   - 先用低速测试
   - 确保机械臂周围无障碍物
   - 随时准备按空格键停止

2. **运动范围**：
   - 注意关节限位
   - 避免奇异点配置
   - 如遇异常立即按空格停止

3. **紧急停止**：
   - 键盘：按空格键
   - 硬件：按示教器急停按钮

## 🔧 工作原理

`keyboard_teleop_joint_node` 的工作流程：

1. **订阅关节状态**：从 `/joint_states` 获取当前位置
2. **接收键盘输入**：读取用户按键
3. **发送轨迹命令**：通过 `/hsr_arm_controller/follow_joint_trajectory` 动作控制机械臂

## 📊 完整启动流程

```bash
# 终端 1: 启动驱动
cd ~/co_605_arm/Hsc3DemoLinux
./start_robot_driver.sh

# 终端 2: 启动键盘控制
cd ~/co_605_arm/Hsc3DemoLinux
source install/setup.bash
ros2 run hsr_socket_driver keyboard_teleop_joint_node

# 终端 3 (可选): 监控关节状态
cd ~/co_605_arm/Hsc3DemoLinux
source install/setup.bash
ros2 topic echo /joint_states
```

## 🐛 故障排查

### 问题 1: "Package not found"

**原因**: 包未编译或环境未 source

**解决**:
```bash
cd ~/co_605_arm/Hsc3DemoLinux
source /opt/ros/humble/setup.bash
colcon build --packages-select hsr_socket_driver
source install/setup.bash
```

### 问题 2: 按键无响应

**检查**:
1. 确保终端窗口处于活动状态（有焦点）
2. 确认驱动节点正在运行
3. 检查是否有错误日志输出

### 问题 3: 机械臂不动

**检查**:
1. 驱动是否成功连接（终端 1 应显示"已连接到控制器"）
2. 机械臂是否已使能
3. 查看驱动日志是否有错误

## 📚 相关文档

- **驱动使用**: `README_ROS2.md`
- **启动脚本**: `start_robot_driver.sh`
- **源代码**: `hsr_socket_driver/src/keyboard_teleop_joint_node.cpp`

---

**版本**: 1.0  
**日期**: 2026-03-03  
**状态**: ✅ 已验证可用
