import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
import xacro

def generate_launch_description():
    # 1. 路徑設定
    pkg_description = get_package_share_directory('amr_description')
    world_file = os.path.join(pkg_description, 'worlds', '1F_map.sdf')
    xacro_file = os.path.join(pkg_description, 'urdf', 'amr_base.xacro')

    # 2. 解析 Xacro 並取得 Robot Description
    robot_description_config = xacro.process_file(xacro_file).toxml()
    
    # 3. Gazebo 模擬器啟動 (Gazebo Harmonic)
    gz_sim = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(
            get_package_share_directory('ros_gz_sim'), 'launch', 'gz_sim.launch.py')]),
        launch_arguments={'gz_args': f'-r {world_file}'}.items(),
    )

    # 4. 機器人狀態發布者 (建立 base_link 等 TF 樹)
    node_robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='screen',
        parameters=[{'robot_description': robot_description_config, 'use_sim_time': True}]
    )

    # 5. 在指定座標召喚機器人 (-5.0, 0.0, 0.0, Yaw=0)
    spawn_robot = Node(
        package='ros_gz_sim',
        executable='create',
        arguments=[
            '-world', 'empty',
            '-name', 'amr_robot',
            '-topic', 'robot_description',
            '-x', '-5.0',
            '-y', '0.0',
            '-z', '0.1',  # 稍微懸空避免卡入地板
            '-Y', '0.0'   # Yaw 角度
        ],
        output='screen',
    )

    # 6. ROS-GZ Bridge (打通資料流：時鐘、雷達、速度、TF)
    bridge = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        arguments=[
            '/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock',
            '/scan@sensor_msgs/msg/LaserScan[gz.msgs.LaserScan',
            '/cmd_vel@geometry_msgs/msg/Twist]gz.msgs.Twist',
            '/tf@tf2_msgs/msg/TFMessage[gz.msgs.Pose_V',
            '/joint_states@sensor_msgs/msg/JointState[gz.msgs.Model'
        ],
        output='screen'
    )

    return LaunchDescription([
        gz_sim,
        node_robot_state_publisher,
        spawn_robot,
        bridge
    ])
