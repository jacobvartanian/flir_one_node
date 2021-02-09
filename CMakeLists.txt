cmake_minimum_required(VERSION 2.8.3)
project(flir_one_node)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
  image_transport
  roscpp
  rospy
  sensor_msgs
  std_msgs
  cv_bridge
)

## System dependencies are found with CMake's conventions
find_package(Boost REQUIRED COMPONENTS system)
find_package(PkgConfig REQUIRED)

pkg_search_module(LIBUSB1 REQUIRED libusb-1.0)
include_directories(SYSTEM ${LIBUSB1_INCLUDE_DIRS})

###################################
## catkin specific configuration ##
###################################
## The catkin_package macro generates cmake config files for your package
## Declare things to be passed to dependent projects
## INCLUDE_DIRS: uncomment this if you package contains header files
## LIBRARIES: libraries you create in this project that dependent projects also need
## CATKIN_DEPENDS: catkin_packages dependent projects also need
## DEPENDS: system dependencies of this project that dependent projects also need
catkin_package(
  INCLUDE_DIRS include
  LIBRARIES flir_one_node
  CATKIN_DEPENDS image_transport roscpp rospy sensor_msgs std_msgs
  DEPENDS system_lib
)

###########
## Build ##
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(include
  ${catkin_INCLUDE_DIRS}
)

## Declare a C++ executable
add_executable(flir_one_node src/flir_one_node.cpp src/control.cpp src/driver_flir.cpp src/usb_packet_driver.cpp)

## Add cmake target dependencies of the executable
add_dependencies(flir_one_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})

target_link_libraries(${PROJECT_NAME}
 ${catkin_LIBRARIES}
 #libusb-1.0
 usb-1.0
)