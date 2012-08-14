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

#include "webgui.h"
#include "config.h"
#include <utUtil/Exception.h>
#include <utUtil/OS.h>
#include <utCalibration/AbsoluteOrientation.h>

#include <stdlib.h>
#include <stdio.h>
#include <list>

static log4cpp::Category& logger( log4cpp::Category::getInstance( "Drivers.WebGui" ) );

namespace Ubitrack { namespace Drivers {

using namespace Dataflow;

/** Wraps call to the method which serves the index page. */
void serveIndexWrapper(struct mg_connection *conn, const struct mg_request_info *ri, void *user_data)
{
	((WebGuiModule*)user_data)->serveIndex(conn, ri, NULL);
}

/** Wraps call to the method which serves the error page. */
void serveErrorPageWrapper(struct mg_connection *conn, const struct mg_request_info *ri, void *user_data)
{
	((WebGuiModule*)user_data)->serveErrorPage(conn, ri, NULL);
}

/** Wraps call to the method which serves the measurement page. */
void serveMeasurementWrapper(struct mg_connection *conn, const struct mg_request_info *ri, void *user_data)
{
	((WebGuiModule*)user_data)->serveMeasurement(conn, ri, NULL);
}

/** Wraps call to the method which serves the event page. */
void serveEventWrapper(struct mg_connection *conn, const struct mg_request_info *ri, void *user_data)
{
	((WebGuiModule*)user_data)->serveEvent(conn, ri, NULL);
}

/** Constructor. */
WebGuiModule::WebGuiModule( const Dataflow::SingleModuleKey& moduleKey, boost::shared_ptr< Graph::UTQLSubgraph > subgraph, FactoryHelper* pFactory )
	: Module< Dataflow::SingleModuleKey, Dataflow::SubgraphIdKey, WebGuiModule, WebGuiDriverBase >( moduleKey, pFactory )
{
	// Get configuration node.
	Graph::UTQLSubgraph::NodePtr config = subgraph->getNode("WebGui");

	// Read server configuration.
	port = config->getAttribute("port").getText();
	LOG4CPP_INFO(logger, "Read port " << port << " from configuration node.");

	// Start webserver and set options.
	ctx = mg_start();
	std::string root = mg_get_option(ctx, "root");
	root = UBITRACK_COMPONENTS_PATH;
	std::string access_log = "access.log";
	std::string error_log = "error.log";
	mg_set_option(ctx, "ports", port.c_str());		// set listening port
	mg_set_option(ctx, "root",  root.c_str());		// change server root directory to webgui directory
	mg_set_option(ctx, "access_log", access_log.c_str());	// set access log
	mg_set_option(ctx, "error_log", error_log.c_str());		// set error log

	// Print webserver options.
	const char *serverPorts = mg_get_option(ctx, "ports");
	const char *rootDirectory = mg_get_option(ctx, "root");
	const char *accessLog = mg_get_option(ctx, "access_log");
	const char *errorLog = mg_get_option(ctx, "error_log");
	LOG4CPP_INFO(logger, "Server ports: " << serverPorts << ".");
	LOG4CPP_INFO(logger, "Server root directory: " << rootDirectory << ".");
	LOG4CPP_INFO(logger, "Server access log: " << accessLog << ".");
	LOG4CPP_INFO(logger, "Server error log: " << errorLog << ".");

	// Register URIs.
	mg_set_uri_callback(ctx, "/", &serveIndexWrapper, this);
	mg_set_uri_callback(ctx, "/events", &serveEventWrapper, this);
	mg_set_uri_callback(ctx, "/measurements", &serveMeasurementWrapper, this);
	mg_set_error_callback(ctx, 0, &serveErrorPageWrapper, this);

	LOG4CPP_INFO( logger, "WebGUI http server module started" );
}

/** Destructor. */
WebGuiModule::~WebGuiModule()
{
	// Stop webserver.
	mg_stop(ctx);

	LOG4CPP_INFO( logger, "WebGUI http server module terminated" );
}

/** Register an event. */
void WebGuiModule::registerEvent(WebGuiDriverBase* driver, std::string eventName, char event)
{
	driverTable[eventName] = driver;	// remember driver
	eventTable[eventName] = event;		// remember event char
	eventName = "/events/" + eventName;
	mg_set_uri_callback(ctx, eventName.c_str(), &serveEventWrapper, this);
}

/** Register a measurement. */
void WebGuiModule::registerMeasurement(std::string measurementName, MeasurementStatus status)
{
	updateMeasurement(measurementName, status);
	measurementName = "/measurements/" + measurementName;
	mg_set_uri_callback(ctx, measurementName.c_str(), &serveMeasurementWrapper, this);
}

/** Update measurement. */
void WebGuiModule::updateMeasurement(std::string measurementName, MeasurementStatus status)
{
	// Get lock for the measurementTable.
	boost::upgrade_lock<boost::shared_mutex> lock(_statusAccess);

	// Get exclusive access.
	boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);

	// Update measurement.
	measurementTable[measurementName] = status;
}

/** Update measurement values. */
void WebGuiModule::updateMeasurementValue(std::string measurementName, std::string measurementValue)
{
	// Get lock for the measurementValuesTable.
	boost::upgrade_lock<boost::shared_mutex> lock(_valuesAccess);

	// Get exclusive access.
	boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);

	// Update measurement.
	measurementValuesTable[measurementName] = measurementValue;
}

/** Embeds the content into a HTML template. */
void WebGuiModule::serveHtml(struct mg_connection *conn, const char *title, const std::string content[], int contentLength)
{
	serveHtml(conn, title, content, contentLength, NULL, 0);
}

/** Embeds the content into a HTML template. */
void WebGuiModule::serveHtml(struct mg_connection *conn, const char *title, const std::string content[], int contentLength, const std::string header[], int headerLength)
{
	// HTTP-Header.
	mg_printf(conn, "%s", 	"HTTP/1.1 200 OK\r\n"
							"Content-Type: text/html\r\n"
							"Connection: close\r\n\r\n");
	// HTTP-Body.
	mg_printf(conn, "<!DOCTYPE html>\n");
	mg_printf(conn, "<html>\n");
	// HTML-Header.
	mg_printf(conn, "<head>\n");
	mg_printf(conn, "	<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n");
	mg_printf(conn, "	<title>%s</title>\n", title);
	for (int i = 0; i < headerLength; i++)
	{
		mg_printf(conn, "	%s\n", header[i].c_str());
	}
	mg_printf(conn, "</head>\n");
	// HTML-Body.
	mg_printf(conn, "<body>\n");
	mg_printf(conn, "	<header><h1>%s</h1></header>\n", title);
	mg_printf(conn, "	<section>\n");
	for (int i = 0; i < contentLength; i++)
	{
		mg_printf(conn, "		%s\n", content[i].c_str());
	}
	mg_printf(conn, "	</section>\n");
	mg_printf(conn, "	<footer><p>&copy; Technische Universität München</p></footer>\n");
	mg_printf(conn, "</body>\n");
	mg_printf(conn, "</html>");
}

/** Serve key-value-pairs as a JSON document. */
void WebGuiModule::serveJson(struct mg_connection *conn, JsonMap& keyValuePairs)
{
	// HTTP-Header.
	mg_printf(conn, "%s", 	"HTTP/1.1 200 OK\r\n"
							"Content-Type: application/json\r\n"
							"Connection: close\r\n\r\n");
	// HTTP-Body.
	mg_printf(conn, "{\n");
	unsigned int i = 0;
	for (JsonMap::iterator it = keyValuePairs.begin(); it != keyValuePairs.end(); ++it)
	{
		mg_printf(conn, "	\"%s\": %s%s", it->first.c_str(), it->second.c_str(), (++i < keyValuePairs.size()) ? ", \n" : "\n");
	}
	mg_printf(conn, "}");
}

/** Serves the index page. */
void WebGuiModule::serveIndex(struct mg_connection *conn, const struct mg_request_info *ri, void *user_data)
{
	// Create header.
	const int headerLength = 2;
	std::string header[headerLength];
	header[0] = "<script src=\"prototype.js\" type=\"text/javascript\"></script>";
	header[1] = "<script src=\"updater.js\" type=\"text/javascript\"></script>";
	
	// Create body.
	const int contentLength = 9;
	std::string content[contentLength];
	content[0] = "<section id=\"measurements\">";
	content[1] = "<h2><a href=\"/measurements\">Measurements</a></h2>";
	content[2] = "</section>";
	content[3] = "<section id=\"events\">";
	content[4] = "<h2><a href=\"/events\">Events</a></h2>";
	content[5] = "</section>";
	content[6] = "<section id=\"log\" style=\"display:none\">";
	content[7] = "<h2>Server messages</h2>";
	content[8] = "</section>";
	serveHtml(conn, "Ubitrack WebGUI", content, contentLength, header, headerLength);
}

/** Serves the error page. */
void WebGuiModule::serveErrorPage(struct mg_connection *conn, const struct mg_request_info *ri, void *user_data)
{
	mg_printf(conn, "HTTP/1.1 %d Not Found\r\n"
					"Content-Type: text/plain\r\n"
					"Connection: close\r\n\r\n", ri->status_code);
	mg_printf(conn, "Error %d.", ri->status_code);
}

/** Serves the measurement page. */
void WebGuiModule::serveMeasurement(struct mg_connection *conn, const struct mg_request_info *ri, void *user_data)
{
	// Read accept header.
	std::string accept = "";
	if (mg_get_header(conn, "Accept") != NULL)
	{
		accept = mg_get_header(conn, "Accept");
	}
	else
	{
		accept = "text/html";
	}
	
	// Get specific measurement ID from URI.
	std::string measurementId = "";
	boost::regex expression("^/measurements/(.*)$");
	boost::smatch matches;
	std::string uri = ri->uri;
	if (boost::regex_match(uri, matches, expression) && matches.size() > 1)
	{
		measurementId = matches[1];
	}
	
	// Check if request wants to wait for new data or just wants the current data.
	const char *waitForUpdateKey = "waitForUpdate";
	char *waitForUpdateValue = mg_get_var(conn, waitForUpdateKey);
	if (waitForUpdateValue != NULL && measurementId.length() > 0)
	{
		// Check current status.
		mg_free(waitForUpdateValue);
		MeasurementStatus currentStatus = MeasurementStatusGreen;
		
		{
			// Get shared lock for the measurementTable.
			boost::shared_lock<boost::shared_mutex> lock(_statusAccess);
			
			// Get current status.
			currentStatus = measurementTable[measurementId];
		}
		
		// Wait for change.
		unsigned int timeout = 1000;	// Timeout in ms (webpage refresh happens at least every timeout).
		unsigned int sleepSpan = 100;	// Time to sleep in ms (arbitrarily chosen, can be easily changed).
		unsigned int waiting = 0;
		while (waiting < timeout)
		{
			{
				// Get shared lock for the measurementTable.
				boost::shared_lock<boost::shared_mutex> lock(_statusAccess);
				
				// Check for update.
				if (currentStatus != measurementTable[measurementId])
				{
					break;
				}
			}
			
			// Wait some time.
			Util::sleep( sleepSpan );
			waiting += sleepSpan;
		}
	}
	
	{
		// Get shared lock for the measurementTable.
		boost::shared_lock<boost::shared_mutex> lock(_statusAccess);
		
		// Check if the request asks for a list of measurements or a specific measurement.
		if (measurementId.length() == 0)
		{
			// Create document depending on MIME type.
			if (accept.find("html") != std::string::npos)
			{
				const int length = 4 + measurementTable.size();
				std::string* content = new std::string[length];
				content[0] = "<p><a href=\"/\">Back to index page</a></p>";
				content[1] = "<p>Available measurements:</p>";
				content[2] = "<ul>";
				int i = 3;
				for (MeasurementMap::iterator it = measurementTable.begin(); it != measurementTable.end(); ++it)
				{
					content[i] = "	<li><a href=\"/measurements/";
					content[i] += it->first;
					content[i] += "\">";
					content[i] += it->first;
					content[i] += "</a></li>";
					i++;
				}
				content[i] = "</ul>";
				serveHtml(conn, "List of measurements", content, length);
				delete [] content;
			}
			else if (accept.find("json") != std::string::npos)
			{
				// Measurement list.
				JsonMap measurementList;
				measurementList["measurements"] = "[";
				unsigned int i = 0;
				for (MeasurementMap::iterator it = measurementTable.begin(); it != measurementTable.end(); ++it)
				{
					measurementList["measurements"] += "\"/measurements/" + it->first + "\"" + ((++i < measurementTable.size()) ? ", " : "");
				}
				measurementList["measurements"] += "]";
				serveJson(conn, measurementList);
			}
		}
		else
		{
			// Check if the measurement is available.
			if (measurementTable.find(measurementId) != measurementTable.end())
			{
				// Create document depending on MIME type.
				if (accept.find("html") != std::string::npos)
				{
					// Send response.
					const int length = 3;
					std::string content[length];
					content[0] = "<p><a href=\"/measurements\">Back to list of measurements</a></p>";
					content[1] = "<p>The status for ";
					content[1] += measurementId;
					content[1] += " is: ";
					if (measurementTable[measurementId] == MeasurementStatusGreen)
					{
						content[2] += "<strong>Ready</strong></p>";
					}
					else if (measurementTable[measurementId] == MeasurementStatusYellow)
					{
						content[2] += "<strong>Single Measurement</strong></p>";
					}
					else
					{
						content[2] += "<strong>Unavailable</strong></p>";
					}
					serveHtml(conn, "Measurement details", content, length);
				}
				else if (accept.find("json") != std::string::npos)
				{
					// Measurement object.
					JsonMap measurement;
					measurement["name"] = "\"" + measurementId + "\"";
					measurement["uri"] = "\"/measurements/" + measurementId + "\"";
					std::stringstream statusStream;
					std::string statusString;
					statusStream << measurementTable[measurementId];
					statusString = statusStream.str();
					measurement["status"] = statusString;
					measurement["values"] = "\"" + measurementValuesTable[measurementId] + "\"";
					serveJson(conn, measurement);
				}
			}
			else
			{	
				// Serve error page (should never happen, because then the URI would not be registered).
				serveErrorPage(conn, ri, NULL);
			}
		}
	}
}

/** Serves the event page. */
void WebGuiModule::serveEvent(struct mg_connection *conn, const struct mg_request_info *ri, void *user_data)
{
	// Read accept header.
	std::string accept = mg_get_header(conn, "Accept");
	accept = (accept.empty()) ? "text/html" : accept;
	
	// Get specific event ID from URI.
	std::string eventId = "";
	boost::regex expression("^/events/(.*)$");
	boost::smatch matches;
	std::string uri = ri->uri;
	if (boost::regex_match(uri, matches, expression) && matches.size() > 1)
	{
		eventId = matches[1];
	}
	
	// Check if the request asks for a list of events or a specific event.
	if (eventId.length() == 0)	// list
	{		
		// Create document depending on MIME type.
		if (accept.find("html") != std::string::npos)	// html
		{
			const int length = eventTable.size() + 4;
//			std::string content[length];
			std::string* content = new std::string[length];

			content[0] = "<p><a href=\"/\">Back to index page</a></p>";
			content[1] = "<p>Registered events:</p>";
			content[2] = "<ul>";
			int i = 3;
			for (EventMap::iterator it = eventTable.begin(); it != eventTable.end(); ++it)
			{
				content[i] = "	<li><a href=\"/events/";
				content[i] += it->first;
				content[i] += "\">";
				content[i] += it->first;
				content[i] += "</a></li>";
				i++;
			}
			content[i] = "</ul>";
			serveHtml(conn, "List of events", content, length);
			delete [] content;
		}
		else if (accept.find("json") != std::string::npos)	// json
		{
			// Event list.
			JsonMap eventList;
			eventList["events"] = "[";
			unsigned int i = 0;
			for (EventMap::iterator it = eventTable.begin(); it != eventTable.end(); ++it)
			{
				eventList["events"] += "\"/events/" + it->first + "\"" + ((++i < eventTable.size()) ? ", " : "");
			}
			eventList["events"] += "]";
			serveJson(conn, eventList);
		}
	}
	else	// specific event
	{
		// Check if someone is registered for the event.
		if (eventTable.find(eventId) != eventTable.end())
		{	
			// Check HTTP method.
			std::string postMethod = "POST";
			if (postMethod.compare(ri->request_method) == 0)
			{
				// Send event.
				Measurement::Timestamp timestamp = Measurement::now();
				Measurement::Button button(timestamp, eventTable[eventId]);
				driverTable[eventId]->send(button);
			
				// Create document depending on MIME type.
				if (accept.find("html") != std::string::npos)
				{
					const int length = 5;
					std::string content[length];
					content[0] = "<p><a href=\"/events\">Back to list of events</a></p>";
					content[1] = "<p>";
					content[1] += eventId;
					content[1] += " pressed.</p>";
					content[2] = "<form method=\"POST\">";
					content[3] = "	<button type=\"submit\">";
					content[3] += eventId;
					content[3] += "</button>";
					content[4] = "</form>";
					serveHtml(conn, "Event details", content, length);
				}
				else if (accept.find("json") != std::string::npos)
				{
					// Event object.
					JsonMap event;
					event["name"] = "\"" + eventId + "\"";
					event["uri"] = "\"/events/" + eventId + "\"";
					event["success"] = "true";
					serveJson(conn, event);
				}
			}
			// Inform about event.
			else
			{
				// Create document depending on MIME type.
				if (accept.find("html") != std::string::npos)
				{
					const int length = 5;
					std::string content[length];
					content[0] = "<p><a href=\"/events\">Back to list of events</a></p>";
					content[1] = "<p>";
					content[1] += eventId;
					content[1] += " is registered.</p>";
					content[2] = "<form method=\"POST\">";
					content[3] = "	<button type=\"submit\">";
					content[3] += eventId;
					content[3] += "</button>";
					content[4] = "</form>";
					serveHtml(conn, "Event details", content, length);
				}
				else if (accept.find("json") != std::string::npos)
				{
					// Event object.
					JsonMap event;
					event["name"] = "\"" + eventId + "\"";
					event["uri"] = "\"/events/" + eventId + "\"";
					serveJson(conn, event);
				}
			}
		}
		else
		{
			// Serve error page (should never happen, because then the URI would not be registered).
			serveErrorPage(conn, ri, NULL);
		}
	}
}


WebGuiDriverBase::WebGuiDriverBase( const std::string& name, boost::shared_ptr< Graph::UTQLSubgraph > subgraph, const Dataflow::SubgraphIdKey& componentKey, WebGuiModule* pModule )
	: WebGuiModule::Component( name, componentKey, pModule )
{
}


/** 
 * Constructor for WebGuiDriver.
 * Output: Creates and registers URIs for button events.
 * Input: Creates and registers URIs for measurements.
 */
template< class EventType >
WebGuiDriver< EventType >::WebGuiDriver( const std::string& name, boost::shared_ptr< Graph::UTQLSubgraph > subgraph, const Dataflow::SubgraphIdKey& componentKey, WebGuiModule* pModule )
	: WebGuiDriverBase( name, subgraph, componentKey, pModule )
	, m_keepRunning(false)
	, m_input( "Input", *this, boost::bind( &WebGuiDriver::receive, this, _1 ) )
	, m_output( "Output", *this )
	, timeout(0)
{
	// Output: Register URI to trigger this output.
	if (subgraph->m_DataflowAttributes.hasAttribute("name"))
	{
		eventName = subgraph->m_DataflowAttributes.getAttributeString("name");
		event = subgraph->m_DataflowAttributes.getAttributeString("event")[0];
		getModule().registerEvent(this, eventName, event);
		LOG4CPP_INFO(logger, "Registered event \"" << event << "\" with name " << eventName << ".");
	}
	// Input: Register measurement.
	else if (subgraph->m_DataflowAttributes.hasAttribute("measurementName"))
	{
		currentUpdate = Measurement::now();
		measurementName = subgraph->m_DataflowAttributes.getAttributeString("measurementName");
		timeout = (unsigned long long int)(1e6 * atoi(subgraph->m_DataflowAttributes.getAttributeString("timeout").c_str()));
		getModule().registerMeasurement(measurementName, MeasurementStatusRed);
		LOG4CPP_INFO(logger, "Registered measurement with name " << measurementName << " and timeout " << timeout << ".");
	}
}

template< class EventType >
void WebGuiDriver< EventType >::start()
{
	// Create thread for timeout detection.
	if (!m_keepRunning)
	{
		m_keepRunning = true;
		if (timeout)
		{
			m_thread.reset( new boost::thread( boost::bind( &WebGuiDriver::threadHandler, this ) ) );
		}
	}
}

template< class EventType >
void WebGuiDriver< EventType >::stop()
{
	// Stop tiemout detection thread.
	if (m_keepRunning)
	{
		m_keepRunning = false;
		if (m_thread)
		{
			m_thread->join();
		}
	}
}

/** Send event. */
template< class EventType >
void WebGuiDriver< EventType >::send( const Measurement::Button& b )
{	
	LOG4CPP_TRACE( logger, "User generated event on component with key \"" << event << "\"" );
	
	// Send event.
	m_output.send( b );
}

/** Receive measurements. */
template<>
void WebGuiDriver< Measurement::Pose >::receive( const Measurement::Pose& p )
{		
	LOG4CPP_TRACE( logger, "Received Pose event on component with key '" << getKey() << "'" );
	
	// Get lock for the update timestamps.
	boost::upgrade_lock<boost::shared_mutex> lock(_updateAccess);

	// Get exclusive access.
	boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
	
	// Remember last update.
	lastUpdate = currentUpdate;
	currentUpdate = p.time();
	
	// Update values.
	std::ostringstream measurementValue;
	measurementValue << p;
	getModule().updateMeasurementValue(measurementName, measurementValue.str());
}

/** Receive measurements. */
template<>
void WebGuiDriver< Measurement::Position >::receive( const Measurement::Position& p )
{		
	LOG4CPP_TRACE( logger, "Received Position event on component with key '" << getKey() << "'" );
	
	// Get lock for the update timestamps.
	boost::upgrade_lock<boost::shared_mutex> lock(_updateAccess);

	// Get exclusive access.
	boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
	
	// Remember last update.
	lastUpdate = currentUpdate;
	currentUpdate = p.time();
	
	// Update values.
	std::ostringstream measurementValue;
	measurementValue << p;
	getModule().updateMeasurementValue(measurementName, measurementValue.str());
}

/** Receive measurements. */
template<>
void WebGuiDriver< Measurement::ErrorPosition >::receive( const Measurement::ErrorPosition& p )
{		
	LOG4CPP_TRACE ( logger, "Received error position event on component with key '" << getKey() << "'" );

	// Get lock for the update timestamps.
	boost::upgrade_lock<boost::shared_mutex> lock (_updateAccess);

	// Get exclusive access.
	boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock (lock);

	// Remember last update.
	lastUpdate = currentUpdate;
	currentUpdate = p.time();

	// Update values.
	std::ostringstream measurementValue;
	measurementValue << Measurement::Position ( p.time(), (*p).value) << " --- RMS: "<< (*p).getRMS();
	LOG4CPP_INFO ( logger, "Received error position event on component with key '" << getKey() << "'" << " : "<<p);
	getModule().updateMeasurementValue (measurementName, measurementValue.str());
}

/** Detect timeout. */
template< class EventType >
void WebGuiDriver< EventType >::threadHandler()
{
	LOG4CPP_INFO( logger, "Measurement timeout detection thread started." );
	
	// Check regularly for timeout violations.
	while (m_keepRunning) {
		// Sleep.
		Util::sleep(100);
		
		// Check if last update is not too old.
		Measurement::Timestamp currentTime = Measurement::now();
		bool isRecent = false;
		bool isCorrectFrequency = false;
		{
			// Get shared lock for the update timestamps.
			boost::shared_lock<boost::shared_mutex> lock(_updateAccess);
			
			// Check if update is not too old and if the frequency is correct.
			isRecent = (currentTime - currentUpdate) <= timeout;
			isCorrectFrequency = (currentUpdate - lastUpdate) <= timeout;
		}
		// Update measurement status.
		if (isRecent && isCorrectFrequency) {
			getModule().updateMeasurement(this->measurementName, MeasurementStatusGreen);
		}
		else if (isRecent) {
			getModule().updateMeasurement(this->measurementName, MeasurementStatusYellow);
		}
		else {
			getModule().updateMeasurement(this->measurementName, MeasurementStatusRed);
		}
	}
}


boost::shared_ptr< WebGuiDriverBase > WebGuiModule::createComponent( const std::string& type, const std::string& name, 
	boost::shared_ptr< Graph::UTQLSubgraph > pConfig, const Dataflow::SubgraphIdKey& key, WebGuiModule* pModule )
{
    LOG4CPP_DEBUG( logger, "starting createComponent ************************");

	/** different measurement input types */
	if ( type == "WebGuiPose" )
		return boost::shared_ptr< WebGuiDriverBase >( new WebGuiDriver< Measurement::Pose > ( name, pConfig, key, pModule ) );
	else if ( type == "WebGuiPosition" )
		return boost::shared_ptr< WebGuiDriverBase >( new WebGuiDriver< Measurement::Position > ( name, pConfig, key, pModule ) );
	else if ( type == "WebGuiErrorPosition" )
		return boost::shared_ptr< WebGuiDriverBase >( new WebGuiDriver< Measurement::ErrorPosition > ( name, pConfig, key, pModule ) );

	/** generic event output type.
		TODO: split into lightweight components, do not use the monolithic WebGuiDriver< Measurement::Pose > to process button events. */
	else if ( type == "WebGuiEvent" )
		return boost::shared_ptr< WebGuiDriverBase >( new WebGuiDriver< Measurement::Pose > ( name, pConfig, key, pModule ) );

	UBITRACK_THROW( "Class " + type + " not supported by webgui module" );
}


/** register module at factory */
UBITRACK_REGISTER_COMPONENT( ComponentFactory* const cf )
{
	std::vector< std::string > components;

	components.push_back( "WebGuiPose" );
	components.push_back( "WebGuiPosition" );
	components.push_back( "WebGuiErrorPosition" );

	components.push_back( "WebGuiEvent" );

	cf->registerModule< WebGuiModule > ( components );
}

} // namespace Drivers
} // namespace Ubitrack
