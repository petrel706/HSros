from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory

import os


def generate_launch_description():
    desc_share = get_package_share_directory("wheeltec_robot_urdf")
    cfg_share = get_package_share_directory("hsr_moveit_config")

    urdf_path = os.path.join(desc_share, "urdf", "S100_diff_robot.urdf")
    with open(urdf_path, "r", encoding="utf-8") as f:
        robot_description = f.read()

    srdf_path = os.path.join(cfg_share, "config", "hsr_arm.srdf")
    with open(srdf_path, "r", encoding="utf-8") as f:
        robot_description_semantic = f.read()

    kinematics_yaml = os.path.join(cfg_share, "config", "kinematics.yaml")
    joint_limits_yaml = os.path.join(cfg_share, "config", "joint_limits.yaml")
    ompl_yaml = os.path.join(cfg_share, "config", "ompl_planning.yaml")
    controllers_yaml = os.path.join(cfg_share, "config", "moveit_controllers.yaml")

    robot_state_publisher = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        name="robot_state_publisher",
        output="screen",
        parameters=[{"robot_description": robot_description}, {"use_sim_time": False}],
        # 真实关节角来自 hsr_socket_driver_node 发布的 /joint_states
    )

    move_group = Node(
        package="moveit_ros_move_group",
        executable="move_group",
        output="screen",
        parameters=[
            {"robot_description": robot_description},
            {"robot_description_semantic": robot_description_semantic},
            {"use_sim_time": False},
            kinematics_yaml,
            joint_limits_yaml,
            ompl_yaml,
            controllers_yaml,
            {
                "planning_plugin": "ompl_interface/OMPLPlanner",
                "planning_scene_monitor.publish_planning_scene": True,
                "planning_scene_monitor.publish_geometry_updates": True,
                "planning_scene_monitor.publish_state_updates": True,
                "planning_scene_monitor.publish_transforms_updates": True,
                "moveit_manage_controllers": True,
                "trajectory_execution.allowed_execution_duration_scaling": 1.2,
                "trajectory_execution.allowed_goal_duration_margin": 0.5,
                "trajectory_execution.allowed_start_tolerance": 0.01,
            },
        ],
    )

    rviz_config = os.path.join(cfg_share, "rviz", "moveit.rviz")
    rviz = Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2",
        output="screen",
        arguments=["-d", rviz_config],
        parameters=[
            {"robot_description": robot_description},
            {"robot_description_semantic": robot_description_semantic},
            {"use_sim_time": False},
        ],
    )

    return LaunchDescription([robot_state_publisher, move_group, rviz])

