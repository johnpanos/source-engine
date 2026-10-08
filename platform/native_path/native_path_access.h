//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The only door to a NativePath's native storage (RFC 0001 "Path
//			representation and encoding"). Platform backends build NativePaths
//			from OS strings and read them back to call the OS; archlint lets no
//			portable module include this header.
//
//=============================================================================//

#ifndef PLATFORM_NATIVE_PATH_ACCESS_H
#define PLATFORM_NATIVE_PATH_ACCESS_H

#include "platform/contracts/path_types.h"

#include <string>
#include <utility>

namespace platform
{

class NativePathAccess
{
public:
	// An empty input is the empty path (kNone).
	static NativePath FromPosixBytes( std::string bytes )
	{
		NativePath path;
		if ( !bytes.empty() )
		{
			path.m_flavor = NativePathFlavor::kPosixBytes;
			path.m_bytes = std::move( bytes );
		}
		return path;
	}

	static NativePath FromWindowsUnits( std::u16string units )
	{
		NativePath path;
		if ( !units.empty() )
		{
			path.m_flavor = NativePathFlavor::kWindowsUtf16;
			path.m_units = std::move( units );
		}
		return path;
	}

	static const std::string &PosixBytes( const NativePath &path ) { return path.m_bytes; }
	static const std::u16string &WindowsUnits( const NativePath &path ) { return path.m_units; }
};

} // namespace platform

#endif // PLATFORM_NATIVE_PATH_ACCESS_H
