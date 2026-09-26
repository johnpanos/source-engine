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
class IDedicatedServerAPI;
class ISoundEmitterSystemBase;
class ISceneFileCache;
class IQueuedLoader;
class IFileSystem;
class IInputSystem;
class IMaterialSystem;
class IDataCache;
class IMDLCache;
class IStudioDataCache;
class IStudioRender;
class IVideoServices;
class IScriptManager;
namespace vgui
{
class IVGui;
class ISurface;
}

extern "C"
{
	ICvarQuery *Engine_CreateCvarQuery();
	IEngineAPI *Engine_CreateClientAPI();
	IDedicatedServerAPI *Engine_CreateDedicatedAPI();
	ISoundEmitterSystemBase *SoundEmitterSystem_Create();
	ISceneFileCache *SceneFileCache_Create();
	IQueuedLoader *Dedicated_CreateQueuedLoader();
	IFileSystem *FileSystemStdio_Create();
	IQueuedLoader *FileSystemStdio_CreateQueuedLoader();
	IInputSystem *InputSystem_Create();
	IMaterialSystem *MaterialSystem_Create();
	IDataCache *DataCache_Create();
	IMDLCache *MDLCache_Create();
	IStudioDataCache *StudioDataCache_Create();
	IStudioRender *StudioRender_Create();
	vgui::IVGui *VGui_Create();
	vgui::ISurface *VGuiSurface_Create();
	IVideoServices *VideoServices_Create();
	IScriptManager *VScript_Create();
}

#endif // APPFRAMEWORK_LINKED_SYSTEMS_H
