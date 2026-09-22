//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer::adapters::platform::DiskFileStore (RFC 0002).
//			See hammer/adapters/platform/disk_file_store.h.
//
//=============================================================================//

#include "hammer/adapters/platform/disk_file_store.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <system_error>

namespace hammer::adapters::platform
{

namespace fs = std::filesystem;

bool DiskFileStore::Exists( const std::string &path ) const
{
	std::error_code ec;
	return fs::exists( fs::path( path ), ec ) && !ec;
}

bool DiskFileStore::Read( const std::string &path, std::string &out ) const
{
	std::ifstream in( path, std::ios::binary );
	if ( !in )
	{
		return false; // absent or unreadable; out unchanged per the contract
	}
	std::string content( ( std::istreambuf_iterator<char>( in ) ),
						 std::istreambuf_iterator<char>() );
	if ( in.bad() )
	{
		return false;
	}
	out = std::move( content );
	return true;
}

bool DiskFileStore::Write( const std::string &path, const std::string &content )
{
	// Atomic whole-file write: stage into a sibling temp file, flush, then rename
	// over the target. A failure before the rename leaves any existing file intact.
	const fs::path target( path );
	fs::path tmp = target;
	tmp += ".hammer-tmp";

	{
		std::ofstream out( tmp, std::ios::binary | std::ios::trunc );
		if ( !out )
		{
			return false;
		}
		out.write( content.data(), static_cast<std::streamsize>( content.size() ) );
		out.flush();
		if ( !out )
		{
			out.close();
			std::error_code rm;
			fs::remove( tmp, rm );
			return false;
		}
	}

	std::error_code ec;
	fs::rename( tmp, target, ec );
	if ( ec )
	{
		std::error_code rm;
		fs::remove( tmp, rm );
		return false;
	}
	return true;
}

bool DiskFileStore::Rename( const std::string &from, const std::string &to )
{
	std::error_code ec;
	fs::rename( fs::path( from ), fs::path( to ), ec );
	return !ec;
}

bool DiskFileStore::Remove( const std::string &path )
{
	std::error_code ec;
	fs::remove( fs::path( path ), ec );
	return !ec;
}

} // namespace hammer::adapters::platform
