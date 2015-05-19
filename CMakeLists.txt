cmake_minimum_required(VERSION 2.8.4)
project(lyli CXX)

set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wstrict-aliasing -Wextra")

################
# Dependencies #
################

find_package(Qt5Core)
find_package(Qt5Widgets)

find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0>=1.0.0)

# libusbpp dependency - if it doesn't exists, clone it
include(ExternalProject)
ExternalProject_Add(libusbpp_project
	HG_REPOSITORY https://bitbucket.org/stativ/libusbpp
	PREFIX "libusbpp"
	CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/libusbpp/install"
)
add_library(usbpp STATIC IMPORTED)
set_property(TARGET usbpp PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/libusbpp/install/lib/libusbpp.a)
include_directories("${CMAKE_BINARY_DIR}/libusbpp/install/include")

##################
# Compiler flags #
##################

set(CMAKE_CXX_FLAGS "-std=c++14")
include_directories(${LIBUSB_INCLUDE_DIRS})

#########
# Build #
#########

add_library(lyliiface STATIC camera.cpp filelist.cpp filelistparser.cpp rawimage.cpp calibrator.cpp)
target_link_libraries(lyliiface usbpp ${OpenCV_LIBS} ${LIBUSB_LIBRARIES})
add_dependencies(lyliiface libusbpp_project)

add_executable(lyli main.cpp)
target_link_libraries(lyli usbpp lyliiface)

if (Qt5Core_FOUND AND Qt5Widgets_FOUND)
	add_subdirectory(ui)
endif()
