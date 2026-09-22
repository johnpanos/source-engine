//========= Copyright Valve Corporation, All rights reserved. ============//
#include "../builtin_shader_provider.h"
#include "tier0/platform.h"

namespace
{
IShaderDLLInternal *ConnectStandardShaders( const BuiltinShaderHostServices &host )
{
	return ConnectBuiltinShaderLibrary( host, true );
}

const BuiltinShaderProvider s_Provider = { "source-standard-materials", "stdshader_dx9",
    ConnectStandardShaders, DisconnectBuiltinShaderLibrary };
}

DLL_EXPORT const BuiltinShaderProvider *StandardShaderLibrary_Describe()
{
	return &s_Provider;
}
