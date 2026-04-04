import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    # 取得套件路徑
    amr_control_dir = get_package_share_directory('amr_mission_control')
    nav2_bringup_dir = get_package_share_directory('nav2_bringup')
    
    # 指定 Week 3 存檔的地圖路徑 (請確認名稱與實際存檔一致)
    map_yaml_file = os.path.join(os.path.expanduser('~'), 'amr_ws', 'maps', 'my_first_map.yaml')
    
    # 指定剛剛修改好的客製化 Nav2 參數檔路徑
    nav2_params_file = os.path.join(amr_control_dir, 'config', 'nav2_params.yaml')

    # 1. 啟動物理世界與車體藍圖 (沿用 Week 2 成果)
    gazebo_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(
            get_package_share_directory('amr_description'), 'launch', 'gazebo.launch.py')]),
    )

    # 2. 啟動 Nav2 導航堆疊 (包含 AMCL 定位與 Costmap/Planner/Controller)
    nav2_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(
            nav2_bringup_dir, 'launch', 'bringup_launch.py')]),
        launch_arguments={
            'map': map_yaml_file,
            'params_file': nav2_params_file,
            'use_sim_time': 'true' # 強制使用 Gazebo 虛擬時間
        }.items()
    )

    # 3. 啟動 RViz2 監控介面 (直接載入 Nav2 官方預設的完美視角配置)
    rviz_config_file = os.path.join(nav2_bringup_dir, 'rviz', 'nav2_default_view.rviz')
    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='screen',
        arguments=['-d', rviz_config_file],
        parameters=[{'use_sim_time': True}]
    )

    return LaunchDescription([gazebo_launch, nav2_launch, rviz_node])
