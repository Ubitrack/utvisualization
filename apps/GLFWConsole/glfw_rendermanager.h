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

#include <utVisualization/RenderAPI/RenderAPI.h>

namespace Ubitrack {
    namespace Visualization {

        class GLFWWindowImpl : VirtualWindow {

        public:
            GLFWWindowImpl(int _width, int _height, const std::string& _title);
            ~GLFWWindowImpl();

            // virtual callbacks for implementation
            virtual void on_window_size(int w, int h);
            virtual void on_window_close();
            virtual void on_keypress(int key, int scancode, int action, int mods);



            virtual void on_render();


            // Implementation of Public interface
            virtual void create();
            virtual void initGL();


        private:
            GLFWwindow*	m_pWindow;

        };
    }
}

#endif //UBITACK_GLFW_RENDERMANAGER_H
