//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Engine_BindRenderCore (RFC 0016 A.7): the composition root hands the
// engine the render core it composed, before the engine initializes. The
// engine drives the core's frames and owns the world's scene through the
// ports in the binding; it links no render module, so every implementation
// arrives here, including the legacy backend's world mesh, light set and
// compute capabilities (RFC 0016 K3). Products that bind nothing (the
// dedicated server, and clients started with -norendercore) run without
// them: the world draws through the legacy brush path and indirect light
// uses its CPU producers.
//
// Like linked_game_modules.h, a legacy ABI package: the entry point is a
// C export of the engine module.
//
//=============================================================================//

#ifndef ENGINE_RENDER_CORE_BINDING_H
#define ENGINE_RENDER_CORE_BINDING_H

struct RenderCoreBinding;

// The definitions export these from their module (DLL_EXPORT); MSVC requires
// the declarations to agree (C2375).
#if defined( _MSC_VER )
#define RENDER_CORE_BINDING_EXPORT __declspec( dllexport )
#else
#define RENDER_CORE_BINDING_EXPORT
#endif

// Copies the binding. Fails if it is incomplete or the engine already bound
// one. The core must outlive the engine's shutdown.
extern "C" RENDER_CORE_BINDING_EXPORT bool Engine_BindRenderCore(
    const RenderCoreBinding *pBinding );

#endif // ENGINE_RENDER_CORE_BINDING_H
