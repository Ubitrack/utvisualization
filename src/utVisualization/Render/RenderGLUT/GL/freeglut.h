#ifdef __APPLE__
	#include <GL/freeglut.h>
	//#include <GLUT/glut.h>
	#define glutSetOption(a,b)
	#define glutMainLoopEvent glutCheckLoop
#else
	#include <GL/freeglut.h>
#endif
