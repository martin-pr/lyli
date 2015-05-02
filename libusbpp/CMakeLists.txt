cmake_minimum_required(VERSION 2.8.4)
project(usbpp CXX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0>=1.0.0)

set(CMAKE_CXX_FLAGS "-std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wstrict-aliasing -Wextra")

include_directories(${LIBUSB_INCLUDE_DIRS})
add_library(usbpp STATIC
	context.cpp device.cpp # basic libusb wrapper
	hiddevice.cpp hidreport.cpp # HID support
	mscbw.cpp mscsw.cpp msdevice.cpp msscsiinquiry.cpp msscsiinquiryresponse.cpp # mass storage
)
target_link_libraries(usbpp ${LIBUSB_LIBRARIES})

add_executable(testhid testhid.cpp)
target_link_libraries(testhid usbpp ${LIBUSB_LIBRARIES})
