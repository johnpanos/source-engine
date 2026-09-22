//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance oracle for the entity property-sheet model
//			(formats.entity_property_sheet.v1). Builds the property grid for an
//			entity against an FGD and checks schema rows (with inherited keys,
//			defaults for unset keys, and choices), extra non-schema keys, and that
//			meta keys are excluded. Build/run via the conformance manifest. Exit 0
//			on success.
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

const char *kFgd = "@BaseClass = Targetname\n[\n\ttargetname(target_source) : \"Name\"\n]\n"
                   "@PointClass base(Targetname) = light : \"Light.\"\n"
                   "[\n"
                   "\t_light(color255) : \"Brightness\" : \"255 255 255 200\"\n"
                   "\tstyle(choices) : \"Appearance\" : \"0\" =\n\t[\n\t\t0 : \"Normal\"\n\t\t1 : "
                   "\"Flicker\"\n\t]\n"
                   "]\n";

// A light entity: targetname set, _light unset (should show default), a custom
// non-schema key, plus meta keys id/classname and an editor block.
const char *kEntity = "entity\n{\n"
                      "\t\"id\" \"7\"\n"
                      "\t\"classname\" \"light\"\n"
                      "\t\"targetname\" \"lamp1\"\n"
                      "\t\"customkey\" \"42\"\n"
                      "\teditor\n\t{\n\t\t\"color\" \"220 30 30\"\n\t}\n"
                      "}\n";

void TestSheet()
{
	auto fgd = ParseFgd( kFgd );
	Check( fgd.ok, "fgd parses" );
	auto pr = ParseKeyValues( kEntity );
	Check( pr.ok && !pr.root.children.empty(), "entity parses" );

	EntityPropertySheet sheet = BuildPropertySheet( pr.root.children.front(), fgd.classes );
	Check( sheet.classname == "light", "classname captured" );
	Check( sheet.classInFgd, "class resolved in FGD" );

	const PropertyRow *tn = Row( sheet, "targetname" );
	Check( tn != nullptr && tn->inSchema && tn->isSet && tn->value == "lamp1",
	    "inherited targetname: in-schema, set, value from entity" );

	const PropertyRow *light = Row( sheet, "_light" );
	Check(
	    light != nullptr && light->inSchema && !light->isSet && light->value == "255 255 255 200",
	    "_light: in-schema, UNSET, shows FGD default (a set-only view would drop it)" );

	const PropertyRow *style = Row( sheet, "style" );
	Check( style != nullptr && style->type == "choices" && style->choices.size() == 2 &&
	           style->value == "0",
	    "style: choices carried, default value" );

	const PropertyRow *custom = Row( sheet, "customkey" );
	Check( custom != nullptr && !custom->inSchema && custom->isSet && custom->value == "42",
	    "customkey: extra non-schema row from the entity" );

	// Meta keys are not rows.
	Check( Row( sheet, "classname" ) == nullptr && Row( sheet, "id" ) == nullptr,
	    "classname/id are not property rows" );
	Check( Row( sheet, "editor" ) == nullptr, "editor block is not a property row" );
}

void TestUnknownClass()
{
	auto fgd = ParseFgd( kFgd );
	auto pr = ParseKeyValues( "entity\n{\n\t\"classname\" \"not_in_fgd\"\n\t\"foo\" \"bar\"\n}\n" );
	EntityPropertySheet sheet = BuildPropertySheet( pr.root.children.front(), fgd.classes );
	Check( sheet.classname == "not_in_fgd", "unknown class captured" );
	Check( !sheet.classInFgd, "unknown class not resolved" );
	Check(
	    sheet.rows.size() == 1 && sheet.rows.front().key == "foo" && !sheet.rows.front().inSchema,
	    "unknown class -> only the entity's non-schema keys" );
}

} // namespace

int main()
{
	TestSheet();
	TestUnknownClass();

	if ( g_failures != 0 )
	{
		std::printf( "formats.entity_property_sheet: %d FAILURE(S)\n", g_failures );
		return 1;
	}
	std::printf( "formats.entity_property_sheet: schema rows (inherited, defaults-when-unset, "
	             "choices) + extra keys + meta exclusion all correct\n" );
	return 0;
}
