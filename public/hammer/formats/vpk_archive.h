//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Hammer's VPK asset source (RFC 0002, hammer.formats): the
//			content.vpk reader (public/content/vpk_archive.h) behind Hammer's
//			IByteStore and IAssetSource ports. Format notes: A .vpk "directory" file (xxx_dir.vpk) holds a tree of
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

#include "content/vpk_archive.h"
#include "hammer/ports/asset_source.h"
#include "hammer/ports/byte_store.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace hammer::formats
{

// The record type is content.vpk's; Hammer adapts its byte store.
using VpkEntry = content::VpkEntry;

class VpkArchive final : public hammer::ports::IAssetSource
{
public:
	// The sentinel archiveIndex meaning "the chunk is in the _dir.vpk data section".
	static constexpr std::uint16_t kInlineArchiveIndex = content::VpkArchive::kInlineArchiveIndex;

	// Parses the directory tree of the VPK whose _dir.vpk file is 'dirVpkPath',
	// reading through 'store'. 'dirVpkPath' should be the path of the directory
	// file (typically ending in "_dir.vpk"); its data-archive siblings are derived
	// from it. Returns nullptr and sets 'error' on a malformed/absent archive. The
	// returned archive keeps a reference to 'store', which must outlive it.
	static std::unique_ptr<VpkArchive> Open(
	    const hammer::ports::IByteStore &store, const std::string &dirVpkPath, std::string &error );

	std::uint32_t Version() const { return m_archive->Version(); }
	const std::vector<VpkEntry> &Entries() const { return m_archive->Entries(); }
	const VpkEntry *Find( const std::string &path ) const { return m_archive->Find( path ); }

	// IAssetSource.
	bool HasAsset( const std::string &path ) const override;
	bool ReadAsset( const std::string &path, std::string &out ) const override;
	void ListAssets( const std::string &prefix, const std::string &extensionWithDot,
	    std::vector<std::string> &out ) const override;

private:
	// content.vpk reads through IByteSource; this forwards to Hammer's store.
	class StoreSource final : public content::IByteSource
	{
	public:
		explicit StoreSource( const hammer::ports::IByteStore &store ) : m_store( store ) {}
		bool Size( const std::string &path, std::uint64_t &outSize ) const override
		{
			return m_store.Size( path, outSize );
		}
		bool ReadRange( const std::string &path, std::uint64_t offset, std::size_t length,
		    std::string &out ) const override
		{
			return m_store.ReadRange( path, offset, length, out );
		}

	private:
		const hammer::ports::IByteStore &m_store;
	};

	explicit VpkArchive( const hammer::ports::IByteStore &store ) : m_source( store ) {}

	StoreSource m_source;
	std::unique_ptr<content::VpkArchive> m_archive;
};

} // namespace hammer::formats

#endif // HAMMER_FORMATS_VPK_ARCHIVE_H
