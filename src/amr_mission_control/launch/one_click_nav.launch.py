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

    # 1. 啟動物理世界與實體 (Gazebo + URDF + Bridge)
    spawn_amr_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(amr_description_dir, 'launch', 'spawn_amr_1f.launch.py')
        )
    )

    # 2. 啟動導航大腦 (Nav2) - 會自動呼叫寫好的 navigation.launch.py
    nav2_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(amr_mission_control_dir, 'launch', 'navigation.launch.py')
        )
    )

    # 3. 啟動視覺化監視器 (RViz2)
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
        nav2_cmd,
        rviz_node
    ])
