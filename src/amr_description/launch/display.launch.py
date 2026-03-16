import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
import xacro

def generate_launch_description():
    pkg_name = 'amr_description'
    pkg_share = get_package_share_directory(pkg_name)

    # 1. 解析 Xacro 檔案
    xacro_file = os.path.join(pkg_share, 'urdf', 'amr_base.xacro')
    robot_description_config = xacro.process_file(xacro_file)
    robot_description = {'robot_description': robot_description_config.toxml()}

    # 2. 取得剛剛存檔的 RViz2 設定檔路徑
    rviz_config_file = os.path.join(pkg_share, 'rviz', 'display.rviz')

    # 設定 robot_state_publisher 節點
    rsp_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='screen',
        parameters=[robot_description]
    )

    # 設定 RViz2 節點 (加入 arguments 自動載入設定)
    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        output='screen',
        arguments=['-d', rviz_config_file]
    )

    # 設定假編碼器 GUI 節點
    jsp_gui_node = Node(
        package='joint_state_publisher_gui',
        executable='joint_state_publisher_gui',
        output='screen'
    )

    return LaunchDescription([rsp_node, jsp_gui_node, rviz_node])
