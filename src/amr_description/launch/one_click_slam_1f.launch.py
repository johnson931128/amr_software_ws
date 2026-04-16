import os
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    # 取得套件路徑
    amr_description_dir = get_package_share_directory('amr_description')
    amr_mission_control_dir = get_package_share_directory('amr_mission_control')

    # 1. 引入基礎實體生成腳本 (Gazebo, URDF, ROS-GZ Bridge)
    spawn_amr_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(amr_description_dir, 'launch', 'spawn_amr_1f.launch.py')
        )
    )

    # 2. 引入 SLAM Toolbox (強制作業時間對齊虛擬時間)
    slam_params_file = os.path.join(amr_mission_control_dir, 'config', 'slam_params.yaml')
    slam_toolbox_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(get_package_share_directory('slam_toolbox'), 'launch', 'online_async_launch.py')
        ),
        launch_arguments={
            'slam_params_file': slam_params_file, 
            'use_sim_time': 'true'
        }.items()
    )

    # 3. 啟動 RViz2 視覺化介面 (強制作業時間對齊虛擬時間)
    rviz_config_file = os.path.join(amr_description_dir, 'rviz', 'display.rviz')
    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', rviz_config_file],
        parameters=[{'use_sim_time': True}],
        output='screen'
    )

    return LaunchDescription([
        spawn_amr_cmd,
        slam_toolbox_cmd,
        rviz_node
    ])
