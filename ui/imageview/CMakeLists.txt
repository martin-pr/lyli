
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
include_directories(.. ../ ${OpenCV_INCLUDE_DIRS})

qt5_wrap_ui(LYLI_UI_IMAGE_SRCS viewerform.ui)
add_library(lyliuiimage STATIC
               viewerform.cpp lytroimage.cpp
               ${LYLI_UI_IMAGE_SRCS})
target_link_libraries(lyliuiimage lyliiface Qt5::Core Qt5::Widgets)
