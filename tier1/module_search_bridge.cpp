//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sys_LoadModule's module search over platform.module-resolver.v1
//			(R11). See module_search_bridge.h.
//
//=============================================================================//

#include "module_search_bridge.h"

#include "../platform/native_path/native_path_access.h"
#include "../platform/posix/foundation_providers.h"
#include "../platform/resolver/module_resolver.h"

#include <cstdio>
#include <cstring>
#include <string>

namespace
{

// The bridge's private probe; Tier 1's legacy Sys_* facade owns it.
const platform::IFileProbe &Probe()
{
	static const std::unique_ptr<platform::IFileProbe> probe = platform::CreatePosixFileProbe();
	return *probe;
}

// The exact native bytes: the result is handed to dlopen, so it must not pass
// through a lossy display conversion.
const std::string &Bytes( const platform::NativePath &path )
{
	return platform::NativePathAccess::PosixBytes( path );
}

void CopyOut( char *out, std::size_t outSize, const std::string &text )
{
	if ( out != nullptr && outSize > 0 )
	{
		std::snprintf( out, outSize, "%s", text.c_str() );
	}
}

} // namespace

bool Tier1_FindModuleWithPrefix( char *out, std::size_t outSize, const char *root,
    const char *moduleName, const char *libDir, const char *extension )
{
	std::string directory = libDir != nullptr ? libDir : "";
	while ( !directory.empty() && directory.back() == '/' )
	{
		directory.pop_back();
	}
	platform::ModuleSearchPolicy policy;
	policy.roots = { platform::PosixNativePath( root ) };
	policy.patterns = { { directory, "lib" }, { directory, "" }, { "", "lib" }, { "", "" } };
	policy.extension = extension != nullptr ? extension : "";
	policy.replaceExtension = true;
	policy.acceptAnyExisting = true; // the legacy stat() rule

	const auto resolver = platform::CreateModuleResolver( Probe(), nullptr );
	auto result = resolver->Resolve( moduleName != nullptr ? moduleName : "", policy );
	if ( result )
	{
		CopyOut( out, outSize, Bytes( result.Value().path ) );
		return true;
	}
	const auto &attempted = result.Error().attempted;
	CopyOut( out, outSize,
	    attempted.empty()
	        ? std::string( root != nullptr ? root : "" ) + "/" +
	              platform::ModuleFileName( moduleName != nullptr ? moduleName : "", policy )
	        : Bytes( attempted.back() ) );
	return false;
}

bool Tier1_ModulePathExists( const char *path )
{
	return Probe().Probe( platform::PosixNativePath( path ) ) != platform::FileKind::kMissing;
}
