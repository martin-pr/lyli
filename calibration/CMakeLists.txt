
add_library(lylicalib STATIC
	pointgrid.cpp linegrid.cpp gridmapper.cpp gridmath.cpp
	calibrator.cpp lensdetector.cpp calibrationdata.cpp preprocessor.cpp fftpreprocessor.cpp serialization.cpp) # lensfilter.cpp
target_link_libraries(lylicalib ${OpenCV_LIBS} ${TBB_LIBRARIES})
