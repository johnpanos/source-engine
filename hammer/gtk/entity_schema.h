//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The GTK Hammer shell's entity schema (RFC 0002, linux-gtk-desktop):
//			the FGD the composition root gives the workspace (--fgd), read from
//			disk with its @include files resolved beside it, as hammer_cli
//			does. Without one the editor accepts any class and the Object
//			Properties window shows raw text rows.
//
//=============================================================================//

#ifndef HAMMER_GTK_ENTITY_SCHEMA_H
#define HAMMER_GTK_ENTITY_SCHEMA_H

#include "foundation/expected.h"
#include "hammer/formats/fgd_entity_catalog.h"

#include <string>

namespace hammer::gtk
{

foundation::Expected<formats::FgdEntityCatalog, formats::FgdCatalogError> LoadEntitySchema(
    const std::string &path );

} // namespace hammer::gtk

#endif // HAMMER_GTK_ENTITY_SCHEMA_H
