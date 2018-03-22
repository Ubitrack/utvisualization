#ifndef __UBITRACK_VISUALIZATION_CONFIG_H_INCLUDED__
#define __UBITRACK_VISUALIZATION_CONFIG_H_INCLUDED__
#define UBITRACK_COMPONENTS_PATH "${UBITRACK_COMPONENT_INSTALL_PATH}"
#define UBITRACK_COMPONENTS_RELAVIVEPATH "${UBITRACK_COMPONENT_INSTALL_DIRECTORY}"

#ifdef _WIN32
// fix for simplified build scripts (ulrich eck)
#   ifdef UTVISUALIZATION_DLL
#       define UBITRACK_DLL
#   endif
#	ifdef UBITRACK_DLL
#		define UBITRACK_EXPORT __declspec( dllexport )
#	else
#		define UBITRACK_EXPORT __declspec( dllimport )
#	endif
#else // _WIN32
#	define UBITRACK_EXPORT
#endif

#ifndef HAVE_LAPACK 
#cmakedefine HAVE_LAPACK
#endif
#ifndef HAVE_OPENCV 
#cmakedefine HAVE_OPENCV
#endif
#endif
