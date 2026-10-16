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

    rviz_config_path = os.path.join(
        get_package_share_directory('wayfinder_description'), 'rviz', 'wayfinder_view.rviz'
    )

    gz_sim = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(get_package_share_directory('ros_gz_sim'), 'launch', 'gz_sim.launch.py')
        ),
        launch_arguments={'gz_args': f'-r {world_path}'}.items()
    )

    nav2_bringup = IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                os.path.join(get_package_share_directory('nav2_bringup'), 'launch', 'bringup_launch.py')
            ),
            launch_arguments={
                'use_sim_time': 'true',
                'map': os.path.join(get_package_share_directory('wayfinder_navigation'), 'maps', 'checkpoint_map.yaml'),
                'params_file': os.path.join(get_package_share_directory('wayfinder_navigation'), 'config', 'nav2_params.yaml'),
                'autostart': 'true',
            }.items()
    )

    return LaunchDescription([
        gz_sim,
        nav2_bringup,
        
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
            package='twist_stamper',
            executable='twist_stamper',
            name='twist_stamper',
            output='screen',
            parameters=[{
                'use_sim_time': True,
                'frame_id': 'base_link'
            }],
            remappings=[
                ('cmd_vel_in', 'cmd_vel'),
                ('cmd_vel_out', 'diff_drive_controller/cmd_vel'),
            ]
        ),

        Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2', 
            output='screen',
            parameters=[{'use_sim_time': True}],
            arguments=['-d', rviz_config_path]
        ),

        Node(
            package='wayfinder_vision',
            executable='marker_detector',
            name='marker_detector',
            output='screen',
            parameters=[{'use_sim_time': True}]
        ),

        Node(
            package='wayfinder_gui',
            executable='gui_node',
            name='gui_node',
            output='screen'
        ),

        Node(
            package='wayfinder_mission',
            executable='mission_node',
            name='mission_node',
            output='screen',
            parameters=[os.path.join(get_package_share_directory('wayfinder_mission'), 'config', 'checkpoints.yaml')]
        )
    ])
