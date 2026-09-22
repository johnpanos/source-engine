//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Test fake for hammer::ports::IFileStore (RFC 0002). In-memory store
//			with fault injection, used to exercise save-failure atomicity. It
//			implements exactly the production port and uses no application
//			singletons or backdoors, per the RFC's fake-provider rule.
//
//=============================================================================//

#ifndef HAMMERTEST_FAKE_FILE_STORE_H
#define HAMMERTEST_FAKE_FILE_STORE_H

#include "hammer/ports/file_store.h"

#include <map>
#include <string>

namespace hammertest
{

class InMemoryFileStore : public hammer::ports::IFileStore
{
public:
	std::map<std::string, std::string> files;
	bool failAllWrites = false;
	bool failNextRename = false;

	bool Exists( const std::string &path ) const override { return files.count( path ) != 0; }

	bool Read( const std::string &path, std::string &out ) const override
	{
		const auto it = files.find( path );
		if ( it == files.end() )
		{
			return false;
		}
		out = it->second;
		return true;
	}

	bool Write( const std::string &path, const std::string &content ) override
	{
		if ( failAllWrites )
		{
			return false; // whole-file write fails without touching an existing file
		}
		files[path] = content;
		return true;
	}

	bool Rename( const std::string &from, const std::string &to ) override
	{
		if ( failNextRename )
		{
			failNextRename = false;
			return false; // both paths keep their prior contents
		}
		const auto it = files.find( from );
		if ( it == files.end() )
		{
			return false;
		}
		files[to] = it->second;
		files.erase( it );
		return true;
	}

	bool Remove( const std::string &path ) override
	{
		files.erase( path );
		return true;
	}
};

} // namespace hammertest

#endif // HAMMERTEST_FAKE_FILE_STORE_H
