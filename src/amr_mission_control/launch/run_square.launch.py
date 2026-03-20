import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, TimerAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node

def generate_launch_description():
    # 1. 呼叫身體：載入已經寫好的 Gazebo 啟動檔
    gazebo_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(
            get_package_share_directory('amr_description'), 'launch', 'gazebo.launch.py')]),
    )

    # 2. 呼叫大腦：啟動 C++ 走正方形節點
    brain_node = Node(
        package='amr_mission_control',
        executable='square_tester',
        output='screen'
    )

    # 3. 完美時機：設定延遲 3 秒後再接上大腦，等 Gazebo 準備好
    delayed_brain = TimerAction(
        period=3.0,
        actions=[brain_node]
    )

    return LaunchDescription([gazebo_launch, delayed_brain])
