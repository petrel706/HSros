# Co602 机械臂 MoveIt2 配置指南

## ✅ 问题已解决

### 原始问题
```bash
$ ros2 launch hsr_moveit_config demo.launch.py
file 'demo.launch.py' was not found
```

**原因**: `hsr_moveit_config` 包是空的，没有配置文件和启动文件。

### 解决方案
已创建完整的 MoveIt2 配置：
- ✅ 创建 `hsr_description` 包（包含 URDF 和 meshes）
- ✅ 修复 URDF 中的 mesh 路径
- ✅ 创建 `co602.srdf` 语义描述文件
- ✅ 创建 `demo.launch.py` 启动文件
- ✅ 成功编译所有包

---

## 🚀 快速启动

### 方式 1: 仿真模式（不连接实体机械臂）

```bash
cd ~/co_605_arm/Hsc3DemoLinux
source install/setup.bash
ros2 launch hsr_moveit_config demo.launch.py
```

这将启动：
- Robot State Publisher（发布机器人模型）
- MoveIt Move Group（运动规划节点）
- RViz2（可视化界面）

### 方式 2: 实体机械臂控制

**终端 1 - 启动驱动**：
```bash
cd ~/co_605_arm/Hsc3DemoLinux
./start_robot_driver.sh
```

**终端 2 - 启动 MoveIt2**：
```bash
cd ~/co_605_arm/Hsc3DemoLinux
source install/setup.bash
ros2 launch hsr_moveit_config demo.launch.py
```

---

## 📦 包结构

### 1. hsr_description
机器人描述包，包含：
- **URDF 文件**: `urdf/co602.urdf`
- **Mesh 文件**: `meshes/co602_490/*.STL`

### 2. hsr_moveit_config
MoveIt2 配置包，包含：
- **SRDF**: `config/co602.srdf` - 语义描述
- **Kinematics**: `config/kinematics.yaml` - 运动学配置
- **Controllers**: `config/moveit_controllers.yaml` - 控制器配置
- **Joint Limits**: `config/joint_limits.yaml` - 关节限位
- **Launch**: `launch/demo.launch.py` - 启动文件

---

## 🎯 机器人配置详情

### 关节定义
- **joint_1**: 基座旋转
- **joint_2**: 肩关节
- **joint_3**: 肘关节
- **joint_4**: 腕关节1
- **joint_5**: 腕关节2
- **joint_6**: 腕关节3（末端旋转）

### 规划组
- **arm**: 包含所有 6 个关节（base_link → link6）

### 预定义姿态
- **home**: 机械臂 Home 位置
  ```
  joint_1: 0°
  joint_2: -90°
  joint_3: 90°
  joint_4: 0°
  joint_5: 90°
  joint_6: 0°
  ```

- **ready**: 准备位置
  ```
  joint_1: 0°
  joint_2: -57.3°
  joint_3: 57.3°
  joint_4: 0°
  joint_5: 57.3°
  joint_6: 0°
  ```

---

## 🎮 使用 MoveIt2 规划运动

### 在 RViz 中操作

1. **启动 demo.launch.py** 后，RViz 窗口会打开

2. **添加 MotionPlanning 插件**（如果没有）：
   - 点击 "Add" → "By display type" → "MotionPlanning"

3. **规划运动**：
   - 在 MotionPlanning 面板中，拖动交互式标记
   - 或在 "Planning" 标签中选择目标姿态（如 "home"）
   - 点击 "Plan" 按钮生成轨迹
   - 点击 "Execute" 执行轨迹

### 使用 Python API

```python
import rclpy
from moveit_py import MoveItPy

rclpy.init()
moveit = MoveItPy(node_name="moveit_py_demo")

# 获取规划组
arm = moveit.get_planning_component("arm")

# 设置目标姿态
arm.set_goal_state(configuration_name="home")

# 规划
plan_result = arm.plan()

# 执行
if plan_result:
    robot = moveit.get_robot_model()
    robot.execute(plan_result.trajectory)
```

---

## 🔧 与实体机械臂集成

### 完整启动流程

```bash
# 终端 1: 启动驱动
cd ~/co_605_arm/Hsc3DemoLinux
./start_robot_driver.sh

# 终端 2: 启动 MoveIt2
cd ~/co_605_arm/Hsc3DemoLinux
source install/setup.bash
ros2 launch hsr_moveit_config demo.launch.py

# 终端 3 (可选): 监控关节状态
ros2 topic echo /joint_states
```

### 数据流

```
RViz2 (用户交互)
    ↓
MoveIt Move Group (运动规划)
    ↓
/hsr_arm_controller/follow_joint_trajectory (轨迹命令)
    ↓
hsr_socket_driver (驱动)
    ↓
实体 Co602 机械臂
```

---

## 📝 配置文件说明

### co602.srdf
定义了：
- 规划组（arm）
- 末端执行器（gripper）
- 禁用碰撞对
- 预定义姿态（home, ready）

### kinematics.yaml
配置运动学求解器：
- 使用 KDL 插件
- 搜索分辨率: 0.005
- 超时时间: 0.1s

### moveit_controllers.yaml
定义控制器接口：
- 控制器名称: `hsr_arm_controller`
- 动作接口: `/hsr_arm_controller/follow_joint_trajectory`
- 关节列表: joint_1 到 joint_6

---

## 🛠️ 故障排查

### 问题 1: demo.launch.py 找不到

**检查编译**:
```bash
cd ~/co_605_arm/Hsc3DemoLinux
source /opt/ros/humble/setup.bash
colcon build --packages-select hsr_description hsr_moveit_config
source install/setup.bash
```

### 问题 2: RViz 中看不到机器人模型

**检查**:
1. URDF 文件路径是否正确
2. Mesh 文件是否存在
3. RViz 中 "RobotModel" 显示是否启用

### 问题 3: 规划失败

**可能原因**:
1. 目标位置超出关节限位
2. 目标位置在奇异点附近
3. 碰撞检测失败

**解决**:
- 检查关节限位配置
- 尝试不同的目标位置
- 调整碰撞检测参数

### 问题 4: 无法执行轨迹（实体机械臂）

**检查**:
1. 驱动是否正常运行
2. 动作服务器是否可用
   ```bash
   ros2 action list
   # 应该看到: /hsr_arm_controller/follow_joint_trajectory
   ```
3. 查看驱动日志是否有错误

---

## 📚 相关文档

- **驱动使用**: `README_ROS2.md`
- **键盘控制**: `KEYBOARD_TELEOP_GUIDE.md`
- **启动脚本**: `start_robot_driver.sh`

---

## 🎓 下一步

1. **熟悉 MoveIt2 界面**
   - 在仿真模式下练习规划和执行

2. **测试实体机械臂**
   - 先用简单的目标位置测试
   - 确保安全距离

3. **开发应用程序**
   - 使用 MoveIt2 Python/C++ API
   - 集成视觉、抓取等功能

4. **高级配置**
   - 使用 MoveIt Setup Assistant 微调配置
   - 添加自定义约束和规划器参数

---

**版本**: 1.0  
**日期**: 2026-03-03  
**状态**: ✅ 已验证可用
