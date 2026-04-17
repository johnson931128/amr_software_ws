# 專題日誌 (2026-04-16)

## 一、 專案執行進度 (Progress)
1. [x] 模擬環境佈署：成功載入 1F 物理地圖與 AMR 機器人模型。
2. [x] SLAM 建圖實測：完成 1F 手動遙控建圖，並產出地圖網格。
3. [x] 地圖後期優化：完成 1f_map.png 像素級修圖（校直牆壁、去除噪點）。
4. [x] 地圖格式轉換：解決 PGM 格式與 Photopea 匯出相容性問題，切換為 PNG 並修改 YAML 映射。
5. [x] 座標數據提取：在 RViz2 渲染報錯（Shader Error）環境下，成功利用「Publish Point 點擊觸發 + 鼠標懸停狀態列」機制，提取出電梯口精確座標。

## 二、 系統除錯紀錄 (Debugging)
1. 視窗顯示 Bug (WSLg DPI 衝突)
   - 原因：WSLg 在 2K/1080P 螢幕環境下的自動縮放導致視窗位移、點擊不準。
   - 解法：於 ~/.bashrc 強制設定 export QT_AUTO_SCREEN_SCALE_FACTOR=0。
2. 模型渲染 Bug (動態 TF 缺失)
   - 原因：缺乏車輪旋轉的動態 TF 廣播，導致 RViz2 看不見車輪。
   - 解法：在 amr_base.xacro 注入 JointStatePublisher 外掛，並在 Bridge 開通 /joint_states。
3. 通訊癱瘓 Bug (Launch 語法錯誤)
   - 原因：spawn_amr_1f.launch.py 中 arguments 陣列字串遺漏「逗號」，導致 ros_gz_bridge 解析失敗。
   - 解法：補齊逗號，成功恢復 /tf 與 /scan 資料流。
4. SLAM 品質 Bug (里程計打滑與採樣不足)
   - 原因：驅動輪摩擦力不足導致原地空轉，且雷達頻率過低導致轉彎鬼影。
   - 解法：URDF 摩擦力 mu1/mu2 提升至 100.0，雷達頻率提升至 30Hz。
5. RViz2 渲染崩潰 Bug (GLSL Shader Error)
   - 原因：WSLg 顯示驅動無法編譯 indexed_8bit_image.frag。
   - 解法：放棄正常渲染，改用「點擊 Publish Point 啟動內部偵測機制 + 鼠標移至地圖 + 觀看左下角狀態列」之混合操作法提取座標。

## 三、 程式碼變更清單與目的 (Modified Files)
1. amr_description/urdf/amr_base.xacro
   - 目的：設定驅動輪高抓地力與 JointState 發布，解決物理打滑與視覺模型殘缺。
2. amr_description/urdf/lidar.xacro
   - 目的：將雷達採樣頻率從 10Hz 提升至 30Hz，減少 SLAM 建圖旋轉時的累積誤差。
3. amr_description/launch/spawn_amr_1f.launch.py
   - 目的：修正 Bridge 參數陣列語法錯誤，確保全系統資料通訊正常。
4. amr_description/launch/one_click_slam_1f.launch.py (新檔案)
   - 目的：建立一鍵啟動腳本，整合 Gazebo、RViz2 與 SLAM_Toolbox 節點。
5. amr_ws/maps/1f_map.yaml
   - 目的：手動修改 image 路徑，將原本指向 .pgm 改為修圖後的 .png。

## 四、 關鍵數據紀錄 (Data - 1F 電梯口座標)
實測提取之電梯口觸發範圍 (Bounding Box)：
- 左上角座標: [ X: 5.010,  Y: -6.130 ]
- 右下角座標: [ X: 0.289,  Y: -9.210 ]
- Z 軸參考值: 0.002 (2D 導航可忽略)
*座標範圍定義：X 軸 [0.289 ~ 5.010], Y 軸 [-9.210 ~ -6.130]。

## 五、 下一步行動 (Next Steps)
1. [ ] 執行 1F 樓層導航測試 (Nav2)：啟動導航系統，將電梯口座標寫入 `state_machine_node.cpp`，測試 AMR 是否能自主避障並精準抵達/觸發電梯等待邏輯。
2. [ ] 建置 B1 樓層地圖：建立地下室模擬場景 (如 `B1_map.sdf`)，並執行 SLAM 建圖、影像後期修飾與存檔之完整標準作業流程。
