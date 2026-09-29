//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/presenters/editor_workspace.h.
//
//=============================================================================//

#include "hammer/presenters/editor_workspace.h"

#include "hammer/tools/block_tool.h"
#include "hammer/tools/clip_tool.h"
#include "hammer/tools/entity_tool.h"
#include "hammer/tools/face_tool.h"
#include "hammer/tools/selection_tool.h"
#include "hammer/tools/vertex_tool.h"
#include "mapgeometry/vec3.h"

#include <algorithm>

namespace hammer::presenters
{

using tools::ToolResultKind;
using viewport::ViewKind;

namespace
{

constexpr double kFrameMarginPixels = 32.0;

const char *KeyName( tools::Key key )
{
	switch ( key )
	{
	case tools::Key::Escape:
		return "Escape";
	case tools::Key::Enter:
		return "Enter";
	case tools::Key::Delete:
		return "Delete";
	case tools::Key::Backspace:
		return "Backspace";
	case tools::Key::Left:
		return "Left";
	case tools::Key::Right:
		return "Right";
	case tools::Key::Up:
		return "Up";
	case tools::Key::Down:
		return "Down";
	case tools::Key::PageUp:
		return "PageUp";
	case tools::Key::PageDown:
		return "PageDown";
	case tools::Key::Home:
		return "Home";
	case tools::Key::End:
		return "End";
	case tools::Key::Tab:
		return "Tab";
	case tools::Key::Space:
		return "Space";
	case tools::Key::F1:
		return "F1";
	case tools::Key::F2:
		return "F2";
	case tools::Key::F3:
		return "F3";
	case tools::Key::F4:
		return "F4";
	case tools::Key::F5:
		return "F5";
	case tools::Key::F6:
		return "F6";
	case tools::Key::F7:
		return "F7";
	case tools::Key::F8:
		return "F8";
	case tools::Key::F9:
		return "F9";
	case tools::Key::F10:
		return "F10";
	case tools::Key::F11:
		return "F11";
	case tools::Key::F12:
		return "F12";
	case tools::Key::None:
	case tools::Key::Character:
		break;
	}
	return "";
}

viewport::SelectionInput InputOf( const app::Selection &selection )
{
	return { selection.objects, selection.faces };
}

} // namespace

EditorWorkspace::EditorWorkspace( const WorkspaceServices &services )
    : m_services( services ),
      m_commands( m_session, m_settings,
          app::SessionServices{ services.codec, services.store, services.builder, services.catalog,
              services.materials, &m_clipboard } ),
      m_instances( services.codec && services.store
                       ? std::make_unique<app::InstancePreview>(
                             *services.codec, *services.store, services.catalog )
                       : nullptr ),
      m_snapshot( viewport::ExtractOptions{
          services.catalog, m_instances.get(), {}, false, viewport::kDefaultPointHalfSize } )
{
	if ( m_instances )
	{
		m_instances->SetSearchRoots( services.instanceRoots );
	}
	m_tools.Add( std::make_unique<tools::SelectionTool>() );
	m_tools.Add( std::make_unique<tools::BlockTool>() );
	m_tools.Add( std::make_unique<tools::EntityTool>() );
	m_tools.Add( std::make_unique<tools::ClipTool>() );
	m_tools.Add( std::make_unique<tools::VertexTool>() );
	auto face = std::make_unique<tools::FaceTool>();
	face->SetLiftCallback(
	    [this]( const scene::FaceTexture &texture )
	    {
		    m_settings.faceTexture = texture;
	    } );
	m_tools.Add( std::move( face ) );
	m_tools.Activate( tools::SelectionTool::kName );

	m_cameras2D[0].SetKind( ViewKind::Top );
	m_cameras2D[1].SetKind( ViewKind::Front );
	m_cameras2D[2].SetKind( ViewKind::Side );

	m_snapshot.Rebuild(
	    m_session.Document(), InputOf( m_session.CurrentSelection() ), m_session.Revision() );
	m_subscription = m_session.Subscribe(
	    [this]( const app::SessionEvent &event )
	    {
		    OnSessionEvent( event );
	    } );

	m_inspector = std::make_unique<EntityInspector>( m_session, services.catalog );
	m_faceInspector = std::make_unique<FaceInspector>( m_session, services.materials );
	m_outliner = std::make_unique<Outliner>( m_session );
	if ( services.catalog )
	{
		m_classPalette =
		    std::make_unique<ClassPalette>( *services.catalog, m_settings, &m_session );
	}
	if ( services.materials )
	{
		m_materialBrowser =
		    std::make_unique<MaterialBrowser>( m_session, *services.materials, m_settings );
	}
	m_history = std::make_unique<HistoryPanel>( m_session );
	m_statusBar = std::make_unique<StatusBar>( m_session, m_settings );
	m_visgroups = std::make_unique<VisgroupPanel>( m_session, m_commands );
	m_problems = std::make_unique<ProblemsPanel>(
	    m_session, m_commands, services.catalog, services.materials );
	m_actions = std::make_unique<ActionCatalog>( m_session, m_settings, m_commands, &m_clipboard );
	m_statusBar->SetTool( std::string( tools::SelectionTool::kName ) );
	FrameDocument();
}

EditorWorkspace::~EditorWorkspace() = default;

viewport::Camera2D &EditorWorkspace::Camera2DFor( ViewKind view )
{
	switch ( view )
	{
	case ViewKind::Front:
		return m_cameras2D[1];
	case ViewKind::Side:
		return m_cameras2D[2];
	case ViewKind::Top:
	case ViewKind::Camera3D:
		break;
	}
	return m_cameras2D[0];
}

void EditorWorkspace::SetViewportSize( ViewKind view, int width, int height )
{
	if ( view == ViewKind::Camera3D )
	{
		m_camera3D.SetViewport( width, height );
	}
	else
	{
		Camera2DFor( view ).SetViewport( width, height );
	}
}

tools::ToolContext EditorWorkspace::ContextFor( ViewKind view )
{
	const tools::ViewRef ref = view == ViewKind::Camera3D
	                               ? tools::ViewRef::Of( m_camera3D )
	                               : tools::ViewRef::Of( Camera2DFor( view ) );
	return tools::ToolContext::Make(
	    m_session, ref, m_settings, m_toolSettings, m_services.catalog, m_services.materials );
}

void EditorWorkspace::OnSessionEvent( const app::SessionEvent &event )
{
	if ( event.kind != app::SessionEventKind::Saved )
	{
		++m_sceneSerial;
	}
	const scene::MapDocument &doc = m_session.Document();
	const viewport::SelectionInput selection = InputOf( m_session.CurrentSelection() );
	switch ( event.kind )
	{
	case app::SessionEventKind::Edited:
	case app::SessionEventKind::Undone:
	case app::SessionEventKind::Redone:
		if ( event.changes )
		{
			m_snapshot.Update(
			    doc, *event.changes, selection, m_snapshot.Revision(), event.revision );
		}
		else
		{
			m_snapshot.Rebuild( doc, selection, event.revision );
		}
		++m_selectionSerial;
		break;
	case app::SessionEventKind::Replaced:
		m_tools.CancelGesture( tools::CancelReason::DocumentReplaced );
		m_snapshot.Rebuild( doc, selection, event.revision );
		++m_selectionSerial;
		FrameDocument();
		break;
	case app::SessionEventKind::SelectionChanged:
		m_snapshot.SetSelection( doc, selection );
		++m_selectionSerial;
		break;
	case app::SessionEventKind::Saved:
		break;
	}
}

void EditorWorkspace::FrameDocument()
{
	const std::optional<scene::Box> &bounds = m_snapshot.Snapshot().bounds;
	for ( viewport::Camera2D &camera : m_cameras2D )
	{
		if ( !bounds )
		{
			camera.SetCenter( {} );
		}
		else if ( !camera.HasArea() || !camera.Frame( *bounds, kFrameMarginPixels ) )
		{
			camera.SetCenter( camera.ToPlane( bounds->Center() ) );
		}
	}
	if ( !bounds )
	{
		m_camera3D.SetPosition( mapgeometry::Vec3d( -512, -512, 384 ) );
		m_camera3D.LookAt( mapgeometry::Vec3d( 0, 0, 0 ) );
		return;
	}
	const mapgeometry::Vec3d center = bounds->Center();
	m_camera3D.SetPosition( center + mapgeometry::Vec3d( -1, -1, 0.75 ) );
	m_camera3D.LookAt( center );
	m_camera3D.Frame( *bounds );
}

InputOutcome EditorWorkspace::Finish(
    InputOutcome outcome, std::uint64_t revisionBefore, std::uint64_t selectionBefore )
{
	outcome.redraw = outcome.redraw || outcome.handled || m_session.Revision() != revisionBefore ||
	                 m_selectionSerial != selectionBefore;
	if ( !outcome.status.empty() )
	{
		m_statusBar->ShowMessage( outcome.status );
	}
	return outcome;
}

InputOutcome EditorWorkspace::OnPointer( ViewKind view, const tools::PointerEvent &event )
{
	const std::uint64_t revision = m_session.Revision();
	const std::uint64_t selection = m_selectionSerial;
	InputOutcome out;
	tools::PointerEvent e = event;
	e.view = view;
	const tools::ToolResult camera = view == ViewKind::Camera3D
	                                     ? m_cameraControl.OnPointer( m_camera3D, e )
	                                     : m_cameraControl.OnPointer( Camera2DFor( view ), e );
	if ( !camera.Is( ToolResultKind::Ignored ) )
	{
		if ( camera.Is( ToolResultKind::CaptureRequested ) )
		{
			m_tools.OnCaptureLost();
		}
		out.handled = true;
		out.redraw = true;
		return Finish( out, revision, selection );
	}
	tools::ToolContext ctx = ContextFor( view );
	const tools::ToolResult result = m_tools.OnPointer( ctx, e );
	out.handled = !result.Is( ToolResultKind::Ignored );
	if ( result.Is( ToolResultKind::Failed ) )
	{
		out.status = result.message;
	}
	return Finish( out, revision, selection );
}

std::string EditorWorkspace::ChordOf( const tools::KeyEvent &event )
{
	std::string chord;
	chord += event.modifiers.Ctrl() ? "Ctrl+" : "";
	chord += event.modifiers.Alt() ? "Alt+" : "";
	chord += event.modifiers.Shift() ? "Shift+" : "";
	if ( event.key == tools::Key::Character )
	{
		chord += std::string( 1, event.character );
	}
	else
	{
		chord += KeyName( event.key );
	}
	return NormalizeChord( chord ).value_or( std::string() );
}

InputOutcome EditorWorkspace::ExecuteAction( const ActionSpec &spec )
{
	InputOutcome out;
	out.actionId = spec.id;
	out.handled = true;
	if ( spec.target == ActionTarget::Command && m_tools.Active() && m_tools.Active()->InGesture() )
	{
		m_tools.OnCaptureLost();
	}
	app::CommandArgs hostArgs;
	if ( !spec.hostArgs.empty() )
	{
		if ( spec.id == "file.open" )
		{
			out.hostRequest = "open_dialog";
			return out;
		}
		if ( m_path.empty() )
		{
			out.hostRequest = "save_dialog";
			return out;
		}
		hostArgs["path"] = m_path;
	}
	ActionResult result = m_actions->Execute( spec.id, hostArgs );
	out.status = m_actions->LastMessage();
	if ( !result )
	{
		return out;
	}
	switch ( result.Value().target )
	{
	case ActionTarget::Tool:
		m_tools.Activate( result.Value().tool );
		m_statusBar->SetTool( result.Value().tool );
		break;
	case ActionTarget::Host:
		out.hostRequest = result.Value().host;
		break;
	case ActionTarget::Command:
		if ( result.Value().runAfter )
		{
			out.hostRequest = "run_map";
		}
		break;
	}
	return out;
}

InputOutcome EditorWorkspace::RunAction( const std::string &id )
{
	const std::uint64_t revision = m_session.Revision();
	const std::uint64_t selection = m_selectionSerial;
	const ActionSpec *spec = ActionCatalog::Find( id );
	InputOutcome out;
	if ( !spec || !m_actions->IsEnabled( id ) )
	{
		(void)m_actions->Execute( id );
		out.status = m_actions->LastMessage();
		out.actionId = id;
		return Finish( out, revision, selection );
	}
	return Finish( ExecuteAction( *spec ), revision, selection );
}

InputOutcome EditorWorkspace::OnKey( ViewKind view, const tools::KeyEvent &event )
{
	const std::uint64_t revision = m_session.Revision();
	const std::uint64_t selection = m_selectionSerial;
	InputOutcome out;
	auto cameraKey = [&]()
	{
		return view == ViewKind::Camera3D ? m_cameraControl.OnKey( event )
		                                  : m_cameraControl.OnKey( Camera2DFor( view ), event );
	};
	bool cameraOffered = false;
	if ( event.IsPress() && ( m_cameraControl.HasCapture() || m_cameraControl.Flying() ) )
	{
		cameraOffered = true;
		if ( !cameraKey().Is( ToolResultKind::Ignored ) )
		{
			out.handled = true;
			return Finish( out, revision, selection );
		}
	}
	std::string disabled;
	if ( event.IsPress() )
	{
		if ( const ActionSpec *spec = ActionCatalog::FindByShortcut( ChordOf( event ) ) )
		{
			if ( m_actions->IsEnabled( spec->id ) )
			{
				return Finish( ExecuteAction( *spec ), revision, selection );
			}
			disabled = m_actions->DisplayLabel( spec->id ) + " is not available now";
		}
	}
	if ( !cameraOffered && !cameraKey().Is( ToolResultKind::Ignored ) )
	{
		out.handled = true;
		return Finish( out, revision, selection );
	}
	tools::ToolContext ctx = ContextFor( view );
	const tools::ToolResult result = m_tools.OnKey( ctx, event );
	out.handled = !result.Is( ToolResultKind::Ignored );
	if ( result.Is( ToolResultKind::Failed ) )
	{
		out.status = result.message;
	}
	else if ( !out.handled && !disabled.empty() )
	{
		out.status = disabled;
	}
	return Finish( out, revision, selection );
}

InputOutcome EditorWorkspace::OnWheel( ViewKind view, const tools::WheelEvent &event )
{
	const std::uint64_t revision = m_session.Revision();
	const std::uint64_t selection = m_selectionSerial;
	InputOutcome out;
	tools::WheelEvent e = event;
	e.view = view;
	const tools::ToolResult camera = view == ViewKind::Camera3D
	                                     ? m_cameraControl.OnWheel( m_camera3D, e )
	                                     : m_cameraControl.OnWheel( Camera2DFor( view ), e );
	if ( !camera.Is( ToolResultKind::Ignored ) )
	{
		out.handled = true;
		return Finish( out, revision, selection );
	}
	tools::ToolContext ctx = ContextFor( view );
	const tools::ToolResult result = m_tools.OnWheel( ctx, e );
	out.handled = !result.Is( ToolResultKind::Ignored );
	if ( result.Is( ToolResultKind::Failed ) )
	{
		out.status = result.message;
	}
	return Finish( out, revision, selection );
}

InputOutcome EditorWorkspace::OnFocusLost()
{
	const std::uint64_t revision = m_session.Revision();
	const std::uint64_t selection = m_selectionSerial;
	m_cameraControl.OnFocusLost();
	m_tools.OnFocusLost();
	InputOutcome out;
	out.redraw = true;
	return Finish( out, revision, selection );
}

InputOutcome EditorWorkspace::Advance( double seconds )
{
	InputOutcome out;
	out.redraw = m_cameraControl.Advance( m_camera3D, seconds );
	out.handled = out.redraw;
	return out;
}

void EditorWorkspace::SetInstanceDocumentPath( const std::string &path )
{
	if ( !m_instances )
	{
		return;
	}
	const std::uint64_t before = m_instances->Revision();
	m_instances->SetDocumentPath( path );
	if ( m_instances->Revision() != before )
	{
		m_snapshot.SetSelection( m_session.Document(), InputOf( m_session.CurrentSelection() ) );
		++m_sceneSerial;
	}
}

void EditorWorkspace::SetInstanceRoots( std::vector<std::string> roots )
{
	if ( !m_instances )
	{
		return;
	}
	const std::uint64_t before = m_instances->Revision();
	m_instances->SetSearchRoots( std::move( roots ) );
	if ( m_instances->Revision() != before )
	{
		m_snapshot.SetSelection( m_session.Document(), InputOf( m_session.CurrentSelection() ) );
		++m_sceneSerial;
	}
}

bool EditorWorkspace::RefreshInstances()
{
	if ( !m_instances || !m_instances->Refresh() )
	{
		return false;
	}
	m_snapshot.SetSelection( m_session.Document(), InputOf( m_session.CurrentSelection() ) );
	++m_sceneSerial;
	return true;
}

app::CommandResult EditorWorkspace::New()
{
	const std::string previous = m_path;
	if ( m_instances )
	{
		m_instances->SetDocumentPath( std::string() );
	}
	app::CommandResult result = m_commands.Execute( "new_map", {} );
	if ( !result )
	{
		SetInstanceDocumentPath( previous );
	}
	if ( result )
	{
		m_path.clear();
		m_statusBar->ShowMessage( "New map" );
	}
	else
	{
		m_statusBar->ShowMessage( "New: " + result.Error().detail );
	}
	return result;
}

app::CommandResult EditorWorkspace::Open( const std::string &path )
{
	// The instance lookups of the opened map use its path from the first
	// extraction (the Replaced event).
	const std::string previous = m_path;
	if ( m_instances )
	{
		m_instances->SetDocumentPath( path );
	}
	app::CommandResult result = m_commands.Execute( "open", { { "path", path } } );
	if ( !result )
	{
		SetInstanceDocumentPath( previous );
	}
	if ( result )
	{
		m_path = path;
		m_statusBar->ShowMessage( "Opened " + path );
	}
	else
	{
		m_statusBar->ShowMessage( "Open: " + result.Error().detail );
	}
	return result;
}

app::CommandResult EditorWorkspace::Save( const std::string &path )
{
	app::CommandResult result = m_commands.Execute( "save", { { "path", path } } );
	if ( result )
	{
		m_path = path;
		SetInstanceDocumentPath( path );
		m_statusBar->ShowMessage( "Saved " + path );
	}
	else
	{
		m_statusBar->ShowMessage( "Save: " + result.Error().detail );
	}
	return result;
}

app::CommandResult EditorWorkspace::Build(
    const std::string &path, bool run, std::string *hostRequest )
{
	app::CommandResult result =
	    m_commands.Execute( "build_map", { { "path", path }, { "publish", run ? "1" : "0" } } );
	if ( result )
	{
		m_path = path;
		SetInstanceDocumentPath( path );
		m_statusBar->ShowMessage( "Built " + path );
		if ( run && hostRequest )
		{
			*hostRequest = "run_map";
		}
	}
	else
	{
		m_statusBar->ShowMessage( "Build: " + result.Error().detail );
	}
	return result;
}

tools::OverlayList EditorWorkspace::Overlay( ViewKind view )
{
	tools::ToolContext ctx = ContextFor( view );
	return m_tools.Overlay( ctx );
}

tools::Cursor EditorWorkspace::CursorFor( ViewKind view, double x, double y )
{
	tools::ToolContext ctx = ContextFor( view );
	return m_tools.CursorFor( ctx, x, y );
}

std::vector<viewport::GridLine> EditorWorkspace::GridLines( ViewKind view )
{
	if ( view == ViewKind::Camera3D )
	{
		return {};
	}
	return viewport::VisibleLines( Camera2DFor( view ), tools::GridFrom( m_settings ) );
}

} // namespace hammer::presenters
