cmake_minimum_required(VERSION 2.8.4)
project(litro CXX)

################
# Dependencies #
################

find_package(Qt5Core)
find_package(Qt5Qml)
find_package(Qt5Quick)
find_package(Qt5Widgets)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0>=1.0.0)

##################
# Compiler flags #
##################

set(CMAKE_CXX_FLAGS "-std=c++14")
include_directories(${LIBUSB_INCLUDE_DIRS})

#########
# Build #
#########

add_subdirectory(usbpp)

add_library(litroiface STATIC camera.cpp filelistparser.cpp)
target_link_libraries(litroiface usbpp)

add_executable(litro main.cpp)
target_link_libraries(litro usbpp litroiface)

if (Qt5Core_FOUND AND Qt5Qml_FOUND AND Qt5Quick_FOUND AND Qt5Widgets_FOUND)
	add_subdirectory(ui)
endif()
