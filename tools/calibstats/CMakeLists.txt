add_executable(calibstats main.cpp)
target_link_libraries(calibstats lyli)
install(TARGETS calibstats RUNTIME DESTINATION bin)
