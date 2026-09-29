//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Light blocked by moving objects (render.dynamic-occlusion.v1,
//          RFC 0011): one rule for every moving thing that casts a shadow (a
//          prop, a physics object, an NPC, a door model) and every light.
//
//          Occluders. Each frame the client publishes every drawn moving
//          object that casts a dynamic shadow in the game's shadow model (the
//          render-to-texture casters these shadows replace: a studio model
//          unless its shadow is disabled, a physics brush; not func_brush or
//          func_door, whose light the bake owns) as oriented boxes: a studio
//          model's hitboxes on their bones, otherwise its collision box. A
//          box belongs to one entity,
//          keyed by its entity handle (index and serial: the key the render
//          core's scene instances use) and its part (the box's index within
//          the entity). The engine versions each box: any change of its
//          transform beyond a small tolerance is a new version.
//
//          Visibility. A light reaches a receiver through the fraction of
//          its samples whose segment from the receiver no box crosses: a
//          point, spot or surface light is a small disk facing the receiver
//          (kDiskSamples; its size kPointLightRadius or kSurfaceLightRadius),
//          so shadows have penumbrae; a distant light one sample far along its
//          direction; an area light kAreaSamples^2 over its rectangle.
//          Each light is judged on its own, so lights mix: a receiver lit by
//          two lights and shadowed from one keeps the other's light in full.
//          A receiver that belongs to an entity (a model's lighting origin)
//          ignores that entity's boxes: an object does not shadow its own
//          lighting.
//
//          Consumers apply it the same way: a light's contribution times its
//          visibility. World lightmaps remove each light's blocked share of
//          its direct light (engine/dynamic_occlusion.cpp); model lighting
//          scales each of a model's lights; the BSP2 DirectOcclusion and the
//          render core's shadows take the same occluders.
//
//          Tier0-free C++ shared by the client, the engine and the
//          conformance suite (unittests/rendertest/test_dynamic_occlusion.cpp).
//
//===========================================================================//

#ifndef RENDER_DYNAMIC_OCCLUSION_H
#define RENDER_DYNAMIC_OCCLUSION_H

#include "render/area_light.h"

#include <cmath>
#include <cstdint>

namespace dynamic_occlusion
{

// A box: its center and three half-extent vectors (orthogonal; their lengths
// are the half extents).
struct Box
{
	float center[3] = {};
	float axes[3][3] = {};
	int entity = 0; // the entity handle it belongs to (0: none)
	int part = 0;   // its index within the entity
};

// A light's samples: points the light leaves from, each with a weight (the
// weights of one light sum to 1).
constexpr int kAreaSamples = 4; // per rectangle axis
constexpr int kMaxSamples = kAreaSamples * kAreaSamples;
// The sizes lights shadow with (Source units): a bulb, and a lit panel.
constexpr float kPointLightRadius = 6.0f;
constexpr float kSurfaceLightRadius = 16.0f;
// A distant light's sample stands this far away along its direction.
constexpr float kDistantLength = 32768.0f;

struct Samples
{
	int count = 0;
	float point[kMaxSamples][3] = {};
	float weight[kMaxSamples] = {};
};

inline Samples PointSamples( const float position[3] )
{
	Samples s;
	s.count = 1;
	for ( int k = 0; k < 3; ++k )
		s.point[0][k] = position[k];
	s.weight[0] = 1.0f;
	return s;
}

// A light with a size: kDiskSamples points over a disk of `radius` around
// `position`, facing `receiver` (the center and a ring), equally weighted.
// Point, spot and surface lights shadow through it, so their shadows have
// penumbrae.
constexpr int kDiskSamples = 9;

inline Samples DiskSamples( const float position[3], float radius, const float receiver[3] )
{
	if ( !( radius > 0.0f ) )
		return PointSamples( position );
	float d[3] = {
	    receiver[0] - position[0], receiver[1] - position[1], receiver[2] - position[2] };
	const float len = std::sqrt( d[0] * d[0] + d[1] * d[1] + d[2] * d[2] );
	if ( !( len > 0.0f ) )
		return PointSamples( position );
	for ( float &c : d )
		c /= len;
	// A basis of the disk's plane.
	const float helper[3] = {
	    std::fabs( d[0] ) < 0.9f ? 1.0f : 0.0f, std::fabs( d[0] ) < 0.9f ? 0.0f : 1.0f, 0.0f };
	float u[3] = { helper[1] * d[2] - helper[2] * d[1], helper[2] * d[0] - helper[0] * d[2],
	    helper[0] * d[1] - helper[1] * d[0] };
	const float ul = std::sqrt( u[0] * u[0] + u[1] * u[1] + u[2] * u[2] );
	for ( float &c : u )
		c /= ul;
	const float v[3] = {
	    d[1] * u[2] - d[2] * u[1], d[2] * u[0] - d[0] * u[2], d[0] * u[1] - d[1] * u[0] };
	Samples s;
	s.count = kDiskSamples;
	for ( int k = 0; k < 3; ++k )
		s.point[0][k] = position[k];
	// The ring at the radius that splits the disk's area evenly with the
	// center sample's share (r sqrt(1 - 1/n) ~ its mean radius).
	const float ring = radius * std::sqrt( 1.0f - 1.0f / float( kDiskSamples ) );
	for ( int i = 1; i < kDiskSamples; ++i )
	{
		const float a = 6.28318531f * float( i - 1 ) / float( kDiskSamples - 1 );
		const float c = std::cos( a ) * ring, sn = std::sin( a ) * ring;
		for ( int k = 0; k < 3; ++k )
			s.point[i][k] = position[k] + c * u[k] + sn * v[k];
	}
	for ( int i = 0; i < kDiskSamples; ++i )
		s.weight[i] = 1.0f / float( kDiskSamples );
	return s;
}

// A light that shines along `direction` from infinitely far away, as seen
// from `receiver`.
inline Samples DistantSamples( const float receiver[3], const float direction[3] )
{
	Samples s;
	s.count = 1;
	const float len = std::sqrt(
	    direction[0] * direction[0] + direction[1] * direction[1] + direction[2] * direction[2] );
	for ( int k = 0; k < 3; ++k )
		s.point[0][k] = receiver[k] - ( len > 0.0f ? direction[k] / len : 0.0f ) * kDistantLength;
	s.weight[0] = 1.0f;
	return s;
}

// An area light's samples: the centers of kAreaSamples^2 cells of its
// rectangle, equally weighted.
inline Samples RectSamples( const area_light::Rect &rect )
{
	Samples s;
	for ( int i = 0; i < kAreaSamples; ++i )
		for ( int j = 0; j < kAreaSamples; ++j )
		{
			const float u = -1.0f + ( 2.0f * i + 1.0f ) / kAreaSamples;
			const float v = -1.0f + ( 2.0f * j + 1.0f ) / kAreaSamples;
			for ( int k = 0; k < 3; ++k )
				s.point[s.count][k] = rect.center[k] + u * rect.halfU[k] + v * rect.halfV[k];
			s.weight[s.count] = 1.0f / float( kMaxSamples );
			++s.count;
		}
	return s;
}

// Whether the point is inside the box.
[[nodiscard]] inline bool Contains( const Box &box, const float p[3] )
{
	for ( int a = 0; a < 3; ++a )
	{
		const float *axis = box.axes[a];
		const float len2 = axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2];
		if ( !( len2 > 0.0f ) )
			return false;
		const float t = ( ( p[0] - box.center[0] ) * axis[0] + ( p[1] - box.center[1] ) * axis[1] +
		                    ( p[2] - box.center[2] ) * axis[2] ) /
		                len2;
		if ( t < -1.0f || t > 1.0f )
			return false;
	}
	return true;
}

// Whether the segment from a to b crosses the box (slabs in the box's frame).
[[nodiscard]] inline bool SegmentHits( const Box &box, const float a[3], const float b[3] )
{
	float tMin = 0.0f, tMax = 1.0f;
	for ( int k = 0; k < 3; ++k )
	{
		const float *axis = box.axes[k];
		const float len2 = axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2];
		if ( !( len2 > 0.0f ) )
			return false;
		// Coordinates along this axis in half-extent units.
		const float origin =
		    ( ( a[0] - box.center[0] ) * axis[0] + ( a[1] - box.center[1] ) * axis[1] +
		        ( a[2] - box.center[2] ) * axis[2] ) /
		    len2;
		const float delta =
		    ( ( b[0] - a[0] ) * axis[0] + ( b[1] - a[1] ) * axis[1] + ( b[2] - a[2] ) * axis[2] ) /
		    len2;
		if ( std::fabs( delta ) < 1.0e-12f )
		{
			if ( origin < -1.0f || origin > 1.0f )
				return false;
			continue;
		}
		float t0 = ( -1.0f - origin ) / delta;
		float t1 = ( 1.0f - origin ) / delta;
		if ( t0 > t1 )
		{
			const float t = t0;
			t0 = t1;
			t1 = t;
		}
		tMin = t0 > tMin ? t0 : tMin;
		tMax = t1 < tMax ? t1 : tMax;
		if ( tMin > tMax )
			return false;
	}
	return true;
}

// The bounding sphere radius of a box.
[[nodiscard]] inline float Radius( const Box &box )
{
	float r2 = 0.0f;
	for ( int a = 0; a < 3; ++a )
		r2 += box.axes[a][0] * box.axes[a][0] + box.axes[a][1] * box.axes[a][1] +
		      box.axes[a][2] * box.axes[a][2];
	return std::sqrt( r2 );
}

// Whether the segment passes within `radius` of `center` (a cheap prefilter).
[[nodiscard]] inline bool SegmentNearSphere(
    const float a[3], const float b[3], const float center[3], float radius )
{
	const float d[3] = { b[0] - a[0], b[1] - a[1], b[2] - a[2] };
	const float m[3] = { center[0] - a[0], center[1] - a[1], center[2] - a[2] };
	const float dd = d[0] * d[0] + d[1] * d[1] + d[2] * d[2];
	float t = dd > 0.0f ? ( m[0] * d[0] + m[1] * d[1] + m[2] * d[2] ) / dd : 0.0f;
	t = t < 0.0f ? 0.0f : ( t > 1.0f ? 1.0f : t );
	float dist2 = 0.0f;
	for ( int k = 0; k < 3; ++k )
	{
		const float e = a[k] + t * d[k] - center[k];
		dist2 += e * e;
	}
	return dist2 <= radius * radius;
}

// The fraction of a light's samples that reach the receiver past the boxes
// (the contract's visibility). `self` is the receiver's own entity (0: none):
// its boxes are ignored.
template <typename BoxRange>
[[nodiscard]] float Visibility(
    const float receiver[3], const Samples &samples, const BoxRange &boxes, int self = 0 )
{
	float visible = 0.0f;
	for ( int s = 0; s < samples.count; ++s )
	{
		bool blocked = false;
		for ( const Box &box : boxes )
		{
			if ( self != 0 && box.entity == self )
				continue;
			if ( !SegmentNearSphere( receiver, samples.point[s], box.center, Radius( box ) ) )
				continue;
			if ( SegmentHits( box, receiver, samples.point[s] ) )
			{
				blocked = true;
				break;
			}
		}
		if ( !blocked )
			visible += samples.weight[s];
	}
	return visible;
}

// The light a receiver takes from several lights, each blocked on its own:
// sum_i contribution_i x Visibility( receiver, samples_i ). This is how
// shadows from several lights mix; every consumer uses it (or applies the
// same per-light product). Contributions are RGB.
template <typename BoxRange>
void MixLights( const float receiver[3], const float ( *contributions )[3], const Samples *samples,
    int count, const BoxRange &boxes, int self, float out[3] )
{
	out[0] = out[1] = out[2] = 0.0f;
	for ( int i = 0; i < count; ++i )
	{
		const float v = Visibility( receiver, samples[i], boxes, self );
		for ( int k = 0; k < 3; ++k )
			out[k] += contributions[i][k] * v;
	}
}

// The client's moving objects for the frame, published to the engine (plain
// data across the client/engine boundary), keyed by ( entity, part ).
static const char *const kOccludersVersion = "VEngineOccluders001";
constexpr int kMaxOccluders = 512;

class IOccluders
{
public:
	// Replaces the frame's occluders (at most kMaxOccluders; more are
	// dropped). `count` 0 clears them.
	virtual void SetOccluders( const Box *boxes, int count ) = 0;

protected:
	~IOccluders() {}
};

} // namespace dynamic_occlusion

#endif // RENDER_DYNAMIC_OCCLUSION_H
