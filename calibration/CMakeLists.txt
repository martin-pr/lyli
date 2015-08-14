
add_library(lylicalib STATIC
	calibrator.cpp lensdetector.cpp linegrid.cpp preprocessor.cpp) # lensfilter.cpp
target_link_libraries(lylicalib ${OpenCV_LIBS})
