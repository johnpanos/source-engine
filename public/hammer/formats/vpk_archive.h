//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Dependency-free reader for Valve Pack (VPK) archives (RFC 0002,
//			hammer.formats). A .vpk "directory" file (xxx_dir.vpk) holds a tree of
//			extension -> path -> file records; each record's payload is a short
//			"preload" run stored inline in the directory plus a chunk stored either
//			in the directory's own data section or in a sibling data archive
//			(xxx_000.vpk, xxx_001.vpk, ...). This reader parses that tree and
//			resolves a file's bytes on demand.
//
//			It is a strict-core codec: it depends only on the C++ standard library
//			and the hammer::ports::IByteStore ranged-read contract -- no tier2,
//			no IBaseFileSystem, no engine CPackedStore, none of which the strict
//			editor core may use. The legacy vpklib CPackedStore is engine-coupled
//			(tier2 + IBaseFileSystem for chunk reads); this is the clean-core
//			equivalent used by the editor's material catalog.
//
//			Supports VPK versions 1 and 2 (the two that exist in shipped content).
//			Signature and MD5 sections present in v2 are not verified here; this
//			is a read path for an editor, not a trust boundary.
//
//=============================================================================//

#ifndef HAMMER_FORMATS_VPK_ARCHIVE_H
#define HAMMER_FORMATS_VPK_ARCHIVE_H

#include "hammer/ports/asset_source.h"
#include "hammer/ports/byte_store.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace hammer::formats
{

// One resolved directory record. 'preloadBytes' precede the chunk and live in the
// directory file at 'preloadOffset'. The chunk (length 'chunkLength') lives in a
// data archive named by 'archiveIndex', at 'chunkOffset', except when
// 'archiveIndex' is kInlineArchiveIndex, where it lives in the directory file's
// own data section.
struct VpkEntry
{
	std::string path; // canonical: lower-case, forward-slash, extension included
	std::uint32_t crc = 0;
	std::uint16_t archiveIndex = 0;
	std::uint16_t preloadBytes = 0;
	std::uint64_t preloadOffset = 0; // absolute offset in the directory file
	std::uint32_t chunkOffset = 0;
	std::uint32_t chunkLength = 0;

	// Total logical file size the caller sees (preload + chunk).
	std::uint64_t TotalSize() const { return std::uint64_t( preloadBytes ) + chunkLength; }
};

class VpkArchive final : public hammer::ports::IAssetSource
{
public:
	// The sentinel archiveIndex meaning "the chunk is in the _dir.vpk data section".
	static constexpr std::uint16_t kInlineArchiveIndex = 0x7fff;

	// Parses the directory tree of the VPK whose _dir.vpk file is 'dirVpkPath',
	// reading through 'store'. 'dirVpkPath' should be the path of the directory
	// file (typically ending in "_dir.vpk"); its data-archive siblings are derived
	// from it. Returns nullptr and sets 'error' on a malformed/absent archive. The
	// returned archive keeps a reference to 'store', which must outlive it.
	static std::unique_ptr<VpkArchive> Open(
	    const hammer::ports::IByteStore &store, const std::string &dirVpkPath, std::string &error );

	std::uint32_t Version() const { return m_version; }
	const std::vector<VpkEntry> &Entries() const { return m_entries; }
	const VpkEntry *Find( const std::string &path ) const;

	// IAssetSource.
	bool HasAsset( const std::string &path ) const override;
	bool ReadAsset( const std::string &path, std::string &out ) const override;
	void ListAssets(
	    const std::string &prefix,
	    const std::string &extensionWithDot,
	    std::vector<std::string> &out ) const override;

private:
	VpkArchive( const hammer::ports::IByteStore &store, std::string dirVpkPath );

	// Builds "<base>_NNN.vpk" for a data-archive index.
	std::string DataArchivePath( std::uint16_t archiveIndex ) const;

	const hammer::ports::IByteStore &m_store;
	std::string m_dirVpkPath;
	std::string m_baseName; // dir path with the "_dir.vpk"/".vpk" suffix removed
	std::uint32_t m_version = 0;
	std::uint64_t m_dataSectionOffset = 0; // where inline chunk data begins in _dir.vpk
	std::vector<VpkEntry> m_entries;
	std::map<std::string, std::size_t> m_index; // canonical path -> index into m_entries
};

} // namespace hammer::formats

#endif // HAMMER_FORMATS_VPK_ARCHIVE_H
