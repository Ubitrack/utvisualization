/*
 * Ubitrack - Library for Ubiquitous Tracking
 * Copyright 2006, Technische Universitaet Muenchen, and individual
 * contributors as indicated by the @authors tag. See the 
 * copyright.txt in the distribution for a full listing of individual
 * contributors.
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA, or see the FSF site: http://www.fsf.org.
 */

/**
 * @file
 * RenderAPI Class of the ubitrack visualization library
 *
 * @author Ulrich Eck <ulrich.eck@tum.de>
 */

#ifndef UBITRACK_UTRENDERAPI_H
#define UBITRACK_UTRENDERAPI_H

#include <utVisualization.h>

#include <string>
#include <map>
#include <deque>
#include <functional>
#include <memory>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>


namespace Ubitrack {
    namespace Drivers {
        class VirtualCamera;
    }
    namespace Visualization {

        class CameraHandle;

        class UBITRACK_EXPORT VirtualWindow {

        public:
            VirtualWindow(int _width, int _height, const std::string& _title);
            ~VirtualWindow();

            virtual bool is_valid();
            virtual bool create();
            virtual void initGL(boost::shared_ptr<CameraHandle>& cam);
            virtual void destroy();

            virtual void reshape( int w, int h);

			// custom extensions
			virtual void setFullscreen(bool fullscreen);
			virtual void onExit();

            //virtual void post_redraw();

            int width() {
                return m_width;
            }

            int height() {
                return m_height;
            }

            std::string& title() {
                return m_title;
            }

        protected:
            int m_width;
            int m_height;
            std::string m_title;

        };


        class UBITRACK_EXPORT CameraHandle {

        public:
            CameraHandle(std::string& _name, int _width, int _height, Drivers::VirtualCamera* _handle);
            ~CameraHandle();

            bool need_setup();
            virtual bool setup(boost::shared_ptr<VirtualWindow>& window);
            boost::shared_ptr<VirtualWindow> get_window();
            virtual void teardown();

            /** render GL context, called from main GL thread _only_ */
            virtual void render(int ellapsed_time);


            // virtual callbacks for implementation
            virtual void on_window_size(int w, int h);
            virtual void on_window_close();
            virtual void on_render(int ellapsed_time);
            virtual int on_keypress(int key, int scancode, int action, int mods);
            virtual int on_cursorpos(double xpos, double ypos);

			// extended commands from frontend
			virtual void on_fullscreen();
			virtual void on_exit();


            // maybe needed for external renderloop synchronization
            virtual void post_redraw();

            /** keyboard callback - legacy of ubitrack rendermodule */
            virtual void keyboard( unsigned char key, int x, int y );


            int initial_width() {
                return m_initial_width;
            }

            int initial_height() {
                return m_initial_height;
            }

            std::string& title() {
                return m_sWindowName;
            }

        protected:
            int m_initial_width;
            int m_initial_height;
            boost::shared_ptr<VirtualWindow> m_pVirtualWindow;

            std::string m_sWindowName;
            bool m_bSetupNeeded;
			bool m_bIsFullScreen;
            Drivers::VirtualCamera* m_pVirtualCamera;
        };


        typedef std::map< unsigned int, boost::shared_ptr<CameraHandle> > CameraHandleMap;



        class UBITRACK_EXPORT RenderManager {

        public:

            typedef std::function<void()> CallbackType;

            RenderManager();
            ~RenderManager();

			void setSharedOpenGLContext(void* ctx);
			void* getSharedOpenGLContext();

            bool need_setup();
            boost::shared_ptr<CameraHandle> setup_pop_front();
            void setup_push_back(boost::shared_ptr<CameraHandle>& handle);

            CameraHandleMap::iterator cameras_begin();
            CameraHandleMap::iterator cameras_end();

            unsigned int camera_count();
            boost::shared_ptr<CameraHandle> get_camera(unsigned int cam_id);

            unsigned int register_camera(boost::shared_ptr<CameraHandle>& handle);
            void unregister_camera(unsigned int cam_id);
            void setup();
            bool any_windows_valid();
            void teardown();

            void notify_ready();
            bool wait_for_event(int timeout);
            void register_notify_callback(CallbackType cb);
            void unregister_notify_callback();



            /** get the main rendermanager object */
            static RenderManager& singleton();

        private:

			CameraHandleMap m_mRegisteredCameras;
            std::deque< boost::shared_ptr<CameraHandle> > m_mCamerasNeedSetup;
            boost::mutex g_globalMutex;
            boost::condition g_continue;
            unsigned int m_iCameraCount;
            boost::mutex m_mutex;
			CallbackType m_notification_slot;

			void* m_sharedOpenGLContext;

        };

    };
};


#endif //UBITRACK_UTRENDERAPI_H
