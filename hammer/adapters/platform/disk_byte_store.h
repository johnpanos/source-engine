//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Non-strict adapter implementing hammer::ports::IByteStore over the
//			local filesystem (RFC 0002, hammer.adapters.platform). It opens a file
//			and serves ranged reads with a seek, so the VPK reader can pull the few
//			kilobytes of a texture it needs out of a multi-hundred-megabyte data
//			archive without loading the whole thing.
//
//			Like DiskFileStore, this is a deliberately NON-strict adapter: it uses
//			<fstream>/<filesystem>, which the strict editor core never may. Native
//			file I/O detail stays quarantined here, behind the port.
//
//=============================================================================//

#ifndef HAMMER_ADAPTERS_PLATFORM_DISK_BYTE_STORE_H
#define HAMMER_ADAPTERS_PLATFORM_DISK_BYTE_STORE_H

#include "hammer/ports/byte_store.h"

namespace hammer::adapters::platform
{

class DiskByteStore : public ports::IByteStore
{
public:
	DiskByteStore() = default;

	bool Size( const std::string &path, std::uint64_t &outSize ) const override;
	bool ReadRange(
	    const std::string &path, std::uint64_t offset, std::size_t length, std::string &out ) const override;
};

} // namespace hammer::adapters::platform

#endif // HAMMER_ADAPTERS_PLATFORM_DISK_BYTE_STORE_H
