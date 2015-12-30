
add_library(lylicalib STATIC
	pointgrid.cpp linegrid.cpp subgrid.cpp gridmapper.cpp
	calibrator.cpp lensdetector.cpp calibrationdata.cpp preprocessor.cpp fftpreprocessor.cpp) # lensfilter.cpp
target_link_libraries(lylicalib ${OpenCV_LIBS})
