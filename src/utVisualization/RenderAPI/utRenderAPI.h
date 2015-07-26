//
// Created by Ulrich Eck on 26/07/2015.
//

#ifndef UBITACK_UTRENDERAPI_H
#define UBITACK_UTRENDERAPI_H

#include <string>
#include <map>

#include <utVisualization/RenderAPI/Config.h>

namespace Ubitrack {
        namespace Visualization {

            class CameraHandlePrivate;

            class VirtualWindow {

            public:
                VirtualCameraImpl(int _width, int _height, const std::string& _title);
                ~VirtualCameraImpl();

                virtual void create();
                virtual void initGL();

                virtual void reshape( int w, int h);

            protected:
                int m_width;
                int m_height;
                std::string m_title;

            };


            class CameraHandle {

            public:
                CameraHandle(std::string& _name, CameraHandlePrivate* _handle);
                ~CameraHandle();

                bool need_setup();
                void setup(VirtualWindow* window);

                void redraw();
                // need more ?

            private:
                std::string m_sWindowName;
                VirtualWindow* m_pVirtualWindow;
                bool m_bSetupNeeded;
                CameraHandlePrivate* m_pCameraHandlePrivate;
            };


            typedef std::map<std::string, CameraHandle*> CameraHandleMap;

            // XXX should be singleton..
            class RenderManager {

            public:
                RenderManager();
                ~RenderManager();

                CameraHandleMap::iterator camerasBegin();
                CameraHandleMap::iterator camerasEnd();


                virtual bool registerCamera(const std::string& _name, CameraHandlePrivate* handle);
                virtual void setup();
                virtual bool all_windows_closed();
                virtual void teardown();
                virtual bool wait_for_event();


            };


            void setDefaultRenderManager(RenderManager* mgr);

        };
};


#endif //UBITACK_UTRENDERAPI_H
