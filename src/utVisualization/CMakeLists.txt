set(the_description "The UbiTrack Visualization Library")
ut_add_module(utvisualization utcore utdataflow utvision)
ut_module_include_directories(utvisualization ${UBITRACK_CORE_DEPS_INCLUDE_DIR} ${OPENCV_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/../")
ut_glob_module_sources(HEADERS "*.h" SOURCES "*.cpp")

# writes config file into binary_dir, so sourcetree will not be cluttered
configure_file(Config_cmake.hh ${CMAKE_CURRENT_SOURCE_DIR}/Config.h)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/Config.h" DESTINATION "${UBITRACK_INCLUDE_INSTALL_PATH}/utVisualization" COMPONENT main)

ut_create_module(${OPENGL_LIBRARIES})
