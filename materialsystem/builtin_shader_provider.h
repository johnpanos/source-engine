//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef BUILTIN_SHADER_PROVIDER_H
#define BUILTIN_SHADER_PROVIDER_H

#include "render/builtin_shader_provider.h"

// Private shaderlib implementation, linked separately into each owning library.
// The material system already owns its CVars; a standard library owns its own.
IShaderDLLInternal *ConnectBuiltinShaderLibrary(
    const BuiltinShaderHostServices &host, bool ownsCVars );
void DisconnectBuiltinShaderLibrary();

#endif
