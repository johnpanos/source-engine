//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The named command layer over EditSession (RFC 0002, hammer.app):
//			one catalog of serializable commands that scripts, UI actions,
//			UI-driven tests and the MCP server share, each routed to the session
//			and the operation families. Every document change is one Execute,
//			so one command is one undo step.
//
//			Addressing. Commands name objects by their script id: the low 32
//			bits of the runtime ObjectId, which never repeat within a document.
//			Id lists are space-separated ("3 7 9"); faces are "solid:side"
//			pairs using the side's persistent VMF id ("3:14 3:15"). A command
//			that takes 'ids' acts on the current selection when it is omitted.
//			Vectors are "x y z".
//
//			Outputs. Creating commands return the new ids; queries return
//			key=value text; other commands return an empty string.
//
//=============================================================================//

#ifndef HAMMER_APP_SESSION_COMMANDS_H
#define HAMMER_APP_SESSION_COMMANDS_H

#include "hammer/app/clipboard.h"
#include "hammer/app/command_script.h"
#include "hammer/app/edit_session.h"
#include "hammer/app/editor_settings.h"
#include "hammer/ports/entity_catalog.h"
#include "hammer/ports/file_store.h"
#include "hammer/ports/map_builder.h"
#include "hammer/ports/map_codec.h"
#include "hammer/ports/material_info.h"

#include <string>
#include <string_view>
#include <vector>

namespace hammer::app
{

// The services commands may use; each is borrowed and may be null, in which
// case the commands that need it are rejected ("no codec in this
// composition").
struct SessionServices
{
	const ports::IMapCodec *codec = nullptr;
	ports::IFileStore *store = nullptr;
	ports::IMapBuilder *builder = nullptr;
	const ports::IEntityCatalog *catalog = nullptr;
	const ports::IMaterialInfo *materials = nullptr;
	// The clipboard copy/cut/paste use; host-owned so documents can share it.
	MapFragment *clipboard = nullptr;
};

class SessionCommands
{
public:
	// Borrows everything; all must outlive this object.
	SessionCommands(
	    EditSession &session, EditorSettings &settings, const SessionServices &services );

	[[nodiscard]] CommandResult Execute( std::string_view name, const CommandArgs &args );
	[[nodiscard]] foundation::Expected<std::vector<std::string>, CommandError> Run(
	    const std::vector<ScriptCommand> &script );

	static const std::vector<CommandInfo> &Catalog();

	// Script id <-> runtime id for the current document.
	static std::uint32_t ScriptId( scene::ObjectId id );
	scene::ObjectId FromScriptId( std::uint32_t scriptId ) const;

private:
	EditSession &m_session;
	EditorSettings &m_settings;
	SessionServices m_services;
};

} // namespace hammer::app

#endif // HAMMER_APP_SESSION_COMMANDS_H
