//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Reading a map file as a detached fragment (RFC 0002, hammer.app):
//			a prefab or an instance's content, decoded through the injected
//			codec (IMapCodec) from the file-store port, never opened as the
//			session's document. The document-level state of the file (world
//			keys, cordons, cameras) is not part of a fragment; its objects and
//			the visgroups they name are (see FragmentFromDocument).
//
//=============================================================================//

#ifndef HAMMER_APP_FRAGMENT_IO_H
#define HAMMER_APP_FRAGMENT_IO_H

#include "foundation/expected.h"
#include "hammer/app/clipboard.h"
#include "hammer/app/document_io.h"
#include "hammer/ports/file_store.h"
#include "hammer/ports/map_codec.h"

#include <string>

namespace hammer::app
{

// Reads and decodes 'path' and returns everything in it as a fragment.
// Errors: ReadFailed (missing or unreadable), DecodeFailed (the codec's
// error with its line, or a file that holds no map objects).
foundation::Expected<MapFragment, DocumentIoError> LoadFragment(
    const ports::IMapCodec &codec, const ports::IFileStore &store, const std::string &path );

} // namespace hammer::app

#endif // HAMMER_APP_FRAGMENT_IO_H
