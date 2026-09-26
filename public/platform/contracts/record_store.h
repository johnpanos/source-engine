//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Capability contract for small durable player records
//			(platform.record-store.v1): named byte records that a product keeps
//			across runs, such as the achievement state in gamestate.txt.
//
//			A record is an opaque byte string; its format belongs to the
//			caller. The store owns only where and how the bytes are kept: a
//			file per record on desktop and iOS, the user-defaults database on
//			tvOS, where apps have no other persistent local storage. The
//			application root selects the provider.
//
//			Contract doc: unittests/platformtest/contracts/platform.record-store.v1.md
//
//=============================================================================//

#ifndef PLATFORM_CONTRACTS_RECORD_STORE_H
#define PLATFORM_CONTRACTS_RECORD_STORE_H

// Contract header: standard library and foundation only. No tier0/tier1, no
// native SDK, no OS-selection macros.
#include "foundation/expected.h"

#include <cstddef>
#include <span>
#include <string_view>
#include <vector>

namespace platform
{

enum class RecordStoreErrorCode
{
	kInvalidKey = 1,
	// The record is larger than MaxRecordBytes(): a Commit that was refused,
	// or a stored record a Load will not return.
	kTooLarge,
	// No record was ever committed under the key. Distinct from kIoFailure: a
	// caller may start from empty state on kNotFound, but must not overwrite a
	// record it failed to read.
	kNotFound,
	// The backing store failed. The record under the key is unchanged.
	kIoFailure,
};

struct RecordStoreError
{
	RecordStoreErrorCode code = RecordStoreErrorCode::kIoFailure;
	// A provider-specific detail (errno for the file store); 0 when none.
	int providerCode = 0;
};

// The longest key a store accepts.
inline constexpr std::size_t kMaxRecordKeyLength = 64;

// Keys are 1 to kMaxRecordKeyLength ASCII letters, digits, '_', '-' and '.',
// not starting with '.'. Keys are compared exactly; callers must not use two
// keys that differ only in case, since a file store on a case-insensitive
// file system would alias them. This is the one definition of a valid key.
constexpr bool IsValidRecordKey( std::string_view key )
{
	if ( key.empty() || key.size() > kMaxRecordKeyLength || key.front() == '.' )
		return false;
	for ( const char c : key )
	{
		const bool ok = ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) ||
		                ( c >= '0' && c <= '9' ) || c == '_' || c == '-' || c == '.';
		if ( !ok )
			return false;
	}
	return true;
}

// A store of named byte records. Every method may be called from any thread,
// including concurrently. Per key, Commit is atomic: a Load, in this process or
// after a restart, returns the whole of some committed record, never a mix,
// and a failed Commit leaves the previous record in place. Once Commit
// returns, Loads in this process return the new record; when it reaches the
// device's durable storage is the provider's documented guarantee.
class IRecordStore
{
public:
	virtual ~IRecordStore() = default;

	// The largest record this store accepts, in bytes. Positive and constant
	// for the store's lifetime.
	virtual std::size_t MaxRecordBytes() const noexcept = 0;

	// The last record committed under `key`.
	virtual foundation::Expected<std::vector<std::byte>, RecordStoreError> Load(
	    std::string_view key ) = 0;

	// Replaces the whole record under `key` with `bytes` (which may be empty).
	virtual foundation::Expected<void, RecordStoreError> Commit(
	    std::string_view key, std::span<const std::byte> bytes ) = 0;
};

} // namespace platform

#endif // PLATFORM_CONTRACTS_RECORD_STORE_H
