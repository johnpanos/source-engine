//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the disk-backed IByteStore. See disk_byte_store.h.
//
//=============================================================================//

#include "hammer/adapters/platform/disk_byte_store.h"

#include <filesystem>
#include <fstream>

namespace hammer::adapters::platform
{

bool DiskByteStore::Size( const std::string &path, std::uint64_t &outSize ) const
{
	std::error_code ec;
	auto sz = std::filesystem::file_size( path, ec );
	if ( ec )
		return false;
	outSize = static_cast<std::uint64_t>( sz );
	return true;
}

bool DiskByteStore::ReadRange(
    const std::string &path, std::uint64_t offset, std::size_t length, std::string &out ) const
{
	std::ifstream in( path, std::ios::binary );
	if ( !in )
		return false;

	// Bound the request against the actual file size so a short read is a failure,
	// as the contract requires, rather than a silent truncation.
	in.seekg( 0, std::ios::end );
	const std::streamoff fileEnd = in.tellg();
	if ( fileEnd < 0 )
		return false;
	const std::uint64_t fileSize = static_cast<std::uint64_t>( fileEnd );
	if ( offset > fileSize || offset + length > fileSize )
		return false;

	out.resize( length );
	if ( length == 0 )
		return true;

	in.seekg( static_cast<std::streamoff>( offset ), std::ios::beg );
	in.read( out.data(), static_cast<std::streamsize>( length ) );
	if ( static_cast<std::size_t>( in.gcount() ) != length )
	{
		out.clear();
		return false;
	}
	return true;
}

} // namespace hammer::adapters::platform
