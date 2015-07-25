set(the_description "The UbiTrack Visualization Render Components")
ut_add_component(Render DEPS utcore utdataflow utvision)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake")

UT_OPTION(USE_GLFW "Use GLFW instead of GLUT" ON )

#if not have_opencv:		
#	sources.remove('BackgroundImage.cpp')
#	sources.remove('ZBufferOutput.cpp')
#	sources.remove('ImageOutput.cpp')
#	headers.remove('BackgroundImage.h')
#	headers.remove('ZBufferOutput.h')
#	headers.remove('ImageOutput.h')

#if have_lapack:
#	env.AppendUnique( **lapack_options )		
#else:
#	sources.remove('PositionErrorVisualization.cpp')
#	sources.remove('PoseErrorVisualization.cpp')
#	headers.remove('PositionErrorVisualization.h')
#	headers.remove('PoseErrorVisualization.h')
IF(USE_GLFW)
  FIND_PACKAGE(GLFW)
ENDIF(USE_GLFW)

IF(GLFW_FOUND)
    ut_component_include_directories(${UBITRACK_CORE_DEPS_INCLUDE_DIR} ${OPENCV_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR} ${GLFW_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
    ut_glob_component_sources(HEADERS "*.h" SOURCES "*.cpp" "RenderGLFW/RenderModule.cpp")
    ut_create_single_component(${OPENGL_LIBRARIES} ${GLFW_LIBRARY})
    add_definitions("-DGLEW_MX")
ELSE(GLFW_FOUND)
    ut_component_include_directories(${UBITRACK_CORE_DEPS_INCLUDE_DIR} ${OPENCV_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR} ${Freeglut_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
    ut_glob_component_sources(HEADERS "*.h" SOURCES "*.cpp" "RenderGLUT/RenderModule.cpp")
    ut_create_single_component(${OPENGL_LIBRARIES} ${Freeglut_LIBRARIES})
ENDIF(GLFW_FOUND)

