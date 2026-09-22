//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared conformance clauses for hammer::ports::IFileStore
//			(ports.file_store.v1). IFileStore is an explicitly polymorphic contract
//			(real filesystem vs in-memory fake vs fault-injecting fake), so this
//			one suite runs against every claiming implementation -- the real
//			DiskFileStore, the InMemoryFileStore test fake, and a deliberately
//			broken provider -- through the base interface. Returns the number of
//			failed clauses.
//
//=============================================================================//

#ifndef HAMMERTEST_PORTS_FILE_STORE_CONTRACT_H
#define HAMMERTEST_PORTS_FILE_STORE_CONTRACT_H

#include "hammer/ports/file_store.h"

#include <cstdio>
#include <string>

namespace hammertest
{

// Exercises the whole-file contract against `store`, using `prefix` to form paths
// (so a disk store and an in-memory store can run in one process without
// colliding; for a disk store, the directory containing `prefix` must exist).
inline int RunFileStoreContract( hammer::ports::IFileStore &store, const std::string &prefix,
								 const char *label )
{
	int failures = 0;
	auto check = [&]( bool ok, const char *clause )
	{
		if ( !ok )
		{
			std::printf( "FAIL [%s] %s\n", label, clause );
			++failures;
		}
	};

	const std::string a = prefix + "alpha.txt";
	const std::string b = prefix + "beta.txt";
	const std::string missing = prefix + "nope.txt";

	// (1) Absent file: Exists false, Read false and leaves out unchanged.
	check( !store.Exists( missing ), "Exists(absent) == false" );
	std::string out = "SENTINEL";
	check( !store.Read( missing, out ), "Read(absent) == false" );
	check( out == "SENTINEL", "Read(absent) leaves out unchanged" );

	// (2) Write creates; Exists true; Read round-trips exactly.
	check( store.Write( a, "hello world" ), "Write(new) == true" );
	check( store.Exists( a ), "Exists after write" );
	out.clear();
	check( store.Read( a, out ) && out == "hello world", "Read round-trips content" );

	// (3) Write replaces existing content wholesale.
	check( store.Write( a, "second" ), "Write(replace) == true" );
	out.clear();
	check( store.Read( a, out ) && out == "second", "Read reflects replacement" );

	// (4) Rename moves content: source gone, target holds it.
	check( store.Rename( a, b ), "Rename(existing) == true" );
	check( !store.Exists( a ), "source gone after rename" );
	check( store.Exists( b ), "target present after rename" );
	out.clear();
	check( store.Read( b, out ) && out == "second", "renamed target keeps content" );

	// (5) Rename of an absent source fails.
	check( !store.Rename( missing, a ), "Rename(absent source) == false" );

	// (6) Remove deletes.
	check( store.Remove( b ), "Remove == true" );
	check( !store.Exists( b ), "gone after remove" );

	return failures;
}

} // namespace hammertest

#endif // HAMMERTEST_PORTS_FILE_STORE_CONTRACT_H
