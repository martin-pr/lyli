add_subdirectory(calibration)
add_subdirectory(filesystem)
add_subdirectory(image)

add_library(lyliiface SHARED camera.cpp context.cpp)
target_link_libraries(lyliiface lylicalib lylifilesystem lyliimage ${USBPP_LIBRARIES} ${OpenCV_LIBS} ${LIBUSB_LIBRARIES} ${JsonCpp_LIBRARIES} pthread)
install(TARGETS lyliiface LIBRARY DESTINATION lib)

add_executable(lyli main.cpp)
target_link_libraries(lyli ${USBPP_LIBRARIES} lyliiface ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS lyli RUNTIME DESTINATION bin)

target_link_libraries(lyli ${USBPP_LIBRARIES})
target_include_directories(lyli PUBLIC ${USBPP_INCLUDE_DIRS})
target_compile_options(lyli PUBLIC ${USBPP_CFLAGS_OTHER})
