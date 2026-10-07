//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.sprite-card.v1: the corners of a SpriteCard particle card,
//			the one definition the render core and the native backend share
//			(RFC 0016 K8 particles).
//
//			The particle library's render operators write one card record per
//			corner (particles/builtin_particle_render_ops.cpp):
//			- a sprite card: the particle's center as the position, the
//			  frame's sheet rectangle (TEXCOORD0) and the next frame's
//			  (TEXCOORD1), frame blend, rotation, radius and yaw (TEXCOORD2)
//			  and the corner id (TEXCOORD3), as spritecard_vsxx.fxc reads them;
//			- a spline card ($splinetype: ropes and sprite trails): the
//			  position is (t along the segment, v, side), TEXCOORD0..3 the
//			  Catmull-Rom points (xyz, width), and in Portal 2 TEXCOORD4 the
//			  sheet range, TEXCOORD5 the end color and TEXCOORD6/7 the end
//			  normals, as splinecard_vsxx.fxc reads them.
//			SpriteCard's vertex shader builds the corners; this header builds
//			them on the CPU from the same inputs. Matrices are D3D9's
//			row-vector convention (p' = p * M), as the material system holds
//			them; corners come out in model space.
//
//			Header-only and dependency-free (render.contracts).
//
//=============================================================================//

#ifndef RENDER_SPRITE_CARD_H
#define RENDER_SPRITE_CARD_H

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace render::sprite_card
{

// A card record as the mesh builder wrote it: the base record's position and
// D3DCOLOR (b, g, r, a), and the eight texture coordinate sets, four floats
// each.
struct Record
{
	float position[3] = {};
	std::uint8_t bgra[4] = { 255, 255, 255, 255 };
	float texCoords[8][4] = {};
};

// SpriteCard's draw constants (spritecard.cpp's dynamic state): the screen
// size limits and fades ($minsize, $maxsize, $startfadesize, $endfadesize)
// and the far fade ($maxdistance less $farfadeinterval, and one over the
// interval), all as SpriteCard's material parameters give them.
struct Sizes
{
	float minSize = 0.0f;
	float maxSize = 20.0f;
	float startFadeSize = 10.0f;
	float endFadeSize = 20.0f;
	float farFadeStart = 99600.0f;
	float farFadeInvRange = 1.0f / 400.0f;
};

// The constants for $maxdistance and $farfadeinterval as spritecard.cpp
// derives them.
inline void SetFarFade( Sizes &sizes, float maxDistance, float farFadeInterval )
{
	sizes.farFadeStart = std::max( 1.0f, maxDistance - farFadeInterval );
	const float range = maxDistance - sizes.farFadeStart;
	sizes.farFadeInvRange = range != 0.0f ? 1.0f / range : 0.0f;
}

enum class Kind : std::uint8_t
{
	kSprite, // spritecard_vsxx
	kSpline  // splinecard_vsxx ($splinetype)
};

struct Frame
{
	float model[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	float view[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	Sizes sizes;
	Kind kind = Kind::kSprite;
	// $orientation: 0 faces the camera, 1 turns about world z, 2 lies
	// parallel to the ground; a spline card faces the camera (0) or turns
	// toward its end normals (3).
	int orientation = 0;
	// A spline card's TEXCOORD4 holds the sheet range and TEXCOORD5 the end
	// color (Portal 2's trails and ropes; a format fact of the mesh).
	bool splineRange = false;
	// ANIMBLEND: the second frame's coordinate comes from TEXCOORD1 and the
	// blend from TEXCOORD2.x ($blendframes on a sprite card).
	bool animBlend = false;

	// Derived by Prepare().
	float modelView[16] = {};
	float invModel[9] = {};
	float eye[3] = {};
};

// One corner: its model-space position, the frame's coordinate and the next
// frame's with the blend between them, and the color the record carries
// (gamma RGB faded by the size and distance fades, linear alpha).
struct Corner
{
	float position[3] = {};
	float uv[2] = {};
	float uv2[2] = {};
	float blend = 0.0f;
	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
};

namespace detail
{

inline float GammaToLinear( float c )
{
	return std::pow( std::max( c, 0.0f ), 2.2f );
}

inline float LinearToGamma( float c )
{
	return std::pow( std::max( c, 0.0f ), 1.0f / 2.2f );
}

inline void ModelToWorld( const Frame &f, const float *p, float *out )
{
	for ( int j = 0; j < 3; ++j )
		out[j] =
		    p[0] * f.model[j] + p[1] * f.model[4 + j] + p[2] * f.model[8 + j] + f.model[12 + j];
}

inline void WorldToModel( const Frame &f, const float *w, float *out )
{
	const float d[3] = { w[0] - f.model[12], w[1] - f.model[13], w[2] - f.model[14] };
	for ( int j = 0; j < 3; ++j )
		out[j] = d[0] * f.invModel[j] + d[1] * f.invModel[3 + j] + d[2] * f.invModel[6 + j];
}

inline void CatmullRom(
    const float *a, const float *b, const float *c, const float *d, float t, int n, float *out )
{
	for ( int k = 0; k < n; ++k )
		out[k] = b[k] + 0.5f * t *
		                    ( c[k] - a[k] +
		                        t * ( 2.0f * a[k] - 5.0f * b[k] + 4.0f * c[k] - d[k] +
		                                t * ( -a[k] + 3.0f * b[k] - 3.0f * c[k] + d[k] ) ) );
}

inline void CatmullRomTangent(
    const float *a, const float *b, const float *c, const float *d, float t, float *out )
{
	for ( int k = 0; k < 3; ++k )
		out[k] = 0.5f * ( c[k] - a[k] +
		                    t * ( 2.0f * a[k] - 5.0f * b[k] + 4.0f * c[k] - d[k] +
		                            t * ( 3.0f * b[k] - a[k] - 3.0f * c[k] + d[k] ) ) +
		                    t * ( 2.0f * a[k] - 5.0f * b[k] + 4.0f * c[k] - d[k] +
		                            2.0f * ( t * ( 3.0f * b[k] - a[k] - 3.0f * c[k] + d[k] ) ) ) );
}

inline void Normalize( float *v )
{
	const float length = std::sqrt( v[0] * v[0] + v[1] * v[1] + v[2] * v[2] );
	if ( length > 0.0f )
	{
		for ( int k = 0; k < 3; ++k )
			v[k] /= length;
	}
}

// spritecard_vsxx: the frames' coordinates past the first.
inline void FrameInputs( const Frame &f, const Record &r, const float *corner, Corner &out )
{
	if ( f.animBlend )
	{
		const float *tc1 = r.texCoords[1];
		out.uv2[0] = tc1[2] + ( tc1[0] - tc1[2] ) * corner[0];
		out.uv2[1] = tc1[3] + ( tc1[1] - tc1[3] ) * corner[1];
		out.blend = r.texCoords[2][0];
	}
	else
	{
		out.uv2[0] = out.uv[0];
		out.uv2[1] = out.uv[1];
		out.blend = 0.0f;
	}
}

inline Corner Sprite( const Frame &f, const Record &r )
{
	Corner out;
	const float *tc0 = r.texCoords[0];
	const float *parms = r.texCoords[2]; // frame blend, rotation, radius, yaw
	const float *corner = r.texCoords[3];
	const float cosYaw = std::cos( parms[3] ), sinYaw = std::sin( parms[3] );
	const float cosRot = std::cos( parms[1] ), sinRot = std::sin( parms[1] );
	const float ix = 2.0f * corner[0] - 1.0f, iy = 2.0f * corner[1] - 1.0f;
	const float x1 = ix * cosRot + iy * sinRot;
	const float y1 = cosRot * iy - sinRot * ix;

	float world[3];
	ModelToWorld( f, r.position, world );
	const float v2p[3] = { world[0] - f.eye[0], world[1] - f.eye[1], world[2] - f.eye[2] };
	const float l = std::sqrt( v2p[0] * v2p[0] + v2p[1] * v2p[1] + v2p[2] * v2p[2] );
	const Sizes &s = f.sizes;
	float rad = std::max( parms[2], s.minSize * l );
	float tint = 1.0f;
	if ( rad > s.startFadeSize * l )
	{
		if ( rad > s.endFadeSize * l )
		{
			tint = 0.0f;
			rad = 0.0f;
		}
		else
		{
			const float range = s.endFadeSize * l - s.startFadeSize * l;
			tint *= range > 0.0f ? 1.0f - ( rad - s.startFadeSize * l ) / range : 0.0f;
		}
	}
	const float farScale =
	    1.0f - std::min( 1.0f, std::max( 0.0f, ( l - s.farFadeStart ) * s.farFadeInvRange ) );
	tint *= farScale;
	if ( farScale <= 0.0f )
		rad = 0.0f;
	rad = std::min( rad, s.maxSize * l );

	if ( f.orientation == 0 )
	{
		// Screen aligned: displaced in view space, which the model-view's
		// (orthonormal) rows take back to model space.
		const float disp[3] = { -x1 * cosYaw, y1, x1 * sinYaw };
		for ( int j = 0; j < 3; ++j )
			out.position[j] = r.position[j] + rad * ( disp[0] * f.modelView[j * 4] +
			                                            disp[1] * f.modelView[j * 4 + 1] +
			                                            disp[2] * f.modelView[j * 4 + 2] );
	}
	else if ( f.orientation == 1 )
	{
		// Z aligned: turns about world z to face the eye.
		if ( l > rad / 2.0f )
		{
			float right[3] = { -v2p[1], v2p[0], 0.0f };
			const float length = std::sqrt( right[0] * right[0] + right[1] * right[1] );
			if ( length > 0.0f )
			{
				right[0] /= length;
				right[1] /= length;
			}
			const float rx = right[0] * cosYaw + right[1] * sinYaw;
			const float ry = right[1] * cosYaw - right[0] * sinYaw;
			world[0] += x1 * rad * rx;
			world[1] += x1 * rad * ry;
			world[2] += y1 * rad;
			if ( l < rad * 2.0f )
			{
				const float t =
				    std::min( 1.0f, std::max( 0.0f, ( l - rad / 2.0f ) / ( rad / 2.0f ) ) );
				tint *= t * t * ( 3.0f - 2.0f * t );
			}
		}
		WorldToModel( f, world, out.position );
	}
	else
	{
		// Parallel to the ground, in model space, at the unclamped radius.
		out.position[0] = r.position[0] + parms[2] * y1;
		out.position[1] = r.position[1] + parms[2] * x1;
		out.position[2] = r.position[2];
	}

	const float gammaTint = LinearToGamma( tint );
	out.color[0] = r.bgra[2] / 255.0f * gammaTint;
	out.color[1] = r.bgra[1] / 255.0f * gammaTint;
	out.color[2] = r.bgra[0] / 255.0f * gammaTint;
	out.color[3] = r.bgra[3] / 255.0f * tint;
	out.uv[0] = tc0[2] + ( tc0[0] - tc0[2] ) * corner[0];
	out.uv[1] = tc0[3] + ( tc0[1] - tc0[3] ) * corner[1];
	FrameInputs( f, r, corner, out );
	return out;
}

inline Corner Spline( const Frame &f, const Record &r )
{
	Corner out;
	const float *tc = &r.texCoords[0][0];
	const float t = r.position[0], v = r.position[1], side = r.position[2];
	float posrad[4];
	CatmullRom( tc, tc + 4, tc + 8, tc + 12, t, 4, posrad );
	float v2p[3] = { 0.0f, 0.0f, 1.0f };
	if ( f.orientation == 0 )
	{
		for ( int k = 0; k < 3; ++k )
			v2p[k] = posrad[k] - f.eye[k];
	}
	else if ( f.orientation == 3 )
	{
		const float *normal0 = tc + 24;
		const float *normal1 = tc + 28;
		for ( int k = 0; k < 3; ++k )
			v2p[k] = normal0[k] + ( normal1[k] - normal0[k] ) * t;
	}
	float tangent[3];
	CatmullRomTangent( tc, tc + 4, tc + 8, tc + 12, t, tangent );
	Normalize( tangent );
	float ofs[3] = { v2p[1] * tangent[2] - v2p[2] * tangent[1],
	    v2p[2] * tangent[0] - v2p[0] * tangent[2], v2p[0] * tangent[1] - v2p[1] * tangent[0] };
	Normalize( ofs );
	float world[3];
	for ( int k = 0; k < 3; ++k )
		world[k] = posrad[k] + ofs[k] * ( posrad[3] * ( side - 0.5f ) );
	WorldToModel( f, world, out.position );

	float rgba[4] = {
	    r.bgra[2] / 255.0f, r.bgra[1] / 255.0f, r.bgra[0] / 255.0f, r.bgra[3] / 255.0f };
	if ( f.splineRange )
	{
		const float *range = tc + 16;
		const float *endColor = tc + 20;
		out.uv[0] = range[2] + ( range[0] - range[2] ) * side;
		out.uv[1] = range[1] + ( range[3] - range[1] ) * v;
		for ( int k = 0; k < 3; ++k )
		{
			const float c1 = GammaToLinear( rgba[k] );
			const float c2 = GammaToLinear( endColor[k] );
			rgba[k] = LinearToGamma( c1 + ( c2 - c1 ) * t );
		}
		rgba[3] = rgba[3] + ( endColor[3] - rgba[3] ) * t;
	}
	else
	{
		out.uv[0] = 1.0f - side;
		out.uv[1] = v;
	}
	for ( int k = 0; k < 4; ++k )
		out.color[k] = rgba[k];
	// A spline card's frame coordinates come from its sheet range alone.
	out.uv2[0] = out.uv[0];
	out.uv2[1] = out.uv[1];
	out.blend = 0.0f;
	return out;
}

} // namespace detail

// Derives the model-view, the inverse model rotation and the eye.
inline void Prepare( Frame &f )
{
	for ( int i = 0; i < 4; ++i )
		for ( int j = 0; j < 4; ++j )
		{
			float sum = 0.0f;
			for ( int k = 0; k < 4; ++k )
				sum += f.model[i * 4 + k] * f.view[k * 4 + j];
			f.modelView[i * 4 + j] = sum;
		}
	// The 3x3 of the row-vector model matrix, inverted.
	const float *m = f.model;
	const float a = m[0], b = m[1], c = m[2];
	const float d = m[4], e = m[5], g = m[6];
	const float h = m[8], i = m[9], k = m[10];
	const float det = a * ( e * k - g * i ) - b * ( d * k - g * h ) + c * ( d * i - e * h );
	const float r = std::fabs( det ) > 1e-20f ? 1.0f / det : 0.0f;
	f.invModel[0] = ( e * k - g * i ) * r;
	f.invModel[1] = ( c * i - b * k ) * r;
	f.invModel[2] = ( b * g - c * e ) * r;
	f.invModel[3] = ( g * h - d * k ) * r;
	f.invModel[4] = ( a * k - c * h ) * r;
	f.invModel[5] = ( c * d - a * g ) * r;
	f.invModel[6] = ( d * i - e * h ) * r;
	f.invModel[7] = ( b * h - a * i ) * r;
	f.invModel[8] = ( a * e - b * d ) * r;
	// The eye is where the view takes to its origin: e * R + t = 0 with R
	// orthonormal, e_j = -sum_k t_k R[j][k] (cEyePos).
	const float *v = f.view;
	for ( int j = 0; j < 3; ++j )
		f.eye[j] = -( v[12] * v[j * 4] + v[13] * v[j * 4 + 1] + v[14] * v[j * 4 + 2] );
}

// The corner one card record describes, for a Prepare()d frame.
inline Corner Expand( const Frame &f, const Record &record )
{
	return f.kind == Kind::kSpline ? detail::Spline( f, record ) : detail::Sprite( f, record );
}

} // namespace render::sprite_card

#endif // RENDER_SPRITE_CARD_H
