//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity suite for the IMapCodec conformance oracle (RFC 0002,
//			ports.map_codec.v1 / formats.vmf_map_codec.v1). Each seeded bad
//			codec wraps the real VmfMapCodec and breaks one promise: it drops
//			connections, regenerates texture axes, loses hidden flags,
//			renumbers side ids, rounds coordinates, flattens brush entities,
//			ignores the serial, sorts keys, drops group membership, issues
//			unstable runtime ids, drops warnings, reuses side ids, drops or
//			shifts displacement data, drops logical positions and comments,
//			drops version and view settings, drops editor colors, or silently
//			drops content the model cannot hold instead of rejecting it. Every
//			one must be DETECTED by the oracle clause named for it; the real
//			codec passes every clause as the control.
//
//=============================================================================//

#include "hammer/formats/vmf_map_codec.h"
#include "hammer/scene/solid_geometry.h"
#include "map_codec_oracle.h"
#include "testing/checks.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <map>
#include <string>
#include <vector>

using namespace hammer::scene;
using hammer::formats::VmfMapCodec;
using hammer::ports::CodecError;
using hammer::ports::DecodedMap;
using hammer::ports::IMapCodec;
using mapgeometry::Vec3d;

namespace
{

// Applies the edits to a copy of every object (Put replaces by id).
MapDocument EditObjects( const MapDocument &doc, const std::function<void( Solid & )> &solid,
    const std::function<void( Entity & )> &entity = {},
    const std::function<void( Group & )> &group = {} )
{
	MapDocument out = doc;
	for ( const auto &[id, s] : doc.Solids() )
	{
		Solid copy = s;
		if ( solid )
		{
			solid( copy );
		}
		out.Put( std::move( copy ) );
	}
	for ( const auto &[id, e] : doc.Entities() )
	{
		Entity copy = e;
		if ( entity )
		{
			entity( copy );
		}
		out.Put( std::move( copy ) );
	}
	for ( const auto &[id, g] : doc.Groups() )
	{
		Group copy = g;
		if ( group )
		{
			group( copy );
		}
		out.Put( std::move( copy ) );
	}
	return out;
}

// Applies 'edit' to every displacement.
MapDocument EditDisplacements(
    const MapDocument &doc, const std::function<void( Displacement & )> &edit )
{
	return EditObjects( doc,
	    [&]( Solid &s )
	    {
		    for ( Side &side : s.sides )
		    {
			    if ( side.displacement )
			    {
				    edit( *side.displacement );
			    }
		    }
	    } );
}

// A codec that behaves like the real one except for a seeded defect.
class BadCodec final : public IMapCodec
{
public:
	using DecodeHook = std::function<void( DecodedMap & )>;
	using EncodeHook = std::function<MapDocument( const MapDocument & )>;

	BadCodec( DecodeHook afterDecode, EncodeHook beforeEncode, std::uint32_t forcedSerial = 0 )
	    : m_afterDecode( std::move( afterDecode ) ), m_beforeEncode( std::move( beforeEncode ) ),
	      m_forcedSerial( forcedSerial )
	{
	}

	std::string_view FormatName() const override { return "vmf-mutant"; }

	foundation::Expected<DecodedMap, CodecError> Decode(
	    std::string_view text, std::uint32_t serial ) const override
	{
		auto result = m_real.Decode( text, m_forcedSerial ? m_forcedSerial : serial );
		if ( !result || !m_afterDecode )
		{
			return result;
		}
		DecodedMap decoded = std::move( result ).Value();
		m_afterDecode( decoded );
		return decoded;
	}

	foundation::Expected<std::string, CodecError> Encode( const MapDocument &doc ) const override
	{
		return m_beforeEncode ? m_real.Encode( m_beforeEncode( doc ) ) : m_real.Encode( doc );
	}

private:
	VmfMapCodec m_real;
	DecodeHook m_afterDecode;
	EncodeHook m_beforeEncode;
	std::uint32_t m_forcedSerial;
};

void RemoveBlocks( kvtext::KeyValueNode &node, const std::string &name )
{
	std::erase_if( node.children,
	    [&]( const kvtext::KeyValueNode &child )
	    {
		    return child.name == name;
	    } );
	for ( kvtext::KeyValueNode &child : node.children )
	{
		RemoveBlocks( child, name );
	}
}

// The escape-hatch codec: instead of rejecting content the model cannot hold,
// it silently strips whatever the strict decoder names (the block, or the line
// of the offending key) and decodes what is left.
class LenientCodec final : public IMapCodec
{
public:
	std::string_view FormatName() const override { return "vmf-lenient"; }

	foundation::Expected<DecodedMap, CodecError> Decode(
	    std::string_view text, std::uint32_t serial ) const override
	{
		std::string current( text );
		for ( int attempt = 0;; ++attempt )
		{
			auto result = m_real.Decode( current, serial );
			const std::string message = result ? std::string() : result.Error().message;
			if ( result || attempt == 64 || message.rfind( "not VMF text", 0 ) == 0 )
			{
				return result;
			}
			const std::size_t block = message.find( "block '" );
			if ( message.find( "unknown" ) != std::string::npos && block != std::string::npos )
			{
				const std::size_t start = block + 7;
				const std::string name =
				    message.substr( start, message.find( '\'', start ) - start );
				kvtext::ParseResult parsed = kvtext::ParseKeyValues( current );
				RemoveBlocks( parsed.root, name );
				current = kvtext::WriteKeyValues( parsed.root );
				continue;
			}
			const int line = result.Error().line;
			if ( line <= 0 )
			{
				return result;
			}
			std::size_t bol = 0;
			for ( int l = 1; l < line && bol != std::string::npos; ++l )
			{
				bol = current.find( '\n', bol );
				bol = bol == std::string::npos ? bol : bol + 1;
			}
			if ( bol == std::string::npos )
			{
				return result;
			}
			const std::size_t eol = current.find( '\n', bol );
			current.erase( bol, eol == std::string::npos ? std::string::npos : eol - bol + 1 );
		}
	}

	foundation::Expected<std::string, CodecError> Encode( const MapDocument &doc ) const override
	{
		return m_real.Encode( doc );
	}

private:
	VmfMapCodec m_real;
};

BadCodec OnDecode( std::function<MapDocument( const MapDocument & )> edit )
{
	return BadCodec(
	    [edit]( DecodedMap &m )
	    {
		    m.document = edit( m.document );
	    },
	    {} );
}

BadCodec OnEncode( std::function<MapDocument( const MapDocument & )> edit )
{
	return BadCodec( {}, std::move( edit ) );
}

// Re-issues every runtime id in reverse id order (content unchanged).
MapDocument ReverseIds( const MapDocument &doc )
{
	MapDocument out( doc.Serial() );
	out.MutableSettings() = doc.Settings();
	std::vector<ObjectId> all;
	for ( const ObjectId id : doc.SolidIds() )
		all.push_back( id );
	for ( const ObjectId id : doc.EntityIds() )
		all.push_back( id );
	for ( const ObjectId id : doc.GroupIds() )
		all.push_back( id );
	std::sort( all.rbegin(), all.rend() );
	std::map<ObjectId, ObjectId> renamed;
	for ( const ObjectId id : all )
	{
		renamed[id] = out.AllocateId();
	}
	auto map = [&]( ObjectId id )
	{
		return id.IsValid() ? renamed[id] : id;
	};
	for ( const auto &[id, s] : doc.Solids() )
	{
		Solid copy = s;
		copy.id = map( s.id );
		copy.owner = map( s.owner );
		copy.group = map( s.group );
		out.Put( std::move( copy ) );
	}
	for ( const auto &[id, e] : doc.Entities() )
	{
		Entity copy = e;
		copy.id = map( e.id );
		copy.group = map( e.group );
		out.Put( std::move( copy ) );
	}
	for ( const auto &[id, g] : doc.Groups() )
	{
		Group copy = g;
		copy.id = map( g.id );
		copy.group = map( g.group );
		out.Put( std::move( copy ) );
	}
	return out;
}

// An edited document holding values %g cannot keep (for the rounding mutant).
MapDocument FractionalDocument()
{
	MapDocument doc( 3 );
	FaceTexture tex;
	tex.material = "A";
	Solid s = MakeBoxSolid( Box{ Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 64.0 / 3.0 ) }, tex );
	s.id = doc.AllocateId();
	s.vmfId = doc.AllocateVmfId();
	for ( Side &side : s.sides )
	{
		side.vmfId = doc.AllocateVmfId();
	}
	doc.Put( s );
	return doc;
}

std::filesystem::path RepoRoot()
{
	if ( std::filesystem::is_directory( "unittests/hammertest/fixtures/portal2" ) )
	{
		return std::filesystem::current_path();
	}
	return std::filesystem::path( __FILE__ )
	    .parent_path()
	    .parent_path()
	    .parent_path()
	    .parent_path();
}

// The real-map clause: a vendored Portal 2 map saved by 'codec' is the
// original semantically and by structural counts.
map_codec_oracle::Findings Portal2Semantics( const IMapCodec &codec, const std::string &original )
{
	map_codec_oracle::Findings f;
	const auto decoded = codec.Decode( original, 1 );
	if ( !f.Expect( decoded.HasValue(), "the Portal 2 map decodes" ) )
	{
		return f;
	}
	const auto encoded = codec.Encode( decoded.Value().document );
	if ( !f.Expect( encoded.HasValue(), "the Portal 2 map encodes" ) )
	{
		return f;
	}
	const kvtext::ParseResult source = kvtext::ParseKeyValues( original );
	const kvtext::ParseResult saved = kvtext::ParseKeyValues( encoded.Value() );
	const kvtext::CompareResult same =
	    map_codec_oracle::CompareFileSemantics( source.root, saved.root );
	f.Expect( same.equal, "saved Portal 2 map is the original: " + same.firstDivergence );
	f.Expect( map_codec_oracle::CountText( source.root ) ==
	              map_codec_oracle::CountDocument( decoded.Value().document ),
	    "Portal 2 counts match" );
	return f;
}

using Clause = std::function<map_codec_oracle::Findings( const IMapCodec & )>;

} // namespace

int main()
{
	testing::Checks checks;
	namespace oracle = map_codec_oracle;

	std::ifstream zooFile(
	    RepoRoot() / "unittests/hammertest/fixtures/portal2/zoo_mechanics.vmf", std::ios::binary );
	const std::string zoo(
	    std::istreambuf_iterator<char>{ zooFile }, std::istreambuf_iterator<char>{} );
	checks.That( !zoo.empty(), "the vendored Portal 2 map is readable" );

	const std::map<std::string, Clause> clauses = {
	    { "RoundTrip",
	        []( const IMapCodec &c )
	        {
		        return oracle::RoundTrip( c, oracle::CanonicalVmf() );
	        } },
	    { "CanonicalText",
	        []( const IMapCodec &c )
	        {
		        return oracle::CanonicalText( c, oracle::CanonicalVmf() );
	        } },
	    { "CanonicalFixture",
	        []( const IMapCodec &c )
	        {
		        return oracle::CanonicalFixture( c );
	        } },
	    { "MissingIdsFixture",
	        []( const IMapCodec &c )
	        {
		        return oracle::MissingIdsFixture( c );
	        } },
	    { "RejectsUnmodeled",
	        []( const IMapCodec &c )
	        {
		        return oracle::RejectsUnmodeled( c );
	        } },
	    { "DocumentRoundTrip",
	        []( const IMapCodec &c )
	        {
		        return oracle::DocumentRoundTrip( c, FractionalDocument() );
	        } },
	    { "Portal2Semantics",
	        [&zoo]( const IMapCodec &c )
	        {
		        return Portal2Semantics( c, zoo );
	        } },
	};

	// Control: the real codec passes every clause used below.
	const VmfMapCodec real;
	for ( const auto &[name, clause] : clauses )
	{
		const oracle::Findings f = clause( real );
		checks.That( f.Ok(),
		    "control: the real codec passes " + name + ( f.Ok() ? "" : ": " + f.Summary() ) );
	}

	struct Mutant
	{
		std::string name;
		std::function<oracle::Findings( const Clause & )> run;
		std::vector<std::string> detectedBy;
	};
	std::vector<Mutant> mutants;
	auto add = [&]( const std::string &name, BadCodec codec, std::vector<std::string> by )
	{
		auto shared = std::make_shared<BadCodec>( std::move( codec ) );
		mutants.push_back( { name,
		    [shared]( const Clause &c )
		    {
			    return c( *shared );
		    },
		    std::move( by ) } );
	};

	add( "drops connections",
	    OnDecode(
	        []( const MapDocument &d )
	        {
		        return EditObjects( d, {},
		            []( Entity &e )
		            {
			            e.connections.clear();
		            } );
	        } ),
	    { "CanonicalFixture", "CanonicalText", "Portal2Semantics" } );
	add( "regenerates texture axes",
	    OnEncode(
	        []( const MapDocument &d )
	        {
		        return EditObjects( d,
		            []( Solid &s )
		            {
			            for ( Side &side : s.sides )
			            {
				            side.texture = WorldAlignedTexture( side.texture, side.Plane().normal );
			            }
		            } );
	        } ),
	    { "RoundTrip", "CanonicalText" } );
	add( "loses hidden flags",
	    OnDecode(
	        []( const MapDocument &d )
	        {
		        return EditObjects(
		            d,
		            []( Solid &s )
		            {
			            s.hidden = false;
		            },
		            []( Entity &e )
		            {
			            e.hidden = false;
		            },
		            []( Group &g )
		            {
			            g.hidden = false;
		            } );
	        } ),
	    { "CanonicalFixture", "CanonicalText", "Portal2Semantics" } );
	add( "renumbers side ids",
	    OnDecode(
	        []( const MapDocument &d )
	        {
		        std::uint32_t next = 5000;
		        return EditObjects( d,
		            [&next]( Solid &s )
		            {
			            for ( Side &side : s.sides )
			            {
				            side.vmfId = next++;
			            }
		            } );
	        } ),
	    { "CanonicalFixture", "CanonicalText", "MissingIdsFixture", "Portal2Semantics" } );
	add( "rounds coordinates to float",
	    OnEncode(
	        []( const MapDocument &d )
	        {
		        return EditObjects( d,
		            []( Solid &s )
		            {
			            for ( Side &side : s.sides )
			            {
				            for ( Vec3d &p : side.points )
				            {
					            p = Vec3d( static_cast<float>( p.x ), static_cast<float>( p.y ),
					                static_cast<float>( p.z ) );
				            }
			            }
		            } );
	        } ),
	    { "DocumentRoundTrip", "RoundTrip" } );
	add( "flattens brush entities into the world",
	    OnDecode(
	        []( const MapDocument &d )
	        {
		        return EditObjects( d,
		            []( Solid &s )
		            {
			            s.owner = ObjectId();
		            } );
	        } ),
	    { "CanonicalFixture", "CanonicalText", "Portal2Semantics" } );
	add( "ignores the requested serial", BadCodec( {}, {}, 1 ), { "RoundTrip" } );
	add( "sorts entity keys",
	    OnDecode(
	        []( const MapDocument &d )
	        {
		        return EditObjects( d, {},
		            []( Entity &e )
		            {
			            std::stable_sort( e.keys.begin(), e.keys.end(),
			                []( const kvtext::KeyValue &a, const kvtext::KeyValue &b )
			                {
				                return a.key < b.key;
			                } );
		            } );
	        } ),
	    { "CanonicalFixture" } );
	add( "drops group membership",
	    OnDecode(
	        []( const MapDocument &d )
	        {
		        return EditObjects(
		            d,
		            []( Solid &s )
		            {
			            s.group = ObjectId();
		            },
		            []( Entity &e )
		            {
			            e.group = ObjectId();
		            },
		            []( Group &g )
		            {
			            g.group = ObjectId();
		            } );
	        } ),
	    { "CanonicalFixture", "CanonicalText" } );
	add( "issues runtime ids in reverse order", OnDecode( ReverseIds ), { "RoundTrip" } );
	add( "drops warnings",
	    BadCodec(
	        []( DecodedMap &m )
	        {
		        m.warnings.clear();
	        },
	        {} ),
	    { "MissingIdsFixture" } );
	add( "reuses side ids across solids",
	    OnDecode(
	        []( const MapDocument &d )
	        {
		        return EditObjects( d,
		            []( Solid &s )
		            {
			            std::uint32_t next = 1;
			            for ( Side &side : s.sides )
			            {
				            side.vmfId = next++;
			            }
		            } );
	        } ),
	    { "RoundTrip", "CanonicalFixture" } );
	add( "drops displacements on save",
	    OnEncode(
	        []( const MapDocument &d )
	        {
		        return EditObjects( d,
		            []( Solid &s )
		            {
			            for ( Side &side : s.sides )
			            {
				            side.displacement.reset();
			            }
		            } );
	        } ),
	    { "RoundTrip", "CanonicalText" } );
	add( "loses displacement rows",
	    OnDecode(
	        []( const MapDocument &d )
	        {
		        return EditDisplacements( d,
		            []( Displacement &disp )
		            {
			            disp.offsets.reset();
			            disp.alphas.reset();
		            } );
	        } ),
	    { "CanonicalFixture", "CanonicalText" } );
	add( "writes displacement values shifted by one",
	    OnEncode(
	        []( const MapDocument &d )
	        {
		        return EditDisplacements( d,
		            []( Displacement &disp )
		            {
			            if ( disp.distances )
			            {
				            std::rotate( disp.distances->begin(), disp.distances->begin() + 1,
				                disp.distances->end() );
			            }
		            } );
	        } ),
	    { "RoundTrip", "CanonicalText" } );
	add( "drops the pre-release displacement keys",
	    OnDecode(
	        []( const MapDocument &d )
	        {
		        return EditDisplacements( d,
		            []( Displacement &disp )
		            {
			            disp.minTess.reset();
			            disp.smoothingAngle.reset();
			            disp.cornerAlphas.reset();
		            } );
	        } ),
	    { "CanonicalFixture", "CanonicalText" } );
	add( "drops logical positions and comments",
	    OnDecode(
	        []( const MapDocument &d )
	        {
		        return EditObjects( d, {},
		            []( Entity &e )
		            {
			            e.editor.logicalPos.reset();
			            e.editor.comments.reset();
		            } );
	        } ),
	    { "CanonicalFixture", "CanonicalText", "Portal2Semantics" } );
	add( "drops version and view settings",
	    OnDecode(
	        []( const MapDocument &d )
	        {
		        MapDocument out = d;
		        out.MutableSettings().version = VersionInfo{};
		        out.MutableSettings().view = ViewSettings{};
		        return out;
	        } ),
	    { "CanonicalFixture", "CanonicalText", "Portal2Semantics" } );
	add( "drops editor colors",
	    OnDecode(
	        []( const MapDocument &d )
	        {
		        return EditObjects(
		            d,
		            []( Solid &s )
		            {
			            s.editor.color.reset();
		            },
		            []( Entity &e )
		            {
			            e.editor.color.reset();
		            },
		            []( Group &g )
		            {
			            g.editor.color.reset();
		            } );
	        } ),
	    { "CanonicalFixture", "CanonicalText", "Portal2Semantics" } );
	{
		auto lenient = std::make_shared<LenientCodec>();
		mutants.push_back( { "silently drops content the model cannot hold",
		    [lenient]( const Clause &c )
		    {
			    return c( *lenient );
		    },
		    { "RejectsUnmodeled" } } );
	}

	for ( const Mutant &mutant : mutants )
	{
		for ( const std::string &name : mutant.detectedBy )
		{
			const oracle::Findings f = mutant.run( clauses.at( name ) );
			checks.That( !f.Ok(), "mutant '" + mutant.name + "' is detected by " + name );
		}
	}

	// The lenient codec really accepts the unmodeled content: detection comes
	// from the rejection cases, not from a crash or a parse failure.
	{
		const LenientCodec lenient;
		std::size_t accepted = 0;
		const std::vector<oracle::RejectionCase> cases = oracle::RejectionCases();
		for ( const oracle::RejectionCase &c : cases )
		{
			accepted += lenient.Decode( c.text, 1 ).HasValue();
		}
		checks.That( accepted * 2 > cases.size(),
		    "the lenient codec accepts most rejection cases (" + std::to_string( accepted ) +
		        " of " + std::to_string( cases.size() ) + ")" );
	}

	// The comparator itself: renamed runtime ids are the same content; one changed
	// field is not.
	{
		const auto decoded = real.Decode( oracle::CanonicalVmf(), 4 );
		checks.That( decoded.HasValue(), "canonical fixture decodes for the comparator checks" );
		if ( decoded )
		{
			const MapDocument &doc = decoded.Value().document;
			std::string why;
			checks.That( oracle::SameContentUpToIds( doc, ReverseIds( doc ), why ),
			    "renamed runtime ids compare equal up to ids" );
			checks.That(
			    !SameContent( doc, ReverseIds( doc ) ), "but not under exact SameContent" );
			const MapDocument changed = EditObjects( doc,
			    []( Solid &s )
			    {
				    if ( s.vmfId == 20 )
				    {
					    s.sides[3].texture.material = "OTHER";
				    }
			    } );
			checks.That( !oracle::SameContentUpToIds( doc, changed, why ),
			    "one changed material is detected" );
			const MapDocument regrouped = EditObjects( doc,
			    []( Solid &s )
			    {
				    s.group = ObjectId();
			    } );
			checks.That( !oracle::SameContentUpToIds( doc, regrouped, why ),
			    "a dropped group reference is detected" );
			MapDocument moved = doc;
			moved.MutableSettings().cameras.clear();
			checks.That(
			    !oracle::SameContentUpToIds( doc, moved, why ), "a settings change is detected" );
		}
	}

	return checks.Report();
}
