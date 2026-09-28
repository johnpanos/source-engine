//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/tools/face_tool.h.
//
//=============================================================================//

#include "hammer/tools/face_tool.h"

#include "hammer/app/ops/texture_ops.h"
#include "hammer/scene/solid_geometry.h"

namespace hammer::tools
{

std::optional<scene::FaceRef> FaceTool::FaceAt(
    const ToolContext &ctx, viewport::ScreenPoint at ) const
{
	if ( !ctx.view.Is3D() )
		return std::nullopt;
	PickSettings picking = ctx.Picking();
	picking.entities = false;
	const std::optional<ObjectPick> pick =
	    PickObject3D( ctx.session.Document(), *ctx.view.camera3D, at.x, at.y, picking );
	if ( !pick || !pick->face.solid.IsValid() )
		return std::nullopt;
	return pick->face;
}

ToolResult FaceTool::OnPointer( ToolContext &ctx, const PointerEvent &event )
{
	if ( !ctx.view.Is3D() )
		return ToolResult::Ignore();
	if ( event.phase == PointerPhase::Down )
	{
		if ( m_gesture ||
		     ( event.button != PointerButton::Left && event.button != PointerButton::Right ) )
			return ToolResult::Ignore();
		m_gesture = Gesture{ event.button, event.Point(), event.modifiers };
		return ToolResult::Capture();
	}
	if ( !m_gesture )
		return ToolResult::Ignore();
	if ( ExceedsDragThreshold( m_gesture->down, event.Point() ) )
	{
		// A drag is not a face action: give the pointer back.
		m_gesture.reset();
		return ToolResult::Release();
	}
	if ( event.phase == PointerPhase::Move )
		return ToolResult::Handle();
	if ( event.button != m_gesture->button )
		return ToolResult::Ignore();
	const Gesture g = *m_gesture;
	m_gesture.reset();
	return g.button == PointerButton::Left ? LeftClick( ctx, g ) : RightClick( ctx, g );
}

ToolResult FaceTool::LeftClick( ToolContext &ctx, const Gesture &g )
{
	const std::optional<scene::FaceRef> face = FaceAt( ctx, g.down );
	if ( g.modifiers.Alt() )
	{
		if ( !face )
			return ToolResult::Fail( "no face under the pointer" );
		const scene::Side *side = app::ops::FindFace( ctx.session.Document(), *face );
		if ( !side )
			return ToolResult::Fail( "no face under the pointer" );
		m_lifted = side->texture;
		m_status = "Lifted " + side->texture.material;
		if ( m_onLift )
			m_onLift( *m_lifted );
		return ToolResult::Release();
	}
	if ( !face )
	{
		if ( g.modifiers.Ctrl() || ctx.session.CurrentSelection().faces.empty() )
			return ToolResult::Release();
		return ResultOf(
		    ctx.session.SelectFaces( {}, app::SelectMode::Replace ), ToolResult::Release() );
	}
	if ( g.modifiers.Shift() )
	{
		std::vector<scene::FaceRef> faces;
		for ( const scene::Side &side : ctx.session.Document().FindSolid( face->solid )->sides )
			faces.push_back( { face->solid, side.vmfId } );
		const app::SelectMode mode =
		    g.modifiers.Ctrl() ? app::SelectMode::Add : app::SelectMode::Replace;
		return ResultOf( ctx.session.SelectFaces( faces, mode ), ToolResult::Release() );
	}
	const app::SelectMode mode =
	    g.modifiers.Ctrl() ? app::SelectMode::Toggle : app::SelectMode::Replace;
	return ResultOf( ctx.session.SelectFaces( { *face }, mode ), ToolResult::Release() );
}

ToolResult FaceTool::RightClick( ToolContext &ctx, const Gesture &g )
{
	const std::optional<scene::FaceRef> face = FaceAt( ctx, g.down );
	if ( !face )
		return ToolResult::Fail( "no face under the pointer" );
	const std::string material = ctx.editor.faceTexture.material;
	const scene::FaceRef target = *face;
	if ( g.modifiers.Alt() )
	{
		const std::vector<scene::FaceRef> &selected = ctx.session.CurrentSelection().faces;
		const scene::Side *source =
		    selected.empty() ? nullptr
		                     : app::ops::FindFace( ctx.session.Document(), selected.front() );
		if ( !source )
			return ToolResult::Fail( "select a face to copy the alignment from" );
		scene::FaceTexture texture = source->texture;
		if ( !material.empty() )
			texture.material = material;
		m_status = "Applied " + texture.material + " with alignment";
		return ResultOf( ctx.session.Execute( "Apply texture",
		                     [texture, target]( scene::DocumentEdit &edit )
		                     {
			                     return app::ops::ApplyTextureFrom( edit, texture, { target },
			                         app::ops::ApplyTextureMode::MaterialValues );
		                     } ),
		    ToolResult::Release() );
	}
	if ( material.empty() )
		return ToolResult::Fail( "no active material" );
	m_status = "Applied " + material;
	return ResultOf( ctx.session.Execute( "Apply material",
	                     [material, target]( scene::DocumentEdit &edit )
	                     {
		                     return app::ops::ApplyMaterial( edit, { target }, material );
	                     } ),
	    ToolResult::Release() );
}

ToolResult FaceTool::OnKey( ToolContext &ctx, const KeyEvent &event )
{
	if ( !event.IsPress() || event.key != Key::Escape )
		return ToolResult::Ignore();
	if ( m_gesture )
	{
		Cancel( CancelReason::Escape );
		return ToolResult::Release();
	}
	if ( ctx.session.CurrentSelection().faces.empty() )
		return ToolResult::Ignore();
	return ResultOf(
	    ctx.session.SelectFaces( {}, app::SelectMode::Replace ), ToolResult::Handle() );
}

void FaceTool::Cancel( CancelReason reason )
{
	if ( m_gesture )
		m_status = std::string( "Cancelled (" ) + CancelReasonName( reason ) + ")";
	m_gesture.reset();
}

OverlayList FaceTool::Overlay( const ToolContext &ctx ) const
{
	OverlayList out;
	const scene::MapDocument &doc = ctx.session.Document();
	for ( const scene::FaceRef &ref : ctx.session.CurrentSelection().faces )
	{
		const scene::Solid *solid = doc.FindSolid( ref.solid );
		if ( !solid )
			continue;
		const mapgeometry::BrushSolid geometry = scene::BuildGeometry( *solid );
		for ( const mapgeometry::BrushFace &face : geometry.faces )
			if ( face.sourcePlane >= 0 && solid->sides[face.sourcePlane].vmfId == ref.side )
				out.Polygon( face.vertices, OverlayRole::Selection );
	}
	return out;
}

Cursor FaceTool::CursorFor( const ToolContext &ctx, double x, double y ) const
{
	if ( !ctx.view.Is3D() )
		return Cursor::Forbidden;
	return FaceAt( ctx, { x, y } ) ? Cursor::Hand : Cursor::Default;
}

} // namespace hammer::tools
