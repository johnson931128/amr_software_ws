# Project AMR: 核心程式碼與系統架構解析 (Week 1)

這份文件專門紀錄 Week 1 開發中，大腦決策層 (C++) 與系統啟動檔 (Python Launch) 的完整運作邏輯與架構拆解。

---

## 🧠 1. 大腦決策層：`square_tester.cpp`

這個 C++ 節點扮演「大腦」的角色，負責下達純粹的運動邏輯（走正方形），而不涉及底層馬達的物理運算。

### 1.1 核心機制拆解

* **`rclcpp::Node` 繼承**：這是 ROS 2 現代 C++ 的標準寫法，讓這個類別具備所有節點該有的通訊能力。
* **Publisher (發布者)**：
  * **目標 Topic**：`/cmd_vel`
  * **訊息型別**：`geometry_msgs::msg::Twist`
  * **功能**：大腦的「嘴巴」，專門對外廣播包含了線速度 (`linear.x`) 與角速度 (`angular.z`) 的移動指令。
* **Timer (計時器/Game Loop)**：
  * **頻率設定**：`100ms` (等於 10 Hz)。
  * **功能**：大腦的「心跳」。每 0.1 秒就會強制喚醒 `timer_callback()` 函式一次，確保機器人有穩定且連續的控制訊號。

### 1.2 狀態機邏輯 (State Machine)
程式碼內部實作了一個極簡的狀態機，透過 `state_` 與 `tick_count_` 兩個變數來切換：
* **State 0 (直走模式)**：
  * 給予前進速度：`msg.linear.x = 0.2` (m/s)。
  * 計數器：執行 30 次 (30 * 0.1s = 3秒)。
  * 條件達成：切換至 State 1，計數器歸零。
* **State 1 (轉彎模式)**：
  * 給予旋轉速度：`msg.angular.z = 0.785` (rad/s，大約是 $\pi/4$)。
  * 計數器：執行 20 次 (20 * 0.1s = 2秒，總計轉動約 90 度)。
  * 條件達成：切換回 State 0，計數器歸零。

---

## 🚀 2. 系統啟動與自動化：`gazebo.launch.py`

Launch 檔扮演「總指揮」的角色。在沒有 Launch 檔的情況下，我們需要手動開 4 個終端機打指令；有了它，就能一鍵喚醒所有系統。

### 2.1 四大核心節點解析

**① 實體解析器 (`robot_state_publisher`)**
* **功能**：讀取我們的 `amr_base.xacro` (車體藍圖)，將它翻譯成標準的 XML 格式 (`robot_description`)。
* **附帶作用**：自動計算並發布車體各個關節之間的 TF (Transform) 座標樹，例如 `base_link` 到 `left_wheel` 的相對位置。

**② 物理世界引擎 (`gz_sim`)**
* **功能**：啟動 Gazebo Harmonic 模擬器軟體。
* **參數**：`empty.sdf -r -v 4` 代表載入空無一物的世界，啟動後自動播放 (`-r`)，並開啟 level 4 的詳細錯誤除錯訊息。

**③ 實體召喚師 (`create` node from `ros_gz_sim`)**
* **功能**：把步驟 ① 解析好的 `robot_description` 模型，正式丟進步驟 ② 的 Gazebo 物理世界中。
* **參數**：`-z 0.1` 指定車子從 Z 軸 10 公分高的半空中掉下來，用來驗證重力與物理碰撞是否正常運作。

**④ 異質通訊橋樑 (`ros_gz_bridge`)**
* **功能**：這是 ROS 2 Jazzy 與新版 Gazebo 之間不可或缺的「翻譯官」。
* **參數**：`/cmd_vel@geometry_msgs/msg/Twist@gz.msgs.Twist`。
* **運作邏輯**：它在背景監聽 ROS 2 的 `/cmd_vel` (大腦發出的指令)，將其打包翻譯成 Gazebo 專屬的 `gz.msgs.Twist` 格式，再餵給底盤的差速驅動外掛。少了這個節點，大腦與身體就會徹底失聯。
