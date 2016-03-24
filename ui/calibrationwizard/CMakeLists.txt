
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
include_directories(.. ../ ${OpenCV_INCLUDE_DIRS})

qt5_wrap_ui(LYLI_UI_CALIBRATION_SRCS pagestart.ui pagecachedir.ui pagefinish.ui)
add_library(lyliuicalibration STATIC
            calibrationwizard.cpp
            ${LYLI_UI_CALIBRATION_SRCS})
target_link_libraries(lyliuicalibration lyliiface Qt5::Core Qt5::Widgets)
