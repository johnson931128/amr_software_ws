# Autonomous Mobile Robot (AMR) Software Stack
![ROS 2](https://img.shields.io/badge/ROS_2-Jazzy-34A853?style=for-the-badge&logo=ros)
![C++](https://img.shields.io/badge/C++-17/20-00599C?style=for-the-badge&logo=c%2B%2B)
![Ubuntu](https://img.shields.io/badge/Ubuntu-24.04-E95420?style=for-the-badge&logo=ubuntu)

## Project Overview
This repository contains the software stack for an Autonomous Mobile Robot (AMR) with cross-floor navigation capabilities. The project emphasizes a **Sim2Real (Simulation to Reality)** approach, utilizing Gazebo for digital twin verification before deploying the deeply decoupled C++ codebase onto physical hardware.

## System Architecture
The system follows a strict "high cohesion, low coupling" design, divided into three main layers:

1. **Mission Control Layer (`amr_mission_control`)**
   - High-level decision making and state machine implementation.
   - Manages inter-floor navigation, map switching (e.g., 1F to B1), and autonomous behaviors.
2. **Navigation & Perception Layer (Nav2 & SLAM)**
   - Core path planning (Global/Local), dynamic obstacle avoidance, and costmap management.
   - Utilizes `slam_toolbox` for 2D occupancy grid generation.
3. **Hardware Abstraction Layer (`amr_hardware` & ESP32 Firmware)**
   - Low-level hardware interface and kinematics conversion.
   - Processes Odometry (Wheel encoders + IMU) and handles motor PID control loops.

## Hardware Specifications
| Component | Specification | Description |
| :--- | :--- | :--- |
| **Main Brain** | Raspberry Pi 5 (8GB) | High-level ROS 2 Nav2 & SLAM execution. |
| **Low-level Controller** | ESP32 NodeMCU-32S | Real-time motor control, encoder reading via PCNT. |
| **LiDAR** | A1M8R5 RPLiDAR | 360-degree 2D laser scanner. |
| **IMU** | MPU6050 | 6-DoF inertial measurement for odometry fusion. |
| **Actuators** | Sha Yang Ye 12V DC | Geared motors with Hall-effect encoders. |

## Build Instructions
Ensure you have sourced your ROS 2 Jazzy environment.
```bash
cd ~/amr_ws
rosdep install -i --from-path src --rosdistro jazzy -y
colcon build --symlink-install
source install/setup.bash
