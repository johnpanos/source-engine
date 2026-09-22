//========= Copyright Valve Corporation, All rights reserved. ============//
#include "render/builtin_shader_provider.h"
#include "tier1/strtools.h"

DLL_EXPORT bool IsBuiltinShaderProviderSelected(
    const BuiltinShaderProvider *provider, const char *requested )
{
	if ( !provider || !provider->id || !provider->id[0] || !provider->legacyModuleName ||
	     !provider->legacyModuleName[0] || !provider->connect || !provider->disconnect )
		return false;
	if ( !requested || !requested[0] )
		return true;
	if ( !Q_stricmp( requested, provider->id ) ||
	     !Q_stricmp( requested, provider->legacyModuleName ) )
		return true;

	// Retained configuration accepts the old basename with this platform's
	// suffix, but never a path, a different library, or runtime discovery.
	int length = Q_strlen( provider->legacyModuleName );
	return !Q_strnicmp( requested, provider->legacyModuleName, length ) &&
	       !Q_stricmp( requested + length, DLL_EXT_STRING );
}
