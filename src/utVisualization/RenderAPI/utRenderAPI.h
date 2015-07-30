//
// Created by Ulrich Eck on 26/07/2015.
//

#ifndef UBITACK_UTRENDERAPI_H
#define UBITACK_UTRENDERAPI_H

#include <string>
#include <map>
#include <deque>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>

#include <utVisualization/RenderAPI/Config.h>

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
            virtual void on_keypress(int key, int scancode, int action, int mods);
            virtual void on_cursorpos(double xpos, double ypos);

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
            Drivers::VirtualCamera* m_pVirtualCamera;
        };


        typedef std::map< unsigned int, boost::shared_ptr<CameraHandle> > CameraHandleMap;

        // XXX should be singleton..
        class UBITRACK_EXPORT RenderManager {

        public:
            RenderManager();
            ~RenderManager();

            bool need_setup();
            boost::shared_ptr<CameraHandle> setup_pop_front();
            void setup_push_back(boost::shared_ptr<CameraHandle>& handle);

            CameraHandleMap::iterator cameras_begin();
            CameraHandleMap::iterator cameras_end();

            unsigned int camera_count();
            boost::shared_ptr<CameraHandle> get_camera(unsigned int cam_id);

            virtual unsigned int register_camera(boost::shared_ptr<CameraHandle>& handle);
            virtual void unregister_camera(unsigned int cam_id);
            virtual void setup();
            virtual bool any_windows_valid();
            virtual void teardown();

            void notify_ready();
            virtual bool wait_for_event(int timeout);

            /** get the main rendermanager object */
            static RenderManager& singleton();

        private:
            CameraHandleMap m_mRegisteredCameras;
            std::deque< boost::shared_ptr<CameraHandle> > m_mCamerasNeedSetup;
            boost::mutex g_globalMutex;
            boost::condition g_continue;
            unsigned int m_iCameraCount;
            boost::mutex m_mutex;

        };

    };
};


#endif //UBITACK_UTRENDERAPI_H
