//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Opening and saving a session's document through the persistence
//			ports (RFC 0002, hammer.app; "Persistence contracts"): the codec is
//			injected (IMapCodec), files go through IFileStore, and saving uses
//			the transactional save policy (temporary file, atomic replace).
//
//			Open builds a detached document and replaces the session's only on
//			success. Save encodes, writes atomically, and only then advances the
//			saved position; a failed save leaves the session modified and the
//			previous file in place (where the store promises atomic replace).
//			Legacy Hammer bumps the world "mapversion" on every save; so does
//			this, as part of the saved content, without recording history.
//
//=============================================================================//

#ifndef HAMMER_APP_DOCUMENT_IO_H
#define HAMMER_APP_DOCUMENT_IO_H

#include "foundation/expected.h"
#include "hammer/app/edit_session.h"
#include "hammer/ports/file_store.h"
#include "hammer/ports/map_codec.h"

#include <string>
#include <vector>

namespace hammer::app
{

enum class DocumentIoStatus
{
	ReadFailed,
	DecodeFailed,
	EncodeFailed,
	WriteFailed,
	Vetoed, // a selection guard kept the current document
};

struct DocumentIoError
{
	DocumentIoStatus status = DocumentIoStatus::ReadFailed;
	std::string message;
	int line = 0; // decode errors: the source line
};

// Reads and decodes 'path', then replaces the session's document. Returns the
// codec's warnings.
foundation::Expected<std::vector<ports::CodecDiagnostic>, DocumentIoError> OpenDocument(
    EditSession &session, const ports::IMapCodec &codec, const ports::IFileStore &store,
    const std::string &path );

// Encodes and atomically writes the session's document to 'path' and marks the
// session saved. With 'bumpMapVersion', the saved file's world "mapversion" is
// one more than the document's, and the document adopts it.
foundation::Expected<void, DocumentIoError> SaveDocumentAs( EditSession &session,
    const ports::IMapCodec &codec, ports::IFileStore &store, const std::string &path,
    bool bumpMapVersion = true );

} // namespace hammer::app

#endif // HAMMER_APP_DOCUMENT_IO_H
