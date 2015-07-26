SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

UT_OPTION(USE_RENDERAPI "Use RenderAPI instead of GLUT" OFF )

add_subdirectory(src/utVisualization/OpenCV)
add_subdirectory(src/utVisualization/Render)

IF(USE_RENDERAPI)
    add_subdirectory(src/utVisualization/RenderAPI)
    add_subdirectory(apps/GLFWConsole)
ENDIF(USE_RENDERAPI)

ut_install_utql_patterns()