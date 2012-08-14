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
 * @ingroup driver_components
 * @file
 *
 * Web GUI driver
 *
 * @author Wolf Rödiger
 */

#include <boost/regex.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <map>
#include <string>
#include <cstdlib>
#include <log4cpp/Category.hh>

#include <utDataflow/PushSupplier.h>
#include <utDataflow/PushConsumer.h>
#include <utDataflow/ComponentFactory.h>
#include <utDataflow/Component.h>
#include <utDataflow/Module.h>
#include <utMeasurement/Measurement.h>
#include <utMeasurement/TimestampSync.h>
#include <utUtil/SerialPort.h>
#include <utUtil/Exception.h>
#include <utUtil/OS.h>

extern "C" {
#include "mongoose.h"
}

// Enum for measurement status.
typedef enum {
	MeasurementStatusGreen = 0,
	MeasurementStatusYellow,
	MeasurementStatusRed
} MeasurementStatus;

namespace Ubitrack { namespace Drivers {

/* MAKE_NODEIDKEY( WebModuleKey, "WebGui" ); */
/* MAKE_NODEIDKEY( WebComponentKey, "Event" ); */


class WebGuiDriverBase;


/**
 * @ingroup driver_components
 *
 * WebGUI HTTP server module. There can be only one such module in
 * the whole dataflow because \c SingleModuleKey is used as the module
 * key. The component provides a simple HTML visualization of all
 * registered \c WebGuiDriver components.
 * The HTML visualization displays status information for the measurements
 * specified in the dataflow. This is done with asynchronous push
 * notifications implemented using AJAX and a slow-load technique.
 * Additionally it features buttons which trigger events as
 * specified in the dataflow.
 */
class WebGuiModule
: public Dataflow::Module< Dataflow::SingleModuleKey, Dataflow::SubgraphIdKey, WebGuiModule, WebGuiDriverBase >
{
public:

	/** constructor */
	WebGuiModule( const Dataflow::SingleModuleKey& moduleKey, boost::shared_ptr< Graph::UTQLSubgraph >, FactoryHelper* pFactory );

	/** destructor */
	~WebGuiModule();
	
	void ErrorExit();

	/** called to register a measurement or an event */
	void registerEvent(WebGuiDriverBase* driver, std::string eventName, char event);
	void registerMeasurement(std::string measurementName, MeasurementStatus status);
	
	/** update measurement */
	void updateMeasurement(std::string measurementName, MeasurementStatus status);
	void updateMeasurementValue(std::string measurementName, std::string measurementValue);
	
	/** serve web requests */
	void serveIndex(struct mg_connection *conn, const struct mg_request_info *ri, void *user_data);
	void serveErrorPage(struct mg_connection *conn, const struct mg_request_info *ri, void *user_data);
	void serveEvent(struct mg_connection *conn, const struct mg_request_info *ri, void *user_data);
	void serveMeasurement(struct mg_connection *conn, const struct mg_request_info *ri, void *user_data);

	/** override this method to allow for registration of templated components within this module */
	boost::shared_ptr< WebGuiDriverBase > createComponent( const std::string& type, const std::string& name, boost::shared_ptr< Graph::UTQLSubgraph > pConfig, const Dataflow::SubgraphIdKey& key, WebGuiModule* pModule );

protected:
	/** map of key values for transformation into json */
	typedef boost::unordered_map<std::string, std::string> JsonMap;
	/** map of events */
	typedef boost::unordered_map<std::string, WebGuiDriverBase*> DriverMap;
	DriverMap driverTable;
	typedef boost::unordered_map<std::string, char> EventMap;
	EventMap eventTable;
	/** map of measurements */
	typedef boost::unordered_map<std::string, MeasurementStatus> MeasurementMap;
	MeasurementMap measurementTable;
	typedef boost::unordered_map<std::string, std::string> MeasurementValuesMap;
	MeasurementValuesMap measurementValuesTable;
	
	/** serve html template */
	void serveHtml(struct mg_connection *conn, const char *title, const std::string content[], int contentLength);
	void serveHtml(struct mg_connection *conn, const char *title, const std::string content[], int contentLength, const std::string header[], int headerLength);
	/** serve json template */
	void serveJson(struct mg_connection *conn, JsonMap& keyValuePairs);
	
	/** server port */
	std::string port;
	/** mongoose server context */
	struct mg_context *ctx;
	/** mutex to restrict the access to the measurementTable */
	boost::shared_mutex _statusAccess;
	/** mutex to restrict the access to the measurementValuesTable */
	boost::shared_mutex _valuesAccess;
};


/**
 * @ingroup driver_components
 *
 * Dummy base class for all components for \c WebGuiModule.
 *
 * TODO: This class has been introduced to allow for templating the webgui driver components so that different measurement types
 * can be supported. We should also separate the incoming measurement handling from the outgoing button events instead of having
 * this empty dummy class and a monolithic (though templated) descendent of it. In trackman, measurement and event patterns
 * are completely separated!
 */
class WebGuiDriverBase
	: public WebGuiModule::Component
{
public:
	/** constructor */
	WebGuiDriverBase( const std::string& name, boost::shared_ptr< Graph::UTQLSubgraph > subgraph, const Dataflow::SubgraphIdKey& componentKey, WebGuiModule* pModule );

	virtual void send( const Measurement::Button& p ) 
	{}
};


/**
 * @ingroup driver_components
 *
 * Component driver class for dataflow visualization and interaction via the \c WebGuiModule module.
 */
template< class EventType >
class WebGuiDriver
	: public WebGuiDriverBase
{
public:	
	/** constructor */
	WebGuiDriver( const std::string& name, boost::shared_ptr< Graph::UTQLSubgraph > subgraph, const Dataflow::SubgraphIdKey& componentKey, WebGuiModule* pModule );
	/** send events */
	virtual void send( const Measurement::Button& p );
	/** receiver measurements */
	void receive( const EventType& p );
	virtual void start();
	virtual void stop();

protected:	
	/** timeout detection thread */
	boost::shared_ptr< boost::thread > m_thread;
	/** thread exit condition */
	bool m_keepRunning;
	/** timeout detection method */
	void threadHandler();
	
	/** mutex to restrict the access to the update timestamps */
	boost::shared_mutex _updateAccess;
	/** input port */
	Dataflow::PushConsumer< EventType > m_input;
	/** output port */
	Dataflow::PushSupplier< Measurement::Button > m_output;
	/** button name */
	std::string eventName;
	/** event char */
	char event;
	/** measurement name */
	std::string measurementName;
	/** measurement timeout */
	Measurement::Timestamp timeout;
	/** time of previous update */
	Measurement::Timestamp lastUpdate;
	/** time of current update */
	Measurement::Timestamp currentUpdate;
};

}

} // namespace Ubitrack::Drivers

