//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance oracle for the FGD entity-schema parser
//			(formats.fgd.v1). Parses a representative FGD and checks class kinds,
//			typed key properties, defaults, choices/flags value lists, base()
//			inheritance, and ResolveClass flattening. Build/run via the conformance
//			manifest (linux-headless-core). Exit 0 on success.
//
//=============================================================================//

#include "hammer/formats/fgd.h"

#include <cstdio>
#include <string>

using hammer::formats::EntityClass;
using hammer::formats::EntityKind;
using hammer::formats::FgdParseResult;
using hammer::formats::FgdProperty;
using hammer::formats::ParseFgd;
using hammer::formats::ResolveClass;

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

const EntityClass *Find( const FgdParseResult &r, const char *name )
{
	for ( const EntityClass &c : r.classes )
	{
		if ( c.name == name )
		{
			return &c;
		}
	}
	return nullptr;
}

const FgdProperty *Prop( const EntityClass &c, const char *name )
{
	for ( const FgdProperty &p : c.properties )
	{
		if ( p.name == name )
		{
			return &p;
		}
	}
	return nullptr;
}

const char *kFgd =
    "@BaseClass = Targetname\n"
    "[\n"
    "\ttargetname(target_source) : \"Name\" : \"\" : \"The name other entities refer to it by.\"\n"
    "]\n"
    "@BaseClass = Angles\n"
    "[\n"
    "\tangles(angle) : \"Pitch Yaw Roll\" : \"0 0 0\"\n"
    "]\n"
    "// a comment line\n"
    "@PointClass base(Targetname, Angles) size(-8 -8 -8, 8 8 8) color(255 255 0) = light : "
    "\"A dynamic light.\"\n"
    "[\n"
    "\t_light(color255) : \"Brightness\" : \"255 255 255 200\"\n"
    "\tstyle(choices) : \"Appearance\" : \"0\" =\n"
    "\t[\n"
    "\t\t0 : \"Normal\"\n"
    "\t\t1 : \"Flicker A\"\n"
    "\t\t10 : \"Fluorescent flicker\"\n"
    "\t]\n"
    "\tspawnflags(flags) =\n"
    "\t[\n"
    "\t\t1 : \"Initially dark\" : 0\n"
    "\t]\n"
    "]\n"
    "@SolidClass = func_detail : \"Detail brushes.\"\n"
    "[\n"
    "\tdisablereceiveshadows(boolean) : \"Disable Receiving Shadows\" : \"0\"\n"
    "]\n"
    "@mapsize(-16384, 16384)\n";

void TestParse()
{
	FgdParseResult r = ParseFgd( kFgd );
	Check( r.ok, "parse: ok" );
	// @mapsize is skipped; four classes remain.
	Check( r.classes.size() == 4, "parse: four classes (mapsize skipped)" );

	const EntityClass *light = Find( r, "light" );
	Check( light != nullptr, "parse: light found" );
	if ( light != nullptr )
	{
		Check( light->kind == EntityKind::Point, "light: PointClass kind" );
		Check( light->description == "A dynamic light.", "light: description" );
		Check( light->bases.size() == 2 && light->bases[0] == "Targetname" &&
		           light->bases[1] == "Angles",
		    "light: base(Targetname, Angles)" );

		const FgdProperty *style = Prop( *light, "style" );
		Check( style != nullptr, "light: style property present" );
		if ( style != nullptr )
		{
			Check( style->type == "choices", "light: style type choices" );
			Check( style->defaultValue == "0", "light: style default 0" );
			Check( style->choices.size() == 3, "light: style has 3 choices" );
			Check( style->choices[0].value == "0" && style->choices[0].label == "Normal",
			    "light: choice 0 = Normal" );
			Check(
			    style->choices[2].value == "10" && style->choices[2].label == "Fluorescent flicker",
			    "light: choice 10 label" );
		}
		const FgdProperty *flags = Prop( *light, "spawnflags" );
		Check( flags != nullptr && flags->type == "flags" && flags->choices.size() == 1 &&
		           flags->choices[0].label == "Initially dark",
		    "light: spawnflags parsed" );
	}

	const EntityClass *detail = Find( r, "func_detail" );
	Check( detail != nullptr && detail->kind == EntityKind::Solid, "func_detail: SolidClass" );

	const EntityClass *tn = Find( r, "Targetname" );
	Check( tn != nullptr && tn->kind == EntityKind::Base, "Targetname: BaseClass" );
	if ( tn != nullptr )
	{
		const FgdProperty *p = Prop( *tn, "targetname" );
		Check( p != nullptr && p->type == "target_source", "Targetname: targetname type" );
		Check( p != nullptr && p->defaultValue.empty(), "Targetname: empty default field" );
		Check( p != nullptr && p->help.find( "refer" ) != std::string::npos,
		    "Targetname: help text after empty default" );
	}
}

void TestResolveInheritance()
{
	FgdParseResult r = ParseFgd( kFgd );
	Check( r.ok, "resolve: parse ok" );

	auto resolved = ResolveClass( r.classes, "light" );
	Check( resolved.has_value(), "resolve: light resolves" );
	if ( resolved )
	{
		// Base properties come first, in base() order, then the class's own.
		Check( !resolved->properties.empty() && resolved->properties[0].name == "targetname",
		    "resolve: inherited targetname first" );
		Check( Prop( *resolved, "angles" ) != nullptr, "resolve: inherited angles present" );
		Check( Prop( *resolved, "_light" ) != nullptr, "resolve: own _light present" );
		Check( Prop( *resolved, "style" ) != nullptr, "resolve: own style present" );
	}

	Check( !ResolveClass( r.classes, "does_not_exist" ).has_value(),
	    "resolve: unknown class -> nullopt" );
}

} // namespace

int main()
{
	TestParse();
	TestResolveInheritance();

	if ( g_failures != 0 )
	{
		std::printf( "formats.fgd: %d FAILURE(S)\n", g_failures );
		return 1;
	}
	std::printf( "formats.fgd: class kinds, typed properties, choices/flags, defaults, and base "
	             "inheritance all parse + resolve correctly\n" );
	return 0;
}
