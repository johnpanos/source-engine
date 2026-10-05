//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: User-defaults provider of platform.record-store.v1
//			(platform.apple): each record is an NSData value under
//			"source.records.<key>" in the app's standard user defaults.
//
//			This is the tvOS record store: tvOS gives apps no persistent local
//			storage outside the bundle except the user-defaults database, and
//			may purge Library/Caches, where the game content and its files
//			live (launcher_main/apple_main.cpp). Apple bounds that database on
//			tvOS, so records are capped at kMaxRecordBytes, well above the
//			achievement state it holds (a few KB).
//
//			NSUserDefaults is thread-safe and replaces a value whole. It writes
//			to disk on its own schedule and when the app leaves the
//			foreground, so a crash may lose the last commit.
//
//=============================================================================//

#ifndef PLATFORM_APPLE_USER_DEFAULTS_RECORD_STORE_H
#define PLATFORM_APPLE_USER_DEFAULTS_RECORD_STORE_H

#include "platform/contracts/record_store.h"

namespace platform
{

class UserDefaultsRecordStore final : public IRecordStore
{
public:
	static constexpr std::size_t kMaxRecordBytes = 64 * 1024;

	std::size_t MaxRecordBytes() const noexcept override;
	foundation::Expected<std::vector<std::byte>, RecordStoreError> Load(
	    std::string_view key ) override;
	foundation::Expected<void, RecordStoreError> Commit(
	    std::string_view key, std::span<const std::byte> bytes ) override;
};

} // namespace platform

#endif // PLATFORM_APPLE_USER_DEFAULTS_RECORD_STORE_H
