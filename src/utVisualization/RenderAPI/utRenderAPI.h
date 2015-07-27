//
// Created by Ulrich Eck on 26/07/2015.
//

#ifndef UBITACK_UTRENDERAPI_H
#define UBITACK_UTRENDERAPI_H

#include <string>
#include <map>
#include <deque>
#include <boost/thread.hpp>

#include <utVisualization/RenderAPI/Config.h>

namespace Ubitrack {
        namespace Visualization {

            class CameraHandlePrivate;

            class VirtualWindow {

            public:
                VirtualWindow(int _width, int _height, const std::string& _title);
                ~VirtualWindow();

                virtual bool is_valid();
                virtual bool create();
                virtual void initGL();
                virtual void destroy();

                virtual void reshape( int w, int h);

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


            class CameraHandle {

            public:
                CameraHandle(std::string& _name, int _width, int _height, CameraHandlePrivate* _handle);
                ~CameraHandle();

                bool need_setup();
                virtual bool setup(VirtualWindow* window);
                VirtualWindow* get_window();
                virtual void teardown();

                /** redraw GL context, called from main GL thread _only_ */
                virtual void activate();
                virtual void redraw();

                /** keyboard callback */
                virtual void keyboard( unsigned char key, int x, int y );

                // more interaction options ...

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
                VirtualWindow* m_pVirtualWindow;

            private:
                std::string m_sWindowName;
                bool m_bSetupNeeded;
                CameraHandlePrivate* m_pCameraHandlePrivate;
            };


            typedef std::map<unsigned int, CameraHandle*> CameraHandleMap;

            // XXX should be singleton..
            class RenderManager {

            public:
                RenderManager();
                ~RenderManager();

                bool need_setup();
                CameraHandle* setup_pop_front();
                void setup_push_back(CameraHandle* handle);


                CameraHandleMap::iterator cameras_begin();
                CameraHandleMap::iterator cameras_end();

                unsigned int camera_count();
                CameraHandle* get_camera(unsigned int cam_id);

                virtual unsigned int register_camera(CameraHandle* handle);
                virtual void unregister_camera(unsigned int cam_id);
                virtual void setup();
                virtual bool any_windows_valid();
                virtual void teardown();
                virtual bool wait_for_event();

            private:
                CameraHandleMap m_mRegisteredCameras;
                std::deque<CameraHandle* > m_mCamerasNeedSetup;
                unsigned int m_iCameraCount;
                boost::mutex m_mutex;

            };


            void setDefaultRenderManager(RenderManager* mgr);

        };
};


#endif //UBITACK_UTRENDERAPI_H
