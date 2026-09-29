//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer/gtk/entity_schema.h.
//
//=============================================================================//

#include "entity_schema.h"

#include <fstream>
#include <iterator>
#include <optional>

namespace hammer::gtk
{

foundation::Expected<formats::FgdEntityCatalog, formats::FgdCatalogError> LoadEntitySchema(
    const std::string &path )
{
	const std::string::size_type slash = path.rfind( '/' );
	const std::string dir = slash == std::string::npos ? "." : path.substr( 0, slash );
	return formats::FgdEntityCatalog::Load( path,
	    [&]( const std::string &name ) -> std::optional<std::string>
	    {
		    std::ifstream in( name == path ? name : dir + "/" + name, std::ios::binary );
		    if ( !in )
		    {
			    return std::nullopt;
		    }
		    return std::string(
		        ( std::istreambuf_iterator<char>( in ) ), std::istreambuf_iterator<char>() );
	    } );
}

} // namespace hammer::gtk
