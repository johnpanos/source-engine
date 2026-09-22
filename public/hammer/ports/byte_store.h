//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Random-access, read-only byte-source port for the editor (RFC 0002,
//			hammer.ports). A narrow contract over ranged reads of an immutable
//			file, so archive/asset codecs (VPK, VTF) can pull the bytes they need
//			without reading whole multi-hundred-megabyte pack files into memory,
//			and without depending on a concrete filesystem, MFC, or tier0.
//
//			This is deliberately distinct from IFileStore: IFileStore is the
//			whole-file, MUTABLE save/load contract (Read/Write/Rename/Remove).
//			IByteStore is whole-file-immutable but RANGED (Size/ReadRange). Real
//			providers (a POSIX stream store) and test fakes (an in-memory buffer
//			map, a fault-injecting fake) implement the same interface, which is
//			what lets VPK chunk resolution be tested with hand-built fixtures.
//
//			This is a genuine polymorphic contract (behavior varies: a real disk
//			store vs an in-memory fake), so a virtual interface is appropriate
//			here per the RFC's guidance.
//
//=============================================================================//

#ifndef HAMMER_PORTS_BYTE_STORE_H
#define HAMMER_PORTS_BYTE_STORE_H

#include <cstdint>
#include <string>

namespace hammer::ports
{

class IByteStore
{
public:
	virtual ~IByteStore() = default;

	// Total byte length of 'path'. Returns false (outSize unchanged) when the file
	// is absent or unreadable.
	virtual bool Size( const std::string &path, std::uint64_t &outSize ) const = 0;

	// Reads exactly 'length' bytes starting at byte 'offset' into 'out' (assigned,
	// not appended). Returns false (out unchanged) when the file is unreadable or
	// the requested range extends past end-of-file: a short read is a failure, not
	// a truncation, so codecs never silently consume partial records. A zero-length
	// read at a valid offset succeeds with an empty 'out'.
	virtual bool ReadRange( const std::string &path, std::uint64_t offset, std::size_t length,
	    std::string &out ) const = 0;
};

} // namespace hammer::ports

#endif // HAMMER_PORTS_BYTE_STORE_H
