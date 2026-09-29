//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The viewports' func_instance content (RFC 0002, hammer.app): the
//			ports::IInstanceContent provider over the injected codec and file
//			store. It reads each instance's file read-only (LoadFragment),
//			places it with the collapse rule (ops::PlaceInstanceContent: the
//			instance's parameters, name fixup and transform) and replaces
//			nested func_instances by their own placed content, so what a
//			viewport draws is what CollapseInstance would merge. Nothing is
//			ever written to the document.
//
//			Lookup (vbsp CMapFile::DeterminePath): an instance's "file"
//			(ops::InstanceFile: '/' separators, ".vmf" added) is looked up
//			  1. in the directory of the file that holds the instance (the
//			     document's path; for a nested instance, its parent's file);
//			  2. under the enclosing "maps/" directory of that file (the path
//			     up to its last "/maps/", case-insensitive);
//			  3. under each search root, in order (the game's instance path).
//			A document without a path uses the roots only.
//
//			Caching. Each file is read and decoded once; each placement is
//			computed once per distinct instance (the file that holds it and
//			the entity's class and keys). Refresh() re-reads every file read
//			so far and repeats every lookup made so far; when any bytes or any
//			lookup's answer changed it forgets everything and bumps
//			Revision(). Changing the document path or the roots does the same.
//
//			Content: see ports/instance_content.h. Quick-hidden objects of an
//			instance file are left out. Nested content is merged with fresh
//			content-local ids (owner and group references follow). A cycle
//			(a file that includes itself) or nesting deeper than maxDepth is
//			Cycle; a nested failure is counted, not fatal.
//
//			Threading: one sequence (the session's). Not copyable.
//
//=============================================================================//

#ifndef HAMMER_APP_INSTANCE_PREVIEW_H
#define HAMMER_APP_INSTANCE_PREVIEW_H

#include "hammer/app/clipboard.h"
#include "hammer/ports/entity_catalog.h"
#include "hammer/ports/file_store.h"
#include "hammer/ports/instance_content.h"
#include "hammer/ports/map_codec.h"

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace hammer::app
{

class InstancePreview final : public ports::IInstanceContent
{
public:
	InstancePreview( const ports::IMapCodec &codec, const ports::IFileStore &store,
	    const ports::IEntityCatalog *catalog = nullptr, int maxDepth = 16 );
	InstancePreview( const InstancePreview & ) = delete;
	InstancePreview &operator=( const InstancePreview & ) = delete;

	// The file the document's instances live in (for lookup rules 1 and 2).
	void SetDocumentPath( const std::string &path );
	void SetSearchRoots( std::vector<std::string> roots );

	std::shared_ptr<const ports::InstanceContent> Content( const scene::Entity &instance ) override;
	std::uint64_t Revision() const override { return m_revision; }

	// See "Caching". Returns whether anything changed.
	bool Refresh();

	// Work counters (tests and budgets): files decoded, placements computed.
	std::size_t FilesDecoded() const { return m_filesDecoded; }
	std::size_t Placements() const { return m_placements; }

private:
	struct FileEntry
	{
		bool read = false;
		std::string bytes;
		std::optional<MapFragment> fragment; // nothing when it did not decode
		std::string error;
	};
	struct Lookup
	{
		std::string base;
		std::string file;
		std::string resolved; // empty = not found
	};

	std::string Resolve( const std::string &base, const std::string &file );
	const FileEntry &File( const std::string &path );
	ports::InstanceContent Expand(
	    const scene::Entity &instance, const std::string &base, std::vector<std::string> &stack );
	void Forget();

	const ports::IMapCodec &m_codec;
	const ports::IFileStore &m_store;
	const ports::IEntityCatalog *m_catalog;
	int m_maxDepth;
	std::string m_documentPath;
	std::vector<std::string> m_roots;
	std::map<std::string, FileEntry> m_files;
	std::map<std::string, Lookup> m_lookups; // base + '\n' + file
	std::map<std::string, std::shared_ptr<const ports::InstanceContent>> m_placed;
	std::uint64_t m_revision = 1;
	std::size_t m_filesDecoded = 0;
	std::size_t m_placements = 0;
};

} // namespace hammer::app

#endif // HAMMER_APP_INSTANCE_PREVIEW_H
