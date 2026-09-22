//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A real, production IFileStore provider backed by the local
//			filesystem (RFC 0002, hammer.adapters.platform). The reusable editor
//			libraries (save orchestration, EditorDocument) depend only on the
//			hammer::ports::IFileStore contract; until now the only implementation
//			was the in-memory test fake. This is the first real provider, so a
//			thin UI sibling can actually persist a document to disk while the app
//			core stays filesystem-agnostic.
//
//			Non-strict adapter: it deliberately uses <filesystem>, which the strict
//			libraries never may. It honors the IFileStore atomicity clause by
//			writing through a sibling temp file and renaming into place.
//
//=============================================================================//

#ifndef HAMMER_ADAPTERS_PLATFORM_DISK_FILE_STORE_H
#define HAMMER_ADAPTERS_PLATFORM_DISK_FILE_STORE_H

#include "hammer/ports/file_store.h"

#include <string>

namespace hammer::adapters::platform
{

//-----------------------------------------------------------------------------
// Purpose: IFileStore over std::filesystem. Whole-file primitives; Write is
//			atomic (temp-file + rename) so a failed or interrupted write never
//			corrupts an existing file. Paths are interpreted by the OS.
//-----------------------------------------------------------------------------
class DiskFileStore : public ports::IFileStore
{
public:
	DiskFileStore() = default;

	bool Exists( const std::string &path ) const override;
	bool Read( const std::string &path, std::string &out ) const override;
	bool Write( const std::string &path, const std::string &content ) override;
	bool Rename( const std::string &from, const std::string &to ) override;
	bool Remove( const std::string &path ) override;
};

} // namespace hammer::adapters::platform

#endif // HAMMER_ADAPTERS_PLATFORM_DISK_FILE_STORE_H
