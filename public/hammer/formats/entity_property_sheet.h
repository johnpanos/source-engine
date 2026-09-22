//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Headless entity property-sheet model for the Hammer editor (RFC 0002,
//			hammer.formats). The entity property editor shows, for the selected
//			entity, one row per key: its FGD-declared type/display/default/choices
//			and the entity's current value, plus any extra keys the entity carries
//			that the FGD does not declare. This module composes the FGD schema
//			(formats.fgd) with the entity's keyvalues to produce exactly that row
//			model -- the data a GTK/MFC property grid renders. Strict, MFC-free,
//			GPU-free: it builds the model; the shell renders it.
//
//=============================================================================//

#ifndef HAMMER_FORMATS_ENTITY_PROPERTY_SHEET_H
#define HAMMER_FORMATS_ENTITY_PROPERTY_SHEET_H

#include "hammer/formats/fgd.h"
#include "hammer/formats/keyvalues.h"

#include <string>
#include <vector>

namespace hammer::formats
{

// One row of the property grid.
struct PropertyRow
{
	std::string key;                // the VMF key
	std::string type;               // FGD type, or "string" for an extra (non-schema) key
	std::string displayName;        // FGD display name, or the key when not in the schema
	std::string value;              // the entity's current value, or the FGD default if unset
	bool isSet = false;             // the entity actually carries this key
	bool inSchema = false;          // the FGD class declares this key
	std::vector<FgdChoice> choices; // for choices/flags types
};

// The full property sheet for one entity.
struct EntityPropertySheet
{
	std::string classname;
	bool classInFgd = false; // the classname resolved to an FGD class
	std::vector<PropertyRow> rows;
};

// Builds the property sheet for 'entity' using the FGD class table. Schema rows
// (from the resolved FGD class, base classes first) come first in declaration
// order -- each with the entity's value or the FGD default -- followed by any
// extra keys the entity has that the schema does not declare (isSet, !inSchema).
// The meta keys "classname", "id", and the "editor" block are not rows.
EntityPropertySheet BuildPropertySheet(
    const KeyValueNode &entity, const std::vector<EntityClass> &fgdClasses );

} // namespace hammer::formats

#endif // HAMMER_FORMATS_ENTITY_PROPERTY_SHEET_H
