
include_directories(.. ../ ../../usbpp ${OpenCV_INCLUDE_DIRS})

add_library(lyliuiconfig STATIC
            lyliconfig.cpp)
target_link_libraries(lyliuiconfig lyliuicalibration usbpp lyliiface Qt5::Core Qt5::Widgets)
