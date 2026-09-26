//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared conformance suite for platform.record-store.v1
//			(platform::IRecordStore). Every provider that claims the contract
//			runs THIS predicate: the in-memory fake and the file store on
//			Linux and on devices, and the user-defaults store on iOS and tvOS
//			devices. The sensitivity suite feeds it broken providers.
//
//			Contract doc: unittests/platformtest/contracts/platform.record-store.v1.md
//
//=============================================================================//

#ifndef PLATFORMTEST_RECORD_STORE_CONFORMANCE_H
#define PLATFORMTEST_RECORD_STORE_CONFORMANCE_H

#include "platform/contracts/record_store.h"

#include <atomic>
#include <cstddef>
#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace platformtest
{

struct RecordStoreReport
{
	int checks = 0;
	int failures = 0;
	const char *firstFailure = nullptr;
	int firstFailureLine = 0;

	void Record( bool ok, const char *what, int line )
	{
		++checks;
		if ( !ok )
		{
			++failures;
			if ( firstFailure == nullptr )
			{
				firstFailure = what;
				firstFailureLine = line;
			}
		}
	}
};

#define RS_CHECK( report, cond ) ( report ).Record( ( cond ), #cond, __LINE__ )

// How the suite reaches one provider's backing storage.
struct RecordStoreFixture
{
	// A new store instance over the same backing storage (a restart).
	std::function<std::unique_ptr<platform::IRecordStore>()> open;
	// Prepended to every key the suite uses, so a store shared with a product
	// (the device's user defaults) keeps its own records.
	std::string keyPrefix;
	// Removes a record the suite used, when the fixture can; the contract has
	// no delete. May be empty for a fresh backing store.
	std::function<void( const std::string &key )> erase;
};

inline std::vector<std::byte> Bytes( const std::string &text )
{
	std::vector<std::byte> out;
	for ( const char c : text )
		out.push_back( static_cast<std::byte>( c ) );
	return out;
}

// A record every byte of which is `value`, `length` long: a concurrent Load
// that returns anything else saw a torn record.
inline std::vector<std::byte> Uniform( unsigned char value, std::size_t length )
{
	return std::vector<std::byte>( length, static_cast<std::byte>( value ) );
}

inline bool IsError( const foundation::Expected<std::vector<std::byte>, platform::RecordStoreError> &r,
    platform::RecordStoreErrorCode code )
{
	return !r && r.Error().code == code;
}

inline bool IsError( const foundation::Expected<void, platform::RecordStoreError> &r,
    platform::RecordStoreErrorCode code )
{
	return !r && r.Error().code == code;
}

inline bool Holds( platform::IRecordStore &store, const std::string &key,
    const std::vector<std::byte> &expected )
{
	auto loaded = store.Load( key );
	return loaded && loaded.Value() == expected;
}

inline RecordStoreReport RunRecordStoreConformance( const RecordStoreFixture &fixture )
{
	using platform::RecordStoreErrorCode;
	RecordStoreReport r;
	const std::string &p = fixture.keyPrefix;
	const std::string keyA = p + "record.a";
	const std::string keyB = p + "record.b";
	const std::string keyEmpty = p + "record.empty";
	const std::string keyLimit = p + "record.limit";
	const std::string keyShared = p + "record.shared";
	const std::string used[] = { keyA, keyB, keyEmpty, keyLimit, keyShared };
	if ( fixture.erase )
	{
		for ( const std::string &key : used )
			fixture.erase( key );
	}

	std::unique_ptr<platform::IRecordStore> store = fixture.open();
	RS_CHECK( r, store != nullptr );
	if ( !store )
		return r;

	// Clause 1: a positive, constant limit.
	const std::size_t max = store->MaxRecordBytes();
	RS_CHECK( r, max > 0 );
	RS_CHECK( r, store->MaxRecordBytes() == max );

	// Clause 2: invalid keys are refused before touching storage.
	const std::string invalid[] = { "", ".hidden", "a/b", "a\\b", "..", "a b", "a~", "a:b",
		"\xC3\xA9", std::string( platform::kMaxRecordKeyLength + 1, 'k' ) };
	for ( const std::string &key : invalid )
	{
		RS_CHECK( r, !platform::IsValidRecordKey( key ) );
		RS_CHECK( r, IsError( store->Commit( key, Bytes( "x" ) ), RecordStoreErrorCode::kInvalidKey ) );
		RS_CHECK( r, IsError( store->Load( key ), RecordStoreErrorCode::kInvalidKey ) );
	}
	RS_CHECK( r, platform::IsValidRecordKey( std::string( platform::kMaxRecordKeyLength, 'k' ) ) );
	RS_CHECK( r, platform::IsValidRecordKey( "GameState.txt" ) );

	// Clause 3: a key never committed is not found (not a failure).
	RS_CHECK( r, IsError( store->Load( keyA ), RecordStoreErrorCode::kNotFound ) );

	// Clause 4: exact round trip, every byte value, and an empty record.
	std::vector<std::byte> all;
	for ( int i = 0; i < 256; ++i )
		all.push_back( static_cast<std::byte>( i ) );
	RS_CHECK( r, store->Commit( keyA, all ).HasValue() );
	RS_CHECK( r, Holds( *store, keyA, all ) );
	RS_CHECK( r, store->Commit( keyEmpty, {} ).HasValue() );
	RS_CHECK( r, Holds( *store, keyEmpty, {} ) );

	// Clause 5: Commit replaces the whole record.
	RS_CHECK( r, store->Commit( keyA, Bytes( std::string( 300, 'L' ) ) ).HasValue() );
	RS_CHECK( r, store->Commit( keyA, Bytes( "short" ) ).HasValue() );
	RS_CHECK( r, Holds( *store, keyA, Bytes( "short" ) ) );

	// Clause 6: keys are independent.
	RS_CHECK( r, store->Commit( keyB, Bytes( "second" ) ).HasValue() );
	RS_CHECK( r, Holds( *store, keyA, Bytes( "short" ) ) );
	RS_CHECK( r, Holds( *store, keyB, Bytes( "second" ) ) );

	// Clause 7: the limit is exact, and a refused Commit keeps the record.
	const std::vector<std::byte> full = Uniform( 0x5a, max );
	RS_CHECK( r, store->Commit( keyLimit, full ).HasValue() );
	RS_CHECK( r, Holds( *store, keyLimit, full ) );
	RS_CHECK( r, IsError( store->Commit( keyLimit, Uniform( 0x33, max + 1 ) ),
	                 RecordStoreErrorCode::kTooLarge ) );
	RS_CHECK( r, Holds( *store, keyLimit, full ) );

	// Clause 8: a new instance over the same storage sees the last commits.
	store.reset();
	store = fixture.open();
	RS_CHECK( r, store != nullptr );
	if ( !store )
		return r;
	RS_CHECK( r, Holds( *store, keyA, Bytes( "short" ) ) );
	RS_CHECK( r, Holds( *store, keyB, Bytes( "second" ) ) );
	RS_CHECK( r, Holds( *store, keyEmpty, {} ) );
	RS_CHECK( r, Holds( *store, keyLimit, full ) );

	// Clause 9: concurrent Commits and Loads; every Load sees a whole record.
	// Record v is v repeated (64 + v) times.
	{
		const int writers = 2;
		const int commitsPerWriter = 100;
		std::atomic<bool> done{ false };
		std::atomic<int> torn{ 0 };
		std::atomic<int> failed{ 0 };
		std::atomic<int> loads{ 0 };
		auto commitValue = [&]( unsigned char v ) {
			if ( !store->Commit( keyShared, Uniform( v, 64u + v ) ) )
				failed.fetch_add( 1 );
		};
		commitValue( 1 );
		std::vector<std::thread> threads;
		for ( int w = 0; w < writers; ++w )
		{
			threads.emplace_back( [&, w] {
				for ( int i = 0; i < commitsPerWriter; ++i )
					commitValue( static_cast<unsigned char>( 1 + ( ( w * commitsPerWriter + i ) % 180 ) ) );
			} );
		}
		std::vector<std::thread> readers;
		for ( int reader = 0; reader < 2; ++reader )
		{
			readers.emplace_back( [&] {
				// Keep reading until the writers finish, and at least 200 times.
				while ( !done.load() || loads.load() < 200 )
				{
					auto loaded = store->Load( keyShared );
					loads.fetch_add( 1 );
					if ( !loaded )
					{
						failed.fetch_add( 1 );
						continue;
					}
					const std::vector<std::byte> &bytes = loaded.Value();
					const bool whole = !bytes.empty() &&
					    bytes.size() == 64u + static_cast<unsigned char>( bytes[0] ) &&
					    bytes == Uniform( static_cast<unsigned char>( bytes[0] ), bytes.size() );
					if ( !whole )
						torn.fetch_add( 1 );
				}
			} );
		}
		for ( std::thread &t : threads )
			t.join();
		done.store( true );
		for ( std::thread &t : readers )
			t.join();
		RS_CHECK( r, torn.load() == 0 );
		RS_CHECK( r, failed.load() == 0 );
		RS_CHECK( r, loads.load() >= 200 );
	}

	store.reset();
	if ( fixture.erase )
	{
		for ( const std::string &key : used )
			fixture.erase( key );
	}
	return r;
}

// Prints one variant's result and adds it to the suite totals.
inline int ReportRecordStoreVariant( const char *name, const RecordStoreReport &r, int &checks,
    int &failures )
{
	checks += r.checks;
	failures += r.failures;
	if ( r.failures != 0 )
	{
		std::printf( "FAIL %s: %d/%d checks failed; first: %s (line %d)\n", name, r.failures,
		    r.checks, r.firstFailure, r.firstFailureLine );
		return 1;
	}
	std::printf( "ok %s: %d checks passed\n", name, r.checks );
	return 0;
}

} // namespace platformtest

#endif // PLATFORMTEST_RECORD_STORE_CONFORMANCE_H
