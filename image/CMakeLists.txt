
add_library(lyliimage STATIC
	rawimage.cpp)
target_link_libraries(lyliimage ${OpenCV_LIBS})

