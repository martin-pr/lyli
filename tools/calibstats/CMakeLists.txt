add_executable(calibstats main.cpp)
target_link_libraries(calibstats lyliiface)
install(TARGETS calibstats RUNTIME DESTINATION bin)
