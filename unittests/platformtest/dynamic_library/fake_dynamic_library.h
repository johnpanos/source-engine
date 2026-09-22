//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Deterministic test backend for platform::IDynamicLibraryLoader
//			(RFC 0001 "test backend"). It loads in-memory fixture libraries with
//			registered symbols -- no OS, no filesystem, no native handles -- so
//			subsystem tests can supply a fake loader without initializing tier1
//			globals (an explicit RFC 0001 acceptance criterion).
//
//			This fake is a CONFORMING provider: it is the positive subject of the
//			shared conformance suite. It is intentionally simple and owns the
//			libraries it hands out; destroying it asserts nothing is still live.
//
//=============================================================================//

#ifndef PLATFORMTEST_FAKE_DYNAMIC_LIBRARY_H
#define PLATFORMTEST_FAKE_DYNAMIC_LIBRARY_H

#include "platform/contracts/dynamic_library.h"

#include <cassert>
#include <cstring>
#include <vector>

namespace platformtest
{

// One in-memory fixture library definition: a path plus a small symbol table.
struct FakeLibraryDef
{
	struct Symbol
	{
		const char *name;
		void *address;
	};

	const char *path;
	std::vector<Symbol> symbols;
};

// A loaded in-memory library. Borrows its symbol table from the loader's
// registry (stable for the loader's lifetime).
class CFakeDynamicLibrary : public platform::IDynamicLibrary
{
public:
	explicit CFakeDynamicLibrary( const FakeLibraryDef *def ) : m_def( def ) {}

	void *FindSymbol( const char *name, platform::DynamicLibraryError *error ) override
	{
		if ( name == nullptr || name[0] == '\0' )
		{
			Fail( error, name, platform::DynamicLibraryStatus::kInvalidArgument );
			return nullptr;
		}
		for ( const FakeLibraryDef::Symbol &s : m_def->symbols )
		{
			if ( std::strcmp( s.name, name ) == 0 )
			{
				if ( error != nullptr )
				{
					*error = platform::DynamicLibraryError{};
				}
				return s.address;
			}
		}
		Fail( error, name, platform::DynamicLibraryStatus::kSymbolNotFound );
		return nullptr;
	}

private:
	static void Fail( platform::DynamicLibraryError *error, const char *requested,
		platform::DynamicLibraryStatus status )
	{
		if ( error != nullptr )
		{
			error->operation = platform::DynamicLibraryOp::kFindSymbol;
			error->status = status;
			error->providerCode = 0;
			error->requested = requested;
		}
	}

	const FakeLibraryDef *m_def;
};

// The deterministic loader. Seeded with fixture definitions; loads them by path.
class CFakeDynamicLibraryLoader : public platform::IDynamicLibraryLoader
{
public:
	explicit CFakeDynamicLibraryLoader( std::vector<FakeLibraryDef> defs,
		bool supportsNoLoad = false )
		: m_defs( std::move( defs ) ), m_supportsNoLoad( supportsNoLoad )
	{
	}

	~CFakeDynamicLibraryLoader() override
	{
		// A loader must not be destroyed while it still owns live libraries
		// (RFC 0001 lifetime rule). In the test backend we assert it.
		assert( m_live.empty() && "loader destroyed with live libraries" );
		for ( CFakeDynamicLibrary *lib : m_live )
		{
			delete lib;
		}
	}

	platform::IDynamicLibrary *Load( const char *path,
		platform::DynamicLibraryError *error ) override
	{
		if ( path == nullptr || path[0] == '\0' )
		{
			FailLoad( error, path, platform::DynamicLibraryStatus::kInvalidArgument );
			return nullptr;
		}
		const FakeLibraryDef *def = Find( path );
		if ( def == nullptr )
		{
			FailLoad( error, path, platform::DynamicLibraryStatus::kNotFound );
			return nullptr;
		}
		if ( error != nullptr )
		{
			*error = platform::DynamicLibraryError{};
		}
		CFakeDynamicLibrary *lib = new CFakeDynamicLibrary( def );
		m_live.push_back( lib );
		return lib;
	}

	void Unload( platform::IDynamicLibrary *library ) override
	{
		for ( std::size_t i = 0; i < m_live.size(); ++i )
		{
			if ( m_live[i] == library )
			{
				delete m_live[i];
				m_live.erase( m_live.begin() + static_cast<std::ptrdiff_t>( i ) );
				return;
			}
		}
		// Null or foreign pointer: no-op, per contract.
	}

	int LiveLibraryCount() const override
	{
		return static_cast<int>( m_live.size() );
	}

	bool SupportsNoLoad() const override { return m_supportsNoLoad; }

	bool TryResolveNoLoad( const char *path,
		platform::DynamicLibraryError *error ) override
	{
		if ( !m_supportsNoLoad )
		{
			if ( error != nullptr )
			{
				error->operation = platform::DynamicLibraryOp::kResolveNoLoad;
				error->status = platform::DynamicLibraryStatus::kUnsupportedNoLoad;
				error->providerCode = 0;
				error->requested = path;
			}
			return false;
		}
		const bool ok = Find( path ) != nullptr;
		if ( error != nullptr )
		{
			*error = platform::DynamicLibraryError{};
			if ( !ok )
			{
				error->operation = platform::DynamicLibraryOp::kResolveNoLoad;
				error->status = platform::DynamicLibraryStatus::kNotFound;
				error->requested = path;
			}
		}
		return ok;
	}

private:
	static void FailLoad( platform::DynamicLibraryError *error, const char *requested,
		platform::DynamicLibraryStatus status )
	{
		if ( error != nullptr )
		{
			error->operation = platform::DynamicLibraryOp::kLoad;
			error->status = status;
			error->providerCode = 0;
			error->requested = requested;
		}
	}

	const FakeLibraryDef *Find( const char *path ) const
	{
		if ( path == nullptr )
		{
			return nullptr;
		}
		for ( const FakeLibraryDef &def : m_defs )
		{
			if ( std::strcmp( def.path, path ) == 0 )
			{
				return &def;
			}
		}
		return nullptr;
	}

	std::vector<FakeLibraryDef> m_defs;
	std::vector<CFakeDynamicLibrary *> m_live;
	bool m_supportsNoLoad;
};

} // namespace platformtest

#endif // PLATFORMTEST_FAKE_DYNAMIC_LIBRARY_H
