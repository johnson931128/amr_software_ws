#!/usr/bin/env python3
import os

# 牆壁全域設定
WALL_HEIGHT = 2.0
WALL_THICKNESS = 0.2

# 牆壁參數陣列：定義格式為 [中心X, 中心Y, X軸長度(厚度或寬度), Y軸長度(厚度或長度), 識別名稱]
# 系統已依據草圖比例進行初步座標推算，中心點 (0,0) 位於地圖中央。
WALLS = [
    # --- 右下角：電梯區域 (綠色區塊 4m x 3m，假設其中心在 x=8, y=-8) ---
    # 電梯口預留上方開口，因此只建置左側與下方牆壁 (L型)
    [6.0, -10.0, WALL_THICKNESS, 8.0, "elevator_left_wall"],
    [8.0, -5.0, 4.0, WALL_THICKNESS, "elevator_top_wall"],
    
    # --- 右上角：樓梯/房間區域 (藍色區塊，U型包覆) ---
    [8.0, 10.0, 6.0, WALL_THICKNESS, "stairs_top_wall"],
    [8.0, 4.0, 6.0, WALL_THICKNESS, "stairs_bottom_wall"],
    [5.0, 7.0, WALL_THICKNESS, 6.0, "stairs_left_wall"],
    
    # --- 左下角：不可碰觸 L 型障礙物 ---
    [-10.0, -5.0, 6.0, WALL_THICKNESS, "bottom_left_horizontal"],
    [-7.0, -10.0, WALL_THICKNESS, 10.0, "bottom_left_vertical"],
    
    # --- 左上角：大型矩形障礙物 ---
    [-8.0, 12.0, 8.0, WALL_THICKNESS, "top_left_box_top"],
    [-8.0, 6.0, 8.0, WALL_THICKNESS, "top_left_box_bottom"],
    [-12.0, 9.0, WALL_THICKNESS, 6.0, "top_left_box_left"],
    [-4.0, 9.0, WALL_THICKNESS, 6.0, "top_left_box_right"],
    
    # --- 中間：小型矩形障礙物 ---
    [-4.0, 1.0, 4.0, WALL_THICKNESS, "mid_box_top"],
    [-4.0, -3.0, 4.0, WALL_THICKNESS, "mid_box_bottom"],
    [-6.0, -1.0, WALL_THICKNESS, 4.0, "mid_box_left"],
    [-2.0, -1.0, WALL_THICKNESS, 4.0, "mid_box_right"],
]

def generate_sdf_links(walls):
    links_xml = ""
    for i, w in enumerate(walls):
        cx, cy, size_x, size_y, name = w
        link = f"""
      <link name="{name}">
        <pose>{cx} {cy} {WALL_HEIGHT/2} 0 0 0</pose>
        <collision name="collision">
          <geometry><box><size>{size_x} {size_y} {WALL_HEIGHT}</size></box></geometry>
        </collision>
        <visual name="visual">
          <geometry><box><size>{size_x} {size_y} {WALL_HEIGHT}</size></box></geometry>
          <material>
            <ambient>0.5 0.5 0.5 1</ambient>
            <diffuse>0.6 0.6 0.6 1</diffuse>
          </material>
        </visual>
      </link>"""
        links_xml += link
    return links_xml

def main():
    sdf_template = f"""<?xml version="1.0" ?>
<sdf version="1.9">
  <world name="floor_1_world">
    <physics name="1ms" type="ignored">
        <max_step_size>0.001</max_step_size>
        <real_time_factor>1.0</real_time_factor>
    </physics>
    <plugin filename="gz-sim-physics-system" name="gz::sim::systems::Physics"/>
    <plugin filename="gz-sim-user-commands-system" name="gz::sim::systems::UserCommands"/>
    <plugin filename="gz-sim-scene-broadcaster-system" name="gz::sim::systems::SceneBroadcaster"/>
    
    <light type="directional" name="sun">
      <cast_shadows>true</cast_shadows>
      <pose>0 0 10 0 0 0</pose>
      <diffuse>0.8 0.8 0.8 1</diffuse>
      <specular>0.2 0.2 0.2 1</specular>
      <direction>-0.5 0.1 -0.9</direction>
    </light>

    <model name="ground_plane">
      <static>true</static>
      <link name="link">
        <collision name="collision">
          <geometry><plane><normal>0 0 1</normal><size>100 100</size></plane></geometry>
        </collision>
        <visual name="visual">
          <geometry><plane><normal>0 0 1</normal><size>100 100</size></plane></geometry>
          <material>
            <ambient>0.8 0.9 0.8 1</ambient>
            <diffuse>0.8 0.9 0.8 1</diffuse>
          </material>
        </visual>
      </link>
    </model>

    <model name="building_1f">
      <static>true</static>
      {generate_sdf_links(WALLS)}
    </model>
  </world>
</sdf>
"""
    # 輸出路徑設定 (請確保路徑符合當前終端機所在位置)
    sdf_template = f"""... (此處保留原本的 SDF 模板內容) ..."""
    
    # ---------------- 修正路徑解析邏輯 ----------------
    # 1. 取得這支 generate_map.py 檔案所在的絕對路徑
    current_script_dir = os.path.dirname(os.path.abspath(__file__))
    
    # 2. 推算回 amr_description 目錄 (因為腳本預期放在 scripts/ 下)
    amr_description_dir = os.path.dirname(current_script_dir)
    
    # 3. 組合成最終的輸出路徑
    output_path = os.path.join(amr_description_dir, "worlds", "1F.world")
    
    # 4. 確保 worlds 目錄存在
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    
    # 5. 寫入檔案
    with open(output_path, "w") as f:
        f.write(sdf_template)
    print(f"[系統提示] 1F.world 已成功生成於 {output_path}")

if __name__ == "__main__":
    main()
