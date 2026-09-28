//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/presenters/status_bar.h.
//
//=============================================================================//

#include "hammer/presenters/status_bar.h"

#include "hammer/app/ops/texture_ops.h"
#include "hammer/scene/map_queries.h"
#include "hammer/scene/solid_geometry.h"
#include "presenter_text.h"

#include <cmath>
#include <optional>

namespace hammer::presenters
{

namespace
{

std::string Rounded( double value )
{
	return scene::FormatNumber( std::round( value * 100.0 ) / 100.0 );
}

double Axis( const mapgeometry::Vec3d &v, int axis )
{
	return axis == 0 ? v.x : axis == 1 ? v.y : v.z;
}

const char *AxisName( int axis )
{
	return axis == 0 ? "x" : axis == 1 ? "y" : "z";
}

} // namespace

std::string SelectionSummary( const scene::DocumentReader &doc, const app::Selection &selection )
{
	std::size_t solids = 0;
	std::size_t entities = 0;
	std::size_t groups = 0;
	std::string entityClass;
	bool oneClass = true;
	for ( scene::ObjectId id : selection.objects )
	{
		if ( doc.FindSolid( id ) )
		{
			++solids;
		}
		else if ( const scene::Entity *e = doc.FindEntity( id ) )
		{
			if ( entities == 0 )
			{
				entityClass = e->classname;
			}
			else if ( !detail::EqualsNoCase( entityClass, e->classname ) )
			{
				oneClass = false;
			}
			++entities;
		}
		else if ( doc.FindGroup( id ) )
		{
			++groups;
		}
	}
	std::size_t faces = 0;
	for ( const scene::FaceRef &face : selection.faces )
	{
		faces += app::ops::FindFace( doc, face ) ? 1 : 0;
	}
	std::string text;
	auto add = [&]( const std::string &part )
	{
		text += ( text.empty() ? "" : ", " ) + part;
	};
	if ( solids )
	{
		add( detail::Counted( solids, "solid", "solids" ) );
	}
	if ( entities )
	{
		add( detail::Counted( entities, "entity", "entities" ) +
		     ( oneClass && !entityClass.empty() ? " (" + entityClass + ")" : std::string() ) );
	}
	if ( groups )
	{
		add( detail::Counted( groups, "group", "groups" ) );
	}
	if ( faces )
	{
		add( detail::Counted( faces, "face", "faces" ) );
	}
	return text.empty() ? std::string( "No selection" ) : text;
}

std::string SelectionSizeText( const scene::DocumentReader &doc, const app::Selection &selection )
{
	std::optional<scene::Box> bounds = scene::ObjectsBounds( doc, selection.objects );
	for ( const scene::FaceRef &face : selection.faces )
	{
		const scene::Solid *solid = doc.FindSolid( face.solid );
		if ( !solid )
		{
			continue;
		}
		const mapgeometry::BrushSolid geometry = scene::BuildGeometry( *solid );
		for ( const mapgeometry::BrushFace &polygon : geometry.faces )
		{
			if ( polygon.sourcePlane < 0 ||
			     static_cast<std::size_t>( polygon.sourcePlane ) >= solid->sides.size() ||
			     solid->sides[polygon.sourcePlane].vmfId != face.side )
			{
				continue;
			}
			for ( const mapgeometry::Vec3d &v : polygon.vertices )
			{
				if ( bounds )
				{
					bounds->Extend( v );
				}
				else
				{
					bounds = scene::PointBox( v );
				}
			}
		}
	}
	if ( !bounds )
	{
		return std::string();
	}
	const mapgeometry::Vec3d size = bounds->Size();
	return "w " + scene::FormatNumber( size.x ) + " h " + scene::FormatNumber( size.z ) + " d " +
	       scene::FormatNumber( size.y );
}

StatusBar::StatusBar( app::EditSession &session, const app::EditorSettings &settings )
    : m_session( session ), m_settings( settings )
{
	m_subscription = m_session.Subscribe(
	    [this]( const app::SessionEvent &event )
	    {
		    const bool documentChanged = event.kind == app::SessionEventKind::Edited ||
		                                 event.kind == app::SessionEventKind::Undone ||
		                                 event.kind == app::SessionEventKind::Redone ||
		                                 event.kind == app::SessionEventKind::Replaced;
		    Rebuild( documentChanged );
	    } );
	Rebuild( false );
}

void StatusBar::Rebuild( bool documentChanged )
{
	const scene::MapDocument &doc = m_session.Document();
	m_selectionText = SelectionSummary( doc, m_session.CurrentSelection() );
	m_sizeText = SelectionSizeText( doc, m_session.CurrentSelection() );
	if ( documentChanged )
	{
		m_message.clear();
	}
	++m_revision;
}

void StatusBar::SetPointer( viewport::ViewKind view, const mapgeometry::Vec3d &world )
{
	std::string text;
	if ( view == viewport::ViewKind::Camera3D )
	{
		text = "x " + Rounded( world.x ) + " y " + Rounded( world.y ) + " z " + Rounded( world.z );
	}
	else
	{
		const viewport::ViewAxes axes = viewport::AxesOf( view );
		const int first = axes.u < axes.v ? axes.u : axes.v;
		const int second = axes.u < axes.v ? axes.v : axes.u;
		text = std::string( AxisName( first ) ) + " " + Rounded( Axis( world, first ) ) + " " +
		       AxisName( second ) + " " + Rounded( Axis( world, second ) );
	}
	if ( text != m_pointerText )
	{
		m_pointerText = std::move( text );
		++m_revision;
	}
}

void StatusBar::ClearPointer()
{
	if ( !m_pointerText.empty() )
	{
		m_pointerText.clear();
		++m_revision;
	}
}

std::string StatusBar::GridText() const
{
	return "Grid " + scene::FormatNumber( m_settings.gridSize );
}

std::string StatusBar::SnapText() const
{
	return m_settings.snapToGrid ? "Snap on" : "Snap off";
}

void StatusBar::SetTool( const std::string &name )
{
	if ( name != m_toolText )
	{
		m_toolText = name;
		++m_revision;
	}
}

void StatusBar::ShowError( const app::EditError &error )
{
	ShowMessage( error.message );
}

void StatusBar::ShowMessage( const std::string &message )
{
	m_message = message;
	++m_revision;
}

void StatusBar::Report( const foundation::Expected<void, app::EditError> &result )
{
	if ( !result )
	{
		ShowError( result.Error() );
	}
}

void StatusBar::ClearMessage()
{
	if ( !m_message.empty() )
	{
		m_message.clear();
		++m_revision;
	}
}

} // namespace hammer::presenters
