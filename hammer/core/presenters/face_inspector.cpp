//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/presenters/face_inspector.h.
//
//=============================================================================//

#include "hammer/presenters/face_inspector.h"

namespace hammer::presenters
{

using app::EditError;
using app::EditErrorCode;

namespace
{

void Fold( NumberField &field, double value )
{
	switch ( field.state )
	{
	case app::PropertyState::kUnset:
		field.state = app::PropertyState::kSingle;
		field.value = value;
		break;
	case app::PropertyState::kSingle:
		if ( field.value != value )
		{
			field.state = app::PropertyState::kMixed;
			field.value = 0.0;
		}
		break;
	case app::PropertyState::kMixed:
		break;
	}
}

const char *JustifyName( app::ops::Justification justification )
{
	switch ( justification )
	{
	case app::ops::Justification::Left:
		return "left";
	case app::ops::Justification::Right:
		return "right";
	case app::ops::Justification::Top:
		return "top";
	case app::ops::Justification::Bottom:
		return "bottom";
	case app::ops::Justification::Center:
		return "center";
	case app::ops::Justification::Fit:
		return "fit";
	}
	return "";
}

} // namespace

FaceInspector::FaceInspector( app::EditSession &session, const ports::IMaterialInfo *materials )
    : m_session( session ), m_materials( materials )
{
	m_subscription = m_session.Subscribe(
	    [this]( const app::SessionEvent & )
	    {
		    Rebuild();
	    } );
	Rebuild();
}

void FaceInspector::Rebuild()
{
	const scene::MapDocument &doc = m_session.Document();
	m_faces.clear();
	m_fields = FaceFields{};
	for ( const scene::FaceRef &face : m_session.CurrentSelection().faces )
	{
		const scene::Side *side = app::ops::FindFace( doc, face );
		if ( !side )
		{
			continue;
		}
		m_faces.push_back( face );
		const scene::FaceTexture &t = side->texture;
		m_fields.material = m_fields.material.AddContributor( t.material );
		Fold( m_fields.shiftU, t.u.shift );
		Fold( m_fields.shiftV, t.v.shift );
		Fold( m_fields.scaleU, t.u.scale );
		Fold( m_fields.scaleV, t.v.scale );
		Fold( m_fields.rotation, t.rotation );
		Fold( m_fields.lightmapScale, t.lightmapScale );
	}
	m_fields.materialKnown = !( m_materials && m_fields.material.IsSingle() &&
	                            !m_materials->Exists( m_fields.material.Value() ) );
	++m_revision;
}

FaceInspector::Result FaceInspector::Run(
    const std::string &label, const app::EditSession::Operation &operation )
{
	if ( m_faces.empty() )
	{
		m_lastError = "no faces selected";
		++m_revision;
		return foundation::MakeUnexpected( EditError{ EditErrorCode::Nothing, m_lastError } );
	}
	auto committed = m_session.Execute( label, operation );
	if ( !committed )
	{
		m_lastError = committed.Error().message;
		++m_revision;
		return foundation::MakeUnexpected( committed.Error() );
	}
	return {};
}

void FaceInspector::ClearError()
{
	if ( !m_lastError.empty() )
	{
		m_lastError.clear();
		++m_revision;
	}
}

FaceInspector::Result FaceInspector::SetValues( const app::ops::TextureValues &values )
{
	std::vector<std::string> parts;
	if ( values.shiftU || values.shiftV )
	{
		parts.push_back( "Set texture shift" );
	}
	if ( values.scaleU || values.scaleV )
	{
		parts.push_back( "Set texture scale" );
	}
	if ( values.rotation )
	{
		parts.push_back( "Set texture rotation" );
	}
	if ( values.lightmapScale )
	{
		parts.push_back( "Set lightmap scale" );
	}
	const std::string label = parts.size() == 1 ? parts.front() : "Set texture values";
	const std::vector<scene::FaceRef> faces = m_faces;
	return Run( label,
	    [&]( scene::DocumentEdit &edit )
	    {
		    return app::ops::SetTextureValues( edit, faces, values );
	    } );
}

FaceInspector::Result FaceInspector::Shift( double deltaU, double deltaV )
{
	const std::vector<scene::FaceRef> faces = m_faces;
	return Run( "Shift texture",
	    [&]( scene::DocumentEdit &edit )
	    {
		    return app::ops::ShiftTexture( edit, faces, deltaU, deltaV );
	    } );
}

FaceInspector::Result FaceInspector::Justify(
    app::ops::Justification justification, bool treatAsOne, int fitU, int fitV )
{
	if ( !m_materials )
	{
		m_lastError = "justify needs material sizes, and no material information is available";
		++m_revision;
		return foundation::MakeUnexpected( EditError{ EditErrorCode::Rejected, m_lastError } );
	}
	const std::vector<scene::FaceRef> faces = m_faces;
	return Run( std::string( "Justify texture " ) + JustifyName( justification ),
	    [&]( scene::DocumentEdit &edit )
	    {
		    return app::ops::JustifyTexture(
		        edit, faces, justification, *m_materials, treatAsOne, fitU, fitV );
	    } );
}

FaceInspector::Result FaceInspector::Align( app::ops::TextureAlignment alignment )
{
	const std::vector<scene::FaceRef> faces = m_faces;
	return Run( alignment == app::ops::TextureAlignment::World ? "Align texture to world"
	                                                           : "Align texture to face",
	    [&]( scene::DocumentEdit &edit )
	    {
		    return app::ops::AlignTexture( edit, faces, alignment );
	    } );
}

FaceInspector::Result FaceInspector::ApplyMaterial( const std::string &material )
{
	const std::vector<scene::FaceRef> faces = m_faces;
	return Run( "Apply material " + material,
	    [&]( scene::DocumentEdit &edit )
	    {
		    return app::ops::ApplyMaterial( edit, faces, material );
	    } );
}

} // namespace hammer::presenters
