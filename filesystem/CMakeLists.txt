
add_library(lylifilesystem STATIC
	filelist.cpp filesystemaccess.cpp image.cpp imageptr.cpp photo.cpp)
add_dependencies(lylifilesystem libusbpp_project)
