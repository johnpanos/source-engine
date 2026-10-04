//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The area lights an emissive model publishes (render.emissive-area-
//          lights.v1, RFC 0011 light set v2): how the client turns a model's
//          self-illuminated triangles into area lights (render/area_light.h),
//          and which of them are lit each frame.
//
//          Emission. A triangle's radiance is the mean, over its texels, of
//          what its material draws as self-illumination at tint 1 (for
//          VertexLitGeneric: the linear base color times the selfillum mask);
//          the caller samples it with SampleTriangle. The tint ($selfillumtint,
//          which proxies animate) scales the light each frame.
//
//          Emitters. Triangles are grouped by the caller (a bone and a
//          material: the light moves with the bone and takes the material's
//          tint). A group is first split by facing: each triangle goes to the
//          axis direction (+-x, +-y, +-z of the group's space) nearest its
//          normal, so panels facing apart (the two faces of a door) never
//          share a rectangle. Each part is then split at its emission-weighted
//          median along its
//          longest axis until each part spans at most kMaxEmitterExtent, at
//          most kMaxEmittersPerGroup parts per facing. Each part becomes one rectangle
//          (FitEmitter): centered on the emission-weighted mean, in the plane
//          of the area-weighted mean normal, its axes and half extents from
//          the emission's second moments in that plane (a uniform rectangle
//          of half extent h has variance h^2 / 3), and one-sided unless the
//          part's normals mostly cancel (a closed shape), then two-sided. Its
//          radiance keeps the part's emitted power: sum( A_i L_i ) over the
//          rectangle's emitting area.
//
//          Selection. A frame lights at most a budget of emitters, the most
//          important at the view first: emitted power times how much of the
//          view its reach covers, reach^2 / ( reach^2 + d^2 ). Emitters the
//          view can see (the caller's test: the client traces the world from
//          the view to the rectangle) come first; the rest only take budget
//          left over, so a large emitter the view cannot see (a lava pit a
//          floor below) never takes the light from small ones beside the
//          view. A lit emitter keeps its light against a newcomer less than
//          kKeepMargin stronger, so two close emitters do not trade it every
//          frame.
//
//          Tier0-free C++ shared by the client and the conformance suite
//          (unittests/rendertest/test_area_light.cpp).
//
//===========================================================================//

#ifndef RENDER_EMISSIVE_AREA_LIGHTS_H
#define RENDER_EMISSIVE_AREA_LIGHTS_H

#include "render/area_light.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

namespace emissive
{

constexpr float kMaxEmitterExtent = 48.0f;
constexpr int kMaxEmittersPerGroup = 4;
// A rectangle is never thinner than this (a line of emitting triangles).
constexpr float kMinHalfExtent = 0.25f;
// Below this |sum A n| / sum A the part is a closed shape: two-sided.
constexpr float kTwoSidedOpenness = 0.35f;
// Texels darker than this emit nothing (the selfillum mask's unmasked part).
constexpr float kMinRadiance = 1.0f / 255.0f;
constexpr float kKeepMargin = 1.25f;
constexpr int kMaxSamplesPerTriangle = 64;

// Reviewed Source surface-source policy v1 (2026-10-04): self-illumination
// and fullbright UnlitGeneric scene surfaces illuminate core receivers.
// Sky/nodraw and already baked texture lights remain ingress exclusions.
inline bool UnlitSource( std::string_view shader )
{
	return shader == "UnlitGeneric" || shader == "UnlitGeneric_DX9";
}

inline bool SurfaceSource( bool selfIllum, std::string_view shader )
{
	return selfIllum || UnlitSource( shader );
}

// Fullbright images emit through their authored coverage, not transparent
// texels. Alpha-tested coverage is binary, as in the visible surface point.
inline void UnlitRadiance( const float rgb[3], float alpha, bool translucent, bool alphaTest,
    float reference, float out[3] )
{
	const float coverage = alphaTest && alpha < reference ? 0.0f
	                       : translucent                  ? std::clamp( alpha, 0.0f, 1.0f )
	                                                      : 1.0f;
	for ( int k = 0; k < 3; ++k )
		out[k] = rgb[k] * coverage;
}

struct Triangle
{
	float p[3][3] = {};     // in the group's space
	float radiance[3] = {}; // mean emitted radiance at tint 1
	int group = 0;
};

struct Emitter
{
	int group = 0;
	area_light::Rect rect;  // in the group's space
	float radiance[3] = {}; // at tint 1
};

namespace detail
{
inline float Luminance( const float rgb[3] )
{
	return 0.2126f * rgb[0] + 0.7152f * rgb[1] + 0.0722f * rgb[2];
}

inline float TriangleArea( const Triangle &t, float normal[3] )
{
	const float e1[3] = { t.p[1][0] - t.p[0][0], t.p[1][1] - t.p[0][1], t.p[1][2] - t.p[0][2] };
	const float e2[3] = { t.p[2][0] - t.p[0][0], t.p[2][1] - t.p[0][1], t.p[2][2] - t.p[0][2] };
	area_light::detail::Cross( e1, e2, normal );
	const float twice = area_light::detail::Length( normal );
	for ( int k = 0; k < 3; ++k )
		normal[k] = twice > 0.0f ? normal[k] / twice : 0.0f;
	return 0.5f * twice;
}

inline bool Emits( const Triangle &t )
{
	return std::max( t.radiance[0], std::max( t.radiance[1], t.radiance[2] ) ) >= kMinRadiance;
}

// The weight a triangle's emission carries in the fit: its area times its
// luminance (its emitted power, up to pi).
inline float Weight( const Triangle &t )
{
	float normal[3];
	return TriangleArea( t, normal ) * Luminance( t.radiance );
}

// Symmetric 2x2 eigen decomposition: the unit major axis (x, y) and the two
// eigenvalues, major first.
inline void Eigen2( float a, float b, float c, float axis[2], float values[2] )
{
	const float mean = 0.5f * ( a + c );
	const float diff = 0.5f * ( a - c );
	const float root = std::sqrt( diff * diff + b * b );
	values[0] = mean + root;
	values[1] = mean - root;
	if ( std::fabs( b ) > 1.0e-12f )
	{
		axis[0] = values[0] - c;
		axis[1] = b;
	}
	else
	{
		axis[0] = a >= c ? 1.0f : 0.0f;
		axis[1] = a >= c ? 0.0f : 1.0f;
	}
	const float length = std::sqrt( axis[0] * axis[0] + axis[1] * axis[1] );
	axis[0] /= length;
	axis[1] /= length;
}
} // namespace detail

// Samples a triangle's mean emission over its UV triangle: the centroids of
// k^2 sub-triangles, k from its texel footprint (at most
// kMaxSamplesPerTriangle samples). `fetch( u, v, rgb )` returns the emission
// at a texture coordinate (wrapping is the caller's).
template <typename Fetch>
void SampleTriangle( const float uv[3][2], int width, int height, Fetch fetch, float out[3] )
{
	const float du1 = ( uv[1][0] - uv[0][0] ) * float( width );
	const float dv1 = ( uv[1][1] - uv[0][1] ) * float( height );
	const float du2 = ( uv[2][0] - uv[0][0] ) * float( width );
	const float dv2 = ( uv[2][1] - uv[0][1] ) * float( height );
	const float texels = 0.5f * std::fabs( du1 * dv2 - du2 * dv1 );
	int k = int( std::ceil( std::sqrt( std::max( texels, 1.0f ) ) ) );
	const int kMaxK = int( std::sqrt( float( kMaxSamplesPerTriangle ) ) );
	k = std::min( std::max( k, 1 ), kMaxK );
	float sum[3] = { 0, 0, 0 };
	int count = 0;
	auto sample = [&]( float a, float b )
	{
		const float c = 1.0f - a - b;
		const float u = c * uv[0][0] + a * uv[1][0] + b * uv[2][0];
		const float v = c * uv[0][1] + a * uv[1][1] + b * uv[2][1];
		float rgb[3];
		fetch( u, v, rgb );
		for ( int ch = 0; ch < 3; ++ch )
			sum[ch] += rgb[ch];
		++count;
	};
	const float step = 1.0f / float( k );
	for ( int i = 0; i < k; ++i )
	{
		for ( int j = 0; i + j < k; ++j )
		{
			sample( ( float( i ) + 1.0f / 3.0f ) * step, ( float( j ) + 1.0f / 3.0f ) * step );
			if ( i + j < k - 1 )
				sample( ( float( i ) + 2.0f / 3.0f ) * step, ( float( j ) + 2.0f / 3.0f ) * step );
		}
	}
	for ( int ch = 0; ch < 3; ++ch )
		out[ch] = count ? sum[ch] / float( count ) : 0.0f;
}

// One rectangle for a set of emitting triangles of one group (see the header
// comment). Returns false when they emit nothing.
inline bool FitEmitter( const std::vector<const Triangle *> &part, Emitter &out )
{
	double weightSum = 0.0, areaSum = 0.0;
	double mean[3] = { 0, 0, 0 }, normalSum[3] = { 0, 0, 0 }, power[3] = { 0, 0, 0 };
	double moment[3][3] = {};
	for ( const Triangle *t : part )
	{
		float normal[3];
		const float area = detail::TriangleArea( *t, normal );
		const float weight = area * detail::Luminance( t->radiance );
		areaSum += area;
		for ( int k = 0; k < 3; ++k )
		{
			normalSum[k] += double( area ) * normal[k];
			power[k] += double( area ) * t->radiance[k];
		}
		if ( !( weight > 0.0f ) )
			continue;
		weightSum += weight;
		// A uniform triangle's second moment about the origin:
		// ( aa' + bb' + cc' + (a+b+c)(a+b+c)' ) / 12.
		double s[3];
		for ( int k = 0; k < 3; ++k )
		{
			s[k] = double( t->p[0][k] ) + t->p[1][k] + t->p[2][k];
			mean[k] += weight * s[k] / 3.0;
		}
		for ( int r = 0; r < 3; ++r )
			for ( int c = 0; c < 3; ++c )
			{
				double m = s[r] * s[c];
				for ( int v = 0; v < 3; ++v )
					m += double( t->p[v][r] ) * t->p[v][c];
				moment[r][c] += weight * m / 12.0;
			}
	}
	if ( !( weightSum > 0.0 ) || !( areaSum > 0.0 ) )
		return false;
	for ( int k = 0; k < 3; ++k )
		mean[k] /= weightSum;
	double cov[3][3];
	for ( int r = 0; r < 3; ++r )
		for ( int c = 0; c < 3; ++c )
			cov[r][c] = moment[r][c] / weightSum - mean[r] * mean[c];

	float normal[3] = { float( normalSum[0] ), float( normalSum[1] ), float( normalSum[2] ) };
	const float normalLength = area_light::detail::Length( normal );
	const float openness = float( normalLength / areaSum );
	if ( normalLength > 0.0f )
		for ( float &n : normal )
			n /= normalLength;
	else
	{
		normal[0] = 0.0f;
		normal[1] = 0.0f;
		normal[2] = 1.0f;
	}
	// A basis of the plane.
	float e1[3], e2[3];
	const float helper[3] = { std::fabs( normal[0] ) < 0.9f ? 1.0f : 0.0f,
	    std::fabs( normal[0] ) < 0.9f ? 0.0f : 1.0f, 0.0f };
	area_light::detail::Cross( helper, normal, e1 );
	const float e1Length = area_light::detail::Length( e1 );
	for ( float &e : e1 )
		e /= e1Length;
	area_light::detail::Cross( normal, e1, e2 );
	auto project = [&]( const float *x, const float *y )
	{
		double sum = 0.0;
		for ( int r = 0; r < 3; ++r )
			for ( int c = 0; c < 3; ++c )
				sum += double( x[r] ) * cov[r][c] * y[c];
		return float( sum );
	};
	float axis[2], values[2];
	detail::Eigen2( project( e1, e1 ), project( e1, e2 ), project( e2, e2 ), axis, values );
	float u[3], v[3];
	for ( int k = 0; k < 3; ++k )
		u[k] = axis[0] * e1[k] + axis[1] * e2[k];
	area_light::detail::Cross( normal, u, v );
	const float hu = std::max( std::sqrt( std::max( 3.0f * values[0], 0.0f ) ), kMinHalfExtent );
	const float hv = std::max( std::sqrt( std::max( 3.0f * values[1], 0.0f ) ), kMinHalfExtent );

	out = Emitter();
	out.group = part.front()->group;
	for ( int k = 0; k < 3; ++k )
	{
		out.rect.center[k] = float( mean[k] );
		out.rect.halfU[k] = hu * u[k];
		out.rect.halfV[k] = hv * v[k];
	}
	out.rect.twoSided = openness < kTwoSidedOpenness;
	const float emittingArea = area_light::Area( out.rect ) * ( out.rect.twoSided ? 2.0f : 1.0f );
	for ( int k = 0; k < 3; ++k )
		out.radiance[k] = float( power[k] / emittingArea );
	return true;
}

namespace detail
{
// The facing bin of a triangle: 0..5 for +x, -x, +y, -y, +z, -z.
inline int Facing( const Triangle &t )
{
	float normal[3];
	TriangleArea( t, normal );
	int axis = 0;
	for ( int k = 1; k < 3; ++k )
		if ( std::fabs( normal[k] ) > std::fabs( normal[axis] ) )
			axis = k;
	return 2 * axis + ( normal[axis] < 0.0f ? 1 : 0 );
}
} // namespace detail

// Every emitter of a model's triangles (see the header comment), group by
// group in ascending group order, each group's facings in bin order.
inline std::vector<Emitter> BuildEmitters( const std::vector<Triangle> &triangles )
{
	std::vector<std::pair<int, const Triangle *>> emitting; // ( group * 6 + facing, triangle )
	for ( const Triangle &t : triangles )
		if ( detail::Emits( t ) )
			emitting.emplace_back( t.group * 6 + detail::Facing( t ), &t );
	std::stable_sort( emitting.begin(), emitting.end(),
	    []( const std::pair<int, const Triangle *> &a, const std::pair<int, const Triangle *> &b )
	    {
		    return a.first < b.first;
	    } );

	std::vector<Emitter> emitters;
	size_t begin = 0;
	while ( begin < emitting.size() )
	{
		size_t end = begin;
		while ( end < emitting.size() && emitting[end].first == emitting[begin].first )
			++end;
		// Split the group's largest part until every part is small enough or
		// the group has its most parts.
		std::vector<std::vector<const Triangle *>> parts( 1 );
		for ( size_t i = begin; i < end; ++i )
			parts[0].push_back( emitting[i].second );
		for ( ;; )
		{
			if ( int( parts.size() ) >= kMaxEmittersPerGroup )
				break;
			int widest = -1, widestAxis = 0;
			float widestExtent = kMaxEmitterExtent;
			for ( size_t i = 0; i < parts.size(); ++i )
			{
				if ( parts[i].size() < 2 )
					continue;
				float lo[3] = { 1e30f, 1e30f, 1e30f }, hi[3] = { -1e30f, -1e30f, -1e30f };
				for ( const Triangle *t : parts[i] )
					for ( int v = 0; v < 3; ++v )
						for ( int k = 0; k < 3; ++k )
						{
							lo[k] = std::min( lo[k], t->p[v][k] );
							hi[k] = std::max( hi[k], t->p[v][k] );
						}
				for ( int k = 0; k < 3; ++k )
					if ( hi[k] - lo[k] > widestExtent )
					{
						widestExtent = hi[k] - lo[k];
						widest = int( i );
						widestAxis = k;
					}
			}
			if ( widest < 0 )
				break;
			std::vector<const Triangle *> &part = parts[size_t( widest )];
			auto centroid = [widestAxis]( const Triangle *t )
			{
				return t->p[0][widestAxis] + t->p[1][widestAxis] + t->p[2][widestAxis];
			};
			std::stable_sort( part.begin(), part.end(),
			    [&]( const Triangle *a, const Triangle *b )
			    {
				    return centroid( a ) < centroid( b );
			    } );
			// The emission-weighted median, never leaving a side empty.
			float total = 0.0f;
			for ( const Triangle *t : part )
				total += detail::Weight( *t );
			float running = 0.0f;
			size_t split = 1;
			for ( ; split < part.size() - 1; ++split )
			{
				running += detail::Weight( *part[split - 1] );
				if ( running >= 0.5f * total )
					break;
			}
			std::vector<const Triangle *> upper( part.begin() + long( split ), part.end() );
			part.resize( split );
			parts.push_back( std::move( upper ) );
		}
		for ( const std::vector<const Triangle *> &part : parts )
		{
			Emitter emitter;
			if ( FitEmitter( part, emitter ) )
				emitters.push_back( emitter );
		}
		begin = end;
	}
	return emitters;
}

// A frame's candidate: a placed emitter at its current tint.
struct Candidate
{
	area_light::AreaLight light; // in world space, reach set
	bool wasLit = false;
};

// The importance of a candidate at the view (see the header comment).
[[nodiscard]] inline float Importance( const area_light::AreaLight &light, const float view[3] )
{
	const float reach = light.reach;
	if ( !( reach > 0.0f ) )
		return 0.0f;
	const float d = area_light::DistanceTo( light.rect, view );
	const float power = detail::Luminance( light.radiance ) * area_light::Area( light.rect ) *
	                    ( light.rect.twoSided ? 2.0f : 1.0f );
	return power * reach * reach / ( reach * reach + d * d );
}

// Marks at most `budget` candidates lit, the most important first: first
// those `inView( index )` says the view can see, then, with budget left over,
// the rest. `inView` is asked in rank order and only until the budget is
// filled, so a caller may make it a trace. A lit candidate is preferred over
// an unlit one less than kKeepMargin stronger. Ties go to the lower index.
// `order`, when given, receives the lit candidates' indices in that order.
template <class InView>
inline void SelectLit( const Candidate *candidates, int count, int budget, const float view[3],
    bool *lit, InView &&inView, std::vector<int> *order = nullptr )
{
	if ( order )
		order->clear();
	std::vector<std::pair<float, int>> ranked;
	ranked.reserve( size_t( count ) );
	for ( int i = 0; i < count; ++i )
	{
		lit[i] = false;
		float importance = Importance( candidates[i].light, view );
		if ( !( importance > 0.0f ) )
			continue;
		if ( candidates[i].wasLit )
			importance *= kKeepMargin;
		ranked.emplace_back( importance, i );
	}
	std::stable_sort( ranked.begin(), ranked.end(),
	    []( const std::pair<float, int> &a, const std::pair<float, int> &b )
	    {
		    return a.first > b.first;
	    } );
	int nLit = 0;
	std::vector<int> hidden;
	for ( size_t r = 0; r < ranked.size() && nLit < budget; ++r )
	{
		const int index = ranked[r].second;
		if ( inView( index ) )
		{
			lit[index] = true;
			++nLit;
			if ( order )
				order->push_back( index );
		}
		else
		{
			hidden.push_back( index );
		}
	}
	for ( size_t h = 0; h < hidden.size() && nLit < budget; ++h, ++nLit )
	{
		lit[hidden[h]] = true;
		if ( order )
			order->push_back( hidden[h] );
	}
}

// Integrate the current source image over its authored triangle mapping,
// then fit with the same power-preserving policy as other emissive sources.
// The caller owns frame selection, radiance policy and image lifetime.
template <typename Fetch>
std::vector<Emitter> BuildMappedEmitters(
    std::span<const area_light::EmissiveTriangle> surfaces, int width, int height, Fetch fetch )
{
	std::vector<Triangle> triangles;
	triangles.reserve( surfaces.size() );
	for ( const auto &surface : surfaces )
	{
		Triangle triangle;
		triangle.group = surface.group;
		for ( int c = 0; c < 3; ++c )
			std::copy_n( surface.position[c], 3, triangle.p[c] );
		SampleTriangle( surface.uv, width, height, fetch, triangle.radiance );
		triangles.push_back( triangle );
	}
	return BuildEmitters( triangles );
}

// Every candidate in view.
inline void SelectLit(
    const Candidate *candidates, int count, int budget, const float view[3], bool *lit )
{
	SelectLit( candidates, count, budget, view, lit,
	    []( int )
	    {
		    return true;
	    } );
}

} // namespace emissive

#endif // RENDER_EMISSIVE_AREA_LIGHTS_H
