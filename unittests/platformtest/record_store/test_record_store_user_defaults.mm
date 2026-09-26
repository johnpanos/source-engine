//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Device conformance suite for the user-defaults record store
//			(platform/apple, the tvOS product's store): the shared
//			platform.record-store.v1 suite against the app's real standard
//			user defaults, plus the provider's own clauses: each record is an
//			NSData under "source.records.<key>", and a value of another type
//			under that name is reported, not returned or silently replaced by
//			a Load.
//
//			Apple devices only (profile apple-uikit-device), through
//			tools/quality/ios_conformance.py. Keys carry a per-run prefix and
//			are removed afterwards.
//
//=============================================================================//

#import <Foundation/Foundation.h>

#include "record_store_conformance.h"
#include "../../../platform/apple/user_defaults_record_store.h"
#include "testing/conformance_result.h"

#include <chrono>
#include <string>

namespace
{

NSString *DefaultsName( const std::string &key )
{
	return [NSString stringWithUTF8String:( "source.records." + key ).c_str()];
}

void Erase( const std::string &key )
{
	[[NSUserDefaults standardUserDefaults] removeObjectForKey:DefaultsName( key )];
}

platformtest::RecordStoreReport RunProviderClauses( const std::string &prefix )
{
	using platform::RecordStoreErrorCode;
	platformtest::RecordStoreReport r;
	platform::UserDefaultsRecordStore store;
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];

	// The record's name and type in the defaults database.
	const std::string key = prefix + "layout";
	RS_CHECK( r, store.Commit( key, platformtest::Bytes( "abc" ) ).HasValue() );
	NSData *data = [defaults dataForKey:DefaultsName( key )];
	RS_CHECK( r, data != nil && data.length == 3 && memcmp( data.bytes, "abc", 3 ) == 0 );

	// Another type under the name: a failure, and a Commit replaces it.
	[defaults setObject:@"not a record" forKey:DefaultsName( key )];
	RS_CHECK( r, platformtest::IsError( store.Load( key ), RecordStoreErrorCode::kIoFailure ) );
	RS_CHECK( r, store.Commit( key, platformtest::Bytes( "again" ) ).HasValue() );
	RS_CHECK( r, platformtest::Holds( store, key, platformtest::Bytes( "again" ) ) );

	// The cap the header documents.
	RS_CHECK( r, store.MaxRecordBytes() == platform::UserDefaultsRecordStore::kMaxRecordBytes );
	Erase( key );
	return r;
}

} // namespace

int main()
{
	int checks = 0;
	int failures = 0;
	@autoreleasepool
	{
		const auto stamp = std::chrono::system_clock::now().time_since_epoch().count();
		const std::string prefix = "conformance." + std::to_string( stamp % 1000000000 ) + ".";

		platformtest::RecordStoreFixture fixture;
		fixture.open = [] { return std::make_unique<platform::UserDefaultsRecordStore>(); };
		fixture.keyPrefix = prefix;
		fixture.erase = []( const std::string &key ) {
			@autoreleasepool
			{
				Erase( key );
			}
		};
		platformtest::ReportRecordStoreVariant( "record_store[user-defaults]",
		    platformtest::RunRecordStoreConformance( fixture ), checks, failures );
		platformtest::ReportRecordStoreVariant( "record_store[user-defaults clauses]",
		    RunProviderClauses( prefix ), checks, failures );
	}
	return testing::ReportConformance( checks, failures );
}
