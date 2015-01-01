project(usbpp CXX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0>=1.0.0)

set(CMAKE_CXX_FLAGS "-std=c++11")

include_directories(${LIBUSB_INCLUDE_DIRS})
add_library(usbpp STATIC context.cpp device.cpp cbw.cpp csw.cpp msdevice.cpp scsiinquiry.cpp scsiinquiryresponse.cpp)
target_link_libraries(usbpp ${LIBUSB_LIBRARIES})
