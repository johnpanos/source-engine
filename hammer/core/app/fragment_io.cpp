//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/app/fragment_io.h.
//
//=============================================================================//

#include "hammer/app/fragment_io.h"

#include "hammer/app/ops/prefab_ops.h"

namespace hammer::app
{

foundation::Expected<MapFragment, DocumentIoError> LoadFragment(
    const ports::IMapCodec &codec, const ports::IFileStore &store, const std::string &path )
{
	std::string text;
	if ( !store.Read( path, text ) )
	{
		return foundation::MakeUnexpected(
		    DocumentIoError{ DocumentIoStatus::ReadFailed, "cannot read " + path, 0 } );
	}
	// Fragment ids are fragment-local keys, so any serial will do.
	auto decoded = codec.Decode( text, 1 );
	if ( !decoded )
	{
		return foundation::MakeUnexpected( DocumentIoError{ DocumentIoStatus::DecodeFailed,
		    path + ": " + decoded.Error().message, decoded.Error().line } );
	}
	std::optional<MapFragment> fragment = ops::FragmentFromDocument( decoded.Value().document );
	if ( !fragment )
	{
		return foundation::MakeUnexpected( DocumentIoError{
		    DocumentIoStatus::DecodeFailed, path + ": the file holds no map objects", 0 } );
	}
	return std::move( *fragment );
}

} // namespace hammer::app
