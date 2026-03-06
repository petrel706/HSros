# Install script for directory: /home/huashu/co_605_arm/Hsc3DemoLinux/hsr_socket_driver

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/huashu/co_605_arm/Hsc3DemoLinux/install/hsr_socket_driver")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE FILES
    "/home/huashu/co_605_arm/Hsc3DemoLinux/hsr_socket_driver/../Hsc3Api/lib/libCommApi.so"
    "/home/huashu/co_605_arm/Hsc3DemoLinux/hsr_socket_driver/../Hsc3Api/lib/libHsc3Api.so"
    "/home/huashu/co_605_arm/Hsc3DemoLinux/hsr_socket_driver/../Hsc3Api/lib/libLogApi.so"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/huashu/co_605_arm/Hsc3DemoLinux/build/hsr_socket_driver/libhsr_hsc3_client.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/hsr_socket_driver_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/hsr_socket_driver_node")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/hsr_socket_driver_node"
         RPATH "$ORIGIN/..")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver" TYPE EXECUTABLE FILES "/home/huashu/co_605_arm/Hsc3DemoLinux/build/hsr_socket_driver/hsr_socket_driver_node")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/hsr_socket_driver_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/hsr_socket_driver_node")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/hsr_socket_driver_node"
         OLD_RPATH "/home/huashu/co_605_arm/Hsc3DemoLinux/hsr_socket_driver/../Hsc3Api/lib:/opt/ros/humble/lib:"
         NEW_RPATH "$ORIGIN/..")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/hsr_socket_driver_node")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_comm_api_training_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_comm_api_training_node")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_comm_api_training_node"
         RPATH "$ORIGIN/..")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver" TYPE EXECUTABLE FILES "/home/huashu/co_605_arm/Hsc3DemoLinux/build/hsr_socket_driver/demo_comm_api_training_node")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_comm_api_training_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_comm_api_training_node")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_comm_api_training_node"
         OLD_RPATH "/home/huashu/co_605_arm/Hsc3DemoLinux/hsr_socket_driver/../Hsc3Api/lib:/opt/ros/humble/lib:"
         NEW_RPATH "$ORIGIN/..")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_comm_api_training_node")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_jog_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_jog_node")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_jog_node"
         RPATH "$ORIGIN/..")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver" TYPE EXECUTABLE FILES "/home/huashu/co_605_arm/Hsc3DemoLinux/build/hsr_socket_driver/demo_training_jog_node")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_jog_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_jog_node")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_jog_node"
         OLD_RPATH "/home/huashu/co_605_arm/Hsc3DemoLinux/hsr_socket_driver/../Hsc3Api/lib:/opt/ros/humble/lib:"
         NEW_RPATH "$ORIGIN/..")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_jog_node")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_jog_single_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_jog_single_node")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_jog_single_node"
         RPATH "$ORIGIN/..")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver" TYPE EXECUTABLE FILES "/home/huashu/co_605_arm/Hsc3DemoLinux/build/hsr_socket_driver/demo_training_jog_single_node")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_jog_single_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_jog_single_node")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_jog_single_node"
         OLD_RPATH "/home/huashu/co_605_arm/Hsc3DemoLinux/hsr_socket_driver/../Hsc3Api/lib:/opt/ros/humble/lib:"
         NEW_RPATH "$ORIGIN/..")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_jog_single_node")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_jog_home_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_jog_home_node")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_jog_home_node"
         RPATH "$ORIGIN/..")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver" TYPE EXECUTABLE FILES "/home/huashu/co_605_arm/Hsc3DemoLinux/build/hsr_socket_driver/demo_training_jog_home_node")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_jog_home_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_jog_home_node")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_jog_home_node"
         OLD_RPATH "/home/huashu/co_605_arm/Hsc3DemoLinux/hsr_socket_driver/../Hsc3Api/lib:/opt/ros/humble/lib:"
         NEW_RPATH "$ORIGIN/..")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_jog_home_node")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_move_joint_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_move_joint_node")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_move_joint_node"
         RPATH "$ORIGIN/..")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver" TYPE EXECUTABLE FILES "/home/huashu/co_605_arm/Hsc3DemoLinux/build/hsr_socket_driver/demo_training_move_joint_node")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_move_joint_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_move_joint_node")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_move_joint_node"
         OLD_RPATH "/home/huashu/co_605_arm/Hsc3DemoLinux/hsr_socket_driver/../Hsc3Api/lib:/opt/ros/humble/lib:"
         NEW_RPATH "$ORIGIN/..")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_training_move_joint_node")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_cartesian_move_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_cartesian_move_node")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_cartesian_move_node"
         RPATH "$ORIGIN/..")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver" TYPE EXECUTABLE FILES "/home/huashu/co_605_arm/Hsc3DemoLinux/build/hsr_socket_driver/demo_cartesian_move_node")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_cartesian_move_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_cartesian_move_node")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_cartesian_move_node"
         OLD_RPATH "/home/huashu/co_605_arm/Hsc3DemoLinux/hsr_socket_driver/../Hsc3Api/lib:/opt/ros/humble/lib:"
         NEW_RPATH "$ORIGIN/..")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_cartesian_move_node")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_sys_alarm_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_sys_alarm_node")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_sys_alarm_node"
         RPATH "$ORIGIN/..")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver" TYPE EXECUTABLE FILES "/home/huashu/co_605_arm/Hsc3DemoLinux/build/hsr_socket_driver/demo_sys_alarm_node")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_sys_alarm_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_sys_alarm_node")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_sys_alarm_node"
         OLD_RPATH "/home/huashu/co_605_arm/Hsc3DemoLinux/hsr_socket_driver/../Hsc3Api/lib:/opt/ros/humble/lib:"
         NEW_RPATH "$ORIGIN/..")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/demo_sys_alarm_node")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/keyboard_teleop_joint_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/keyboard_teleop_joint_node")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/keyboard_teleop_joint_node"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver" TYPE EXECUTABLE FILES "/home/huashu/co_605_arm/Hsc3DemoLinux/build/hsr_socket_driver/keyboard_teleop_joint_node")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/keyboard_teleop_joint_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/keyboard_teleop_joint_node")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/keyboard_teleop_joint_node"
         OLD_RPATH "/opt/ros/humble/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/hsr_socket_driver/keyboard_teleop_joint_node")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE DIRECTORY FILES "/home/huashu/co_605_arm/Hsc3DemoLinux/hsr_socket_driver/include/")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/hsr_socket_driver" TYPE DIRECTORY FILES
    "/home/huashu/co_605_arm/Hsc3DemoLinux/hsr_socket_driver/launch"
    "/home/huashu/co_605_arm/Hsc3DemoLinux/hsr_socket_driver/config"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/ament_index/resource_index/package_run_dependencies" TYPE FILE FILES "/home/huashu/co_605_arm/Hsc3DemoLinux/build/hsr_socket_driver/ament_cmake_index/share/ament_index/resource_index/package_run_dependencies/hsr_socket_driver")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/ament_index/resource_index/parent_prefix_path" TYPE FILE FILES "/home/huashu/co_605_arm/Hsc3DemoLinux/build/hsr_socket_driver/ament_cmake_index/share/ament_index/resource_index/parent_prefix_path/hsr_socket_driver")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/hsr_socket_driver/environment" TYPE FILE FILES "/opt/ros/humble/share/ament_cmake_core/cmake/environment_hooks/environment/ament_prefix_path.sh")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/hsr_socket_driver/environment" TYPE FILE FILES "/home/huashu/co_605_arm/Hsc3DemoLinux/build/hsr_socket_driver/ament_cmake_environment_hooks/ament_prefix_path.dsv")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/hsr_socket_driver/environment" TYPE FILE FILES "/opt/ros/humble/share/ament_cmake_core/cmake/environment_hooks/environment/path.sh")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/hsr_socket_driver/environment" TYPE FILE FILES "/home/huashu/co_605_arm/Hsc3DemoLinux/build/hsr_socket_driver/ament_cmake_environment_hooks/path.dsv")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/hsr_socket_driver" TYPE FILE FILES "/home/huashu/co_605_arm/Hsc3DemoLinux/build/hsr_socket_driver/ament_cmake_environment_hooks/local_setup.bash")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/hsr_socket_driver" TYPE FILE FILES "/home/huashu/co_605_arm/Hsc3DemoLinux/build/hsr_socket_driver/ament_cmake_environment_hooks/local_setup.sh")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/hsr_socket_driver" TYPE FILE FILES "/home/huashu/co_605_arm/Hsc3DemoLinux/build/hsr_socket_driver/ament_cmake_environment_hooks/local_setup.zsh")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/hsr_socket_driver" TYPE FILE FILES "/home/huashu/co_605_arm/Hsc3DemoLinux/build/hsr_socket_driver/ament_cmake_environment_hooks/local_setup.dsv")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/hsr_socket_driver" TYPE FILE FILES "/home/huashu/co_605_arm/Hsc3DemoLinux/build/hsr_socket_driver/ament_cmake_environment_hooks/package.dsv")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/ament_index/resource_index/packages" TYPE FILE FILES "/home/huashu/co_605_arm/Hsc3DemoLinux/build/hsr_socket_driver/ament_cmake_index/share/ament_index/resource_index/packages/hsr_socket_driver")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/hsr_socket_driver/cmake" TYPE FILE FILES
    "/home/huashu/co_605_arm/Hsc3DemoLinux/build/hsr_socket_driver/ament_cmake_core/hsr_socket_driverConfig.cmake"
    "/home/huashu/co_605_arm/Hsc3DemoLinux/build/hsr_socket_driver/ament_cmake_core/hsr_socket_driverConfig-version.cmake"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/hsr_socket_driver" TYPE FILE FILES "/home/huashu/co_605_arm/Hsc3DemoLinux/hsr_socket_driver/package.xml")
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/huashu/co_605_arm/Hsc3DemoLinux/build/hsr_socket_driver/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
