#include "utRenderAPIPrivate.h"

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/function.hpp>

using namespace Ubitrack;
using namespace Ubitrack::Visualization;

// the singleton render manager object
static boost::scoped_ptr< RenderManager > g_pRenderManager;
static int g_RefRenderManager = 0;



VirtualWindow::VirtualWindow(int _width, int _height, const std::string &_title)
        : m_width(_width), m_height(_height), m_title(_title) {
}

VirtualWindow::~VirtualWindow() {

}

bool VirtualWindow::is_valid() {
    return false;
}

bool VirtualWindow::create() {
    return false;
}

void VirtualWindow::destroy() {

}

void VirtualWindow::initGL() {

}

void VirtualWindow::reshape(int w, int h) {

}

CameraHandle::CameraHandle(std::string &_name, int _width, int _height, Drivers::VirtualCamera *_handle)
        : m_sWindowName(_name)
        , m_initial_width(_width)
        , m_initial_height(_height)
        , m_bSetupNeeded(true)
        , m_pVirtualWindow(NULL)
        , m_pVirtualCamera(_handle)
{

}

CameraHandle::~CameraHandle() {
// delete m_pVirtualCamera ?
}

bool CameraHandle::need_setup() {
    return m_bSetupNeeded;
}

VirtualWindow* CameraHandle::get_window() {
    return m_pVirtualWindow;
}

bool CameraHandle::setup(VirtualWindow *window) {
    m_pVirtualWindow = window;
    if (window->create()) {
        // do something to window..
        // extend in subclass
        return true;
    }
    return false;
}

void CameraHandle::teardown() {

    if (m_pVirtualWindow != NULL) {
        m_pVirtualWindow->destroy();
    }
}

void CameraHandle::render() {
    // extend in subclass
}



void CameraHandle::on_window_size(int w, int h) {

}

void CameraHandle::on_window_close() {

}

void CameraHandle::on_keypress(int key, int scancode, int action, int mods) {

}


void CameraHandle::on_render() {

}

void CameraHandle::on_cursorpos(double xpos, double ypos) {

}

void CameraHandle::post_redraw() {
    // maybe needed
}

void CameraHandle::keyboard(unsigned char key, int x, int y) {
    // extend in subclass
}



RenderManager& RenderManager::singleton()
{
    // race condition between network receiving thread and main thread
    static boost::mutex singletonMutex;
    boost::mutex::scoped_lock l( singletonMutex );

    // create a new singleton if necessary
    if ( !g_pRenderManager )
        g_pRenderManager.reset( new RenderManager() );

    return *g_pRenderManager;
}



RenderManager::RenderManager()
        : m_iCameraCount(0) {
}

RenderManager::~RenderManager() {

}


void RenderManager::setup() {
    // anything to do here ... most setup should be done in the client
}

bool RenderManager::need_setup() {
    return (m_mCamerasNeedSetup.size() > 0);
}


bool RenderManager::any_windows_valid() {
    bool awv = false;
    for (CameraHandleMap::iterator it=m_mRegisteredCameras.begin(); it != m_mRegisteredCameras.end(); ++it) {
        awv |= it->second->get_window()->is_valid();
    }
    return awv;
}

CameraHandle *RenderManager::setup_pop_front() {
    boost::mutex::scoped_lock lock( m_mutex );
    if (m_mCamerasNeedSetup.size() > 0) {
        CameraHandle* cam = m_mCamerasNeedSetup.front();
        m_mCamerasNeedSetup.pop_front();
        return cam;
    }
    return NULL;
}

void RenderManager::setup_push_back(CameraHandle *handle) {
    boost::mutex::scoped_lock lock( m_mutex );
    m_mCamerasNeedSetup.push_back(handle);
}

void RenderManager::teardown() {
    for (CameraHandleMap::iterator it=m_mRegisteredCameras.begin(); it != m_mRegisteredCameras.end(); ++it) {
        it->second->teardown();
    }
}

CameraHandleMap::iterator RenderManager::cameras_begin() {
    return m_mRegisteredCameras.begin();
}

CameraHandleMap::iterator RenderManager::cameras_end() {
    return m_mRegisteredCameras.end();
}

bool RenderManager::wait_for_event() {
    // not implemented
    return false;
}

unsigned int RenderManager::register_camera(CameraHandle *handle) {
    boost::mutex::scoped_lock lock( m_mutex );
    unsigned int new_id = m_iCameraCount++;
    m_mRegisteredCameras[new_id] = handle;
    m_mCamerasNeedSetup.push_back(handle);
    return new_id;
}

void RenderManager::unregister_camera(unsigned int cam_id) {
    boost::mutex::scoped_lock lock( m_mutex );
    if (m_mRegisteredCameras.find(cam_id) != m_mRegisteredCameras.end()) {
        m_mRegisteredCameras.erase(cam_id);
    }
    m_iCameraCount--;
}

unsigned int RenderManager::camera_count() {
    return m_iCameraCount;
}

CameraHandle *RenderManager::get_camera(unsigned int cam_id) {
    boost::mutex::scoped_lock lock( m_mutex );
    if (m_mRegisteredCameras.find(cam_id) != m_mRegisteredCameras.end()) {
        return m_mRegisteredCameras[cam_id];
    }
    return NULL;
}
