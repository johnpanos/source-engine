//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.lab.volumetric (RFC 0016 K11 "Volumetric fog", the
//			participating-media term): render.pass.volumetric over an
//			opaque wall 600 units from the eye, drawn by the unlit point, on
//			render.pass.lights' light grid subdivided 8 x 4, against a C++
//			oracle in double that shares no code with the shaders:
//
//			transmittance  a homogeneous absorbing medium (albedo 0) gives the
//			               wall's radiance times exp( -sigma_t d ), d the
//			               ray's length to the wall, within 0.5 percent +
//			               1e-4 at every pixel, at two densities; a height
//			               fog gives exp( -its analytic optical depth ) within
//			               2 percent + 1e-4;
//			emission       a purely emitting medium gives e ( 1 - T ) /
//			               sigma_t within 0.5 percent + 1e-4;
//			scatter.point  single scattering of a point light (inverse square,
//			               source radius 2) in a medium of albedo 0.6 and
//			               Henyey-Greenstein g 0.5 (and g -0.3) before a black
//			               wall matches a numerical integral along each pixel's
//			               ray (Simpson in the angle about the ray's closest
//			               approach, 2,000 intervals) with the medium's
//			               transmittance on both legs: over the pixels whose
//			               ray passes 50 units or more from the light, the
//			               mean relative error is at most 3 percent and the
//			               95th percentile at most 8 percent, and the frame's
//			               mean within 2 percent (tolerances fixed before the
//			               first run, 2026-09-29);
//			neutral        density zero (the scattering and absorbing medium
//			               and the light above, scaled by 0) and an empty
//			               medium are bitwise the frame without the pass, with
//			               the surface program's legacy range fog on;
//			history        a frame after a camera cut is bitwise the frame a
//			               new renderer draws for that camera (the pass keeps
//			               no history).
//
//			Seeded stages (--sensitivity, volumetric_defects_spv.h): the phase
//			ignored and the albedo ignored (inject) fail scatter.point;
//			extinction applied twice and the slice off by one (composite) fail
//			transmittance.
//
//			RENDER_LAB_IMAGES=<dir> writes each case's frame and oracle.
//
//=============================================================================//

#include "lab_canvas.h"
#include "lab_media.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "render/device/device.h"
#include "render/material/unlit_family.h"
#include "render/math/matrix.h"
#include "render/pass/lights/clusters.h"
#include "render/pass/volumetric/volumetric.h"
#include "spv/volumetric_defects_spv.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace render::lab
{

namespace
{

using namespace render::device;
namespace vol = render::pass::volumetric;
namespace lights = render::pass::lights;
using math::float3;

constexpr std::uint32_t kWidth = 192;
constexpr std::uint32_t kHeight = 144;
constexpr double kWall = 600.0;
constexpr double kHorizontalFov = 90.0;
constexpr double kPi = 3.14159265358979323846;

struct Camera
{
	float3 eye{ 0.0f, 0.0f, 0.0f };
	float3 forward{ 1.0f, 0.0f, 0.0f };
	float3 up{ 0.0f, 0.0f, 1.0f };
};

struct Lab
{
	IRenderDevice2 &device;
	resources::TextureCache textures;
	material::GroupResidency groups;
	std::unique_ptr<Canvas> canvas;
	std::unique_ptr<material::UnlitFamily> family;
	vol::VolumetricPrograms programs;
	std::uint64_t nextGroup = 1;
	std::uint64_t drawGroup = 0;
	std::vector<std::byte> wall;

	explicit Lab( IRenderDevice2 &d ) : device( d ), textures( d ), groups( d, textures ) {}
};

std::optional<std::string> Prepare( Lab &lab )
{
	if ( std::optional<std::string> why =
	         Canvas::Create( lab.device, kWidth, kHeight, lab.canvas ) )
		return why;
	auto family = material::UnlitFamily::Create( lab.device, kCanvasColor, kCanvasDepth );
	if ( !family )
		return std::string( "the unlit point was refused" );
	lab.family = std::move( family ).Value();
	if ( !StageConstant(
	         lab.textures, "vol/white", Format::kRGBA8Srgb, ByteTexel( 255, 255, 255, 255 ) ) )
		return std::string( "the white texture was refused" );
	lab.drawGroup = lab.nextGroup++;
	if ( !lab.groups.Set( lab.drawGroup, lab.family->DrawGroup() ) )
		return std::string( "the draw group was refused" );
	// The wall x = kWall, both windings (one is culled).
	const float s = 5000.0f;
	const float corners[4][2] = { { -s, -s }, { s, -s }, { s, s }, { -s, s } };
	for ( int corner : { 0, 1, 2, 0, 2, 3, 0, 2, 1, 0, 3, 2 } )
	{
		material::SurfaceFlatVertex vertex;
		vertex.position[0] = float( kWall );
		vertex.position[1] = corners[corner][0];
		vertex.position[2] = corners[corner][1];
		const auto *bytes = reinterpret_cast<const std::byte *>( &vertex );
		lab.wall.insert( lab.wall.end(), bytes, bytes + sizeof( vertex ) );
	}
	return std::nullopt;
}

struct Scene
{
	Camera camera;
	vol::Medium medium;
	std::vector<vol::MediumLight> lights;
	float wall[3] = { 1.0f, 1.0f, 1.0f };
	bool legacyFog = false;
	bool pass = true;
};

struct ViewMatrices
{
	math::float4x4 view;
	math::float4x4 projection;
};

ViewMatrices Matrices( const Camera &camera )
{
	const float aspect = float( kWidth ) / float( kHeight );
	const float horizontal = float( kHorizontalFov * kPi / 180.0 );
	const float vertical = 2.0f * std::atan( std::tan( horizontal / 2.0f ) / aspect );
	const float3 target = camera.eye + camera.forward;
	return { math::LookAt( camera.eye, target, camera.up ),
	    math::Perspective( vertical, aspect, 1.0f, 65536.0f ) };
}

// Draws the scene's wall and, with `renderer`, the pass over it.
std::optional<std::string> Render(
    Lab &lab, const Scene &scene, vol::VolumetricRenderer *renderer, CanvasImage &image )
{
	const ViewMatrices m = Matrices( scene.camera );
	material::SurfaceFrame terms;
	if ( scene.legacyFog )
	{
		terms.fogColor[0] = 0.3f;
		terms.fogColor[1] = 0.4f;
		terms.fogColor[2] = 0.5f;
		terms.fogColor[3] = 0.0f; // range fog
		terms.fogParams[0] = 0.0f;
		terms.fogParams[2] = 0.9f;
		terms.fogParams[3] = 1.0f / 800.0f;
	}
	const std::uint64_t frameGroup = lab.nextGroup++;
	const std::uint64_t materialGroup = lab.nextGroup++;
	material::UnlitClaim claim;
	claim.claimed = true;
	std::copy( scene.wall, scene.wall + 3, claim.constants.tint );
	auto request = lab.family->Request( claim, "vol/white" );
	if ( !request || !lab.groups.Set( frameGroup, lab.family->FrameGroup( terms ) ) ||
	     !lab.groups.Set( materialGroup, request.Value().material ) )
		return std::string( "a group was refused" );
	if ( std::optional<std::string> why =
	         lab.canvas->Render( lab.textures, lab.groups, {}, { 0, 0, 0, 1 }, nullptr ) )
		return why;
	auto pipeline = lab.family->Pipeline( claim );
	if ( !pipeline )
		return std::string( "no pipeline" );
	const auto group = [&]( std::uint64_t id ) -> BindGroupId
	{
		const material::ResidentGroup *resident = lab.groups.Group( id );
		return resident ? resident->group : BindGroupId();
	};
	CanvasDraw draw;
	draw.pipeline = pipeline.Value();
	draw.groups[std::size_t( BindGroupRole::kFrame )] = group( frameGroup );
	draw.groups[std::size_t( BindGroupRole::kMaterial )] = group( materialGroup );
	draw.groups[std::size_t( BindGroupRole::kDraw )] = group( lab.drawGroup );
	draw.vertices = lab.canvas->Vertices( lab.wall );
	draw.vertexCount = 12;
	material::FamilyDrawConstants constants;
	const math::float4x4 toClip = math::Multiply( m.projection, m.view );
	std::memcpy( constants.toClip, &toClip, sizeof( constants.toClip ) );
	constants.world[0] = constants.world[5] = constants.world[10] = constants.world[15] = 1.0f;
	const auto *bytes = reinterpret_cast<const std::byte *>( &constants );
	draw.constants.assign( bytes, bytes + request.Value().drawConstantBytes );
	const CanvasDraw draws[] = { draw };

	lights::ClusterViewDesc desc;
	desc.view = m.view;
	desc.projection = m.projection;
	desc.widthPixels = kWidth;
	desc.heightPixels = kHeight;
	desc.nearZ = 1.0f;
	desc.farZ = 65536.0f;
	auto lightGrid = lights::CreateClusterGrid( desc, lights::DesktopClusterLimits() );
	if ( !lightGrid )
		return std::string( "the light grid does not build" );
	auto grid = lights::SubdivideClusterGrid( lightGrid.Value(), 8, 4 );
	if ( !grid )
		return std::string( "the fog grid does not subdivide" );
	const vol::FroxelLayout layout = FroxelLayoutOf( grid.Value() );
	vol::VolumetricView view;
	view.froxels = &layout;
	view.projection = m.projection;
	view.eye = scene.camera.eye;
	vol::VolumetricFrame frame;
	frame.medium = &scene.medium;
	frame.lights = scene.lights;
	CanvasPost post;
	if ( renderer && scene.pass )
		post = [&]( CommandEncoder &encoder, TextureId color,
		           TextureId depth ) -> std::optional<std::string>
		{
			vol::VolumetricTargets targets;
			targets.color = color;
			targets.depth = depth;
			targets.width = kWidth;
			targets.height = kHeight;
			if ( !renderer->Record( encoder, view, frame, targets ) )
				return std::string( "the volumetric pass did not record" );
			return std::nullopt;
		};
	std::optional<std::string> why =
	    lab.canvas->Render( lab.textures, lab.groups, draws, { 0, 0, 0, 1 }, &image, post );
	if ( renderer )
		renderer->Collect( CompletionToken() ); // the canvas waited idle
	lab.groups.Remove( frameGroup );
	lab.groups.Remove( materialGroup );
	return why;
}

// ---- the oracle (double; shares no code with the shaders)

struct D3
{
	double x = 0, y = 0, z = 0;
};
D3 Add( D3 a, D3 b )
{
	return { a.x + b.x, a.y + b.y, a.z + b.z };
}
D3 Sub( D3 a, D3 b )
{
	return { a.x - b.x, a.y - b.y, a.z - b.z };
}
D3 Mul( D3 a, double s )
{
	return { a.x * s, a.y * s, a.z * s };
}
double Dot( D3 a, D3 b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
D3 CrossD( D3 a, D3 b )
{
	return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}
D3 Unit( D3 a )
{
	return Mul( a, 1.0 / std::sqrt( Dot( a, a ) ) );
}
D3 Of( float3 v )
{
	return { v.x, v.y, v.z };
}

// The unit ray through a pixel's centre.
D3 PixelRay( const Camera &camera, double px, double py )
{
	const D3 forward = Unit( Of( camera.forward ) );
	const D3 right = Unit( CrossD( forward, Of( camera.up ) ) );
	const D3 up = CrossD( right, forward );
	const double tanH = std::tan( kHorizontalFov * kPi / 360.0 );
	const double tanV = tanH * double( kHeight ) / double( kWidth );
	const double ndcX = 2.0 * px / kWidth - 1.0;
	const double ndcY = 1.0 - 2.0 * py / kHeight;
	return Unit( Add( forward, Add( Mul( right, ndcX * tanH ), Mul( up, ndcY * tanV ) ) ) );
}

double WallDistance( const Camera &camera, D3 ray )
{
	return ( kWall - double( camera.eye.x ) ) / ray.x;
}

double Phase( double g, double c )
{
	return ( 1.0 - g * g ) / ( 4.0 * kPi * std::pow( 1.0 + g * g - 2.0 * g * c, 1.5 ) );
}

// In-scattered radiance (one channel of `color`) along a ray of length
// `length` in a homogeneous medium filling space, from one inverse-square
// point light: the integral of T(t) sigma_s p pi E(x) T_light(x) dt.
double PointScatter( D3 eye, D3 ray, double length, D3 light, double color, double extinction,
    double albedo, double g, double sourceRadius )
{
	const double t0 = Dot( Sub( light, eye ), ray );
	const D3 closest = Add( eye, Mul( ray, t0 ) );
	const double b =
	    std::max( std::sqrt( Dot( Sub( light, closest ), Sub( light, closest ) ) ), 1e-3 );
	const double u0 = std::atan( ( 0.0 - t0 ) / b );
	const double u1 = std::atan( ( length - t0 ) / b );
	const int n = 2000;
	const double h = ( u1 - u0 ) / n;
	double sum = 0.0;
	for ( int i = 0; i <= n; ++i )
	{
		const double u = u0 + h * i;
		const double t = t0 + b * std::tan( u );
		const double dt = b / ( std::cos( u ) * std::cos( u ) );
		const D3 x = Add( eye, Mul( ray, t ) );
		const D3 toX = Sub( x, light );
		const double d2 = Dot( toX, toX );
		const double d = std::sqrt( d2 );
		const double irradiance = kPi * color * 1.0e4 / std::max( d2, sourceRadius * sourceRadius );
		const double c = Dot( Mul( toX, 1.0 / d ), Mul( ray, -1.0 ) );
		const double value = std::exp( -extinction * t ) * extinction * albedo * Phase( g, c ) *
		                     irradiance * std::exp( -extinction * d ) * dt;
		const double weight = ( i == 0 || i == n ) ? 1.0 : ( i % 2 ? 4.0 : 2.0 );
		sum += weight * value;
	}
	return sum * h / 3.0;
}

bool SameImage( const CanvasImage &a, const CanvasImage &b )
{
	return a.rgba.size() == b.rgba.size() &&
	       std::memcmp( a.rgba.data(), b.rgba.data(), a.rgba.size() * sizeof( float ) ) == 0;
}

void SaveImage( const std::vector<float> &rgba, const std::string &name )
{
	const char *directory = std::getenv( "RENDER_LAB_IMAGES" );
	if ( !directory || !*directory )
		return;
	(void)WritePfm( std::filesystem::path( directory ) / ( name + ".pfm" ), kWidth, kHeight, rgba );
}

std::string Text( const char *format, double a, double b = 0, double c = 0, double d = 0 )
{
	char text[200];
	std::snprintf( text, sizeof( text ), format, a, b, c, d );
	return text;
}

vol::FogVolume Everywhere( float extinction, float albedo, float g )
{
	vol::FogVolume volume;
	volume.mins = { -20000.0f, -20000.0f, -20000.0f };
	volume.maxs = { 20000.0f, 20000.0f, 20000.0f };
	volume.extinction = extinction;
	volume.albedo = albedo;
	volume.anisotropy = g;
	return volume;
}

// Every pixel within `relative` + 1e-4 of `expected( x, y, channel )`.
template <typename Expected>
void Judge( Results &results, const std::string &name, const CanvasImage &image, double relative,
    Expected expected )
{
	std::uint32_t outside = 0;
	double worst = 0.0;
	std::vector<float> exact( image.rgba.size(), 1.0f );
	for ( std::uint32_t y = 0; y < kHeight; ++y )
	{
		for ( std::uint32_t x = 0; x < kWidth; ++x )
		{
			for ( int c = 0; c < 3; ++c )
			{
				const double want = expected( x, y, c );
				const double got = image.At( x, y )[c];
				exact[( std::size_t( y ) * kWidth + x ) * 4 + c] = float( want );
				const double error = std::fabs( got - want );
				worst = std::max( worst, error / ( std::fabs( want ) + 1e-12 ) );
				if ( !( error <= relative * std::fabs( want ) + 1e-4 ) )
					++outside;
			}
		}
	}
	SaveImage( image.rgba, name + ".gpu" );
	SaveImage( exact, name + ".exact" );
	results.That( outside == 0, name,
	    Text( "%.0f of %.0f values outside; worst relative error %.5f", outside,
	        double( kWidth * kHeight * 3 ), worst ) );
}

std::optional<std::string> VolumetricChecks( Lab &lab, Results &results )
{
	auto created = vol::VolumetricRenderer::Create( lab.device, kCanvasColor, lab.programs );
	if ( !created )
		return std::string( "the volumetric pass was refused" );
	std::unique_ptr<vol::VolumetricRenderer> renderer = std::move( created ).Value();
	const Camera camera;

	// Transmittance: a homogeneous absorbing medium.
	for ( const float extinction : { 0.0f, 0.002f, 0.005f } )
	{
		Scene scene;
		scene.medium.volumes = { Everywhere( extinction, 0.0f, 0.0f ) };
		scene.wall[0] = 1.0f;
		scene.wall[1] = 0.6f;
		scene.wall[2] = 0.3f;
		CanvasImage image;
		if ( std::optional<std::string> why = Render( lab, scene, renderer.get(), image ) )
			return why;
		Judge( results, Text( "transmittance.homogeneous-%.3f", extinction ), image, 0.005,
		    [&]( std::uint32_t x, std::uint32_t y, int c )
		    {
			    const D3 ray = PixelRay( camera, x + 0.5, y + 0.5 );
			    return double( scene.wall[c] ) *
			           std::exp( -double( extinction ) * WallDistance( camera, ray ) );
		    } );
	}

	// Transmittance: a height fog (the controller's), absorbing only.
	{
		Scene scene;
		scene.camera.forward = math::Normalize( float3{ 1.0f, 0.0f, 0.15f } );
		scene.medium.fog.heightDensity = 0.004f;
		scene.medium.fog.heightFalloff = 0.002f;
		scene.medium.fog.baseHeight = -20.0f;
		scene.medium.fog.albedo = 0.0f;
		CanvasImage image;
		if ( std::optional<std::string> why = Render( lab, scene, renderer.get(), image ) )
			return why;
		Judge( results, "transmittance.height-fog", image, 0.02,
		    [&]( std::uint32_t x, std::uint32_t y, int )
		    {
			    const D3 ray = PixelRay( scene.camera, x + 0.5, y + 0.5 );
			    const double length = WallDistance( scene.camera, ray );
			    const double h0 = 0.004, f = 0.002, z0 = -20.0;
			    const double dz = ray.z;
			    const double at = std::exp( -f * ( 0.0 - z0 ) );
			    const double tau =
			        std::fabs( f * dz ) < 1e-9
			            ? h0 * at * length
			            : h0 * at * ( 1.0 - std::exp( -f * dz * length ) ) / ( f * dz );
			    return std::exp( -tau );
		    } );
	}

	// Emission: a purely emitting, absorbing medium before a black wall.
	{
		Scene scene;
		vol::FogVolume volume = Everywhere( 0.002f, 0.0f, 0.0f );
		volume.emission = { 0.001f, 0.002f, 0.003f };
		scene.medium.volumes = { volume };
		std::fill( scene.wall, scene.wall + 3, 0.0f );
		CanvasImage image;
		if ( std::optional<std::string> why = Render( lab, scene, renderer.get(), image ) )
			return why;
		const double e[3] = { 0.001, 0.002, 0.003 };
		Judge( results, "emission.homogeneous", image, 0.005,
		    [&]( std::uint32_t x, std::uint32_t y, int c )
		    {
			    const D3 ray = PixelRay( camera, x + 0.5, y + 0.5 );
			    const double t = WallDistance( camera, ray );
			    return e[c] * ( 1.0 - std::exp( -0.002 * t ) ) / 0.002;
		    } );
	}

	// Single scattering from a point light, against the numerical integral.
	const D3 lightAt{ 250.0, 90.0, 60.0 };
	const double color[3] = { 2.0, 1.6, 1.2 };
	const double extinction = 0.0012;
	const double albedo = 0.6;
	for ( const double g : { 0.5, -0.3 } )
	{
		Scene scene;
		scene.medium.volumes = { Everywhere( float( extinction ), float( albedo ), float( g ) ) };
		vol::MediumLight light;
		light.position = { float( lightAt.x ), float( lightAt.y ), float( lightAt.z ) };
		light.color = { float( color[0] ), float( color[1] ), float( color[2] ) };
		scene.lights = { light };
		std::fill( scene.wall, scene.wall + 3, 0.0f );
		CanvasImage image;
		if ( std::optional<std::string> why = Render( lab, scene, renderer.get(), image ) )
			return why;
		std::vector<double> errors;
		std::vector<float> exact( image.rgba.size(), 1.0f );
		double sumLab = 0.0, sumRef = 0.0;
		std::uint32_t excluded = 0;
		for ( std::uint32_t y = 0; y < kHeight; y += 2 )
		{
			for ( std::uint32_t x = 0; x < kWidth; x += 2 )
			{
				const D3 eye = Of( camera.eye );
				const D3 ray = PixelRay( camera, x + 0.5, y + 0.5 );
				const double length = WallDistance( camera, ray );
				const double t0 = Dot( Sub( lightAt, eye ), ray );
				const D3 closest = Add( eye, Mul( ray, std::clamp( t0, 0.0, length ) ) );
				const double miss =
				    std::sqrt( Dot( Sub( lightAt, closest ), Sub( lightAt, closest ) ) );
				double worst = 0.0;
				for ( int c = 0; c < 3; ++c )
				{
					const double want = PointScatter(
					    eye, ray, length, lightAt, color[c], extinction, albedo, g, 2.0 );
					const double got = image.At( x, y )[c];
					exact[( std::size_t( y ) * kWidth + x ) * 4 + c] = float( want );
					sumLab += got;
					sumRef += want;
					worst = std::max( worst, std::fabs( got - want ) / want );
				}
				if ( miss < 50.0 )
					++excluded;
				else
					errors.push_back( worst );
			}
		}
		const std::string name = Text( "scatter.point.g%+.1f", g );
		SaveImage( image.rgba, name + ".gpu" );
		SaveImage( exact, name + ".exact" );
		std::sort( errors.begin(), errors.end() );
		double mean = 0.0;
		for ( double e : errors )
			mean += e;
		mean /= double( std::max<std::size_t>( errors.size(), 1 ) );
		const double p95 = errors.empty() ? 1.0 : errors[errors.size() * 95 / 100];
		std::printf( "INFO %s: %zu judged pixels (%u near the light excluded), mean relative "
		             "error %.4f, p95 %.4f, frame mean %.5f against %.5f\n",
		    name.c_str(), errors.size(), excluded, mean, p95, sumLab, sumRef );
		results.That( errors.size() > 1000 && mean <= 0.03, name + ".mean",
		    Text( "mean relative error %.4f over %.0f pixels (at most 0.03)", mean,
		        double( errors.size() ) ) );
		results.That(
		    p95 <= 0.08, name + ".p95", Text( "95th percentile %.4f (at most 0.08)", p95 ) );
		results.That( std::fabs( sumLab / sumRef - 1.0 ) <= 0.02, name + ".frame",
		    Text( "frame mean ratio %.4f (within 0.02 of 1)", sumLab / sumRef ) );
	}

	// Neutral: density zero and an empty medium are the frame without the
	// pass, bitwise, with the surface program's legacy range fog on.
	{
		Scene scene;
		scene.legacyFog = true;
		scene.wall[0] = 0.8f;
		scene.wall[1] = 0.5f;
		scene.wall[2] = 0.2f;
		CanvasImage without;
		scene.pass = false;
		if ( std::optional<std::string> why = Render( lab, scene, renderer.get(), without ) )
			return why;
		scene.pass = true;
		CanvasImage empty;
		if ( std::optional<std::string> why = Render( lab, scene, renderer.get(), empty ) )
			return why;
		scene.medium.volumes = { Everywhere( 0.004f, 0.6f, 0.5f ) };
		scene.medium.fog.density = 0.001f;
		scene.medium.fog.heightDensity = 0.002f;
		vol::MediumLight light;
		light.position = { 250.0f, 90.0f, 60.0f };
		light.color = { 2.0f, 1.6f, 1.2f };
		scene.lights = { light };
		scene.medium.densityScale = 0.0f;
		CanvasImage zero;
		if ( std::optional<std::string> why = Render( lab, scene, renderer.get(), zero ) )
			return why;
		scene.medium.densityScale = 1.0f;
		CanvasImage fogged;
		if ( std::optional<std::string> why = Render( lab, scene, renderer.get(), fogged ) )
			return why;
		const float *center = without.At( kWidth / 2, kHeight / 2 );
		results.That( center[0] < 0.79f && center[0] > 0.3f, "neutral.legacy-fog-drawn",
		    Text( "the wall's red with legacy range fog %.4f (0.8 without)", center[0] ) );
		results.That( SameImage( without, empty ), "neutral.empty-medium-bitwise",
		    "an empty medium changes the frame" );
		results.That( SameImage( without, zero ), "neutral.density-zero-bitwise",
		    "density zero changes the frame" );
		results.That( !SameImage( without, fogged ), "neutral.control-fog-changes-frame",
		    "the same medium at density one leaves the frame unchanged" );
	}

	// History: a frame after a camera cut is the frame a new renderer draws.
	{
		Scene first;
		first.medium.volumes = { Everywhere( 0.003f, 0.6f, 0.4f ) };
		vol::MediumLight light;
		light.position = { 250.0f, 90.0f, 60.0f };
		light.color = { 2.0f, 1.6f, 1.2f };
		first.lights = { light };
		Scene second = first;
		second.camera.eye = { 60.0f, -120.0f, 30.0f };
		second.camera.forward = math::Normalize( float3{ 1.0f, 0.35f, -0.05f } );
		CanvasImage before, after, fresh;
		if ( std::optional<std::string> why = Render( lab, first, renderer.get(), before ) )
			return why;
		if ( std::optional<std::string> why = Render( lab, second, renderer.get(), after ) )
			return why;
		auto another = vol::VolumetricRenderer::Create( lab.device, kCanvasColor, lab.programs );
		if ( !another )
			return std::string( "a second volumetric pass was refused" );
		if ( std::optional<std::string> why = Render( lab, second, another.Value().get(), fresh ) )
			return why;
		results.That( SameImage( after, fresh ), "history.none-after-camera-cut",
		    "the frame after the cut differs from a new renderer's" );
		results.That( !SameImage( before, after ), "history.control-views-differ",
		    "the two cameras drew the same frame" );
	}
	return std::nullopt;
}

// The seeded stage a sensitivity run replaces, by its module.
struct SeededStage
{
	const std::uint32_t *words;
	int stage; // 0 inject, 1 composite
};
const SeededStage kSeededStages[] = { { spirv::kVolumetricPhaseIgnored, 0 },
    { spirv::kVolumetricAlbedoIgnored, 0 }, { spirv::kVolumetricExtinctionTwice, 1 },
    { spirv::kVolumetricSliceOffByOne, 1 } };

std::optional<std::string> RunOnce( bool validate, std::span<const std::uint32_t> module,
    Results &results, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( std::optional<std::string> why = CreateLabDevice( validate, counter, device ) )
		return why;
	{
		Lab lab( *device );
		for ( const SeededStage &seeded : kSeededStages )
		{
			if ( module.empty() || module.data() != seeded.words )
				continue;
			( seeded.stage == 0 ? lab.programs.inject : lab.programs.composite ) = module;
		}
		if ( std::optional<std::string> why = Prepare( lab ) )
			return why;
		if ( std::optional<std::string> why = VolumetricChecks( lab, results ) )
			return why;
		(void)device->WaitIdle();
	}
	device.reset();
	messages = counter.load();
	return std::nullopt;
}

const Seeded kVolumetricSeeded[] = {
    { "phase-ignored", spirv::kVolumetricPhaseIgnored, "scatter.point" },
    { "albedo-ignored", spirv::kVolumetricAlbedoIgnored, "scatter.point" },
    { "extinction-twice", spirv::kVolumetricExtinctionTwice, "transmittance." },
    { "slice-off-by-one", spirv::kVolumetricSliceOffByOne, "transmittance." } };

} // namespace

int RunVolumetricSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "volumetric", kVolumetricSeeded, RunOnce );
}

} // namespace render::lab
