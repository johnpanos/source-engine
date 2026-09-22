//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Concrete IFileStore provider for the GTK desktop shell (RFC 0002,
//			hammer.ports consumer). The editor core depends only on the narrow
//			ports::IFileStore contract; this host-owned adapter binds it to the
//			real filesystem via std::filesystem. It is intentionally part of the
//			GTK product, not the strict headless core: the core stays free of any
//			concrete filesystem so its save-atomicity is testable with fakes.
//
//			Write() is atomic-ish (write temp + rename) so a failed write cannot
//			corrupt an existing file, matching the IFileStore::Write guarantee.
//
//=============================================================================//

#ifndef HAMMER_GTK_POSIX_FILE_STORE_H
#define HAMMER_GTK_POSIX_FILE_STORE_H

#include "hammer/ports/file_store.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>

namespace hammergtk
{

class PosixFileStore final : public hammer::ports::IFileStore
{
public:
	bool Exists( const std::string &path ) const override
	{
		std::error_code ec;
		return std::filesystem::exists( std::filesystem::path( path ), ec ) && !ec;
	}

	bool Read( const std::string &path, std::string &out ) const override
	{
		std::ifstream in( path, std::ios::binary );
		if ( !in )
		{
			return false;
		}
		std::ostringstream ss;
		ss << in.rdbuf();
		if ( !in.good() && !in.eof() )
		{
			return false;
		}
		out = ss.str();
		return true;
	}

	bool Write( const std::string &path, const std::string &content ) override
	{
		// Write to a sibling temp file then atomically rename over the target, so
		// a failure mid-write leaves any existing file intact.
		const std::string tmp = path + ".tmp-hammergtk";
		{
			std::ofstream os( tmp, std::ios::binary | std::ios::trunc );
			if ( !os )
			{
				return false;
			}
			os << content;
			os.flush();
			if ( !os.good() )
			{
				os.close();
				std::remove( tmp.c_str() );
				return false;
			}
		}
		std::error_code ec;
		std::filesystem::rename( tmp, path, ec );
		if ( ec )
		{
			std::remove( tmp.c_str() );
			return false;
		}
		return true;
	}

	bool Rename( const std::string &from, const std::string &to ) override
	{
		std::error_code ec;
		std::filesystem::rename( from, to, ec );
		return !ec;
	}

	bool Remove( const std::string &path ) override
	{
		std::error_code ec;
		std::filesystem::remove( std::filesystem::path( path ), ec );
		return !ec;
	}
};

} // namespace hammergtk

#endif // HAMMER_GTK_POSIX_FILE_STORE_H
