cmake_minimum_required(VERSION 2.8...3.27.6)


PROJECT(vtktp)

add_definitions(-std=c++17)
set(CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS, "${CXX_FLAGS}")
set(CMAKE_BUILD_TYPE Debug)


IF(APPLE)

SET( OperatingSystem "Mac os X")
set (MY_DATA_PATH  /home/clement/Documents/BigData/canevas_devoir/)


find_package(VTK 9 REQUIRED)

ENDIF (APPLE)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

SET(OperatingSystem "Linux")

set (MY_DATA_PATH /home/clement/Documents/BigData/canevas_devoir/)

find_package(VTK)
include(${VTK_USE_FILE})

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")


MESSAGE(WARNING "System detected:${OperatingSystem}" )
MESSAGE(WARNING "Path to data: ${MY_DATA_PATH}" )


configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
  )
 

file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

include_directories("${PROJECT_BINARY_DIR}")

add_executable(${PROJECT_NAME} ${SRC_LIST})

 
target_link_libraries(${PROJECT_NAME} ${VTK_LIBRARIES})


enable_testing()
add_test(SimpleTest ${PROJECT_NAME})
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}
                  DEPENDS ${PROJECT_NAME})
