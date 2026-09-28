//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/tools/tool_manager.h.
//
//=============================================================================//

#include "hammer/tools/tool_manager.h"

namespace hammer::tools
{

ITool *ToolManager::Add( std::unique_ptr<ITool> tool )
{
	if ( !tool || Find( tool->Name() ) )
		return nullptr;
	m_tools.push_back( std::move( tool ) );
	return m_tools.back().get();
}

ITool *ToolManager::Find( std::string_view name ) const
{
	for ( const std::unique_ptr<ITool> &tool : m_tools )
		if ( tool->Name() == name )
			return tool.get();
	return nullptr;
}

std::vector<std::string_view> ToolManager::Names() const
{
	std::vector<std::string_view> names;
	for ( const std::unique_ptr<ITool> &tool : m_tools )
		names.push_back( tool->Name() );
	return names;
}

bool ToolManager::Activate( std::string_view name )
{
	ITool *tool = Find( name );
	if ( !tool )
		return false;
	if ( tool == m_active )
		return true;
	DeactivateAll();
	m_active = tool;
	return true;
}

void ToolManager::DeactivateAll()
{
	if ( m_active )
	{
		if ( m_active->InGesture() )
			m_active->Cancel( CancelReason::ToolSwitched );
		m_active->Deactivate();
	}
	m_active = nullptr;
	m_capture.reset();
}

ToolResult ToolManager::Finish( ToolResult result, viewport::ViewKind view )
{
	if ( result.Is( ToolResultKind::CaptureRequested ) )
		m_capture = view;
	if ( m_capture && ( !m_active || !m_active->InGesture() ) )
	{
		m_capture.reset();
		if ( result.Is( ToolResultKind::Handled ) || result.Is( ToolResultKind::CaptureRequested ) )
			result.kind = ToolResultKind::CaptureReleased;
	}
	return result;
}

ToolResult ToolManager::OnPointer( ToolContext &ctx, const PointerEvent &event )
{
	if ( !ctx.view.Valid() || ctx.view.kind != event.view )
		return ToolResult::Fail( "the event's view does not match the tool context" );
	if ( !m_active )
		return ToolResult::Ignore();
	if ( m_capture )
	{
		if ( *m_capture != event.view )
			return ToolResult::Ignore();
	}
	else if ( !m_active->SupportedViews().Contains( event.view ) )
	{
		return ToolResult::Ignore();
	}
	return Finish( m_active->OnPointer( ctx, event ), event.view );
}

ToolResult ToolManager::OnKey( ToolContext &ctx, const KeyEvent &event )
{
	if ( !ctx.view.Valid() )
		return ToolResult::Fail( "the tool context has no view" );
	if ( !m_active )
		return ToolResult::Ignore();
	if ( m_capture && *m_capture != ctx.view.kind )
		return ToolResult::Ignore();
	if ( !m_capture && !m_active->SupportedViews().Contains( ctx.view.kind ) )
		return ToolResult::Ignore();
	return Finish( m_active->OnKey( ctx, event ), ctx.view.kind );
}

ToolResult ToolManager::OnWheel( ToolContext &ctx, const WheelEvent &event )
{
	if ( !ctx.view.Valid() || ctx.view.kind != event.view )
		return ToolResult::Fail( "the event's view does not match the tool context" );
	if ( !m_active || !m_active->SupportedViews().Contains( event.view ) )
		return ToolResult::Ignore();
	if ( m_capture && *m_capture != event.view )
		return ToolResult::Ignore();
	return Finish( m_active->OnWheel( ctx, event ), event.view );
}

void ToolManager::CancelGesture( CancelReason reason )
{
	if ( m_active && m_active->InGesture() )
		m_active->Cancel( reason );
	m_capture.reset();
}

void ToolManager::OnFocusLost()
{
	CancelGesture( CancelReason::FocusLost );
}

void ToolManager::OnCaptureLost()
{
	CancelGesture( CancelReason::CaptureLost );
}

OverlayList ToolManager::Overlay( const ToolContext &ctx ) const
{
	if ( !m_active || !ctx.view.Valid() || !m_active->SupportedViews().Contains( ctx.view.kind ) )
		return {};
	return m_active->Overlay( ctx );
}

Cursor ToolManager::CursorFor( const ToolContext &ctx, double x, double y ) const
{
	if ( !m_active || !ctx.view.Valid() )
		return Cursor::Default;
	if ( !m_capture && !m_active->SupportedViews().Contains( ctx.view.kind ) )
		return Cursor::Forbidden;
	return m_active->CursorFor( ctx, x, y );
}

std::string ToolManager::Status() const
{
	return m_active ? m_active->Status() : std::string();
}

} // namespace hammer::tools
