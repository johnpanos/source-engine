//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: UTF-16 <-> UTF-8 conversion private to the Win32 providers.
//
//=============================================================================//

#ifndef PLATFORM_WIN32_TEXT_H
#define PLATFORM_WIN32_TEXT_H

#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace platform::win32
{

// Converts UTF-16 to UTF-8; unpaired surrogates fail (false).
inline bool ToUtf8( const wchar_t *text, int length, std::string &out )
{
	out.clear();
	if ( length == 0 )
	{
		return true;
	}
	const int n = WideCharToMultiByte(
	    CP_UTF8, WC_ERR_INVALID_CHARS, text, length, nullptr, 0, nullptr, nullptr );
	if ( n <= 0 )
	{
		return false;
	}
	out.resize( static_cast<std::size_t>( n ) );
	WideCharToMultiByte(
	    CP_UTF8, WC_ERR_INVALID_CHARS, text, length, out.data(), n, nullptr, nullptr );
	return true;
}

inline bool ToUtf8( const wchar_t *text, std::string &out )
{
	return ToUtf8( text, static_cast<int>( wcslen( text ) ), out );
}

inline std::wstring ToWide( const char *utf8 )
{
	const int n = MultiByteToWideChar( CP_UTF8, MB_ERR_INVALID_CHARS, utf8, -1, nullptr, 0 );
	if ( n <= 0 )
	{
		return std::wstring();
	}
	std::wstring out( static_cast<std::size_t>( n ), L'\0' );
	MultiByteToWideChar( CP_UTF8, MB_ERR_INVALID_CHARS, utf8, -1, out.data(), n );
	out.resize( static_cast<std::size_t>( n - 1 ) );
	return out;
}

} // namespace platform::win32

#endif // PLATFORM_WIN32_TEXT_H
