//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Optional window presentation contract queried from ILauncherMgr.
//=============================================================================//

#ifndef ILAUNCHERWINDOWPRESENTATION_H
#define ILAUNCHERWINDOWPRESENTATION_H

#define LAUNCHER_WINDOW_PRESENTATION_INTERFACE_VERSION "LauncherWindowPresentation001"

class ILauncherWindowPresentation
{
public:
	virtual ~ILauncherWindowPresentation() = default;

	// Called on the window thread after the fullscreen transition. A borderless
	// window is windowed but has no resize edge; fullscreen cannot be dragged.
	virtual bool ApplyWindowPresentation( bool windowed, bool borderless ) = 0;
};

#endif // ILAUNCHERWINDOWPRESENTATION_H
