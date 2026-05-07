cmake_minimum_required(VERSION 3.8)
project(amr_hardware)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 尋找需要的 ROS 2 核心套件
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(pluginlib REQUIRED)

# 定義要編譯的 C++ 檔案與包含路徑
add_library(${PROJECT_NAME} SHARED
  src/amr_system.cpp
)
target_include_directories(${PROJECT_NAME} PRIVATE
  include
)
ament_target_dependencies(${PROJECT_NAME}
  rclcpp
  hardware_interface
  pluginlib
)

# 註冊 plugin 描述檔
pluginlib_export_plugin_description_file(hardware_interface amr_hardware.xml)

# 設定安裝路徑
install(TARGETS ${PROJECT_NAME}
  DESTINATION lib/${PROJECT_NAME}
)
install(DIRECTORY include/
  DESTINATION include
)
install(FILES amr_hardware.xml
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
