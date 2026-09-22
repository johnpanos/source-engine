//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Typed construction entry points for linked first-party app systems.
// The returned instances are owned by their linked module. The application
// controls Connect/Init/Shutdown/Disconnect and must not delete the instances.
// Legacy CreateInterface exports remain available to existing binary consumers.
//
//=============================================================================//

#ifndef APPFRAMEWORK_LINKED_SYSTEMS_H
#define APPFRAMEWORK_LINKED_SYSTEMS_H

class ICvarQuery;
class IEngineAPI;
class IInputSystem;
class IMaterialSystem;
class IDataCache;
class IMDLCache;
class IStudioDataCache;
class IStudioRender;
class IPhysics;
class IVideoServices;
namespace vgui
{
class IVGui;
class ISurface;
}

extern "C"
{
	ICvarQuery *Engine_CreateCvarQuery();
	IEngineAPI *Engine_CreateClientAPI();
	IInputSystem *InputSystem_Create();
	IMaterialSystem *MaterialSystem_Create();
	IDataCache *DataCache_Create();
	IMDLCache *MDLCache_Create();
	IStudioDataCache *StudioDataCache_Create();
	IStudioRender *StudioRender_Create();
	IPhysics *Physics_Create();
	vgui::IVGui *VGui_Create();
	vgui::ISurface *VGuiSurface_Create();
	IVideoServices *VideoServices_Create();
}

#endif // APPFRAMEWORK_LINKED_SYSTEMS_H
