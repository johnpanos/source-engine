//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: POSIX platform paths (platform.paths.v1): desktop Linux locations,
//			and root-supplied locations for app containers.
//
//=============================================================================//

#include "foundation_providers.h"

#include <climits>
#include <cstdlib>
#include <cstring>
#include <string>

#include <unistd.h>

namespace platform
{
namespace
{

// Normalizes an absolute native path: collapses repeated '/', resolves "." and
// ".." lexically and drops a trailing '/'. Refuses relative paths, empty
// paths and paths that are not valid UTF-8 (false).
bool NormalizeAbsolute( const char *native, std::string &out )
{
	if ( native == nullptr || native[0] != '/' )
	{
		return false;
	}
	// Reject malformed UTF-8 up front; paths are reported in UTF-8.
	for ( const unsigned char *p = reinterpret_cast<const unsigned char *>( native ); *p != 0; )
	{
		int extra = *p < 0x80                      ? 0
		            : ( *p >= 0xc2 && *p <= 0xdf ) ? 1
		            : ( *p >= 0xe0 && *p <= 0xef ) ? 2
		            : ( *p >= 0xf0 && *p <= 0xf4 ) ? 3
		                                           : -1;
		if ( extra < 0 )
		{
			return false;
		}
		for ( int i = 1; i <= extra; ++i )
		{
			if ( ( p[i] & 0xc0 ) != 0x80 )
			{
				return false;
			}
		}
		p += extra + 1;
	}

	std::string result;
	const char *c = native;
	while ( *c != '\0' )
	{
		while ( *c == '/' )
		{
			++c;
		}
		const char *start = c;
		while ( *c != '\0' && *c != '/' )
		{
			++c;
		}
		const std::string segment( start, c );
		if ( segment.empty() || segment == "." )
		{
			continue;
		}
		if ( segment == ".." )
		{
			const std::size_t slash = result.rfind( '/' );
			result.erase( slash == std::string::npos ? 0 : slash );
			continue;
		}
		result += '/';
		result += segment;
	}
	out = result.empty() ? "/" : result;
	return true;
}

std::string ParentOf( const std::string &path )
{
	const std::size_t slash = path.rfind( '/' );
	return slash == 0 || slash == std::string::npos ? "/" : path.substr( 0, slash );
}

class CPosixPlatformPaths final : public IPlatformPaths
{
public:
	// Slots are indexed by PlatformPathId; an empty string is unavailable.
	std::string values[5];

	bool IsAvailable( PlatformPathId id ) const override { return !Slot( id ).empty(); }

	int GetPath( PlatformPathId id, char *buffer, int bufferSize ) const override
	{
		const std::string &v = Slot( id );
		if ( v.empty() || buffer == nullptr || bufferSize <= static_cast<int>( v.size() ) )
		{
			return -1;
		}
		std::memcpy( buffer, v.c_str(), v.size() + 1 );
		return static_cast<int>( v.size() );
	}

private:
	const std::string &Slot( PlatformPathId id ) const
	{
		static const std::string kNone;
		const int i = static_cast<int>( id );
		return i >= 0 && i < 5 ? values[i] : kNone;
	}
};

bool Assign( CPosixPlatformPaths &paths, PlatformPathId id, const char *native )
{
	if ( native == nullptr )
	{
		return true; // unavailable
	}
	return NormalizeAbsolute( native, paths.values[static_cast<int>( id )] );
}

} // namespace

std::unique_ptr<IPlatformPaths> CreateSuppliedPlatformPaths( const PlatformPathValues &values )
{
	auto paths = std::make_unique<CPosixPlatformPaths>();
	if ( !Assign( *paths, PlatformPathId::kExecutableFile, values.executableFile ) ||
	     !Assign( *paths, PlatformPathId::kUserData, values.userData ) ||
	     !Assign( *paths, PlatformPathId::kTemp, values.temp ) ||
	     !Assign( *paths, PlatformPathId::kNativeLibraryDir, values.nativeLibraryDir ) )
	{
		return nullptr;
	}
	const std::string &file = paths->values[static_cast<int>( PlatformPathId::kExecutableFile )];
	if ( !file.empty() )
	{
		paths->values[static_cast<int>( PlatformPathId::kExecutableDir )] = ParentOf( file );
	}
	return paths;
}

std::unique_ptr<IPlatformPaths> CreateLinuxPlatformPaths()
{
	char exe[PATH_MAX];
	const ssize_t n = readlink( "/proc/self/exe", exe, sizeof( exe ) - 1 );
	if ( n <= 0 )
	{
		return nullptr;
	}
	exe[n] = '\0';

	std::string userData;
	const char *xdg = std::getenv( "XDG_DATA_HOME" );
	const char *home = std::getenv( "HOME" );
	if ( xdg != nullptr && xdg[0] == '/' )
	{
		userData = xdg;
	}
	else if ( home != nullptr && home[0] == '/' )
	{
		userData = std::string( home ) + "/.local/share";
	}
	const char *tmp = std::getenv( "TMPDIR" );
	const std::string temp = tmp != nullptr && tmp[0] == '/' ? tmp : "/tmp";
	std::string exeFile;
	if ( !NormalizeAbsolute( exe, exeFile ) )
	{
		return nullptr;
	}
	const std::string libraries = ParentOf( exeFile );

	// An environment value that cannot be normalized makes only that location
	// unavailable.
	std::string scratch;
	PlatformPathValues values;
	values.executableFile = exe;
	values.userData = !userData.empty() && NormalizeAbsolute( userData.c_str(), scratch )
	                      ? userData.c_str()
	                      : nullptr;
	values.temp = NormalizeAbsolute( temp.c_str(), scratch ) ? temp.c_str() : "/tmp";
	values.nativeLibraryDir = libraries.c_str();
	return CreateSuppliedPlatformPaths( values );
}

} // namespace platform
