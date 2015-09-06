
add_library(lyliimage STATIC
	metadata.cpp rawimage.cpp)
target_link_libraries(lyliimage ${OpenCV_LIBS})

