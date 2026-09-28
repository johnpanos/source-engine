//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Dynamic lights through open portals (render.portal-lights.v1,
//			RFC 0016 K7 with RFC 0011 G10's portal set).
//
//			An open portal E and its linked portal X join two places. A light
//			L in front of E shines through the pair: a receiver R in front of
//			X sees it at L's image, toLinked(E) applied to L, which lies behind
//			X. The image stands in for L on X's side (same color, radius and
//			falloff; the transform is rigid, so distance to the image is the
//			path length through the portal) but only where the path from L
//			really passes through E's opening. That clip is decided in E's
//			frame: R reaches L through the pair when the segment from L to
//			E's inverse image of R enters E's rectangle from its front
//			(ReachesThroughPortal).
//
//			A light makes an image through E when it is in front of E and its
//			radius reaches E's rectangle (EntersPortal). One hop: images are
//			not imaged again.
//
//			Portals are indirect_portals::PortalInput (render/indirect_portals.h):
//			Source units, forward out of the wall, the row-major 3x4 toLinked.
//			Header-only and plain data: the engine, the client and the render
//			core share one definition.
//
//=============================================================================//

#ifndef RENDER_PORTAL_LIGHTS_H
#define RENDER_PORTAL_LIGHTS_H

#include "render/indirect_portals.h"

#include <cmath>

namespace portal_lights
{

using indirect_portals::PortalInput;

inline float Dot( const float a[3], const float b[3] )
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

// toLinked applied to a point / a direction.
inline void ImagePoint( const PortalInput &portal, const float in[3], float out[3] )
{
	for ( int r = 0; r < 3; ++r )
		out[r] = portal.toLinked[r * 4 + 0] * in[0] + portal.toLinked[r * 4 + 1] * in[1] +
		         portal.toLinked[r * 4 + 2] * in[2] + portal.toLinked[r * 4 + 3];
}

inline void ImageVector( const PortalInput &portal, const float in[3], float out[3] )
{
	for ( int r = 0; r < 3; ++r )
		out[r] = portal.toLinked[r * 4 + 0] * in[0] + portal.toLinked[r * 4 + 1] * in[1] +
		         portal.toLinked[r * 4 + 2] * in[2];
}

// The inverse of ImagePoint (toLinked is rigid: its inverse rotation is the
// transpose).
inline void InverseImagePoint( const PortalInput &portal, const float in[3], float out[3] )
{
	const float q[3] = {
	    in[0] - portal.toLinked[3], in[1] - portal.toLinked[7], in[2] - portal.toLinked[11] };
	for ( int c = 0; c < 3; ++c )
		out[c] = portal.toLinked[c] * q[0] + portal.toLinked[4 + c] * q[1] +
		         portal.toLinked[8 + c] * q[2];
}

// The distance from a point to the portal's rectangle.
inline float DistanceToRectangle( const PortalInput &portal, const float point[3] )
{
	const float d[3] = {
	    point[0] - portal.origin[0], point[1] - portal.origin[1], point[2] - portal.origin[2] };
	const float along[2] = { Dot( d, portal.right ), Dot( d, portal.up ) };
	const float half[2] = { portal.halfWidth, portal.halfHeight };
	float outside = 0.0f;
	for ( int k = 0; k < 2; ++k )
	{
		const float excess = std::fabs( along[k] ) - half[k];
		if ( excess > 0.0f )
			outside += excess * excess;
	}
	const float normal = Dot( d, portal.forward );
	return std::sqrt( outside + normal * normal );
}

// Whether a light of this radius at `light` makes an image through `portal`:
// strictly in front of it, and reaching its rectangle.
inline bool EntersPortal( const PortalInput &portal, const float light[3], float radius )
{
	const float d[3] = {
	    light[0] - portal.origin[0], light[1] - portal.origin[1], light[2] - portal.origin[2] };
	return Dot( d, portal.forward ) > 0.0f && radius > 0.0f &&
	       DistanceToRectangle( portal, light ) < radius;
}

// Whether the segment from `from` to `to` enters the portal's rectangle from
// its front.
inline bool ThroughAperture( const PortalInput &portal, const float from[3], const float to[3] )
{
	float d[3], o[3];
	for ( int c = 0; c < 3; ++c )
	{
		d[c] = to[c] - from[c];
		o[c] = portal.origin[c] - from[c];
	}
	const float facing = Dot( d, portal.forward );
	if ( facing >= 0.0f )
		return false;
	const float t = Dot( o, portal.forward ) / facing;
	if ( t <= 0.0f || t >= 1.0f )
		return false;
	float local[3];
	for ( int c = 0; c < 3; ++c )
		local[c] = from[c] + d[c] * t - portal.origin[c];
	return std::fabs( Dot( local, portal.right ) ) <= portal.halfWidth &&
	       std::fabs( Dot( local, portal.up ) ) <= portal.halfHeight;
}

// Whether a receiver on the linked side sees the light at `light` (in front
// of `entry`) through the pair.
inline bool ReachesThroughPortal(
    const PortalInput &entry, const float light[3], const float receiver[3] )
{
	float back[3];
	InverseImagePoint( entry, receiver, back );
	return ThroughAperture( entry, light, back );
}

} // namespace portal_lights

#endif // RENDER_PORTAL_LIGHTS_H
