cmake_minimum_required(VERSION 3.8)
project(network_bridge)

# 默认使用C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

# 查找依赖
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

# 添加可执行文件
add_executable(communicate_node src/up.cpp)
target_include_directories(communicate_node PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
ament_target_dependencies(communicate_node
  rclcpp
  std_msgs
)

# 安装
install(TARGETS communicate_node
  DESTINATION lib/${PROJECT_NAME}
)

# 安装launch文件（如果有）
install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

ament_package()