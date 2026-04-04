cmake_minimum_required(VERSION 3.8)
project(amr_mission_control)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)


# 1. 包含標頭檔的目錄
include_directories(include)

# 2. 新增執行檔 (將 .cpp 編譯為 Node)
add_executable(state_machine_node src/state_machine_node.cpp)

# 3. 連結 ROS 2 的相依套件
ament_target_dependencies(state_machine_node
  rclcpp
  std_msgs
  geometry_msgs
)

# 4. 安裝設定：把執行檔放到 ROS 2 找得到的地方
install(TARGETS
  state_machine_node
  DESTINATION lib/${PROJECT_NAME}
)

add_executable(square_tester src/square_tester.cpp)
ament_target_dependencies(square_tester rclcpp geometry_msgs)

install(TARGETS
  square_tester
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

install(DIRECTORY launch config maps
  DESTINATION share/${PROJECT_NAME}
)


ament_package()
