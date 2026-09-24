//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Engine lifecycle of the root-bound debug API server (engine/debugapi_host.cpp,
// public/engine/debugapi_binding.h). Builds without --debug-api=enabled get
// inline no-ops, so callers need no conditionals.
//
//=============================================================================//

#ifndef ENGINE_DEBUGAPI_ENGINE_H
#define ENGINE_DEBUGAPI_ENGINE_H

#ifdef SOURCE_DEBUGAPI
// After the game systems initialize. False (with a warning) when a bound server
// cannot start; true when nothing was bound.
bool DebugApi_EngineStart();
// Once per engine frame on the main thread.
void DebugApi_EngineFrame();
// Before the game systems shut down; answers deferred calls and closes clients.
void DebugApi_EngineStop();
#else
inline bool DebugApi_EngineStart()
{
	return true;
}
inline void DebugApi_EngineFrame()
{
}
inline void DebugApi_EngineStop()
{
}
#endif

#endif // ENGINE_DEBUGAPI_ENGINE_H
