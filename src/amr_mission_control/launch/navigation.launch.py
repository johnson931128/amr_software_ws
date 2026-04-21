import os
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    # 取得套件路徑
    amr_mission_control_dir = get_package_share_directory('amr_mission_control')
    nav2_bringup_dir = get_package_share_directory('nav2_bringup')

    # 1. 宣告外部參數：設定預設要載入的地圖為 1f_map.yaml
    map_yaml_file = LaunchConfiguration('map')
    declare_map_yaml_cmd = DeclareLaunchArgument(
        'map',
        default_value=os.path.join('/home/fanshunjie/amr_ws/maps', '1f_map.yaml'), # 注意：請把"你的用戶名"換成你虛擬機的帳號！
        description='Full path to map yaml file to load'
    )

    # 2. 宣告外部參數：設定 Nav2 的演算法參數檔
    params_file = LaunchConfiguration('params_file')
    declare_params_file_cmd = DeclareLaunchArgument(
        'params_file',
        default_value=os.path.join(amr_mission_control_dir, 'config', 'nav2_params.yaml'),
        description='Full path to the ROS2 parameters file to use for all launched nodes'
    )

    # 3. 呼叫官方 Nav2 啟動檔
    bringup_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(nav2_bringup_dir, 'launch', 'bringup_launch.py')
        ),
        launch_arguments={
            'map': map_yaml_file,
            'params_file': params_file,
            'use_sim_time': 'true' # 因為我們還在 Gazebo 模擬階段，這行很重要
        }.items()
    )

    return LaunchDescription([
        declare_map_yaml_cmd,
        declare_params_file_cmd,
        bringup_cmd
    ])
