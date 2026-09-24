//========= Portal 2 port ======================================================//
//
// Purpose: Base-game API the imported Portal 2 sources expect from the later
//          (CS:GO-era) base game, expressed against this tree's Source SDK 2013
//          base game and engine.
//
// The Portal 2 client and server builds force-include this header (see
// game/client/wscript and game/server/wscript); no other game sees it. It is
// processed before any other header, so it may only define macros, forward
// declarations and self-contained helpers. Each block names the feature this
// engine lacks and the chosen single-player / unsupported behavior, so the
// difference from retail Portal 2 stays explicit and searchable.
//
//=============================================================================//

#ifndef PORTAL2_BASE_COMPAT_H
#define PORTAL2_BASE_COMPAT_H
#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

//-----------------------------------------------------------------------------
// Platform predicates. This fork builds no console targets.
//-----------------------------------------------------------------------------
#ifndef IsGameConsole
#define IsGameConsole() false
#endif
#ifndef IsPS3
#define IsPS3() false
#endif
#ifndef IsPlatformPS3
#define IsPlatformPS3() false
#endif
#ifndef IsX360
#define IsX360() false
#endif

//-----------------------------------------------------------------------------
// Split screen. This engine has one local player per client, so the Portal 2
// split-screen scaffolding compiles to its single-player form (the same form
// CS:GO uses when SPLIT_SCREEN_STUBS is defined).
//-----------------------------------------------------------------------------
#define SPLIT_SCREEN_STUBS
#ifndef MAX_SPLITSCREEN_PLAYERS
#define MAX_SPLITSCREEN_PLAYERS 1
#endif
#ifndef MAX_SPLITSCREEN_CLIENT_BITS
#define MAX_SPLITSCREEN_CLIENT_BITS 1
#endif
#ifndef MAX_SPLITSCREEN_CLIENTS
#define MAX_SPLITSCREEN_CLIENTS ( 1 << MAX_SPLITSCREEN_CLIENT_BITS )
#endif

#define VGUI_SCREENSIZE_SPLITSCREEN_GUARD( slot )
#define ACTIVE_SPLITSCREEN_PLAYER_GUARD( slot )
#define ACTIVE_SPLITSCREEN_PLAYER_GUARD_ENT( entity )
#define ACTIVE_SPLITSCREEN_PLAYER_GUARD_VGUI( slot )
#define ACTIVE_SPLITSCREEN_PLAYER_GUARD_ENT_VGUI( entity )
#define HACK_GETLOCALPLAYER_GUARD( desc )
#define VGUI_ABSPOS_SPLITSCREEN_GUARD( slot )
#define VGUI_ABSPOS_SPLITSCREEN_GUARD_INVERT( slot )
#define FOR_EACH_VALID_SPLITSCREEN_PLAYER( iteratorName ) \
	for ( int iteratorName = 0; iteratorName == 0; ++iteratorName )
#define ASSERT_LOCAL_PLAYER_RESOLVABLE()
#define ASSERT_LOCAL_PLAYER_NOT_RESOLVABLE()
#define GET_ACTIVE_SPLITSCREEN_SLOT() ( 0 )

//-----------------------------------------------------------------------------
// CEG copy protection is not built into this fork.
//-----------------------------------------------------------------------------
#include "CegClientWrapper.h"

//-----------------------------------------------------------------------------
// Server and shared additions (wave cohort S)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Client additions (wave cohort C)
//-----------------------------------------------------------------------------

#endif // PORTAL2_BASE_COMPAT_H
