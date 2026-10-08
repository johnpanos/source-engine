//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Win32 platform paths (platform.paths.v1).
//
//=============================================================================//

#include "foundation_providers.h"

#include "win32_text.h"

#include <cstring>
#include <initguid.h>
#include <knownfolders.h>
#include <shlobj.h>
#include <string>
#include <vector>

namespace platform
{
namespace
{

// Turns a native absolute path into an engine path: '\' to '/', the "\\?\"
// prefix dropped, repeated separators collapsed, "." and ".." resolved and
// no trailing '/' except after a drive ("C:/"). False when it is not an
// absolute drive path.
bool ToEnginePath( const std::wstring &native, std::string &out )
{
	std::string utf8;
	if ( !win32::ToUtf8( native.c_str(), static_cast<int>( native.size() ), utf8 ) )
	{
		return false;
	}
	for ( char &c : utf8 )
	{
		if ( c == '\\' )
		{
			c = '/';
		}
	}
	if ( utf8.rfind( "//?/", 0 ) == 0 )
	{
		utf8.erase( 0, 4 );
	}
	const bool drive =
	    utf8.size() >= 3 &&
	    ( ( utf8[0] >= 'A' && utf8[0] <= 'Z' ) || ( utf8[0] >= 'a' && utf8[0] <= 'z' ) ) &&
	    utf8[1] == ':' && utf8[2] == '/';
	if ( !drive )
	{
		return false;
	}
	std::vector<std::string> segments;
	std::size_t i = 3;
	while ( i <= utf8.size() )
	{
		const std::size_t slash = utf8.find( '/', i );
		const std::size_t end = slash == std::string::npos ? utf8.size() : slash;
		const std::string segment = utf8.substr( i, end - i );
		if ( segment == ".." )
		{
			if ( !segments.empty() )
			{
				segments.pop_back();
			}
		}
		else if ( !segment.empty() && segment != "." )
		{
			segments.push_back( segment );
		}
		i = end + 1;
	}
	out = utf8.substr( 0, 2 );
	if ( segments.empty() )
	{
		out += '/';
	}
	for ( const std::string &s : segments )
	{
		out += '/';
		out += s;
	}
	return true;
}

std::string ParentOf( const std::string &path )
{
	const std::size_t slash = path.rfind( '/' );
	return slash <= 2 ? path.substr( 0, 3 ) : path.substr( 0, slash );
}

class CWin32PlatformPaths final : public IPlatformPaths
{
public:
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

} // namespace

std::unique_ptr<IPlatformPaths> CreateWin32PlatformPaths()
{
	auto paths = std::make_unique<CWin32PlatformPaths>();
	std::wstring exe( 32768, L'\0' );
	const DWORD n = GetModuleFileNameW( nullptr, exe.data(), static_cast<DWORD>( exe.size() ) );
	if ( n == 0 || n >= exe.size() )
	{
		return nullptr;
	}
	exe.resize( n );
	std::string &file = paths->values[static_cast<int>( PlatformPathId::kExecutableFile )];
	if ( !ToEnginePath( exe, file ) )
	{
		return nullptr;
	}
	paths->values[static_cast<int>( PlatformPathId::kExecutableDir )] = ParentOf( file );
	paths->values[static_cast<int>( PlatformPathId::kNativeLibraryDir )] = ParentOf( file );

	PWSTR appData = nullptr;
	if ( SUCCEEDED( SHGetKnownFolderPath( FOLDERID_LocalAppData, 0, nullptr, &appData ) ) )
	{
		ToEnginePath( appData, paths->values[static_cast<int>( PlatformPathId::kUserData )] );
	}
	CoTaskMemFree( appData );

	std::wstring temp( MAX_PATH + 2, L'\0' );
	const DWORD t = GetTempPathW( static_cast<DWORD>( temp.size() ), temp.data() );
	if ( t > 0 && t < temp.size() )
	{
		temp.resize( t );
		ToEnginePath( temp, paths->values[static_cast<int>( PlatformPathId::kTemp )] );
	}
	return paths;
}

} // namespace platform
