//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.formats VMF map codec conformance (RFC 0002, R08 domain
//			logic; contracts formats.vmf_map_codec.v1 and ports.map_codec.v1).
//			Runs the shared IMapCodec oracle (map_codec_oracle.h) against the
//			strict VmfMapCodec: the canonical feature fixture field by field and
//			byte for byte (typed displacements, editor logical positions and
//			comments, version and view settings), both cordon forms, missing
//			ids, an edited document whose runtime ids are not in write order,
//			and the repository's sample maps.
//
//			Negative checks: every kind of content the model cannot hold is a
//			CodecError with its block path and line; parse errors report a line;
//			foreign text is rejected; content VMF cannot hold fails Encode
//			instead of being written wrongly.
//
//			Optional corpus: when HAMMER_VMF_CORPUS names a directory, every
//			.vmf in it is also round-tripped (not part of the required run).
//
//=============================================================================//

#include "hammer/formats/vmf_map_codec.h"
#include "hammer/scene/solid_geometry.h"
#include "map_codec_oracle.h"
#include "testing/checks.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <string>
#include <vector>

using namespace hammer::scene;
using hammer::formats::VmfMapCodec;
using mapgeometry::Vec3d;

namespace
{

std::string ReadFile( const std::filesystem::path &path )
{
	std::ifstream input( path, std::ios::binary );
	return std::string( std::istreambuf_iterator<char>{ input }, std::istreambuf_iterator<char>{} );
}

// The repository root: the working directory when it holds the samples, else
// derived from this source file's path.
std::filesystem::path RepoRoot()
{
	if ( std::filesystem::is_directory( "hammer/gtk/samples" ) )
	{
		return std::filesystem::current_path();
	}
	return std::filesystem::path( __FILE__ )
	    .parent_path()
	    .parent_path()
	    .parent_path()
	    .parent_path();
}

void Report( testing::Checks &checks, const map_codec_oracle::Findings &f, const std::string &what )
{
	checks.That( f.Ok(), what + ( f.Ok() ? "" : ": " + f.Summary() ) );
}

Solid MakeSolid( MapDocument &doc, const Box &box, const std::string &material )
{
	FaceTexture tex;
	tex.material = material;
	Solid s = MakeBoxSolid( box, tex );
	s.id = doc.AllocateId();
	s.vmfId = doc.AllocateVmfId();
	for ( Side &side : s.sides )
	{
		side.vmfId = doc.AllocateVmfId();
	}
	return s;
}

// A full power-2 displacement with values %.10g cannot hold exactly.
Displacement MakeDisplacement()
{
	Displacement d;
	d.power = 2;
	d.startPosition = Vec3d( 0, 0, 64.0 / 3.0 );
	d.flags = 4;
	d.elevation = 0.1 + 0.2;
	d.subdivided = true;
	d.normals = std::vector<Vec3d>( 25, Vec3d( 0, 0, 1 ) );
	d.distances = std::vector<double>( 25, 1.0 / 3.0 );
	d.offsets = std::vector<Vec3d>( 25, Vec3d( 0.5, 0, 0 ) );
	d.offsetNormals = std::vector<Vec3d>( 25, Vec3d( 0, 0, 1 ) );
	d.alphas = std::vector<double>( 25, 255 );
	d.triangleTags = std::vector<int>( 32, 9 );
	d.allowedVerts = std::vector<std::int64_t>( 10, -1 );
	return d;
}

// A document as editing leaves it: runtime ids in creation order (an entity
// before the world solid, groups last), values that %.10g cannot hold exactly,
// and every settings field populated.
MapDocument EditedDocument()
{
	MapDocument doc( 3 );

	Entity light;
	light.id = doc.AllocateId();
	light.vmfId = doc.AllocateVmfId();
	light.classname = "light";
	light.SetOrigin( Vec3d( 1.0 / 3.0, -2.5, 1e-7 ) );
	light.SetKey( "_light", "255 255 255 200" );
	light.connections.push_back( { "OnUser1", "lamp", "TurnOff", "", 1.0 / 3.0, -1, ',' } );
	light.connections.push_back( { "OnUser2", "lamp", "SetPattern", "x,y", 0.25, 2, '\x1b' } );
	light.editor.color = Rgb{ 1, 2, 3 };
	light.editor.visgroupIds = { 7 };
	light.editor.logicalPos = std::array<int, 2>{ -250, 1000 };
	light.editor.comments = "a lamp";
	doc.Put( light );

	Solid wall = MakeSolid(
	    doc, Box{ Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 64.0 / 3.0 ) }, "BRICK/BRICKWALL001A" );
	wall.sides[0].texture.u.shift = 0.1 + 0.2;
	wall.sides[0].texture.rotation = 1.0 / 7.0;
	wall.sides[0].texture.smoothingGroups = 0xFFFFFFFFu;
	wall.sides[0].displacement = MakeDisplacement();
	wall.editor.color = Rgb{ 5, 6, 7 };

	Entity door;
	door.id = doc.AllocateId();
	door.vmfId = doc.AllocateVmfId();
	door.classname = "func_door";
	door.hidden = true;
	door.keys = { { "speed", "100" }, { "speed", "200" } };
	Solid panel = MakeSolid( doc, Box{ Vec3d( 64, 0, 0 ), Vec3d( 72, 64, 96 ) }, "METAL/DOOR01" );
	panel.owner = door.id;
	panel.hidden = true;

	Group outer;
	outer.id = doc.AllocateId();
	outer.vmfId = doc.AllocateVmfId();
	outer.hidden = true;
	Group inner;
	inner.id = doc.AllocateId();
	inner.vmfId = doc.AllocateVmfId();
	inner.group = outer.id;
	inner.editor.color = Rgb{ 9, 9, 9 };
	wall.group = inner.id;
	door.group = outer.id;

	doc.Put( wall );
	doc.Put( door );
	doc.Put( panel );
	doc.Put( outer );
	doc.Put( inner );

	DocumentSettings &s = doc.MutableSettings();
	s.version = VersionInfo{ 400, 5195, 12, 100, true };
	s.view = ViewSettings{ false, true, true, 8, true };
	s.worldVmfId = 1;
	s.SetWorldKey( "skyname", "sky_dust" );
	Visgroup child;
	child.id = 8;
	child.name = "Child";
	Visgroup parent;
	parent.id = 7;
	parent.name = "Parent";
	parent.color = Rgb{ 10, 20, 30 };
	parent.children = { child };
	s.visgroups = { parent };
	s.cameras = { { Vec3d( 1.0 / 3.0, 2, 3 ), Vec3d( 4, 5, 6 ) } };
	s.activeCamera = 0;
	Cordon cordon;
	cordon.active = true;
	cordon.boxes = { { Vec3d( -8, -8, -8 ), Vec3d( 8, 8, 8 + 1.0 / 3.0 ) } };
	s.cordons = { cordon };
	s.cordonsActive = true;
	s.cordonForm = DocumentSettings::CordonForm::Single;
	return doc;
}

bool EncodeFails( const MapDocument &doc, const std::string &fragment )
{
	const auto result = VmfMapCodec().Encode( doc );
	return !result && result.Error().message.find( fragment ) != std::string::npos;
}

} // namespace

int main()
{
	testing::Checks checks;
	const VmfMapCodec codec;
	checks.That( codec.FormatName() == "vmf", "format name" );

	// --- The shared port oracle ------------------------------------------------------------
	Report( checks, map_codec_oracle::CanonicalFixture( codec ), "canonical fixture fields" );
	Report( checks, map_codec_oracle::CanonicalText( codec, map_codec_oracle::CanonicalVmf() ),
	    "canonical fixture byte for byte" );
	Report( checks, map_codec_oracle::RoundTrip( codec, map_codec_oracle::CanonicalVmf() ),
	    "canonical fixture round trip" );
	Report( checks, map_codec_oracle::SingleCordonFixture( codec ), "single cordon form" );
	Report( checks, map_codec_oracle::MissingIdsFixture( codec ), "missing ids" );
	Report( checks, map_codec_oracle::RejectsUnmodeled( codec ), "unmodeled content is rejected" );
	Report( checks, map_codec_oracle::DocumentRoundTrip( codec, EditedDocument() ),
	    "edited document round trip up to runtime ids" );
	Report( checks, map_codec_oracle::DocumentRoundTrip( codec, MapDocument( 4 ) ),
	    "new empty document round trip" );
	checks.That( map_codec_oracle::RejectionCases().size() >= 30,
	    "the rejection catalogue covers 30 kinds" );

	// Edited ids come back renumbered in write order, and the document then
	// round-trips with identical ids and exact numbers.
	{
		const MapDocument edited = EditedDocument();
		const auto text = codec.Encode( edited );
		checks.That( text.HasValue(), "edited document encodes" );
		if ( text )
		{
			Report( checks, map_codec_oracle::RoundTrip( codec, text.Value(), 3 ),
			    "encoded edited document round trip" );
			const auto decoded = codec.Decode( text.Value(), 3 );
			if ( decoded )
			{
				const MapDocument &d = decoded.Value().document;
				const Solid &original = *edited.FindSolid( edited.SolidIds().front() );
				const Solid *wall = map_codec_oracle::ByVmfId( d.Solids(), original.vmfId );
				checks.That( wall && wall->sides[0].texture.u.shift == 0.1 + 0.2 &&
				                 wall->sides[0].texture.rotation == 1.0 / 7.0 &&
				                 wall->sides[0].points == original.sides[0].points,
				    "numbers %.10g cannot hold come back bit for bit" );
				checks.That( wall && wall->sides[0].displacement == MakeDisplacement(),
				    "a typed displacement comes back exactly" );
				checks.That( wall && wall->sides[0].texture.smoothingGroups == 0xFFFFFFFFu,
				    "all 32 smoothing-group bits survive" );
				checks.That(
				    d.SolidIds().size() == 2 && d.FindSolid( d.SolidIds().front() ) == wall,
				    "world solids get the lowest runtime ids" );
			}
			checks.That(
			    text.Value().find( "lamp,TurnOff,,0.3333333333333333,-1" ) != std::string::npos,
			    "an inexact connection delay is written exactly" );
			checks.That(
			    text.Value().find( "\"logicalpos\" \"[-250 1000]\"" ) != std::string::npos &&
			        text.Value().find( "\"comments\" \"a lamp\"" ) != std::string::npos,
			    "logical position and comments are written" );
			checks.That( text.Value().find( "\"allowed_verts\"" ) == std::string::npos &&
			                 text.Value().find( "\"10\" \"-1 -1 -1 -1 -1 -1 -1 -1 -1 -1\"" ) !=
			                     std::string::npos,
			    "allowed_verts is keyed by its word count" );
		}
	}

	// Cordon form normalization: a named cordon, or cordons without a form, are
	// written in the list form.
	{
		MapDocument doc( 5 );
		Cordon named;
		named.name = "named";
		named.boxes = { { Vec3d( 0, 0, 0 ), Vec3d( 1, 1, 1 ) } };
		doc.MutableSettings().cordons = { named };
		doc.MutableSettings().cordonsActive = true;
		doc.MutableSettings().cordonForm = DocumentSettings::CordonForm::Single;
		const auto text = codec.Encode( doc );
		checks.That( text && text.Value().find( "cordons\n{" ) != std::string::npos &&
		                 text.Value().find( "\"name\" \"named\"" ) != std::string::npos,
		    "a named cordon is written in the list form" );
		Report( checks, map_codec_oracle::DocumentRoundTrip( codec, doc ), "named single cordon" );
		doc.MutableSettings().cordonForm = DocumentSettings::CordonForm::None;
		Report(
		    checks, map_codec_oracle::DocumentRoundTrip( codec, doc ), "cordons without a form" );
	}

	// --- VMF-specific error text ---------------------------------------------------------------
	{
		const std::string text = map_codec_oracle::InsertAfter(
		    map_codec_oracle::SmallVmf(), "\"id\" \"4\"", "\t\t\t\"foo\" \"1\"\n" );
		const auto result = codec.Decode( text, 1 );
		checks.That( !result &&
		                 result.Error().message == "world/solid[0]/side[1]: unknown key 'foo'" &&
		                 result.Error().line == map_codec_oracle::LineOf( text, "\"foo\"" ),
		    "an unknown key names its block path and line exactly" );
	}

	// --- Negative: text that is not VMF --------------------------------------------------------
	{
		const auto open = codec.Decode( "versioninfo\n{\n\t\"editorversion\" \"400\"\n", 1 );
		checks.That( !open && open.Error().line == 4, "an unclosed block reports the end line" );
		const auto stray = codec.Decode( "world\n{\n}\n}\n", 1 );
		checks.That( !stray && stray.Error().line == 4, "a stray close brace reports its line" );
		const auto quote = codec.Decode( "world\n{\n\t\"id\" \"1\n}\n", 1 );
		checks.That( !quote && quote.Error().line == 3, "an unterminated string reports its line" );
		const auto json = codec.Decode( "{\"world\": {\"id\": 1}}", 1 );
		checks.That( !json && json.Error().line == 1, "JSON is rejected" );
		const auto words = codec.Decode( "hello world", 1 );
		checks.That( !words, "loose key/value text is rejected" );
		const auto empty = codec.Decode( "", 1 );
		checks.That( empty && empty.Value().document.ObjectCount() == 0 &&
		                 empty.Value().document.Settings().WorldKey( "mapversion" ),
		    "empty text is an empty map with the default world" );
	}

	// --- Negative: content VMF cannot hold fails Encode --------------------------------------
	{
		MapDocument doc( 6 );
		Entity e;
		e.id = doc.AllocateId();
		e.vmfId = doc.AllocateVmfId();
		e.classname = "info_target";
		e.keys = { { "message", "say \"hi\"" } };
		doc.Put( e );
		checks.That( EncodeFails( doc, "holds a '\"'" ), "a quote in a value fails Encode" );

		MapDocument named( 6 );
		Entity shadow = e;
		shadow.id = named.AllocateId();
		shadow.keys = { { "classname", "light" } };
		named.Put( shadow );
		checks.That( EncodeFails( named, "holds a 'classname' key" ),
		    "a second classname among the keys fails Encode" );

		MapDocument owners( 6 );
		Solid orphan = MakeSolid( owners, Box{ Vec3d( 0, 0, 0 ), Vec3d( 8, 8, 8 ) }, "A" );
		orphan.owner = owners.AllocateId();
		owners.Put( orphan );
		checks.That( EncodeFails( owners, "owned by an entity that is not in the document" ),
		    "a solid owned by a missing entity fails Encode" );

		MapDocument groups( 6 );
		Solid grouped = MakeSolid( groups, Box{ Vec3d( 0, 0, 0 ), Vec3d( 8, 8, 8 ) }, "A" );
		grouped.group = groups.AllocateId();
		groups.Put( grouped );
		checks.That( EncodeFails( groups, "refers to a group" ), "a missing group fails Encode" );

		MapDocument nan( 6 );
		Solid bad = MakeSolid( nan, Box{ Vec3d( 0, 0, 0 ), Vec3d( 8, 8, 8 ) }, "A" );
		bad.sides[0].points[0].x = std::numeric_limits<double>::quiet_NaN();
		nan.Put( bad );
		checks.That( EncodeFails( nan, "not finite" ), "a NaN coordinate fails Encode" );

		MapDocument conn( 6 );
		Entity relay;
		relay.id = conn.AllocateId();
		relay.vmfId = conn.AllocateVmfId();
		relay.classname = "logic_relay";
		relay.connections.push_back( { "OnTrigger", "t", "In", "a,b", 0, -1, ',' } );
		conn.Put( relay );
		checks.That( EncodeFails( conn, "cannot be written" ),
		    "a comma parameter with the comma separator fails Encode" );

		MapDocument disp( 6 );
		Solid displaced = MakeSolid( disp, Box{ Vec3d( 0, 0, 0 ), Vec3d( 8, 8, 8 ) }, "A" );
		Displacement short_ = MakeDisplacement();
		short_.distances->pop_back();
		displaced.sides[0].displacement = short_;
		disp.Put( displaced );
		checks.That(
		    EncodeFails( disp, "displacement distances has 24 entries; its power needs 25" ),
		    "a displacement array that does not match its power fails Encode" );
		displaced.sides[0].displacement->power = 7;
		disp.Put( displaced );
		checks.That(
		    EncodeFails( disp, "power 7" ), "an unsupported displacement power fails Encode" );
	}

	// --- Repository sample maps -----------------------------------------------------------------
	{
		const std::filesystem::path samples = RepoRoot() / "hammer" / "gtk" / "samples";
		std::vector<std::filesystem::path> files;
		if ( std::filesystem::is_directory( samples ) )
		{
			for ( const auto &entry : std::filesystem::directory_iterator( samples ) )
			{
				if ( entry.path().extension() == ".vmf" )
				{
					files.push_back( entry.path() );
				}
			}
		}
		std::sort( files.begin(), files.end() );
		checks.That( files.size() >= 3, "the repository sample maps are found" );
		for ( const std::filesystem::path &file : files )
		{
			const std::string text = ReadFile( file );
			Report( checks, map_codec_oracle::RoundTrip( codec, text, 11 ),
			    "sample " + file.filename().string() + " round trip" );
			if ( file.filename() == "displacement.vmf" )
			{
				const auto decoded = codec.Decode( text, 11 );
				const Solid *s =
				    decoded ? map_codec_oracle::ByVmfId( decoded.Value().document.Solids(), 2 )
				            : nullptr;
				checks.That( s && s->sides[0].displacement && s->sides[0].displacement->distances &&
				                 ( *s->sides[0].displacement->distances )[6] == 28.1 &&
				                 !s->sides[0].displacement->offsets,
				    "sample displacement.vmf decodes its typed displacement" );
			}
		}
	}

	// --- Optional corpus (never part of the required run) ---------------------------------------
	if ( const char *corpus = std::getenv( "HAMMER_VMF_CORPUS" ) )
	{
		for ( const auto &entry : std::filesystem::directory_iterator( corpus ) )
		{
			if ( entry.path().extension() != ".vmf" )
			{
				continue;
			}
			const std::string text = ReadFile( entry.path() );
			Report( checks, map_codec_oracle::RoundTrip( codec, text, 12 ),
			    "corpus " + entry.path().filename().string() + " round trip" );
			Report( checks,
			    map_codec_oracle::SemanticText(
			        codec, text, map_codec_oracle::NumberText::ByValue ),
			    "corpus " + entry.path().filename().string() + " semantic" );
		}
	}

	return checks.Report();
}
