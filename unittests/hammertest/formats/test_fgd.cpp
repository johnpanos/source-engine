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
#include "testing/conformance_result.h"

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
int g_checks = 0;
int g_failures = 0;

void Check( bool ok, const char *label )
{
	++g_checks;
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

// R08-DOMAIN additions: helpers, I/O, flag defaults, readonly, @include and
// @KeyFrameClass, and their resolution along the base chain.
const char *kFgdExtended =
    "@include \"base.fgd\"\n"
    "// comment between includes\n"
    "@include \"extra.fgd\"\n"
    "@BaseClass color(0 0 255) sphere(radius) = Sized\n"
    "[\n"
    "\tinput Enable(void) : \"Base enable.\"\n"
    "\toutput OnUser1(void) : \"Fired by FireUser1.\"\n"
    "]\n"
    "@PointClass base(sized) studio(\"models/editor/x.mdl\") color(255 0 0) sphere(inner) "
    "line(255 255 255, targetname, target) halfgridsnap = thing : \"A thing.\"\n"
    "[\n"
    "\tangles(angle) readonly : \"Angles\" : \"0 0 0\"\n"
    "\tspawnflags(flags) =\n"
    "\t[\n"
    "\t\t1 : \"Start off\" : 1\n"
    "\t\t2 : \"Other\" : 0\n"
    "\t\t4 : \"No default\"\n"
    "\t]\n"
    "\tinput ENABLE(integer) : \"Own enable.\"\n"
    "\tinput Disable(void) : \"Disable \" + \"it.\"\n"
    "\toutput OnTrigger(string) : \"\"\n"
    "]\n"
    "@KeyFrameClass size(-4 -4 -4, 4 4 4) iconsprite(\"editor/keyframe.vmt\") = keyframe_x []\n";

const hammer::formats::FgdHelper *Helper( const EntityClass &c, const char *name, int nth = 0 )
{
	for ( const hammer::formats::FgdHelper &h : c.helpers )
	{
		if ( h.name == name && nth-- == 0 )
		{
			return &h;
		}
	}
	return nullptr;
}

const hammer::formats::FgdIo *Io(
    const std::vector<hammer::formats::FgdIo> &list, const char *name )
{
	for ( const hammer::formats::FgdIo &io : list )
	{
		if ( io.name == name )
		{
			return &io;
		}
	}
	return nullptr;
}

void TestExtendedParse()
{
	FgdParseResult r = ParseFgd( kFgdExtended );
	Check( r.ok, "extended: parse ok" );
	Check( r.includes.size() == 2 && r.includes[0] == "base.fgd" && r.includes[1] == "extra.fgd",
	    "extended: two @includes in order" );
	Check( r.includeLines.size() == 2 && r.includeLines[0] == 1 && r.includeLines[1] == 3,
	    "extended: @include lines 1 and 3" );
	Check( r.classes.size() == 3, "extended: three classes (keyframe kept)" );

	const EntityClass *thing = Find( r, "thing" );
	Check( thing != nullptr, "extended: thing found" );
	if ( thing != nullptr )
	{
		Check( thing->line == 9, "thing: directive line recorded" );
		Check( thing->helpers.size() == 5, "thing: five helpers (base excluded)" );
		const hammer::formats::FgdHelper *studio = Helper( *thing, "studio" );
		Check( studio != nullptr && studio->args.size() == 1 &&
		           studio->args[0] == "models/editor/x.mdl",
		    "thing: studio(\"path\") unquoted" );
		const hammer::formats::FgdHelper *line = Helper( *thing, "line" );
		Check( line != nullptr && line->args.size() == 3 && line->args[0] == "255 255 255" &&
		           line->args[1] == "targetname" && line->args[2] == "target",
		    "thing: line args split at commas" );
		const hammer::formats::FgdHelper *snap = Helper( *thing, "halfgridsnap" );
		Check( snap != nullptr && snap->args.empty(), "thing: bare helper word kept, no args" );
		Check( !thing->helpers.empty() && thing->helpers[0].name == "studio",
		    "thing: helpers in declared order" );

		const FgdProperty *angles = Prop( *thing, "angles" );
		Check( angles != nullptr && angles->readOnly, "thing: readonly modifier" );
		Check(
		    angles != nullptr && angles->displayName == "Angles" && angles->defaultValue == "0 0 0",
		    "thing: readonly keeps the colon fields" );
		const FgdProperty *flags = Prop( *thing, "spawnflags" );
		Check( flags != nullptr && !flags->readOnly, "thing: spawnflags not readonly" );
		Check( flags != nullptr && flags->choices.size() == 3 &&
		           flags->choices[0].defaultValue == "1" && flags->choices[1].defaultValue == "0" &&
		           flags->choices[2].defaultValue.empty(),
		    "thing: flag default fields 1, 0 and absent" );

		Check( thing->inputs.size() == 2 && thing->outputs.size() == 1,
		    "thing: own inputs and outputs stored" );
		const hammer::formats::FgdIo *disable = Io( thing->inputs, "Disable" );
		Check( disable != nullptr && disable->type == "void" && disable->help == "Disable it.",
		    "thing: input type and concatenated help" );
		const hammer::formats::FgdIo *trigger = Io( thing->outputs, "OnTrigger" );
		Check( trigger != nullptr && trigger->type == "string" && trigger->help.empty(),
		    "thing: output type, empty help" );
	}

	const EntityClass *key = Find( r, "keyframe_x" );
	Check( key != nullptr && key->kind == EntityKind::KeyFrame, "keyframe_x: KeyFrameClass kind" );
	if ( key != nullptr )
	{
		const hammer::formats::FgdHelper *size = Helper( *key, "size" );
		Check( size != nullptr && size->args.size() == 2 && size->args[0] == "-4 -4 -4" &&
		           size->args[1] == "4 4 4",
		    "keyframe_x: size args" );
		const hammer::formats::FgdHelper *icon = Helper( *key, "iconsprite" );
		Check( icon != nullptr && icon->args.size() == 1 && icon->args[0] == "editor/keyframe.vmt",
		    "keyframe_x: iconsprite path" );
	}

	// The original FGD: light's helpers are size and color; base() is not a helper.
	FgdParseResult legacy = ParseFgd( kFgd );
	const EntityClass *light = Find( legacy, "light" );
	Check( light != nullptr && light->helpers.size() == 2 && light->helpers[0].name == "size" &&
	           light->helpers[0].args.size() == 2 && light->helpers[0].args[0] == "-8 -8 -8" &&
	           light->helpers[1].name == "color" && light->helpers[1].args.size() == 1 &&
	           light->helpers[1].args[0] == "255 255 0",
	    "light: size and color helpers" );
	Check( legacy.includes.empty(), "light fgd: no includes" );

	Check(
	    !ParseFgd( "@include\n@PointClass = e []\n" ).ok, "@include without a file is an error" );
	FgdParseResult sphere = ParseFgd( "@PointClass sphere() = e []\n" );
	const EntityClass *e = Find( sphere, "e" );
	Check( e != nullptr && e->helpers.size() == 1 && e->helpers[0].name == "sphere" &&
	           e->helpers[0].args.empty(),
	    "sphere(): helper with an empty argument list" );
}

void TestExtendedResolve()
{
	FgdParseResult r = ParseFgd( kFgdExtended );
	// base(sized) names @BaseClass Sized: class names resolve case-insensitively.
	auto thing = ResolveClass( r.classes, "thing" );
	Check( thing.has_value(), "resolve extended: thing resolves" );
	if ( thing )
	{
		// Own color replaces the base's; own sphere replaces the base sphere in its
		// place; the other helpers follow in order.
		Check( thing->helpers.size() == 5, "resolve extended: five helpers after merge" );
		Check( thing->helpers.size() == 5 && thing->helpers[0].name == "color" &&
		           thing->helpers[0].args[0] == "255 0 0" && thing->helpers[1].name == "sphere" &&
		           thing->helpers[1].args[0] == "inner" && thing->helpers[2].name == "studio",
		    "resolve extended: derived helpers replace base helpers in place" );
		const hammer::formats::FgdHelper *second = Helper( *thing, "color", 1 );
		Check( second == nullptr, "resolve extended: base color dropped" );

		// Inputs merge by name, case-insensitively, derived winning in place.
		Check( thing->inputs.size() == 2 && thing->inputs[0].name == "ENABLE" &&
		           thing->inputs[0].type == "integer" && thing->inputs[1].name == "Disable",
		    "resolve extended: derived input overrides the base's in place" );
		Check( thing->outputs.size() == 2 && thing->outputs[0].name == "OnUser1" &&
		           thing->outputs[1].name == "OnTrigger",
		    "resolve extended: base outputs first" );
	}

	// Two helpers of one name declared by the class itself both survive.
	FgdParseResult twin = ParseFgd( "@BaseClass sphere(a) = B []\n"
	                                "@PointClass base(B) sphere(x) sphere(y) = e []\n" );
	auto e = ResolveClass( twin.classes, "e" );
	Check(
	    e && e->helpers.size() == 2 && e->helpers[0].args[0] == "x" && e->helpers[1].args[0] == "y",
	    "resolve extended: repeated own helpers survive, base one replaced" );

	// A dangling '+' after a help string (as in Valve's halflife2.fgd) ends it.
	FgdParseResult dangling = ParseFgd( "@PointClass = e\n[\n"
	                                    "\ta(float) : \"A\" : 1 : \"one \" + \"two\" +\n\n"
	                                    "\tb(string) : \"B\"\n]\n" );
	const EntityClass *d = Find( dangling, "e" );
	Check( dangling.ok && d != nullptr && d->properties.size() == 2 &&
	           d->properties[0].help == "one two" && d->properties[1].name == "b",
	    "parse: dangling '+' after a string is accepted" );

	// A differently-cased key in a derived class overrides the base key in place.
	FgdParseResult cased =
	    ParseFgd( "@BaseClass = B [ TargetName(string) : \"N\" : \"a\" ]\n"
	              "@PointClass base(B) = e [ targetname(string) : \"N\" : \"b\" ]\n" );
	auto c = ResolveClass( cased.classes, "e" );
	Check( c && c->properties.size() == 1 && c->properties[0].defaultValue == "b",
	    "resolve extended: key override is case-insensitive" );
}

} // namespace

int main()
{
	TestParse();
	TestResolveInheritance();
	TestExtendedParse();
	TestExtendedResolve();

	if ( g_failures != 0 )
	{
		std::printf( "formats.fgd: %d FAILURE(S)\n", g_failures );
		return testing::ReportConformance( g_checks, g_failures );
	}
	std::printf( "formats.fgd: class kinds, typed properties, choices/flags, defaults, and base "
	             "inheritance all parse + resolve correctly\n" );
	return testing::ReportConformance( g_checks, g_failures );
}
