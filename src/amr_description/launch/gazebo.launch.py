import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
import xacro

def generate_launch_description():
    pkg_share = get_package_share_directory('amr_description')

    # 1. 解析 URDF (Xacro)
    xacro_file = os.path.join(pkg_share, 'urdf', 'amr_base.xacro')
    robot_description = {'robot_description': xacro.process_file(xacro_file).toxml()}

    # 2. Robot State Publisher (發布 TF)
    rsp_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        parameters=[robot_description, {'use_sim_time': True}]
    )

    # 2.5 關節狀態發布器 (專門回報輪子角度給 RViz2)
    jsp_node = Node(
        package='joint_state_publisher',
        executable='joint_state_publisher',
        parameters=[{'use_sim_time': True}]
    )



    # 3. 啟動 Gazebo Harmonic
    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(
            get_package_share_directory('ros_gz_sim'), 'launch', 'gz_sim.launch.py')]),
        launch_arguments={'gz_args': 'empty.sdf -r -v 4'}.items()
    )

    # 4. 召喚機器人
    spawn_entity = Node(
        package='ros_gz_sim',
        executable='create',
        arguments=['-topic', 'robot_description', '-name', 'amr_robot', '-z', '0.1'],
        output='screen'
    )


	# 5. 重點：自動搭起大腦與實體世界的所有通訊橋樑
    bridge = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        arguments=[
            '/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock',
            '/cmd_vel@geometry_msgs/msg/Twist@gz.msgs.Twist',
            '/scan@sensor_msgs/msg/LaserScan@gz.msgs.LaserScan',
            '/tf@tf2_msgs/msg/TFMessage[gz.msgs.Pose_V',
            '/odom@nav_msgs/msg/Odometry[gz.msgs.Odometry'
        ],
        output='screen'
    )


    return LaunchDescription([rsp_node, jsp_node, gazebo, spawn_entity, bridge])
