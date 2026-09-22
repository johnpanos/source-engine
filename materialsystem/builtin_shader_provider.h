//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef BUILTIN_SHADER_PROVIDER_H
#define BUILTIN_SHADER_PROVIDER_H

class ICvar;
class IMaterialSystemHardwareConfig;
class IShaderSystem;
class IShaderDLLInternal;
struct MaterialSystem_Config_t;

// Material-system sequence only. All four borrowers outlive the connection.
// Connect validates before publishing; duplicate connections fail unchanged.
// Call Disconnect only after materials and dictionaries release their shaders.
struct BuiltinShaderHostServices
{
	IMaterialSystemHardwareConfig *hardware;
	const MaterialSystem_Config_t *config;
	IShaderSystem *shaders;
	ICvar *cvars;
};

struct BuiltinShaderProvider
{
	const char *id;
	const char *legacyModuleName;
	IShaderDLLInternal *( *connect )( const BuiltinShaderHostServices &host );
	void ( *disconnect )();
};

// Private shaderlib implementation, linked separately into each owning library.
// The material system already owns its CVars; a standard library owns its own.
IShaderDLLInternal *ConnectBuiltinShaderLibrary(
    const BuiltinShaderHostServices &host, bool ownsCVars );
void DisconnectBuiltinShaderLibrary();

// Validates the root's requested catalog entry without connecting or mutating it.
bool IsBuiltinShaderProviderSelected(
    const BuiltinShaderProvider *provider, const char *requested );

extern "C" const BuiltinShaderProvider *StandardShaderLibrary_Describe();

#endif
