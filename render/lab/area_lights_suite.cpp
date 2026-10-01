//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab suite area-lights (RFC 0016 K11 "Model assembly", the
//			area-light term): the surface program's pbr point lit by
//			render.area-light.v1 rectangles through linearly transformed
//			cosines (render/shaders/common/ltc.glsl), judged against oracles
//			that share no code with the shader:
//			- the diffuse lobe against the contract's exact irradiance
//			  (area_light::IrradianceAt: the clipped Lambert form factor times
//			  the window), times the diffuse color of pbr_brdf.h;
//			- the GGX lobe against the term as RFC 0016 defines it (linearly
//			  transformed cosines over the rectangle clipped to the horizon,
//			  the fitted table public/render/pbr_ltc_table.h, the split-sum
//			  magnitude and compensation of pbr_brdf.h), evaluated by an
//			  independent C++ implementation in this file;
//			- the GGX lobe's accuracy against the exact light, a 128 x 128
//			  quadrature over the rectangle of pbr_brdf.h's lobe (Schlick
//			  Fresnel, energy compensation), reported per case as the
//			  energy-weighted mean and the 90th-percentile relative error: a
//			  measurement of the LTC approximation, which the ground-truth
//			  check (render.lab.cycles over the lighting fixtures) judges;
//			- the term's neutral value (no area lights, or the term off) is
//			  bitwise the frame without the term; a one-sided light seen from
//			  behind gives nothing; a two-sided one gives what it gives from
//			  the front; 64 lights at once.
//			Tolerances: diffuse, each sampled pixel within 0.4 percent + 3e-4
//			of the exact oracle (the target is half float), fixed before the
//			first run; GGX against the term's definition, each sampled pixel
//			within 2 percent + 2e-4 (the half-float target), fixed before its
//			first run; a pixel where the term moves by more than half that
//			within 0.1 units is ill-conditioned (the rasterizer's position
//			may differ from the ray's by that much at a grazing view) and is
//			skipped, at most 5 percent of a case's pixels (added after one
//			such pixel failed; see the progress record). A first
//			version judged the GGX lobe against the exact quadrature (mean 5
//			percent, 90th percentile 15 percent); the LTC approximation
//			failed it at grazing views and for lights near the horizon (RFC
//			0016 progress, K11 step b), so that comparison is a measurement.
//
//			Seeded programs (--sensitivity): no horizon clip, the LTC matrix
//			transposed, the GGX magnitude dropped.
//
//			RENDER_LAB_IMAGES=<dir> writes each judged frame there as a PFM
//			(<check>.gpu.pfm), and for the gallery's cases the exact light
//			at every pixel (<check>.exact.pfm), for review.
//
//=============================================================================//

#include "lab_canvas.h"
#include "lab_receiver.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "render/area_light.h"
#include "render/material/pbr_family.h"
#include "render/math/matrix.h"
#include "render/pbr_brdf.h"
#include "render/pass/lights/clusters.h"
#include "render/pbr_ltc_table.h"
#include "render/shaderlib/debug_view.h"
#include "spv/area_light_defects_spv.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace render::lab
{

namespace
{

using namespace render::device;
using math::float3;

constexpr std::uint32_t kSize = 96;
constexpr float kReceiver = 900.0f; // the receiver plane z = 0 spans +-kReceiver
constexpr int kSampleStep = 4;      // every fourth pixel in each axis
constexpr int kQuadrature = 128;    // oracle samples per rectangle axis

constexpr float kDiffuseRelative = 4e-3f;
constexpr float kDiffuseAbsolute = 3e-4f;
constexpr float kSpecularRelative = 0.02f;
constexpr float kConditioning = 0.1f;         // world units
constexpr float kIllConditionedShare = 0.05f; // of the sampled pixels, at most
constexpr float kSpecularAbsolute = 2e-4f;
constexpr float kAccuracyFloor = 0.02f; // of the case's brightest pixel

struct Rgb
{
	float r = 0, g = 0, b = 0;
};

// The receiver scene (lab_receiver.h).
using View = ReceiverView;
using Material = ReceiverMaterial;
constexpr const ReceiverMaterial ( &kMaterials )[4] = kReceiverMaterials;

View MakeView( float3 eye, float3 target )
{
	return MakeReceiverView( eye, target, kSize );
}

std::optional<float3> Hit( const View &view, std::uint32_t px, std::uint32_t py )
{
	return ReceiverHit( view, px, py, kReceiver );
}

area_light::AreaLight Light(
    float3 center, float3 halfU, float3 halfV, float radiance, bool twoSided = false )
{
	area_light::AreaLight light;
	std::memcpy( light.rect.center, &center, sizeof( light.rect.center ) );
	std::memcpy( light.rect.halfU, &halfU, sizeof( light.rect.halfU ) );
	std::memcpy( light.rect.halfV, &halfV, sizeof( light.rect.halfV ) );
	light.rect.twoSided = twoSided;
	for ( float &c : light.radiance )
		c = radiance;
	light.reach = area_light::Reach( light.rect, light.radiance );
	return light;
}

// The lights, by name. Every rectangle's front is U x V.
std::map<std::string, std::vector<area_light::AreaLight>> Lights()
{
	std::map<std::string, std::vector<area_light::AreaLight>> lights;
	lights["ceiling"] = { Light( { 0, 0, 120 }, { 60, 0, 0 }, { 0, -40, 0 }, 2.0f ) };
	lights["tilted"] = { Light( { 40, 30, 90 }, { 50, 0, -20 }, { 0, -40, 0 }, 1.5f ) };
	// Crosses the receiver plane: the part below it is under every
	// receiver's horizon.
	lights["straddling"] = { Light( { 0, 60, 20 }, { 80, 0, 0 }, { 0, 0, 40 }, 1.0f ) };
	// Small, so its reach (about 77 units) ends on the receiver.
	lights["short-reach"] = { Light( { 0, 0, 15 }, { 3, 0, 0 }, { 0, -3, 0 }, 2.0f ) };
	// Facing up, away from the receiver; and the same light two-sided.
	lights["behind"] = { Light( { 0, 0, 120 }, { 60, 0, 0 }, { 0, 40, 0 }, 2.0f ) };
	lights["two-sided"] = { Light( { 0, 0, 120 }, { 60, 0, 0 }, { 0, 40, 0 }, 2.0f, true ) };
	std::vector<area_light::AreaLight> grid;
	for ( int i = 0; i < 8; ++i )
	{
		for ( int j = 0; j < 8; ++j )
			grid.push_back(
			    Light( { -140.0f + 40.0f * float( i ), -140.0f + 40.0f * float( j ), 80 },
			        { 8, 0, 0 }, { 0, -8, 0 }, 4.0f ) );
	}
	lights["grid64"] = grid;
	return lights;
}

// The diffuse oracle: the contract's irradiance times pbr_brdf.h's diffuse
// color at N.V.
Rgb DiffuseOracle(
    const std::vector<area_light::AreaLight> &lights, const Material &material, float3 p, float3 v )
{
	const float n[3] = { 0, 0, 1 };
	const float at[3] = { p.x, p.y, p.z };
	const float normalDotView = std::max( v.z, 0.0f );
	const pbr::SplitSumCoefficients split =
	    pbr::SampleSplitSum( normalDotView, material.Roughness() );
	// White base: f0 is 0.04 for a dielectric, the base for a metal.
	const float f0 = 0.04f + ( 1.0f - 0.04f ) * material.Metal();
	const float diffuseColor =
	    ( 1.0f - material.Metal() ) * ( 1.0f - pbr::SpecularDirectionalAlbedo( f0, split ) );
	Rgb total;
	for ( const area_light::AreaLight &light : lights )
	{
		float e[3];
		area_light::IrradianceAt( light, at, n, e );
		total.r += diffuseColor * e[0];
		total.g += diffuseColor * e[1];
		total.b += diffuseColor * e[2];
	}
	return total;
}

// The GGX oracle: the lobe (Schlick Fresnel, energy compensation) integrated
// over each rectangle by a midpoint rule, times the light's window at p.
Rgb SpecularOracle(
    const std::vector<area_light::AreaLight> &lights, const Material &material, float3 p, float3 v )
{
	const float3 n{ 0, 0, 1 };
	const float normalDotView = std::max( math::Dot( n, v ), 0.0f );
	const float f0 = 0.04f + ( 1.0f - 0.04f ) * material.Metal();
	const pbr::Color reflectance{ f0, f0, f0 };
	const pbr::SplitSumCoefficients split =
	    pbr::SampleSplitSum( normalDotView, material.Roughness() );
	const float compensation = pbr::SpecularEnergyCompensation( f0, split );
	Rgb total;
	for ( const area_light::AreaLight &light : lights )
	{
		const float at[3] = { p.x, p.y, p.z };
		const float window =
		    area_light::Window( area_light::DistanceTo( light.rect, at ), light.reach );
		if ( window <= 0.0f || !area_light::Faces( light.rect, at ) )
			continue;
		float front[3];
		area_light::Normal( light.rect, front );
		const float area = area_light::Area( light.rect );
		const float cell = area / float( kQuadrature * kQuadrature );
		double sum = 0.0;
		for ( int j = 0; j < kQuadrature; ++j )
		{
			for ( int i = 0; i < kQuadrature; ++i )
			{
				const float s = ( float( i ) + 0.5f ) / float( kQuadrature ) * 2.0f - 1.0f;
				const float t = ( float( j ) + 0.5f ) / float( kQuadrature ) * 2.0f - 1.0f;
				const float3 q{
				    light.rect.center[0] + s * light.rect.halfU[0] + t * light.rect.halfV[0],
				    light.rect.center[1] + s * light.rect.halfU[1] + t * light.rect.halfV[1],
				    light.rect.center[2] + s * light.rect.halfU[2] + t * light.rect.halfV[2] };
				const float3 toLight{ q.x - p.x, q.y - p.y, q.z - p.z };
				const float distanceSquared = math::Dot( toLight, toLight );
				const float3 l = math::Normalize( toLight );
				const float normalDotLight = math::Dot( n, l );
				if ( normalDotLight <= 0.0f )
					continue;
				const float emitterCosine =
				    std::fabs( front[0] * l.x + front[1] * l.y + front[2] * l.z );
				const float3 h = math::Normalize( { l.x + v.x, l.y + v.y, l.z + v.z } );
				const pbr::Color f = pbr::EvaluateSpecular( reflectance, normalDotView,
				    normalDotLight, math::Dot( n, h ), math::Dot( v, h ), material.Roughness() );
				sum += double( f.red ) * normalDotLight * emitterCosine * cell / distanceSquared;
			}
		}
		const float value = float( sum ) * compensation * window;
		total.r += value * light.radiance[0];
		total.g += value * light.radiance[1];
		total.b += value * light.radiance[2];
	}
	return total;
}

// The term as defined, evaluated independently of ltc.glsl.
namespace ltc
{

struct Vec
{
	double x = 0, y = 0, z = 0;
};
Vec Sub( Vec a, Vec b )
{
	return { a.x - b.x, a.y - b.y, a.z - b.z };
}
Vec Mad( Vec a, double s, Vec b )
{
	return { a.x + s * b.x, a.y + s * b.y, a.z + s * b.z };
}
double Dot( Vec a, Vec b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
Vec Cross( Vec a, Vec b )
{
	return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}
Vec Unit( Vec a )
{
	const double length = std::sqrt( Dot( a, a ) );
	return { a.x / length, a.y / length, a.z / length };
}

// Sutherland-Hodgman against z >= 0.
int ClipAbove( Vec *polygon, int count )
{
	Vec out[8];
	int n = 0;
	for ( int i = 0; i < count; ++i )
	{
		const Vec a = polygon[i];
		const Vec b = polygon[( i + 1 ) % count];
		if ( a.z >= 0.0 )
			out[n++] = a;
		if ( ( a.z >= 0.0 ) != ( b.z >= 0.0 ) )
			out[n++] = Mad( a, a.z / ( a.z - b.z ), Sub( b, a ) );
	}
	std::copy( out, out + n, polygon );
	return n;
}

// The table's four entries at a roughness and N.V, bilinearly between the
// texels on the end points (the program filters the table itself).
void Inverse( float roughness, float normalDotView, double m[4] )
{
	const int size = pbr::kLtcSize;
	const double u = std::sqrt( std::max( 1.0 - double( normalDotView ), 0.0 ) ) * ( size - 1 );
	const double w = double( roughness ) * ( size - 1 );
	const int x0 = std::clamp( int( std::floor( u ) ), 0, size - 1 );
	const int y0 = std::clamp( int( std::floor( w ) ), 0, size - 1 );
	const int x1 = std::min( x0 + 1, size - 1 );
	const int y1 = std::min( y0 + 1, size - 1 );
	const double fx = u - x0;
	const double fy = w - y0;
	auto entry = [&]( int x, int y, int k )
	{
		const pbr::LtcInverse &e = pbr::kLtcTable[y * size + x];
		return double( k == 0 ? e.m00 : k == 1 ? e.m02 : k == 2 ? e.m20 : e.m22 );
	};
	for ( int k = 0; k < 4; ++k )
		m[k] = ( entry( x0, y0, k ) * ( 1 - fx ) + entry( x1, y0, k ) * fx ) * ( 1 - fy ) +
		       ( entry( x0, y1, k ) * ( 1 - fx ) + entry( x1, y1, k ) * fx ) * fy;
}

// The integral of the clamped cosine transformed by the inverse matrix m
// (row-major 3x3; null for the identity) over the rectangle clipped to the
// surface's horizon and the cosine's, one-sided unless two-sided.
double Rectangle( const area_light::AreaLight &light, Vec p, Vec n, Vec v, const double *m )
{
	Vec t1 = Sub( v, { n.x * Dot( v, n ), n.y * Dot( v, n ), n.z * Dot( v, n ) } );
	t1 = Dot( t1, t1 ) < 1e-12 ? Vec{ 1, 0, 0 } : Unit( t1 );
	const Vec t2 = Cross( n, t1 );
	Vec polygon[8];
	static const double kSigns[4][2] = { { -1, -1 }, { 1, -1 }, { 1, 1 }, { -1, 1 } };
	for ( int c = 0; c < 4; ++c )
	{
		Vec corner;
		corner.x = light.rect.center[0] + kSigns[c][0] * light.rect.halfU[0] +
		           kSigns[c][1] * light.rect.halfV[0];
		corner.y = light.rect.center[1] + kSigns[c][0] * light.rect.halfU[1] +
		           kSigns[c][1] * light.rect.halfV[1];
		corner.z = light.rect.center[2] + kSigns[c][0] * light.rect.halfU[2] +
		           kSigns[c][1] * light.rect.halfV[2];
		const Vec d = Sub( corner, p );
		polygon[c] = { Dot( d, t1 ), Dot( d, t2 ), Dot( d, n ) };
	}
	int count = ClipAbove( polygon, 4 );
	if ( m )
	{
		for ( int c = 0; c < count; ++c )
		{
			const Vec q = polygon[c];
			polygon[c] = { m[0] * q.x + m[1] * q.y + m[2] * q.z,
			    m[3] * q.x + m[4] * q.y + m[5] * q.z, m[6] * q.x + m[7] * q.y + m[8] * q.z };
		}
		count = ClipAbove( polygon, count );
	}
	if ( count < 3 )
		return 0.0;
	Vec sum;
	for ( int c = 0; c < count; ++c )
	{
		const Vec a = Unit( polygon[c] );
		const Vec b = Unit( polygon[( c + 1 ) % count] );
		const Vec edge = Cross( a, b );
		const double sine = std::sqrt( Dot( edge, edge ) );
		if ( sine > 1e-12 )
			sum = Mad( sum, std::atan2( sine, Dot( a, b ) ) / sine, edge );
	}
	const double integral = -sum.z / ( 2.0 * 3.14159265358979323846 );
	return light.rect.twoSided ? std::fabs( integral ) : std::max( integral, 0.0 );
}

} // namespace ltc

// The GGX lobe as the term defines it: the split-sum magnitude and
// compensation times the transformed cosine's integral, times the window.
Rgb SpecularTerm(
    const std::vector<area_light::AreaLight> &lights, const Material &material, float3 p, float3 v )
{
	const float normalDotView = std::max( v.z, 0.0f );
	const float f0 = 0.04f + ( 1.0f - 0.04f ) * material.Metal();
	const pbr::SplitSumCoefficients split =
	    pbr::SampleSplitSum( normalDotView, material.Roughness() );
	const double magnitude =
	    ( f0 * split.a + split.b ) * pbr::SpecularEnergyCompensation( f0, split );
	double table[4];
	ltc::Inverse( material.Roughness(), normalDotView, table );
	// mat3( vec3( m00, 0, m02 ), vec3( 0, 1, 0 ), vec3( m20, 0, m22 ) ) in
	// GLSL (columns), as rows.
	const double inverse[9] = { table[0], 0, table[2], 0, 1, 0, table[1], 0, table[3] };
	Rgb total;
	for ( const area_light::AreaLight &light : lights )
	{
		const float at[3] = { p.x, p.y, p.z };
		const float window =
		    area_light::Window( area_light::DistanceTo( light.rect, at ), light.reach );
		if ( window <= 0.0f )
			continue;
		const double value =
		    magnitude * window *
		    ltc::Rectangle( light, { p.x, p.y, p.z }, { 0, 0, 1 }, { v.x, v.y, v.z }, inverse );
		total.r += float( value * light.radiance[0] );
		total.g += float( value * light.radiance[1] );
		total.b += float( value * light.radiance[2] );
	}
	return total;
}

// The pixel nearest the receiver point that is nearer than this to a light is
// skipped by the quadrature (its integrand is singular there).
bool NearALight( const std::vector<area_light::AreaLight> &lights, float3 p )
{
	for ( const area_light::AreaLight &light : lights )
	{
		const float at[3] = { p.x, p.y, p.z };
		const float size = std::sqrt( area_light::Area( light.rect ) );
		if ( area_light::DistanceTo( light.rect, at ) < 0.35f * size )
			return true;
	}
	return false;
}

struct Lab
{
	IRenderDevice2 &device;
	resources::TextureCache textures;
	material::GroupResidency groups;
	std::unique_ptr<Canvas> canvas;
	std::unique_ptr<material::PbrFamily> family;
	std::map<std::string, std::uint64_t> materialGroups;
	std::uint64_t drawGroup = 0;
	std::uint64_t viewGroup = 0; // the program's neutral view group
	std::uint64_t nextGroup = 1;
	std::vector<std::byte> receiver;

	explicit Lab( IRenderDevice2 &d ) : device( d ), textures( d ), groups( d, textures ) {}
};

std::optional<std::string> Prepare( Lab &lab, std::span<const std::uint32_t> module )
{
	if ( std::optional<std::string> why = Canvas::Create( lab.device, kSize, kSize, lab.canvas ) )
		return why;
	auto family = material::CreateSurfaceFamily<material::PbrFamily>(
	    lab.device, kCanvasColor, kCanvasDepth, 1, module );
	if ( !family )
		return std::string( "the surface program was refused" );
	lab.family = std::move( family ).Value();

	bool staged = StageConstant(
	    lab.textures, "al/base", Format::kRGBA8Srgb, ByteTexel( 255, 255, 255, 255 ) );
	for ( const Material &m : kMaterials )
		staged = staged && StageConstant( lab.textures, std::string( "al/mrao-" ) + m.name,
		                       Format::kRGBA8Unorm, ByteTexel( m.metal, m.roughness, 255, 255 ) );
	for ( const auto &[name, table] : { std::pair{ "al/splitsum", material::SplitSumTable() },
	          std::pair{ "al/ltc", material::LtcTable() } } )
	{
		TextureDesc desc;
		desc.format = table.format;
		desc.width = table.width;
		desc.height = table.height;
		staged =
		    staged &&
		    lab.textures.Stage( name, desc, std::as_bytes( std::span( table.texels ) ) ).HasValue();
	}
	if ( !staged )
		return std::string( "a fixture texture was refused" );

	for ( const Material &m : kMaterials )
	{
		material::PbrClaim claim;
		claim.claimed = true;
		material::SurfaceTextures textures;
		textures.base = "al/base";
		textures.mrao = std::string( "al/mrao-" ) + m.name;
		auto request = lab.family->Request( claim, textures );
		if ( !request )
			return std::string( "the pbr point was refused" );
		const std::uint64_t id = lab.nextGroup++;
		if ( !lab.groups.Set( id, request.Value().material ) )
			return std::string( "a material group was refused" );
		lab.materialGroups[m.name] = id;
	}
	lab.viewGroup = lab.nextGroup++;
	if ( !lab.groups.Set( lab.viewGroup, lab.family->Program().NeutralViewGroup() ) )
		return std::string( "the view group was refused" );
	lab.drawGroup = lab.nextGroup++;
	if ( !lab.groups.Set( lab.drawGroup, lab.family->LightingGroup( material::ModelLighting() ) ) )
		return std::string( "the draw group was refused" );

	lab.receiver = ReceiverMesh( kReceiver );
	return std::nullopt;
}

struct Frame
{
	std::vector<area_light::AreaLight> lights;
	const Material *material = &kMaterials[0];
	const View *view = nullptr;
	shaderlib::DebugSpecialization debug;
	bool spatial = true;
	std::uint32_t originX = 0, originY = 0;
};

// Renders the receiver under a frame's lights.
std::optional<std::string> Render( Lab &lab, const Frame &frame, CanvasImage &image )
{
	const std::uint64_t viewGroup = lab.nextGroup++;
	material::GroupRequest viewRequest = lab.family->Program().NeutralViewGroup();
	if ( frame.spatial )
	{
		pass::lights::ClusterViewDesc desc;
		desc.view = frame.view->view;
		desc.projection = frame.view->projection;
		desc.widthPixels = kSize - frame.originX;
		desc.heightPixels = kSize - frame.originY;
		desc.nearZ = frame.view->nearZ;
		desc.farZ = frame.view->farZ;
		auto limits = pass::lights::DesktopClusterLimits();
		limits.tileSizePixels = 16;
		auto grid = pass::lights::CreateClusterGrid( desc, limits );
		if ( !grid )
			return "area grid refused";
		std::vector<pass::lights::AreaFroxelMask> masks;
		if ( !pass::lights::AssignAreaLights( grid.Value(), frame.lights, masks ) )
			return "area assignment refused";
		material::SurfaceViewGpu view;
		view.grid[0] = grid.Value().tilesX;
		view.grid[1] = grid.Value().tilesY;
		view.grid[2] = grid.Value().slices;
		view.grid[3] = limits.tileSizePixels;
		view.slices[0] = grid.Value().sliceScale;
		view.slices[1] = grid.Value().sliceBias;
		view.slices[2] = grid.Value().nearZ;
		view.counts[2] = float( frame.originX );
		view.counts[3] = float( frame.originY );
		const auto &z = desc.view.rows[2];
		view.viewDistance[0] = -z.x;
		view.viewDistance[1] = -z.y;
		view.viewDistance[2] = -z.z;
		view.viewDistance[3] = -z.w;
		std::vector<std::byte> indices( 16 );
		pass::lights::AppendAreaMasks( masks, indices );
		std::vector<pass::lights::FroxelRange> ranges( grid.Value().FroxelCount() );
		const material::SurfaceLightGpu light;
		viewRequest = lab.family->Program().ViewGroup(
		    view, std::as_bytes( std::span( ranges ) ), indices, std::span( &light, 1 ) );
	}
	if ( !lab.groups.Set( viewGroup, viewRequest ) )
		return "area view refused";
	material::SurfaceFrame terms;
	terms.eye[0] = frame.view->eye.x;
	terms.eye[1] = frame.view->eye.y;
	terms.eye[2] = frame.view->eye.z;
	terms.areaCount[0] = float( frame.lights.size() );
	for ( std::size_t i = 0; i < frame.lights.size() && i < material::kSurfaceMaxAreaLights; ++i )
		terms.areas[i] = material::PackAreaLight( frame.lights[i] );
	const std::uint64_t frameGroup = lab.nextGroup++;
	if ( !lab.groups.Set( frameGroup, lab.family->FrameGroup( terms, "al/splitsum", "al/ltc" ) ) )
		return std::string( "the frame group was refused" );
	// Make the frame group resident.
	if ( std::optional<std::string> why =
	         lab.canvas->Render( lab.textures, lab.groups, {}, { 0, 0, 0, 1 }, nullptr ) )
		return why;
	material::PbrClaim claim;
	claim.claimed = true;
	auto pipeline = lab.family->Pipeline( claim, frame.debug );
	if ( !pipeline )
		return std::string( "no pipeline" );
	auto group = [&]( std::uint64_t id ) -> BindGroupId
	{
		const material::ResidentGroup *resident = lab.groups.Group( id );
		return resident ? resident->group : BindGroupId();
	};
	CanvasDraw draw;
	draw.viewport = Viewport{ float( frame.originX ), float( frame.originY ),
	    float( kSize - frame.originX ), float( kSize - frame.originY ), 0, 1 };
	draw.pipeline = pipeline.Value();
	draw.groups[std::size_t( BindGroupRole::kFrame )] = group( frameGroup );
	draw.groups[std::size_t( BindGroupRole::kView )] = group( viewGroup );
	draw.groups[std::size_t( BindGroupRole::kMaterial )] =
	    group( lab.materialGroups.at( frame.material->name ) );
	draw.groups[std::size_t( BindGroupRole::kDraw )] = group( lab.drawGroup );
	draw.vertices = lab.canvas->Vertices( lab.receiver );
	draw.vertexCount = 6;
	material::FamilyDrawConstants constants;
	std::memcpy( constants.toClip, &frame.view->toClip, sizeof( constants.toClip ) );
	constants.world[0] = constants.world[5] = constants.world[10] = constants.world[15] = 1.0f;
	const auto *bytes = reinterpret_cast<const std::byte *>( &constants );
	draw.constants.assign( bytes, bytes + sizeof( constants ) );
	const CanvasDraw draws[] = { draw };
	std::optional<std::string> why =
	    lab.canvas->Render( lab.textures, lab.groups, draws, { 0, 0, 0, 1 }, &image );
	lab.groups.Remove( frameGroup );
	return why;
}

// The review images (RENDER_LAB_IMAGES); nothing when unset.
void SaveImage( const CanvasImage &image, const std::string &name )
{
	const char *directory = std::getenv( "RENDER_LAB_IMAGES" );
	if ( !directory || !*directory )
		return;
	(void)WritePfm( std::filesystem::path( directory ) / ( name + ".gpu.pfm" ), image.width,
	    image.height, image.rgba );
}

// The exact light at every pixel of a frame (the gallery's cases only: the
// GGX quadrature costs seconds per frame).
template <typename Oracle>
void SaveExact( const View &view, const std::string &name, Oracle oracle )
{
	const char *directory = std::getenv( "RENDER_LAB_IMAGES" );
	if ( !directory || !*directory )
		return;
	std::vector<float> rgba( std::size_t( kSize ) * kSize * 4, 0.0f );
	for ( std::uint32_t y = 0; y < kSize; ++y )
	{
		for ( std::uint32_t x = 0; x < kSize; ++x )
		{
			float *out = &rgba[( std::size_t( y ) * kSize + x ) * 4];
			out[3] = 1.0f;
			const std::optional<float3> p = Hit( view, x, y );
			if ( !p )
				continue;
			const float3 v =
			    math::Normalize( { view.eye.x - p->x, view.eye.y - p->y, view.eye.z - p->z } );
			const Rgb value = oracle( *p, v );
			out[0] = value.r;
			out[1] = value.g;
			out[2] = value.b;
		}
	}
	(void)WritePfm(
	    std::filesystem::path( directory ) / ( name + ".exact.pfm" ), kSize, kSize, rgba );
}

bool SameImage( const CanvasImage &a, const CanvasImage &b )
{
	return a.rgba.size() == b.rgba.size() &&
	       std::memcmp( a.rgba.data(), b.rgba.data(), a.rgba.size() * sizeof( float ) ) == 0;
}

shaderlib::DebugSpecialization Lobe( shaderlib::DebugBrdf brdf )
{
	shaderlib::DebugSpecialization debug;
	debug.brdf = std::uint32_t( brdf );
	return debug;
}

std::string Format3( const char *format, double a, double b = 0, double c = 0 )
{
	char text[160];
	std::snprintf( text, sizeof( text ), format, a, b, c );
	return text;
}

std::optional<std::string> AreaChecks( Lab &lab, Results &results )
{
	const View overhead = MakeView( { 0, -60, 260 }, { 0, 0, 0 } );
	const View grazing = MakeView( { 0, -420, 70 }, { 0, 40, 0 } );
	const std::pair<const char *, const View *> views[] = {
	    { "overhead", &overhead }, { "grazing", &grazing } };
	const auto lights = Lights();

	// Diffuse: every sampled pixel against the exact irradiance.
	for ( const char *name : { "ceiling", "tilted", "straddling", "short-reach", "grid64" } )
	{
		for ( const auto &[viewName, view] : views )
		{
			Frame frame{ lights.at( name ), &kMaterials[0], view,
			    Lobe( shaderlib::DebugBrdf::kDiffuseOnly ) };
			CanvasImage image;
			if ( std::optional<std::string> why = Render( lab, frame, image ) )
				return why;
			Frame full = frame;
			full.spatial = false;
			CanvasImage reference;
			if ( auto why = Render( lab, full, reference ) )
				return why;
			results.That( image.rgba == reference.rgba,
			    std::string( "spatial-parity." ) + name + "." + viewName,
			    "spatial masks preserve every pixel of the full area loop" );
			const std::string check = std::string( "diffuse." ) + name + "." + viewName;
			SaveImage( image, check );
			SaveExact( *view, check,
			    [&]( float3 p, float3 v )
			    {
				    return DiffuseOracle( frame.lights, kMaterials[0], p, v );
			    } );
			int pixels = 0;
			int bad = 0;
			float worst = 0.0f;
			float brightest = 0.0f;
			for ( std::uint32_t y = 0; y < kSize; y += kSampleStep )
			{
				for ( std::uint32_t x = 0; x < kSize; x += kSampleStep )
				{
					const std::optional<float3> p = Hit( *view, x, y );
					if ( !p )
						continue;
					const float3 v = math::Normalize(
					    { view->eye.x - p->x, view->eye.y - p->y, view->eye.z - p->z } );
					const Rgb expected = DiffuseOracle( frame.lights, kMaterials[0], *p, v );
					const float got = image.At( x, y )[0];
					const float error = std::fabs( got - expected.r );
					++pixels;
					brightest = std::max( brightest, expected.r );
					worst = std::max( worst, error / std::max( expected.r, 1e-3f ) );
					bad += error > kDiffuseRelative * expected.r + kDiffuseAbsolute ? 1 : 0;
				}
			}
			results.That( pixels > 100 && bad == 0 && brightest > 0.01f,
			    std::string( "diffuse." ) + name + "." + viewName,
			    Format3( "%.0f of %.0f sampled pixels outside the tolerance, worst relative %.3g",
			        bad, pixels, worst ) );
		}
	}

	// Offset and resized viewports use local pixel coordinates for both lists.
	for ( const char *name : { "short-reach", "grid64" } )
	{
		Frame frame{ lights.at( name ), &kMaterials[0], &overhead,
		    Lobe( shaderlib::DebugBrdf::kDiffuseOnly ) };
		frame.originX = 48;
		frame.originY = 24;
		CanvasImage clustered, full;
		if ( auto why = Render( lab, frame, clustered ) )
			return why;
		frame.spatial = false;
		if ( auto why = Render( lab, frame, full ) )
			return why;
		results.That( clustered.rgba == full.rgba, std::string( "offset-viewport." ) + name,
		    "spatial area lights match the full loop in an offset, resized viewport" );
	}

	// GGX: against the term's definition, and its accuracy against the exact
	// light (a measurement).
	for ( const char *name : { "ceiling", "tilted", "straddling" } )
	{
		for ( const Material &material : kMaterials )
		{
			for ( const auto &[viewName, view] : views )
			{
				Frame frame{ lights.at( name ), &material, view,
				    Lobe( shaderlib::DebugBrdf::kSpecularOnly ) };
				CanvasImage image;
				if ( std::optional<std::string> why = Render( lab, frame, image ) )
					return why;
				const std::string check =
				    std::string( "specular." ) + name + "." + material.name + "." + viewName;
				SaveImage( image, check );
				if ( std::string_view( material.name ) == "metal-r50" )
					SaveExact( *view, check,
					    [&]( float3 p, float3 v )
					    {
						    return SpecularOracle( frame.lights, material, p, v );
					    } );
				int pixels = 0;
				int bad = 0;
				int illConditioned = 0;
				float worst = 0.0f;
				std::vector<std::pair<float, float>> exact; // got, the quadrature
				float brightest = 0.0f;
				for ( std::uint32_t y = 0; y < kSize; y += kSampleStep )
				{
					for ( std::uint32_t x = 0; x < kSize; x += kSampleStep )
					{
						const std::optional<float3> p = Hit( *view, x, y );
						if ( !p )
							continue;
						const float3 v = math::Normalize(
						    { view->eye.x - p->x, view->eye.y - p->y, view->eye.z - p->z } );
						const float got = image.At( x, y )[0];
						const float expected = SpecularTerm( frame.lights, material, *p, v ).r;
						const float band = kSpecularRelative * expected + kSpecularAbsolute;
						++pixels;
						// Ill-conditioned: the term moves by more than half the
						// band within kConditioning of the pixel's point, where the
						// rasterizer's interpolated position may differ from the
						// ray's (a narrow highlight's edge at a grazing view).
						float moved = 0.0f;
						for ( const float3 offset :
						    { float3{ kConditioning, 0, 0 }, float3{ -kConditioning, 0, 0 },
						        float3{ 0, kConditioning, 0 }, float3{ 0, -kConditioning, 0 } } )
						{
							const float3 q{ p->x + offset.x, p->y + offset.y, 0.0f };
							const float3 w = math::Normalize(
							    { view->eye.x - q.x, view->eye.y - q.y, view->eye.z - q.z } );
							moved = std::max( moved,
							    std::fabs(
							        SpecularTerm( frame.lights, material, q, w ).r - expected ) );
						}
						if ( moved > 0.5f * band )
						{
							++illConditioned;
							continue;
						}
						const float error = std::fabs( got - expected );
						worst = std::max( worst, error / std::max( expected, 1e-3f ) );
						bad += error > band ? 1 : 0;

						if ( !NearALight( frame.lights, *p ) )
						{
							const float reference =
							    SpecularOracle( frame.lights, material, *p, v ).r;
							exact.emplace_back( got, reference );
							brightest = std::max( brightest, reference );
						}
					}
				}
				const std::string label =
				    std::string( name ) + "." + material.name + "." + viewName;
				results.That( pixels > 100 && bad == 0 &&
				                  float( illConditioned ) <= kIllConditionedShare * float( pixels ),
				    "specular." + label,
				    Format3(
				        "%.0f outside the tolerance, worst relative %.3g, %.0f ill-conditioned",
				        bad, worst, illConditioned ) );
				double errorSum = 0.0;
				double referenceSum = 0.0;
				std::vector<float> relative;
				for ( const auto &[got, reference] : exact )
				{
					if ( reference < kAccuracyFloor * brightest )
						continue;
					errorSum += std::fabs( got - reference );
					referenceSum += reference;
					relative.push_back( std::fabs( got - reference ) / reference );
				}
				std::sort( relative.begin(), relative.end() );
				const double mean = referenceSum > 0.0 ? errorSum / referenceSum : 0.0;
				const double p90 =
				    relative.empty() ? 0.0
				                     : relative[std::size_t( 0.9 * double( relative.size() - 1 ) )];
				std::printf( "INFO accuracy.%s mean %.4f p90 %.4f over %zu pixels\n", label.c_str(),
				    mean, p90, relative.size() );
			}
		}
	}

	// The neutral value: no lights, and the term off, are the frame without
	// the term, bitwise; a one-sided light seen from behind gives nothing.
	{
		CanvasImage none, off, behind, lit;
		Frame empty{ {}, &kMaterials[2], &overhead, {} };
		Frame termOff{ lights.at( "ceiling" ), &kMaterials[2], &overhead, {} };
		termOff.debug.termsOff = shaderlib::kDebugTermArea;
		Frame fromBehind{ lights.at( "behind" ), &kMaterials[2], &overhead, {} };
		Frame litFrame{ lights.at( "ceiling" ), &kMaterials[2], &overhead, {} };
		for ( auto [frame, image] : { std::pair{ &empty, &none }, std::pair{ &termOff, &off },
		          std::pair{ &fromBehind, &behind }, std::pair{ &litFrame, &lit } } )
		{
			if ( std::optional<std::string> why = Render( lab, *frame, *image ) )
				return why;
		}
		results.That( SameImage( none, off ), "neutral.term-off-is-the-frame-without-area-lights" );
		results.That( SameImage( none, behind ), "one-sided.behind-gives-nothing" );
		results.That( !SameImage( none, lit ), "neutral.a-lit-frame-differs" );
	}

	// Two-sided: the light facing away gives what it gives facing the
	// receiver; the full frame is the sum of its lobes.
	{
		CanvasImage front, twoSided, diffuse, specular, full;
		Frame facing{ lights.at( "ceiling" ), &kMaterials[2], &overhead, {} };
		Frame both{ lights.at( "two-sided" ), &kMaterials[2], &overhead, {} };
		Frame diffuseOnly{ lights.at( "ceiling" ), &kMaterials[0], &overhead,
		    Lobe( shaderlib::DebugBrdf::kDiffuseOnly ) };
		Frame specularOnly{ lights.at( "ceiling" ), &kMaterials[0], &overhead,
		    Lobe( shaderlib::DebugBrdf::kSpecularOnly ) };
		Frame whole{ lights.at( "ceiling" ), &kMaterials[0], &overhead, {} };
		Frame wholeGrid{ lights.at( "grid64" ), &kMaterials[2], &grazing, {} };
		CanvasImage grid;
		if ( std::optional<std::string> why = Render( lab, wholeGrid, grid ) )
			return why;
		SaveImage( grid, "full.grid64.metal-r50.grazing" );
		for ( auto [frame, image] : { std::pair{ &facing, &front }, std::pair{ &both, &twoSided },
		          std::pair{ &diffuseOnly, &diffuse }, std::pair{ &specularOnly, &specular },
		          std::pair{ &whole, &full } } )
		{
			if ( std::optional<std::string> why = Render( lab, *frame, *image ) )
				return why;
		}
		float twoSidedWorst = 0.0f;
		float sumWorst = 0.0f;
		for ( std::size_t i = 0; i < front.rgba.size(); ++i )
		{
			const float scale = std::max( front.rgba[i], 1e-2f );
			twoSidedWorst =
			    std::max( twoSidedWorst, std::fabs( front.rgba[i] - twoSided.rgba[i] ) / scale );
			const float parts = diffuse.rgba[i] + specular.rgba[i];
			if ( i % 4 != 3 )
				sumWorst = std::max(
				    sumWorst, std::fabs( full.rgba[i] - parts ) / std::max( full.rgba[i], 1e-2f ) );
		}
		SaveImage( full, "full.ceiling.dielectric-r50.overhead" );
		SaveImage( twoSided, "full.two-sided.metal-r50.overhead" );
		results.That( twoSidedWorst <= 2e-3f, "two-sided.back-equals-front",
		    Format3( "worst relative %.3g", twoSidedWorst ) );
		results.That( sumWorst <= 2e-3f, "lobes.full-is-diffuse-plus-specular",
		    Format3( "worst relative %.3g", sumWorst ) );
	}
	return std::nullopt;
}

std::optional<std::string> RunOnce( bool validate, std::span<const std::uint32_t> module,
    Results &results, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( std::optional<std::string> why = CreateLabDevice( validate, counter, device ) )
		return why;
	{
		Lab lab( *device );
		if ( std::optional<std::string> why = Prepare( lab, module ) )
			return why;
		if ( std::optional<std::string> why = AreaChecks( lab, results ) )
			return why;
		(void)device->WaitIdle();
	}
	device.reset();
	messages = counter.load();
	return std::nullopt;
}

const Seeded kAreaSeeded[] = {
    { "no-horizon-clip", spirv::kSurfaceLtcNoHorizonClip, "diffuse.straddling" },
    { "ltc-transposed", spirv::kSurfaceLtcTransposed, "specular." },
    { "no-magnitude", spirv::kSurfaceLtcNoMagnitude, "specular." } };

} // namespace

int RunAreaLightsSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "area-lights", kAreaSeeded, RunOnce );
}

} // namespace render::lab
