cmake_minimum_required(VERSION 3.0.2)
project(dvrk_moveit)

find_package(catkin REQUIRED
        COMPONENTS
        interactive_markers
        moveit_core
        moveit_visual_tools
        moveit_ros_planning
        moveit_ros_planning_interface
        moveit_ros_perception
        pluginlib
        geometric_shapes
        pcl_ros
        pcl_conversions
        tf2_ros
        tf2_eigen
        tf2_geometry_msgs
        stomp_moveit
)

catkin_package(
)

include_directories(
  include/
  ${catkin_INCLUDE_DIRS}
)

add_executable(basic_io src/basic_io.cpp)
target_link_libraries(basic_io ${catkin_LIBRARIES})

add_executable(basic_planning src/basic_planning.cpp)
target_link_libraries(basic_planning ${catkin_LIBRARIES})

add_executable(ros_planning src/ros_planning.cpp)
target_link_libraries(ros_planning ${catkin_LIBRARIES})

add_executable(move_planning src/move_planning.cpp)
target_link_libraries(move_planning ${catkin_LIBRARIES})

add_executable(go2point src/go2point.cpp)
target_link_libraries(go2point ${catkin_LIBRARIES})

add_executable(waypoints_trajectory src/waypoints_trajectory.cpp)
target_link_libraries(waypoints_trajectory ${catkin_LIBRARIES})

add_executable(spline_planning src/spline_planning.cpp src/dvrk_moveit_class.cpp)
target_link_libraries(spline_planning ${catkin_LIBRARIES})

add_executable(full_planning src/full_planning.cpp src/dvrk_moveit_class.cpp)
target_link_libraries(full_planning ${catkin_LIBRARIES})

add_executable(dvrk_planexe src/dvrk_planexe.cpp src/dvrk_moveit_class.cpp)
target_link_libraries(dvrk_planexe ${catkin_LIBRARIES})

add_executable(dvrk_test src/dvrk_test.cpp src/dvrk_moveit_class.cpp)
target_link_libraries(dvrk_test ${catkin_LIBRARIES})
