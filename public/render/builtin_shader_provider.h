//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef RENDER_BUILTIN_SHADER_PROVIDER_H
#define RENDER_BUILTIN_SHADER_PROVIDER_H

class ICvar;
class IMaterialSystem;
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

// The definitions export these from their module (DLL_EXPORT); MSVC requires
// the declarations to agree (C2375).
#if defined( _MSC_VER )
#define BUILTIN_SHADER_PROVIDER_EXPORT __declspec( dllexport )
#else
#define BUILTIN_SHADER_PROVIDER_EXPORT
#endif

// Validates the root's requested catalog entry without connecting or mutating it.
extern "C" BUILTIN_SHADER_PROVIDER_EXPORT bool IsBuiltinShaderProviderSelected(
    const BuiltinShaderProvider *provider, const char *requested );

extern "C" BUILTIN_SHADER_PROVIDER_EXPORT const BuiltinShaderProvider *
StandardShaderLibrary_Describe();

// Copies the descriptor before Connect; its strings and linked code outlive the
// material connection. Duplicate/replacement bindings fail without mutation.
extern "C" BUILTIN_SHADER_PROVIDER_EXPORT bool MaterialSystem_BindBuiltinShaderProvider(
    IMaterialSystem *materialSystem, const BuiltinShaderProvider *provider );

#endif
