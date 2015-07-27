//
// Created by Ulrich Eck on 26/07/2015.
//

#ifndef UBITACK_UTRENDERAPIPRIVATE_H
#define UBITACK_UTRENDERAPIPRIVATE_H

#include "utRenderAPI.h"
#include <boost/thread.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

namespace Ubitrack {
    namespace Visualization {

        RenderManager *g_render_manager = NULL;

        unsigned int registerVirtualCamera(std::string& name, CameraHandlePrivate* camera) {
            if (g_render_manager != NULL) {
                CameraHandle* cam = new CameraHandle(name, camera);
                unsigned int cam_id = g_render_manager->register_camera(cam);
            }
        }
    };
};

#endif //UBITACK_UTRENDERAPIPRIVATE_H
