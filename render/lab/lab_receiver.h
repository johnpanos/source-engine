//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's receiver scene (RFC 0016 K11): a white pbr plane,
//			z = 0 facing +z, seen through a square perspective view, with
//			the ray through each pixel centre for the suites' oracles. The
//			area-light and clustered-light suites judge their terms on it.
//
//=============================================================================//

#ifndef RENDER_LAB_LAB_RECEIVER_H
#define RENDER_LAB_LAB_RECEIVER_H

#include "render/math/matrix.h"
#include "render/math/vector.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace render::lab
{

struct ReceiverView
{
	math::float3 eye;
	std::uint32_t size = 0; // the square target's width and height in pixels
	math::float4x4 view;
	math::float4x4 projection;
	math::float4x4 toClip;
	math::float4x4 fromClip;
	float nearZ = 1.0f;
	float farZ = 8192.0f;
};

// A 70 degree view from `eye` toward `target`, z up.
ReceiverView MakeReceiverView( math::float3 eye, math::float3 target, std::uint32_t size );

// Where a pixel centre's ray meets the plane z = 0 within +-extent*0.98.
std::optional<math::float3> ReceiverHit(
    const ReceiverView &view, std::uint32_t px, std::uint32_t py, float extent );

// The unit vector from a receiver point to the eye.
math::float3 ToEye( const ReceiverView &view, math::float3 p );

// A receiver material: MRAO bytes (the texture the suites stage), white base.
struct ReceiverMaterial
{
	const char *name;
	int metal;
	int roughness;
	float Metal() const { return float( metal ) / 255.0f; }
	float Roughness() const { return std::max( float( roughness ) / 255.0f, 0.02f ); }
	// f0 of the white base: 0.04 for a dielectric, 1 for a metal.
	float F0() const { return 0.04f + ( 1.0f - 0.04f ) * Metal(); }
};

inline constexpr ReceiverMaterial kReceiverMaterials[] = { { "dielectric-r50", 0, 128 },
    { "metal-r25", 255, 64 }, { "metal-r50", 255, 128 }, { "metal-r80", 255, 204 } };

// The plane as two triangles of SurfaceModelVertex (normal +z, tangent +x).
std::vector<std::byte> ReceiverMesh( float extent );

} // namespace render::lab

#endif // RENDER_LAB_LAB_RECEIVER_H
