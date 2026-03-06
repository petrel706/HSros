cmake_minimum_required(VERSION 3.8)
project(hsr_socket_driver)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(trajectory_msgs REQUIRED)
find_package(control_msgs REQUIRED)

set(HSC3API_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../Hsc3Api" CACHE PATH "Path to Hsc3Api SDK root")

set(HSC3API_INCLUDE_DIR "${HSC3API_ROOT}/include")
set(HSC3API_LIB_DIR "${HSC3API_ROOT}/lib")

if(EXISTS "${HSC3API_ROOT}/lib/x64")
  # 兼容旧目录结构
  set(HSC3API_LIB_DIR "${HSC3API_ROOT}/lib/x64")
endif()

if(NOT EXISTS "${HSC3API_INCLUDE_DIR}/CommApi.h")
  message(FATAL_ERROR "Cannot find CommApi.h under HSC3API_ROOT='${HSC3API_ROOT}'. "
                      "Please set -DHSC3API_ROOT=/path/to/Hsc3Api")
endif()

add_library(hsr_hsc3_client
  src/hsc3_client.cpp
)

# 兼容 Hsc3 官方示例所使用的宏（主要是 _LINUX_ 影响 DLL_EXPORT 定义）
target_compile_definitions(hsr_hsc3_client
  PUBLIC
    _LINUX_=
)

target_include_directories(hsr_hsc3_client
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${HSC3API_INCLUDE_DIR}
)
target_link_directories(hsr_hsc3_client PUBLIC ${HSC3API_LIB_DIR})
target_link_libraries(hsr_hsc3_client
  CommApi
  Hsc3Api
  LogApi
  pthread
  m
  boost_system
  boost_thread
  boost_chrono
)

ament_target_dependencies(hsr_hsc3_client
  rclcpp
)

# 让可执行文件运行时能直接找到 Hsc3Api 的 .so（减少依赖 LD_LIBRARY_PATH）
set_target_properties(hsr_hsc3_client PROPERTIES
  BUILD_RPATH "${HSC3API_LIB_DIR}"
  INSTALL_RPATH "$ORIGIN/.."
)

add_executable(hsr_socket_driver_node src/hsr_socket_driver_node.cpp)
target_link_libraries(hsr_socket_driver_node hsr_hsc3_client)
ament_target_dependencies(hsr_socket_driver_node
  rclcpp
  rclcpp_action
  sensor_msgs
  trajectory_msgs
  control_msgs
)
set_target_properties(hsr_socket_driver_node PROPERTIES
  BUILD_RPATH "${HSC3API_LIB_DIR}"
  INSTALL_RPATH "$ORIGIN/.."
)

add_executable(demo_comm_api_training_node src/demo_comm_api_training_node.cpp)
target_link_libraries(demo_comm_api_training_node hsr_hsc3_client)
ament_target_dependencies(demo_comm_api_training_node rclcpp)
set_target_properties(demo_comm_api_training_node PROPERTIES
  BUILD_RPATH "${HSC3API_LIB_DIR}"
  INSTALL_RPATH "$ORIGIN/.."
)

add_executable(demo_training_jog_node src/demo_training_jog_node.cpp)
target_link_libraries(demo_training_jog_node hsr_hsc3_client)
ament_target_dependencies(demo_training_jog_node rclcpp)
set_target_properties(demo_training_jog_node PROPERTIES
  BUILD_RPATH "${HSC3API_LIB_DIR}"
  INSTALL_RPATH "$ORIGIN/.."
)

add_executable(demo_training_jog_single_node src/demo_training_jog_single_node.cpp)
target_link_libraries(demo_training_jog_single_node hsr_hsc3_client)
ament_target_dependencies(demo_training_jog_single_node
  rclcpp
  rclcpp_action
  sensor_msgs
  trajectory_msgs
  control_msgs
)
set_target_properties(demo_training_jog_single_node PROPERTIES
  BUILD_RPATH "${HSC3API_LIB_DIR}"
  INSTALL_RPATH "$ORIGIN/.."
)

add_executable(demo_training_jog_home_node src/demo_training_jog_home_node.cpp)
target_link_libraries(demo_training_jog_home_node hsr_hsc3_client)
ament_target_dependencies(demo_training_jog_home_node
  rclcpp
  rclcpp_action
  trajectory_msgs
  control_msgs
)
set_target_properties(demo_training_jog_home_node PROPERTIES
  BUILD_RPATH "${HSC3API_LIB_DIR}"
  INSTALL_RPATH "$ORIGIN/.."
)

add_executable(demo_training_move_joint_node src/demo_training_move_joint_node.cpp)
target_link_libraries(demo_training_move_joint_node hsr_hsc3_client)
ament_target_dependencies(demo_training_move_joint_node
  rclcpp
  rclcpp_action
  trajectory_msgs
  control_msgs
)
set_target_properties(demo_training_move_joint_node PROPERTIES
  BUILD_RPATH "${HSC3API_LIB_DIR}"
  INSTALL_RPATH "$ORIGIN/.."
)

add_executable(demo_cartesian_move_node src/demo_cartesian_move_node.cpp)
target_link_libraries(demo_cartesian_move_node hsr_hsc3_client)
ament_target_dependencies(demo_cartesian_move_node rclcpp)
set_target_properties(demo_cartesian_move_node PROPERTIES
  BUILD_RPATH "${HSC3API_LIB_DIR}"
  INSTALL_RPATH "$ORIGIN/.."
)

add_executable(demo_sys_alarm_node src/demo_sys_alarm_node.cpp)
target_link_libraries(demo_sys_alarm_node hsr_hsc3_client)
ament_target_dependencies(demo_sys_alarm_node rclcpp)
set_target_properties(demo_sys_alarm_node PROPERTIES
  BUILD_RPATH "${HSC3API_LIB_DIR}"
  INSTALL_RPATH "$ORIGIN/.."
)

add_executable(keyboard_teleop_joint_node src/keyboard_teleop_joint_node.cpp)
ament_target_dependencies(keyboard_teleop_joint_node
  rclcpp
  rclcpp_action
  sensor_msgs
  trajectory_msgs
  control_msgs
)

# 把 Hsc3Api 的运行时 .so 一并安装到本包的 lib/ 下（避免每次手动设置 LD_LIBRARY_PATH）
install(FILES
  ${HSC3API_LIB_DIR}/libCommApi.so
  ${HSC3API_LIB_DIR}/libHsc3Api.so
  ${HSC3API_LIB_DIR}/libLogApi.so
  DESTINATION lib
)

install(TARGETS
  hsr_hsc3_client
  hsr_socket_driver_node
  demo_comm_api_training_node
  demo_training_jog_node
  demo_training_jog_single_node
  demo_training_jog_home_node
  demo_training_move_joint_node
  demo_cartesian_move_node
  demo_sys_alarm_node
  keyboard_teleop_joint_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include
)

install(DIRECTORY launch config
  DESTINATION share/${PROJECT_NAME}
)

ament_package()

