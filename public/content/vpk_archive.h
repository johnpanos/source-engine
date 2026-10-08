//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: content.vpk: a dependency-free reader for Valve Pack (VPK)
//			archives, shared by Hammer, kiln's packagers and content tools
//			(extracted from hammer.formats, RFC 0027 "Libraries first").
//
//			A .vpk "directory" file (xxx_dir.vpk) holds a tree of extension ->
//			path -> file records; each record's payload is a short "preload"
//			run stored inline in the directory plus a chunk stored either in
//			the directory's own data section or in a sibling data archive
//			(xxx_000.vpk, xxx_001.vpk, ...). This reader parses that tree and
//			resolves a file's bytes on demand through an IByteSource.
//
//			Supports VPK versions 1 and 2 (the two that exist in shipped
//			content). Signature and MD5 sections in v2 are not verified: a read
//			path, not a trust boundary. No global state; an archive is safe to
//			read from several threads if its byte source is.
//
//=============================================================================//

#ifndef PUBLIC_CONTENT_VPK_ARCHIVE_H
#define PUBLIC_CONTENT_VPK_ARCHIVE_H

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace content
{

// Ranged reads of named files. A short read is a failure, never a truncation.
class IByteSource
{
public:
	virtual ~IByteSource() = default;
	// Total byte length of 'path'; false (outSize unchanged) when absent.
	virtual bool Size( const std::string &path, std::uint64_t &outSize ) const = 0;
	// Exactly 'length' bytes from 'offset' into 'out' (assigned); false when
	// unreadable or past end-of-file. A zero-length read at a valid offset
	// succeeds with an empty 'out'.
	virtual bool ReadRange( const std::string &path, std::uint64_t offset, std::size_t length,
	    std::string &out ) const = 0;
};

// IByteSource over the host file system; paths are used as given.
class FileByteSource final : public IByteSource
{
public:
	bool Size( const std::string &path, std::uint64_t &outSize ) const override;
	bool ReadRange( const std::string &path, std::uint64_t offset, std::size_t length,
	    std::string &out ) const override;
};

// One resolved directory record. 'preloadBytes' precede the chunk and live in
// the directory file at 'preloadOffset'. The chunk lives in the data archive
// 'archiveIndex' at 'chunkOffset', or, for kInlineArchiveIndex, in the
// directory file's own data section.
struct VpkEntry
{
	std::string path; // "dir/name.ext" as stored (shipped archives are lower case)
	std::uint32_t crc = 0;
	std::uint16_t archiveIndex = 0;
	std::uint16_t preloadBytes = 0;
	std::uint64_t preloadOffset = 0; // absolute offset in the directory file
	std::uint32_t chunkOffset = 0;
	std::uint32_t chunkLength = 0;

	std::uint64_t TotalSize() const { return std::uint64_t( preloadBytes ) + chunkLength; }
};

class VpkArchive
{
public:
	static constexpr std::uint16_t kInlineArchiveIndex = 0x7fff;

	// Parses the VPK whose directory file is 'dirVpkPath', reading through
	// 'source', which must outlive the archive. nullptr and 'error' on a
	// malformed or absent archive.
	static std::unique_ptr<VpkArchive> Open(
	    const IByteSource &source, const std::string &dirVpkPath, std::string &error );

	std::uint32_t Version() const { return m_version; }
	const std::vector<VpkEntry> &Entries() const { return m_entries; }
	const VpkEntry *Find( const std::string &path ) const;
	// The file's bytes; false when absent or unreadable.
	bool Read( const std::string &path, std::string &out ) const;
	// Paths under 'prefix' ending in 'extensionWithDot' (either may be empty).
	void List( const std::string &prefix, const std::string &extensionWithDot,
	    std::vector<std::string> &out ) const;

private:
	VpkArchive( const IByteSource &source, std::string dirVpkPath );
	std::string DataArchivePath( std::uint16_t archiveIndex ) const;

	const IByteSource &m_store;
	std::string m_dirVpkPath;
	std::string m_baseName; // the directory path without "_dir.vpk"/".vpk"
	std::uint32_t m_version = 0;
	std::uint64_t m_dataSectionOffset = 0;
	std::vector<VpkEntry> m_entries;
	std::map<std::string, std::size_t> m_index;
};

} // namespace content

#endif // PUBLIC_CONTENT_VPK_ARCHIVE_H
