//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the strict-core VPK archive reader. See
//			public/hammer/formats/vpk_archive.h for the contract and format notes.
//
//=============================================================================//

#include "hammer/formats/vpk_archive.h"

#include <algorithm>

namespace hammer::formats
{

namespace
{

constexpr std::uint32_t kVpkSignature = 0x55aa1234u;

// Little-endian scalar reads out of an in-memory buffer with bounds checking.
// 'pos' advances on success; a failure leaves it unspecified and returns false.
bool ReadU32( const std::string &buf, std::size_t &pos, std::uint32_t &out )
{
	if ( pos + 4 > buf.size() )
		return false;
	const unsigned char *p = reinterpret_cast<const unsigned char *>( buf.data() ) + pos;
	out = std::uint32_t( p[0] ) | ( std::uint32_t( p[1] ) << 8 ) | ( std::uint32_t( p[2] ) << 16 )
	    | ( std::uint32_t( p[3] ) << 24 );
	pos += 4;
	return true;
}

bool ReadU16( const std::string &buf, std::size_t &pos, std::uint16_t &out )
{
	if ( pos + 2 > buf.size() )
		return false;
	const unsigned char *p = reinterpret_cast<const unsigned char *>( buf.data() ) + pos;
	out = std::uint16_t( std::uint16_t( p[0] ) | ( std::uint16_t( p[1] ) << 8 ) );
	pos += 2;
	return true;
}

// Reads a NUL-terminated string starting at 'pos'. Returns false if no terminator
// is found before end-of-buffer.
bool ReadCString( const std::string &buf, std::size_t &pos, std::string &out )
{
	std::size_t end = buf.find( '\0', pos );
	if ( end == std::string::npos )
		return false;
	out.assign( buf, pos, end - pos );
	pos = end + 1;
	return true;
}

} // namespace

VpkArchive::VpkArchive( const hammer::ports::IByteStore &store, std::string dirVpkPath )
    : m_store( store ), m_dirVpkPath( std::move( dirVpkPath ) )
{
	// Derive the base name used for data archives: strip a trailing "_dir.vpk"
	// (the usual form) or a plain ".vpk".
	m_baseName = m_dirVpkPath;
	auto ends_with = []( const std::string &s, const std::string &suffix ) {
		return s.size() >= suffix.size() && s.compare( s.size() - suffix.size(), suffix.size(), suffix ) == 0;
	};
	if ( ends_with( m_baseName, "_dir.vpk" ) )
		m_baseName.erase( m_baseName.size() - 8 );
	else if ( ends_with( m_baseName, ".vpk" ) )
		m_baseName.erase( m_baseName.size() - 4 );
}

std::string VpkArchive::DataArchivePath( std::uint16_t archiveIndex ) const
{
	char digits[4];
	// archiveIndex is < kInlineArchiveIndex here; three zero-padded digits is the
	// shipped convention (000..999); wider indices are not produced by vpk tools.
	digits[0] = char( '0' + ( archiveIndex / 100 ) % 10 );
	digits[1] = char( '0' + ( archiveIndex / 10 ) % 10 );
	digits[2] = char( '0' + archiveIndex % 10 );
	digits[3] = '\0';
	return m_baseName + "_" + digits + ".vpk";
}

std::unique_ptr<VpkArchive> VpkArchive::Open(
    const hammer::ports::IByteStore &store, const std::string &dirVpkPath, std::string &error )
{
	std::uint64_t fileSize = 0;
	if ( !store.Size( dirVpkPath, fileSize ) )
	{
		error = "vpk: cannot open directory file '" + dirVpkPath + "'";
		return nullptr;
	}
	if ( fileSize < 12 )
	{
		error = "vpk: directory file too small to hold a header";
		return nullptr;
	}

	// Read the fixed 12-byte prefix (signature, version, treeSize) first, then the
	// version-2 extension if present.
	std::string head;
	if ( !store.ReadRange( dirVpkPath, 0, 12, head ) )
	{
		error = "vpk: failed to read header";
		return nullptr;
	}
	std::size_t pos = 0;
	std::uint32_t signature = 0, version = 0, treeSize = 0;
	ReadU32( head, pos, signature );
	ReadU32( head, pos, version );
	ReadU32( head, pos, treeSize );
	if ( signature != kVpkSignature )
	{
		error = "vpk: bad signature (not a VPK file)";
		return nullptr;
	}
	if ( version != 1 && version != 2 )
	{
		error = "vpk: unsupported version " + std::to_string( version );
		return nullptr;
	}

	const std::uint64_t headerSize = ( version == 1 ) ? 12u : 28u;
	if ( fileSize < headerSize + treeSize )
	{
		error = "vpk: directory file shorter than header+tree";
		return nullptr;
	}

	// Read the whole directory tree into memory (a few MB at most) and parse it.
	std::string tree;
	if ( treeSize > 0 && !store.ReadRange( dirVpkPath, headerSize, treeSize, tree ) )
	{
		error = "vpk: failed to read directory tree";
		return nullptr;
	}

	std::unique_ptr<VpkArchive> archive( new VpkArchive( store, dirVpkPath ) );
	archive->m_version = version;
	archive->m_dataSectionOffset = headerSize + treeSize;

	std::size_t tp = 0;
	while ( true )
	{
		std::string ext;
		if ( !ReadCString( tree, tp, ext ) )
		{
			error = "vpk: truncated extension list";
			return nullptr;
		}
		if ( ext.empty() )
			break; // end of tree

		while ( true )
		{
			std::string dir;
			if ( !ReadCString( tree, tp, dir ) )
			{
				error = "vpk: truncated directory list";
				return nullptr;
			}
			if ( dir.empty() )
				break; // end of this extension

			while ( true )
			{
				std::string name;
				if ( !ReadCString( tree, tp, name ) )
				{
					error = "vpk: truncated file list";
					return nullptr;
				}
				if ( name.empty() )
					break; // end of this directory

				VpkEntry entry;
				std::uint16_t terminator = 0;
				if ( !ReadU32( tree, tp, entry.crc ) || !ReadU16( tree, tp, entry.preloadBytes )
				    || !ReadU16( tree, tp, entry.archiveIndex )
				    || !ReadU32( tree, tp, entry.chunkOffset ) || !ReadU32( tree, tp, entry.chunkLength )
				    || !ReadU16( tree, tp, terminator ) )
				{
					error = "vpk: truncated file record";
					return nullptr;
				}
				if ( terminator != 0xffff )
				{
					error = "vpk: bad file-record terminator";
					return nullptr;
				}

				// The preload run lives inline in the tree right after the record.
				entry.preloadOffset = headerSize + tp;
				if ( tp + entry.preloadBytes > tree.size() )
				{
					error = "vpk: preload run runs past the directory tree";
					return nullptr;
				}
				tp += entry.preloadBytes;

				// Canonical path: "dir/name.ext", or "name.ext" when dir is the
				// root marker " " (a single space) or empty.
				if ( dir == " " || dir.empty() )
					entry.path = name + "." + ext;
				else
					entry.path = dir + "/" + name + "." + ext;

				archive->m_index[entry.path] = archive->m_entries.size();
				archive->m_entries.push_back( std::move( entry ) );
			}
		}
	}

	return archive;
}

const VpkEntry *VpkArchive::Find( const std::string &path ) const
{
	auto it = m_index.find( path );
	if ( it == m_index.end() )
		return nullptr;
	return &m_entries[it->second];
}

bool VpkArchive::HasAsset( const std::string &path ) const
{
	return m_index.find( path ) != m_index.end();
}

bool VpkArchive::ReadAsset( const std::string &path, std::string &out ) const
{
	const VpkEntry *entry = Find( path );
	if ( !entry )
		return false;

	std::string assembled;
	assembled.reserve( static_cast<std::size_t>( entry->TotalSize() ) );

	if ( entry->preloadBytes > 0 )
	{
		std::string preload;
		if ( !m_store.ReadRange( m_dirVpkPath, entry->preloadOffset, entry->preloadBytes, preload ) )
			return false;
		assembled += preload;
	}

	if ( entry->chunkLength > 0 )
	{
		std::string chunk;
		if ( entry->archiveIndex == kInlineArchiveIndex )
		{
			if ( !m_store.ReadRange(
			         m_dirVpkPath, m_dataSectionOffset + entry->chunkOffset, entry->chunkLength, chunk ) )
				return false;
		}
		else
		{
			if ( !m_store.ReadRange(
			         DataArchivePath( entry->archiveIndex ), entry->chunkOffset, entry->chunkLength, chunk ) )
				return false;
		}
		assembled += chunk;
	}

	out = std::move( assembled );
	return true;
}

void VpkArchive::ListAssets(
    const std::string &prefix, const std::string &extensionWithDot, std::vector<std::string> &out ) const
{
	for ( const VpkEntry &entry : m_entries )
	{
		if ( !prefix.empty()
		    && ( entry.path.size() < prefix.size()
		         || entry.path.compare( 0, prefix.size(), prefix ) != 0 ) )
			continue;
		if ( !extensionWithDot.empty()
		    && ( entry.path.size() < extensionWithDot.size()
		         || entry.path.compare(
		                entry.path.size() - extensionWithDot.size(), extensionWithDot.size(), extensionWithDot )
		             != 0 ) )
			continue;
		out.push_back( entry.path );
	}
}

} // namespace hammer::formats
