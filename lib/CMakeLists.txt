file(GLOB_RECURSE sources *.cpp)

add_library(lyli SHARED ${sources})
target_link_libraries(lyli ${USBPP_LIBRARIES} ${OpenCV_LIBS} ${LIBUSB_LIBRARIES} ${JsonCpp_LIBRARIES} pthread ${TBB_LIBRARIES})
install(TARGETS lyli LIBRARY DESTINATION lib)

add_executable(lyli-bin main.cpp)
set_target_properties(lyli-bin PROPERTIES OUTPUT_NAME lyli)
target_link_libraries(lyli-bin ${USBPP_LIBRARIES} lyli ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS lyli-bin RUNTIME DESTINATION bin)

target_link_libraries(lyli-bin ${USBPP_LIBRARIES})
target_include_directories(lyli-bin PUBLIC ${USBPP_INCLUDE_DIRS})
target_compile_options(lyli-bin PUBLIC ${USBPP_CFLAGS_OTHER})
