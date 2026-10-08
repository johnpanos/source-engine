//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Win32 file probe (platform.file-probe.v1, R11) and native-path
//			constructors.
//
//=============================================================================//

#include "foundation_providers.h"

#include "../native_path/native_path_access.h"
#include "win32_text.h"

#include <string>

namespace platform
{
namespace
{

class CWin32FileProbe final : public IFileProbe
{
public:
	FileKind Probe( const NativePath &path ) const override
	{
		if ( path.Flavor() != NativePathFlavor::kWindowsUtf16 )
		{
			return FileKind::kMissing;
		}
		const std::u16string &units = NativePathAccess::WindowsUnits( path );
		if ( units.find( u'\0' ) != std::u16string::npos )
		{
			return FileKind::kMissing;
		}
		const std::wstring wide( units.begin(), units.end() );
		// Open the target the way the loader would, so a link resolves to what
		// it names; a dangling link is missing.
		HANDLE file =
		    CreateFileW( wide.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		        nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr );
		if ( file == INVALID_HANDLE_VALUE )
		{
			return FileKind::kMissing;
		}
		BY_HANDLE_FILE_INFORMATION info{};
		const BOOL ok = GetFileInformationByHandle( file, &info );
		const DWORD type = GetFileType( file );
		CloseHandle( file );
		if ( !ok )
		{
			return type == FILE_TYPE_DISK ? FileKind::kMissing : FileKind::kOther;
		}
		if ( ( info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 )
		{
			return FileKind::kDirectory;
		}
		return type == FILE_TYPE_DISK ? FileKind::kRegularFile : FileKind::kOther;
	}
};

} // namespace

std::unique_ptr<IFileProbe> CreateWin32FileProbe()
{
	return std::make_unique<CWin32FileProbe>();
}

NativePath Win32NativePath( const wchar_t *units )
{
	if ( units == nullptr )
	{
		return NativePath();
	}
	const std::wstring wide( units );
	return NativePathAccess::FromWindowsUnits( std::u16string( wide.begin(), wide.end() ) );
}

NativePath Win32CurrentDirectory()
{
	const DWORD n = GetCurrentDirectoryW( 0, nullptr );
	if ( n == 0 )
	{
		return NativePath();
	}
	std::wstring wide( n, L'\0' );
	const DWORD written = GetCurrentDirectoryW( n, wide.data() );
	wide.resize( written );
	return NativePathAccess::FromWindowsUnits( std::u16string( wide.begin(), wide.end() ) );
}

} // namespace platform
