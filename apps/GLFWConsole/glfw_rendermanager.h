//
// Created by Ulrich Eck on 26/07/2015.
//

#ifndef UBITACK_GLFW_RENDERMANAGER_H
#define UBITACK_GLFW_RENDERMANAGER_H

#include <string>

#ifdef HAVE_GLEW
	#include "GL/glew.h"
#endif
#include <GLFW/glfw3.h>

#include <utVisualization/utRenderAPI.h>

namespace Ubitrack {
    namespace Visualization {

        class GLFWWindowImpl : public VirtualWindow {

        public:
            GLFWWindowImpl(int _width, int _height, const std::string& _title);
            ~GLFWWindowImpl();

            virtual void pre_render();
            virtual void post_render();

            // Implementation of Public interface
            virtual bool is_valid();
            virtual bool create();
            virtual void initGL(boost::shared_ptr<CameraHandle>& cam);
            virtual void destroy();

        private:
            GLFWwindow*	m_pWindow;
            boost::shared_ptr<CameraHandle> m_pEventHandler;
        };

        // callback implementations for GLFW
        inline static void WindowResizeCallback(
                GLFWwindow *win,
                int w,
                int h) {
            CameraHandle *cam = static_cast<CameraHandle*>(glfwGetWindowUserPointer(win));
            cam->on_window_size(w, h);
        }

        inline static void WindowRefreshCallback(GLFWwindow *win) {
            CameraHandle *cam = static_cast<CameraHandle*>(glfwGetWindowUserPointer(win));
            cam->on_render(glfwGetTime());
        }

        inline static void WindowCloseCallback(GLFWwindow *win) {
            CameraHandle *cam = static_cast<CameraHandle*>(glfwGetWindowUserPointer(win));
            cam->on_window_close();
        }

        inline static void WindowKeyCallback(GLFWwindow *win,
                                             int key,
                                             int scancode,
                                             int action,
                                             int mods) {
            CameraHandle *cam = static_cast<CameraHandle*>(glfwGetWindowUserPointer(win));
//            if ((mods & GLFW_MOD_ALT ) && (action == GLFW_PRESS)) {
//                switch(key) {
//                case GLFW_KEY_F:
//                    if (m_isFullscreen) {
//
//                    }
//                    GLFWmonitor* monitor = glfwGetWindowMonitor(win);
//                    const GLFWvidmode* wmode = glfwGetVideoMode(monitor);
//                    glfwSetWindowMonitor(win, monitor, 0, 0, wmode->width, wmode->height, wmode->refreshRate);
//                    break;
//                case GLFW_KEY_Q:
//                    break;
//                default:
//                    // should convert from GLFW to some common format here ..
//                    cam->on_keypress(key, scancode, action, mods);
//                }
//
//            }
            /** accessing the alt-modifier needs a change in the function signature.
            if (glutGetModifiers() & GLUT_ACTIVE_ALT)
            {
                switch ( key )
                {
                    case 'f':
                        #ifdef	_WIN32
                            // need to work around freeglut for multi-monitor fullscreen
                            makeWindowFullscreen( m_moduleKey, Math::Vector< int, 2 >( 0xFFFF, 0xFFFF ) );
                        #else
        //					glutFullScreen();
                        #endif
                        break;
                    case 'i': m_info = !m_info; break;
                    case 'v': m_doSync = !m_doSync; break;
                    case 's': m_parity = !m_parity; break;
                    // case 'q': delete this; break;
                }
            }
            else
            **/
            cam->on_keypress(key, scancode, action, mods);
        }


        inline static void WindowCursorPosCallback(GLFWwindow *win,
                                             double xpos,
                                             double ypos) {
            CameraHandle *cam = static_cast<CameraHandle*>(glfwGetWindowUserPointer(win));
            cam->on_cursorpos(xpos, ypos);
        }

    }
}

#endif //UBITACK_GLFW_RENDERMANAGER_H
