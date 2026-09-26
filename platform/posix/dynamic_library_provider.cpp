//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: POSIX implementation of the dynamic-library loader contract.
//
//=============================================================================//

#include "dynamic_library_provider.h"

#include <algorithm>
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <memory>
#include <mutex>
#include <string>
#include <unistd.h>
#include <vector>

namespace platform
{

namespace
{

DynamicLibraryError Failure(
    DynamicLibraryOp operation, DynamicLibraryStatus status, const char *requested, int code = 0 )
{
	return { operation, status, code, requested };
}

void Report( DynamicLibraryError *error, const DynamicLibraryError &value )
{
	if ( error != nullptr )
	{
		*error = value;
	}
}

class PosixDynamicLibrary final : public IDynamicLibrary
{
public:
	PosixDynamicLibrary( void *handle, IDynamicLibraryObserver &observer )
	    : m_Handle( handle ), m_Observer( observer )
	{
	}

	void *FindSymbol( const char *name, DynamicLibraryError *error ) override
	{
		void *symbol = nullptr;
		DynamicLibraryError result =
		    Failure( DynamicLibraryOp::kFindSymbol, DynamicLibraryStatus::kOk, name );
		if ( name == nullptr || name[0] == '\0' )
		{
			result.status = DynamicLibraryStatus::kInvalidArgument;
		}
		else
		{
			// dlerror is per-thread; clear it so a null result can be told
			// apart from a symbol whose value is null.
			dlerror();
			symbol = dlsym( m_Handle, name );
			if ( symbol == nullptr && dlerror() != nullptr )
			{
				result.status = DynamicLibraryStatus::kSymbolNotFound;
			}
		}
		m_Observer.OnFindSymbol( this, name, symbol, result );
		Report( error, result );
		return symbol;
	}

	void *Handle() const { return m_Handle; }

private:
	void *m_Handle;
	IDynamicLibraryObserver &m_Observer;
};

class PosixDynamicLibraryLoader final : public IDynamicLibraryLoader
{
public:
	explicit PosixDynamicLibraryLoader( IDynamicLibraryObserver &observer ) : m_Observer( observer )
	{
	}

	~PosixDynamicLibraryLoader() override
	{
		std::lock_guard<std::mutex> lock( m_Mutex );
		if ( !m_Live.empty() )
		{
			// Symbols handed out by live libraries would dangle; a loader that
			// outlives none of its libraries is a contract violation, not a
			// recoverable state.
			std::fprintf( stderr, "platform: POSIX loader destroyed with %zu live librar%s\n",
			    m_Live.size(), m_Live.size() == 1 ? "y" : "ies" );
			std::abort();
		}
	}

	IDynamicLibrary *Load( const char *path, DynamicLibraryError *error ) override
	{
		DynamicLibraryError result =
		    Failure( DynamicLibraryOp::kLoad, DynamicLibraryStatus::kOk, path );
		std::string resolved;
		PosixDynamicLibrary *library = Open( path, result, resolved );
		m_Observer.OnLoad( path, library != nullptr ? resolved.c_str() : nullptr, library, result );
		Report( error, result );
		return library;
	}

	void Unload( IDynamicLibrary *library ) override
	{
		std::unique_ptr<PosixDynamicLibrary> owned;
		{
			std::lock_guard<std::mutex> lock( m_Mutex );
			auto found = std::find_if( m_Live.begin(), m_Live.end(),
			    [library]( const std::unique_ptr<PosixDynamicLibrary> &live )
			    {
				    return live.get() == library;
			    } );
			// Null and foreign pointers are a no-op.
			if ( found == m_Live.end() )
			{
				return;
			}
			owned = std::move( *found );
			m_Live.erase( found );
		}
		dlclose( owned->Handle() );
		m_Observer.OnUnload( owned.get() );
	}

	int LiveLibraryCount() const override
	{
		std::lock_guard<std::mutex> lock( m_Mutex );
		return static_cast<int>( m_Live.size() );
	}

	bool SupportsNoLoad() const override { return false; }

	bool TryResolveNoLoad( const char *path, DynamicLibraryError *error ) override
	{
		Report( error, Failure( DynamicLibraryOp::kResolveNoLoad,
		                   DynamicLibraryStatus::kUnsupportedNoLoad, path ) );
		return false;
	}

private:
	PosixDynamicLibrary *Open(
	    const char *path, DynamicLibraryError &result, std::string &resolved )
	{
		if ( path == nullptr || path[0] == '\0' )
		{
			result.status = DynamicLibraryStatus::kInvalidArgument;
			return nullptr;
		}
		// A bare file name would make dlopen search the library path; open the
		// given file instead.
		const std::string opened =
		    std::strchr( path, '/' ) != nullptr ? std::string( path ) : std::string( "./" ) + path;
		if ( access( opened.c_str(), F_OK ) != 0 )
		{
			result.status = DynamicLibraryStatus::kNotFound;
			result.providerCode = errno;
			return nullptr;
		}
		void *handle = dlopen( opened.c_str(), RTLD_NOW | RTLD_LOCAL );
		if ( handle == nullptr )
		{
			// The file exists but is not a loadable library (format, missing
			// dependency, relocation). dlopen reports text only, no number.
			result.status = DynamicLibraryStatus::kProviderError;
			return nullptr;
		}
		// Telemetry names the file actually opened, canonicalized when possible.
		char canonical[PATH_MAX];
		resolved = realpath( opened.c_str(), canonical ) != nullptr ? canonical : opened;
		auto library = std::make_unique<PosixDynamicLibrary>( handle, m_Observer );
		PosixDynamicLibrary *created = library.get();
		std::lock_guard<std::mutex> lock( m_Mutex );
		m_Live.push_back( std::move( library ) );
		return created;
	}

	IDynamicLibraryObserver &m_Observer;
	mutable std::mutex m_Mutex;
	std::vector<std::unique_ptr<PosixDynamicLibrary>> m_Live;
};

} // namespace

std::unique_ptr<IDynamicLibraryLoader> CreatePosixDynamicLibraryLoader(
    IDynamicLibraryObserver &observer )
{
	return std::make_unique<PosixDynamicLibraryLoader>( observer );
}

} // namespace platform
