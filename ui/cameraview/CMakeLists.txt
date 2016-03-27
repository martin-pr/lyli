
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
include_directories(.. ../ ../../usbpp ${OpenCV_INCLUDE_DIRS})

qt5_wrap_ui(LYLI_UI_CAMERA_SRCS cameraform.ui)
add_library(lyliuicamera STATIC
            cameracontext.cpp calibrationchecker.cpp cameraform.cpp cameralistmodel.cpp
            imagelistmodel.cpp imagelistitem.cpp imagelistdelegate.cpp imagedownloader.cpp
            ${LYLI_UI_CAMERA_SRCS})
target_link_libraries(lyliuicamera lyliuicalibration usbpp lyliiface Qt5::Core Qt5::Widgets)
