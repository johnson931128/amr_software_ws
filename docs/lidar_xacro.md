# LiDAR (RPLIDAR A1M8) Xacro 模型語法解析

本文件紀錄了在 ROS 2 中使用 URDF/Xacro 定義雷達感測器的三大核心區塊語法。

---

## 1. 座標變換與安裝位置 (`<joint>`)

`<joint>` 標籤負責定義 TF (Transform) 座標樹，也就是雷達與車體的相對位置。

* **`type="fixed"`**：固定關節。雷達是鎖死在車體上的，不會像輪子一樣旋轉。
* **`<parent link="base_link"/>`**：父節點。以車體中心點作為測量基準。
* **`<child link="laser_frame"/>`**：子節點。定義這個雷達專屬的座標系名稱。
* **`<origin xyz="0 0 0.15" rpy="0 0 0"/>`**：
  * `xyz`：平移距離 (公尺)。`0 0 0.15` 代表雷達安裝在車身正中央，往上方 (Z軸) 浮空 15 公分處。
  * `rpy`：旋轉角度 (Roll, Pitch, Yaw，單位為弧度)。`0 0 0` 代表雷達水平安裝，沒有傾斜。

---

## 2. 實體外觀與物理屬性 (`<link>`)

`<link>` 標籤負責定義在 Gazebo 與 RViz2 中看到的樣子，以及物理碰撞的邊界。

* **`<visual>` (視覺外觀)**：
  * **`<geometry><cylinder .../></geometry>`**：定義形狀為圓柱體 (Cylinder)，並設定半徑 (`radius`) 與高度 (`length`)。
  * **`<material>`**：定義在 RViz2 中顯示的顏色（此處設為黑色）。
* **`<collision>` (碰撞邊界)**：
  * 定義物理引擎計算撞擊時的體積。通常與 `<visual>` 的形狀大小一模一樣，確保車子不會穿牆。
* **`<xacro:inertial_cylinder ...>` (慣性矩與質量)**：
  * 呼叫我們自定義的巨集 (Macro)，賦予這個雷達真實的質量（0.17 kg），這樣 Gazebo 才能計算正確的重心與運動慣性。

---

## 3. Gazebo 模擬器感測器外掛 (`<gazebo>`)

這是讓模型「活過來」的關鍵。URDF 原生不支援感測器邏輯，必須靠 `<gazebo>` 標籤呼叫外部的 C++ 外掛。

* **`<sensor type="gpu_lidar">`**：
  * 宣告這是一個雷達感測器。特別使用 `gpu_lidar`（而非一般 `lidar` 或 `ray`），讓運算交由你的 RTX 5060 Ti 處理，大幅提升模擬的 Real Time Factor (RTF)。
* **`<topic>/scan</topic>`**：定義點雲數據發布到 ROS 2 的哪一個 Topic。
* **`<update_rate>10</update_rate>`**：雷達掃描頻率，A1M8 規格約為 10Hz (每秒轉 10 圈)。
* **`<ray>` (雷射射線參數)**：
  * **`<horizontal><samples>360</samples>`**：水平掃描 360 個點 (每度一個點)。
  * **`<min_angle>` / `<max_angle>`**：掃描範圍。`-3.14159` 到 `3.14159` 代表 $-\pi$ 到 $\pi$，也就是完整的 360 度環景。
  * **`<range>`**：
    * `<min>0.15</min>`：盲區。距離小於 15 公分的障礙物測不到。
    * `<max>12.0</max>`：最大偵測距離為 12 公尺。
    * `<resolution>0.01</resolution>`：測距精準度為 1 公分。
* **`<visualize>true</visualize>`**：在 Gazebo 畫面中顯示半透明的藍色雷射掃描線，方便 Debug 觀察射線是否被車體自己擋住。
