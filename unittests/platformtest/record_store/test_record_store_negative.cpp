//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity check for the platform.record-store.v1 suite: the SAME
//			shared predicate must reject each deliberately broken provider
//			(one defect each, modeled on real ways a store goes wrong) while
//			the conforming fake passes.
//
//			Build/run: tools/quality/conformance.py check --suite platform.record_store.sensitivity
//
//=============================================================================//

#include "fake_record_store.h"
#include "record_store_conformance.h"
#include "testing/conformance_result.h"

#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <functional>

namespace
{

using platform::RecordStoreError;
using platform::RecordStoreErrorCode;
using platformtest::FakeRecordBacking;
using platformtest::FakeRecordStore;
using platformtest::RecordError;

using LoadResult = foundation::Expected<std::vector<std::byte>, RecordStoreError>;
using CommitResult = foundation::Expected<void, RecordStoreError>;

// Accepts any key: a path separator would reach the file system.
class AcceptsInvalidKeys : public FakeRecordStore
{
public:
	using FakeRecordStore::FakeRecordStore;
	LoadResult Load( std::string_view key ) override
	{
		std::lock_guard<std::mutex> lock( m_backing->mutex );
		auto it = m_backing->records.find( key );
		if ( it == m_backing->records.end() )
			return foundation::MakeUnexpected( RecordError( RecordStoreErrorCode::kNotFound ) );
		return it->second;
	}
	CommitResult Commit( std::string_view key, std::span<const std::byte> bytes ) override
	{
		std::lock_guard<std::mutex> lock( m_backing->mutex );
		m_backing->records[std::string( key )] = std::vector<std::byte>( bytes.begin(), bytes.end() );
		return CommitResult::Ok();
	}
};

// Reports a missing record as a failure, so a new player could never save.
class MissingAsFailure : public FakeRecordStore
{
public:
	using FakeRecordStore::FakeRecordStore;
	LoadResult Load( std::string_view key ) override
	{
		LoadResult r = FakeRecordStore::Load( key );
		if ( !r && r.Error().code == RecordStoreErrorCode::kNotFound )
			return foundation::MakeUnexpected( RecordError( RecordStoreErrorCode::kIoFailure ) );
		return r;
	}
};

// Treats an empty record as no record (a nil/empty NSData confusion).
class EmptyAsMissing : public FakeRecordStore
{
public:
	using FakeRecordStore::FakeRecordStore;
	LoadResult Load( std::string_view key ) override
	{
		LoadResult r = FakeRecordStore::Load( key );
		if ( r && r.Value().empty() )
			return foundation::MakeUnexpected( RecordError( RecordStoreErrorCode::kNotFound ) );
		return r;
	}
};

// Stores the record as a C string: it stops at the first NUL byte.
class StopsAtNul : public FakeRecordStore
{
public:
	using FakeRecordStore::FakeRecordStore;
	CommitResult Commit( std::string_view key, std::span<const std::byte> bytes ) override
	{
		std::size_t length = 0;
		while ( length < bytes.size() && bytes[length] != std::byte{ 0 } )
			++length;
		return FakeRecordStore::Commit( key, bytes.first( length ) );
	}
};

// Appends to the record instead of replacing it.
class Appends : public FakeRecordStore
{
public:
	using FakeRecordStore::FakeRecordStore;
	CommitResult Commit( std::string_view key, std::span<const std::byte> bytes ) override
	{
		if ( !platform::IsValidRecordKey( key ) )
			return foundation::MakeUnexpected( RecordError( RecordStoreErrorCode::kInvalidKey ) );
		std::lock_guard<std::mutex> lock( m_backing->mutex );
		std::vector<std::byte> &record = m_backing->records[std::string( key )];
		record.insert( record.end(), bytes.begin(), bytes.end() );
		return CommitResult::Ok();
	}
};

// Clears the old record before finding the new one too large.
class OversizeClobbers : public FakeRecordStore
{
public:
	using FakeRecordStore::FakeRecordStore;
	CommitResult Commit( std::string_view key, std::span<const std::byte> bytes ) override
	{
		if ( platform::IsValidRecordKey( key ) && bytes.size() > m_max )
		{
			std::lock_guard<std::mutex> lock( m_backing->mutex );
			m_backing->records.erase( std::string( key ) );
		}
		return FakeRecordStore::Commit( key, bytes );
	}
};

// Ignores its own limit.
class IgnoresLimit : public FakeRecordStore
{
public:
	using FakeRecordStore::FakeRecordStore;
	CommitResult Commit( std::string_view key, std::span<const std::byte> bytes ) override
	{
		FakeRecordStore unlimited( m_backing, bytes.size() + m_max );
		return unlimited.Commit( key, bytes );
	}
};

// Keys collide when they share their first seven characters.
class TruncatesKeys : public FakeRecordStore
{
public:
	using FakeRecordStore::FakeRecordStore;
	LoadResult Load( std::string_view key ) override
	{
		return FakeRecordStore::Load( platform::IsValidRecordKey( key ) ? key.substr( 0, 7 ) : key );
	}
	CommitResult Commit( std::string_view key, std::span<const std::byte> bytes ) override
	{
		return FakeRecordStore::Commit( platform::IsValidRecordKey( key ) ? key.substr( 0, 7 ) : key, bytes );
	}
};

// Publishes a record in two halves; a Load between them sees a torn record.
// The second half waits (briefly) for a Load so the tear is observed.
class TornCommits : public FakeRecordStore
{
public:
	using FakeRecordStore::FakeRecordStore;
	LoadResult Load( std::string_view key ) override
	{
		LoadResult r = FakeRecordStore::Load( key );
		{
			std::lock_guard<std::mutex> lock( m_tearMutex );
			++m_loads;
		}
		m_tearLoaded.notify_all();
		return r;
	}
	CommitResult Commit( std::string_view key, std::span<const std::byte> bytes ) override
	{
		if ( !platform::IsValidRecordKey( key ) || bytes.size() > m_max )
			return FakeRecordStore::Commit( key, bytes );
		std::lock_guard<std::mutex> serial( m_commitMutex );
		const std::size_t half = bytes.size() / 2;
		{
			std::lock_guard<std::mutex> lock( m_backing->mutex );
			std::vector<std::byte> &record = m_backing->records[std::string( key )];
			if ( record.size() < bytes.size() )
				record.resize( bytes.size() );
			std::copy( bytes.begin(), bytes.begin() + half, record.begin() );
		}
		{
			std::unique_lock<std::mutex> lock( m_tearMutex );
			const long seen = m_loads;
			m_tearLoaded.wait_for( lock, std::chrono::milliseconds( 20 ), [&] { return m_loads != seen; } );
		}
		return FakeRecordStore::Commit( key, bytes );
	}

private:
	std::mutex m_commitMutex;
	std::mutex m_tearMutex;
	std::condition_variable m_tearLoaded;
	long m_loads = 0;
};

// Keeps records only in the instance: a restart loses them.
class ForgetsOnRestart : public FakeRecordStore
{
public:
	ForgetsOnRestart() : FakeRecordStore( std::make_shared<FakeRecordBacking>() ) {}
};

template <typename Store> platformtest::RecordStoreFixture SharedFixture()
{
	auto backing = std::make_shared<FakeRecordBacking>();
	platformtest::RecordStoreFixture fixture;
	fixture.open = [backing] { return std::make_unique<Store>( backing ); };
	return fixture;
}

int g_checks = 0;
int g_failures = 0;

void Expect( const char *name, const platformtest::RecordStoreFixture &fixture, bool shouldPass )
{
	const platformtest::RecordStoreReport r = platformtest::RunRecordStoreConformance( fixture );
	const bool passed = r.failures == 0;
	++g_checks;
	if ( passed != shouldPass )
	{
		++g_failures;
		std::printf( "FAIL %s: expected the suite to %s it (%d/%d checks failed)\n", name,
		    shouldPass ? "pass" : "reject", r.failures, r.checks );
		return;
	}
	if ( shouldPass )
		std::printf( "ok %s: conforming provider passes (%d checks)\n", name, r.checks );
	else
		std::printf( "ok %s: rejected; first failure: %s (line %d)\n", name, r.firstFailure,
		    r.firstFailureLine );
}

} // namespace

int main()
{
	Expect( "fake", SharedFixture<FakeRecordStore>(), true );
	Expect( "accepts-invalid-keys", SharedFixture<AcceptsInvalidKeys>(), false );
	Expect( "missing-as-failure", SharedFixture<MissingAsFailure>(), false );
	Expect( "empty-as-missing", SharedFixture<EmptyAsMissing>(), false );
	Expect( "stops-at-nul", SharedFixture<StopsAtNul>(), false );
	Expect( "appends", SharedFixture<Appends>(), false );
	Expect( "oversize-clobbers", SharedFixture<OversizeClobbers>(), false );
	Expect( "ignores-limit", SharedFixture<IgnoresLimit>(), false );
	Expect( "truncates-keys", SharedFixture<TruncatesKeys>(), false );
	Expect( "torn-commits", SharedFixture<TornCommits>(), false );
	{
		platformtest::RecordStoreFixture fixture;
		fixture.open = [] { return std::make_unique<ForgetsOnRestart>(); };
		Expect( "forgets-on-restart", fixture, false );
	}
	return testing::ReportConformance( g_checks, g_failures );
}
