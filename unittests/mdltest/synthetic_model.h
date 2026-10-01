//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Test-only writer of synthetic studio models (MDL + VVD + VTX
//			bytes) for content.studio-model and its consumers. It writes the
//			layouts public/mdl/studio_model.h documents (the engine's on-disk
//			records), independently of the reader: a model is described as
//			body parts, models and meshes whose triangles are given in the
//			files' order (clockwise seen from outside). Bones, per-vertex
//			weights, animations (their frame-0 bytes, encoded by the helpers
//			below: RLE records or frame x bone data, in the MDL, a section or
//			an .ani block, or zero-frame data) and sequences are written as
//			the reader's header documents them.
//
//=============================================================================//

#ifndef MDLTEST_SYNTHETIC_MODEL_H
#define MDLTEST_SYNTHETIC_MODEL_H

#include "mdl/studio_model.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

namespace mdltest
{

struct SyntheticMesh
{
	std::int32_t textureRef = 0;
	std::vector<mdl::Vertex> vertices;
	std::vector<mdl::BoneWeights> weights; // empty: every vertex bone 0, weight 1
	std::vector<std::uint16_t> stored; // as the file stores them: clockwise from outside
	bool tristrip = false;             // 'stored' is one triangle strip
	// Optional lower-LOD topology, into this mesh's original vertices. An
	// explicit empty list removes its geometry at that level.
	std::vector<std::vector<std::uint16_t>> lodIndices;
};

using SyntheticSubModel = std::vector<SyntheticMesh>; // one mstudiomodel_t

struct SyntheticBone
{
	std::string name = "root";
	std::int32_t parent = -1;
	mdl::Float3 position;
	mdl::Quaternion rotation;
	mdl::Float3 euler;
	mdl::Float3 positionScale;
	mdl::Float3 rotationScale;
	mdl::Matrix3x4 poseToBone;
	std::uint32_t flags = 0;
};

struct SyntheticAnimation
{
	std::uint32_t flags = 0; // animdesc flags: 0x04 delta, 0x40 frame x bone
	std::string data;        // frame 0's encoded data (see Records / FrameAnim)
	std::int32_t block = 0;  // 0: in the MDL; n > 0: .ani block n; -1: no block
	bool sectioned = false;  // reached through section 0 (sectionframes 30)
	std::string zeroFrame;   // zero-frame data (count 1); empty: none
};

struct SyntheticSequence
{
	std::string label = "idle";
	std::uint32_t flags = 0;
	mdl::Float3 bbMin;
	mdl::Float3 bbMax;
	std::vector<std::int16_t> blends = { 0 }; // the blend grid's animations (groupsize n x 1)
	std::vector<float> weights;               // per bone; empty: 1 for every bone
};

struct SyntheticModel
{
	std::int32_t version = 49;
	std::int32_t checksum = 0x5eed1234;
	std::string name = "synthetic/box.mdl";
	std::vector<std::string> textures;
	std::vector<std::string> cdMaterials;
	std::vector<std::vector<std::int16_t>> skins; // [family][ref]
	std::vector<std::vector<SyntheticSubModel>> bodyParts;
	bool fixups = false; // store the VVD vertices rotated, restored by two fixups
	std::uint32_t flags = 0;
	std::vector<SyntheticBone> bones; // empty: one identity root
	std::vector<SyntheticAnimation> animations;
	std::vector<SyntheticSequence> sequences;
	std::string aniName = "synthetic/box.ani";
	std::vector<std::string> includes; // $includemodel paths
	std::vector<float> lodSwitches = { 0.0f };
	// [LOD] replacements as (Studio texture slot, replacement name).
	std::vector<std::vector<std::pair<std::uint16_t, std::string>>> lodReplacements;
};

struct SyntheticFiles
{
	std::string mdl;
	std::string vvd;
	std::string vtx;
	std::string ani; // written when an animation lives in a block
};

namespace detail
{

inline void Put32( std::string &out, std::size_t at, std::int32_t value )
{
	const auto u = static_cast<std::uint32_t>( value );
	for ( int i = 0; i < 4; ++i )
	{
		out[at + i] = static_cast<char>( ( u >> ( 8 * i ) ) & 0xff );
	}
}

inline void PutF( std::string &out, std::size_t at, float value )
{
	std::uint32_t bits = 0;
	std::memcpy( &bits, &value, 4 );
	Put32( out, at, static_cast<std::int32_t>( bits ) );
}

inline void Put16( std::string &out, std::size_t at, std::uint16_t value )
{
	out[at] = static_cast<char>( value & 0xff );
	out[at + 1] = static_cast<char>( value >> 8 );
}

inline std::size_t Grow( std::string &out, std::size_t bytes )
{
	const std::size_t at = out.size();
	out.append( bytes, '\0' );
	return at;
}

inline std::size_t AppendString( std::string &out, const std::string &text )
{
	const std::size_t at = out.size();
	out += text;
	out.push_back( '\0' );
	return at;
}

} // namespace detail

// --- Frame-0 encoders (compressed_vector.h and studio.h layouts) --------------------------

inline std::string Bytes16( std::initializer_list<std::uint16_t> words )
{
	std::string out;
	for ( std::uint16_t w : words )
	{
		out.push_back( static_cast<char>( w & 0xff ) );
		out.push_back( static_cast<char>( w >> 8 ) );
	}
	return out;
}

// float16 of 'value' (normal range, rounded toward zero).
inline std::uint16_t Half( float value )
{
	std::uint32_t bits = 0;
	std::memcpy( &bits, &value, 4 );
	const std::uint16_t sign = static_cast<std::uint16_t>( ( bits >> 16 ) & 0x8000 );
	const int exponent = int( ( bits >> 23 ) & 0xff ) - 127 + 15;
	if ( value == 0.0f || exponent <= 0 )
	{
		return sign;
	}
	return static_cast<std::uint16_t>(
	    sign | ( std::min( exponent, 30 ) << 10 ) | ( ( bits >> 13 ) & 0x3ff ) );
}

inline std::string Vector48( mdl::Float3 p )
{
	return Bytes16( { Half( p.x ), Half( p.y ), Half( p.z ) } );
}

inline std::string Quaternion48( mdl::Quaternion q )
{
	const auto x = static_cast<std::uint16_t>(
	    std::clamp<long>( std::lround( q.x * 32768.0f ) + 32768, 0, 65535 ) );
	const auto y = static_cast<std::uint16_t>(
	    std::clamp<long>( std::lround( q.y * 32768.0f ) + 32768, 0, 65535 ) );
	const auto z = static_cast<std::uint16_t>(
	    std::clamp<long>( std::lround( q.z * 16384.0f ) + 16384, 0, 32767 ) );
	return Bytes16(
	    { x, y, static_cast<std::uint16_t>( ( z & 0x7fff ) | ( q.w < 0 ? 0x8000 : 0 ) ) } );
}

inline std::string Quaternion64( mdl::Quaternion q )
{
	const auto part = []( float v )
	{
		return static_cast<std::uint64_t>( std::lround( v * 1048576.0f ) + 1048576 ) & 0x1fffff;
	};
	const std::uint64_t bits = part( q.x ) | ( part( q.y ) << 21 ) | ( part( q.z ) << 42 ) |
	                           ( std::uint64_t( q.w < 0 ? 1 : 0 ) << 63 );
	std::string out;
	for ( int i = 0; i < 8; ++i )
	{
		out.push_back( static_cast<char>( ( bits >> ( 8 * i ) ) & 0xff ) );
	}
	return out;
}

// Quaternion48S: components 'first', first+1 and first+2 (cyclic) stored, the
// fourth (its sign in the last bit) rebuilt.
inline std::string Quaternion48S( mdl::Quaternion q, int first )
{
	const float c[4] = { q.x, q.y, q.z, q.w };
	const auto part = []( float v )
	{
		return static_cast<std::uint16_t>( std::lround( v * 23168.0f ) + 16384 ) & 0x7fff;
	};
	const std::uint16_t a = part( c[first] ) | ( ( first >> 1 ) << 15 );
	const std::uint16_t b = part( c[( first + 1 ) % 4] ) | ( ( first & 1 ) << 15 );
	const std::uint16_t d =
	    part( c[( first + 2 ) % 4] ) | ( c[( first + 3 ) % 4] < 0 ? 0x8000 : 0 );
	return Bytes16( { a, b, d } );
}

// An mstudioanim_valueptr_t and its three streams, each one run of one valid
// frame holding 'values[k]' (a zero value writes no stream).
inline std::string ValueStreams( const std::int16_t ( &values )[3] )
{
	std::string pointers( 6, '\0' );
	std::string streams;
	for ( int k = 0; k < 3; ++k )
	{
		if ( values[k] == 0 )
		{
			continue;
		}
		detail::Put16( pointers, 2 * k, static_cast<std::uint16_t>( 6 + streams.size() ) );
		streams += Bytes16( { 0x0101, static_cast<std::uint16_t>( values[k] ) } );
	}
	return pointers + streams;
}

// Animated rotation and position together: the rotation's value pointer, the
// position's right after it, then both pointers' streams.
inline std::string RotationPositionStreams(
    const std::int16_t ( &rotation )[3], const std::int16_t ( &position )[3] )
{
	std::string pointers( 12, '\0' );
	std::string streams;
	for ( int k = 0; k < 6; ++k )
	{
		const std::int16_t value = k < 3 ? rotation[k] : position[k - 3];
		if ( value == 0 )
		{
			continue;
		}
		const std::size_t pointer = k < 3 ? 0 : 6;
		detail::Put16(
		    pointers, 2 * k, static_cast<std::uint16_t>( 12 + streams.size() - pointer ) );
		streams += Bytes16( { 0x0101, static_cast<std::uint16_t>( value ) } );
	}
	return pointers + streams;
}

// One RLE record's bone, flags and data (the bytes after its 4-byte header).
struct Record
{
	std::uint8_t bone = 0;
	std::uint8_t flags = 0;
	std::string data;
};

// Records chained by nextoffset (the last 0).
inline std::string Records( const std::vector<Record> &records )
{
	std::string out;
	for ( std::size_t i = 0; i < records.size(); ++i )
	{
		const Record &r = records[i];
		std::string header( 4, '\0' );
		header[0] = static_cast<char>( r.bone );
		header[1] = static_cast<char>( r.flags );
		const std::size_t next = i + 1 < records.size() ? 4 + r.data.size() : 0;
		detail::Put16( header, 2, static_cast<std::uint16_t>( next ) );
		out += header + r.data;
	}
	return out;
}

// Frame x bone data for frame 0: per-bone flags, the constant block and the
// frame's block (mstudio_frame_anim_t).
inline std::string FrameAnim(
    const std::vector<std::uint8_t> &flags, const std::string &constants, const std::string &frame )
{
	std::string out( 24, '\0' );
	for ( std::uint8_t f : flags )
	{
		out.push_back( static_cast<char>( f ) );
	}
	const std::size_t constantsAt = out.size();
	out += constants;
	const std::size_t frameAt = out.size();
	out += frame;
	detail::Put32( out, 0, static_cast<std::int32_t>( constantsAt ) );
	detail::Put32( out, 4, static_cast<std::int32_t>( frameAt ) );
	detail::Put32( out, 8, static_cast<std::int32_t>( frame.size() ) );
	return out;
}

inline SyntheticFiles WriteModel( const SyntheticModel &model )
{
	using namespace detail;
	SyntheticFiles files;
	const bool v49 = model.version == 49;

	// --- VVD: every mesh's vertices, body part / model / mesh order ---
	std::vector<mdl::Vertex> all;
	std::vector<mdl::BoneWeights> allWeights;
	for ( const auto &part : model.bodyParts )
	{
		for ( const SyntheticSubModel &sub : part )
		{
			for ( const SyntheticMesh &mesh : sub )
			{
				all.insert( all.end(), mesh.vertices.begin(), mesh.vertices.end() );
				for ( std::size_t v = 0; v < mesh.vertices.size(); ++v )
				{
					allWeights.push_back(
					    v < mesh.weights.size() ? mesh.weights[v] : mdl::BoneWeights{} );
				}
			}
		}
	}
	std::string &vvd = files.vvd;
	Grow( vvd, 64 );
	Put32( vvd, 0, 0x56534449 );
	Put32( vvd, 4, 4 );
	Put32( vvd, 8, model.checksum );
	Put32( vvd, 12, static_cast<std::int32_t>( model.lodSwitches.size() ) );
	for ( std::size_t lod = 0; lod < model.lodSwitches.size(); ++lod )
		Put32( vvd, 16 + lod * 4, static_cast<std::int32_t>( all.size() ) );
	const std::size_t half = all.size() / 2;
	std::vector<mdl::Vertex> stored = all;
	std::vector<mdl::BoneWeights> storedWeights = allWeights;
	if ( model.fixups )
	{
		// stored = all[half..] ++ all[..half]; fixups restore 'all'.
		stored.assign( all.begin() + static_cast<std::ptrdiff_t>( half ), all.end() );
		stored.insert(
		    stored.end(), all.begin(), all.begin() + static_cast<std::ptrdiff_t>( half ) );
		storedWeights.assign(
		    allWeights.begin() + static_cast<std::ptrdiff_t>( half ), allWeights.end() );
		storedWeights.insert( storedWeights.end(), allWeights.begin(),
		    allWeights.begin() + static_cast<std::ptrdiff_t>( half ) );
		Put32( vvd, 48, 2 );
		const std::size_t table = Grow( vvd, 24 );
		Put32( vvd, 52, static_cast<std::int32_t>( table ) );
		Put32( vvd, table + 0, 0 );
		Put32( vvd, table + 4, static_cast<std::int32_t>( all.size() - half ) );
		Put32( vvd, table + 8, static_cast<std::int32_t>( half ) );
		Put32( vvd, table + 12, 0 );
		Put32( vvd, table + 16, 0 );
		Put32( vvd, table + 20, static_cast<std::int32_t>( all.size() - half ) );
	}
	const std::size_t data = Grow( vvd, stored.size() * 48 );
	Put32( vvd, 56, static_cast<std::int32_t>( data ) );
	for ( std::size_t i = 0; i < stored.size(); ++i )
	{
		const std::size_t at = data + i * 48;
		const mdl::BoneWeights &w = storedWeights[i];
		for ( int k = 0; k < 3; ++k )
		{
			PutF( vvd, at + 4 * k, w.weights[k] );
			vvd[at + 12 + k] = static_cast<char>( w.bones[k] );
		}
		vvd[at + 15] = static_cast<char>( w.count );
		const mdl::Vertex &v = stored[i];
		PutF( vvd, at + 16, v.position.x );
		PutF( vvd, at + 20, v.position.y );
		PutF( vvd, at + 24, v.position.z );
		PutF( vvd, at + 28, v.normal.x );
		PutF( vvd, at + 32, v.normal.y );
		PutF( vvd, at + 36, v.normal.z );
		PutF( vvd, at + 40, v.u );
		PutF( vvd, at + 44, v.v );
	}

	// --- MDL ---
	std::string &mdl = files.mdl;
	Grow( mdl, 408 );
	Put32( mdl, 0, 0x54534449 );
	Put32( mdl, 4, model.version );
	Put32( mdl, 8, model.checksum );
	std::memcpy( &mdl[12], model.name.data(), std::min<std::size_t>( model.name.size(), 63 ) );
	const std::size_t textures = Grow( mdl, model.textures.size() * 64 );
	Put32( mdl, 204, static_cast<std::int32_t>( model.textures.size() ) );
	Put32( mdl, 208, static_cast<std::int32_t>( textures ) );
	const std::size_t cd = Grow( mdl, model.cdMaterials.size() * 4 );
	Put32( mdl, 212, static_cast<std::int32_t>( model.cdMaterials.size() ) );
	Put32( mdl, 216, static_cast<std::int32_t>( cd ) );
	const std::size_t refs = model.skins.empty() ? 0 : model.skins.front().size();
	const std::size_t skins = Grow( mdl, model.skins.size() * refs * 2 );
	Put32( mdl, 220, static_cast<std::int32_t>( refs ) );
	Put32( mdl, 224, static_cast<std::int32_t>( model.skins.size() ) );
	Put32( mdl, 228, static_cast<std::int32_t>( skins ) );
	for ( std::size_t f = 0; f < model.skins.size(); ++f )
	{
		for ( std::size_t r = 0; r < refs; ++r )
		{
			Put16( mdl, skins + ( f * refs + r ) * 2,
			    static_cast<std::uint16_t>( model.skins[f][r] ) );
		}
	}
	const std::size_t parts = Grow( mdl, model.bodyParts.size() * 16 );
	Put32( mdl, 232, static_cast<std::int32_t>( model.bodyParts.size() ) );
	Put32( mdl, 236, static_cast<std::int32_t>( parts ) );
	std::size_t vertexCursor = 0;
	std::int32_t base = 1;
	for ( std::size_t p = 0; p < model.bodyParts.size(); ++p )
	{
		const std::size_t part = parts + p * 16;
		const auto &subs = model.bodyParts[p];
		const std::size_t subsAt = Grow( mdl, subs.size() * 148 );
		Put32( mdl, part + 4, static_cast<std::int32_t>( subs.size() ) );
		Put32( mdl, part + 8, base );
		Put32( mdl, part + 12, static_cast<std::int32_t>( subsAt - part ) );
		base *= std::max<std::int32_t>( 1, static_cast<std::int32_t>( subs.size() ) );
		for ( std::size_t s = 0; s < subs.size(); ++s )
		{
			const std::size_t sub = subsAt + s * 148;
			const std::size_t meshes = Grow( mdl, subs[s].size() * 116 );
			Put32( mdl, sub + 72, static_cast<std::int32_t>( subs[s].size() ) );
			Put32( mdl, sub + 76, static_cast<std::int32_t>( meshes - sub ) );
			std::size_t count = 0;
			for ( const SyntheticMesh &mesh : subs[s] )
			{
				count += mesh.vertices.size();
			}
			Put32( mdl, sub + 80, static_cast<std::int32_t>( count ) );
			Put32( mdl, sub + 84, static_cast<std::int32_t>( vertexCursor * 48 ) );
			std::size_t offset = 0;
			for ( std::size_t m = 0; m < subs[s].size(); ++m )
			{
				const std::size_t mesh = meshes + m * 116;
				Put32( mdl, mesh + 0, subs[s][m].textureRef );
				Put32( mdl, mesh + 8, static_cast<std::int32_t>( subs[s][m].vertices.size() ) );
				Put32( mdl, mesh + 12, static_cast<std::int32_t>( offset ) );
				offset += subs[s][m].vertices.size();
			}
			vertexCursor += count;
		}
	}
	for ( std::size_t t = 0; t < model.textures.size(); ++t )
	{
		const std::size_t at = AppendString( mdl, model.textures[t] );
		Put32( mdl, textures + t * 64, static_cast<std::int32_t>( at - ( textures + t * 64 ) ) );
	}
	for ( std::size_t c = 0; c < model.cdMaterials.size(); ++c )
	{
		Put32( mdl, cd + c * 4,
		    static_cast<std::int32_t>( AppendString( mdl, model.cdMaterials[c] ) ) );
	}
	// --- Skeleton, animations, sequences ---
	Put32( mdl, 152, static_cast<std::int32_t>( model.flags ) );
	std::vector<SyntheticBone> bones = model.bones;
	if ( bones.empty() )
	{
		bones.emplace_back();
	}
	const std::size_t boneAt = Grow( mdl, bones.size() * 216 );
	Put32( mdl, 156, static_cast<std::int32_t>( bones.size() ) );
	Put32( mdl, 160, static_cast<std::int32_t>( boneAt ) );
	for ( std::size_t b = 0; b < bones.size(); ++b )
	{
		const SyntheticBone &bone = bones[b];
		const std::size_t at = boneAt + b * 216;
		Put32( mdl, at + 4, bone.parent );
		const float fields[] = { bone.position.x, bone.position.y, bone.position.z, bone.rotation.x,
		    bone.rotation.y, bone.rotation.z, bone.rotation.w, bone.euler.x, bone.euler.y,
		    bone.euler.z, bone.positionScale.x, bone.positionScale.y, bone.positionScale.z,
		    bone.rotationScale.x, bone.rotationScale.y, bone.rotationScale.z };
		for ( std::size_t f = 0; f < std::size( fields ); ++f )
		{
			PutF( mdl, at + 32 + f * 4, fields[f] );
		}
		for ( int r = 0; r < 3; ++r )
		{
			for ( int c = 0; c < 4; ++c )
			{
				PutF( mdl, at + 96 + ( r * 4 + c ) * 4, bone.poseToBone.m[r][c] );
			}
		}
		Put32( mdl, at + 160, static_cast<std::int32_t>( bone.flags ) );
	}
	for ( std::size_t b = 0; b < bones.size(); ++b )
	{
		const std::size_t at = boneAt + b * 216;
		Put32( mdl, at, static_cast<std::int32_t>( AppendString( mdl, bones[b].name ) - at ) );
	}
	const std::size_t descs = Grow( mdl, model.animations.size() * 100 );
	Put32( mdl, 180, static_cast<std::int32_t>( model.animations.size() ) );
	Put32( mdl, 184, static_cast<std::int32_t>( descs ) );
	std::int32_t blockCount = 0;
	for ( const SyntheticAnimation &a : model.animations )
	{
		blockCount = std::max( blockCount, a.block + 1 );
	}
	std::vector<std::pair<std::size_t, std::size_t>> blockRanges(
	    static_cast<std::size_t>( blockCount ) );
	if ( blockCount > 1 )
	{
		files.ani.assign( 16, '\0' ); // a stand-in header
	}
	for ( std::size_t a = 0; a < model.animations.size(); ++a )
	{
		const SyntheticAnimation &anim = model.animations[a];
		const std::size_t desc = descs + a * 100;
		Put32( mdl, desc + 12, static_cast<std::int32_t>( anim.flags ) );
		Put32( mdl, desc + 16, 1 );
		PutF( mdl, desc + 8, 30.0f );
		std::int32_t index = 0;
		if ( anim.block == 0 )
		{
			index = static_cast<std::int32_t>( mdl.size() - desc );
			mdl += anim.data;
		}
		else if ( anim.block > 0 )
		{
			const std::size_t start = files.ani.size();
			files.ani += anim.data;
			blockRanges[static_cast<std::size_t>( anim.block )] = { start, files.ani.size() };
		}
		if ( anim.sectioned )
		{
			const std::size_t sections = Grow( mdl, 16 );
			Put32( mdl, desc + 80, static_cast<std::int32_t>( sections - desc ) );
			Put32( mdl, desc + 84, 30 );
			Put32( mdl, sections, anim.block );
			Put32( mdl, sections + 4, index );
			Put32( mdl, desc + 52, 0 );
			Put32( mdl, desc + 56, 0 );
		}
		else
		{
			Put32( mdl, desc + 52, anim.block );
			Put32( mdl, desc + 56, index );
		}
		if ( !anim.zeroFrame.empty() )
		{
			Put16( mdl, desc + 90, 1 );
			Put32( mdl, desc + 92, static_cast<std::int32_t>( mdl.size() - desc ) );
			mdl += anim.zeroFrame;
		}
		Put32( mdl, desc + 4, static_cast<std::int32_t>( AppendString( mdl, "@anim" ) - desc ) );
	}
	if ( blockCount > 1 )
	{
		const std::size_t table = Grow( mdl, blockRanges.size() * 8 );
		Put32( mdl, 352, blockCount );
		Put32( mdl, 356, static_cast<std::int32_t>( table ) );
		for ( std::size_t b = 0; b < blockRanges.size(); ++b )
		{
			Put32( mdl, table + b * 8, static_cast<std::int32_t>( blockRanges[b].first ) );
			Put32( mdl, table + b * 8 + 4, static_cast<std::int32_t>( blockRanges[b].second ) );
		}
		Put32( mdl, 348, static_cast<std::int32_t>( AppendString( mdl, model.aniName ) ) );
	}
	const std::size_t seqs = Grow( mdl, model.sequences.size() * 212 );
	Put32( mdl, 188, static_cast<std::int32_t>( model.sequences.size() ) );
	Put32( mdl, 192, static_cast<std::int32_t>( seqs ) );
	for ( std::size_t q = 0; q < model.sequences.size(); ++q )
	{
		const SyntheticSequence &seq = model.sequences[q];
		const std::size_t at = seqs + q * 212;
		Put32( mdl, at + 12, static_cast<std::int32_t>( seq.flags ) );
		const float box[] = {
		    seq.bbMin.x, seq.bbMin.y, seq.bbMin.z, seq.bbMax.x, seq.bbMax.y, seq.bbMax.z };
		for ( std::size_t f = 0; f < 6; ++f )
		{
			PutF( mdl, at + 32 + f * 4, box[f] );
		}
		Put32( mdl, at + 56, static_cast<std::int32_t>( seq.blends.size() ) );
		Put32( mdl, at + 68, static_cast<std::int32_t>( seq.blends.size() ) );
		Put32( mdl, at + 72, 1 );
		const std::size_t blends = Grow( mdl, seq.blends.size() * 2 + 2 );
		for ( std::size_t b = 0; b < seq.blends.size(); ++b )
		{
			Put16( mdl, blends + b * 2, static_cast<std::uint16_t>( seq.blends[b] ) );
		}
		Put32( mdl, at + 60, static_cast<std::int32_t>( blends - at ) );
		const std::size_t weights = Grow( mdl, bones.size() * 4 );
		for ( std::size_t b = 0; b < bones.size(); ++b )
		{
			PutF( mdl, weights + b * 4, b < seq.weights.size() ? seq.weights[b] : 1.0f );
		}
		Put32( mdl, at + 156, static_cast<std::int32_t>( weights - at ) );
		Put32( mdl, at + 4, static_cast<std::int32_t>( AppendString( mdl, seq.label ) - at ) );
	}
	const std::size_t groups = Grow( mdl, model.includes.size() * 8 );
	Put32( mdl, 336, static_cast<std::int32_t>( model.includes.size() ) );
	Put32( mdl, 340, static_cast<std::int32_t>( groups ) );
	for ( std::size_t g = 0; g < model.includes.size(); ++g )
	{
		const std::size_t at = groups + g * 8;
		Put32( mdl, at, static_cast<std::int32_t>( AppendString( mdl, "" ) - at ) );
		Put32(
		    mdl, at + 4, static_cast<std::int32_t>( AppendString( mdl, model.includes[g] ) - at ) );
	}
	Put32( mdl, 76, static_cast<std::int32_t>( mdl.size() ) );

	// --- VTX ---
	std::string &vtx = files.vtx;
	const std::size_t groupStride = v49 ? 33 : 25;
	const std::size_t stripStride = v49 ? 35 : 27;
	Grow( vtx, 36 );
	Put32( vtx, 0, 7 );
	Put32( vtx, 16, model.checksum );
	Put32( vtx, 20, static_cast<std::int32_t>( model.lodSwitches.size() ) );
	const std::size_t replacementLists = Grow( vtx, model.lodSwitches.size() * 8 );
	Put32( vtx, 24, static_cast<std::int32_t>( replacementLists ) );
	for ( std::size_t lod = 0; lod < model.lodReplacements.size(); ++lod )
	{
		const std::size_t list = replacementLists + lod * 8;
		const auto &source = model.lodReplacements[lod];
		const std::size_t entries = Grow( vtx, source.size() * 6 );
		Put32( vtx, list, static_cast<std::int32_t>( source.size() ) );
		Put32( vtx, list + 4, static_cast<std::int32_t>( entries - list ) );
		for ( std::size_t i = 0; i < source.size(); ++i )
		{
			const std::size_t entry = entries + i * 6;
			Put16( vtx, entry, source[i].first );
			const std::size_t name = AppendString( vtx, source[i].second );
			Put32( vtx, entry + 2, static_cast<std::int32_t>( name - entry ) );
		}
	}
	const std::size_t vparts = Grow( vtx, model.bodyParts.size() * 8 );
	Put32( vtx, 28, static_cast<std::int32_t>( model.bodyParts.size() ) );
	Put32( vtx, 32, static_cast<std::int32_t>( vparts ) );
	for ( std::size_t p = 0; p < model.bodyParts.size(); ++p )
	{
		const std::size_t part = vparts + p * 8;
		const auto &subs = model.bodyParts[p];
		const std::size_t vsubs = Grow( vtx, subs.size() * 8 );
		Put32( vtx, part, static_cast<std::int32_t>( subs.size() ) );
		Put32( vtx, part + 4, static_cast<std::int32_t>( vsubs - part ) );
		for ( std::size_t s = 0; s < subs.size(); ++s )
		{
			const std::size_t sub = vsubs + s * 8;
			const std::size_t lods = Grow( vtx, model.lodSwitches.size() * 12 );
			Put32( vtx, sub, static_cast<std::int32_t>( model.lodSwitches.size() ) );
			Put32( vtx, sub + 4, static_cast<std::int32_t>( lods - sub ) );
			for ( std::size_t level = 0; level < model.lodSwitches.size(); ++level )
			{
				const std::size_t lod = lods + level * 12;
				PutF( vtx, lod + 8, model.lodSwitches[level] );
				const std::size_t vmeshes = Grow( vtx, subs[s].size() * 9 );
				Put32( vtx, lod, static_cast<std::int32_t>( subs[s].size() ) );
				Put32( vtx, lod + 4, static_cast<std::int32_t>( vmeshes - lod ) );
				for ( std::size_t m = 0; m < subs[s].size(); ++m )
				{
					const SyntheticMesh &mesh = subs[s][m];
					const auto &topology = level > 0 && level <= mesh.lodIndices.size()
					                           ? mesh.lodIndices[level - 1]
					                           : mesh.stored;
					const std::size_t vmesh = vmeshes + m * 9;
					const std::size_t group = Grow( vtx, groupStride );
					Put32( vtx, vmesh, 1 );
					Put32( vtx, vmesh + 4, static_cast<std::int32_t>( group - vmesh ) );
					const std::size_t verts = Grow( vtx, mesh.vertices.size() * 9 );
					for ( std::size_t v = 0; v < mesh.vertices.size(); ++v )
					{
						vtx[verts + v * 9 + 3] = 1;
						Put16( vtx, verts + v * 9 + 4, static_cast<std::uint16_t>( v ) );
					}
					const std::size_t indices = Grow( vtx, topology.size() * 2 );
					for ( std::size_t i = 0; i < topology.size(); ++i )
					{
						Put16( vtx, indices + i * 2, topology[i] );
					}
					const std::size_t strip = Grow( vtx, stripStride );
					Put32( vtx, group + 0, static_cast<std::int32_t>( mesh.vertices.size() ) );
					Put32( vtx, group + 4, static_cast<std::int32_t>( verts - group ) );
					Put32( vtx, group + 8, static_cast<std::int32_t>( topology.size() ) );
					Put32( vtx, group + 12, static_cast<std::int32_t>( indices - group ) );
					Put32( vtx, group + 16, 1 );
					Put32( vtx, group + 20, static_cast<std::int32_t>( strip - group ) );
					Put32( vtx, strip + 0, static_cast<std::int32_t>( topology.size() ) );
					Put32( vtx, strip + 4, 0 );
					Put32( vtx, strip + 8, static_cast<std::int32_t>( mesh.vertices.size() ) );
					Put32( vtx, strip + 12, 0 );
					vtx[strip + 18] = static_cast<char>( mesh.tristrip ? 0x02 : 0x01 );
				}
			}
		}
	}
	return files;
}

// An axis-aligned box of half-extents (hx, hy, hz) centred on 'center': six
// faces of four vertices each (outward normals, uv 0..1 per face), stored
// clockwise from outside.
inline SyntheticMesh BoxMesh( mdl::Float3 center, mdl::Float3 half, std::int32_t textureRef = 0 )
{
	SyntheticMesh mesh;
	mesh.textureRef = textureRef;
	struct Face
	{
		mdl::Float3 n, u, v; // normal, right, up (u x v = n)
	};
	const Face faces[6] = {
	    { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } },
	    { { -1, 0, 0 }, { 0, -1, 0 }, { 0, 0, 1 } },
	    { { 0, 1, 0 }, { -1, 0, 0 }, { 0, 0, 1 } },
	    { { 0, -1, 0 }, { 1, 0, 0 }, { 0, 0, 1 } },
	    { { 0, 0, 1 }, { 1, 0, 0 }, { 0, 1, 0 } },
	    { { 0, 0, -1 }, { -1, 0, 0 }, { 0, 1, 0 } },
	};
	for ( const Face &f : faces )
	{
		const auto base = static_cast<std::uint16_t>( mesh.vertices.size() );
		const float corners[4][2] = { { -1, -1 }, { 1, -1 }, { 1, 1 }, { -1, 1 } };
		for ( const auto &c : corners )
		{
			mdl::Vertex v;
			v.position = { center.x + half.x * ( f.n.x + c[0] * f.u.x + c[1] * f.v.x ),
			    center.y + half.y * ( f.n.y + c[0] * f.u.y + c[1] * f.v.y ),
			    center.z + half.z * ( f.n.z + c[0] * f.u.z + c[1] * f.v.z ) };
			v.normal = f.n;
			v.u = ( c[0] + 1.0f ) * 0.5f;
			v.v = ( 1.0f - c[1] ) * 0.5f;
			mesh.vertices.push_back( v );
		}
		// Counter-clockwise from outside is 0 1 2, 0 2 3; stored clockwise.
		for ( std::uint16_t i : { 0, 2, 1, 0, 3, 2 } )
		{
			mesh.stored.push_back( static_cast<std::uint16_t>( base + i ) );
		}
	}
	return mesh;
}

// A one-texture box model (see BoxMesh), material "<cd><texture>".
inline SyntheticModel LodPixelModel()
{
	SyntheticModel model;
	model.name = "synthetic/lod-pixels.mdl";
	model.textures = { "base" };
	model.skins = { { 0 } };
	model.lodSwitches = { 0.0f, 100.0f, -1.0f };
	model.lodReplacements = { {}, { { 0, "lower" } }, {} };
	SyntheticMesh mesh;
	for ( unsigned int side = 0; side < 2; ++side )
	{
		const float left = side == 0 ? -0.8f : 0.2f;
		for ( const auto &xy : { std::pair{ left, -0.5f }, std::pair{ left + 0.6f, -0.5f },
		          std::pair{ left + 0.6f, 0.5f }, std::pair{ left, 0.5f } } )
		{
			mdl::Vertex vertex;
			vertex.position = { xy.first, xy.second, 0.5f };
			vertex.normal = { 0, 0, 1 };
			mesh.vertices.push_back( vertex );
			mdl::BoneWeights weight;
			weight.bones[0] = static_cast<std::uint8_t>( side );
			mesh.weights.push_back( weight );
		}
	}
	mesh.stored = { 0, 2, 1, 0, 3, 2 };
	mesh.lodIndices = { { 4, 6, 5, 4, 7, 6 }, {} };
	model.bodyParts = { { { mesh }, {} } };
	SyntheticBone first, second;
	first.flags = 0x400; // BONE_USED_BY_VERTEX_AT_LOD(0)
	second.name = "lower_lod";
	second.flags = 0x800; // BONE_USED_BY_VERTEX_AT_LOD(1)
	model.bones = { first, second };
	return model;
}

inline SyntheticModel BoxModel( mdl::Float3 half, const std::string &cdMaterials,
    const std::string &texture, std::int32_t version = 49 )
{
	SyntheticModel model;
	model.version = version;
	model.textures = { texture };
	model.cdMaterials = { cdMaterials };
	model.skins = { { 0 } };
	model.bodyParts = { { { BoxMesh( { 0, 0, 0 }, half ) } } };
	return model;
}

} // namespace mdltest

#endif // MDLTEST_SYNTHETIC_MODEL_H
