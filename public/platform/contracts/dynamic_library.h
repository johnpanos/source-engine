//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Capability contract for dynamic-library loading (RFC 0001).
//
//			This is the reference platform capability from RFC 0001: portable
//			code requests dynamic-library loading through this contract; a
//			Win32, POSIX, console, or deterministic test provider satisfies it.
//			The contract is behavioral, not OS-specific. It intentionally does
//			NOT expose native handle types, and it does not depend on tier0,
//			tier1, or the CreateInterface registry -- module loading and the
//			in-process interface registry are separate responsibilities.
//
//			Semantics required of every provider (see the shared conformance
//			suite unittests/platformtest/dynamic_library/):
//			  - a loaded library has exactly one owner (the loader);
//			  - symbol addresses cannot outlive the loaded library;
//			  - errors are structured and readable without parsing log text;
//			  - unsupported optional behavior is reported explicitly, never
//			    silently emulated;
//			  - the loader cannot be destroyed while libraries it created are
//			    still live.
//
//			Path resolution, depot/local-copy, and signature policy are NOT part
//			of this primitive; they belong to a separate module resolver
//			(RFC 0001, "Module resolver"). This contract opens one already
//			resolved path.
//
//=============================================================================//

#ifndef PLATFORM_CONTRACTS_DYNAMIC_LIBRARY_H
#define PLATFORM_CONTRACTS_DYNAMIC_LIBRARY_H

// Contract header: standard library and foundation vocabulary only. No
// tier0/tier1, no native SDK, no OS-selection macros. This header must compile
// under the linux-headless-core conformance profile (include root "public",
// C++20, -Wall -Wextra -Werror).

#include "foundation/expected.h"

#include <utility>

namespace platform
{

// The operation a structured error refers to. Lets a caller distinguish a load
// failure from a symbol-lookup failure without string parsing.
enum class DynamicLibraryOp
{
	kLoad = 0,
	kFindSymbol,
	kUnload,
	kResolveNoLoad,
};

// Compact domain status. A provider maps its native failure onto exactly one of
// these and additionally carries its native code in DynamicLibraryError.
enum class DynamicLibraryStatus
{
	kOk = 0,
	kInvalidArgument,   // a required argument was null/empty
	kNotFound,          // the library could not be opened at the given path
	kSymbolNotFound,    // the library opened but the symbol is absent
	kUnsupportedNoLoad, // resolve-without-load requested but not supported here
	kProviderError,     // any other provider/native failure (see providerCode)
};

// Structured error payload. It is a plain value: no allocation, no logging, and
// no ownership. `requested` borrows the caller-supplied path or symbol name (the
// caller still owns that string); it is provided so a diagnostic at the
// application boundary can name the exact request without the provider copying
// it. `providerCode` carries the native error (errno / GetLastError / dlerror
// class) when the provider has one, else 0.
struct DynamicLibraryError
{
	DynamicLibraryOp operation = DynamicLibraryOp::kLoad;
	DynamicLibraryStatus status = DynamicLibraryStatus::kOk;
	int providerCode = 0;
	const char *requested = nullptr;

	bool IsOk() const { return status == DynamicLibraryStatus::kOk; }
};

// A loaded library. Owned by the loader that produced it; destroyed only through
// IDynamicLibraryLoader::Unload. Symbol pointers it returns are borrowed and are
// valid only until the library is unloaded.
class IDynamicLibrary
{
public:
	virtual ~IDynamicLibrary() = default;

	// Returns the address of `name`, or nullptr on failure. On failure, when
	// `error` is non-null, it is filled with operation kFindSymbol and a status
	// of kSymbolNotFound (absent symbol) or kInvalidArgument (null/empty name).
	// On success `error`, if provided, is left with status kOk.
	virtual void *FindSymbol( const char *name, DynamicLibraryError *error ) = 0;
};

// Loads and unloads libraries and owns their lifetime. A provider implementation
// is a platform backend; portable code depends only on this interface.
class IDynamicLibraryLoader
{
public:
	virtual ~IDynamicLibraryLoader() = default;

	// Opens the already-resolved `path`. Returns a non-null library owned by this
	// loader on success; the caller must return it through Unload before the
	// loader is destroyed. Returns nullptr on failure and, when `error` is
	// non-null, fills it with operation kLoad and the failing status
	// (kInvalidArgument for a null/empty path, kNotFound when the path cannot be
	// opened, kProviderError otherwise).
	virtual IDynamicLibrary *Load( const char *path, DynamicLibraryError *error ) = 0;

	// Unloads a library previously returned by this loader's Load. After Unload,
	// the pointer and every symbol obtained from it are invalid. Unloading a null
	// or foreign pointer is a no-op.
	virtual void Unload( IDynamicLibrary *library ) = 0;

	// Number of libraries this loader currently owns (loaded and not yet
	// unloaded). Required observable state: it lets a consumer and the
	// conformance suite verify that shutdown releases handles and that a loader
	// is never destroyed while libraries it created are still live.
	virtual int LiveLibraryCount() const = 0;

	// Optional capability: whether the loader can resolve/validate a path without
	// running the library's initialization ("no-load"). Reported explicitly so an
	// unsupported optional behavior is never silently emulated.
	virtual bool SupportsNoLoad() const = 0;

	// Resolve `path` without loading. Returns true when the path is resolvable.
	// When SupportsNoLoad() is false, this MUST return false and, if `error` is
	// non-null, fill it with operation kResolveNoLoad and status
	// kUnsupportedNoLoad -- an explicit report, not a silent success.
	virtual bool TryResolveNoLoad( const char *path, DynamicLibraryError *error ) = 0;
};

// Load-site telemetry (RFC 0001 rank 4). A native provider reports every
// load request, symbol lookup and unload to the observer its composition root
// supplies; the legacy Sys_* bridge adapts it to the Tier 0 telemetry stream.
// Callbacks run synchronously on the calling thread after the native call and
// must not call back into the loader. `library` identifies one load from its
// OnLoad until its OnUnload (null for a failed load). `resolvedPath` is the
// canonical native path the provider opened, or null when the load failed or
// the provider cannot name it. Strings are borrowed for the callback only.
class IDynamicLibraryObserver
{
public:
	virtual ~IDynamicLibraryObserver() = default;
	virtual void OnLoad( const char *path, const char *resolvedPath, const IDynamicLibrary *library,
	    const DynamicLibraryError &result ) = 0;
	virtual void OnFindSymbol( const IDynamicLibrary *library, const char *name, const void *symbol,
	    const DynamicLibraryError &result ) = 0;
	virtual void OnUnload( const IDynamicLibrary *library ) = 0;
};

// Scoped ownership of one loaded library (RFC 0001 "Dynamic-library
// ownership"): a move-only value that returns the library to the loader that
// created it when it is destroyed, reset or assigned over. It is not reference
// counted and allocates nothing. The loader must outlive it, and symbols
// obtained through it are valid only while it owns the library.
class LoadedLibrary
{
public:
	LoadedLibrary() = default;
	LoadedLibrary( IDynamicLibraryLoader &loader, IDynamicLibrary *library ) noexcept
	    : m_Loader( library != nullptr ? &loader : nullptr ), m_Library( library )
	{
	}
	LoadedLibrary( LoadedLibrary &&other ) noexcept
	    : m_Loader( std::exchange( other.m_Loader, nullptr ) ),
	      m_Library( std::exchange( other.m_Library, nullptr ) )
	{
	}
	LoadedLibrary &operator=( LoadedLibrary &&other ) noexcept
	{
		if ( this != &other )
		{
			Reset();
			m_Loader = std::exchange( other.m_Loader, nullptr );
			m_Library = std::exchange( other.m_Library, nullptr );
		}
		return *this;
	}
	LoadedLibrary( const LoadedLibrary & ) = delete;
	LoadedLibrary &operator=( const LoadedLibrary & ) = delete;
	~LoadedLibrary() { Reset(); }

	explicit operator bool() const noexcept { return m_Library != nullptr; }
	IDynamicLibrary *Get() const noexcept { return m_Library; }

	// As IDynamicLibrary::FindSymbol. An empty value finds nothing and reports
	// kInvalidArgument.
	void *FindSymbol( const char *name, DynamicLibraryError *error = nullptr ) const
	{
		if ( m_Library == nullptr )
		{
			if ( error != nullptr )
			{
				*error = { DynamicLibraryOp::kFindSymbol, DynamicLibraryStatus::kInvalidArgument, 0,
				    name };
			}
			return nullptr;
		}
		return m_Library->FindSymbol( name, error );
	}

	// Transfers ownership to the caller, who must return the library through
	// IDynamicLibraryLoader::Unload; the value becomes empty.
	[[nodiscard]] IDynamicLibrary *Release() noexcept
	{
		m_Loader = nullptr;
		return std::exchange( m_Library, nullptr );
	}

	// Returns the library to its loader now; the value becomes empty.
	void Reset() noexcept
	{
		if ( m_Library != nullptr )
		{
			m_Loader->Unload( m_Library );
		}
		m_Loader = nullptr;
		m_Library = nullptr;
	}

private:
	IDynamicLibraryLoader *m_Loader = nullptr;
	IDynamicLibrary *m_Library = nullptr;
};

// Loads `path` through `loader` into a scoped value, or returns the structured
// load error.
[[nodiscard]] inline foundation::Expected<LoadedLibrary, DynamicLibraryError> LoadScoped(
    IDynamicLibraryLoader &loader, const char *path )
{
	DynamicLibraryError error;
	IDynamicLibrary *library = loader.Load( path, &error );
	if ( library == nullptr )
	{
		return foundation::MakeUnexpected( error );
	}
	return LoadedLibrary( loader, library );
}

} // namespace platform

#endif // PLATFORM_CONTRACTS_DYNAMIC_LIBRARY_H
