//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance suite for the FGD-backed entity catalog
//			(formats.fgd_entity_catalog.v1) and the entity catalog port
//			(ports.entity_catalog.v1). The shared port suite runs against the
//			FGD catalog and against the test fake populated with the same
//			classes (Liskov: both claim the port). Then the FGD-specific
//			clauses: display hints, flag defaults, readonly keys, kinds,
//			include processing (depth-first, once each, cycle-safe, later
//			files override) and load errors with their file and line.
//
//			Optional corpus run: set HAMMER_FGD_CORPUS_DIR to a directory with
//			portal2.fgd and the files it includes (a Portal 2 install's bin/)
//			to load the real set. It adds checks only when set; the required
//			checks use self-contained text.
//
//=============================================================================//

#include "fakes/fake_entity_catalog.h"
#include "formats/entity_catalog_conformance.h"
#include "hammer/formats/fgd_entity_catalog.h"
#include "testing/checks.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using hammer::formats::FgdEntityCatalog;
using hammer::ports::EntityClassInfo;
using hammer::ports::EntityClassKind;
using hammer::ports::KeyType;
using hammertest::EntityCatalogExpectations;
using hammertest::FakeEntityCatalog;
using mapgeometry::Vec3d;

namespace
{

const char *kBaseFgd = "@BaseClass = Targetname\n"
                       "[\n"
                       "\ttargetname(target_source) : \"Name\" : : \"The name others use.\"\n"
                       "\tinput Kill(void) : \"Removes this entity.\"\n"
                       "\toutput OnUser1(void) : \"Fired by FireUser1.\"\n"
                       "]\n"
                       "@BaseClass = Angles\n"
                       "[\n"
                       "\tangles(angle) : \"Pitch Yaw Roll (Y Z X)\" : \"0 0 0\"\n"
                       "]\n"
                       "@BaseClass base(Targetname) = Parentname\n"
                       "[\n"
                       "\tparentname(target_destination) : \"Parent\"\n"
                       "]\n"
                       "@BaseClass size(-8 -8 -8, 8 8 8) color(0 200 0) = Sized\n"
                       "[\n"
                       "]\n"
                       "@PointClass = info_target : \"Old description.\" []\n";

const char *kGameFgd =
    "@include \"base.fgd\"\n"
    "@PointClass base(Parentname, Angles, Sized) studio(\"models/editor/thing.mdl\") = "
    "prop_thing : \"A thing.\"\n"
    "[\n"
    "\tangles(angle) readonly : \"Angles\" : \"0 90 0\"\n"
    "\tmodel(studio) : \"World Model\" : \"models/props/thing.mdl\"\n"
    "\tspawnflags(flags) =\n"
    "\t[\n"
    "\t\t1 : \"Start off\" : 1\n"
    "\t\t2 : \"Silent\" : 0\n"
    "\t]\n"
    "\tinput ENABLE(void) : \"Enable.\"\n"
    "\toutput OnTrigger(void) : \"Fired on trigger.\"\n"
    "]\n"
    "@PointClass base(Targetname) iconsprite(\"editor/light.vmt\") size(16 16 32) "
    "color(255 255 0) = Light : \"A light.\"\n"
    "[\n"
    "\t_light(color255) : \"Brightness\" : \"255 255 255 200\"\n"
    "]\n"
    "@PointClass base(Targetname) studio() = prop_default_model\n"
    "[\n"
    "\tmodel(studio) : \"World Model\" : \"models/props/default.mdl\"\n"
    "]\n"
    "@SolidClass base(Targetname) = func_Door : \"A door.\"\n"
    "[\n"
    "\tspeed(float) : \"Speed\" : \"100\"\n"
    "]\n"
    "@NPCClass base(Targetname) = npc_thing []\n"
    "@FilterClass base(Targetname) = filter_thing []\n"
    "@KeyFrameClass base(Targetname) = keyframe_thing []\n"
    "@MoveClass base(Targetname) = move_thing []\n"
    "@PointClass = INFO_TARGET : \"New description.\" []\n";

// A loader over an in-memory directory; records every name asked for.
struct MemoryFiles
{
	std::vector<std::pair<std::string, std::string>> files;
	mutable std::vector<std::string> asked;

	hammer::formats::FgdLoader Loader() const
	{
		return [this]( const std::string &name ) -> std::optional<std::string>
		{
			asked.push_back( name );
			for ( const auto &file : files )
			{
				if ( file.first == name )
				{
					return file.second;
				}
			}
			return std::nullopt;
		};
	}
};

EntityCatalogExpectations Expectations()
{
	EntityCatalogExpectations e;
	e.classNames = { "prop_thing", "Light", "prop_default_model", "func_Door", "npc_thing",
	    "filter_thing", "keyframe_thing", "move_thing", "INFO_TARGET" };
	e.baseClassNames = { "Targetname", "Angles", "Parentname", "Sized" };
	e.unknownNames = { "prop_thin", "prop_things", "" };
	e.classes = {
	    { "prop_thing", EntityClassKind::Point,
	        { "targetname", "parentname", "angles", "model", "spawnflags" }, { "Kill", "ENABLE" },
	        { "OnUser1", "OnTrigger" } },
	    { "light", EntityClassKind::Point, { "targetname", "_light" }, { "Kill" }, { "OnUser1" } },
	    { "FUNC_DOOR", EntityClassKind::Solid, { "targetname", "speed" }, { "Kill" },
	        { "OnUser1" } },
	    { "npc_thing", EntityClassKind::Other, { "targetname" }, { "Kill" }, { "OnUser1" } },
	    { "filter_thing", EntityClassKind::Other, { "targetname" }, { "Kill" }, { "OnUser1" } },
	    { "keyframe_thing", EntityClassKind::Other, { "targetname" }, { "Kill" }, { "OnUser1" } },
	    { "move_thing", EntityClassKind::Other, { "targetname" }, { "Kill" }, { "OnUser1" } },
	    { "info_target", EntityClassKind::Point, {}, {}, {} },
	};
	e.defaults = { { "prop_thing", "angles", "0 90 0" }, { "func_door", "speed", "100" },
	    { "prop_thing", "model", "models/props/thing.mdl" } };
	return e;
}

// The same classes as the FGD above, declared directly on the fake.
FakeEntityCatalog EquivalentFake()
{
	using C = FakeEntityCatalog;
	const auto kill = C::Io( "Kill" );
	const auto user1 = C::Io( "OnUser1" );
	FakeEntityCatalog f;
	f.AddPoint( "prop_thing",
	     { C::Key( "targetname", "target_source" ), C::Key( "parentname", "target_destination" ),
	         C::Key( "angles", "angle", "0 90 0" ),
	         C::Key( "model", "studio", "models/props/thing.mdl" ),
	         C::Key( "spawnflags", "flags" ) },
	     { kill, C::Io( "ENABLE" ) }, { user1, C::Io( "OnTrigger" ) } )
	    .Box( Vec3d( -8, -8, -8 ), Vec3d( 8, 8, 8 ) )
	    .Model( "models/editor/thing.mdl" );
	f.AddPoint( "Light",
	     { C::Key( "targetname", "target_source" ),
	         C::Key( "_light", "color255", "255 255 255 200" ) },
	     { kill }, { user1 } )
	    .Box( Vec3d( -8, -8, -16 ), Vec3d( 8, 8, 16 ) )
	    .Color( Vec3d( 255, 255, 0 ) )
	    .Sprite( "editor/light.vmt" );
	f.AddPoint( "prop_default_model",
	    { C::Key( "targetname", "target_source" ),
	        C::Key( "model", "studio", "models/props/default.mdl" ) },
	    { kill }, { user1 } );
	f.AddSolid( "func_Door",
	    { C::Key( "targetname", "target_source" ), C::Key( "speed", "float", "100" ) }, { kill },
	    { user1 } );
	for ( const char *name : { "npc_thing", "filter_thing", "keyframe_thing", "move_thing" } )
	{
		f.AddKind( EntityClassKind::Other, name, { C::Key( "targetname", "target_source" ) },
		    { kill }, { user1 } );
	}
	f.AddPoint( "info_target" ).Description( "Old description." );
	f.AddPoint( "INFO_TARGET" ).Description( "New description." ); // replaces, as a later FGD would
	return f;
}

void TestPortConformance( testing::Checks &checks )
{
	MemoryFiles dir;
	dir.files = { { "base.fgd", kBaseFgd }, { "game.fgd", kGameFgd } };
	auto loaded = FgdEntityCatalog::Load( "game.fgd", dir.Loader() );
	checks.That( loaded.HasValue(), "FGD catalog loads" );
	if ( !loaded )
	{
		std::printf( "load error: %s:%zu %s\n", loaded.Error().file.c_str(), loaded.Error().line,
		    loaded.Error().message.c_str() );
		return;
	}
	const std::size_t before = checks.Failures();
	hammertest::RunEntityCatalogConformance( checks, loaded.Value(), Expectations() );
	checks.Equal( checks.Failures(), before, "port suite passes on FgdEntityCatalog" );

	const FakeEntityCatalog fake = EquivalentFake();
	const std::size_t beforeFake = checks.Failures();
	hammertest::RunEntityCatalogConformance( checks, fake, Expectations() );
	checks.Equal( checks.Failures(), beforeFake, "port suite passes on FakeEntityCatalog" );

	// The fake and the FGD catalog agree on what the port exposes.
	for ( const std::string &name : loaded.Value().ClassNames() )
	{
		const EntityClassInfo *a = loaded.Value().Find( name );
		const EntityClassInfo *b = fake.Find( name );
		bool same = a && b && a->kind == b->kind && a->keys.size() == b->keys.size() &&
		            a->inputs.size() == b->inputs.size() &&
		            a->outputs.size() == b->outputs.size() && a->boxMins == b->boxMins &&
		            a->boxMaxs == b->boxMaxs;
		for ( std::size_t i = 0; same && i < a->keys.size(); ++i )
		{
			same = a->keys[i].key == b->keys[i].key && a->keys[i].type == b->keys[i].type &&
			       a->keys[i].defaultValue == b->keys[i].defaultValue;
		}
		checks.That( same, "fake mirrors the FGD catalog: " + name );
	}
}

void TestFgdSpecifics( testing::Checks &checks )
{
	MemoryFiles dir;
	dir.files = { { "base.fgd", kBaseFgd }, { "game.fgd", kGameFgd } };
	auto loaded = FgdEntityCatalog::Load( "game.fgd", dir.Loader() );
	if ( !checks.That( loaded.HasValue(), "specifics: catalog loads" ) )
	{
		return;
	}
	// Moving the catalog keeps Find's pointers valid.
	const EntityClassInfo *before = loaded.Value().Find( "prop_thing" );
	FgdEntityCatalog catalog = std::move( loaded.Value() );
	const EntityClassInfo *thing = catalog.Find( "prop_thing" );
	checks.That( thing != nullptr && thing == before, "move keeps result pointers" );
	checks.That( catalog.Files() == std::vector<std::string>{ "base.fgd", "game.fgd" },
	    "files: include read before its includer" );
	checks.That( dir.asked == std::vector<std::string>{ "game.fgd", "base.fgd" },
	    "loader asked for the entry, then its include, once each" );
	if ( thing == nullptr )
	{
		return;
	}

	checks.That( thing->description == "A thing.", "prop_thing: description" );
	checks.That( thing->boxMins == Vec3d( -8, -8, -8 ) && thing->boxMaxs == Vec3d( 8, 8, 8 ),
	    "prop_thing: size() inherited from a base class" );
	checks.That( thing->color == Vec3d( 0, 200, 0 ), "prop_thing: color() inherited" );
	checks.Equal( thing->model, std::string( "models/editor/thing.mdl" ),
	    "prop_thing: studio(\"path\") is the model hint" );
	checks.That( thing->sprite.empty(), "prop_thing: no sprite" );

	const hammer::ports::KeyDefinition *angles = thing->FindKey( "angles" );
	checks.That( angles && angles->readOnly && angles->type == KeyType::Angle &&
	                 angles->typeName == "angle" && angles->displayName == "Angles",
	    "prop_thing: own readonly angles replaces the base key" );
	const hammer::ports::KeyDefinition *target = thing->FindKey( "targetname" );
	checks.That( target && !target->readOnly && target->type == KeyType::TargetSource &&
	                 target->displayName == "Name" && target->defaultValue.empty() &&
	                 target->help == "The name others use.",
	    "prop_thing: inherited targetname fields" );
	const hammer::ports::KeyDefinition *flags = thing->FindKey( "spawnflags" );
	checks.That( flags && flags->type == KeyType::Flags && flags->choices.size() == 2 &&
	                 flags->choices[0].value == "1" && flags->choices[0].label == "Start off" &&
	                 flags->choices[0].defaultOn && !flags->choices[1].defaultOn,
	    "prop_thing: flags with defaultOn from the third field" );
	checks.That( thing->inputs.size() == 2 && thing->inputs[1].name == "ENABLE" &&
	                 thing->inputs[1].type == "void" && thing->inputs[1].help == "Enable.",
	    "prop_thing: input fields" );

	const EntityClassInfo *light = catalog.Find( "LIGHT" );
	checks.That(
	    light && light->boxMins == Vec3d( -8, -8, -16 ) && light->boxMaxs == Vec3d( 8, 8, 16 ),
	    "Light: size(x y z) is a centered box" );
	checks.That( light && light->color == Vec3d( 255, 255, 0 ), "Light: color" );
	checks.That( light && light->sprite == "editor/light.vmt" && light->model.empty(),
	    "Light: iconsprite path" );
	const hammer::ports::KeyDefinition *brightness = light ? light->FindKey( "_LIGHT" ) : nullptr;
	checks.That( brightness && brightness->type == KeyType::Color255 &&
	                 brightness->defaultValue == "255 255 255 200",
	    "Light: color255 key" );

	const EntityClassInfo *studio = catalog.Find( "prop_default_model" );
	checks.That( studio && studio->model == "models/props/default.mdl",
	    "studio() without a path uses the model key's default" );

	const EntityClassInfo *door = catalog.Find( "func_door" );
	checks.That( door && door->kind == EntityClassKind::Solid && !door->boxMins && !door->color,
	    "func_Door: solid, no hints declared" );

	const EntityClassInfo *info = catalog.Find( "info_target" );
	checks.That( info && info->name == "INFO_TARGET" && info->description == "New description.",
	    "a later class of the same name (any case) replaces the earlier one" );
}

void TestIncludes( testing::Checks &checks )
{
	// A cycle, a repeated include and differently spelled names: each file once.
	MemoryFiles dir;
	dir.files = {
	    { "a.fgd", "@include \"sub\\B.FGD\"\n@include \"c.fgd\"\n@PointClass = from_a []\n" },
	    { "sub\\B.FGD", "@include \"a.fgd\"\n@include \"c.fgd\"\n@PointClass = from_b []\n" },
	    { "c.fgd", "@include \"SUB/b.fgd\"\n@PointClass = from_c : \"c\" []\n" },
	};
	auto cyc = FgdEntityCatalog::Load( "a.fgd", dir.Loader() );
	checks.That( cyc.HasValue(), "include cycle loads" );
	if ( cyc )
	{
		checks.That(
		    cyc.Value().Files() == std::vector<std::string>{ "c.fgd", "sub\\B.FGD", "a.fgd" },
		    "includes depth-first, once each (names case- and slash-insensitive)" );
		checks.That(
		    cyc.Value().ClassNames() == std::vector<std::string>{ "from_a", "from_b", "from_c" },
		    "every file's classes present" );
	}
	checks.Equal( dir.asked.size(), std::size_t( 3 ), "loader asked once per file" );

	// FromTexts: files in order, the later one overriding.
	auto texts = FgdEntityCatalog::FromTexts( {
	    { "one.fgd", "@PointClass = info_x : \"one\" []\n" },
	    { "two.fgd", "@PointClass = INFO_X : \"two\" []\n@include \"one.fgd\"\n" },
	} );
	checks.That( texts && texts.Value().ClassNames().size() == 1 &&
	                 texts.Value().Find( "info_x" )->description == "two",
	    "FromTexts: later file overrides; an include already read is not re-read" );

	// Errors carry the file and line.
	MemoryFiles missing;
	missing.files = { { "game.fgd", "// header\n@include \"base.fgd\"\n@include \"gone.fgd\"\n" },
	    { "base.fgd", "@PointClass = e []\n" } };
	auto noInclude = FgdEntityCatalog::Load( "game.fgd", missing.Loader() );
	checks.That( !noInclude && noInclude.Error().file == "game.fgd" &&
	                 noInclude.Error().line == 3 &&
	                 noInclude.Error().message.find( "gone.fgd" ) != std::string::npos,
	    "missing include: includer's file and the @include line" );

	auto noEntry = FgdEntityCatalog::Load( "nope.fgd", missing.Loader() );
	checks.That( !noEntry && noEntry.Error().file == "nope.fgd" && noEntry.Error().line == 0,
	    "missing entry file reported" );

	MemoryFiles broken;
	broken.files = { { "game.fgd", "@include \"base.fgd\"\n" },
	    { "base.fgd", "@PointClass = ok []\n@PointClass = bad\n[\n\tnotype : \"x\"\n]\n" } };
	auto parse = FgdEntityCatalog::Load( "game.fgd", broken.Loader() );
	checks.That( !parse && parse.Error().file == "base.fgd" && parse.Error().line == 4,
	    "parse error in an include: that file and line" );

	MemoryFiles unknownBase;
	unknownBase.files = {
	    { "game.fgd", "@PointClass = ok []\n\n@PointClass base(Nope) = e []\n" } };
	auto base = FgdEntityCatalog::Load( "game.fgd", unknownBase.Loader() );
	checks.That( !base && base.Error().file == "game.fgd" && base.Error().line == 3 &&
	                 base.Error().message.find( "Nope" ) != std::string::npos,
	    "unknown base class: the class's file and line" );

	// The Portal 2 header shape: CRLF lines and three includes at lines 7-9.
	MemoryFiles portal;
	portal.files = { { "portal2.fgd",
	    "//====== Copyright (c) Valve ======\r\n//\r\n// Purpose: Aperture\r\n//\r\n"
	    "//=====\r\n\r\n@include \"base.fgd\"\r\n@include \"portal.fgd\"\r\n"
	    "@include \"halflife2.fgd\"\r\n\r\n@SolidClass = worldspawn []\r\n" } };
	auto p2 = FgdEntityCatalog::Load( "portal2.fgd", portal.Loader() );
	checks.That( !p2 && p2.Error().file == "portal2.fgd" && p2.Error().line == 7 &&
	                 p2.Error().message.find( "base.fgd" ) != std::string::npos,
	    "CRLF file: the first missing include reported at line 7" );
}

std::optional<std::string> ReadFile( const std::string &path )
{
	std::ifstream in( path, std::ios::binary );
	if ( !in )
	{
		return std::nullopt;
	}
	std::ostringstream text;
	text << in.rdbuf();
	return text.str();
}

// Optional: the real Portal 2 FGD set.
void TestCorpus( testing::Checks &checks )
{
	const char *dir = std::getenv( "HAMMER_FGD_CORPUS_DIR" );
	if ( dir == nullptr || *dir == '\0' )
	{
		std::printf( "note: HAMMER_FGD_CORPUS_DIR unset; the real-FGD corpus run is skipped\n" );
		return;
	}
	const std::string root = dir;
	auto loaded = FgdEntityCatalog::Load( "portal2.fgd",
	    [&root]( const std::string &name )
	    {
		    return ReadFile( root + "/" + name );
	    } );
	if ( !checks.That( loaded.HasValue(), "corpus: portal2.fgd and its includes load" ) )
	{
		std::printf( "corpus error: %s:%zu %s\n", loaded.Error().file.c_str(), loaded.Error().line,
		    loaded.Error().message.c_str() );
		return;
	}
	const FgdEntityCatalog &catalog = loaded.Value();
	std::printf(
	    "corpus: %zu files, %zu classes\n", catalog.Files().size(), catalog.ClassNames().size() );
	checks.That( catalog.ClassNames().size() > 300, "corpus: hundreds of classes" );
	const EntityClassInfo *start = catalog.Find( "info_player_start" );
	checks.That(
	    start && start->model == "models/editor/playerstart.mdl" && start->FindKey( "angles" ),
	    "corpus: info_player_start" );
	const EntityClassInfo *door = catalog.Find( "func_door" );
	checks.That( door && door->kind == EntityClassKind::Solid && door->HasInput( "open" ) &&
	                 door->HasOutput( "onfullyopen" ),
	    "corpus: func_door" );
	const EntityClassInfo *light = catalog.Find( "light" );
	checks.That( light && !light->sprite.empty(), "corpus: light has an icon sprite" );
	EntityCatalogExpectations e;
	e.classNames = catalog.ClassNames();
	e.baseClassNames = { "Targetname", "Parentname", "Angles" };
	hammertest::RunEntityCatalogConformance( checks, catalog, e );
}

} // namespace

int main()
{
	testing::Checks checks;
	TestPortConformance( checks );
	TestFgdSpecifics( checks );
	TestIncludes( checks );
	TestCorpus( checks );
	return checks.Report();
}
