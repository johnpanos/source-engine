//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Optional platform services queried from ILauncherMgr (RFC 0001
//			R18). The launcher manager is the window provider: it alone links
//			the platform SDK. Engine, UI and surface code ask it for displays,
//			the clipboard, URLs, the pointer and system cursors through this
//			contract instead of including the SDK.
//
//			Displays are addressed by index in the platform's display order,
//			the convention of sdl_displayindex and the saved video config. Every
//			method is called on the window thread unless it says otherwise.
//
//=============================================================================//

#ifndef ILAUNCHERPLATFORMSERVICES_H
#define ILAUNCHERPLATFORMSERVICES_H

#ifdef _WIN32
#pragma once
#endif

#define LAUNCHER_PLATFORM_SERVICES_INTERFACE_VERSION "LauncherPlatformServices001"

typedef struct SDL_Cursor SDL_Cursor;

struct LauncherDisplayRect
{
	int x;
	int y;
	int width;
	int height;
};

struct LauncherDisplayMode
{
	int width;
	int height;
	int refreshHz; // 0 when the platform does not report one
};

// System cursor shapes, in vgui's CursorCode order from dc_arrow.
enum LauncherSystemCursor
{
	LauncherCursor_Arrow,
	LauncherCursor_IBeam,
	LauncherCursor_Wait,
	LauncherCursor_Crosshair,
	LauncherCursor_WaitArrow,
	LauncherCursor_SizeNWSE,
	LauncherCursor_SizeNESW,
	LauncherCursor_SizeWE,
	LauncherCursor_SizeNS,
	LauncherCursor_SizeAll,
	LauncherCursor_No,
	LauncherCursor_Hand,
	LauncherCursor_Count,
};

class ILauncherPlatformServices
{
public:
	virtual ~ILauncherPlatformServices() = default;

	virtual int GetDisplayCount() = 0;
	// False, leaving 'out' unchanged, for an index outside [0, GetDisplayCount()).
	virtual bool GetDisplayBounds( int display, LauncherDisplayRect &out ) = 0;
	// The display's desktop mode, or its current mode while a fullscreen
	// window has changed it.
	virtual bool GetDisplayMode( int display, bool desktop, LauncherDisplayMode &out ) = 0;

	// UTF-8 text. GetClipboardText returns a copy the caller frees with
	// FreeClipboardText, or null when the clipboard holds no text.
	virtual bool SetClipboardText( const char *utf8 ) = 0;
	virtual char *GetClipboardText() = 0;
	virtual void FreeClipboardText( char *text ) = 0;

	// Opens a URL or file in the user's default handler.
	virtual bool OpenURL( const char *url ) = 0;

	// The game window: shown, and asked to redraw (an expose with no rendered
	// frame behind it).
	virtual void ShowWindow() = 0;
	virtual void RequestRedraw() = 0;

	// Pointer position and window size in window coordinates.
	virtual bool GetPointerPosition( int &x, int &y ) = 0;
	virtual bool GetWindowSize( int &width, int &height ) = 0;

	// A system cursor for ILauncherMgr::SetMouseCursor, owned by the launcher
	// until it shuts down; null when the platform has none.
	virtual SDL_Cursor *GetSystemCursor( LauncherSystemCursor cursor ) = 0;
};

#if defined( USE_SDL )
#include "appframework/ilaunchermgr.h"

// The services of the launcher manager, or null without one (dedicated and tool
// products have no launcher manager).
inline ILauncherPlatformServices *LauncherPlatformServices( ILauncherMgr *manager )
{
	return manager ? static_cast<ILauncherPlatformServices *>(
	                     manager->QueryInterface( LAUNCHER_PLATFORM_SERVICES_INTERFACE_VERSION ) )
	               : nullptr;
}
#endif

#endif // ILAUNCHERPLATFORMSERVICES_H
