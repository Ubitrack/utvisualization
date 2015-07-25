SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_subdirectory(src/utVisualization/OpenCV)
add_subdirectory(src/utVisualization/Render)

ut_install_utql_patterns()