//
// Created by Ulrich Eck on 26/07/2015.
//

#ifndef UBITACK_UTRENDERAPIPRIVATE_H
#define UBITACK_UTRENDERAPIPRIVATE_H

#include "utRenderAPI.h"




class CameraHandlePrivate {
public:
    CameraHandlePrivate();
    ~CameraHandlePrivate();

private:
    void* m_pModuleHandle;
};


namespace Ubitrack {
    namespace Visualization {
        registerVirtualCamera(CameraHandlePrivate* camera);
    };
};

#endif //UBITACK_UTRENDERAPIPRIVATE_H
