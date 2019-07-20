
add_library(lyliimage SHARED
	lightfieldimage.cpp metadata.cpp rawimage.cpp)
target_link_libraries(lyliimage ${OpenCV_LIBS})

