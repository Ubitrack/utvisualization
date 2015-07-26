
#include "utRenderAPIPrivate.h"

using namespace Ubitrack;
using namespace Ubitrack::Visualization;


RenderManager* g_render_manager = NULL;


// some unavoidable globals to connect the dataflow components to the rendermanager
std::deque< CameraHandle* > g_setup;
std::map< std::string, int > g_names;
std::map< int, CameraHandle* > g_modules;
std::map< int, VirtualWindow > g_windows;
std::map< VirtualWindow*, int > g_windows_rev;

boost::mutex g_globalMutex;
boost::condition g_setup_performed;
boost::condition g_continue;
boost::condition g_cleanup_done;




void setDefaultRenderManager(RenderManager* mgr) {
    if (g_render_manager != NULL) {
        delete g_render_manager;
    }
    g_render_manager = mgr;
}