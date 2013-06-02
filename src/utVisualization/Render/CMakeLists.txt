set(the_description "The UbiTrack Visualization Render Components")
ut_add_component(Render DEPS utcore utdataflow utvision)
ut_component_include_directories(${TINYXML_INCLUDE_DIR} ${LOG4CPP_INCLUDE_DIR} ${BOOSTBINDINGS_INCLUDE_DIR} ${LAPACK_INCLUDE_DIR}  ${Boost_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR} ${Freeglut_INCLUDE_DIR})

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

ut_glob_component_sources(HEADERS "*.h" SOURCES "*.cpp")
ut_create_single_component(${OPENGL_LIBRARIES} ${Freeglut_LIBRARIES})