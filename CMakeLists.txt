cmake_minimum_required(VERSION 2.8.4)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules)
project(lyli CXX)

set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wstrict-aliasing -Wextra")

################
# Dependencies #
################

# GUI (optional)
find_package(Qt5Core)
find_package(Qt5Widgets)

# minimal dependencies
find_package(JsonCpp REQUIRED 1.4.0)
find_package(OpenCV REQUIRED)
find_package(TBB REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0>=1.0.0)

# libusbpp dependency - if it doesn't exists, clone it
include(ExternalProject)
ExternalProject_Add(libusbpp_project
	HG_REPOSITORY https://bitbucket.org/stativ/libusbpp
	PREFIX "libusbpp"
	CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/libusbpp/install" "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
)
add_library(usbpp STATIC IMPORTED)
set_property(TARGET usbpp PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/libusbpp/install/lib/libusbpp.a)
include_directories("${CMAKE_BINARY_DIR}/libusbpp/install/include")

##################
# Compiler flags #
##################

set(CMAKE_CXX_FLAGS "-std=c++14")
add_definitions(${TBB_DEFINITIONS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${JsonCpp_INCLUDE_DIRS})
include_directories(${TBB_INCLUDE_DIRS})
include_directories(${LIBUSB_INCLUDE_DIRS})

#########
# Build #
#########

add_subdirectory(calibration)
add_subdirectory(filesystem)
add_subdirectory(image)

add_library(lyliiface STATIC camera.cpp)
target_link_libraries(lyliiface lylicalib lylifilesystem lyliimage usbpp ${OpenCV_LIBS} ${LIBUSB_LIBRARIES} ${JsonCpp_LIBRARIES})
add_dependencies(lyliiface libusbpp_project)

add_executable(lyli main.cpp)
target_link_libraries(lyli usbpp lyliiface)

if (Qt5Core_FOUND AND Qt5Widgets_FOUND)
	add_subdirectory(ui)
endif()

# aditional tools
add_subdirectory(tools)
