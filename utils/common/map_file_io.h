//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Bounded file adapters for the map-container tool source/sink API.
//
//=============================================================================//

#ifndef UTILS_COMMON_MAP_FILE_IO_H
#define UTILS_COMMON_MAP_FILE_IO_H

#include "mapcontainer/map_container_builder.h"

#include <cstdint>
#include <fstream>
#include <limits>
#include <string>
#include <utility>

namespace mapcontainer::tooling
{

class FileByteSource final : public IMapByteSource
{
public:
	explicit FileByteSource( const char *pPath ) : m_File( pPath, std::ios::binary )
	{
		if ( !m_File )
			return;
		m_File.seekg( 0, std::ios::end );
		const std::streamoff end = m_File.tellg();
		if ( end < 0 )
			return;
		m_Size = uint64_t( end );
		m_Valid = true;
	}

	bool IsOpen() const { return m_Valid; }
	uint64_t Size() const override { return m_Size; }
	bool ReadAt( uint64_t offset, void *pDest, size_t size ) override
	{
		if ( !m_Valid || offset > m_Size || size > m_Size - offset ||
		     offset > uint64_t( std::numeric_limits<std::streamoff>::max() ) ||
		     size > size_t( std::numeric_limits<std::streamsize>::max() ) )
			return false;
		m_File.clear();
		m_File.seekg( std::streamoff( offset ), std::ios::beg );
		if ( !m_File )
			return false;
		m_File.read( static_cast<char *>( pDest ), std::streamsize( size ) );
		return m_File.gcount() == std::streamsize( size );
	}

private:
	std::ifstream m_File;
	uint64_t m_Size = 0;
	bool m_Valid = false;
};

class FileByteSink final : public IMapByteSink
{
public:
	explicit FileByteSink( std::string path ) : m_Path( std::move( path ) ) {}
	bool ResetToZeroes( uint64_t size ) override
	{
		m_File.close();
		m_File.open( m_Path, std::ios::binary | std::ios::trunc );
		if ( !m_File || size > uint64_t( std::numeric_limits<std::streamoff>::max() ) )
			return false;
		m_Size = size;
		if ( size == 0 )
			return true;
		m_File.seekp( std::streamoff( size - 1 ), std::ios::beg );
		m_File.put( '\0' );
		return !!m_File;
	}
	bool WriteAt( uint64_t offset, const void *pData, size_t size ) override
	{
		if ( !m_File || offset > m_Size || size > m_Size - offset ||
		     size > size_t( std::numeric_limits<std::streamsize>::max() ) )
			return false;
		m_File.seekp( std::streamoff( offset ), std::ios::beg );
		m_File.write( static_cast<const char *>( pData ), std::streamsize( size ) );
		return !!m_File;
	}
	bool Finish()
	{
		m_File.flush();
		m_File.close();
		return !m_File.fail();
	}

private:
	std::string m_Path;
	std::ofstream m_File;
	uint64_t m_Size = 0;
};

} // namespace mapcontainer::tooling

#endif // UTILS_COMMON_MAP_FILE_IO_H
