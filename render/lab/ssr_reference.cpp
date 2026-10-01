//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's reference for render.pass.ssr; see ssr_reference.h.
//
//=============================================================================//

#include "ssr_reference.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace render::lab
{

namespace
{

using Vec4 = std::array<double, 4>;
using Mat4 = std::array<double, 16>; // row-major

Mat4 ToDouble( const math::float4x4 &m )
{
	Mat4 out{};
	for ( int r = 0; r < 4; ++r )
	{
		out[r * 4 + 0] = m.rows[r].x;
		out[r * 4 + 1] = m.rows[r].y;
		out[r * 4 + 2] = m.rows[r].z;
		out[r * 4 + 3] = m.rows[r].w;
	}
	return out;
}

// Gauss-Jordan with partial pivoting.
Mat4 Invert( Mat4 a )
{
	Mat4 inv{};
	for ( int i = 0; i < 4; ++i )
		inv[i * 4 + i] = 1.0;
	for ( int c = 0; c < 4; ++c )
	{
		int pivot = c;
		for ( int r = c + 1; r < 4; ++r )
			if ( std::fabs( a[r * 4 + c] ) > std::fabs( a[pivot * 4 + c] ) )
				pivot = r;
		for ( int k = 0; k < 4; ++k )
		{
			std::swap( a[c * 4 + k], a[pivot * 4 + k] );
			std::swap( inv[c * 4 + k], inv[pivot * 4 + k] );
		}
		const double scale = 1.0 / a[c * 4 + c];
		for ( int k = 0; k < 4; ++k )
		{
			a[c * 4 + k] *= scale;
			inv[c * 4 + k] *= scale;
		}
		for ( int r = 0; r < 4; ++r )
		{
			if ( r == c )
				continue;
			const double f = a[r * 4 + c];
			for ( int k = 0; k < 4; ++k )
			{
				a[r * 4 + k] -= f * a[c * 4 + k];
				inv[r * 4 + k] -= f * inv[c * 4 + k];
			}
		}
	}
	return inv;
}

Vec4 Apply( const Mat4 &m, const Vec4 &v )
{
	Vec4 out{};
	for ( int r = 0; r < 4; ++r )
		out[r] = m[r * 4] * v[0] + m[r * 4 + 1] * v[1] + m[r * 4 + 2] * v[2] + m[r * 4 + 3] * v[3];
	return out;
}

struct Vec3
{
	double x = 0, y = 0, z = 0;
};

Vec3 operator+( Vec3 a, Vec3 b )
{
	return { a.x + b.x, a.y + b.y, a.z + b.z };
}
Vec3 operator-( Vec3 a, Vec3 b )
{
	return { a.x - b.x, a.y - b.y, a.z - b.z };
}
Vec3 operator*( Vec3 a, double s )
{
	return { a.x * s, a.y * s, a.z * s };
}
double Dot( Vec3 a, Vec3 b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
Vec3 Normalized( Vec3 a )
{
	return a * ( 1.0 / std::sqrt( Dot( a, a ) ) );
}

double Smoothstep( double edge0, double edge1, double x )
{
	const double t = std::clamp( ( x - edge0 ) / ( edge1 - edge0 ), 0.0, 1.0 );
	return t * t * ( 3.0 - 2.0 * t );
}

struct View
{
	std::uint32_t width, height;
	Mat4 toClip;
	Mat4 fromClip;

	// Screen position (pixels from the top left) and depth to NDC and back.
	Vec3 Ndc( double sx, double sy, double depth ) const
	{
		return { sx / width * 2.0 - 1.0, 1.0 - sy / height * 2.0, depth };
	}
	Vec3 World( double sx, double sy, double depth ) const
	{
		const Vec3 n = Ndc( sx, sy, depth );
		const Vec4 h = Apply( fromClip, { n.x, n.y, n.z, 1.0 } );
		return { h[0] / h[3], h[1] / h[3], h[2] / h[3] };
	}
	// The clip w (view-space distance along the view axis) of a screen point.
	double W( double sx, double sy, double depth ) const
	{
		const Vec3 n = Ndc( sx, sy, depth );
		return 1.0 / Apply( fromClip, { n.x, n.y, n.z, 1.0 } )[3];
	}
};

// Bilinear at a level, clamped to its edges; (x, y) in that level's texels.
void Bilinear( const SsrPyramidLevel &level, double x, double y, double out[3] )
{
	x -= 0.5;
	y -= 0.5;
	const double fx0 = std::floor( x ), fy0 = std::floor( y );
	const double fx = x - fx0, fy = y - fy0;
	const auto at = [&]( double tx, double ty, int c )
	{
		const auto ix = std::uint32_t( std::clamp( tx, 0.0, double( level.width - 1 ) ) );
		const auto iy = std::uint32_t( std::clamp( ty, 0.0, double( level.height - 1 ) ) );
		return level.rgba[( std::size_t( iy ) * level.width + ix ) * 4 + std::size_t( c )];
	};
	for ( int c = 0; c < 3; ++c )
	{
		const double top = at( fx0, fy0, c ) * ( 1 - fx ) + at( fx0 + 1, fy0, c ) * fx;
		const double bottom = at( fx0, fy0 + 1, c ) * ( 1 - fx ) + at( fx0 + 1, fy0 + 1, c ) * fx;
		out[c] = top * ( 1 - fy ) + bottom * fy;
	}
}

} // namespace

std::vector<SsrPyramidLevel> ReferencePyramid(
    const SsrReferenceInputs &inputs, std::uint32_t maxMip )
{
	std::vector<SsrPyramidLevel> levels( 1 );
	levels[0].width = inputs.width;
	levels[0].height = inputs.height;
	levels[0].rgba.assign( inputs.lit.begin(), inputs.lit.end() );
	while ( levels.size() <= maxMip && ( levels.back().width > 1 || levels.back().height > 1 ) )
	{
		const SsrPyramidLevel &from = levels.back();
		SsrPyramidLevel next;
		next.width = std::max( 1u, from.width / 2 );
		next.height = std::max( 1u, from.height / 2 );
		next.rgba.resize( std::size_t( next.width ) * next.height * 4 );
		for ( std::uint32_t y = 0; y < next.height; ++y )
		{
			for ( std::uint32_t x = 0; x < next.width; ++x )
			{
				for ( int c = 0; c < 4; ++c )
				{
					double sum = 0.0;
					for ( std::uint32_t b = 0; b < 2; ++b )
						for ( std::uint32_t a = 0; a < 2; ++a )
						{
							const std::uint32_t sx = std::min( 2 * x + a, from.width - 1 );
							const std::uint32_t sy = std::min( 2 * y + b, from.height - 1 );
							sum += from.rgba[( std::size_t( sy ) * from.width + sx ) * 4 +
							                 std::size_t( c )];
						}
					next.rgba[( std::size_t( y ) * next.width + x ) * 4 + std::size_t( c )] =
					    sum * 0.25;
				}
			}
		}
		levels.push_back( std::move( next ) );
	}
	return levels;
}

std::vector<SsrReferencePixel> ReferenceSsr( const SsrReferenceInputs &inputs,
    const pass::ssr::SsrParams &params, SsrReferenceDefect defect,
    const std::array<double, 3> &tilt )
{
	const std::uint32_t W = inputs.width, H = inputs.height;
	View view{ W, H, ToDouble( inputs.toClip ), {} };
	view.fromClip = Invert( view.toClip );
	const std::vector<SsrPyramidLevel> pyramid = ReferencePyramid( inputs, params.maxMip );
	const double maxLevel = double( pyramid.size() - 1 );
	const Vec3 eye{ inputs.eye[0], inputs.eye[1], inputs.eye[2] };
	const auto depthAt = [&]( std::uint32_t x, std::uint32_t y )
	{
		return double( inputs.depth[std::size_t( y ) * W + x] );
	};

	std::vector<SsrReferencePixel> result( std::size_t( W ) * H );
	for ( std::uint32_t py = 0; py < H; ++py )
	{
		for ( std::uint32_t px = 0; px < W; ++px )
		{
			const std::size_t index = std::size_t( py ) * W + px;
			SsrReferencePixel &pixel = result[index];
			const float *lit = &inputs.lit[index * 4];
			std::copy( lit, lit + 4, pixel.out );
			const double depth = depthAt( px, py );
			const float *nr = &inputs.normalRoughness[index * 4];
			const double roughness = nr[2];
			if ( !( depth < 1.0 ) || !( roughness < params.roughnessCutoff ) )
				continue;
			pixel.traced = true;

			// 1. The pixel's surface and the reflected ray.
			const double cx = px + 0.5, cy = py + 0.5;
			const Vec3 P = view.World( cx, cy, depth );
			float decoded[3];
			pass::ssr::OctDecode( { nr[0], nr[1] }, decoded );
			const Vec3 N{ decoded[0], decoded[1], decoded[2] };
			const Vec3 V = Normalized( eye - P );
			const Vec3 R =
			    Normalized( N * ( 2.0 * Dot( N, V ) ) - V + Vec3{ tilt[0], tilt[1], tilt[2] } );
			if ( !( Dot( R, N ) > 0.0 ) )
				continue;

			// 2. The origin clear of the surface, and the screen segment.
			const Vec3 right = view.World( cx + 1.0, cy, depth );
			const double footprint = std::sqrt( Dot( right - P, right - P ) );
			const Vec3 O = P + N * footprint;
			const Vec4 c0 = Apply( view.toClip, { O.x, O.y, O.z, 1.0 } );
			const Vec4 dc = Apply( view.toClip, { R.x, R.y, R.z, 0.0 } );
			if ( !( c0[3] > 0.0 ) || c0[2] < 0.0 )
				continue; // the origin is behind the near plane
			double tEnd = 1e7;
			if ( dc[2] < 0.0 )
				tEnd = std::min( tEnd, -c0[2] / dc[2] ); // the near plane
			if ( dc[2] - dc[3] > 0.0 )
				tEnd = std::min( tEnd, ( c0[3] - c0[2] ) / ( dc[2] - dc[3] ) ); // the far plane
			if ( dc[3] < 0.0 )
				tEnd = std::min( tEnd, 0.999999 * ( -c0[3] / dc[3] ) ); // w stays positive
			const Vec4 c1{ c0[0] + tEnd * dc[0], c0[1] + tEnd * dc[1], c0[2] + tEnd * dc[2],
			    c0[3] + tEnd * dc[3] };
			// Screen coordinates (pixels) and depth of both ends.
			const auto screen = [&]( const Vec4 &c )
			{
				return Vec3{
				    ( c[0] / c[3] + 1.0 ) * 0.5 * W, ( 1.0 - c[1] / c[3] ) * 0.5 * H, c[2] / c[3] };
			};
			const Vec3 s0 = screen( c0 ), s1 = screen( c1 );
			const Vec3 delta = s1 - s0;
			// Clip u in [0, 1] to the screen rectangle (Liang-Barsky).
			double uLow = 0.0, uHigh = 1.0;
			const auto clip = [&]( double p, double q )
			{
				if ( p == 0.0 )
					return q >= 0.0;
				const double r = q / p;
				if ( p < 0.0 )
					uLow = std::max( uLow, r );
				else
					uHigh = std::min( uHigh, r );
				return uLow <= uHigh;
			};
			if ( !clip( -delta.x, s0.x ) || !clip( delta.x, W - s0.x ) || !clip( -delta.y, s0.y ) ||
			     !clip( delta.y, H - s0.y ) || uLow > 0.0 )
				continue; // the origin is off the screen, or the ray never is on it

			// 3. Walk every texel the segment crosses (Amanatides and Woo).
			const auto at = [&]( double u )
			{
				return Vec3{ s0.x + u * delta.x, s0.y + u * delta.y, s0.z + u * delta.z };
			};
			std::int64_t tx = std::int64_t( std::floor( s0.x ) );
			std::int64_t ty = std::int64_t( std::floor( s0.y ) );
			const int stepX = delta.x > 0 ? 1 : -1;
			const int stepY = delta.y > 0 ? 1 : -1;
			const double inf = std::numeric_limits<double>::infinity();
			const double uDeltaX = delta.x != 0.0 ? std::fabs( 1.0 / delta.x ) : inf;
			const double uDeltaY = delta.y != 0.0 ? std::fabs( 1.0 / delta.y ) : inf;
			double uNextX =
			    delta.x != 0.0
			        ? ( ( delta.x > 0 ? double( tx + 1 ) : double( tx ) ) - s0.x ) / delta.x
			        : inf;
			double uNextY =
			    delta.y != 0.0
			        ? ( ( delta.y > 0 ? double( ty + 1 ) : double( ty ) ) - s0.y ) / delta.y
			        : inf;
			double uEnter = 0.0;
			bool first = true;
			while ( uEnter < uHigh && pixel.steps < params.maxSteps )
			{
				const double uExit = std::min( { uNextX, uNextY, uHigh } );
				if ( !first && tx >= 0 && ty >= 0 && tx < std::int64_t( W ) &&
				     ty < std::int64_t( H ) )
				{
					++pixel.steps;
					const double d = depthAt( std::uint32_t( tx ), std::uint32_t( ty ) );
					const Vec3 a = at( uEnter ), b = at( uExit );
					if ( defect == SsrReferenceDefect::kThicknessInFront )
					{
						// Seeded: within the thickness in front, or behind at any
						// distance.
						const Vec3 far = a.z > b.z ? a : b;
						const double front =
						    view.W( far.x, far.y, d ) - view.W( far.x, far.y, far.z );
						if ( far.z >= d || front < params.thickness )
						{
							pixel.hit = true;
							pixel.hitX = far.x;
							pixel.hitY = far.y;
							pixel.hitDepth = far.z;
							pixel.hitTexelX = std::uint32_t( tx );
							pixel.hitTexelY = std::uint32_t( ty );
							pixel.behind = 0.0;
							break;
						}
					}
					else if ( std::max( a.z, b.z ) >= d )
					{
						double uHit = uEnter;
						if ( a.z < d )
							uHit = uEnter + ( d - a.z ) / ( b.z - a.z ) * ( uExit - uEnter );
						const Vec3 h = at( uHit );
						const double behind =
						    a.z < d ? 0.0 : view.W( h.x, h.y, h.z ) - view.W( h.x, h.y, d );
						if ( !( behind < params.thickness ) && pixel.passedBehind++ == 0 )
						{
							pixel.firstBehindX = std::uint32_t( tx );
							pixel.firstBehindY = std::uint32_t( ty );
							pixel.firstBehind = behind;
						}
						if ( behind < params.thickness )
						{
							pixel.hit = true;
							pixel.end = SsrReferencePixel::End::kHit;
							pixel.hitX = h.x;
							pixel.hitY = h.y;
							pixel.hitDepth = h.z;
							pixel.hitTexelX = std::uint32_t( tx );
							pixel.hitTexelY = std::uint32_t( ty );
							pixel.behind = behind;
							break;
						}
					}
				}
				first = false;
				uEnter = uExit;
				if ( uNextX < uNextY )
				{
					tx += stepX;
					uNextX += uDeltaX;
				}
				else
				{
					ty += stepY;
					uNextY += uDeltaY;
				}
			}
			if ( !pixel.hit )
			{
				pixel.end = pixel.steps >= params.maxSteps ? SsrReferencePixel::End::kMaxSteps
				            : uHigh < 1.0                  ? SsrReferencePixel::End::kScreenEdge
				                                           : SsrReferencePixel::End::kRayEnd;
				continue;
			}

			// 4. Confidence. The hit's footprint J: a ray differential per
			// screen axis, from the camera ray one pixel along it, through the
			// pixel's plane and the hit's plane.
			const Vec3 X = view.World( pixel.hitX, pixel.hitY, pixel.hitDepth );
			const std::size_t hitIndex = std::size_t( pixel.hitTexelY ) * W + pixel.hitTexelX;
			const float *hitNr = &inputs.normalRoughness[hitIndex * 4];
			if ( hitNr[3] > 1.5f )
			{
				pixel.hit = false;
				pixel.end = SsrReferencePixel::End::kCameraOnlyEmitter;
				continue;
			}
			float hitDecoded[3];
			pass::ssr::OctDecode( { hitNr[0], hitNr[1] }, hitDecoded );
			const Vec3 Nh{ hitDecoded[0], hitDecoded[1], hitDecoded[2] };
			double J = 1.0;
			for ( int axis = 0; axis < 2; ++axis )
			{
				const double sx = cx + ( axis == 0 ? 1.0 : 0.0 );
				const double sy = cy + ( axis == 1 ? 1.0 : 0.0 );
				const Vec3 nearPoint = view.World( sx, sy, 0.0 );
				const Vec3 D = view.World( sx, sy, 1.0 ) - nearPoint;
				const double toPlane = Dot( D, N );
				if ( std::fabs( toPlane ) < 1e-12 )
					continue;
				const Vec3 Pa = nearPoint + D * ( Dot( P - nearPoint, N ) / toPlane );
				const Vec3 Va = Normalized( eye - Pa );
				const Vec3 Ra = N * ( 2.0 * Dot( N, Va ) ) - Va;
				const Vec3 Oa = Pa + N * footprint;
				const double toHitPlane = Dot( Ra, Nh );
				if ( std::fabs( toHitPlane ) < 1e-12 )
					continue;
				const double s = Dot( X - Oa, Nh ) / toHitPlane;
				if ( !( s > 0.0 ) )
					continue;
				const Vec3 Xa = Oa + Ra * s;
				const Vec4 c = Apply( view.toClip, { Xa.x, Xa.y, Xa.z, 1.0 } );
				if ( !( c[3] > 0.0 ) )
					continue;
				const double hx = ( c[0] / c[3] + 1.0 ) * 0.5 * W;
				const double hy = ( 1.0 - c[1] / c[3] ) * 0.5 * H;
				J = std::max( J, std::hypot( hx - pixel.hitX, hy - pixel.hitY ) );
			}
			pixel.footprint = J;
			const double fadeW = double( params.edgeFade ) * W * J;
			const double fadeH = double( params.edgeFade ) * H * J;
			const double edge = Smoothstep( 0.0, 1.0,
			    std::min( { pixel.hitX / fadeW, ( W - pixel.hitX ) / fadeW, pixel.hitY / fadeH,
			        ( H - pixel.hitY ) / fadeH } ) );
			const double thicknessFade = 1.0 - Smoothstep( 0.0, params.thickness, pixel.behind );
			const double fadeStart = double( params.roughnessFadeStart ) * params.roughnessCutoff;
			const double roughnessFade =
			    1.0 - Smoothstep( fadeStart, params.roughnessCutoff, roughness );
			pixel.confidence = edge * thicknessFade * roughnessFade;
			pixel.edge = edge;
			pixel.thicknessFade = thicknessFade;
			pixel.roughnessFade = roughnessFade;

			// 5. The reflected light: the pyramid at the lobe's footprint.
			const double L = std::hypot( pixel.hitX - cx, pixel.hitY - cy );
			pixel.mip = std::clamp(
			    std::log2( std::max( 1.0, 2.0 * roughness * roughness * L ) ), 0.0, maxLevel );
			const double lower = std::floor( pixel.mip );
			const double upper = std::min( lower + 1.0, maxLevel );
			double low[3], high[3];
			for ( auto [level, out] : { std::pair{ lower, low }, std::pair{ upper, high } } )
			{
				const SsrPyramidLevel &l = pyramid[std::size_t( level )];
				Bilinear( l, pixel.hitX * l.width / W, pixel.hitY * l.height / H, out );
			}
			const double f = pixel.mip - lower;
			if ( !( pixel.confidence > 0.0 ) )
				continue;
			const float *weight = &inputs.specularWeight[index * 4];
			const float *ibl = &inputs.iblRadiance[index * 4];
			for ( int k = 0; k < 3; ++k )
			{
				const double reflected = low[k] * ( 1.0 - f ) + high[k] * f;
				pixel.reflected[k] = float( reflected );
				pixel.out[k] = float(
				    double( lit[k] ) + pixel.confidence * weight[k] * ( reflected - ibl[k] ) );
			}
		}
	}
	return result;
}

} // namespace render::lab
