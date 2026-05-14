cmake_minimum_required(VERSION 3.8)
project(amr_hardware)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 1. 尋找依賴套件
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(std_msgs REQUIRED)

# =========================================================
# 2. 保留你原本的 amr_system.cpp 
# (如果它裡面目前還沒寫好或會報錯，你可以先在每一行前面加 # 把它註解掉)
#dd_executable(amr_system_node src/amr_system.cpp)
#arget_include_directories(amr_system_node PUBLIC
# $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
# $<INSTALL_INTERFACE:include>)
#ment_target_dependencies(amr_system_node rclcpp std_msgs geometry_msgs)
# =========================================================

# =========================================================
# 3. 註冊我們剛寫的 cmd_vel_to_wheel (作為動態載入組件 Component)
add_library(cmd_vel_to_wheel_node SHARED
  src/cmd_vel_to_wheel.cpp
)
target_include_directories(cmd_vel_to_wheel_node PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
ament_target_dependencies(cmd_vel_to_wheel_node
  rclcpp
  rclcpp_components
  geometry_msgs
  std_msgs
)
# 將它註冊為可執行的節點外掛
rclcpp_components_register_node(cmd_vel_to_wheel_node
  PLUGIN "amr_hardware::CmdVelToWheelNode"
  EXECUTABLE cmd_vel_to_wheel_exec
)
# =========================================================

# 4. 設定安裝路徑 (讓 ROS 2 找得到編譯好的檔案)
install(TARGETS
# amr_system_node
  cmd_vel_to_wheel_node
  ARCHIVE DESTINATION lib/${PROJECT_NAME}
  LIBRARY DESTINATION lib/${PROJECT_NAME}
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include
)

ament_package()
