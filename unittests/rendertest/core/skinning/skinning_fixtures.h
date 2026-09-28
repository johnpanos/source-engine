//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.skinning fixtures (RFC 0016 K6): seeded random skinned
//			meshes with stereo flex, shared by the headless reference suite
//			and the Vulkan suite.
//
//=============================================================================//

#ifndef RENDERTEST_CORE_SKINNING_FIXTURES_H
#define RENDERTEST_CORE_SKINNING_FIXTURES_H

#include "render/pass/skinning/skinning.h"

#include <cmath>
#include <cstdint>
#include <random>
#include <vector>

namespace rendertest::skinning
{

using namespace render::pass::skinning;

struct Mesh
{
	std::vector<SkinVertex> vertices;
	std::vector<BoneMatrix> bones;
	std::vector<std::uint32_t> flexOffsets; // empty without flex
	std::vector<FlexDelta> flexDeltas;
	std::vector<FlexWeights> flexWeights;

	SkinInputs Inputs() const { return { vertices, bones, flexOffsets, flexDeltas, flexWeights }; }
};

// A rotation (from a random unit quaternion), scaled by up to 10 %, and a
// translation of up to 512 units: the pose-to-world matrices of a model
// standing somewhere in a map.
inline BoneMatrix RandomBone( std::mt19937 &random )
{
	std::uniform_real_distribution<float> unit( -1.0f, 1.0f );
	float q[4];
	float length = 0.0f;
	do
	{
		for ( float &c : q )
			c = unit( random );
		length = std::sqrt( q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3] );
	} while ( length < 0.1f );
	for ( float &c : q )
		c /= length;
	const float scale = 1.0f + 0.1f * unit( random );
	const float x = q[0], y = q[1], z = q[2], w = q[3];
	BoneMatrix bone;
	const float r[3][3] = {
	    { 1 - 2 * ( y * y + z * z ), 2 * ( x * y - z * w ), 2 * ( x * z + y * w ) },
	    { 2 * ( x * y + z * w ), 1 - 2 * ( x * x + z * z ), 2 * ( y * z - x * w ) },
	    { 2 * ( x * z - y * w ), 2 * ( y * z + x * w ), 1 - 2 * ( x * x + y * y ) } };
	for ( int i = 0; i < 3; ++i )
	{
		for ( int j = 0; j < 3; ++j )
			bone.rows[i][j] = scale * r[i][j];
		bone.rows[i][3] = 512.0f * unit( random );
	}
	return bone;
}

// Vertices within 64 units of the model origin, one to three bones with
// weights summing to one (studiomdl's layout: unused weights 0), about one
// in 32 bone indices past the palette, and on flexed meshes up to four
// deltas per vertex over `flexCount` flexes whose two weights differ.
inline Mesh RandomMesh( std::uint32_t seed, std::uint32_t vertexCount, std::uint32_t boneCount,
    std::uint32_t flexCount )
{
	std::mt19937 random( seed );
	std::uniform_real_distribution<float> unit( -1.0f, 1.0f );
	std::uniform_real_distribution<float> positive( 0.0f, 1.0f );
	Mesh mesh;
	for ( std::uint32_t b = 0; b < boneCount; ++b )
		mesh.bones.push_back( RandomBone( random ) );
	for ( std::uint32_t v = 0; v < vertexCount; ++v )
	{
		SkinVertex vertex;
		for ( int k = 0; k < 3; ++k )
		{
			vertex.position[k] = 64.0f * unit( random );
			vertex.normal[k] = unit( random );
			vertex.tangent[k] = unit( random );
		}
		vertex.tangent[3] = random() % 2 ? 1.0f : -1.0f;
		const int used = 1 + static_cast<int>( random() % 3 );
		float w0 = used == 1 ? 1.0f : positive( random );
		float w1 = used == 3 ? ( 1.0f - w0 ) * positive( random ) : used == 2 ? 1.0f - w0 : 0.0f;
		vertex.weight0 = w0;
		vertex.weight1 = w1;
		std::uint32_t bones = 0;
		for ( int b = 0; b < 3; ++b )
		{
			std::uint32_t index = b < used ? random() % boneCount : 0;
			if ( random() % 32 == 0 )
				index = boneCount + random() % ( 256 - boneCount ); // past the palette
			bones |= ( index & 0xffu ) << ( 8 * b );
		}
		vertex.bones = bones;
		mesh.vertices.push_back( vertex );
	}
	if ( flexCount > 0 )
	{
		for ( std::uint32_t f = 0; f < flexCount; ++f )
			mesh.flexWeights.push_back( { { positive( random ), positive( random ) } } );
		mesh.flexOffsets.push_back( 0 );
		for ( std::uint32_t v = 0; v < vertexCount; ++v )
		{
			const std::uint32_t deltas = random() % 5;
			for ( std::uint32_t d = 0; d < deltas; ++d )
			{
				FlexDelta delta;
				for ( int k = 0; k < 3; ++k )
				{
					delta.position[k] = 4.0f * unit( random );
					delta.normal[k] = 0.5f * unit( random );
				}
				delta.flex = random() % flexCount;
				delta.side = positive( random );
				delta.wrinkle = unit( random );
				mesh.flexDeltas.push_back( delta );
			}
			mesh.flexOffsets.push_back( static_cast<std::uint32_t>( mesh.flexDeltas.size() ) );
		}
	}
	return mesh;
}

} // namespace rendertest::skinning

#endif // RENDERTEST_CORE_SKINNING_FIXTURES_H
