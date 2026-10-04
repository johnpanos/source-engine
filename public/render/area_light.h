//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Area lights (render.area-light.v1, RFC 0011 light set v2): light
//          that leaves an emitting surface, not a point. An area light is an
//          oriented rectangle of uniform radiance; emissive model materials
//          ($selfillum) and lit screens are published as area lights.
//
//          The definitions here are the contract every consumer evaluates
//          (the CPU lightmap and model-light paths, and the render core's
//          clustered LTC evaluation): the rectangle, its radiance, its reach,
//          and the exact irradiance it gives a point (IrradianceAt), which is
//          also the oracle the consumers are judged against.
//
//          Units. Radiance is linear RGB in the lightmap's unit: an emitter
//          of radiance 1 shows the brightness of a white surface under a
//          lightmap value of 1, so a selfillum texel's linear value is its
//          radiance. A receiver's light is in the same unit: a surface
//          surrounded by an emitter of radiance L on its whole hemisphere
//          receives L (irradiance / pi).
//
//          Exact light. A point p with normal n receives L times the form
//          factor from p to the rectangle clipped to n's hemisphere:
//          F = (1 / 2pi) sum_k gamma_k (n . g_k), over the clipped polygon's
//          edges, gamma_k the angle the edge subtends at p and g_k the unit
//          normal of the plane through p and the edge (Lambert's formula).
//          A one-sided rectangle emits only on its front (normal U x V).
//
//          Relation to point lights (light_set::RuntimeLight). Both are in
//          the same unit. An inverse-square light of color C gives a receiver
//          facing it C (100 / d)^2 cos; a legacy dlight gives C times
//          light_set::Falloff. An area light of radiance L and area A seen
//          from far away on its axis gives L A cos_e cos_r / ( pi d^2 ), so
//          it carries the power of an inverse-square light of color
//          C = L A / ( pi 100^2 ) (on its axis; kPointEquivalentScale).
//
//          Identity and count. A light keeps its light-set ID while it stays
//          lit (the engine keeps its dlight slot by the client's key; the
//          light set keys IDs by slot and key). The client publishes at most
//          kMaxFrameAreaLights, most important first; the first
//          kMaxAreaLights carry dlight slots (the CPU lightmap and model
//          paths), and a snapshot holds them all (the render core evaluates
//          them per pixel).
//
//          Reach. Beyond its reach a light gives nothing, so consumers can
//          bound the surfaces and clusters it touches. The reach is where the
//          brightest channel's irradiance on axis would fall to
//          kReachThreshold, capped at kMaxReach; the light is windowed by
//          ( 1 - (d / reach)^4 )^2, d the distance to the rectangle, so it
//          goes to 0 smoothly there. The window is part of the definition:
//          every consumer applies it.
//
//          Tier0-free C++ so the engine, the client and the conformance suite
//          (unittests/rendertest/test_area_light.cpp) share one definition.
//
//===========================================================================//

#ifndef RENDER_AREA_LIGHT_H
#define RENDER_AREA_LIGHT_H

#include <cmath>

namespace energy_field
{
struct Surface;
}

namespace area_light
{

constexpr float kPi = 3.14159265358979323846f;

// Irradiance (in the lightmap unit) below which a light is not worth a
// surface's rebuild: Source's MIN_LIGHTING_VALUE.
constexpr float kReachThreshold = 1.0f / 256.0f;
// The farthest any area light reaches.
constexpr float kMaxReach = 768.0f;

// C = L A kPointEquivalentScale: the inverse-square color of equal on-axis
// far-field light (see the header comment).
constexpr float kPointEquivalentScale = 1.0f / ( kPi * 100.0f * 100.0f );

struct Rect
{
	float center[3] = {};
	float halfU[3] = {}; // half extent along the rectangle's first axis
	float halfV[3] = {}; // half extent along its second; front is U x V
	bool twoSided = false;
};

struct AreaLight
{
	Rect rect;
	float radiance[3] = {}; // linear RGB, the lightmap unit
	float reach = 0.0f;     // Reach( rect, radiance ); 0 gives no light
};

namespace detail
{
inline float Dot( const float a[3], const float b[3] )
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}
inline void Cross( const float a[3], const float b[3], float out[3] )
{
	out[0] = a[1] * b[2] - a[2] * b[1];
	out[1] = a[2] * b[0] - a[0] * b[2];
	out[2] = a[0] * b[1] - a[1] * b[0];
}
inline float Length( const float a[3] )
{
	return std::sqrt( Dot( a, a ) );
}
} // namespace detail

// The rectangle's area.
[[nodiscard]] inline float Area( const Rect &rect )
{
	float n[3];
	detail::Cross( rect.halfU, rect.halfV, n );
	return 4.0f * detail::Length( n );
}

// The unit front normal (U x V); zero for a degenerate rectangle.
inline void Normal( const Rect &rect, float out[3] )
{
	detail::Cross( rect.halfU, rect.halfV, out );
	const float length = detail::Length( out );
	for ( int k = 0; k < 3; ++k )
		out[k] = length > 0.0f ? out[k] / length : 0.0f;
}

// Corners in order around the rectangle (counterclockwise seen from the front).
inline void Corners( const Rect &rect, float out[4][3] )
{
	static const float kSigns[4][2] = { { -1, -1 }, { 1, -1 }, { 1, 1 }, { -1, 1 } };
	for ( int c = 0; c < 4; ++c )
		for ( int k = 0; k < 3; ++k )
			out[c][k] =
			    rect.center[k] + kSigns[c][0] * rect.halfU[k] + kSigns[c][1] * rect.halfV[k];
}

// The point of the rectangle nearest p.
inline void ClosestPoint( const Rect &rect, const float p[3], float out[3] )
{
	float d[3];
	for ( int k = 0; k < 3; ++k )
		d[k] = p[k] - rect.center[k];
	const float uu = detail::Dot( rect.halfU, rect.halfU );
	const float vv = detail::Dot( rect.halfV, rect.halfV );
	float s = uu > 0.0f ? detail::Dot( d, rect.halfU ) / uu : 0.0f;
	float t = vv > 0.0f ? detail::Dot( d, rect.halfV ) / vv : 0.0f;
	s = s < -1.0f ? -1.0f : ( s > 1.0f ? 1.0f : s );
	t = t < -1.0f ? -1.0f : ( t > 1.0f ? 1.0f : t );
	for ( int k = 0; k < 3; ++k )
		out[k] = rect.center[k] + s * rect.halfU[k] + t * rect.halfV[k];
}

[[nodiscard]] inline float DistanceTo( const Rect &rect, const float p[3] )
{
	float q[3];
	ClosestPoint( rect, p, q );
	const float d[3] = { p[0] - q[0], p[1] - q[1], p[2] - q[2] };
	return detail::Length( d );
}

// The reach of a rectangle of this radiance (see the header comment).
[[nodiscard]] inline float Reach( const Rect &rect, const float radiance[3] )
{
	float brightest = radiance[0];
	brightest = radiance[1] > brightest ? radiance[1] : brightest;
	brightest = radiance[2] > brightest ? radiance[2] : brightest;
	const float area = Area( rect );
	if ( !( brightest > 0.0f ) || !( area > 0.0f ) )
		return 0.0f;
	// On axis, far away, F -> A / ( pi d^2 ).
	const float reach = std::sqrt( brightest * area / ( kPi * kReachThreshold ) );
	return reach < kMaxReach ? reach : kMaxReach;
}

// The window at distance d from the rectangle (1 at the rectangle, 0 at reach).
[[nodiscard]] inline float Window( float distance, float reach )
{
	if ( !( reach > 0.0f ) || distance >= reach )
		return 0.0f;
	const float ratio = distance * distance / ( reach * reach );
	const float edge = 1.0f - ratio * ratio;
	return edge * edge;
}

// Whether p sees the emitting side (a one-sided light lights only its front).
[[nodiscard]] inline bool Faces( const Rect &rect, const float p[3] )
{
	if ( rect.twoSided )
		return true;
	float n[3];
	Normal( rect, n );
	const float d[3] = { p[0] - rect.center[0], p[1] - rect.center[1], p[2] - rect.center[2] };
	return detail::Dot( n, d ) > 0.0f;
}

// The vector form factor of a polygon seen from p: (1 / 2pi) sum_k gamma_k g_k,
// oriented toward the polygon. Its dot with a unit normal is the form factor
// when the polygon lies in that normal's hemisphere.
inline void VectorFormFactor(
    const float ( *corners )[3], int count, const float p[3], float out[3] )
{
	out[0] = out[1] = out[2] = 0.0f;
	if ( count < 3 )
		return;
	float r[8][3];
	float toward[3] = { 0, 0, 0 };
	for ( int c = 0; c < count; ++c )
	{
		for ( int k = 0; k < 3; ++k )
		{
			r[c][k] = corners[c][k] - p[k];
			toward[k] += r[c][k];
		}
		const float length = detail::Length( r[c] );
		if ( !( length > 0.0f ) )
			return; // p on a corner: undefined, give nothing
		for ( int k = 0; k < 3; ++k )
			r[c][k] /= length;
	}
	for ( int c = 0; c < count; ++c )
	{
		const float *a = r[c];
		const float *b = r[( c + 1 ) % count];
		float g[3];
		detail::Cross( a, b, g );
		const float sine = detail::Length( g );
		if ( !( sine > 0.0f ) )
			continue;
		const float cosine = detail::Dot( a, b );
		const float gamma = std::atan2( sine, cosine );
		for ( int k = 0; k < 3; ++k )
			out[k] += gamma * g[k] / sine;
	}
	for ( int k = 0; k < 3; ++k )
		out[k] /= 2.0f * kPi;
	// The vector irradiance points toward the source.
	if ( detail::Dot( out, toward ) < 0.0f )
		for ( int k = 0; k < 3; ++k )
			out[k] = -out[k];
}

// Clips a polygon to the half space dot(n, x - p) >= 0 (Sutherland-Hodgman);
// returns the clipped count (at most count + 1).
inline int ClipToHemisphere(
    const float ( *in )[3], int count, const float p[3], const float n[3], float ( *out )[3] )
{
	int written = 0;
	for ( int c = 0; c < count; ++c )
	{
		const float *a = in[c];
		const float *b = in[( c + 1 ) % count];
		const float da = ( a[0] - p[0] ) * n[0] + ( a[1] - p[1] ) * n[1] + ( a[2] - p[2] ) * n[2];
		const float db = ( b[0] - p[0] ) * n[0] + ( b[1] - p[1] ) * n[1] + ( b[2] - p[2] ) * n[2];
		if ( da >= 0.0f )
		{
			for ( int k = 0; k < 3; ++k )
				out[written][k] = a[k];
			++written;
		}
		if ( ( da >= 0.0f ) != ( db >= 0.0f ) )
		{
			const float t = da / ( da - db );
			for ( int k = 0; k < 3; ++k )
				out[written][k] = a[k] + t * ( b[k] - a[k] );
			++written;
		}
	}
	return written;
}

// The exact form factor from a point p with unit normal n to the rectangle:
// clipped to n's hemisphere, and 0 behind a one-sided light.
[[nodiscard]] inline float FormFactor( const Rect &rect, const float p[3], const float n[3] )
{
	if ( !Faces( rect, p ) )
		return 0.0f;
	float corners[4][3];
	Corners( rect, corners );
	float clipped[5][3];
	const int count = ClipToHemisphere( corners, 4, p, n, clipped );
	if ( count < 3 )
		return 0.0f;
	float v[3];
	VectorFormFactor( clipped, count, p, v );
	const float f = detail::Dot( v, n );
	return f > 0.0f ? f : 0.0f;
}

// The light (lightmap unit) a point with unit normal n receives: radiance
// times the form factor times the window. This is the contract's oracle.
inline void IrradianceAt( const AreaLight &light, const float p[3], const float n[3], float out[3] )
{
	const float window = Window( DistanceTo( light.rect, p ), light.reach );
	const float f = window > 0.0f ? FormFactor( light.rect, p, n ) * window : 0.0f;
	for ( int k = 0; k < 3; ++k )
		out[k] = light.radiance[k] * f;
}

// A point light standing in for an area light at one receiver (a model's
// lighting origin, which has no single normal): placed along the direction of
// the unclipped vector form factor at the rectangle's distance, with the
// intensity that gives the receiver, facing that direction, the area light's
// full light there. `intensityAt100` is the light a receiver facing it at the
// inverse-square reference distance (100 units) gets, so an inverse-square
// light of that intensity reproduces the area light at the receiver.
struct Representative
{
	bool lit = false;
	float position[3] = {};
	float intensityAt100[3] = {};
};

constexpr float kRepresentativeReferenceDistance = 100.0f;
// Closer than this, the stand-in keeps this distance (a receiver touching the
// rectangle must not get an unbounded inverse-square light).
constexpr float kRepresentativeMinDistance = 1.0f;

[[nodiscard]] inline Representative RepresentativeAt( const AreaLight &light, const float p[3] )
{
	Representative rep;
	if ( !Faces( light.rect, p ) )
		return rep;
	float distance = DistanceTo( light.rect, p );
	const float window = Window( distance, light.reach );
	if ( !( window > 0.0f ) )
		return rep;
	float corners[4][3];
	Corners( light.rect, corners );
	float v[3];
	VectorFormFactor( corners, 4, p, v );
	const float f = detail::Length( v );
	if ( !( f > 0.0f ) )
		return rep;
	distance = distance > kRepresentativeMinDistance ? distance : kRepresentativeMinDistance;
	const float scale = f * window * distance * distance /
	                    ( kRepresentativeReferenceDistance * kRepresentativeReferenceDistance );
	for ( int k = 0; k < 3; ++k )
	{
		rep.position[k] = p[k] + v[k] / f * distance;
		rep.intensityAt100[k] = light.radiance[k] * scale;
	}
	rep.lit = true;
	return rep;
}

// The client's area lights for the frame, published to the engine (plain data
// across the client/engine boundary). The engine carries each as a dlight
// slot flagged DLIGHT_AREA (public/dlight.h), so the lightmap, model-light and
// light-set paths find it; `keys` are the client's stable identities (the
// same emitter keeps its key while it stays lit).
static const char *const kAreaLightsVersion = "VEngineAreaLights002";
// At most this many area lights carry dlight slots at once (the CPU lightmap
// and model paths' budget, RFC 0011).
constexpr int kMaxAreaLights = 8;
// At most this many area lights in a frame (the light set's, which the render
// core evaluates per pixel; the client's r_area_lights may ask for fewer).
constexpr int kMaxFrameAreaLights = 64;

// A self-illuminated world face or overlay the engine found (world_emitters):
// its rectangle and radiance at tint 1 (reach unset), and its material, whose
// live $selfillumtint the client applies.
struct WorldEmitterInfo
{
	AreaLight light;
	char material[128] = {};
};

class IAreaLights
{
public:
	// The map's self-illuminated world faces and overlays; returns how many
	// there are (writing at most `max`).
	virtual int GetWorldEmitters( WorldEmitterInfo *out, int max ) = 0;

	// Replaces the frame's area lights, most important first (at most
	// kMaxFrameAreaLights; more are dropped). The first kMaxAreaLights take
	// dlight slots. A light left out goes dark; `count` 0 clears them all.
	virtual void SetAreaLights( const AreaLight *lights, const int *keys, int count ) = 0;

protected:
	~IAreaLights() {}
};

// v3 preserves v2's layout/vtable. New sources can explicitly target the core
// without allocating CPU lightmap/model stand-in slots. One publication still
// replaces the complete frame; receiver policy is independent of light identity.
static const char *const kAreaLightsFrameVersion = "VEngineAreaLights003";
class IAreaLights3 : public IAreaLights
{
public:
	virtual void SetFrameAreaLights( const AreaLight *lights, const int *keys,
	    const bool *coreOnly, int count ) = 0;

	// Largest rectangular SolidEnergy face of a brush model, in model space.
	// No material-name list, collision-bound substitution or inferred emission.
	// False when the model has no supported face. The caller owns the copy.
	virtual bool GetEnergyFieldSurface( int modelIndex, energy_field::Surface &out ) = 0;
};

// Authored self-illuminated surface geometry for the core source policy.
// Positions are model-local for a brush, world-space for modelIndex 0.
// group joins fragments of one surface; entity identifies an overlay's live
// proxy owner (-1 when none). Copies contain no borrowed material or entity.
struct EmissiveTriangle
{
	float position[3][3] = {};
	float uv[3][2] = {};
	int group = 0;
	int entity = -1;
	char material[128] = {};
};

// v4 preserves both earlier vtables. Geometry ingress performs no lighting.
inline constexpr const char *kAreaLightsGeometryVersion = "VEngineAreaLights004";
class IAreaLights4 : public IAreaLights3
{
public:
	// Returns the required count, writing at most max. Zero denotes no authored
	// source geometry. World faces already represented by baked texture lights
	// are excluded to avoid publishing their direct light a second time.
	virtual int GetEmissiveTriangles( int modelIndex, EmissiveTriangle *out, int max ) = 0;
};

} // namespace area_light

#endif // RENDER_AREA_LIGHT_H
