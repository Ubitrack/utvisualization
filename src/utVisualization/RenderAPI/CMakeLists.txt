set(the_description "The UbiTrack RenderAPI")
ut_add_module(utrenderapi utcore utdataflow utvision)
ut_module_include_directories(utrenderapi ${UBITRACK_CORE_DEPS_INCLUDE_DIR} ${OPENCV_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR})
ut_glob_module_sources(HEADERS "*.h" SOURCES "*.cpp")

# writes config file into binary_dir, so sourcetree will not be cluttered
configure_file(Config_cmake.hh ${CMAKE_CURRENT_SOURCE_DIR}/Config.h)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/Config.h" DESTINATION "${UBITRACK_INCLUDE_INSTALL_PATH}/utVisualization/RenderAPI" COMPONENT main)

ut_create_module(${OPENGL_LIBRARIES})
