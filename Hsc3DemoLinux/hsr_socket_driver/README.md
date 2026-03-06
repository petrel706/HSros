# hsr_socket_driver

把 `/home/wheeltec/Desktop/Hsc3DemoLinux` 中的纯 C++ 示例，移植为 ROS2 功能包，提供：

- `joint_states` 发布
- `follow_joint_trajectory` action（用于后续对接 MoveIt2/ros2_control，当前为“最小可用版”逐点 `moveTo`）
- 一组 demo 节点（CommApi/关节点动/回 HOME/关节点位运动/笛卡尔直线运动/系统报警监听）

## 1) 工作空间位置

本工作空间创建在：

- `/home/wheeltec/wheeltec_ros2`

包在：

- `/home/wheeltec/wheeltec_ros2/src/hsr_socket_driver`

## 2) Hsc3Api SDK 依赖

当前使用软链接方式引用你原来的 SDK：

- `hsr_socket_driver/third_party/Hsc3Api` -> `/home/wheeltec/Desktop/Hsc3DemoLinux/Hsc3Api`

如果你把 SDK 移动了，请重新建立软链接，或在编译时指定：

```bash
colcon build --cmake-args -DHSC3API_ROOT=/path/to/Hsc3Api
```

## 3) 编译

```bash
cd /home/wheeltec/wheeltec_ros2
source /opt/ros/humble/setup.bash
colcon build --symlink-install
source install/setup.bash
```

> 说明：本包会在 `install/hsr_socket_driver/lib/` 下安装 `libCommApi.so/libHsc3Api.so/libLogApi.so`，
> 可执行文件的 rpath 已设置为 `$ORIGIN/..`，一般不需要再手动设置 `LD_LIBRARY_PATH`。

## 4) 运行（driver）

```bash
source /home/wheeltec/wheeltec_ros2/install/setup.bash
ros2 launch hsr_socket_driver driver.launch.py
```

你也可以直接运行 node（不使用 launch）：

```bash
source /home/wheeltec/wheeltec_ros2/install/setup.bash
ros2 run hsr_socket_driver hsr_socket_driver_node --ros-args -p ip:=10.10.57.213 -p port:=23234
```

发布话题：

- `/joint_states` (`sensor_msgs/msg/JointState`)

Action：

- `/hsr_arm_controller/follow_joint_trajectory` (`control_msgs/action/FollowJointTrajectory`)

说明：

- `driver.launch.py` 默认把参数 `trajectory_action_name` 设置为 `hsr_arm_controller/follow_joint_trajectory`（便于 MoveIt2 controller 配置对接）。
- 如需修改 action 名称，请改 `driver.launch.py` 的 `trajectory_action_name`，并在 demo 里用 `-p action_name:=...` 保持一致。
- 如需“启动置 DI0=1，退出置 DI0=0”（用于满足使能条件），请只在 `driver.launch.py` 中配置 `force_virtual_din/reset_din_on_exit`，不要让 demo 去控制 DI。

## 5) 运行（demo）

```bash
ros2 run hsr_socket_driver demo_comm_api_training_node    >通信/SDK 基础连通性示例。
ros2 run hsr_socket_driver demo_training_jog_node         >点动（jog）示例（直连控制器版本，不负责 DI0 控制）
ros2 run hsr_socket_driver demo_training_jog_single_node  >单轴单次点动示例（通过 FollowJointTrajectory action）
ros2 run hsr_socket_driver demo_training_jog_home_node    >回 HOME（回零）示例（通过 FollowJointTrajectory action）
ros2 run hsr_socket_driver demo_training_move_joint_node  >关节空间运动示例（通过 FollowJointTrajectory action）
ros2 run hsr_socket_driver demo_cartesian_move_node --ros-args -p mode:=relative_z -p steps:=10 -p dz_mm:=-1.0  >笛卡尔空间运动示例（直线/相对位移
ros2 run hsr_socket_driver demo_sys_alarm_node          >读取系统报警/状态示例。
```

提示：

- 通过 action 的 demo 需要 driver 常驻运行（确保 action server 存在）。
- 如果你的 driver action 名称不是默认值，请显式指定：

```bash
ros2 run hsr_socket_driver demo_training_jog_home_node --ros-args -p action_name:=hsr_arm_controller/follow_joint_trajectory
```

