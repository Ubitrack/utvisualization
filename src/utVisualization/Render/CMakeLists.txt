set(the_description "The UbiTrack Visualization Render Components")

ut_add_component(Render DEPS utcore utdataflow utvision utrenderapi)
ut_component_include_directories(${UBITRACK_CORE_DEPS_INCLUDE_DIR} ${OPENCV_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/../../")
ut_glob_component_sources(HEADERS "*.h" SOURCES "*.cpp")
ut_create_single_component(${OPENGL_LIBRARIES})

