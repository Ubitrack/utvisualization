set(the_description "The UbiTrack utGLFWConsole app")
ut_add_app(utGLFWConsole DEPS utcore utdataflow utfacade utvision utvisualization)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")

FIND_PACKAGE(GLFW)
IF(GLFW_FOUND)
ut_app_include_directories(${UBITRACK_CORE_DEPS_INCLUDE_DIR} ${OPENCV_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR} ${GLFW_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/../../src")
ut_glob_app_sources(SOURCES "glfw_*.cpp")
ut_create_executable(${PTHREAD_LIBRARIES} ${OPENGL_LIBRARIES} ${GLFW_LIBRARY})
ENDIF(GLFW_FOUND)
#set_target_properties(${the_target} PROPERTIES LINK_FLAGS "/NODEFAULTLIB:atlthunk.lib /NODEFAULTLIB:atlsd.lib /DEBUG")
# ut_module_include_directories(../../src ${CMAKE_CURRENT_BINARY_DIR}/../../src)

