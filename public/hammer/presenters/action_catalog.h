//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The editor's actions for menus, toolbars and the keyboard (RFC
//			0002, hammer.presenters). An action is data -- {id, label,
//			category, shortcuts, target, command name, fixed arguments, enabled
//			rule, checked rule} -- and app::SessionCommands is the ONE authority
//			that performs it, so a menu item, a key and a script run the same
//			command and produce the same undo step.
//
//			Targets:
//			  Command  runs SessionCommands::Execute( command, args ). args are
//			           the fixed arguments, the rule-derived one (a toggle's
//			           next value, the next grid size) and the host arguments
//			           the host passes to Execute (hostArgs names the ones it
//			           must pass, such as the map path for save and build).
//			  Tool     tool switching: tools live above hammer.app, so they are
//			           not commands. Execute returns the tool name (the
//			           hammer.tools kName: selection, block, entity, clip,
//			           vertex, face) and the host routes it to its tool
//			           manager.
//			  Host     view concerns without a command (the properties
//			           window): Execute returns the host action name.
//			map.build_and_run also sets runAfter: after a successful build the
//			host launches the game.
//
//			Enabled rules read session state: history (CanUndo/CanRedo), the
//			selection's kinds (objects, solids reached through groups and brush
//			entities, groups, brush entities, faces), the clipboard, quick-hidden
//			objects and the grid range. Checked rules read app::EditorSettings
//			(texture lock, snap, granularity). Execute refuses a disabled action
//			(ActionErrorKind::Disabled) without running anything.
//
//			Shortcuts are normalized chords (NormalizeChord): modifiers in the
//			order Ctrl, Alt, Shift, Meta, whatever order and case they are typed
//			in ("shift+ctrl+w" -> "Ctrl+Shift+W"); letters upper case; named
//			keys canonical (Del -> Delete, Ins -> Insert, Return -> Enter, Esc
//			-> Escape, PgUp -> PageUp, PgDn -> PageDown, f9 -> F9). An action
//			may have several shortcuts (legacy alternates); every chord maps to
//			one action.
//
//			Default keys are legacy Hammer's (hammer.rc accelerators,
//			editorkeys.txt) plus the Source 2 keys of the ergonomics brief
//			(RFC/0002-progress.md). Conflicts and their resolution:
//			  Ctrl+H  hammer.rc: hide unselected; editorkeys.txt: hollow. The
//			          shipped accelerator wins; hollow takes Source 2's F.
//			  Ctrl+D  legacy: displacement tool; Source 2: duplicate. Duplicate
//			          (the displacement tool is not one of the tools here;
//			          displacement commands stay keyless).
//			  Ctrl+A  legacy: autosize the four views; select all. Select all
//			          (view layout belongs to the host).
//			  Ctrl+I  legacy: flip vertical (2D view); invert selection uses
//			          Ctrl+Shift+I instead, and flips are not actions yet.
//			  Shift+V legacy morph tool = the vertex tool; Shift+A legacy
//			          texture application tool = the face tool.
//			  F9 / Shift+F9: legacy "run map" / Source 2 build and run: F9
//			          builds, Shift+F9 builds, publishes and runs.
//
//			Messages: every Execute leaves a status-bar message (LastMessage):
//			the label, with a one-line command output appended ("Copy: 3"), or
//			the failure ("Save: no file store in this composition").
//
//			The subscription is RAII; the catalog may be destroyed before or
//			after its session, but calls need a live session and commands.
//
//=============================================================================//

#ifndef HAMMER_PRESENTERS_ACTION_CATALOG_H
#define HAMMER_PRESENTERS_ACTION_CATALOG_H

#include "foundation/expected.h"
#include "hammer/app/clipboard.h"
#include "hammer/app/edit_session.h"
#include "hammer/app/editor_settings.h"
#include "hammer/app/session_commands.h"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace hammer::presenters
{

enum class ActionTarget
{
	Command,
	Tool,
	Host,
};

enum class EnableRule
{
	Always,
	CanUndo,
	CanRedo,
	HasSelection,     // objects or faces
	HasObjects,       // selected objects
	HasSolids,        // the objects reach at least one solid
	HasGroups,        // a selected group
	HasBrushEntities, // a selected brush entity or a solid owned by one
	HasFaces,
	HasClipboard,
	AnyHidden, // a quick-hidden object exists
	GridCanGrow,
	GridCanShrink,
};

enum class CheckRule
{
	None,
	TextureLock,
	SnapToGrid,
	IgnoreGroups, // granularity is not Groups
	GranularityGroups,
	GranularityObjects,
	GranularitySolids,
};

enum class ArgRule
{
	Fixed,      // only the fixed arguments
	Toggle,     // toggleArg = checked ? offValue : onValue
	GridHigher, // size = twice the grid (at most 1024)
	GridLower,  // size = half the grid (at least 1)
};

struct ActionSpec
{
	std::string id;                     // stable: "edit.undo", "tools.block", "map.check"
	std::string label;                  // menu text
	std::string category;               // menu: File, Edit, View, Tools, Map
	std::vector<std::string> shortcuts; // normalized chords; the first is shown
	ActionTarget target = ActionTarget::Command;
	std::string command; // Command: the SessionCommands name
	app::CommandArgs args;
	std::vector<std::string> hostArgs; // arguments the host must pass
	ArgRule argRule = ArgRule::Fixed;
	std::string toggleArg;
	std::string onValue;
	std::string offValue;
	std::string tool; // Tool: the tool name
	std::string host; // Host: the host action name
	EnableRule enabled = EnableRule::Always;
	CheckRule checked = CheckRule::None;
	bool runAfter = false; // the host runs the game after success
};

enum class ActionErrorKind
{
	UnknownAction,
	Disabled,
	MissingArgument, // a host argument was not passed
	CommandFailed,
};

struct ActionError
{
	ActionErrorKind kind = ActionErrorKind::CommandFailed;
	std::string message;
	std::optional<app::CommandError> command; // CommandFailed
};

struct ActionOutcome
{
	ActionTarget target = ActionTarget::Command;
	std::string output; // the command's output
	std::string tool;   // Tool: the tool to activate
	std::string host;   // Host: the host action to perform
	bool runAfter = false;
};

using ActionResult = foundation::Expected<ActionOutcome, ActionError>;

// The canonical chord for 'text', or nothing when it is not a chord (empty,
// only modifiers, two keys, an unknown modifier).
std::optional<std::string> NormalizeChord( std::string_view text );

class ActionCatalog
{
public:
	// 'clipboard' is the one the commands use (may be null: paste disabled).
	ActionCatalog( app::EditSession &session, const app::EditorSettings &settings,
	    app::SessionCommands &commands, const app::MapFragment *clipboard );
	ActionCatalog( const ActionCatalog & ) = delete;
	ActionCatalog &operator=( const ActionCatalog & ) = delete;

	// Every action, in menu order.
	static const std::vector<ActionSpec> &Specs();
	// The categories in menu order.
	static const std::vector<std::string> &Categories();
	static std::vector<const ActionSpec *> InCategory( std::string_view category );
	static const ActionSpec *Find( std::string_view id );
	// The action bound to 'chord' (normalized first), or nullptr.
	static const ActionSpec *FindByShortcut( std::string_view chord );

	// Moves on every session change and every Execute.
	std::uint64_t Revision() const { return m_revision; }

	bool IsEnabled( std::string_view id ) const;
	// Nothing for actions that are not toggles.
	std::optional<bool> IsChecked( std::string_view id ) const;
	// The label to show: edit.undo and edit.redo name their entry.
	std::string DisplayLabel( std::string_view id ) const;
	// "Undo Move" / "Undo", "Redo Move" / "Redo".
	std::string UndoLabel() const;
	std::string RedoLabel() const;

	ActionResult Execute( std::string_view id, const app::CommandArgs &hostArgs = {} );
	const std::string &LastMessage() const { return m_lastMessage; }

private:
	bool Evaluate( EnableRule rule ) const;
	bool Evaluate( CheckRule rule ) const;
	ActionResult Fail( ActionError error );

	app::EditSession &m_session;
	const app::EditorSettings &m_settings;
	app::SessionCommands &m_commands;
	const app::MapFragment *m_clipboard = nullptr;
	app::SessionSubscription m_subscription;
	std::uint64_t m_revision = 0;
	std::string m_lastMessage;
};

} // namespace hammer::presenters

#endif // HAMMER_PRESENTERS_ACTION_CATALOG_H
