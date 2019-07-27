
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
include_directories(.. ../ ../../usbpp ${OpenCV_INCLUDE_DIRS})

qt5_wrap_ui(LYLI_UI_CAMERA_SRCS cameraform.ui)
add_library(lyliuicamera SHARED
            cameracontext.cpp cameracalibrator.cpp cameraform.cpp cameralistmodel.cpp
            imagelistmodel.cpp imagelistitem.cpp imagelistdelegate.cpp imagedownloader.cpp
            ${LYLI_UI_CAMERA_SRCS})
target_link_libraries(lyliuicamera lyliuicalibration usbpp lyli Qt5::Core Qt5::Widgets)
install(TARGETS lyliuicamera LIBRARY DESTINATION lib)
