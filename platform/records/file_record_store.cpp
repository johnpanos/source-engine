//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: File provider of platform.record-store.v1; see
//			file_record_store.h.
//
//=============================================================================//

#include "platform/records/file_record_store.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <utility>

namespace platform
{

namespace
{

using foundation::MakeUnexpected;

RecordStoreError Error( RecordStoreErrorCode code, int providerCode = 0 )
{
	RecordStoreError error;
	error.code = code;
	error.providerCode = providerCode;
	return error;
}

// Owns a stdio stream; fclose's result is checked where it matters.
struct FileCloser
{
	void operator()( std::FILE *file ) const noexcept { std::fclose( file ); }
};

} // namespace

FileRecordStore::FileRecordStore( std::string directory, std::size_t maxRecordBytes )
    : m_directory( std::move( directory ) ), m_maxRecordBytes( maxRecordBytes )
{
	if ( m_maxRecordBytes == 0 )
		std::abort();
}

std::size_t FileRecordStore::MaxRecordBytes() const noexcept
{
	return m_maxRecordBytes;
}

std::string FileRecordStore::PathFor( std::string_view key ) const
{
	std::string path = m_directory;
	if ( !path.empty() && path.back() != '/' && path.back() != '\\' )
		path += '/';
	path += key;
	return path;
}

foundation::Expected<std::vector<std::byte>, RecordStoreError> FileRecordStore::Load(
    std::string_view key )
{
	if ( !IsValidRecordKey( key ) )
		return MakeUnexpected( Error( RecordStoreErrorCode::kInvalidKey ) );

	const std::string path = PathFor( key );
	std::lock_guard<std::mutex> lock( m_mutex );
	errno = 0;
	std::unique_ptr<std::FILE, FileCloser> file( std::fopen( path.c_str(), "rb" ) );
	if ( !file )
	{
		const int error = errno;
		if ( error == ENOENT || error == ENOTDIR )
			return MakeUnexpected( Error( RecordStoreErrorCode::kNotFound, error ) );
		return MakeUnexpected( Error( RecordStoreErrorCode::kIoFailure, error ) );
	}

	// Read one byte past the limit to tell a full-size record from a larger one.
	std::vector<std::byte> bytes;
	std::byte chunk[4096];
	for ( ;; )
	{
		const std::size_t read = std::fread( chunk, 1, sizeof( chunk ), file.get() );
		bytes.insert( bytes.end(), chunk, chunk + read );
		if ( bytes.size() > m_maxRecordBytes )
			return MakeUnexpected( Error( RecordStoreErrorCode::kTooLarge ) );
		if ( read < sizeof( chunk ) )
			break;
	}
	if ( std::ferror( file.get() ) )
		return MakeUnexpected( Error( RecordStoreErrorCode::kIoFailure, errno ) );
	return bytes;
}

foundation::Expected<void, RecordStoreError> FileRecordStore::Commit(
    std::string_view key, std::span<const std::byte> bytes )
{
	if ( !IsValidRecordKey( key ) )
		return MakeUnexpected( Error( RecordStoreErrorCode::kInvalidKey ) );
	if ( bytes.size() > m_maxRecordBytes )
		return MakeUnexpected( Error( RecordStoreErrorCode::kTooLarge ) );

	const std::string path = PathFor( key );
	const std::string staged = path + "~";
	std::lock_guard<std::mutex> lock( m_mutex );

	errno = 0;
	std::FILE *file = std::fopen( staged.c_str(), "wb" );
	if ( !file )
		return MakeUnexpected( Error( RecordStoreErrorCode::kIoFailure, errno ) );
	const bool written = std::fwrite( bytes.data(), 1, bytes.size(), file ) == bytes.size() &&
	                     std::fflush( file ) == 0;
	const int writeError = errno;
	const bool closed = std::fclose( file ) == 0;
	if ( !written || !closed )
	{
		std::remove( staged.c_str() );
		return MakeUnexpected( Error( RecordStoreErrorCode::kIoFailure, writeError ) );
	}

	errno = 0;
	if ( std::rename( staged.c_str(), path.c_str() ) != 0 )
	{
		int renameError = errno;
		// The Windows CRT refuses an existing target with EACCES or EEXIST; see
		// the header. On POSIX these mean the directory is not writable or the
		// target is a non-empty directory, and then remove fails too, so the
		// record stays.
		const bool targetInTheWay = renameError == EACCES || renameError == EEXIST;
		if ( targetInTheWay && std::remove( path.c_str() ) == 0 )
		{
			errno = 0;
			if ( std::rename( staged.c_str(), path.c_str() ) == 0 )
				return foundation::Expected<void, RecordStoreError>::Ok();
			renameError = errno;
		}
		std::remove( staged.c_str() );
		return MakeUnexpected( Error( RecordStoreErrorCode::kIoFailure, renameError ) );
	}
	return foundation::Expected<void, RecordStoreError>::Ok();
}

} // namespace platform
