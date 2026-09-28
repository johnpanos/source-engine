//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/app/document_io.h.
//
//=============================================================================//

#include "hammer/app/document_io.h"

#include "hammer/app/save_orchestrator.h"

#include <cstdlib>

namespace hammer::app
{

namespace
{

foundation::Unexpected<DocumentIoError> IoError(
    DocumentIoStatus status, std::string message, int line = 0 )
{
	return foundation::MakeUnexpected( DocumentIoError{ status, std::move( message ), line } );
}

// The next document serial: distinct from the current one, so ids from the
// previous document never resolve in the new one.
std::uint32_t NextSerial( const EditSession &session )
{
	const std::uint32_t current = session.Document().Serial();
	return current == 0xffffffffu ? 1 : current + 1;
}

} // namespace

foundation::Expected<std::vector<ports::CodecDiagnostic>, DocumentIoError> OpenDocument(
    EditSession &session, const ports::IMapCodec &codec, const ports::IFileStore &store,
    const std::string &path )
{
	std::string text;
	if ( !store.Read( path, text ) )
	{
		return IoError( DocumentIoStatus::ReadFailed, "cannot read " + path );
	}
	auto decoded = codec.Decode( text, NextSerial( session ) );
	if ( !decoded )
	{
		return IoError( DocumentIoStatus::DecodeFailed, path + ": " + decoded.Error().message,
		    decoded.Error().line );
	}
	ports::DecodedMap map = std::move( decoded ).Value();
	if ( auto replaced = session.Replace( std::move( map.document ) ); !replaced )
	{
		return IoError( DocumentIoStatus::Vetoed, replaced.Error().message );
	}
	return map.warnings;
}

foundation::Expected<void, DocumentIoError> SaveDocumentAs( EditSession &session,
    const ports::IMapCodec &codec, ports::IFileStore &store, const std::string &path,
    bool bumpMapVersion )
{
	scene::MapDocument copy = session.Document();
	if ( bumpMapVersion )
	{
		scene::DocumentSettings &settings = copy.MutableSettings();
		const std::string *current = settings.WorldKey( "mapversion" );
		const long long next =
		    ( current ? std::atoll( current->c_str() ) : settings.version.mapVersion ) + 1;
		settings.SetWorldKey( "mapversion", std::to_string( next ) );
		settings.version.mapVersion = static_cast<int>( next );
	}
	auto encoded = codec.Encode( copy );
	if ( !encoded )
	{
		return IoError( DocumentIoStatus::EncodeFailed, encoded.Error().message );
	}
	if ( SaveDocument( store, path, encoded.Value() ) != SaveStatus::kOk )
	{
		return IoError( DocumentIoStatus::WriteFailed, "cannot write " + path );
	}
	if ( bumpMapVersion )
	{
		// The saved version number is bookkeeping, not an edit: adopt it
		// without a history entry so the document stays unmodified.
		session.SetMapVersion( copy.Settings().version.mapVersion );
	}
	session.MarkSaved();
	return {};
}

} // namespace hammer::app
