import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch.conditions import IfCondition, UnlessCondition
from ament_index_python.packages import get_package_share_directory
from moveit_configs_utils import MoveItConfigsBuilder


def generate_launch_description():
    # Get package directories
    moveit_config_pkg = get_package_share_directory('hsr_moveit_config')
    
    # Build MoveIt configuration
    moveit_config = (
        MoveItConfigsBuilder("arm", package_name="hsr_moveit_config")
        .robot_description(file_path=os.path.join(
            get_package_share_directory('hsr_description'),
            'urdf',
            'co602.urdf'
        ))
        .robot_description_semantic(file_path=os.path.join(
            moveit_config_pkg,
            'config',
            'co602.srdf'
        ))
        .trajectory_execution(file_path=os.path.join(
            moveit_config_pkg,
            'config',
            'moveit_controllers.yaml'
        ))
        .planning_pipelines(
            pipelines=["ompl"]
        )
        .to_moveit_configs()
    )
    
    # Declare arguments
    use_sim_time = LaunchConfiguration('use_sim_time', default='false')
    
    # Start robot state publisher
    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        output='screen',
        parameters=[
            moveit_config.robot_description,
            {'use_sim_time': use_sim_time}
        ]
    )
    
    # Start MoveGroup node
    move_group_node = Node(
        package='moveit_ros_move_group',
        executable='move_group',
        output='screen',
        parameters=[
            moveit_config.to_dict(),
            {'use_sim_time': use_sim_time}
        ]
    )
    
    # RViz
    rviz_config_file = os.path.join(
        moveit_config_pkg,
        'config',
        'moveit.rviz'
    )
    
    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='log',
        arguments=['-d', rviz_config_file] if os.path.exists(rviz_config_file) else [],
        parameters=[
            moveit_config.robot_description,
            moveit_config.robot_description_semantic,
            moveit_config.robot_description_kinematics,
            moveit_config.planning_pipelines,
            {'use_sim_time': use_sim_time}
        ]
    )
    
    return LaunchDescription([
        DeclareLaunchArgument(
            'use_sim_time',
            default_value='false',
            description='Use simulation (Gazebo) clock if true'
        ),
        robot_state_publisher,
        move_group_node,
        rviz_node
    ])
