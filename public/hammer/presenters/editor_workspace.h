//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The editor workspace (RFC 0002, hammer.presenters): the one object
//			a UI shell binds. It composes -- it never chooses providers; the
//			host passes them in WorkspaceServices -- one document's editing
//			state, its tools, its four views and every presenter, and routes
//			normalized input through one documented policy.
//
//			Owns, in construction order (destruction is the reverse, so
//			presenters and subscriptions go before the session):
//			  app::EditSession, app::EditorSettings, the app::MapFragment
//			  clipboard, app::SessionCommands (over the services);
//			  tools::ToolManager with every tool registered (selection, block,
//			  entity, clip, vertex, face; the face tool's lift writes
//			  EditorSettings::faceTexture), tools::ToolSettings and a
//			  tools::CameraController; the Selection tool starts active;
//			  three viewport::Camera2D (Top, Front, Side) and a Camera3D;
//			  a viewport::SnapshotCache kept current from session events
//			  (Edited/Undone/Redone: Update with the change set and revisions;
//			  Replaced: Rebuild; SelectionChanged: SetSelection);
//			  the presenters: entity and face inspector, outliner, history
//			  panel, status bar, visgroup panel, problems panel, action
//			  catalog, and the class palette / material browser when the
//			  catalog / material port exist.
//
//			Input routing (ONE policy; every entry returns an InputOutcome):
//			  OnPointer(view, e):
//			    1. the camera controller first. When it takes the pointer
//			       (CaptureRequested) the active tool's gesture is cancelled
//			       with ToolManager::OnCaptureLost; Handled/CaptureReleased end
//			       there.
//			    2. otherwise the ToolManager, with a ToolContext::Make for that
//			       view's camera. A Failed tool result becomes the status
//			       message.
//			  OnKey(view, e) (presses; releases skip step 1):
//			    0. while the camera is being driven (a mouse-look/pan drag, or
//			       fly keys held) keys go to the camera controller first, so
//			       Shift+A while flying is "fly left fast", not the Face tool.
//			    1. action shortcuts (ActionCatalog, the chord built from the
//			       key and modifiers). An enabled action runs:
//			         Tool target  activates the tool;
//			         Host target  is returned as hostRequest;
//			         file.save / map.build(_and_run) use the current map path
//			           (the last Open/Save); without one they return the host
//			           request "save_dialog"; file.open returns "open_dialog";
//			         other commands run through SessionCommands.
//			       A tool gesture in progress is cancelled (OnCaptureLost)
//			       before a command action runs. A disabled action falls
//			       through (a tool may use the key), and its "not available"
//			       text is the status when nothing else handles the key.
//			    2. the camera controller (Space, fly keys; 2D zoom keys).
//			    3. the active tool (Enter commits, Escape cancels, ...).
//			  OnWheel(view, e): camera controller (2D zoom, 3D dolly), else the
//			    tool.
//			  OnFocusLost(): the camera controller releases keys and drags and
//			    the tool manager cancels the gesture (no edit).
//			  Advance(seconds): 3D fly; redraw when the camera moved.
//			redraw is set when anything a view shows may have changed: a
//			handled event, a camera change, or a new document revision or
//			selection.
//
//			Document lifecycle, through the commands: New(), Open(path),
//			Save(path), Build(path, run). Build with run returns the host
//			request "run_map" on success. On replacement (new or opened map)
//			the tool gesture is cancelled (CancelReason::DocumentReplaced) and
//			the cameras frame the new map: 2D views centre its bounds (and fit
//			them when the view has a size), the 3D view looks at its centre
//			from above-behind (an empty map: the origin, from (-512, -512,
//			384)).
//
//			Per-frame queries: Snapshot(), Overlay(view), CursorFor(view, x,
//			y), GridLines(view) (2D; empty for 3D), ToolStatus() and the
//			StatusBar() presenter.
//
//			Threading: the session's single sequence. Not copyable.
//
//=============================================================================//

#ifndef HAMMER_PRESENTERS_EDITOR_WORKSPACE_H
#define HAMMER_PRESENTERS_EDITOR_WORKSPACE_H

#include "hammer/app/clipboard.h"
#include "hammer/app/edit_session.h"
#include "hammer/app/editor_settings.h"
#include "hammer/app/session_commands.h"
#include "hammer/ports/entity_catalog.h"
#include "hammer/ports/file_store.h"
#include "hammer/ports/map_builder.h"
#include "hammer/ports/map_codec.h"
#include "hammer/ports/material_info.h"
#include "hammer/presenters/action_catalog.h"
#include "hammer/presenters/class_palette.h"
#include "hammer/presenters/entity_inspector.h"
#include "hammer/presenters/face_inspector.h"
#include "hammer/presenters/history_panel.h"
#include "hammer/presenters/material_browser.h"
#include "hammer/presenters/outliner.h"
#include "hammer/presenters/problems_panel.h"
#include "hammer/presenters/status_bar.h"
#include "hammer/presenters/visgroup_panel.h"
#include "hammer/tools/camera_controller.h"
#include "hammer/tools/tool_manager.h"
#include "hammer/viewport/camera.h"
#include "hammer/viewport/extraction.h"
#include "hammer/viewport/grid.h"

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace hammer::presenters
{

// The providers a composition root chose; each is borrowed and may be null.
struct WorkspaceServices
{
	const ports::IMapCodec *codec = nullptr;
	ports::IFileStore *store = nullptr;
	ports::IMapBuilder *builder = nullptr;
	const ports::IEntityCatalog *catalog = nullptr;
	const ports::IMaterialInfo *materials = nullptr;
};

struct InputOutcome
{
	bool handled = false;
	bool redraw = false;
	// A request only the host can fulfil: "open_dialog", "save_dialog",
	// "run_map", or a host-target action's name ("properties").
	std::string hostRequest;
	std::string status;   // a message for the status bar (also shown there)
	std::string actionId; // the action a key ran, if any
};

class EditorWorkspace
{
public:
	explicit EditorWorkspace( const WorkspaceServices &services );
	~EditorWorkspace();
	EditorWorkspace( const EditorWorkspace & ) = delete;
	EditorWorkspace &operator=( const EditorWorkspace & ) = delete;

	// --- Owned state ----------------------------------------------------------------
	app::EditSession &Session() { return m_session; }
	const app::EditSession &Session() const { return m_session; }
	app::EditorSettings &Settings() { return m_settings; }
	const app::MapFragment &Clipboard() const { return m_clipboard; }
	app::SessionCommands &Commands() { return m_commands; }
	tools::ToolManager &Tools() { return m_tools; }
	tools::ToolSettings &ToolOptions() { return m_toolSettings; }
	tools::CameraController &CameraControl() { return m_cameraControl; }
	// The 2D camera of Top, Front or Side (Camera3D: the Top camera).
	viewport::Camera2D &Camera2DFor( viewport::ViewKind view );
	viewport::Camera3D &Camera3DView() { return m_camera3D; }
	void SetViewportSize( viewport::ViewKind view, int width, int height );

	// --- Presenters -------------------------------------------------------------------
	EntityInspector &Inspector() { return *m_inspector; }
	FaceInspector &Faces() { return *m_faceInspector; }
	Outliner &Tree() { return *m_outliner; }
	ClassPalette *Classes() { return m_classPalette.get(); }         // null without a catalog
	MaterialBrowser *Materials() { return m_materialBrowser.get(); } // null without materials
	HistoryPanel &History() { return *m_history; }
	StatusBar &Status() { return *m_statusBar; }
	VisgroupPanel &Visgroups() { return *m_visgroups; }
	ProblemsPanel &Problems() { return *m_problems; }
	ActionCatalog &Actions() { return *m_actions; }

	// --- Input ------------------------------------------------------------------------
	InputOutcome OnPointer( viewport::ViewKind view, const tools::PointerEvent &event );
	InputOutcome OnKey( viewport::ViewKind view, const tools::KeyEvent &event );
	InputOutcome OnWheel( viewport::ViewKind view, const tools::WheelEvent &event );
	InputOutcome OnFocusLost();
	InputOutcome Advance( double seconds );
	// Runs an action as a menu or toolbar would (the same path as its key).
	InputOutcome RunAction( const std::string &id );

	// --- Document lifecycle --------------------------------------------------------------
	app::CommandResult New();
	app::CommandResult Open( const std::string &path );
	app::CommandResult Save( const std::string &path );
	// On success with 'run', 'hostRequest' receives "run_map".
	app::CommandResult Build(
	    const std::string &path, bool run, std::string *hostRequest = nullptr );
	// The path of the last successful Open or Save; empty for a new map.
	const std::string &MapPath() const { return m_path; }

	// Frames every camera on the document's visible bounds (2D views need a
	// viewport size; an empty map centres them on the origin). Open, New and
	// every document replacement do this; hosts call it for "reset views".
	void FrameDocument();

	// --- Per-frame queries ----------------------------------------------------------------
	const viewport::RenderSnapshot &Snapshot() const { return m_snapshot.Snapshot(); }
	std::uint64_t SnapshotRevision() const { return m_snapshot.Revision(); }
	tools::OverlayList Overlay( viewport::ViewKind view );
	tools::Cursor CursorFor( viewport::ViewKind view, double x, double y );
	std::vector<viewport::GridLine> GridLines( viewport::ViewKind view );
	std::string ToolStatus() const { return m_tools.Status(); }

	// The chord ("Ctrl+Z", "Shift+B", "F9") a key event stands for.
	static std::string ChordOf( const tools::KeyEvent &event );

private:
	tools::ToolContext ContextFor( viewport::ViewKind view );
	void OnSessionEvent( const app::SessionEvent &event );
	InputOutcome Finish(
	    InputOutcome outcome, std::uint64_t revisionBefore, std::uint64_t selectionBefore );
	InputOutcome ExecuteAction( const ActionSpec &spec );

	WorkspaceServices m_services;
	app::EditSession m_session;
	app::EditorSettings m_settings;
	app::MapFragment m_clipboard;
	app::SessionCommands m_commands;

	tools::ToolManager m_tools;
	tools::ToolSettings m_toolSettings;
	tools::CameraController m_cameraControl;
	std::array<viewport::Camera2D, 3> m_cameras2D; // Top, Front, Side
	viewport::Camera3D m_camera3D;

	viewport::SnapshotCache m_snapshot;
	std::uint64_t m_selectionSerial = 0; // bumps on every selection change
	std::string m_path;
	app::SessionSubscription m_subscription;

	std::unique_ptr<EntityInspector> m_inspector;
	std::unique_ptr<FaceInspector> m_faceInspector;
	std::unique_ptr<Outliner> m_outliner;
	std::unique_ptr<ClassPalette> m_classPalette;
	std::unique_ptr<MaterialBrowser> m_materialBrowser;
	std::unique_ptr<HistoryPanel> m_history;
	std::unique_ptr<StatusBar> m_statusBar;
	std::unique_ptr<VisgroupPanel> m_visgroups;
	std::unique_ptr<ProblemsPanel> m_problems;
	std::unique_ptr<ActionCatalog> m_actions;
};

} // namespace hammer::presenters

#endif // HAMMER_PRESENTERS_EDITOR_WORKSPACE_H
