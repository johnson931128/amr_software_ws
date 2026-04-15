#!/usr/bin/env python3
import os

# 牆壁全域設定
WALL_HEIGHT = 2.0
WALL_THICKNESS = 0.2

# 牆壁參數陣列：定義格式為 [中心X, 中心Y, X軸長度(厚度或寬度), Y軸長度(厚度或長度), 識別名稱]
WALLS = [
    # --- 1. 電梯區域 (內部淨空: 寬 2.0m x 深 3.0m，門口朝向 +Y) ---
    # 左牆與右牆之間的內側距離為 (9.1 - 0.1) - (6.9 + 0.1) = 2.0m 完美通道
    [8.0, -9.6, 2.4, WALL_THICKNESS, "elevator_back_wall"],   # 背牆 (-Y)
    [6.9, -8.0, WALL_THICKNESS, 3.4, "elevator_left_wall"],   # 左牆
    [9.1, -8.0, WALL_THICKNESS, 3.4, "elevator_right_wall"],  # 右牆
    
    # --- 2. 樓梯/房間區域 (內部淨空: 3.0m x 3.0m，門口朝向 -X) ---
    # 上牆與下牆之間的內側距離為 (7.6 - 0.1) - (4.4 + 0.1) = 3.0m 完美通道
    [9.6, 6.0, WALL_THICKNESS, 3.4, "stairs_back_wall"],      # 背牆 (+X)
    [8.0, 7.6, 3.4, WALL_THICKNESS, "stairs_top_wall"],       # 上牆 (+Y)
    [8.0, 4.4, 3.4, WALL_THICKNESS, "stairs_bottom_wall"],    # 下牆 (-Y)
    
    # --- 3. 左下角：不可碰觸 L 型障礙物 ---
    [-10.0, -5.0, 6.0, WALL_THICKNESS, "bottom_left_horizontal"],
    [-7.0, -10.0, WALL_THICKNESS, 10.0, "bottom_left_vertical"],
    
    # --- 4. 左上角：大型矩形障礙物 ---
    [-8.0, 12.0, 8.0, WALL_THICKNESS, "top_left_box_top"],
    [-8.0, 6.0, 8.0, WALL_THICKNESS, "top_left_box_bottom"],
    [-12.0, 9.0, WALL_THICKNESS, 6.0, "top_left_box_left"],
    [-4.0, 9.0, WALL_THICKNESS, 6.0, "top_left_box_right"],
    
    # --- 5. 中間：小型矩形障礙物 (稍微避開樓梯出口動線) ---
    [-4.0, 0.0, 4.0, WALL_THICKNESS, "mid_box_top"],
    [-4.0, -3.0, 4.0, WALL_THICKNESS, "mid_box_bottom"],
    [-6.0, -1.5, WALL_THICKNESS, 3.0, "mid_box_left"],
    [-2.0, -1.5, WALL_THICKNESS, 3.0, "mid_box_right"],
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
    
    # 精確對齊開發者指定之正確工作區路徑
    output_path = os.path.expanduser("~/amr_ws/src/amr_description/worlds/1F.world")
    
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    
    with open(output_path, "w") as f:
        f.write(sdf_template)
    print(f"[系統提示] 1F.world 已成功生成於 {output_path}")

if __name__ == "__main__":
    main()
