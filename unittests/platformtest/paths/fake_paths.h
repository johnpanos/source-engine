//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Deterministic test backend for platform::IPlatformPaths (RFC 0001).
//			Returns fixed, normalized engine paths with no OS access, so subsystem
//			tests get reproducible locations. kNativeLibraryDir is reported
//			unavailable to exercise the explicit-absence path. This is a
//			CONFORMING provider: the positive subject of the shared suite.
//
//=============================================================================//

#ifndef PLATFORMTEST_FAKE_PATHS_H
#define PLATFORMTEST_FAKE_PATHS_H

#include "platform/contracts/paths.h"

#include <cstring>

namespace platformtest
{

class CFakePlatformPaths : public platform::IPlatformPaths
{
public:
	bool IsAvailable( platform::PlatformPathId id ) const override
	{
		return Value( id ) != nullptr;
	}

	int GetPath( platform::PlatformPathId id, char *buffer, int bufferSize ) const override
	{
		const char *value = Value( id );
		if ( value == nullptr || buffer == nullptr || bufferSize <= 0 )
		{
			return -1;
		}
		const int len = static_cast<int>( std::strlen( value ) );
		if ( len + 1 > bufferSize )
		{
			return -1; // does not fit; no partial write
		}
		std::memcpy( buffer, value, static_cast<std::size_t>( len ) + 1 );
		return len;
	}

private:
	static const char *Value( platform::PlatformPathId id )
	{
		switch ( id )
		{
		case platform::PlatformPathId::kExecutableFile:
			return "/opt/game/bin/engine";
		case platform::PlatformPathId::kExecutableDir:
			return "/opt/game/bin";
		case platform::PlatformPathId::kUserData:
			return "/home/player/.local/share/game";
		case platform::PlatformPathId::kTemp:
			return "/tmp/game";
		case platform::PlatformPathId::kNativeLibraryDir:
			return nullptr; // intentionally unavailable in the fake
		}
		return nullptr;
	}
};

} // namespace platformtest

#endif // PLATFORMTEST_FAKE_PATHS_H
