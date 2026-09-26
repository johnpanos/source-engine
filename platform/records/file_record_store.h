//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: File provider of platform.record-store.v1 (platform.records): each
//			record is the file <directory>/<key>, so a product keeps its
//			existing files (the achievement manager's GameState.txt).
//
//			Commit writes <key>~ ('~' is never in a key) and renames it over
//			the record, so where rename replaces its target (POSIX), a crash
//			leaves the old or the new record whole. Where rename refuses an
//			existing target (Windows), Commit removes the record first; a crash
//			between the two steps loses the record. Commit does not flush the
//			file to the device, so power loss may drop the last commit.
//
//			The directory must exist; Commit into a missing directory fails
//			with kIoFailure, and Load from one reports kNotFound.
//
//=============================================================================//

#ifndef PLATFORM_RECORDS_FILE_RECORD_STORE_H
#define PLATFORM_RECORDS_FILE_RECORD_STORE_H

#include "platform/contracts/record_store.h"

#include <cstddef>
#include <mutex>
#include <string>

namespace platform
{

class FileRecordStore final : public IRecordStore
{
public:
	static constexpr std::size_t kDefaultMaxRecordBytes = 1024 * 1024;

	// `directory` may end with a separator. `maxRecordBytes` must be positive.
	explicit FileRecordStore(
	    std::string directory, std::size_t maxRecordBytes = kDefaultMaxRecordBytes );

	std::size_t MaxRecordBytes() const noexcept override;
	foundation::Expected<std::vector<std::byte>, RecordStoreError> Load(
	    std::string_view key ) override;
	foundation::Expected<void, RecordStoreError> Commit(
	    std::string_view key, std::span<const std::byte> bytes ) override;

	// The file that holds the record under a valid key.
	std::string PathFor( std::string_view key ) const;

private:
	const std::string m_directory;
	const std::size_t m_maxRecordBytes;
	// Serializes Commits (they share the <key>~ file) and keeps Loads from
	// observing the remove-then-rename fallback half done.
	std::mutex m_mutex;
};

} // namespace platform

#endif // PLATFORM_RECORDS_FILE_RECORD_STORE_H
