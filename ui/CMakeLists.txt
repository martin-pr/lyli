
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
include_directories(.. ../usbpp ${OpenCV_INCLUDE_DIRS})

qt5_wrap_ui(LYLI_UI_SRCS cameraform.ui viewerform.ui mainwindow.ui)
add_executable(lyli-qt main.cpp mainwindow.cpp progressbar.cpp
               cameraform.cpp cameralistmodel.cpp
               imagelistmodel.cpp imagelistitem.cpp imagelistdelegate.cpp imagedownloader.cpp
               viewerform.cpp lytroimage.cpp
               ${LYLI_UI_SRCS})
target_link_libraries(lyli-qt usbpp lyliiface Qt5::Core Qt5::Widgets)
