# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# FindGLEW
# --------
#
# Find the OpenGL Extension Wrangler Library (GLEW)
#
# IMPORTED Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the :prop_tgt:`IMPORTED` target ``GLEW::GLEW``,
# if GLEW has been found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module defines the following variables:
#
# ::
#
#   GLEW_INCLUDE_DIRS - include directories for GLEW
#   GLEW_LIBRARIES - libraries to link against GLEW
#   GLEW_FOUND - true if GLEW has been found and can be used


macro( select_library_configurations basename )
    if(NOT ${basename}_LIBRARY_RELEASE)
        set(${basename}_LIBRARY_RELEASE "${basename}_LIBRARY_RELEASE-NOTFOUND" CACHE FILEPATH "Path to a library.")
    endif()
    if(NOT ${basename}_LIBRARY_DEBUG)
        set(${basename}_LIBRARY_DEBUG "${basename}_LIBRARY_DEBUG-NOTFOUND" CACHE FILEPATH "Path to a library.")
    endif()

    if( ${basename}_LIBRARY_DEBUG AND ${basename}_LIBRARY_RELEASE AND
           NOT ${basename}_LIBRARY_DEBUG STREQUAL ${basename}_LIBRARY_RELEASE AND
           ( CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE ) )
        # if the generator supports configuration types or CMAKE_BUILD_TYPE
        # is set, then set optimized and debug options.
        set( ${basename}_LIBRARY "" )
        foreach( _libname IN LISTS ${basename}_LIBRARY_RELEASE )
            list( APPEND ${basename}_LIBRARY optimized "${_libname}" )
        endforeach()
        foreach( _libname IN LISTS ${basename}_LIBRARY_DEBUG )
            list( APPEND ${basename}_LIBRARY debug "${_libname}" )
        endforeach()
    elseif( ${basename}_LIBRARY_RELEASE )
        set( ${basename}_LIBRARY ${${basename}_LIBRARY_RELEASE} )
    elseif( ${basename}_LIBRARY_DEBUG )
        set( ${basename}_LIBRARY ${${basename}_LIBRARY_DEBUG} )
    else()
        set( ${basename}_LIBRARY "${basename}_LIBRARY-NOTFOUND")
    endif()

    set( ${basename}_LIBRARIES "${${basename}_LIBRARY}" )

    if( ${basename}_LIBRARY )
        set( ${basename}_FOUND TRUE )
    endif()

    mark_as_advanced( ${basename}_LIBRARY_RELEASE
        ${basename}_LIBRARY_DEBUG
    )
endmacro()


find_path(GLEW_INCLUDE_DIR NAMES GL/glew.h
    HINTS
    $ENV{GLEWDIR}/include
    ${OPENGL_INCLUDE_DIR}
    "${EXTERNAL_LIBRARIES_DIR}/GLEW/include"
    /usr/include
    /usr/local/include
    /usr/include/GL
    /sw/include
    /opt/local/include
    /opt/graphics/OpenGL/include
    /opt/graphics/OpenGL/contrib/libglew
    DOC "The directory where GL/glew.h resides"
)

if(NOT GLEW_LIBRARY)
  find_library(GLEW_LIBRARY_RELEASE NAMES GLEW glew32 glew glew32s PATH_SUFFIXES lib64 lib
    PATHS
    ${GLEW_LIBRARY_DIR}  # provided by glew config
    $ENV{GLEWDIR}
    "${EXTERNAL_LIBRARIES_DIR}/glew"
    /usr
    /usr/local
    /sw
    /opt/local
    DOC "The GLEW library")

  find_library(GLEW_LIBRARY_DEBUG NAMES GLEWd glew32d glewd PATH_SUFFIXES lib64 lib
    PATHS
    ${GLEW_LIBRARY_DIR}  # provided by glew config
    $ENV{GLEWDIR}
    "${EXTERNAL_LIBRARIES_DIR}/glew"
    /usr
    /usr/local
    /sw
    /opt/local
    DOC "The GLEW library")

  # include(SelectLibraryConfigurations.cmake)
  select_library_configurations(GLEW)
endif ()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLEW
                                  REQUIRED_VARS GLEW_INCLUDE_DIR GLEW_LIBRARY)

if(GLEW_FOUND)
  set(GLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIR})

  if(NOT GLEW_LIBRARIES)
    set(GLEW_LIBRARIES ${GLEW_LIBRARY})
  endif()

  if (NOT TARGET GLEW::GLEW)
    add_library(GLEW::GLEW UNKNOWN IMPORTED)
    set_target_properties(GLEW::GLEW PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${GLEW_INCLUDE_DIRS}")

    if(GLEW_LIBRARY_RELEASE)
      set_property(TARGET GLEW::GLEW APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
      set_target_properties(GLEW::GLEW PROPERTIES IMPORTED_LOCATION_RELEASE "${GLEW_LIBRARY_RELEASE}")
    endif()

    if(GLEW_LIBRARY_DEBUG)
      set_property(TARGET GLEW::GLEW APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
      set_target_properties(GLEW::GLEW PROPERTIES IMPORTED_LOCATION_DEBUG "${GLEW_LIBRARY_DEBUG}")
    endif()

    if(NOT GLEW_LIBRARY_RELEASE AND NOT GLEW_LIBRARY_DEBUG)
      set_property(TARGET GLEW::GLEW APPEND PROPERTY IMPORTED_LOCATION "${GLEW_LIBRARY}")
    endif()
  endif()
endif()

mark_as_advanced(GLEW_INCLUDE_DIR)
