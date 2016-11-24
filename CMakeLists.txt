SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_subdirectory(src/utVisualization)
add_subdirectory(apps/GLFWConsole)
ut_install_utql_patterns()