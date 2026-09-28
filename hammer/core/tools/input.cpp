//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/tools/input.h and the small
//			out-of-line parts of public/hammer/tools/tool.h.
//
//=============================================================================//

#include "hammer/tools/input.h"

#include "hammer/tools/tool.h"

#include <utility>

namespace hammer::tools
{

KeyEvent KeyEvent::Press( Key key, Modifiers modifiers )
{
	KeyEvent event;
	event.key = key;
	event.modifiers = modifiers;
	return event;
}

KeyEvent KeyEvent::Release( Key key, Modifiers modifiers )
{
	KeyEvent event = Press( key, modifiers );
	event.phase = KeyPhase::Release;
	return event;
}

KeyEvent KeyEvent::Char( char character, Modifiers modifiers, KeyPhase phase )
{
	KeyEvent event;
	event.key = Key::Character;
	event.character = ( character >= 'A' && character <= 'Z' )
	                      ? static_cast<char>( character - 'A' + 'a' )
	                      : character;
	event.modifiers = modifiers;
	event.phase = phase;
	return event;
}

const char *CancelReasonName( CancelReason reason )
{
	switch ( reason )
	{
	case CancelReason::Escape:
		return "escape";
	case CancelReason::FocusLost:
		return "focus lost";
	case CancelReason::CaptureLost:
		return "capture lost";
	case CancelReason::ToolSwitched:
		return "tool switched";
	case CancelReason::DocumentReplaced:
		return "document replaced";
	}
	return "unknown";
}

const char *ToolResultName( ToolResultKind kind )
{
	switch ( kind )
	{
	case ToolResultKind::Ignored:
		return "Ignored";
	case ToolResultKind::Handled:
		return "Handled";
	case ToolResultKind::CaptureRequested:
		return "CaptureRequested";
	case ToolResultKind::CaptureReleased:
		return "CaptureReleased";
	case ToolResultKind::Failed:
		return "Failed";
	}
	return "unknown";
}

const char *CursorName( Cursor cursor )
{
	switch ( cursor )
	{
	case Cursor::Default:
		return "default";
	case Cursor::Crosshair:
		return "crosshair";
	case Cursor::Move:
		return "move";
	case Cursor::ResizeN:
		return "resize-n";
	case Cursor::ResizeS:
		return "resize-s";
	case Cursor::ResizeE:
		return "resize-e";
	case Cursor::ResizeW:
		return "resize-w";
	case Cursor::ResizeNE:
		return "resize-ne";
	case Cursor::ResizeNW:
		return "resize-nw";
	case Cursor::ResizeSE:
		return "resize-se";
	case Cursor::ResizeSW:
		return "resize-sw";
	case Cursor::Rotate:
		return "rotate";
	case Cursor::Forbidden:
		return "forbidden";
	case Cursor::Hand:
		return "hand";
	case Cursor::Text:
		return "text";
	}
	return "unknown";
}

void OverlayList::Line(
    const mapgeometry::Vec3d &from, const mapgeometry::Vec3d &to, OverlayRole role )
{
	OverlayItem item;
	item.kind = OverlayKind::WorldLine;
	item.role = role;
	item.world = { from, to };
	items.push_back( std::move( item ) );
}

void OverlayList::Box(
    const mapgeometry::Vec3d &mins, const mapgeometry::Vec3d &maxs, OverlayRole role )
{
	OverlayItem item;
	item.kind = OverlayKind::WorldBox;
	item.role = role;
	item.world = { mins, maxs };
	items.push_back( std::move( item ) );
}

void OverlayList::Polygon( std::vector<mapgeometry::Vec3d> points, OverlayRole role )
{
	OverlayItem item;
	item.kind = OverlayKind::WorldPolygon;
	item.role = role;
	item.world = std::move( points );
	items.push_back( std::move( item ) );
}

void OverlayList::Rect( viewport::ScreenPoint a, viewport::ScreenPoint b, OverlayRole role )
{
	OverlayItem item;
	item.kind = OverlayKind::ScreenRect;
	item.role = role;
	item.a = a;
	item.b = b;
	items.push_back( std::move( item ) );
}

void OverlayList::Handle(
    viewport::ScreenPoint center, double halfSize, HandleShape shape, OverlayRole role )
{
	OverlayItem item;
	item.kind = OverlayKind::ScreenHandle;
	item.role = role;
	item.a = center;
	item.size = halfSize;
	item.shape = shape;
	items.push_back( std::move( item ) );
}

void OverlayList::Label( viewport::ScreenPoint anchor, std::string text, OverlayRole role )
{
	OverlayItem item;
	item.kind = OverlayKind::ScreenLabel;
	item.role = role;
	item.a = anchor;
	item.text = std::move( text );
	items.push_back( std::move( item ) );
}

void OverlayList::Append( const OverlayList &other )
{
	items.insert( items.end(), other.items.begin(), other.items.end() );
}

std::size_t OverlayList::Count( OverlayKind kind, OverlayRole role ) const
{
	std::size_t count = 0;
	for ( const OverlayItem &item : items )
		if ( item.kind == kind && item.role == role )
			++count;
	return count;
}

std::size_t OverlayList::CountRole( OverlayRole role ) const
{
	std::size_t count = 0;
	for ( const OverlayItem &item : items )
		if ( item.role == role )
			++count;
	return count;
}

viewport::GridPolicy GridFrom( const app::EditorSettings &editor )
{
	viewport::GridPolicy grid;
	int size = viewport::GridPolicy::kMinSize;
	while ( size < viewport::GridPolicy::kMaxSize && size * 1.5 <= editor.gridSize )
		size *= 2;
	grid.SetSize( size );
	grid.SetEnabled( editor.snapToGrid );
	return grid;
}

std::optional<viewport::ScreenPoint> ViewRef::Project( const mapgeometry::Vec3d &world ) const
{
	if ( Is2D() )
		return camera2D->WorldToScreen( world );
	if ( Is3D() )
		return camera3D->WorldToScreen( world );
	return std::nullopt;
}

} // namespace hammer::tools
