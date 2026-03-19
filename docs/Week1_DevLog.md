# Project AMR: 軟硬體解耦自主移動機器人 - Week 1 開發紀錄

## 本週核心目標
建立 ROS 2 基礎工作區，完成「模擬先行 (Sim-to-Real)」的第一階段：
1. **數位孿生建置**：完成阿克曼轉向底盤的參數化建模 (URDF/Xacro)。
2. **大腦決策層打底**：以現代 C++ (C++11) 實作高內聚、低耦合的跨樓層任務狀態機與排程器。

## 開發環境
* **OS**: Ubuntu 24.04
* **Middleware**: ROS 2 Jazzy
* **Language**: C++, Python, XML/Xacro
* **Tools**: Vim, CMake, Git, RViz2

## 實作進度

### 1. 車體物理建模 (amr_description)
* 使用 `xacro:macro` 模組化設計四輪底盤，提升程式碼重用性。
* 成功定義 `base_link` 至各輪胎 (`front_left_wheel` 等) 的 TF 轉換樹 (Transform Tree)。
* 撰寫 Python Launch 腳本，自動解析 Xacro 並一鍵啟動 `robot_state_publisher` 與 `rviz2` (支援自動載入 .rviz 設定檔)。

### 2. 任務決策層大腦 (amr_mission_control)
* **強型別狀態機**：使用 C++ `enum class` 嚴格管控 AMR 狀態 (IDLE, NAV_TO_ELEVATOR, ENTER_ELEVATOR 等)，避免傳統整數狀態造成的型別不安全問題。
* **任務排程器 (Waypoint Scheduler)**：導入 C++ STL 容器 `std::queue<Point2D>`，模擬遊戲 AI 的 NPC 巡邏邏輯，實現非同步的座標點派發與消化機制。
* **Game Loop 概念**：透過 `rclcpp::TimerBase` 設定 2 秒週期的狀態更新迴圈。

## 踩坑與除錯紀錄 (Troubleshooting)

1. **缺漏 Xacro 解析套件**
   * *報錯*：`Could not find a package configuration file provided by "xacro"`
   * *解法*：系統底層未安裝該套件，執行 `sudo apt install ros-jazzy-xacro` 後重新編譯。
2. **RViz2 無法渲染輪子 (TF 斷層)**
   * *報錯*：`No transform from [front_left_wheel] to [base_link]`
   * *解法*：因為關節屬性為 `continuous`，在未連接實體編碼器前，需在 Launch 檔加入 `joint_state_publisher_gui` 來模擬輪胎旋轉的 Joint States。
3. **Git Commit 身分驗證失敗**
   * *報錯*：`Author identity unknown`
   * *解法*：使用 `git config --global user.email` 與 `user.name` 補齊全域簽名設定後再次 Commit。

## 📝 常用指令備忘錄 (Commands Cheat Sheet)

* **編譯並建立軟連結 (修改 Python/URDF 免重編譯)**：
  `colcon build --symlink-install`
* **僅編譯特定 C++ 套件**：
  `colcon build --packages-select amr_mission_control`
* **載入環境變數**：
  `source install/setup.bash`
* **啟動車體視覺化與大腦節點**：
  `ros2 launch amr_description display.launch.py`
  `ros2 run amr_mission_control state_machine_node`

---
**Next Step (Week 2)**：為車體模型加上 `<collision>` (碰撞邊界) 與 `<inertial>` (質量慣性)，並將數位孿生推進 Gazebo 物理引擎，測試鍵盤遙控與重力反應。
