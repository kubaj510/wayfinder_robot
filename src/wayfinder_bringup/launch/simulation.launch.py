import os
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import xacro

def generate_launch_description():
    xacro_path = os.path.join(
        get_package_share_directory('wayfinder_description'), 'urdf', 'wayfinder.urdf.xacro'
    )
    robot_description = xacro.process_file(xacro_path).toxml()

    world_path = os.path.join(
        get_package_share_directory('wayfinder_gazebo'), 'worlds', 'test_world.sdf'
    )

    gz_sim = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(get_package_share_directory('ros_gz_sim'), 'launch', 'gz_sim.launch.py')
        ),
        launch_arguments={'gz_args': f'-r {world_path}'}.items()
    )

    return LaunchDescription([
        gz_sim,
        
        Node(
            package='robot_state_publisher', 
            executable='robot_state_publisher',
            parameters=[{'robot_description': robot_description, 'use_sim_time': True}]
        ),
        
        Node(
            package='ros_gz_sim', 
            executable='create',
            arguments=['-topic', 'robot_description', '-name', 'wayfinder', '-z', '0.5'],
            output='screen'
        ),

        Node(
            package='ros_gz_bridge',
            executable='parameter_bridge',
            parameters=[{'config_file': os.path.join(
                get_package_share_directory('wayfinder_gazebo'), 'config', 'gz_bridge.yaml'
            )}],
            output='screen'
        ),

        Node(
            package='controller_manager', 
            executable='spawner', 
            arguments=['joint_state_broadcaster']
        ),

        Node(
            package='controller_manager', 
            executable='spawner', 
            arguments=['diff_drive_controller']
        ),

        Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            output='screen',
            parameters=[{'use_sim_time': True}]
        ),
    ])
