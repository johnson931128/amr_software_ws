import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node

def generate_launch_description():
    # 1. 呼叫身體與感測器：直接載入 Week 2 寫好的 Gazebo 啟動模組
    # 這會啟動物理引擎、生成機器人，並自動搭好 ros_gz_bridge 橋樑
    gazebo_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(
            get_package_share_directory('amr_description'), 'launch', 'gazebo.launch.py')]),
    )

    # 2. 啟動 SLAM 大腦：載入 slam_toolbox 的預設線上建圖模式
    # online_async 代表「非同步線上建圖」，最適合運算資源受限的環境，它會以最高效能處理 /scan 並發布 /map
	# 取得參數檔路徑
    slam_params_file = os.path.join(
        get_package_share_directory('amr_mission_control'), 'config', 'slam_params.yaml')

    # 啟動 SLAM 大腦，並掛載自定義參數
    slam_toolbox = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(
            get_package_share_directory('slam_toolbox'), 'launch', 'online_async_launch.py')]),
        launch_arguments={
            'use_sim_time': 'true',
            'slam_params_file': slam_params_file
        }.items()
    )
    # 3. 啟動上帝視角：載入 RViz2
    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        output='screen',
        parameters=[{'use_sim_time': True}]
    )

    return LaunchDescription([gazebo_launch, slam_toolbox, rviz_node])
