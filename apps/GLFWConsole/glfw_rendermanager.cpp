//
// Created by Ulrich Eck on 26/07/2015.
//

#include "glfw_rendermanager.h"

using namespace Ubitrack::Visualization;


GLFWWindowImpl::GLFWWindowImpl(int _width, int _height, const std::string &_title)
        : VirtualWindow(_width, _height, _title), m_pWindow(NULL) {

}

bool GLFWWindowImpl::is_valid() {
    // maybe more checks.
    return (m_pWindow != NULL ) && (!glfwWindowShouldClose(m_pWindow));
}

bool GLFWWindowImpl::create() {
    m_pWindow = glfwCreateWindow(m_width, m_height, m_title.c_str(), NULL, NULL);
    // set fullscreen ?
    return m_pWindow != NULL;
}

void GLFWWindowImpl::initGL() {
    // log error ??
    if (m_pWindow == NULL)
        return;

#ifdef HAVE_GLEW
    // Init GLEW for this context:
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        // a problem occured when trying to init glew, report it:
        std::cout << "GLEW Error occured, Description: " <<  glewGetErrorString(err) << std::endl;
        glfwDestroyWindow(m_pWindow);
        return;
    }
#endif


    // GL: enable and set colors
    glEnable(GL_COLOR_MATERIAL);
    glClearColor(0.0, 0.0, 0.0, 1.0); // TODO: make this configurable (but black is best for optical see-through ar!)

    // GL: enable and set depth parameters
    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0);

    // GL: disable backface culling
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);

    // GL: light parameters
    GLfloat light_pos[] = {1.0f, 1.0f, 1.0f, 0.0f};
    GLfloat light_amb[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat light_dif[] = {0.9f, 0.9f, 0.9f, 1.0f};

    // GL: enable lighting
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_dif);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // GL: bitmap handling
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // GL: alpha blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    // GL: misc stuff
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);

    // setup callbacks:
    glfwSetWindowSizeCallback(m_pWindow, g_reshape);
    glfwSetKeyCallback(m_pWindow, g_keyboard);
}

void GLFWWindowImpl::destroy() {
    if (m_pWindow != NULL) {
        glfwDestroyWindow(m_pWindow);
        m_pWindow = NULL;
    }
}