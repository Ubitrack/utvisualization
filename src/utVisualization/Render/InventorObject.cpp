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

#include "InventorObject.h"

#include <Inventor/SoSceneManager.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/nodes/SoTransparencyType.h>

namespace Ubitrack { namespace Drivers {


InventorObject::InventorObject( const std::string& name, boost::shared_ptr< Graph::UTQLSubgraph > subgraph, 
	const VirtualObjectKey& componentKey, VirtualCamera* pModule )
	: TrackedObject( name, subgraph, componentKey, pModule )
{
	// load object path
	Graph::UTQLSubgraph::NodePtr objectNode = subgraph->getNode( "Object" );
	if ( !objectNode )
		UBITRACK_THROW( "No Object node in RenderModule configuration" );

	std::string path = objectNode->getAttribute( "virtualObjectInventorPath" ).getText();
	if ( path.length() == 0)
		UBITRACK_THROW( "VirtualObject component with empty virtualObjectInventorPath  attribute" );
	
	// Fixme: at the moment we look at the file extension to
	// decide if we should try the .iv or the .wrl loader.
	bool isVRML = true;
	std::string::size_type suffixpos = path.find_last_of(".");
	
	if (suffixpos != std::string::npos) {
		std::string suffix = path.substr(suffixpos);
		// Is there a std::string::to_upper()?
		if (suffix.compare( "iv" ) || suffix.compare( "IV" ) || 
			suffix.compare( "Iv" ) || suffix.compare( "iV" ) ) {
			isVRML = false;
		}
	}
		
	// load inventor
	SoDB::init();
	SoInput sceneInput;
	if (!sceneInput.openFile(path.c_str()))
		UBITRACK_THROW( "Could not load inventor/wrl file." );
	if (!isVRML) // read .iv files
		m_rootNode = SoDB::readAll(&sceneInput);
	else // read vrml files
		m_rootNode = reinterpret_cast<SoNode*>( SoDB::readAllVRML(&sceneInput) );
	if (!m_rootNode)
		UBITRACK_THROW( "Unable to get root node." );
	// The loader does not increase reference count,
	// we have to do it manually
	m_rootNode->ref();
	SoDB::setRealTimeInterval(1/60.0);
}

InventorObject::~InventorObject() {
	if (m_rootNode) {
		// coin will delete the object if not longer referenced.
		m_rootNode->unref();
	}
}

/** render the object, if up-to-date tracking information is available */
void InventorObject::draw3DContent( Measurement::Timestamp& t, int parity )
{
	SoDB::getSensorManager()->processTimerQueue();
	SoDB::getSensorManager()->processDelayQueue(TRUE);
	
	// We need the viewport size, so we query the GL context.
	// Not sure if this is 100% sane... (probably not)
	GLint viewportSize[4];
	glGetIntegerv(GL_VIEWPORT, viewportSize); 
	SbViewportRegion region(viewportSize[2], viewportSize[3]);
	SoGLRenderAction renderAction(region);
	
	renderAction.apply(m_rootNode);
}

} } // namespace Ubitrack::Drivers

