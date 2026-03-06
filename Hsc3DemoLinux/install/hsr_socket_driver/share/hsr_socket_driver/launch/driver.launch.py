from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription(
        [
            Node(
                package="hsr_socket_driver",
                executable="hsr_socket_driver_node",
                name="hsr_socket_driver",
                output="screen",
                parameters=[{
                    "use_sim_time": False,
                    "ip": "10.10.57.213",
                    "port": 23234,
                    "gp_id": 0,
                    "op_mode": "EXT",
                    "enable_on_start": True,
                    # 需要“DI0=1 才能上使能”的场景：启动时置 1，退出时置回 0
                    "force_virtual_din": True,
                    "force_din_port": 0,
                    "force_din_value": True,
                    "force_din_hold_ms": 2000,
                    # 注意：若 DI0 是“真实 DI”，需要允许覆盖才能写入（有安全风险，请确保现场允许）
                    "allow_override_real_din": True,
                    "reset_din_on_exit": True,
                    # 给 MoveIt2 对接：MoveIt 默认期望 /<controller_name>/follow_joint_trajectory
                    "trajectory_action_name": "hsr_arm_controller/follow_joint_trajectory",
                    "publish_rate_hz": 50.0,
                    "joint_names": ["joint_1", "joint_2", "joint_3", "joint_4", "joint_5", "joint_6"],
                }],
            ),
        ]
    )

