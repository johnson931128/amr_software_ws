# Autonomous Mobile Robot (AMR) Software Stack

![ROS 2](https://img.shields.io/badge/ROS_2-Jazzy-34A853?style=for-the-badge&logo=ros)
![C++](https://img.shields.io/badge/C++-17/20-00599C?style=for-the-badge&logo=c%2B%2B)
![Ubuntu](https://img.shields.io/badge/Ubuntu-24.04-E95420?style=for-the-badge&logo=ubuntu)
![Raspberry Pi](https://img.shields.io/badge/Raspberry%20Pi-5-C51A4A?style=for-the-badge&logo=Raspberry-Pi)
![Espressif](https://img.shields.io/badge/ESP32-NodeMCU--32S-E7352C?style=for-the-badge&logo=espressif)
![Gazebo](https://img.shields.io/badge/Gazebo-Harmonic-FF7300?style=for-the-badge)

## 📖 1. Project Overview

This project delivers a production-grade software and hardware architecture for an Autonomous Mobile Robot (AMR) capable of **cross-floor navigation**. Adhering to the **Sim2Real (Simulation to Reality)** framework, the deployment pipeline verifies the robot's kinematics, spatial coordinates, and sensory inputs within a Gazebo Harmonic digital twin environment before deploying the modular C++ stack directly onto the physical hardware.

The system encapsulates high-performance SLAM (Simultaneous Localization and Mapping), Nav2 navigation algorithms, and AMCL probabilistic tracking. High-level execution is governed by a custom-built Finite State Machine (FSM) that autonomously coordinates elevator entry and dynamically hot-swaps multi-floor 2D occupancy grid maps.

---

## 🛠️ 2. Hardware Specifications & Functional Roles

The AMR utilizes a decoupled "Brain & Cerebellum" design pattern. High-level computational processing is handled by the main microprocessor, while low-level real-time hardware execution is isolated onto a dedicated microcontroller.

| Hardware Component | Specification / Model | Functional Role & System Description |
| :--- | :--- | :--- |
| **Main Brain** | Raspberry Pi 5 (8GB) | **[High-Level Decision & Algorithm Layer]**<br>Runs Ubuntu 24.04 Server and ROS 2 Jazzy. Responsible for execution of the Nav2 pipeline, SLAM spatial computing, parsing RPLIDAR point clouds, and executing the C++ cross-floor FSM logic. |
| **Low-Level MCU** | ESP32 NodeMCU-32S | **[Hardware Abstraction & Control Layer]**<br>Maintains an active data bridge with the Brain via micro-ROS. Runs a strict 50ms (20Hz) execution loop handling PID velocity adjustment, static deadzone injection, and raw sensor fusion for Odometry. |
| **Environment Sensor**| RPLiDAR A1M8 | Executes 360-degree laser scanning at 10Hz. Publishes raw data to the `/scan` topic used to compute dynamic local/global costmap inflation layers. |
| **Attitude Sensor** | MPU6050 (IMU) | Connects to the ESP32 via I2C (`0x68`). Samples Z-axis angular velocity (Yaw) to mitigate track slippage errors during in-place rotations. |
| **Motor Driver** | L298N Dual H-Bridge | Receives hardware-isolated PWM channels and directional logic states from the ESP32 to modulate 12V high-current output to the drivetrain. |
| **Actuators** | 12V DC Geared Motors | High-torque differential-drive motors coupled with integrated magnetic Hall-effect encoders to provide high-resolution tick-count telemetry. |

---

## 🏗️ 3. System Architecture & Workspace Directory Tree

The workspace maintains a highly modular layout divided into distinct packages to enforce clean boundaries between simulation, hardware bridges, and high-level behaviors.

```text
amr_ws/
├── maps/                        # Static map assets generated via SLAM (.yaml, .pgm)
│   ├── 1f_map.yaml              # 1st Floor structural occupancy grid configuration
│   └── B1_map.yaml              # Basement 1 structural occupancy grid configuration
│
└── src/
    ├── amr_mission_control/     # 【Mission Control Layer】
    │   ├── config/              
    │   │   ├── nav2_params.yaml # Customized Nav2 behaviors (inflation bounds, etc.)
    │   │   └── slam_params.yaml # Tuned slam_toolbox parameters
    │   ├── launch/              
    │   │   ├── mapper.launch.py        # Spawns mapping stacks (SLAM + RViz2 + Gazebo)
    │   │   ├── navigation.launch.py    # Spawns localization and path planning (Nav2)
    │   │   └── one_click_nav.launch.py # Universal orchestration file connecting Sim2Real
    │   └── src/                 
    │       └── state_machine_node.cpp  # [Core] High-level C++ FSM. Dispatches Action Goals and requests map swaps.
    │
    ├── amr_hardware/            # 【Hardware Abstraction Layer】
    │   ├── include/.../cmd_vel_to_wheel.hpp
    │   └── src/
    │       └── cmd_vel_to_wheel.cpp    # [Core] Kinematics Bridge: Converts /cmd_vel to wheel speeds and forwards telemetry.
    │
    └── amr_description/         # 【Physical Modeling & Simulation Layer】
        ├── urdf/                
        │   ├── amr_base.xacro   # Structural, visual, inertial, and gazebo plugin properties
        │   └── lidar.xacro      # Modular LiDAR joint settings and ray-trace parameters
        ├── worlds/              
        │   └── 1F_map.sdf       # 3D physical world asset simulating the facility layout
        └── launch/              
            ├── gazebo.launch.py       # Provisions the virtual physics server
            └── spawn_amr_1f.launch.py # Instantiates the AMR URDF in the simulation
```

---

## 🚀 4. Comprehensive Execution Flow

### Step 1: Workspace Initialization and Compilation
Ensure your system has properly initialized dependencies against the ROS 2 Jazzy distribution:
```bash
cd ~/amr_ws
# Resolve package dependency mappings automatically
rosdep install --from-paths src --ignore-src -r -y
# Compile using symlinked installation paths
colcon build --symlink-install
source install/setup.bash
```

### Step 2: Triggering the Navigation Stack & Digital Twin (Sim2Real)
Execute the master launch sequence. This orchestrates the Gazebo simulation backend, brings up the Nav2 navigation stacks, and initializes RViz2:
```bash
# Override potential rendering starvation bugs under virtualized shells (WSLg)
unset LIBGL_ALWAYS_SOFTWARE
export WAYLAND_DISPLAY=
export QT_QPA_PLATFORM=xcb

# Launch the integrated simulation and navigation stack
ros2 launch amr_mission_control one_click_nav.launch.py
```
* **Critical Step**: Upon RViz2 generation, select the `2D Pose Estimate` interactive tool and visually align the robot with its true starting location to initialize AMCL particle filters.

### Step 3: Invoking the C++ Finite State Machine Brain
Open a secondary terminal to launch the behavioral engine:
```bash
source ~/amr_ws/install/setup.bash
ros2 run amr_mission_control state_machine_node
```

### Step 4: Provisioning Physical Hardware Nodes
When migrating from simulation to physical deployment:
1. Ensure the Raspberry Pi is on the same subnet, matching the domain: `export ROS_DOMAIN_ID=30`.
2. Connect the RPLiDAR via physical serial and launch the hardware node:
   ```bash
   ros2 launch sllidar_ros2 sllidar_a1_launch.py serial_port:=/dev/rplidar
   ```
3. Use PlatformIO to compile and flash the codebase (`main.cpp`) to the ESP32 to start micro-ROS telemetry and PID control loops.

---

## ⚠️ 5. Developer Log & Troubleshooting Reference

* **WSLg RViz2 Interface Freezing (X11 & Wayland Thread Starvation)**
  * **Symptom**: RViz2 initializes, but the graphical window stops responding to user mouse inputs while CPU utilization spikes to 100%.
  * **Cause**: WSLg defaults to fallback software rendering (`llvmpipe`), blocking the UI thread during intense costmap updates.
  * **Mitigation**: Prepend execution calls with `unset LIBGL_ALWAYS_SOFTWARE` and force X11 backend fallback with `export QT_QPA_PLATFORM=xcb`.

* **ROS 2 DDS Network Isolation (WSL2 NAT Boundary Failures)**
  * **Symptom**: Nodes within a WSL2 shell cannot discover topics broadcast by a physical Raspberry Pi on the local network.
  * **Cause**: WSL2 isolates network adapters inside a virtualized NAT subnet, dropping UDP Multicast packets required by DDS.
  * **Mitigation**: Move physical testing onto a dedicated host running native Ubuntu 24.04 LTS, or execute nodes directly on the Raspberry Pi 5.

* **Simulation Time Synchronization Desync (Transform Extrapolation Errors)**
  * **Symptom**: Nav2 logs warning messages indicating `Lookup would require extrapolation into the future`, causing path planning to abort.
  * **Cause**: Nodes reading system hardware clocks mismatch with Gazebo's simulation clock.
  * **Mitigation**: Explicitly declare the parameter `'use_sim_time': True` across all custom nodes and launch scripts.
