//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: POSIX file probe (platform.file-probe.v1, R11) and native-path
//			constructors.
//
//=============================================================================//

#include "foundation_providers.h"

#include "../native_path/native_path_access.h"

#include <climits>
#include <string>

#include <sys/stat.h>
#include <unistd.h>

namespace platform
{
namespace
{

class CPosixFileProbe final : public IFileProbe
{
public:
	FileKind Probe( const NativePath &path ) const override
	{
		if ( path.Flavor() != NativePathFlavor::kPosixBytes )
		{
			return FileKind::kMissing;
		}
		const std::string &bytes = NativePathAccess::PosixBytes( path );
		if ( bytes.find( '\0' ) != std::string::npos )
		{
			return FileKind::kMissing; // not representable as a C path
		}
		struct stat info{};
		if ( stat( bytes.c_str(), &info ) != 0 )
		{
			return FileKind::kMissing;
		}
		if ( S_ISREG( info.st_mode ) )
		{
			return FileKind::kRegularFile;
		}
		return S_ISDIR( info.st_mode ) ? FileKind::kDirectory : FileKind::kOther;
	}
};

} // namespace

std::unique_ptr<IFileProbe> CreatePosixFileProbe()
{
	return std::make_unique<CPosixFileProbe>();
}

NativePath PosixNativePath( const char *bytes )
{
	return NativePathAccess::FromPosixBytes(
	    bytes != nullptr ? std::string( bytes ) : std::string() );
}

NativePath PosixCurrentDirectory()
{
	char buffer[PATH_MAX];
	if ( getcwd( buffer, sizeof( buffer ) ) == nullptr )
	{
		return NativePath();
	}
	return NativePathAccess::FromPosixBytes( buffer );
}

} // namespace platform
