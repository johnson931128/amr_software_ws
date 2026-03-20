# Project AMR: 軟硬體解耦自主移動機器人 - Week 1 開發紀錄

**開發者：** Johnson
**日期：** 2026-03-20
**專案目標：** 建立 ROS 2 基礎工作區，完成「模擬先行 (Sim-to-Real)」的第一階段，實現差速底盤建模與 C++ 開迴路控制。

---

## 🎯 1. 專題進度與架構安排 (Project Planning)

### 1.1 軟硬體解耦架構 (Service-Oriented Architecture)
本專題嚴格遵守「大腦（邏輯）」與「小腦/身體（演算法與物理實體）」解耦的原則：
* **大腦層 (C++)**：純粹的狀態機 (State Machine)，負責發布任務、監控狀態，絕不處理底層運動學公式。
* **物理層 (Gazebo)**：負責重力、碰撞與馬達模擬。
* **通訊層 (ROS 2)**：大腦與實體間透過 `Topic` (連續控制)、`Action` (長耗時任務)、`Service` (瞬間請求) 進行非同步通訊。

### 1.2 實機部署硬體策略 (Sim-to-Real Strategy)
針對實驗室的硬體限制（Raspberry Pi 3/4），制定以下資源榨乾策略：
* **放棄高耗能視覺 SLAM**：改採輕量級 2D LiDAR SLAM (`slam_toolbox`)。
* **記憶體極限微調**：目標使用 8GB RAM 的 Pi 4。若僅有 4GB 版本，將採用 **Headless Mode (無圖形介面)**，捨棄 Nav2 大禮包，僅單點安裝核心 `costmap` 與 `planner` 節點，並調降全域地圖解析度。

---

## 🛠️ 2. 開發環境建置與轉移 (Environment Setup)

### 2.1 逃離 VirtualBox，擁抱 WSL 2
* **痛點**：VirtualBox 無法直通 GPU，導致最新版 Gazebo Harmonic 的 3D 渲染引擎 (Ogre2) 因缺乏 OpenGL 硬體加速而呈現「全黑畫面 (Black Screen)」。
* **解決方案**：全面遷移至 **Windows Subsystem for Linux (WSL 2)**。
* **當前環境**：
  * **OS**: Ubuntu 24.04 (Noble)
  * **Middleware**: ROS 2 Jazzy
  * **Simulator**: Gazebo Harmonic
  * **硬體加速**: 成功調用本機 NVIDIA GeForce RTX 5060 Ti，RTF (Real Time Factor) 穩定保持在 1.0。

### 2.2 多機開發的 SSH 與 Git 管理
因 GitHub 棄用密碼驗證，於 WSL 2 中重新生成 `ed25519` SSH 金鑰並綁定 GitHub，實現專案從 VirtualBox 到 WSL 2 的無縫 `git clone` 轉移。

---

## 🚀 3. 核心實作內容 (Core Implementations)

### 3.1 差速驅動車體建模 (`amr_base.xacro`)
放棄複雜的阿克曼轉向，改採機動性最強的「差速驅動 (Differential Drive)」：
* **物理屬性 (Inertia & Collision)**：建立 `inertial_macros.xacro` 物理公式庫，賦予車身 (5kg) 與輪胎 (0.5kg) 真實質量與碰撞邊界。
* **Gazebo Harmonic 外掛**：植入新版 `gz-sim-diff-drive-system` 外掛，監聽 `/cmd_vel`。

### 3.2 C++ 決策大腦 (`square_tester.cpp`)
撰寫測試節點，驗證大腦對底盤的開迴路控制（走正方形）：
* 引入 `<geometry_msgs/msg/twist.hpp>`，封裝線速度 (`linear.x`) 與角速度 (`angular.z`)。
* 實作 `rclcpp::TimerBase`，以 10Hz (100ms) 頻率更新狀態機 (3秒直走、2秒轉彎90度)。

### 3.3 全自動化啟動檔 (`gazebo.launch.py`)
整合四個核心節點，實現一鍵啟動：
1. `robot_state_publisher`: 解析 Xacro 並廣播 TF 座標樹。
2. `ros_gz_sim`: 啟動 Gazebo Harmonic 物理引擎。
3. `create`: 將 AMR 模型生成至 3D 世界中。
4. **`ros_gz_bridge` (關鍵)**：自動搭起 ROS 2 與 Gazebo 之間的通訊橋樑。

---

## 🐛 4. 踩坑與除錯紀錄 (Troubleshooting)

| 遭遇問題 | 錯誤訊息 / 症狀 | 根本原因 | 解決方案 |
| :--- | :--- | :--- | :--- |
| **套件找不到** | `has no installation candidate` | 誤用舊版 Gazebo Classic 套件名稱 (`gazebo-ros-pkgs`)。 | 改安裝新版 Harmonic 專用橋接套件：`ros-jazzy-ros-gz`。 |
| **CMake 編譯失敗** | `can't find '/.../meshes'` | 專案採純幾何建模，未建立 `meshes` 3D 模型資料夾，觸發 CMake 安裝路徑嚴格檢查。 | 執行 `mkdir -p meshes` 建立空資料夾滿足編譯器規則。 |
| **編譯器失憶** | 找不到 `ament_cmake` | 新開啟的 WSL 2 終端機分頁未載入 ROS 2 環境變數。 | 手動執行 `source /opt/ros/jazzy/setup.bash`，並寫入 `.bashrc`。 |
| **車體接收不到指令** | C++ 程式狂跑，但車子原地不動 | ROS 2 Jazzy 與 Gazebo Harmonic 之間的 Topic 協定不互通。 | 啟動 `ros_gz_bridge parameter_bridge`，轉譯 `geometry_msgs/msg/Twist` 至 `gz.msgs.Twist`。 |
| **編譯快取污染** | `ModuleNotFoundError: No module named 'ament_package'` | 在未載入 ROS 2 環境變數（未 source）的狀態下不慎執行了 `colcon build`，導致 CMake 將錯誤的環境狀態寫入系統快取。即使後續補上 source，編譯器仍會讀取錯誤快取而持續罷工。 | 執行大清洗 `rm -rf build/ install/ log/`，徹底移除受污染的快取資料夾，重新 `source /opt/ros/jazzy/setup.bash` 後再次編譯即可完美解決。這凸顯了 ROS 2 開發中環境變數與底層 CMake 建置系統之間高度依賴的關係。 |


---
**⏭️ Next Step (Week 2)**：
在車體前方掛載 RPLIDAR A1M8 雷達模型，並導入感測器外掛。在 WSL 2 中開啟 RViz2，觀察雷達點雲數據，驗證 TF (Transform) 座標變換樹是否正確對接。









