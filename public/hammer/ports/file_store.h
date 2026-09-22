//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: File-store port for the editor (RFC 0002, hammer.ports). A narrow
//			contract over whole-file primitives, so save orchestration and codecs
//			depend on behavior, not on a concrete filesystem, MFC, or tier0. Real
//			providers and test fakes implement the same interface; that is what
//			lets save-failure atomicity be tested with fault injection.
//
//			This is a genuine polymorphic contract (behavior varies: a real POSIX
//			store vs an in-memory fake vs a fault-injecting fake), so a virtual
//			interface is appropriate here per the RFC's guidance.
//
//=============================================================================//

#ifndef HAMMER_PORTS_FILE_STORE_H
#define HAMMER_PORTS_FILE_STORE_H

#include <string>

namespace hammer::ports
{

class IFileStore
{
public:
	virtual ~IFileStore() = default;

	virtual bool Exists( const std::string &path ) const = 0;

	// Reads the whole file into 'out'. Returns false (out unchanged) if absent.
	virtual bool Read( const std::string &path, std::string &out ) const = 0;

	// Writes the whole file, creating or replacing it. Returns success. A failed
	// write must not partially corrupt an existing file at 'path' (implementations
	// that cannot promise this must document it).
	virtual bool Write( const std::string &path, const std::string &content ) = 0;

	// Atomically replaces 'to' with 'from' (rename). Returns success. When it
	// fails, both paths retain their prior contents.
	virtual bool Rename( const std::string &from, const std::string &to ) = 0;

	// Removes a file. Returns success (true if it did not exist is provider
	// choice; the orchestrator does not rely on that distinction).
	virtual bool Remove( const std::string &path ) = 0;
};

} // namespace hammer::ports

#endif // HAMMER_PORTS_FILE_STORE_H
