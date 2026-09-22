//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef APPFRAMEWORK_WINDOW_PROVIDER_H
#define APPFRAMEWORK_WINDOW_PROVIDER_H

class ILauncherMgr;

// The linked module owns this descriptor and its instance. The composition root
// chooses a descriptor before Connect; IAppSystem owns the ordinary lifecycle.
// No window, SDL header, or native handle crosses the construction boundary.
struct WindowProviderDescriptor
{
	const char *name;
	ILauncherMgr *( *create )();
};

const WindowProviderDescriptor *WindowProvider_Describe();
void WindowProvider_ReportVersion();
void WindowProvider_ShowError( const char *title, const char *message );
void WindowProvider_Prepare();

#endif // APPFRAMEWORK_WINDOW_PROVIDER_H
