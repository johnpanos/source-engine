//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity suite for formats.entity_property_sheet.v1. Pins the
//			properties a naive implementation gets wrong: FGD schema rows appear
//			even when the entity sets none of them (a "show only set keys" view
//			fails), each unset row carries the FGD default, and the meta keys
//			"classname"/"id" are never rows (a "show all pairs" view fails). Exit 0
//			when all hold.
//
//=============================================================================//

#include "hammer/formats/entity_property_sheet.h"
#include "hammer/formats/fgd.h"
#include "hammer/formats/keyvalues.h"

#include <cstdio>
#include <string>

using hammer::formats::BuildPropertySheet;
using hammer::formats::EntityPropertySheet;
using hammer::formats::ParseFgd;
using hammer::formats::ParseKeyValues;
using hammer::formats::PropertyRow;

namespace
{
int g_failures = 0;

void Check( bool ok, const char *label )
{
	if ( !ok )
	{
		std::printf( "FAIL: %s\n", label );
		++g_failures;
	}
}

const PropertyRow *Row( const EntityPropertySheet &s, const char *key )
{
	for ( const PropertyRow &r : s.rows )
	{
		if ( r.key == key )
		{
			return &r;
		}
	}
	return nullptr;
}

const char *kFgd = "@PointClass = prop_x : \"X.\"\n"
                   "[\n"
                   "\tmodel(studio) : \"World Model\" : \"models/x.mdl\"\n"
                   "\tskin(integer) : \"Skin\" : \"0\"\n"
                   "]\n";

} // namespace

int main()
{
	auto fgd = ParseFgd( kFgd );
	Check( fgd.ok, "fgd parses" );

	// An entity that sets NONE of the schema keys, only meta keys.
	auto pr = ParseKeyValues( "entity\n{\n\t\"id\" \"3\"\n\t\"classname\" \"prop_x\"\n}\n" );
	EntityPropertySheet sheet = BuildPropertySheet( pr.root.children.front(), fgd.classes );

	// Schema rows appear despite nothing being set (a set-only view fails).
	const PropertyRow *model = Row( sheet, "model" );
	const PropertyRow *skin = Row( sheet, "skin" );
	Check( model != nullptr && !model->isSet && model->value == "models/x.mdl",
	    "unset schema row 'model' present with its default" );
	Check( skin != nullptr && !skin->isSet && skin->value == "0",
	    "unset schema row 'skin' present with its default" );

	// Meta keys must never be rows (a show-all-pairs view fails).
	Check( Row( sheet, "id" ) == nullptr, "'id' is not a property row" );
	Check( Row( sheet, "classname" ) == nullptr, "'classname' is not a property row" );

	// Exactly the two schema rows, no extras.
	Check( sheet.rows.size() == 2, "exactly the two schema rows (no meta, no extras)" );

	if ( g_failures != 0 )
	{
		std::printf( "formats.entity_property_sheet negative: ORACLE UNSOUND (%d)\n", g_failures );
		return 1;
	}
	std::printf( "formats.entity_property_sheet negative: unset schema rows show defaults; meta "
	             "keys excluded; no spurious rows\n" );
	return 0;
}
