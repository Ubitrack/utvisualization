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



#ifdef HAVE_GLEW
	#include "GL/glew.h"
#endif

#ifdef _WIN32
	#include <utUtil/CleanWindows.h>
	#include <GL/gl.h>
	#include <GL/glu.h>
#elif __APPLE__
	#include <OpenGL/OpenGL.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glext.h> // Linux headers
	#include <GL/wglext.h> // Windows headers - Not sure which ones cygwin needs. Just try it
	#include <GL/glu.h>
#endif

#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <signal.h>
#include <iostream>
#include <vector>
#ifdef _WIN32
#include <conio.h>
#endif


#include <boost/thread.hpp>
#include <boost/program_options.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

#include <utFacade/AdvancedFacade.h>
#include <utUtil/Exception.h>
#include <utUtil/Logging.h>
#include <utUtil/OS.h>

#include "glfw_rendermanager.h"
#include "utVisualization/utRenderAPI.h"


using namespace Ubitrack;
using namespace Ubitrack::Visualization;

bool bStop = false;

void ctrlC ( int i )
{
	bStop = true;
}

void CheckForGLErrors(std::string a_szMessage)
{
    GLenum error = glGetError();
    while (error != GL_NO_ERROR)
    {
        std::cout << "Error: " << a_szMessage.c_str() << ", ErrorID: " << error << ": " << gluErrorString(error);
        error = glGetError(); // get next error if any.
    }
}

int main( int ac, char** av )
{
	signal ( SIGINT, &ctrlC );
	
	try
	{

		// program options
		std::string sServerAddress;
		std::string sUtqlFile;
		std::string sExtraUtqlFile;
		std::string sComponentsPath;
		std::string sLogConfig = "log4cpp.conf";
		bool bNoExit;

		try
		{
			// describe program options
			namespace po = boost::program_options;
			po::options_description poDesc( "Allowed options", 80 );
			poDesc.add_options()
				( "help", "print this help message" )
				( "server,s", po::value< std::string >( &sServerAddress ), "Ubitrack server address <host>[:<port>] to connect to" )
				( "components_path", po::value< std::string >( &sComponentsPath ), "Directory from which to load components" )
				("log_config", po::value< std::string >(&sLogConfig), "Logging configuration file")
				("utql", po::value< std::string >(&sUtqlFile), "UTQL request or response file, depending on whether a server is specified. "
					"Without specifying this option, the UTQL file can also be given directly on the command line." )
				( "extra-dataflow", po::value< std::string >( &sExtraUtqlFile ), "Additional UTQL response file to be loaded directly without using the server" )
				( "noexit", "do not exit on return" )
				( "path", "path to ubitrack bin directory" )
				#ifdef _WIN32
				( "priority", po::value< int >( 0 ),"set priority of console thread, -1: lower, 0: normal, 1: higher, 2: real time (needs admin)" )
				#endif
			;
			
			// specify default options
			po::positional_options_description inputOptions;
			inputOptions.add( "utql", 1 );		
			
			// parse options from command line and environment
			po::variables_map poOptions;
			po::store( po::command_line_parser( ac, av ).options( poDesc ).positional( inputOptions ).run(), poOptions );
			po::store( po::parse_environment( poDesc, "UBITRACK_" ), poOptions );
			po::notify( poOptions );

			#ifdef _WIN32
			// set to quite high priority, see: http://msdn.microsoft.com/en-us/library/windows/desktop/ms686219%28v=vs.85%29.aspx
			// carefully use this function under windows to steer ubitrack's cpu time:
			if(poOptions.count("priority") != 0) {
				
				int prio = poOptions["priority"].as<int>();
				switch(prio){
				case -1:
					SetPriorityClass( GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS );
					break;
				case 0:
					//SetPriorityClass( GetCurrentProcess(), NORMAL_PRIORITY_CLASS  );
					break;
				case 1:
					SetPriorityClass( GetCurrentProcess(), HIGH_PRIORITY_CLASS  );
					break;
				case 2:
					SetPriorityClass( GetCurrentProcess(), REALTIME_PRIORITY_CLASS  );
					break;
				}
			}
			#endif

			bNoExit = poOptions.count( "noexit" ) != 0;
			
			// print help message if nothing specified
			if ( poOptions.count( "help" ) || sUtqlFile.empty() )
			{
				std::cout << "Syntax: utConsole [options] [--utql] <UTQL file>" << std::endl << std::endl;
				std::cout << poDesc << std::endl;
				return 1;
			}
			
			// initialize logging		
			Util::initLogging(sLogConfig.c_str());

		}
		catch( std::exception& e )
		{
			std::cerr << "Error parsing command line parameters : " << e.what() << std::endl;
			std::cerr << "Try utConsole --help for help" << std::endl;
			return 1;
		}

		// Init GLFW
		glfwInit();

		glfwWindowHint(GLFW_SAMPLES, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

		glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

		// create and register render manager
		RenderManager& pRenderManager = RenderManager::singleton();


		// configure ubitrack
		std::cout << "Loading components..." << std::endl << std::flush;
		Facade::AdvancedFacade utFacade( sComponentsPath );

		if ( sServerAddress.empty() )
		{
			std::cout << "Instantiating dataflow network from " << sUtqlFile << "..." << std::endl << std::flush;
			utFacade.loadDataflow( sUtqlFile );
		}
		else
		{
			if ( !sExtraUtqlFile.empty() )
				utFacade.loadDataflow( sExtraUtqlFile, false );
				
			std::cout << "Connecting to server " << sServerAddress << "..." << std::endl << std::flush;
			utFacade.connectToServer( sServerAddress );
			
			std::cout << "Sending UTQL to to server " << sUtqlFile << "..." << std::endl << std::flush;
			utFacade.sendUtqlToServer( sUtqlFile );
		}

		std::cout << "Starting dataflow" << std::endl;
		utFacade.startDataflow();

		// setup rendermanager
		pRenderManager.setup();
		unsigned int windows_opened = 0;

		while( !bStop && (( windows_opened == 0 ) || ( pRenderManager.any_windows_valid() )))
		{
			boost::shared_ptr<CameraHandle> cam;
			boost::shared_ptr<GLFWWindowImpl> win;
			while (pRenderManager.need_setup()) {
				cam = pRenderManager.setup_pop_front();
				std::cout << "Camera setup: " << cam->title() << std::endl;
				win.reset(new GLFWWindowImpl(cam->initial_width(),
											 cam->initial_height(),
											 cam->title()));

				// XXX can this be simplified ??
				boost::shared_ptr<VirtualWindow> win_ = boost::dynamic_pointer_cast<VirtualWindow>(win);
				if (!cam->setup(win_)) {
					pRenderManager.setup_push_back(cam);
				} else {
					win->initGL(cam);
#ifdef WIN32
					Util::sleep(30);
#endif
					windows_opened++;
				}
                glfwPollEvents();
			}

			std::vector< unsigned int > chToDelete;
			CameraHandleMap::iterator pos = pRenderManager.cameras_begin();
			CameraHandleMap::iterator end = pRenderManager.cameras_end();
			int ellapsed_time = (int)(glfwGetTime() * 1000.);
			while ( pos != end ) {
				bool is_valid = false;
				if (pos->second) {
					cam = pos->second;
					if (cam->get_window()) {
						win = boost::dynamic_pointer_cast<GLFWWindowImpl>(cam->get_window());
						if ((win) && (win->is_valid())) {
                            win->pre_render();
//							cam->pre_render();
							cam->render(ellapsed_time);
							//cam->post_render(); ??
							is_valid = true;
                            win->post_render();  // make this loop through all current windows??
                            CheckForGLErrors("Render Error");
						}
					}
				}
				if (!is_valid) {
					chToDelete.push_back(pos->first);
				}
				pos++;
				glfwPollEvents();
			}

			if (chToDelete.size() > 0) {
				for (unsigned int i = 0; i < chToDelete.size(); i++) {
					unsigned int cam_id = chToDelete.at(i);
					pRenderManager.get_camera(cam_id)->teardown();
					pRenderManager.unregister_camera(cam_id);
                    glfwPollEvents();
				}
			}
			// need a way to exit the loop here ..
			pRenderManager.wait_for_event(100);
		}

		// teardown rendermanager
		pRenderManager.teardown();

		std::cout << "Stopping dataflow..." << std::endl << std::flush;
		utFacade.stopDataflow();

		glfwTerminate();

		std::cout << "Finished, cleaning up..." << std::endl << std::flush;
	}
	catch( Util::Exception& e )
	{
		std::cout << "exception occurred" << std::endl << std::flush;
		std::cerr << e << std::endl;
	}

	std::cout << "utConsole terminated." << std::endl << std::flush;
}
