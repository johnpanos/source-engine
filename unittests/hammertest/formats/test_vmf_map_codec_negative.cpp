//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity suite for the IMapCodec conformance oracle (RFC 0002,
//			ports.map_codec.v1 / formats.vmf_map_codec.v1). Each seeded bad
//			codec wraps the real VmfMapCodec and breaks one promise (drops
//			connections, regenerates texture axes, loses hidden flags,
//			renumbers side ids, rounds coordinates, flattens brush entities,
//			drops verbatim content, ignores the serial, sorts keys, drops group
//			membership, issues unstable runtime ids, drops warnings, reuses
//			side ids, drops displacements). Every one must be DETECTED by the
//			same oracle clause the positive suite passes with the real codec;
//			the real codec is run through each clause as the control.
//
//=============================================================================//

#include "hammer/formats/vmf_map_codec.h"
#include "hammer/scene/solid_geometry.h"
#include "map_codec_oracle.h"
#include "testing/checks.h"

#include <algorithm>
#include <functional>
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

// Applies 'edit' to a copy of every object (Put replaces by id).
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

using Clause = std::function<map_codec_oracle::Findings( const IMapCodec & )>;

} // namespace

int main()
{
	testing::Checks checks;
	namespace oracle = map_codec_oracle;

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
	    { "OddityFixture",
	        []( const IMapCodec &c )
	        {
		        return oracle::OddityFixture( c );
	        } },
	    { "DocumentRoundTrip",
	        []( const IMapCodec &c )
	        {
		        return oracle::DocumentRoundTrip( c, FractionalDocument() );
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
		BadCodec codec;
		std::vector<std::string> detectedBy;
	};
	std::vector<Mutant> mutants;

	mutants.push_back( { "drops connections",
	    OnDecode(
	        []( const MapDocument &d )
	        {
		        return EditObjects( d, {},
		            []( Entity &e )
		            {
			            e.connections.clear();
		            } );
	        } ),
	    { "CanonicalFixture", "CanonicalText" } } );

	mutants.push_back( { "regenerates texture axes",
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
	    { "RoundTrip", "CanonicalText" } } );

	mutants.push_back( { "loses hidden flags",
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
	    { "CanonicalFixture", "CanonicalText" } } );

	mutants.push_back( { "renumbers side ids",
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
	    { "CanonicalFixture", "CanonicalText", "OddityFixture" } } );

	mutants.push_back( { "rounds coordinates to float",
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
	    { "DocumentRoundTrip", "RoundTrip" } } );

	mutants.push_back( { "flattens brush entities into the world",
	    OnDecode(
	        []( const MapDocument &d )
	        {
		        return EditObjects( d,
		            []( Solid &s )
		            {
			            s.owner = ObjectId();
		            } );
	        } ),
	    { "CanonicalFixture", "CanonicalText" } } );

	mutants.push_back( { "drops content it does not model",
	    OnDecode(
	        []( const MapDocument &d )
	        {
		        MapDocument out = EditObjects(
		            d,
		            []( Solid &s )
		            {
			            s.extraChildren.clear();
		            },
		            []( Entity &e )
		            {
			            e.extraChildren.clear();
		            } );
		        out.MutableSettings().unknownBlocks.clear();
		        out.MutableSettings().worldExtraChildren.clear();
		        return out;
	        } ),
	    { "CanonicalFixture", "CanonicalText", "OddityFixture" } } );

	mutants.push_back( { "ignores the requested serial", BadCodec( {}, {}, 1 ), { "RoundTrip" } } );

	mutants.push_back( { "sorts entity keys",
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
	    { "CanonicalFixture" } } );

	mutants.push_back( { "drops group membership",
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
	    { "CanonicalFixture", "CanonicalText" } } );

	mutants.push_back(
	    { "issues runtime ids in reverse order", OnDecode( ReverseIds ), { "RoundTrip" } } );

	mutants.push_back( { "drops warnings",
	    BadCodec(
	        []( DecodedMap &m )
	        {
		        m.warnings.clear();
	        },
	        {} ),
	    { "OddityFixture" } } );

	mutants.push_back( { "reuses side ids across solids",
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
	    { "RoundTrip", "CanonicalFixture" } } );

	mutants.push_back( { "drops displacements on save",
	    OnEncode(
	        []( const MapDocument &d )
	        {
		        return EditObjects( d,
		            []( Solid &s )
		            {
			            for ( Side &side : s.sides )
			            {
				            side.dispinfo.reset();
			            }
		            } );
	        } ),
	    { "RoundTrip", "CanonicalText" } } );

	for ( const Mutant &mutant : mutants )
	{
		for ( const std::string &name : mutant.detectedBy )
		{
			const oracle::Findings f = clauses.at( name )( mutant.codec );
			checks.That( !f.Ok(), "mutant '" + mutant.name + "' is detected by " + name );
		}
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
