//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Test-only writer of synthetic studio models (MDL + VVD + VTX
//			bytes) for content.studio-model and its consumers. It writes the
//			layouts public/mdl/studio_model.h documents (the engine's on-disk
//			records), independently of the reader: a model is described as
//			body parts, models and meshes whose triangles are given in the
//			files' order (clockwise seen from outside).
//
//=============================================================================//

#ifndef MDLTEST_SYNTHETIC_MODEL_H
#define MDLTEST_SYNTHETIC_MODEL_H

#include "mdl/studio_model.h"

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace mdltest
{

struct SyntheticMesh
{
	std::int32_t textureRef = 0;
	std::vector<mdl::Vertex> vertices;
	std::vector<std::uint16_t> stored; // as the file stores them: clockwise from outside
	bool tristrip = false;             // 'stored' is one triangle strip
};

using SyntheticSubModel = std::vector<SyntheticMesh>; // one mstudiomodel_t

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
};

struct SyntheticFiles
{
	std::string mdl;
	std::string vvd;
	std::string vtx;
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

inline SyntheticFiles WriteModel( const SyntheticModel &model )
{
	using namespace detail;
	SyntheticFiles files;
	const bool v49 = model.version == 49;

	// --- VVD: every mesh's vertices, body part / model / mesh order ---
	std::vector<mdl::Vertex> all;
	for ( const auto &part : model.bodyParts )
	{
		for ( const SyntheticSubModel &sub : part )
		{
			for ( const SyntheticMesh &mesh : sub )
			{
				all.insert( all.end(), mesh.vertices.begin(), mesh.vertices.end() );
			}
		}
	}
	std::string &vvd = files.vvd;
	Grow( vvd, 64 );
	Put32( vvd, 0, 0x56534449 );
	Put32( vvd, 4, 4 );
	Put32( vvd, 8, model.checksum );
	Put32( vvd, 12, 1 );
	Put32( vvd, 16, static_cast<std::int32_t>( all.size() ) );
	const std::size_t half = all.size() / 2;
	std::vector<mdl::Vertex> stored = all;
	if ( model.fixups )
	{
		// stored = all[half..] ++ all[..half]; fixups restore 'all'.
		stored.assign( all.begin() + static_cast<std::ptrdiff_t>( half ), all.end() );
		stored.insert(
		    stored.end(), all.begin(), all.begin() + static_cast<std::ptrdiff_t>( half ) );
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
		vvd[at] = 0; // weights: one bone, weight 1
		PutF( vvd, at, 1.0f );
		vvd[at + 15] = 1;
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
	Put32( mdl, 76, static_cast<std::int32_t>( mdl.size() ) );

	// --- VTX ---
	std::string &vtx = files.vtx;
	const std::size_t groupStride = v49 ? 33 : 25;
	const std::size_t stripStride = v49 ? 35 : 27;
	Grow( vtx, 36 );
	Put32( vtx, 0, 7 );
	Put32( vtx, 16, model.checksum );
	Put32( vtx, 20, 1 );
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
			const std::size_t lod = Grow( vtx, 12 );
			Put32( vtx, sub, 1 );
			Put32( vtx, sub + 4, static_cast<std::int32_t>( lod - sub ) );
			const std::size_t vmeshes = Grow( vtx, subs[s].size() * 9 );
			Put32( vtx, lod, static_cast<std::int32_t>( subs[s].size() ) );
			Put32( vtx, lod + 4, static_cast<std::int32_t>( vmeshes - lod ) );
			for ( std::size_t m = 0; m < subs[s].size(); ++m )
			{
				const SyntheticMesh &mesh = subs[s][m];
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
				const std::size_t indices = Grow( vtx, mesh.stored.size() * 2 );
				for ( std::size_t i = 0; i < mesh.stored.size(); ++i )
				{
					Put16( vtx, indices + i * 2, mesh.stored[i] );
				}
				const std::size_t strip = Grow( vtx, stripStride );
				Put32( vtx, group + 0, static_cast<std::int32_t>( mesh.vertices.size() ) );
				Put32( vtx, group + 4, static_cast<std::int32_t>( verts - group ) );
				Put32( vtx, group + 8, static_cast<std::int32_t>( mesh.stored.size() ) );
				Put32( vtx, group + 12, static_cast<std::int32_t>( indices - group ) );
				Put32( vtx, group + 16, 1 );
				Put32( vtx, group + 20, static_cast<std::int32_t>( strip - group ) );
				Put32( vtx, strip + 0, static_cast<std::int32_t>( mesh.stored.size() ) );
				Put32( vtx, strip + 4, 0 );
				Put32( vtx, strip + 8, static_cast<std::int32_t>( mesh.vertices.size() ) );
				Put32( vtx, strip + 12, 0 );
				vtx[strip + 18] = static_cast<char>( mesh.tristrip ? 0x02 : 0x01 );
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
