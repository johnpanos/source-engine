//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab suite map-terms (RFC 0016 K11 "Model assembly", end to
//			end): the surface program's pbr point reading the map's indirect
//			terms, drawn through the Vulkan adapter and judged against oracles
//			that share no code with the shaders:
//			- the lightmap basis on a world surface (kSurfaceBakedLightmap,
//			  with kSurfaceDirectionalLightmap on a directional page): the
//			  baked view (RFC 0014 view 8) is the flat page times the frame's
//			  lightmap scale, a directional page's E0 * clamp( 1 + beta .
//			  ( n - N ), 0, 4 ) at the normal map's normal, the flat light
//			  bitwise without one; the lit pixel is pbr_brdf.h's diffuse color
//			  times it (0.5 percent); the baked term off removes it;
//			- the reflection probes on a world floor (kSurfaceReflectionProbes)
//			  in the RPRB fixture's room (quality/fixtures/reflection/rprb/
//			  valid.rprb): a white metal's image-specular view (view 10) is
//			  mapcontainer::ReflectionProbesView::Radiance at each pixel's
//			  floor point and reflected ray, the directional albedo being one,
//			  within 2 percent + 2e-3 (the probe texture is filtered by the
//			  sampler); without the term, and with a texture carrying no
//			  probes, the frame is the ambient cube's (zero here), bitwise;
//			- the probe volume on a model surface (kSurfaceProbeVolume) over
//			  the PRBV gpu fixture (quality/fixtures/gi/prbv/gpu.prbv): the
//			  diffuse lobe (cl_render_debug_brdf 1) is pbr_brdf.h's diffuse
//			  color times mapcontainer::ProbeVolumeView::Sample with
//			  visibility where a grid covers the point, within 1 percent +
//			  2e-3, and the ambient cube (zero here) where none does; without
//			  the term the frame is zero.
//			A pixel whose oracle moves by more than half its bound within a
//			hundredth of a unit (the fixtures are in Source and clip-space
//			units) is ill-conditioned and skipped, at most 5 percent of a
//			view. Tolerances were fixed before the first run.
//
//=============================================================================//

#include "lab_canvas.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "mapcontainer/probe_volume.h"
#include "mapcontainer/reflection_probes.h"
#include "render/material/surface_program.h"
#include "render/math/matrix.h"
#include "render/pbr_brdf.h"
#include "render/shaderlib/debug_view.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace render::lab
{

namespace
{

using namespace render::device;
using math::float3;

constexpr std::uint32_t kSize = 64;
constexpr float kFlat[3] = { 0.4f, 0.6f, 0.8f };
constexpr float kGradient[3] = { 0.5f, -0.3f, 0.2f };
// The normal map's texel (RGBA8): a normal tilted toward +x and -y.
constexpr int kNormalTexel[2] = { 200, 90 };
constexpr float kConditioning = 0.01f;
constexpr float kIllConditionedShare = 0.05f;

std::vector<std::byte> HalfPage( const float rgb[3] )
{
	std::vector<std::byte> page( 4 * 4 * 8 );
	for ( std::size_t i = 0; i < 16; ++i )
	{
		const std::uint16_t half[4] = { FloatToHalf( rgb[0] ), FloatToHalf( rgb[1] ),
		    FloatToHalf( rgb[2] ), FloatToHalf( 1.0f ) };
		std::memcpy( page.data() + i * 8, half, 8 );
	}
	return page;
}

float Rounded( float value )
{
	return HalfToFloat( FloatToHalf( value ) );
}

std::vector<unsigned char> Load( const std::string &path )
{
	std::ifstream file( path, std::ios::binary );
	return std::vector<unsigned char>(
	    std::istreambuf_iterator<char>{ file }, std::istreambuf_iterator<char>{} );
}

struct View
{
	float3 eye;
	math::float4x4 toClip;
	math::float4x4 fromClip;
};

View MakeView( float3 eye, float3 target, float3 up, float nearPlane )
{
	View view;
	view.eye = eye;
	view.toClip =
	    math::Multiply( math::Perspective( 70.0f * 3.14159265f / 180.0f, 1.0f, nearPlane, 8192.0f ),
	        math::LookAt( eye, target, up ) );
	view.fromClip = *math::Inverse( view.toClip );
	return view;
}

// Where a pixel centre's ray meets the plane z = `height` inside the
// rectangle low..high, if it does.
std::optional<float3> Hit( const View &view, std::uint32_t px, std::uint32_t py, float height,
    const float low[2], const float high[2] )
{
	const float nx = ( float( px ) + 0.5f ) / float( kSize ) * 2.0f - 1.0f;
	const float ny = 1.0f - ( float( py ) + 0.5f ) / float( kSize ) * 2.0f;
	const math::float4 a = math::Transform( view.fromClip, { nx, ny, 0.0f, 1.0f } );
	const math::float4 b = math::Transform( view.fromClip, { nx, ny, 1.0f, 1.0f } );
	const float3 nearPoint{ a.x / a.w, a.y / a.w, a.z / a.w - height };
	const float3 farPoint{ b.x / b.w, b.y / b.w, b.z / b.w - height };
	if ( !( ( nearPoint.z > 0.0f ) != ( farPoint.z > 0.0f ) ) )
		return std::nullopt;
	const float t = nearPoint.z / ( nearPoint.z - farPoint.z );
	const float3 p{ nearPoint.x + ( farPoint.x - nearPoint.x ) * t,
	    nearPoint.y + ( farPoint.y - nearPoint.y ) * t, height };
	// Away from the rectangle's edges, where the rasterizer decides coverage.
	const float margin = 0.02f * ( high[0] - low[0] );
	if ( p.x < low[0] + margin || p.x > high[0] - margin || p.y < low[1] + margin ||
	     p.y > high[1] - margin )
		return std::nullopt;
	return p;
}

// One draw of the surface program.
struct Scene
{
	material::SurfaceVariant variant;
	material::SurfaceTextures textures;
	material::SurfaceFrame frame;
	material::SurfaceMapTextures map;
	std::string page;
	std::string gradient;
	std::vector<std::byte> vertices;
	const View *view = nullptr;
	shaderlib::DebugSpecialization debug;
};

struct Lab
{
	IRenderDevice2 &device;
	resources::TextureCache textures;
	material::GroupResidency groups;
	std::unique_ptr<Canvas> canvas;
	std::unique_ptr<material::SurfaceProgram> program;
	std::uint64_t nextGroup = 1;

	explicit Lab( IRenderDevice2 &d ) : device( d ), textures( d ), groups( d, textures ) {}
};

std::optional<std::string> Prepare( Lab &lab )
{
	if ( std::optional<std::string> why = Canvas::Create( lab.device, kSize, kSize, lab.canvas ) )
		return why;
	auto program = material::SurfaceProgram::Create( lab.device, kCanvasColor, kCanvasDepth, 1 );
	if ( !program )
		return std::string( "the surface program was refused" );
	lab.program = std::move( program ).Value();
	TextureDesc page;
	page.format = Format::kRGBA16Float;
	page.width = page.height = 4;
	page.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	bool staged = lab.textures.Stage( "mt/flat", page, HalfPage( kFlat ) ).HasValue() &&
	              lab.textures.Stage( "mt/gradient", page, HalfPage( kGradient ) ).HasValue();
	staged = staged && StageConstant( lab.textures, "mt/base", Format::kRGBA8Srgb,
	                       ByteTexel( 255, 255, 255, 255 ) );
	staged = staged && StageConstant( lab.textures, "mt/dielectric", Format::kRGBA8Unorm,
	                       ByteTexel( 0, 255, 255, 255 ) );
	staged = staged && StageConstant( lab.textures, "mt/metal", Format::kRGBA8Unorm,
	                       ByteTexel( 255, 77, 255, 255 ) );
	staged = staged && StageConstant( lab.textures, "mt/normal", Format::kRGBA8Unorm,
	                       ByteTexel( kNormalTexel[0], kNormalTexel[1], 255, 255 ) );
	const material::PbrSplitSumTable table = material::SplitSumTable();
	TextureDesc tableDesc;
	tableDesc.format = table.format;
	tableDesc.width = table.width;
	tableDesc.height = table.height;
	staged =
	    staged &&
	    lab.textures.Stage( "mt/splitsum", tableDesc, std::as_bytes( std::span( table.texels ) ) )
	        .HasValue();
	if ( !staged )
		return std::string( "a fixture texture was refused" );
	return std::nullopt;
}

std::optional<std::string> Render( Lab &lab, const Scene &scene, CanvasImage &image )
{
	auto request = lab.program->Request( scene.variant, {}, scene.textures );
	if ( !request )
		return std::string( "the pbr point was refused" );
	const bool model = scene.variant.layout == material::SurfaceVertexLayout::kModel;
	const std::uint64_t materialGroup = lab.nextGroup++;
	const std::uint64_t frameGroup = lab.nextGroup++;
	const std::uint64_t drawGroup = lab.nextGroup++;
	const std::uint64_t viewGroup = lab.nextGroup++; // no clustered lights
	material::SurfaceFrame frame = scene.frame;
	frame.eye[0] = scene.view->eye.x;
	frame.eye[1] = scene.view->eye.y;
	frame.eye[2] = scene.view->eye.z;
	if ( !lab.groups.Set( materialGroup, request.Value().material ) ||
	     !lab.groups.Set( viewGroup, lab.program->NeutralViewGroup() ) ||
	     !lab.groups.Set(
	         frameGroup, lab.program->FrameGroup( frame, "mt/splitsum", {}, scene.map ) ) ||
	     !lab.groups.Set(
	         drawGroup, model ? lab.program->DrawGroup( {}, {} )
	                          : lab.program->DrawGroup( scene.page, {}, {}, scene.gradient ) ) )
		return std::string( "a group was refused" );
	auto pipeline = lab.program->DebugPipeline( request.Value().pipeline, scene.debug );
	if ( !pipeline )
		return std::string( "no pipeline" );
	// Make the groups resident.
	if ( std::optional<std::string> why =
	         lab.canvas->Render( lab.textures, lab.groups, {}, { 0, 0, 0, 1 }, nullptr ) )
		return why;
	auto group = [&]( std::uint64_t id ) -> BindGroupId
	{
		const material::ResidentGroup *resident = lab.groups.Group( id );
		return resident ? resident->group : BindGroupId();
	};
	CanvasDraw draw;
	draw.pipeline = pipeline.Value();
	draw.groups[std::size_t( BindGroupRole::kFrame )] = group( frameGroup );
	draw.groups[std::size_t( BindGroupRole::kView )] = group( viewGroup );
	draw.groups[std::size_t( BindGroupRole::kMaterial )] = group( materialGroup );
	draw.groups[std::size_t( BindGroupRole::kDraw )] = group( drawGroup );
	draw.vertices = lab.canvas->Vertices( scene.vertices );
	draw.vertexCount = 6;
	material::FamilyDrawConstants constants;
	std::memcpy( constants.toClip, &scene.view->toClip, sizeof( constants.toClip ) );
	constants.world[0] = constants.world[5] = constants.world[10] = constants.world[15] = 1.0f;
	const auto *bytes = reinterpret_cast<const std::byte *>( &constants );
	draw.constants.assign( bytes, bytes + request.Value().drawConstantBytes );
	const CanvasDraw draws[] = { draw };
	std::optional<std::string> why =
	    lab.canvas->Render( lab.textures, lab.groups, draws, { 0, 0, 0, 1 }, &image );
	for ( std::uint64_t id : { materialGroup, frameGroup, drawGroup, viewGroup } )
		lab.groups.Remove( id );
	return why;
}

// A quad of six world vertices on z = `height` over low..high, facing +z.
std::vector<std::byte> WorldQuad( float height, const float low[2], const float high[2] )
{
	std::vector<std::byte> out;
	const float corners[4][2] = {
	    { low[0], low[1] }, { high[0], low[1] }, { high[0], high[1] }, { low[0], high[1] } };
	for ( int corner : { 0, 1, 2, 0, 2, 3 } )
	{
		material::SurfaceWorldVertex vertex;
		vertex.position[0] = corners[corner][0];
		vertex.position[1] = corners[corner][1];
		vertex.position[2] = height;
		vertex.uv[0] = vertex.lightmapUv[0] = corners[corner][0] > low[0] ? 1.0f : 0.0f;
		vertex.uv[1] = vertex.lightmapUv[1] = corners[corner][1] > low[1] ? 1.0f : 0.0f;
		const auto *bytes = reinterpret_cast<const std::byte *>( &vertex );
		out.insert( out.end(), bytes, bytes + sizeof( vertex ) );
	}
	return out;
}

std::vector<std::byte> ModelQuad( float height, const float low[2], const float high[2] )
{
	std::vector<std::byte> out;
	const float corners[4][2] = {
	    { low[0], low[1] }, { high[0], low[1] }, { high[0], high[1] }, { low[0], high[1] } };
	for ( int corner : { 0, 1, 2, 0, 2, 3 } )
	{
		material::SurfaceModelVertex vertex;
		vertex.position[0] = corners[corner][0];
		vertex.position[1] = corners[corner][1];
		vertex.position[2] = height;
		vertex.normal[2] = 1.0f;
		vertex.tangent[0] = 1.0f;
		vertex.tangent[3] = 1.0f;
		const auto *bytes = reinterpret_cast<const std::byte *>( &vertex );
		out.insert( out.end(), bytes, bytes + sizeof( vertex ) );
	}
	return out;
}

bool Within( const float *actual, const float expected[3], float relative, float absolute )
{
	for ( int k = 0; k < 3; ++k )
	{
		if ( !( std::fabs( actual[k] - expected[k] ) <=
		         relative * std::fabs( expected[k] ) + absolute ) )
			return false;
	}
	return true;
}

std::string Describe( std::uint32_t x, std::uint32_t y, const float expected[3], const float *got )
{
	char text[192];
	std::snprintf( text, sizeof( text ),
	    "pixel %u,%u: expected (%.5g %.5g %.5g), got (%.5g %.5g %.5g)", x, y, expected[0],
	    expected[1], expected[2], got[0], got[1], got[2] );
	return text;
}

// Every covered pixel of `image` against `oracle`, skipping ill-conditioned
// ones.
struct Comparison
{
	std::size_t judged = 0;
	std::size_t skipped = 0;
	std::size_t failures = 0;
	std::string first;
};

using Oracle = std::function<void( const float3 &, float out[3] )>;

Comparison Compare( const CanvasImage &image, const View &view, float height, const float low[2],
    const float high[2], float relative, float absolute, const Oracle &oracle )
{
	Comparison c;
	for ( std::uint32_t y = 0; y < kSize; ++y )
	{
		for ( std::uint32_t x = 0; x < kSize; ++x )
		{
			const std::optional<float3> hit = Hit( view, x, y, height, low, high );
			if ( !hit )
				continue;
			float expected[3];
			oracle( *hit, expected );
			bool conditioned = true;
			for ( int axis = 0; axis < 2 && conditioned; ++axis )
			{
				for ( float step : { -kConditioning, kConditioning } )
				{
					float3 moved = *hit;
					( axis == 0 ? moved.x : moved.y ) += step;
					float other[3];
					oracle( moved, other );
					conditioned =
					    conditioned && Within( other, expected, 0.5f * relative, 0.5f * absolute );
				}
			}
			if ( !conditioned )
			{
				++c.skipped;
				continue;
			}
			++c.judged;
			if ( !Within( image.At( x, y ), expected, relative, absolute ) && c.failures++ == 0 )
				c.first = Describe( x, y, expected, image.At( x, y ) );
		}
	}
	return c;
}

// The pixels match `oracle`, and few enough are ill-conditioned.
void JudgePixels( Results &results, const std::string &name, const CanvasImage &image,
    const View &view, float height, const float low[2], const float high[2], float relative,
    float absolute, const Oracle &oracle )
{
	const Comparison c = Compare( image, view, height, low, high, relative, absolute, oracle );
	results.That( c.judged > kSize * kSize / 8 && c.failures == 0, name,
	    std::to_string( c.failures ) + " of " + std::to_string( c.judged ) + " pixels differ" +
	        ( c.first.empty() ? "" : "; first " + c.first ) );
	results.That( float( c.skipped ) <= kIllConditionedShare * float( c.skipped + c.judged ),
	    name + ".conditioned",
	    std::to_string( c.skipped ) + " of " + std::to_string( c.skipped + c.judged ) +
	        " pixels ill-conditioned" );
}

// The negative control: a tenth of the pixels at least do not match the
// wrong `oracle`, so the comparison can fail.
void Rejects( Results &results, const std::string &name, const CanvasImage &image, const View &view,
    float height, const float low[2], const float high[2], float relative, float absolute,
    const Oracle &oracle )
{
	const Comparison c = Compare( image, view, height, low, high, relative, absolute, oracle );
	results.That( c.judged > 0 && c.failures * 10 >= c.judged, name,
	    std::to_string( c.failures ) + " of " + std::to_string( c.judged ) + " pixels differ" );
}

bool AllZero( const CanvasImage &image )
{
	for ( std::size_t i = 0; i < image.rgba.size(); i += 4 )
	{
		if ( image.rgba[i] != 0.0f || image.rgba[i + 1] != 0.0f || image.rgba[i + 2] != 0.0f )
			return false;
	}
	return true;
}

std::optional<std::string> LightmapChecks( Lab &lab, Results &results )
{
	const float low[2] = { -50, -50 }, high[2] = { 50, 50 };
	const View view = MakeView( { 0, 0, 100 }, { 0, 0, 0 }, { 0, 1, 0 }, 1.0f );
	shaderlib::DebugSpecialization baked;
	baked.view = std::uint32_t( shaderlib::DebugView::kBakedLight );
	shaderlib::DebugSpecialization termOff;
	termOff.termsOff = shaderlib::kDebugTermBaked;
	auto scene = [&]( bool normalMap, bool directional, shaderlib::DebugSpecialization debug )
	{
		Scene s;
		s.variant.layout = material::SurfaceVertexLayout::kWorld;
		s.variant.terms = material::kSurfacePbr | material::kSurfaceBakedLightmap |
		                  ( directional ? material::kSurfaceDirectionalLightmap : 0u ) |
		                  ( normalMap ? material::kSurfaceBump : 0u );
		s.textures.base = "mt/base";
		s.textures.mrao = "mt/dielectric";
		s.textures.bump = normalMap ? "mt/normal" : "";
		s.page = "mt/flat";
		s.gradient = "mt/gradient";
		s.vertices = WorldQuad( 0.0f, low, high );
		s.view = &view;
		s.debug = debug;
		return s;
	};
	CanvasImage flatImage, directionalImage, smoothImage, mappedImage, litImage, offImage;
	for ( auto [s, image] : { std::pair{ scene( false, false, baked ), &flatImage },
	          std::pair{ scene( true, true, baked ), &directionalImage },
	          std::pair{ scene( false, true, baked ), &smoothImage },
	          std::pair{ scene( true, false, baked ), &mappedImage },
	          std::pair{ scene( false, false, {} ), &litImage },
	          std::pair{ scene( false, false, termOff ), &offImage } } )
	{
		if ( std::optional<std::string> why = Render( lab, s, *image ) )
			return why;
	}
	const float scale = material::kLightmapScaleLinear;
	float flat[3], gradient[3], flatLight[3];
	for ( int k = 0; k < 3; ++k )
	{
		flat[k] = Rounded( kFlat[k] );
		gradient[k] = Rounded( kGradient[k] );
		flatLight[k] = flat[k] * scale;
	}
	// The mapped normal: the texel decoded, in the frame S = +x, T = +y, N = +z.
	const double nx = kNormalTexel[0] / 255.0 * 2.0 - 1.0;
	const double ny = kNormalTexel[1] / 255.0 * 2.0 - 1.0;
	const double nz = std::sqrt( std::max( 0.0, 1.0 - nx * nx - ny * ny ) );
	const double length = std::sqrt( nx * nx + ny * ny + nz * nz );
	const double gain = std::clamp(
	    1.0 + ( gradient[0] * nx + gradient[1] * ny + gradient[2] * ( nz - length ) ) / length, 0.0,
	    4.0 );
	const float directional[3] = {
	    float( flatLight[0] * gain ), float( flatLight[1] * gain ), float( flatLight[2] * gain ) };
	const auto constant = [&]( const float value[3] )
	{
		return [value]( const float3 &, float out[3] )
		{
			std::copy( value, value + 3, out );
		};
	};
	JudgePixels( results, "lightmap.flat-baked", flatImage, view, 0.0f, low, high, 5e-3f, 2e-4f,
	    constant( flatLight ) );
	JudgePixels( results, "lightmap.flat-ignores-normal-map", mappedImage, view, 0.0f, low, high,
	    5e-3f, 2e-4f, constant( flatLight ) );
	JudgePixels( results, "lightmap.directional-baked", directionalImage, view, 0.0f, low, high,
	    5e-3f, 2e-4f, constant( directional ) );
	results.That(
	    smoothImage.rgba == flatImage.rgba, "lightmap.directional-smooth-is-flat-bitwise" );
	JudgePixels( results, "lightmap.lit-is-diffuse-times-baked", litImage, view, 0.0f, low, high,
	    5e-3f, 2e-4f,
	    [&]( const float3 &p, float out[3] )
	    {
		    const float3 toEye{ view.eye.x - p.x, view.eye.y - p.y, view.eye.z - p.z };
		    const float normalDotView =
		        toEye.z / std::sqrt( toEye.x * toEye.x + toEye.y * toEye.y + toEye.z * toEye.z );
		    const float diffuse = 1.0f - pbr::SpecularDirectionalAlbedo(
		                                     0.04f, pbr::SampleSplitSum( normalDotView, 1.0f ) );
		    for ( int k = 0; k < 3; ++k )
			    out[k] = flatLight[k] * diffuse;
	    } );
	results.That( AllZero( offImage ), "lightmap.term-off-removes-baked" );
	return std::nullopt;
}

std::optional<std::string> ReflectionProbeChecks( Lab &lab, Results &results )
{
	const std::vector<unsigned char> bytes = Load( "quality/fixtures/reflection/rprb/valid.rprb" );
	mapcontainer::ReflectionProbesLayout layout{};
	if ( std::optional<std::string> why =
	         StageReflectionProbes( lab.textures, "mt/probes", std::as_bytes( std::span( bytes ) ),
	             mapcontainer::ReflectionProbeMode::Blend, true, layout ) )
		return why;
	const mapcontainer::ReflectionProbesView probes( bytes.data(), layout );
	// The fixture room's floor (6 x 4 m), seen from above its middle.
	const float low[2] = { 0.0f, 0.0f }, high[2] = { 236.0f, 157.0f };
	const View view = MakeView( { 118, 78, 100 }, { 150, 78, 0 }, { 0, 0, 1 }, 1.0f );
	shaderlib::DebugSpecialization specular;
	specular.view = std::uint32_t( shaderlib::DebugView::kImageSpecular );
	auto scene = [&]( bool term, std::string texture )
	{
		Scene s;
		s.variant.layout = material::SurfaceVertexLayout::kWorld;
		s.variant.terms =
		    material::kSurfacePbr | ( term ? material::kSurfaceReflectionProbes : 0u );
		s.textures.base = "mt/base";
		s.textures.mrao = "mt/metal";
		s.map.reflectionProbes = std::move( texture );
		s.vertices = WorldQuad( 0.0f, low, high );
		s.view = &view;
		s.debug = specular;
		return s;
	};
	CanvasImage lit, off, unmarked;
	if ( std::optional<std::string> why = Render( lab, scene( true, "mt/probes" ), lit ) )
		return why;
	if ( std::optional<std::string> why = Render( lab, scene( false, "mt/probes" ), off ) )
		return why;
	if ( std::optional<std::string> why = Render( lab, scene( true, "" ), unmarked ) )
		return why;
	const float roughness = std::max( 77.0f / 255.0f, 0.02f );
	JudgePixels( results, "probes.image-specular", lit, view, 0.0f, low, high, 0.02f, 2e-3f,
	    [&]( const float3 &p, float out[3] )
	    {
		    const float position[3] = { p.x, p.y, p.z };
		    const float normal[3] = { 0, 0, 1 };
		    float toEye[3] = { view.eye.x - p.x, view.eye.y - p.y, view.eye.z - p.z };
		    const float length =
		        std::sqrt( toEye[0] * toEye[0] + toEye[1] * toEye[1] + toEye[2] * toEye[2] );
		    for ( float &v : toEye )
			    v /= length;
		    // reflect( -view, n ) with n = +z.
		    const float reflected[3] = { -toEye[0], -toEye[1], toEye[2] };
		    probes.Radiance( position, normal, reflected, roughness,
		        mapcontainer::ReflectionProbeMode::Blend, out );
	    } );
	Rejects( results, "probes.rejects-direction-only", lit, view, 0.0f, low, high, 0.02f, 2e-3f,
	    [&]( const float3 &p, float out[3] )
	    {
		    const float position[3] = { p.x, p.y, p.z };
		    const float normal[3] = { 0, 0, 1 };
		    float toEye[3] = { view.eye.x - p.x, view.eye.y - p.y, view.eye.z - p.z };
		    const float length =
		        std::sqrt( toEye[0] * toEye[0] + toEye[1] * toEye[1] + toEye[2] * toEye[2] );
		    for ( float &v : toEye )
			    v /= length;
		    const float reflected[3] = { -toEye[0], -toEye[1], toEye[2] };
		    probes.Radiance( position, normal, reflected, roughness,
		        mapcontainer::ReflectionProbeMode::DirectionOnly, out );
	    } );
	results.That( AllZero( off ), "probes.term-off-is-the-ambient-cube" );
	results.That( unmarked.rgba == off.rgba, "probes.no-marker-is-term-off-bitwise" );
	return std::nullopt;
}

std::optional<std::string> ProbeVolumeChecks( Lab &lab, Results &results )
{
	const std::vector<unsigned char> bytes = Load( "quality/fixtures/gi/prbv/gpu.prbv" );
	mapcontainer::ProbeVolumeLayout layout{};
	if ( std::optional<std::string> why = StageProbeVolume(
	         lab.textures, "mt/volume", std::as_bytes( std::span( bytes ) ), layout ) )
		return why;
	const mapcontainer::ProbeVolumeView volume( bytes.data(), layout );
	// The fixture's grid covers x <= 0.2 of the quad z = 0.5 (clip-space units).
	const float low[2] = { -1.0f, -1.0f }, high[2] = { 1.0f, 1.0f };
	const View view = MakeView( { 0, 0, 3.4f }, { 0, 0, 0.5f }, { 0, 1, 0 }, 0.05f );
	shaderlib::DebugSpecialization diffuseOnly;
	diffuseOnly.brdf = std::uint32_t( shaderlib::DebugBrdf::kDiffuseOnly );
	auto scene = [&]( bool term )
	{
		Scene s;
		s.variant.layout = material::SurfaceVertexLayout::kModel;
		s.variant.terms = material::kSurfacePbr | ( term ? material::kSurfaceProbeVolume : 0u );
		s.textures.base = "mt/base";
		s.textures.mrao = "mt/dielectric";
		s.map.probeAtlas = "mt/volume-atlas";
		s.map.probeGrids = "mt/volume-grids";
		s.vertices = ModelQuad( 0.5f, low, high );
		s.view = &view;
		s.debug = diffuseOnly;
		return s;
	};
	CanvasImage lit, off;
	if ( std::optional<std::string> why = Render( lab, scene( true ), lit ) )
		return why;
	if ( std::optional<std::string> why = Render( lab, scene( false ), off ) )
		return why;
	std::size_t covered = 0;
	JudgePixels( results, "volume.diffuse", lit, view, 0.5f, low, high, 0.01f, 2e-3f,
	    [&]( const float3 &p, float out[3] )
	    {
		    const float position[3] = { p.x, p.y, p.z };
		    const float normal[3] = { 0, 0, 1 };
		    float irradiance[3] = { 0, 0, 0 };
		    if ( volume.Sample(
		             position, normal, mapcontainer::ProbeVolumeLayer::Total, true, irradiance ) )
			    ++covered;
		    const float3 toEye{ view.eye.x - p.x, view.eye.y - p.y, view.eye.z - p.z };
		    const float normalDotView =
		        toEye.z / std::sqrt( toEye.x * toEye.x + toEye.y * toEye.y + toEye.z * toEye.z );
		    const float diffuse = 1.0f - pbr::SpecularDirectionalAlbedo(
		                                     0.04f, pbr::SampleSplitSum( normalDotView, 1.0f ) );
		    for ( int k = 0; k < 3; ++k )
			    out[k] = irradiance[k] * diffuse;
	    } );
	results.That( covered > 0, "volume.grid-covers-part", std::to_string( covered ) + " samples" );
	Rejects( results, "volume.rejects-ambient-cube-alone", lit, view, 0.5f, low, high, 0.01f, 2e-3f,
	    []( const float3 &, float out[3] )
	    {
		    out[0] = out[1] = out[2] = 0.0f;
	    } );
	results.That( AllZero( off ), "volume.term-off-is-the-ambient-cube" );
	return std::nullopt;
}

std::optional<std::string> RunOnce(
    bool validate, std::span<const std::uint32_t>, Results &results, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( std::optional<std::string> why = CreateLabDevice( validate, counter, device ) )
		return why;
	{
		Lab lab( *device );
		if ( std::optional<std::string> why = Prepare( lab ) )
			return why;
		for ( auto checks : { LightmapChecks, ReflectionProbeChecks, ProbeVolumeChecks } )
		{
			if ( std::optional<std::string> why = checks( lab, results ) )
				return why;
		}
		(void)device->WaitIdle();
	}
	device.reset();
	messages = counter.load();
	return std::nullopt;
}

} // namespace

int RunMapTermsSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "map-terms", std::span<const Seeded>(), RunOnce );
}

} // namespace render::lab
