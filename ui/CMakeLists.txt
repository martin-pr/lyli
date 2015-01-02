
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

include_directories(.. ../usbpp)
qt5_add_resources(LyliResources qml.qrc)
add_executable(lyli-qt main.cpp imagelistmodel.cpp thumbnailprovider.cpp ${LyliResources})
target_link_libraries(lyli-qt usbpp lyliiface Qt5::Core Qt5::Qml Qt5::Quick Qt5::Widgets)
