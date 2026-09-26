//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Deterministic in-memory provider of platform.record-store.v1, the
//			positive subject of the shared suite. Instances opened over the
//			same FakeRecordBacking share its records (a restart).
//
//=============================================================================//

#ifndef PLATFORMTEST_FAKE_RECORD_STORE_H
#define PLATFORMTEST_FAKE_RECORD_STORE_H

#include "platform/contracts/record_store.h"

#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace platformtest
{

struct FakeRecordBacking
{
	std::mutex mutex;
	std::map<std::string, std::vector<std::byte>, std::less<>> records;
};

inline platform::RecordStoreError RecordError( platform::RecordStoreErrorCode code )
{
	platform::RecordStoreError error;
	error.code = code;
	return error;
}

class FakeRecordStore : public platform::IRecordStore
{
public:
	explicit FakeRecordStore( std::shared_ptr<FakeRecordBacking> backing, std::size_t max = 4096 )
	    : m_backing( std::move( backing ) ), m_max( max )
	{
	}

	std::size_t MaxRecordBytes() const noexcept override { return m_max; }

	foundation::Expected<std::vector<std::byte>, platform::RecordStoreError> Load(
	    std::string_view key ) override
	{
		if ( !platform::IsValidRecordKey( key ) )
			return foundation::MakeUnexpected( RecordError( platform::RecordStoreErrorCode::kInvalidKey ) );
		std::lock_guard<std::mutex> lock( m_backing->mutex );
		auto it = m_backing->records.find( key );
		if ( it == m_backing->records.end() )
			return foundation::MakeUnexpected( RecordError( platform::RecordStoreErrorCode::kNotFound ) );
		return it->second;
	}

	foundation::Expected<void, platform::RecordStoreError> Commit(
	    std::string_view key, std::span<const std::byte> bytes ) override
	{
		if ( !platform::IsValidRecordKey( key ) )
			return foundation::MakeUnexpected( RecordError( platform::RecordStoreErrorCode::kInvalidKey ) );
		if ( bytes.size() > m_max )
			return foundation::MakeUnexpected( RecordError( platform::RecordStoreErrorCode::kTooLarge ) );
		std::lock_guard<std::mutex> lock( m_backing->mutex );
		m_backing->records[std::string( key )] = std::vector<std::byte>( bytes.begin(), bytes.end() );
		return foundation::Expected<void, platform::RecordStoreError>::Ok();
	}

protected:
	std::shared_ptr<FakeRecordBacking> m_backing;
	std::size_t m_max;
};

} // namespace platformtest

#endif // PLATFORMTEST_FAKE_RECORD_STORE_H
