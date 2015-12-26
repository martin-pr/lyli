
add_library(lylicalib STATIC
	calibrator.cpp lensdetector.cpp calibrationdata.cpp pointgrid.cpp preprocessor.cpp fftpreprocessor.cpp) # lensfilter.cpp
target_link_libraries(lylicalib ${OpenCV_LIBS})
