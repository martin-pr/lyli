# pkg-config to find libusbpp-1.0
find_package(PkgConfig REQUIRED)
pkg_search_module(USBPP REQUIRED usbpp)

add_library(lylifilesystem STATIC
	filelist.cpp filesystemaccess.cpp image.cpp imageptr.cpp photo.cpp)

target_link_libraries(lylifilesystem ${USBPP_LIBRARIES})
target_include_directories(lylifilesystem PUBLIC ${USBPP_INCLUDE_DIRS})
target_compile_options(lylifilesystem PUBLIC ${USBPP_CFLAGS_OTHER})
