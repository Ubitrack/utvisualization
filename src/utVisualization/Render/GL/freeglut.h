#ifdef __APPLE__
	#include <GLUT/glut.h>
	#define glutSetOption(a,b)
	#define glutMainLoopEvent glutCheckLoop
#else
	#include <GL/freeglut.h>
#endif
