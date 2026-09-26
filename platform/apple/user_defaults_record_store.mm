//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: User-defaults provider of platform.record-store.v1; see
//			user_defaults_record_store.h.
//
//=============================================================================//

#include "user_defaults_record_store.h"

#import <Foundation/Foundation.h>

#include <string>

namespace platform
{

namespace
{

using foundation::MakeUnexpected;

RecordStoreError Error( RecordStoreErrorCode code )
{
	RecordStoreError error;
	error.code = code;
	return error;
}

// Keys are ASCII (IsValidRecordKey), so this conversion cannot fail.
NSString *DefaultsKey( std::string_view key )
{
	std::string name = "source.records.";
	name.append( key );
	return [NSString stringWithUTF8String:name.c_str()];
}

} // namespace

std::size_t UserDefaultsRecordStore::MaxRecordBytes() const noexcept
{
	return kMaxRecordBytes;
}

foundation::Expected<std::vector<std::byte>, RecordStoreError> UserDefaultsRecordStore::Load(
    std::string_view key )
{
	if ( !IsValidRecordKey( key ) )
		return MakeUnexpected( Error( RecordStoreErrorCode::kInvalidKey ) );

	// Callers include worker threads with no autorelease pool of their own.
	@autoreleasepool
	{
		id value = [[NSUserDefaults standardUserDefaults] objectForKey:DefaultsKey( key )];
		if ( value == nil )
			return MakeUnexpected( Error( RecordStoreErrorCode::kNotFound ) );
		// Something other than a record under our prefix; don't overwrite it
		// blindly.
		if ( ![value isKindOfClass:[NSData class]] )
			return MakeUnexpected( Error( RecordStoreErrorCode::kIoFailure ) );
		NSData *data = value;
		if ( data.length > kMaxRecordBytes )
			return MakeUnexpected( Error( RecordStoreErrorCode::kTooLarge ) );
		const std::byte *first = static_cast<const std::byte *>( data.bytes );
		return std::vector<std::byte>( first, first + data.length );
	}
}

foundation::Expected<void, RecordStoreError> UserDefaultsRecordStore::Commit(
    std::string_view key, std::span<const std::byte> bytes )
{
	if ( !IsValidRecordKey( key ) )
		return MakeUnexpected( Error( RecordStoreErrorCode::kInvalidKey ) );
	if ( bytes.size() > kMaxRecordBytes )
		return MakeUnexpected( Error( RecordStoreErrorCode::kTooLarge ) );

	@autoreleasepool
	{
		NSData *data = [NSData dataWithBytes:bytes.data() length:bytes.size()];
		[[NSUserDefaults standardUserDefaults] setObject:data forKey:DefaultsKey( key )];
	}
	return foundation::Expected<void, RecordStoreError>::Ok();
}

} // namespace platform
