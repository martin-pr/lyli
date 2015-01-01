
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

include_directories(.. ../usbpp)
qt5_add_resources(LitroResources qml.qrc)
add_executable(litro-qt main.cpp imagelistmodel.cpp ${LitroResources})
target_link_libraries(litro-qt usbpp litroiface Qt5::Core Qt5::Qml Qt5::Quick Qt5::Widgets)
