//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/presenters/action_catalog.h.
//
//=============================================================================//

#include "hammer/presenters/action_catalog.h"

#include "hammer/scene/map_queries.h"
#include "presenter_text.h"

#include <cmath>

namespace hammer::presenters
{

namespace
{

ActionSpec Cmd( std::string id, std::string label, std::string category,
    std::vector<std::string> shortcuts, std::string command, EnableRule enabled,
    app::CommandArgs args = {}, std::vector<std::string> hostArgs = {} )
{
	ActionSpec spec;
	spec.id = std::move( id );
	spec.label = std::move( label );
	spec.category = std::move( category );
	spec.shortcuts = std::move( shortcuts );
	spec.target = ActionTarget::Command;
	spec.command = std::move( command );
	spec.enabled = enabled;
	spec.args = std::move( args );
	spec.hostArgs = std::move( hostArgs );
	return spec;
}

ActionSpec Toggle(
    ActionSpec spec, std::string arg, std::string on, std::string off, CheckRule checked )
{
	spec.argRule = ArgRule::Toggle;
	spec.toggleArg = std::move( arg );
	spec.onValue = std::move( on );
	spec.offValue = std::move( off );
	spec.checked = checked;
	return spec;
}

ActionSpec Checked( ActionSpec spec, CheckRule checked )
{
	spec.checked = checked;
	return spec;
}

ActionSpec WithRule( ActionSpec spec, ArgRule rule )
{
	spec.argRule = rule;
	return spec;
}

ActionSpec RunAfter( ActionSpec spec )
{
	spec.runAfter = true;
	return spec;
}

ActionSpec ToolAction( std::string id, std::string label, std::string shortcut, std::string tool )
{
	ActionSpec spec;
	spec.id = std::move( id );
	spec.label = std::move( label );
	spec.category = "Tools";
	spec.shortcuts = { std::move( shortcut ) };
	spec.target = ActionTarget::Tool;
	spec.tool = std::move( tool );
	return spec;
}

ActionSpec HostAction( std::string id, std::string label, std::string category,
    std::string shortcut, std::string host, CheckRule checked = CheckRule::None )
{
	ActionSpec spec;
	spec.id = std::move( id );
	spec.label = std::move( label );
	spec.category = std::move( category );
	if ( !shortcut.empty() )
		spec.shortcuts = { std::move( shortcut ) };
	spec.target = ActionTarget::Host;
	spec.host = std::move( host );
	spec.checked = checked;
	return spec;
}

std::vector<ActionSpec> BuildSpecs()
{
	using E = EnableRule;
	return {
	    // File
	    Cmd( "file.new", "New", "File", { "Ctrl+N" }, "new_map", E::Always ),
	    Cmd( "file.open", "Open...", "File", { "Ctrl+O" }, "open", E::Always, {}, { "path" } ),
	    Cmd( "file.save", "Save", "File", { "Ctrl+S" }, "save", E::Always, {}, { "path" } ),
	    // Edit
	    Cmd( "edit.undo", "Undo", "Edit", { "Ctrl+Z", "Alt+Backspace" }, "undo", E::CanUndo ),
	    Cmd( "edit.redo", "Redo", "Edit", { "Ctrl+Y" }, "redo", E::CanRedo ),
	    Cmd( "edit.cut", "Cut", "Edit", { "Ctrl+X", "Shift+Delete" }, "cut", E::HasObjects ),
	    Cmd( "edit.copy", "Copy", "Edit", { "Ctrl+C", "Ctrl+Insert" }, "copy", E::HasObjects ),
	    Cmd(
	        "edit.paste", "Paste", "Edit", { "Ctrl+V", "Shift+Insert" }, "paste", E::HasClipboard ),
	    Cmd( "edit.duplicate", "Duplicate", "Edit", { "Ctrl+D" }, "duplicate", E::HasObjects ),
	    Cmd( "edit.delete", "Delete", "Edit", { "Delete" }, "delete", E::HasObjects ),
	    Cmd( "edit.select_all", "Select All", "Edit", { "Ctrl+A" }, "select_all", E::Always ),
	    Cmd( "edit.select_none", "Select None", "Edit", { "Shift+Q" }, "select_none",
	        E::HasSelection ),
	    Cmd( "edit.invert_selection", "Invert Selection", "Edit", { "Ctrl+Shift+I" },
	        "invert_selection", E::Always ),
	    HostAction( "edit.properties", "Properties", "Edit", "Alt+Enter", "properties" ),
	    // View
	    Cmd( "view.hide", "Hide", "View", { "H" }, "hide", E::HasObjects ),
	    Cmd( "view.hide_unselected", "Hide Unselected", "View", { "Ctrl+H" }, "hide_unselected",
	        E::HasObjects ),
	    Cmd( "view.unhide_all", "Unhide All", "View", { "U" }, "unhide_all", E::AnyHidden ),
	    WithRule( Cmd( "view.grid_higher", "Larger Grid", "View", { "]", "Alt+S" }, "set_grid",
	                  E::GridCanGrow ),
	        ArgRule::GridHigher ),
	    WithRule( Cmd( "view.grid_lower", "Smaller Grid", "View", { "[", "Alt+A" }, "set_grid",
	                  E::GridCanShrink ),
	        ArgRule::GridLower ),
	    Toggle( Cmd( "view.snap_to_grid", "Snap to Grid", "View", { "Shift+W" }, "set_snap",
	                E::Always ),
	        "on", "1", "0", CheckRule::SnapToGrid ),
	    // Tools
	    ToolAction( "tools.selection", "Selection Tool", "Shift+S", "selection" ),
	    ToolAction( "tools.block", "Block Tool", "Shift+B", "block" ),
	    ToolAction( "tools.entity", "Entity Tool", "Shift+E", "entity" ),
	    ToolAction( "tools.clip", "Clipping Tool", "Shift+X", "clip" ),
	    ToolAction( "tools.vertex", "Vertex Tool", "Shift+V", "vertex" ),
	    ToolAction( "tools.face", "Face Edit Tool", "Shift+A", "face" ),
	    Cmd( "tools.group", "Group", "Tools", { "Ctrl+G" }, "group", E::HasObjects ),
	    Cmd( "tools.ungroup", "Ungroup", "Tools", { "Ctrl+U" }, "ungroup", E::HasGroups ),
	    Cmd( "tools.tie_to_entity", "Tie to Entity", "Tools", { "Ctrl+T" }, "tie_to_entity",
	        E::HasSolids ),
	    Cmd( "tools.move_to_world", "Move to World", "Tools", { "Ctrl+Shift+W" }, "move_to_world",
	        E::HasBrushEntities ),
	    Cmd( "tools.carve", "Carve", "Tools", { "Ctrl+Shift+C" }, "carve", E::HasSolids ),
	    Cmd( "tools.hollow", "Make Hollow", "Tools", { "F" }, "hollow", E::HasSolids,
	        { { "thickness", "16" } } ),
	    Cmd( "tools.snap_selected", "Snap Selected to Grid", "Tools", { "Ctrl+B" }, "snap",
	        E::HasObjects ),
	    Cmd( "tools.apply_material", "Apply Current Material", "Tools", { "Shift+T" },
	        "apply_material", E::HasSelection ),
	    // Legacy Tools > Replace Textures... (no shortcut); the host shows the
	    // ReplaceTexturesDialog model.
	    HostAction(
	        "tools.replace_textures", "Replace Textures...", "Tools", "", "replace_textures" ),
	    Toggle( Cmd( "tools.texture_lock", "Texture Lock", "Tools", { "Shift+L" },
	                "set_texture_lock", E::Always ),
	        "on", "1", "0", CheckRule::TextureLock ),
	    Toggle( Cmd( "tools.ignore_groups", "Ignore Groups", "Tools", { "Ctrl+W" },
	                "set_granularity", E::Always ),
	        "mode", "objects", "groups", CheckRule::IgnoreGroups ),
	    Checked( Cmd( "tools.select_groups", "Select Groups", "Tools", {}, "set_granularity",
	                 E::Always, { { "mode", "groups" } } ),
	        CheckRule::GranularityGroups ),
	    Checked( Cmd( "tools.select_objects", "Select Objects", "Tools", {}, "set_granularity",
	                 E::Always, { { "mode", "objects" } } ),
	        CheckRule::GranularityObjects ),
	    Checked( Cmd( "tools.select_solids", "Select Solids", "Tools", {}, "set_granularity",
	                 E::Always, { { "mode", "solids" } } ),
	        CheckRule::GranularitySolids ),
	    // Map
	    Cmd( "map.check", "Check for Problems", "Map", { "Alt+P" }, "check_map", E::Always ),
	    Cmd( "map.fix_all", "Fix All Problems", "Map", {}, "fix_all", E::Always ),
	    Cmd( "map.build", "Build Map", "Map", { "F9" }, "build_map", E::Always, {}, { "path" } ),
	    RunAfter( Cmd( "map.build_and_run", "Build and Run", "Map", { "Shift+F9" }, "build_map",
	        E::Always, { { "publish", "1" } }, { "path" } ) ),
	};
}

std::string CanonicalKey( const std::string &key )
{
	static const std::pair<const char *, const char *> kAliases[] = {
	    { "del", "Delete" },
	    { "delete", "Delete" },
	    { "ins", "Insert" },
	    { "insert", "Insert" },
	    { "return", "Enter" },
	    { "enter", "Enter" },
	    { "esc", "Escape" },
	    { "escape", "Escape" },
	    { "backspace", "Backspace" },
	    { "back", "Backspace" },
	    { "pgup", "PageUp" },
	    { "pageup", "PageUp" },
	    { "pgdn", "PageDown" },
	    { "pagedown", "PageDown" },
	    { "space", "Space" },
	    { "tab", "Tab" },
	    { "home", "Home" },
	    { "end", "End" },
	    { "up", "Up" },
	    { "down", "Down" },
	    { "left", "Left" },
	    { "right", "Right" },
	};
	const std::string lower = detail::Lower( key );
	for ( const auto &alias : kAliases )
	{
		if ( lower == alias.first )
		{
			return alias.second;
		}
	}
	if ( key.size() == 1 )
	{
		const char c = key[0];
		return std::string( 1, ( c >= 'a' && c <= 'z' ) ? static_cast<char>( c - 'a' + 'A' ) : c );
	}
	if ( lower.size() >= 2 && lower[0] == 'f' &&
	     lower.find_first_not_of( "0123456789", 1 ) == std::string::npos )
	{
		return "F" + lower.substr( 1 );
	}
	std::string out = lower;
	out[0] = static_cast<char>( out[0] >= 'a' && out[0] <= 'z' ? out[0] - 'a' + 'A' : out[0] );
	return out;
}

} // namespace

std::optional<std::string> NormalizeChord( std::string_view text )
{
	std::vector<std::string> parts;
	std::string current;
	for ( std::size_t i = 0; i < text.size(); ++i )
	{
		const char c = text[i];
		if ( c == ' ' )
		{
			continue;
		}
		// A '+' is a separator unless it is the key itself ("Ctrl++", "+").
		if ( c == '+' && !current.empty() )
		{
			parts.push_back( current );
			current.clear();
		}
		else
		{
			current.push_back( c );
		}
	}
	if ( !current.empty() )
	{
		parts.push_back( current );
	}
	bool ctrl = false, alt = false, shift = false, meta = false;
	std::optional<std::string> key;
	for ( const std::string &part : parts )
	{
		const std::string lower = detail::Lower( part );
		if ( lower == "ctrl" || lower == "control" )
		{
			ctrl = true;
		}
		else if ( lower == "alt" )
		{
			alt = true;
		}
		else if ( lower == "shift" )
		{
			shift = true;
		}
		else if ( lower == "meta" || lower == "super" || lower == "cmd" )
		{
			meta = true;
		}
		else if ( key )
		{
			return std::nullopt; // two keys
		}
		else
		{
			key = CanonicalKey( part );
		}
	}
	if ( !key )
	{
		return std::nullopt;
	}
	std::string chord;
	chord += ctrl ? "Ctrl+" : "";
	chord += alt ? "Alt+" : "";
	chord += shift ? "Shift+" : "";
	chord += meta ? "Meta+" : "";
	return chord + *key;
}

const std::vector<ActionSpec> &ActionCatalog::Specs()
{
	static const std::vector<ActionSpec> specs = BuildSpecs();
	return specs;
}

const std::vector<std::string> &ActionCatalog::Categories()
{
	static const std::vector<std::string> categories = { "File", "Edit", "View", "Tools", "Map" };
	return categories;
}

std::vector<const ActionSpec *> ActionCatalog::InCategory( std::string_view category )
{
	std::vector<const ActionSpec *> out;
	for ( const ActionSpec &spec : Specs() )
	{
		if ( spec.category == category )
		{
			out.push_back( &spec );
		}
	}
	return out;
}

const ActionSpec *ActionCatalog::Find( std::string_view id )
{
	for ( const ActionSpec &spec : Specs() )
	{
		if ( spec.id == id )
		{
			return &spec;
		}
	}
	return nullptr;
}

const ActionSpec *ActionCatalog::FindByShortcut( std::string_view chord )
{
	const std::optional<std::string> normalized = NormalizeChord( chord );
	if ( !normalized )
	{
		return nullptr;
	}
	for ( const ActionSpec &spec : Specs() )
	{
		for ( const std::string &shortcut : spec.shortcuts )
		{
			if ( shortcut == *normalized )
			{
				return &spec;
			}
		}
	}
	return nullptr;
}

ActionCatalog::ActionCatalog( app::EditSession &session, const app::EditorSettings &settings,
    app::SessionCommands &commands, const app::MapFragment *clipboard )
    : m_session( session ), m_settings( settings ), m_commands( commands ), m_clipboard( clipboard )
{
	m_subscription = m_session.Subscribe(
	    [this]( const app::SessionEvent & )
	    {
		    ++m_revision;
	    } );
}

bool ActionCatalog::Evaluate( EnableRule rule ) const
{
	const scene::MapDocument &doc = m_session.Document();
	const app::Selection &selection = m_session.CurrentSelection();
	switch ( rule )
	{
	case EnableRule::Always:
		return true;
	case EnableRule::CanUndo:
		return m_session.History().CanUndo();
	case EnableRule::CanRedo:
		return m_session.History().CanRedo();
	case EnableRule::HasSelection:
		return !selection.Empty();
	case EnableRule::HasObjects:
		return !selection.objects.empty();
	case EnableRule::HasSolids:
		for ( scene::ObjectId id : scene::ExpandToLeaves( doc, selection.objects ) )
		{
			if ( doc.FindSolid( id ) )
			{
				return true;
			}
		}
		return false;
	case EnableRule::HasGroups:
		for ( scene::ObjectId id : selection.objects )
		{
			if ( doc.FindGroup( id ) )
			{
				return true;
			}
		}
		return false;
	case EnableRule::HasBrushEntities:
		for ( scene::ObjectId id : selection.objects )
		{
			const scene::Solid *s = doc.FindSolid( id );
			if ( ( s && s->owner.IsValid() ) ||
			     ( doc.FindEntity( id ) && !scene::EntitySolids( doc, id ).empty() ) )
			{
				return true;
			}
		}
		return false;
	case EnableRule::HasFaces:
		return !selection.faces.empty();
	case EnableRule::HasClipboard:
		return m_clipboard && !m_clipboard->Empty();
	case EnableRule::AnyHidden:
		for ( const auto &entry : doc.Solids() )
		{
			if ( entry.second.hidden )
			{
				return true;
			}
		}
		for ( const auto &entry : doc.Entities() )
		{
			if ( entry.second.hidden )
			{
				return true;
			}
		}
		for ( const auto &entry : doc.Groups() )
		{
			if ( entry.second.hidden )
			{
				return true;
			}
		}
		return false;
	case EnableRule::GridCanGrow:
		return m_settings.gridSize < 1024.0;
	case EnableRule::GridCanShrink:
		return m_settings.gridSize > 1.0;
	}
	return false;
}

bool ActionCatalog::Evaluate( CheckRule rule ) const
{
	switch ( rule )
	{
	case CheckRule::None:
		return false;
	case CheckRule::TextureLock:
		return m_settings.textureLock;
	case CheckRule::SnapToGrid:
		return m_settings.snapToGrid;
	case CheckRule::IgnoreGroups:
		return m_settings.granularity != app::SelectionGranularity::Groups;
	case CheckRule::GranularityGroups:
		return m_settings.granularity == app::SelectionGranularity::Groups;
	case CheckRule::GranularityObjects:
		return m_settings.granularity == app::SelectionGranularity::Objects;
	case CheckRule::GranularitySolids:
		return m_settings.granularity == app::SelectionGranularity::Solids;
	}
	return false;
}

bool ActionCatalog::IsEnabled( std::string_view id ) const
{
	const ActionSpec *spec = Find( id );
	return spec && Evaluate( spec->enabled );
}

std::optional<bool> ActionCatalog::IsChecked( std::string_view id ) const
{
	const ActionSpec *spec = Find( id );
	if ( !spec || spec->checked == CheckRule::None )
	{
		return std::nullopt;
	}
	return Evaluate( spec->checked );
}

std::string ActionCatalog::UndoLabel() const
{
	const app::HistoryEntry *entry = m_session.History().UndoEntry();
	return entry ? "Undo " + entry->label : std::string( "Undo" );
}

std::string ActionCatalog::RedoLabel() const
{
	const app::HistoryEntry *entry = m_session.History().RedoEntry();
	return entry ? "Redo " + entry->label : std::string( "Redo" );
}

std::string ActionCatalog::DisplayLabel( std::string_view id ) const
{
	if ( id == "edit.undo" )
	{
		return UndoLabel();
	}
	if ( id == "edit.redo" )
	{
		return RedoLabel();
	}
	const ActionSpec *spec = Find( id );
	return spec ? spec->label : std::string();
}

ActionResult ActionCatalog::Fail( ActionError error )
{
	m_lastMessage = error.message;
	++m_revision;
	return foundation::MakeUnexpected( std::move( error ) );
}

ActionResult ActionCatalog::Execute( std::string_view id, const app::CommandArgs &hostArgs )
{
	const ActionSpec *spec = Find( id );
	if ( !spec )
	{
		return Fail( ActionError{ ActionErrorKind::UnknownAction,
		    "unknown action '" + std::string( id ) + "'", std::nullopt } );
	}
	const std::string label = DisplayLabel( id );
	if ( !Evaluate( spec->enabled ) )
	{
		return Fail( ActionError{
		    ActionErrorKind::Disabled, label + " is not available now", std::nullopt } );
	}
	ActionOutcome outcome;
	outcome.target = spec->target;
	outcome.runAfter = spec->runAfter;
	if ( spec->target == ActionTarget::Tool || spec->target == ActionTarget::Host )
	{
		outcome.tool = spec->tool;
		outcome.host = spec->host;
		m_lastMessage = label;
		++m_revision;
		return outcome;
	}

	app::CommandArgs args = spec->args;
	switch ( spec->argRule )
	{
	case ArgRule::Fixed:
		break;
	case ArgRule::Toggle:
		args[spec->toggleArg] = Evaluate( spec->checked ) ? spec->offValue : spec->onValue;
		break;
	case ArgRule::GridHigher:
	case ArgRule::GridLower:
	{
		const long grid = std::lround( m_settings.gridSize );
		long size = spec->argRule == ArgRule::GridHigher ? grid * 2 : grid / 2;
		size = size < 1 ? 1 : size > 1024 ? 1024 : size;
		args["size"] = std::to_string( size );
		break;
	}
	}
	for ( const auto &entry : hostArgs )
	{
		args[entry.first] = entry.second;
	}
	for ( const std::string &name : spec->hostArgs )
	{
		if ( !args.count( name ) )
		{
			return Fail( ActionError{ ActionErrorKind::MissingArgument,
			    label + ": the host did not pass '" + name + "'", std::nullopt } );
		}
	}
	app::CommandResult result = m_commands.Execute( spec->command, args );
	if ( !result )
	{
		const app::CommandError &error = result.Error();
		return Fail( ActionError{ ActionErrorKind::CommandFailed,
		    label + ": " +
		        ( error.detail.empty() ? app::CommandStatusName( error.status ) : error.detail ),
		    error } );
	}
	outcome.output = result.Value();
	const bool oneLine =
	    !outcome.output.empty() && outcome.output.find( '\n' ) == std::string::npos;
	m_lastMessage = oneLine ? label + ": " + outcome.output : label;
	++m_revision;
	return outcome;
}

} // namespace hammer::presenters
