//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.debug-views (RFC 0014 D0) in render_lab: every installed
//			view of the catalog (views 1-17) drawn by the core's programs on
//			analytic fixtures and compared with its formula within one 8-bit
//			step; the not-applicable hatch; the program filter; the
//			controls' validation and specialization; default identity.
//
//			Fixtures: 18-unit quads 100 units in front of the camera, one per
//			8x8 cell of a 256x256 linear target, each drawn by one program
//			with one debug specialization. Their inputs are known: constant
//			textures (a base of sRGB (188, 125, 64, 128), a bump texel of
//			(160, 96, 224)), a lightmap page of (0.25, 0.5, 0.75) and pages
//			holding NaN, infinity, a negative value and 4.0, a cube of
//			(0.8, 0.4, 0.2), vertex colors of (64, 128, 192), a tilted vertex
//			normal, and known PBR, vertex-lit and lines inputs. The direct
//			light and image-specular views of the pbr program are judged
//			relationally: each equals the frame minus the frame without that
//			term.
//
//			--seeded <defect> draws the lightmapped program built with one
//			seeded defect (debug_view.glsl's SEEDED_DEBUG_*); --sensitivity
//			runs the control and each defect, and passes when the control
//			passes and each defect fails its own checks.
//
//=============================================================================//

#include "lab_canvas.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "render/frame/debug_controls.h"
#include "render/frame/debug_specialization.h"
#include "render/graph/compiled_graph.h"
#include "render/graph/executor.h"
#include "render/graph/graph_builder.h"
#include "render/material/lightmapped_family.h"
#include "render/material/pbr_family.h"
#include "render/material/program_resolver.h"
#include "render/material/vertexlit_family.h"
#include "render/material/vmt_import.h"
#include "render/math/matrix.h"
#include "render/pass/lines/lines.h"
#include "render/pbr_brdf.h"
#include "render/shaderlib/debug_view.h"
#include "spv/debug_view_defects_spv.h"
#include "testing/conformance_result.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <functional>
#include <map>
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

constexpr std::uint32_t kSize = 256;
constexpr float kDepth = 100.0f;  // the quads' distance along the view axis
constexpr float kCell = 25.0f;    // an 8x8 grid over the 200-unit field at kDepth
constexpr float kHalfQuad = 9.0f; // each quad is 18 units square
constexpr float kTolerance = 1.0f / 255.0f;
constexpr float kLightmapScale = 2.0f;

// The programs the lab composes, for the view filter.
constexpr std::string_view kLabPrograms[] = {
    "lightmapped", "unlit", "preview", "pbr", "vertexlit", "lines" };

struct Rgb
{
	float r = 0, g = 0, b = 0;
};

float SrgbToLinear( float c )
{
	return c <= 0.04045f ? c / 12.92f : std::pow( ( c + 0.055f ) / 1.055f, 2.4f );
}

Rgb Decode( int r, int g, int b )
{
	return { SrgbToLinear( r / 255.0f ), SrgbToLinear( g / 255.0f ), SrgbToLinear( b / 255.0f ) };
}

float Luminance( Rgb c )
{
	return 0.2126f * c.r + 0.7152f * c.g + 0.0722f * c.b;
}

Rgb Hatch( std::uint32_t x, std::uint32_t y )
{
	const float v = ( ( x + y ) & 7u ) < 4u ? 0.25f : 0.5f;
	return { v, v, v };
}

// Where a cell's quad lands on screen, and the ray through a pixel.
class Camera
{
public:
	Camera()
	{
		const float3 eye{ 0, 0, 0 };
		const float fov = 90.0f * 3.14159265358979f / 180.0f;
		m_ToClip = math::Multiply( math::Perspective( fov, 1.0f, 1.0f, 4096.0f ),
		    math::LookAt( eye, float3{ 1, 0, 0 }, float3{ 0, 0, 1 } ) );
		m_FromClip = *math::Inverse( m_ToClip );
	}
	const math::float4x4 &ToClip() const { return m_ToClip; }

	// The quad's centre in world space.
	static float3 Centre( int cell )
	{
		const int column = cell % 8;
		const int row = cell / 8;
		return { kDepth, 100.0f - kCell * 0.5f - kCell * float( column ),
		    100.0f - kCell * 0.5f - kCell * float( row ) };
	}

	// A world point's pixel (x right, y down).
	void Pixel( float3 p, float &x, float &y ) const
	{
		const math::float4 clip = math::Transform( m_ToClip, { p.x, p.y, p.z, 1.0f } );
		x = ( clip.x / clip.w * 0.5f + 0.5f ) * float( kSize );
		y = ( 0.5f - clip.y / clip.w * 0.5f ) * float( kSize );
	}

	// Where a pixel centre's ray meets the plane x = kDepth.
	float3 Hit( std::uint32_t px, std::uint32_t py ) const
	{
		const float nx = ( float( px ) + 0.5f ) / float( kSize ) * 2.0f - 1.0f;
		const float ny = 1.0f - ( float( py ) + 0.5f ) / float( kSize ) * 2.0f;
		const math::float4 a = math::Transform( m_FromClip, { nx, ny, 0.0f, 1.0f } );
		const math::float4 b = math::Transform( m_FromClip, { nx, ny, 1.0f, 1.0f } );
		const float3 near{ a.x / a.w, a.y / a.w, a.z / a.w };
		const float3 far{ b.x / b.w, b.y / b.w, b.z / b.w };
		const float t = ( kDepth - near.x ) / ( far.x - near.x );
		return { kDepth, near.y + ( far.y - near.y ) * t, near.z + ( far.z - near.z ) * t };
	}

	// The pixels well inside a cell's quad (4 pixels in from its edges).
	std::vector<std::pair<std::uint32_t, std::uint32_t>> Interior( int cell ) const
	{
		const float3 c = Centre( cell );
		float x0, y0, x1, y1;
		Pixel( { c.x, c.y + kHalfQuad, c.z + kHalfQuad }, x0, y0 );
		Pixel( { c.x, c.y - kHalfQuad, c.z - kHalfQuad }, x1, y1 );
		const int left = int( std::ceil( std::min( x0, x1 ) ) ) + 4;
		const int right = int( std::floor( std::max( x0, x1 ) ) ) - 4;
		const int top = int( std::ceil( std::min( y0, y1 ) ) ) + 4;
		const int bottom = int( std::floor( std::max( y0, y1 ) ) ) - 4;
		std::vector<std::pair<std::uint32_t, std::uint32_t>> pixels;
		for ( int y = top; y <= bottom; y += 2 )
		{
			for ( int x = left; x <= right; x += 2 )
				pixels.emplace_back( std::uint32_t( x ), std::uint32_t( y ) );
		}
		return pixels;
	}

	// The quad's texture coordinates at a pixel: u along -y, v along -z.
	static void Uv( int cell, float3 hit, float &u, float &v )
	{
		const float3 c = Centre( cell );
		u = ( c.y + kHalfQuad - hit.y ) / ( 2.0f * kHalfQuad );
		v = ( c.z + kHalfQuad - hit.z ) / ( 2.0f * kHalfQuad );
	}

private:
	math::float4x4 m_ToClip;
	math::float4x4 m_FromClip;
};

// The corner positions and coordinates of a cell's quad, as two triangles.
struct Corner
{
	float3 position;
	float u = 0, v = 0;
};

std::array<Corner, 6> QuadCorners( int cell )
{
	const float3 c = Camera::Centre( cell );
	auto at = [&]( float u, float v ) -> Corner
	{
		return {
		    { c.x, c.y + kHalfQuad - 2.0f * kHalfQuad * u, c.z + kHalfQuad - 2.0f * kHalfQuad * v },
		    u, v };
	};
	const Corner a = at( 0, 0 ), b = at( 1, 0 ), d = at( 0, 1 ), e = at( 1, 1 );
	return { a, b, e, a, e, d };
}

template <typename T> std::vector<std::byte> Bytes( const T &value )
{
	const auto bytes = std::as_bytes( std::span( &value, 1 ) );
	return { bytes.begin(), bytes.end() };
}

// The analytic scene: textures, programs, groups and one quad per case.
struct Inputs
{
	float3 normal;   // the lightmapped and vertex-lit quads' vertex normal
	float3 tangentS; // and tangents
	float3 tangentT;
	std::uint8_t vertexColor[4] = { 64, 128, 192, 255 };
	Rgb base = Decode( 188, 125, 64 );
	float baseAlpha = 128.0f / 255.0f;
	float bump[3] = { 160.0f / 255.0f, 96.0f / 255.0f, 224.0f / 255.0f };
	Rgb page{ 0.25f, 0.5f, 0.75f };
	Rgb cube{ 0.8f, 0.4f, 0.2f };
	Rgb pbrBase = Decode( 200, 150, 100 );
	float mrao[3] = { 64.0f / 255.0f, 153.0f / 255.0f, 204.0f / 255.0f };
	Rgb vertexLitBase = Decode( 100, 200, 50 );
	float vertexLitColor[3] = { 0.5f, 1.0f, 0.25f };
	std::uint8_t lineColor[4] = { 200, 50, 100, 255 };
};

// A 4x4 texture of one texel value, staged under `name`.

std::vector<std::byte> HalfTexel( float r, float g, float b, float a = 1.0f )
{
	const std::uint16_t half[4] = {
	    FloatToHalf( r ), FloatToHalf( g ), FloatToHalf( b ), FloatToHalf( a ) };
	return Bytes( half );
}

enum class Program
{
	kLightmapped,   // LightmappedGeneric, the base texture
	kBumped,        // + $bumpmap
	kEnvmapped,     // + $envmap with $envmaptint
	kSelfIllum,     // + $selfillum with $selfillumtint
	kSelfIllumZero, // + $selfillum with $selfillumtint 0 (the emission term's neutral value)
	kUnlit,         // UnlitGeneric with $vertexcolor
	kPbr,
	kModernMesh,    // VertexLitGeneric mapped to the PBR mesh point
	kInstancedMesh, // the same material on object-space vertices and a draw transform
	kVertexLit,
	kLines
};

std::string_view ProgramName( Program program )
{
	switch ( program )
	{
	case Program::kUnlit:
		return "unlit";
	case Program::kPbr:
	case Program::kModernMesh:
	case Program::kInstancedMesh:
		return "pbr";
	case Program::kVertexLit:
		return "vertexlit";
	case Program::kLines:
		return "lines";
	default:
		return "lightmapped";
	}
}

// One quad: a program, its lightmap page, the frame's debug controls, and the
// expected output (nullopt: skip the pixel).
struct Case
{
	std::string name;
	Program program = Program::kLightmapped;
	std::string page = "lab:page:normal";
	frame::DebugControls debug;
	std::function<std::optional<Rgb>( std::uint32_t x, std::uint32_t y, float3 hit, int cell )>
	    expect;
	float tolerance = kTolerance;
	// pbr: the material and lighting variants (Prepare's names) and a mesh
	// that replaces the quad (SurfaceModelVertex triangles).
	std::string pbrMaterial = "default";
	std::string pbrLighting = "default";
	std::vector<std::byte> mesh;
};

frame::DebugControls View( std::uint32_t view )
{
	frame::DebugControls debug;
	debug.view = view;
	return debug;
}

struct Lab
{
	IRenderDevice2 &device;
	std::unique_ptr<material::ProgramResolver> resolver;
	std::unique_ptr<material::ProgramResolver> modelResolver;
	std::unique_ptr<material::PbrFamily> pbr;
	std::unique_ptr<material::VertexLitFamily> vertexLit;
	std::unique_ptr<pass::lines::LinesRenderer> lines;
	resources::TextureCache textures;
	material::GroupResidency groups;
	std::unique_ptr<Canvas> canvas;
	Camera camera;
	Inputs inputs;
	std::map<Program, material::ResolvedProgram> programs;
	material::ResolvedProgram modernMesh;
	material::ResolvedProgram instancedMesh;
	std::uint64_t modernMaterial = 0;
	std::uint64_t modernDraw = 0;
	std::uint64_t modernFrame = 0;
	std::uint64_t instancedMaterial = 0;
	std::uint64_t instancedDraw = 0;
	std::uint64_t instancedFrame = 0;
	std::map<std::string, std::uint64_t> drawGroups; // lightmap page -> group id
	std::uint64_t frameGroup = 0;
	std::uint64_t pbrFrameGroup = 0;
	std::map<std::string, std::uint64_t> pbrMaterials; // variant -> material group
	std::map<std::string, std::uint64_t> pbrLightings; // variant -> draw group
	std::map<std::string, material::ModelLighting> pbrLightingValues;
	std::map<std::string, std::array<float, 4>>
	    pbrMrao; // variant -> metal, rough, ao, emission scale
	std::uint64_t vertexLitLighting = 0;
	std::uint64_t vertexLitFrameGroup = 0; // the vertexlit family's program's frame terms
	std::uint64_t nextGroup = 1;
	std::map<Program, std::uint64_t> materialGroups;
	std::map<std::uint64_t, std::uint64_t> viewGroups;      // view layout -> neutral view group
	material::ProgramRequest pbrRequest;                    // the default material's
	std::map<std::string, device::PipelineId> pbrPipelines; // by material variant
	material::ProgramRequest vertexLitRequest;

	explicit Lab( IRenderDevice2 &d ) : device( d ), textures( d ), groups( d, textures ) {}
};

std::optional<std::string> Prepare( Lab &lab, std::span<const std::uint32_t> lightmappedFragment )
{
	Inputs &in = lab.inputs;
	in.normal = math::Normalize( { -0.742f, 0.3f, -0.6f } );
	in.tangentS = math::Normalize( math::Cross( in.normal, { 0, 0, 1 } ) );
	in.tangentT = math::Cross( in.normal, in.tangentS );

	material::ProgramModules modules;
	modules.lightmappedFragment = lightmappedFragment;
	auto resolver = material::ProgramResolver::Create(
	    lab.device, kCanvasColor, kCanvasDepth, 1, material::VertexLayout::kSurface, modules );
	if ( !resolver )
		return "no programs: " + resolver.Error();
	lab.resolver = std::move( resolver ).Value();
	lab.resolver->SetWorldPbr( true, material::kSurfaceClustered );
	// An imported model takes the same PBR mesh point as native PBR materials.
	auto modelMaterial =
	    material::MapVariables( "VertexLitGeneric", { { "$basetexture", "lab/vl/base" } }, {} );
	if ( !modelMaterial || material::ClaimForDrawing( modelMaterial.Value() ) ||
	     !material::ClaimForMesh( modelMaterial.Value() ) )
		return std::string( "the modern model claim is not mesh-only" );
	auto modelProgram = lab.resolver->ResolveMesh( modelMaterial.Value() );
	if ( !modelProgram || modelProgram.Value().name != "pbr" ||
	     modelProgram.Value().request.vertexStride != sizeof( material::SurfaceWorldVertex ) ||
	     !modelProgram.Value().drawInputs.empty() )
		return std::string( "VertexLitGeneric did not resolve to the PBR mesh point" );
	lab.modernMesh = std::move( modelProgram ).Value();
	// The game uploads one object-space mesh per model and varies the draw
	// transform per instance. The same claim must produce that vertex layout.
	auto modelResolver = material::ProgramResolver::Create(
	    lab.device, kCanvasColor, kCanvasDepth, 1, material::VertexLayout::kModel );
	if ( !modelResolver )
		return std::string( "the object-space model resolver was refused" );
	modelResolver.Value()->SetWorldPbr( true, material::kSurfaceClustered );
	auto instanced = modelResolver.Value()->ResolveMesh( modelMaterial.Value() );
	if ( !instanced || !instanced.Value().request.pipeline.IsValid() ||
	     instanced.Value().request.vertexStride != sizeof( material::SurfaceModelVertex ) ||
	     instanced.Value().request.drawConstantBytes != sizeof( material::FamilyDrawConstants ) )
		return std::string( "the object-space mesh point is not drawable" );
	lab.modelResolver = std::move( modelResolver ).Value();
	lab.instancedMesh = std::move( instanced ).Value();
	material::SurfaceConstants modelConstants;
	if ( lab.modernMesh.request.material.constants.size() != sizeof( modelConstants ) )
		return std::string( "the modern model constants have the wrong size" );
	std::memcpy( &modelConstants, lab.modernMesh.request.material.constants.data(),
	    sizeof( modelConstants ) );
	if ( modelConstants.pbrFactors[0] != 0.0f || modelConstants.pbrFactors[3] != 0.0f ||
	     !lab.resolver->DrawGroup( lab.modernMesh, {} ) )
		return std::string( "the modern model reads the wrong material or draw inputs" );
	// Rim lighting is mapped by VertexLitFamily; enabled cloak remains unsupported.
	auto cloak = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "lab/vl/base" }, { "$cloakpassenabled", "1" } }, {} );
	if ( !cloak || material::ClaimForMesh( cloak.Value() ) ||
	     lab.resolver->ResolveMesh( cloak.Value() ) )
		return std::string( "the modern model silently accepted an unmapped term" );
	auto pbr = material::PbrFamily::Create( lab.device, kCanvasColor, kCanvasDepth );
	auto vertexLit = material::VertexLitFamily::Create( lab.device, kCanvasColor, kCanvasDepth );
	auto lines = pass::lines::LinesRenderer::Create( lab.device, kCanvasColor, kCanvasDepth );
	if ( !pbr || !vertexLit || !lines )
		return std::string( "a program's layouts were refused" );
	lab.pbr = std::move( pbr ).Value();
	lab.vertexLit = std::move( vertexLit ).Value();
	lab.lines = std::move( lines ).Value();
	if ( std::optional<std::string> why = Canvas::Create( lab.device, kSize, kSize, lab.canvas ) )
		return why;

	// The textures.
	auto &cache = lab.textures;
	bool staged =
	    StageConstant(
	        cache, "materials/lab/base", Format::kRGBA8Srgb, ByteTexel( 188, 125, 64, 128 ) ) &&
	    StageConstant(
	        cache, "materials/lab/bump", Format::kRGBA8Unorm, ByteTexel( 160, 96, 224, 255 ) ) &&
	    StageConstant( cache, "materials/lab/cube", Format::kRGBA16Float,
	        HalfTexel( in.cube.r, in.cube.g, in.cube.b ), true ) &&
	    StageConstant( cache, "lab:page:normal", Format::kRGBA16Float,
	        HalfTexel( in.page.r, in.page.g, in.page.b ) ) &&
	    StageConstant( cache, "lab:page:nan", Format::kRGBA16Float,
	        HalfTexel( std::nanf( "" ), std::nanf( "" ), std::nanf( "" ) ) ) &&
	    StageConstant( cache, "lab:page:inf", Format::kRGBA16Float,
	        HalfTexel( INFINITY, INFINITY, INFINITY ) ) &&
	    StageConstant(
	        cache, "lab:page:negative", Format::kRGBA16Float, HalfTexel( -0.5f, -0.5f, -0.5f ) ) &&
	    StageConstant(
	        cache, "lab:page:bright", Format::kRGBA16Float, HalfTexel( 4.0f, 4.0f, 4.0f ) ) &&
	    StageConstant(
	        cache, "lab/pbr/base", Format::kRGBA8Srgb, ByteTexel( 200, 150, 100, 255 ) ) &&
	    StageConstant(
	        cache, "lab/pbr/mrao", Format::kRGBA8Unorm, ByteTexel( 64, 153, 204, 255 ) ) &&
	    StageConstant(
	        cache, "lab/pbr/mrao-ao1", Format::kRGBA8Unorm, ByteTexel( 64, 153, 255, 255 ) ) &&
	    StageConstant(
	        cache, "lab/pbr/mrao-rough102", Format::kRGBA8Unorm, ByteTexel( 64, 102, 204, 255 ) ) &&
	    StageConstant( cache, "lab/pbr/mrao-metal102", Format::kRGBA8Unorm,
	        ByteTexel( 102, 153, 204, 255 ) ) &&
	    StageConstant(
	        cache, "lab/pbr/emission", Format::kRGBA8Srgb, ByteTexel( 60, 120, 180, 255 ) ) &&
	    StageConstant( cache, "lab:page:zero", Format::kRGBA16Float, HalfTexel( 0, 0, 0 ) ) &&
	    StageConstant( cache, "lab/vl/base", Format::kRGBA8Srgb, ByteTexel( 100, 200, 50, 255 ) );
	staged = staged && StageConstant( cache, "materials/lab/vl/base", Format::kRGBA8Srgb,
	                       ByteTexel( 100, 200, 50, 255 ) );
	{
		const material::PbrSplitSumTable table = material::SplitSumTable();
		TextureDesc desc;
		desc.format = table.format;
		desc.width = table.width;
		desc.height = table.height;
		staged = staged &&
		         cache.Stage( "lab/pbr/splitsum", desc, std::as_bytes( std::span( table.texels ) ) )
		             .HasValue();
	}
	if ( !staged )
		return std::string( "a fixture texture was refused" );

	// The lightmapped programs, through the one resolver.
	auto resolve = [&]( Program program, const char *shader,
	                   std::vector<material::VmtPair> variables ) -> std::optional<std::string>
	{
		auto mapped = material::MapVariables( shader, std::move( variables ), {} );
		if ( !mapped )
			return "a fixture material does not map: " + mapped.Error().detail;
		auto resolved = lab.resolver->Resolve( mapped.Value() );
		if ( !resolved )
			return "a fixture material is not drawn: " + resolved.Error();
		lab.programs[program] = std::move( resolved ).Value();
		const std::uint64_t id = lab.nextGroup++;
		if ( !lab.groups.Set( id, lab.programs[program].request.material ) )
			return std::string( "a fixture material group was refused" );
		lab.materialGroups[program] = id;
		return std::nullopt;
	};
	const material::VmtPair base{ "$basetexture", "lab/base" };
	for ( auto &[program, shader, variables] :
	    std::vector<std::tuple<Program, const char *, std::vector<material::VmtPair>>>{
	        { Program::kLightmapped, "LightmappedGeneric", { base } },
	        { Program::kBumped, "LightmappedGeneric", { base, { "$bumpmap", "lab/bump" } } },
	        { Program::kEnvmapped, "LightmappedGeneric",
	            { base, { "$envmap", "lab/cube" }, { "$envmaptint", "[0.5 0.5 0.5]" } } },
	        { Program::kSelfIllum, "LightmappedGeneric",
	            { base, { "$selfillum", "1" }, { "$selfillumtint", "[2 1 0.5]" } } },
	        { Program::kSelfIllumZero, "LightmappedGeneric",
	            { base, { "$selfillum", "1" }, { "$selfillumtint", "[0 0 0]" } } },
	        { Program::kUnlit, "UnlitGeneric", { base, { "$vertexcolor", "1" } } } } )
	{
		if ( std::optional<std::string> why = resolve( program, shader, variables ) )
			return why;
	}
	material::FrameTerms terms;
	terms.lightmapScale = kLightmapScale;
	const auto frame = lab.resolver->FrameGroup( lab.programs[Program::kLightmapped], terms );
	lab.frameGroup = lab.nextGroup++;
	if ( !frame || !lab.groups.Set( lab.frameGroup, *frame ) )
		return std::string( "the lightmapped frame group was refused" );
	for ( const char *page : { "lab:page:normal", "lab:page:nan", "lab:page:inf",
	          "lab:page:negative", "lab:page:bright", "lab:page:zero" } )
	{
		const auto group = lab.resolver->DrawGroup( lab.programs[Program::kLightmapped], { page } );
		const std::uint64_t id = lab.nextGroup++;
		if ( !group || !lab.groups.Set( id, *group ) )
			return std::string( "a lightmap group was refused" );
		lab.drawGroups[page] = id;
	}

	// pbr: material variants (MRAO and emission) and lighting variants: an
	// ambient cube of 0.2 and one directional light, the same without the
	// light, and the light under a black cube.
	lab.pbrFrameGroup = lab.nextGroup++;
	if ( !lab.groups.Set( lab.pbrFrameGroup,
	         lab.pbr->FrameGroup( material::SurfaceFrame(), "lab/pbr/splitsum" ) ) )
		return std::string( "the pbr frame group was refused" );
	struct PbrVariant
	{
		const char *name;
		const char *mrao;
		std::array<int, 3> texel;
		bool emission;
	};
	for ( const PbrVariant &variant :
	    { PbrVariant{ "default", "lab/pbr/mrao", { 64, 153, 204 }, false },
	        PbrVariant{ "ao1", "lab/pbr/mrao-ao1", { 64, 153, 255 }, false },
	        PbrVariant{ "rough102", "lab/pbr/mrao-rough102", { 64, 102, 204 }, false },
	        PbrVariant{ "metal102", "lab/pbr/mrao-metal102", { 102, 153, 204 }, false },
	        PbrVariant{ "emission", "lab/pbr/mrao", { 64, 153, 204 }, true } } )
	{
		material::PbrClaim claim;
		claim.claimed = true;
		claim.emission = variant.emission;
		claim.constants.emission[0] = variant.emission ? 1.5f : 0.0f;
		material::SurfaceTextures pbrTextures;
		pbrTextures.base = "lab/pbr/base";
		pbrTextures.mrao = variant.mrao;
		pbrTextures.emission = variant.emission ? "lab/pbr/emission" : "";
		auto pbrRequest = lab.pbr->Request( claim, pbrTextures );
		if ( !pbrRequest )
			return std::string( "the pbr program was refused" );
		if ( std::string_view( variant.name ) == "default" )
			lab.pbrRequest = pbrRequest.Value();
		lab.pbrPipelines[variant.name] = pbrRequest.Value().pipeline;
		const std::uint64_t id = lab.nextGroup++;
		if ( !lab.groups.Set( id, pbrRequest.Value().material ) )
			return std::string( "a pbr material group was refused" );
		lab.pbrMaterials[variant.name] = id;
		lab.pbrMrao[variant.name] = { variant.texel[0] / 255.0f, variant.texel[1] / 255.0f,
		    variant.texel[2] / 255.0f, variant.emission ? 1.5f : 0.0f };
	}
	lab.materialGroups[Program::kPbr] = lab.pbrMaterials["default"];
	const float eye[3] = { 0, 0, 0 };
	const float cube[6][3] = { { 0.2f, 0.2f, 0.2f }, { 0.2f, 0.2f, 0.2f }, { 0.2f, 0.2f, 0.2f },
	    { 0.2f, 0.2f, 0.2f }, { 0.2f, 0.2f, 0.2f }, { 0.2f, 0.2f, 0.2f } };
	const float black[6][3] = {};
	material::ModelLightDesc sun;
	sun.type = material::ModelLightType::kDirectional;
	sun.color[0] = 1.0f;
	sun.color[1] = 0.9f;
	sun.color[2] = 0.8f;
	sun.direction[0] = 0.8f;
	sun.direction[1] = -0.3f;
	sun.direction[2] = -0.52f;
	lab.pbrLightingValues["default"] =
	    material::PackSourceModelLighting( eye, cube, std::span( &sun, 1 ) );
	lab.pbrLightingValues["no-lights"] = material::PackSourceModelLighting( eye, cube, {} );
	lab.pbrLightingValues["cube-zero"] =
	    material::PackSourceModelLighting( eye, black, std::span( &sun, 1 ) );
	for ( const auto &[name, value] : lab.pbrLightingValues )
	{
		const std::uint64_t id = lab.nextGroup++;
		if ( !lab.groups.Set( id, lab.pbr->LightingGroup( value ) ) )
			return std::string( "a pbr lighting group was refused" );
		lab.pbrLightings[name] = id;
	}
	lab.modernMaterial = lab.nextGroup++;
	lab.modernDraw = lab.nextGroup++;
	lab.modernFrame = lab.nextGroup++;
	material::FrameTerms modernTerms;
	modernTerms.splitSumTable = "lab/pbr/splitsum";
	auto modernDraw = lab.resolver->DrawGroup( lab.modernMesh, {} );
	auto modernFrame = lab.resolver->FrameGroup( lab.modernMesh, modernTerms );
	if ( !modernDraw || !modernFrame ||
	     !lab.groups.Set( lab.modernMaterial, lab.modernMesh.request.material ) ||
	     !lab.groups.Set( lab.modernDraw, *modernDraw ) ||
	     !lab.groups.Set( lab.modernFrame, *modernFrame ) )
		return std::string( "the modern model's groups were refused" );
	lab.instancedMaterial = lab.nextGroup++;
	lab.instancedDraw = lab.nextGroup++;
	lab.instancedFrame = lab.nextGroup++;
	auto instancedDraw = lab.modelResolver->DrawGroup( lab.instancedMesh, {} );
	auto instancedFrame = lab.modelResolver->FrameGroup( lab.instancedMesh, modernTerms );
	if ( !instancedDraw || !instancedFrame ||
	     !lab.groups.Set( lab.instancedMaterial, lab.instancedMesh.request.material ) ||
	     !lab.groups.Set( lab.instancedDraw, *instancedDraw ) ||
	     !lab.groups.Set( lab.instancedFrame, *instancedFrame ) )
		return std::string( "the object-space mesh groups were refused" );
	const material::ModelLighting &lighting = lab.pbrLightingValues["default"];

	// vertexlit: the same lighting.
	material::VertexLitClaim vertexLitClaim;
	vertexLitClaim.claimed = true;
	std::copy( in.vertexLitColor, in.vertexLitColor + 3, vertexLitClaim.constants.tint );
	auto vertexLitRequest = lab.vertexLit->Request( vertexLitClaim, "lab/vl/base" );
	if ( !vertexLitRequest )
		return std::string( "the vertexlit program was refused" );
	lab.vertexLitRequest = vertexLitRequest.Value();
	lab.materialGroups[Program::kVertexLit] = lab.nextGroup++;
	lab.vertexLitLighting = lab.nextGroup++;
	lab.vertexLitFrameGroup = lab.nextGroup++;
	if ( !lab.groups.Set(
	         lab.materialGroups[Program::kVertexLit], lab.vertexLitRequest.material ) ||
	     !lab.groups.Set( lab.vertexLitLighting, lab.vertexLit->LightingGroup( lighting ) ) ||
	     !lab.groups.Set( lab.vertexLitFrameGroup, lab.vertexLit->FrameGroup() ) )
		return std::string( "a vertexlit group was refused" );

	// Each program's neutral view group (no clustered lights), by layout.
	std::vector<const material::ProgramRequest *> requests = { &lab.pbrRequest,
	    &lab.vertexLitRequest, &lab.modernMesh.request, &lab.instancedMesh.request };
	for ( const auto &[program, resolved] : lab.programs )
		requests.push_back( &resolved.request );
	for ( const material::ProgramRequest *request : requests )
	{
		if ( !request->viewLayout.IsValid() || !request->neutralView ||
		     lab.viewGroups.count( request->viewLayout.value ) )
			continue;
		const std::uint64_t id = lab.nextGroup++;
		if ( !lab.groups.Set( id, *request->neutralView ) )
			return std::string( "a view group was refused" );
		lab.viewGroups[request->viewLayout.value] = id;
	}
	return std::nullopt;
}

// The quad of one case as the program's vertex.
std::vector<std::byte> QuadVertices( const Lab &lab, Program program, int cell )
{
	const Inputs &in = lab.inputs;
	std::vector<std::byte> bytes;
	for ( const Corner &corner : QuadCorners( cell ) )
	{
		if ( program == Program::kPbr || program == Program::kInstancedMesh )
		{
			material::SurfaceModelVertex v;
			const float3 position = program == Program::kInstancedMesh
			                            ? corner.position - Camera::Centre( cell )
			                            : corner.position;
			std::memcpy( v.position, &position, sizeof( v.position ) );
			std::memcpy( v.normal, &in.normal, sizeof( v.normal ) );
			std::memcpy( v.tangent, &in.tangentS, sizeof( float ) * 3 );
			v.tangent[3] = 1.0f;
			v.uv[0] = corner.u;
			v.uv[1] = corner.v;
			const auto b = Bytes( v );
			bytes.insert( bytes.end(), b.begin(), b.end() );
		}
		else if ( program == Program::kVertexLit )
		{
			material::SurfaceModelVertex v;
			std::memcpy( v.position, &corner.position, sizeof( v.position ) );
			std::memcpy( v.normal, &in.normal, sizeof( v.normal ) );
			std::memcpy( v.tangent, &in.tangentS, sizeof( float ) * 3 );
			v.tangent[3] = 1.0f;
			v.uv[0] = corner.u;
			v.uv[1] = corner.v;
			const auto b = Bytes( v );
			bytes.insert( bytes.end(), b.begin(), b.end() );
		}
		else
		{
			material::SurfaceWorldVertex v;
			std::memcpy( v.position, &corner.position, sizeof( v.position ) );
			v.uv[0] = corner.u;
			v.uv[1] = corner.v;
			v.lightmapUv[0] = corner.u;
			v.lightmapUv[1] = corner.v;
			std::copy( in.vertexColor, in.vertexColor + 4, v.color );
			std::memcpy( v.normal, &in.normal, sizeof( v.normal ) );
			std::memcpy( v.tangentS, &in.tangentS, sizeof( v.tangentS ) );
			std::memcpy( v.tangentT, &in.tangentT, sizeof( v.tangentT ) );
			const auto b = Bytes( v );
			bytes.insert( bytes.end(), b.begin(), b.end() );
		}
	}
	return bytes;
}

// Draws cases (case i in cell cells[i]) and reads the frame. Lines cases are
// drawn by the lines pass through a graph onto the frame afterwards.
std::optional<std::string> DrawFrame(
    Lab &lab, std::span<const Case> cases, std::span<const int> cells, CanvasImage &image )
{
	std::vector<CanvasDraw> draws;
	material::FamilyDrawConstants constants;
	std::memcpy( constants.toClip, &lab.camera.ToClip(), sizeof( constants.toClip ) );
	const math::float4x4 identity = math::float4x4::Identity();
	std::memcpy( constants.world, &identity, sizeof( constants.world ) );
	for ( std::size_t i = 0; i < cases.size(); ++i )
	{
		const Case &c = cases[i];
		if ( c.program == Program::kLines )
			continue;
		const shaderlib::DebugSpecialization debug =
		    frame::DebugSpecializationFor( c.debug, ProgramName( c.program ) );
		CanvasDraw draw;
		const std::vector<std::byte> vertices = QuadVertices( lab, c.program, cells[i] );
		draw.vertices = lab.canvas->Vertices( vertices );
		draw.vertexCount = 6;
		auto group = [&]( std::uint64_t id ) -> BindGroupId
		{
			const material::ResidentGroup *resident = lab.groups.Group( id );
			return resident ? resident->group : BindGroupId();
		};
		if ( c.program == Program::kPbr )
		{
			auto pipeline = lab.pbr->DebugPipeline( lab.pbrPipelines.at( c.pbrMaterial ), debug );
			if ( !pipeline )
				return std::string( "no pbr debug pipeline" );
			draw.pipeline = pipeline.Value();
			draw.groups[std::size_t( BindGroupRole::kFrame )] = group( lab.pbrFrameGroup );
			draw.groups[std::size_t( BindGroupRole::kView )] =
			    group( lab.viewGroups.at( lab.pbrRequest.viewLayout.value ) );
			draw.groups[std::size_t( BindGroupRole::kDraw )] =
			    group( lab.pbrLightings.at( c.pbrLighting ) );
			draw.groups[std::size_t( BindGroupRole::kMaterial )] =
			    group( lab.pbrMaterials.at( c.pbrMaterial ) );
			draw.constants = Bytes( constants );
			if ( !c.mesh.empty() )
			{
				draw.vertices = lab.canvas->Vertices( c.mesh );
				draw.vertexCount =
				    std::uint32_t( c.mesh.size() / sizeof( material::SurfaceModelVertex ) );
			}
		}
		else if ( c.program == Program::kModernMesh )
		{
			auto pipeline = lab.resolver->DebugPipeline( lab.modernMesh, debug );
			if ( !pipeline )
				return pipeline.Error();
			draw.pipeline = pipeline.Value();
			draw.groups[std::size_t( BindGroupRole::kFrame )] = group( lab.modernFrame );
			draw.groups[std::size_t( BindGroupRole::kView )] =
			    group( lab.viewGroups.at( lab.modernMesh.request.viewLayout.value ) );
			draw.groups[std::size_t( BindGroupRole::kMaterial )] = group( lab.modernMaterial );
			draw.groups[std::size_t( BindGroupRole::kDraw )] = group( lab.modernDraw );
			draw.constants = Bytes( constants.toClip );
		}
		else if ( c.program == Program::kInstancedMesh )
		{
			auto pipeline = lab.modelResolver->DebugPipeline( lab.instancedMesh, debug );
			if ( !pipeline )
				return pipeline.Error();
			draw.pipeline = pipeline.Value();
			draw.groups[std::size_t( BindGroupRole::kFrame )] = group( lab.instancedFrame );
			draw.groups[std::size_t( BindGroupRole::kView )] =
			    group( lab.viewGroups.at( lab.instancedMesh.request.viewLayout.value ) );
			draw.groups[std::size_t( BindGroupRole::kMaterial )] = group( lab.instancedMaterial );
			draw.groups[std::size_t( BindGroupRole::kDraw )] = group( lab.instancedDraw );
			material::FamilyDrawConstants placed;
			const math::float4x4 world = math::Translation( Camera::Centre( cells[i] ) );
			const math::float4x4 toClip = math::Multiply( lab.camera.ToClip(), world );
			std::memcpy( placed.toClip, &toClip, sizeof( placed.toClip ) );
			std::memcpy( placed.world, &world, sizeof( placed.world ) );
			draw.constants = Bytes( placed );
		}
		else if ( c.program == Program::kVertexLit )
		{
			auto pipeline = lab.vertexLit->DebugPipeline( lab.vertexLitRequest.pipeline, debug );
			if ( !pipeline )
				return std::string( "no vertexlit debug pipeline" );
			draw.pipeline = pipeline.Value();
			draw.groups[std::size_t( BindGroupRole::kFrame )] = group( lab.vertexLitFrameGroup );
			draw.groups[std::size_t( BindGroupRole::kView )] =
			    group( lab.viewGroups.at( lab.vertexLitRequest.viewLayout.value ) );
			draw.groups[std::size_t( BindGroupRole::kMaterial )] =
			    group( lab.materialGroups[Program::kVertexLit] );
			draw.groups[std::size_t( BindGroupRole::kDraw )] = group( lab.vertexLitLighting );
			draw.constants = Bytes( constants );
		}
		else
		{
			const material::ResolvedProgram &program = lab.programs.at( c.program );
			auto pipeline = lab.resolver->DebugPipeline( program, debug );
			if ( !pipeline )
				return pipeline.Error();
			draw.pipeline = pipeline.Value();
			draw.groups[std::size_t( BindGroupRole::kFrame )] = group( lab.frameGroup );
			draw.groups[std::size_t( BindGroupRole::kView )] =
			    group( lab.viewGroups.at( program.request.viewLayout.value ) );
			draw.groups[std::size_t( BindGroupRole::kMaterial )] =
			    group( lab.materialGroups.at( c.program ) );
			draw.groups[std::size_t( BindGroupRole::kDraw )] = group( lab.drawGroups.at( c.page ) );
			draw.constants = Bytes( constants.toClip );
		}
		for ( const BindGroupId &id : draw.groups )
			(void)id;
		draws.push_back( std::move( draw ) );
	}
	// Groups become resident at the canvas's first recording; the fixtures
	// record a warm-up frame first so every group exists here.
	for ( const CanvasDraw &draw : draws )
	{
		if ( !draw.pipeline.IsValid() )
			return std::string( "a draw has no pipeline" );
	}
	if ( std::optional<std::string> why =
	         lab.canvas->Render( lab.textures, lab.groups, draws, { 0, 0, 0, 1 }, &image ) )
		return why;

	// The lines cases, on a target of their own through the lines pass's
	// graph, copied into the image.
	bool anyLines = false;
	pass::lines::LineList list;
	shaderlib::DebugSpecialization linesDebug;
	std::vector<int> lineCells;
	for ( std::size_t i = 0; i < cases.size(); ++i )
	{
		if ( cases[i].program != Program::kLines )
			continue;
		// One specialization per lines frame: the lines pass takes the
		// frame's; the suite draws one lines case per frame.
		linesDebug = frame::DebugSpecializationFor( cases[i].debug, "lines" );
		anyLines = true;
		lineCells.push_back( cells[i] );
		const auto corners = QuadCorners( cells[i] );
		const pass::lines::Rgba8 color{ lab.inputs.lineColor[0], lab.inputs.lineColor[1],
		    lab.inputs.lineColor[2], lab.inputs.lineColor[3] };
		const pass::lines::Style style{ pass::lines::Space::kWorld, false };
		list.Triangle( style, corners[0].position, corners[1].position, corners[2].position, color,
		    color, color );
		list.Triangle( style, corners[3].position, corners[4].position, corners[5].position, color,
		    color, color );
	}
	if ( !anyLines )
		return std::nullopt;
	TextureDesc colorDesc;
	colorDesc.format = kCanvasColor;
	colorDesc.width = colorDesc.height = kSize;
	colorDesc.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kCopySource };
	BufferDesc readbackDesc;
	readbackDesc.size = std::uint64_t( kSize ) * kSize * 8;
	readbackDesc.usages = { ResourceUsage::kCopyDestination };
	readbackDesc.memory = MemoryKind::kReadback;
	auto color = lab.device.CreateTexture( colorDesc );
	auto readback = lab.device.CreateBuffer( readbackDesc );
	if ( !color || !readback )
		return std::string( "the lines target was refused" );
	graph::GraphBuilder builder;
	const graph::ResourceRef colorRef = builder.ImportTexture(
	    "color", color.Value(), colorDesc, ResourceUsage::kUndefined, ResourceUsage::kCopySource );
	TextureDesc depthDesc = colorDesc;
	depthDesc.format = kCanvasDepth;
	depthDesc.usages = { ResourceUsage::kDepthWrite };
	const graph::ResourceRef depthRef = builder.CreateTexture( "depth", depthDesc );
	const graph::ResourceRef readbackRef = builder.ImportBuffer( "readback", readback.Value(),
	    readbackDesc, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	pass::lines::LinesTargets targets;
	targets.color = colorRef;
	targets.depth = depthRef;
	targets.width = targets.height = kSize;
	targets.clear = { 0, 0, 0, 1 };
	pass::lines::LinesView view;
	view.worldToClip = lab.camera.ToClip();
	view.width = view.height = kSize;
	view.debug = linesDebug;
	auto added = lab.lines->AddPasses( builder, list, {}, view, targets );
	if ( !added )
		return std::string( "the lines passes were refused" );
	builder.AddPass( "readback", graph::PassKind::kCopy )
	    .Read( colorRef, ResourceUsage::kCopySource )
	    .Write( readbackRef, ResourceUsage::kCopyDestination )
	    .SideEffect()
	    .Execute(
	        [colorRef, readbackRef]( graph::RecordContext &context )
	        {
		        context.Encoder().CopyTextureToBuffer( context.Texture( colorRef ),
		            context.Buffer( readbackRef ), { 0, 0, 0, kSize, kSize } );
	        } );
	auto compiled = graph::CompileGraph( std::move( builder ) );
	if ( !compiled )
		return std::string( "the lines graph did not compile" );
	graph::SerialGraphExecutor executor;
	auto executed = executor.Execute( compiled.Value(), lab.device );
	if ( !executed )
		return std::string( "the lines graph did not execute" );
	(void)lab.device.WaitIdle();
	lab.lines->Collect( executed.Value().token );
	std::vector<std::byte> pixels( readbackDesc.size );
	if ( !lab.device.ReadBuffer( readback.Value(), 0, pixels ) )
		return std::string( "the lines target did not read back" );
	// Copy the lines cells into the image.
	for ( int cell : lineCells )
	{
		for ( auto [x, y] : lab.camera.Interior( cell ) )
		{
			for ( int c = 0; c < 4; ++c )
			{
				std::uint16_t half;
				std::memcpy( &half,
				    pixels.data() + ( ( std::size_t( y ) * kSize + x ) * 4 + c ) * 2,
				    sizeof( half ) );
				image.rgba[( std::size_t( y ) * kSize + x ) * 4 + c] = HalfToFloat( half );
			}
		}
	}
	(void)lab.device.Release( color.Value(), executed.Value().token );
	(void)lab.device.Release( readback.Value(), executed.Value().token );
	return std::nullopt;
}

std::string Describe( const Rgb &expected, const float *actual )
{
	char text[160];
	std::snprintf( text, sizeof( text ), "expected (%.4f %.4f %.4f), got (%.4f %.4f %.4f)",
	    expected.r, expected.g, expected.b, actual[0], actual[1], actual[2] );
	return text;
}

bool Near( float expected, float actual, float tolerance )
{
	if ( std::isnan( expected ) )
		return std::isnan( actual );
	return std::fabs( expected - actual ) <= tolerance;
}

// Compares every case's interior pixels with its expectation.
void Judge( const Lab &lab, std::span<const Case> cases, std::span<const int> cells,
    const CanvasImage &image, Results &results )
{
	for ( std::size_t i = 0; i < cases.size(); ++i )
	{
		const Case &c = cases[i];
		std::size_t compared = 0;
		std::string failure;
		for ( auto [x, y] : lab.camera.Interior( cells[i] ) )
		{
			const std::optional<Rgb> expected = c.expect( x, y, lab.camera.Hit( x, y ), cells[i] );
			if ( !expected )
				continue;
			++compared;
			const float *actual = image.At( x, y );
			if ( failure.empty() && !( Near( expected->r, actual[0], c.tolerance ) &&
			                            Near( expected->g, actual[1], c.tolerance ) &&
			                            Near( expected->b, actual[2], c.tolerance ) ) )
				failure = "pixel (" + std::to_string( x ) + ", " + std::to_string( y ) +
				          "): " + Describe( *expected, actual );
		}
		results.That( compared >= 8 && failure.empty(), c.name,
		    compared < 8 ? "compared " + std::to_string( compared ) + " pixels" : failure );
	}
}

// The constant expectation.
auto Constant( Rgb value )
{
	return [value]( std::uint32_t, std::uint32_t, float3, int ) -> std::optional<Rgb>
	{
		return value;
	};
}

auto HatchExpected()
{
	return []( std::uint32_t x, std::uint32_t y, float3, int ) -> std::optional<Rgb>
	{
		return Hatch( x, y );
	};
}

Rgb Scale( Rgb c, float s )
{
	return { c.r * s, c.g * s, c.b * s };
}

Rgb Multiply( Rgb a, Rgb b )
{
	return { a.r * b.r, a.g * b.g, a.b * b.b };
}

Rgb Encoded( float3 n )
{
	return { n.x * 0.5f + 0.5f, n.y * 0.5f + 0.5f, n.z * 0.5f + 0.5f };
}

// The pixel cases, from the programs' constants.
std::vector<Case> PixelCases( const Lab &lab )
{
	const Inputs &in = lab.inputs;
	std::vector<Case> cases;
	auto add = [&]( std::string name, Program program, frame::DebugControls debug, auto expect,
	               std::string page = "lab:page:normal" )
	{
		Case c;
		c.name = std::move( name );
		c.program = program;
		c.debug = debug;
		c.expect = expect;
		c.page = std::move( page );
		cases.push_back( std::move( c ) );
	};
	using V = shaderlib::DebugView;
	const auto view = []( V v )
	{
		return View( std::uint32_t( v ) );
	};

	// The lightmapped constants the resolver packed (the formulas read them).
	material::SurfaceConstants lightmapped;
	std::memcpy( &lightmapped,
	    lab.programs.at( Program::kLightmapped ).request.material.constants.data(),
	    sizeof( lightmapped ) );
	material::SurfaceConstants envmapped;
	std::memcpy( &envmapped,
	    lab.programs.at( Program::kEnvmapped ).request.material.constants.data(),
	    sizeof( envmapped ) );
	material::SurfaceConstants selfIllum;
	std::memcpy( &selfIllum,
	    lab.programs.at( Program::kSelfIllum ).request.material.constants.data(),
	    sizeof( selfIllum ) );
	const Rgb tint{ lightmapped.tint[0], lightmapped.tint[1], lightmapped.tint[2] };
	const Rgb albedo = Multiply( in.base, tint );

	add( "view.1.albedo.lightmapped", Program::kLightmapped, view( V::kAlbedo ),
	    Constant( albedo ) );
	add( "view.2.normal.lightmapped", Program::kLightmapped, view( V::kWorldNormal ),
	    Constant( Encoded( in.normal ) ) );
	{
		const float3 t{ in.bump[0] * 2 - 1, in.bump[1] * 2 - 1, in.bump[2] * 2 - 1 };
		const float3 bumped =
		    math::Normalize( { t.x * in.tangentS.x + t.y * in.tangentT.x + t.z * in.normal.x,
		        t.x * in.tangentS.y + t.y * in.tangentT.y + t.z * in.normal.y,
		        t.x * in.tangentS.z + t.y * in.tangentT.z + t.z * in.normal.z } );
		add( "view.2.normal.bumped", Program::kBumped, view( V::kWorldNormal ),
		    Constant( Encoded( bumped ) ) );
		add( "view.3.normal-map.bumped", Program::kBumped, view( V::kNormalMap ),
		    Constant( { in.bump[0], in.bump[1], in.bump[2] } ) );
	}
	add( "view.3.hatch.lightmapped-without-bump", Program::kLightmapped, view( V::kNormalMap ),
	    HatchExpected() );
	add(
	    "view.4.hatch.lightmapped", Program::kLightmapped, view( V::kRoughness ), HatchExpected() );
	add( "view.5.hatch.pbr-unfiltered", Program::kPbr, view( V::kFilteredRoughness ),
	    HatchExpected() );
	add(
	    "view.6.hatch.lightmapped", Program::kLightmapped, view( V::kMetalness ), HatchExpected() );
	add( "view.7.ao.lightmapped-neutral", Program::kLightmapped, view( V::kAmbientOcclusion ),
	    Constant( { 1, 1, 1 } ) );
	add( "view.8.baked.lightmapped", Program::kLightmapped, view( V::kBakedLight ),
	    Constant( Scale( in.page, kLightmapScale ) ) );
	{
		frame::DebugControls scaled = view( V::kBakedLight );
		scaled.viewScale = 2.0f;
		add( "view.8.baked.scale-2", Program::kLightmapped, scaled,
		    Constant( Scale( in.page, kLightmapScale * 2.0f ) ) );
	}
	add( "view.8.baked.bumped-basis", Program::kBumped, view( V::kBakedLight ),
	    Constant( Scale( in.page, kLightmapScale ) ) );
	add( "view.8.hatch.unlit", Program::kUnlit, view( V::kBakedLight ), HatchExpected() );
	add( "view.9.hatch.lightmapped", Program::kLightmapped, view( V::kDirectLight ),
	    HatchExpected() );
	{
		// The env map's fast path with its constants (lightmapped.frag).
		Rgb specular = Multiply(
		    in.cube, { envmapped.envTint[0], envmapped.envTint[1], envmapped.envTint[2] } );
		const float fresnelScale = envmapped.envContrast[3];
		const Rgb contrast{
		    envmapped.envContrast[0], envmapped.envContrast[1], envmapped.envContrast[2] };
		specular = { specular.r + ( specular.r * specular.r - specular.r ) * contrast.r,
		    specular.g + ( specular.g * specular.g - specular.g ) * contrast.g,
		    specular.b + ( specular.b * specular.b - specular.b ) * contrast.b };
		const float grey = 0.299f * specular.r + 0.587f * specular.g + 0.114f * specular.b;
		specular = { grey + ( specular.r - grey ) * envmapped.envSaturation[0],
		    grey + ( specular.g - grey ) * envmapped.envSaturation[1],
		    grey + ( specular.b - grey ) * envmapped.envSaturation[2] };
		// Fresnel is constant only where its scale is zero (the fast path).
		if ( fresnelScale == 0.0f )
			add( "view.10.image-specular.envmapped", Program::kEnvmapped, view( V::kImageSpecular ),
			    Constant( Scale( specular, envmapped.envTint[3] ) ) );
		else
			add( "view.10.image-specular.envmapped-fast-path", Program::kEnvmapped,
			    view( V::kImageSpecular ),
			    []( std::uint32_t, std::uint32_t, float3, int ) -> std::optional<Rgb>
			    {
				    return Rgb{ -1, -1, -1 }; // the fixture must take the fast path
			    } );
	}
	add( "view.10.hatch.lightmapped-without-envmap", Program::kLightmapped,
	    view( V::kImageSpecular ), HatchExpected() );
	add( "view.11.hatch.lightmapped", Program::kLightmapped, view( V::kScreenSpaceReflections ),
	    HatchExpected() );
	add( "view.11.hatch.pbr", Program::kPbr, view( V::kScreenSpaceReflections ), HatchExpected() );
	add( "view.12.emission.selfillum", Program::kSelfIllum, view( V::kEmissive ),
	    Constant( Scale(
	        Multiply( { selfIllum.selfIllumTint[0], selfIllum.selfIllumTint[1],
	                      selfIllum.selfIllumTint[2] },
	            Multiply( in.base, { selfIllum.tint[0], selfIllum.tint[1], selfIllum.tint[2] } ) ),
	        in.baseAlpha ) ) );
	add( "view.12.hatch.lightmapped-without-selfillum", Program::kLightmapped, view( V::kEmissive ),
	    HatchExpected() );
	const auto checker = []( std::uint32_t, std::uint32_t, float3 hit,
	                         int cell ) -> std::optional<Rgb>
	{
		float u, v;
		Camera::Uv( cell, hit, u, v );
		const float fu = 8.0f * u, fv = 8.0f * v;
		// Skip pixels at a cell edge, where interpolation may round either way.
		if ( std::fabs( fu - std::round( fu ) ) < 0.1f ||
		     std::fabs( fv - std::round( fv ) ) < 0.1f )
			return std::nullopt;
		const float value = float( int( std::floor( fu ) + std::floor( fv ) ) % 2 );
		return Rgb{ value, value, value };
	};
	add( "view.13.uv-checker.lightmapped", Program::kLightmapped, view( V::kUvChecker ), checker );
	add( "view.14.vertex-color.lightmapped", Program::kLightmapped, view( V::kVertexColor ),
	    Constant( { in.vertexColor[0] / 255.0f, in.vertexColor[1] / 255.0f,
	        in.vertexColor[2] / 255.0f } ) );
	add( "view.14.vertex-color.unlit-decoded", Program::kUnlit, view( V::kVertexColor ),
	    Constant( { std::pow( in.vertexColor[0] / 255.0f, 2.2f ),
	        std::pow( in.vertexColor[1] / 255.0f, 2.2f ),
	        std::pow( in.vertexColor[2] / 255.0f, 2.2f ) } ) );
	{
		frame::DebugControls depth = view( V::kLinearDepth );
		depth.viewRange = 512.0f;
		add( "view.15.linear-depth.lightmapped", Program::kLightmapped, depth,
		    Constant( { kDepth / 512.0f, kDepth / 512.0f, kDepth / 512.0f } ) );
		add( "view.15.linear-depth.pbr", Program::kPbr, depth,
		    Constant( { kDepth / 512.0f, kDepth / 512.0f, kDepth / 512.0f } ) );
	}
	const Rgb final = Multiply( albedo, Scale( in.page, kLightmapScale ) );
	add( "view.16.nan.lightmapped", Program::kLightmapped, view( V::kNanInfNegative ),
	    Constant( { 1, 0, 1 } ), "lab:page:nan" );
	add( "view.16.inf.lightmapped", Program::kLightmapped, view( V::kNanInfNegative ),
	    Constant( { 0, 1, 1 } ), "lab:page:inf" );
	add( "view.16.negative.lightmapped", Program::kLightmapped, view( V::kNanInfNegative ),
	    Constant( { 1, 1, 0 } ), "lab:page:negative" );
	add( "view.16.finite.lightmapped", Program::kLightmapped, view( V::kNanInfNegative ),
	    Constant( Scale( { 1, 1, 1 }, Luminance( final ) * 0.5f ) ) );
	add( "view.17.over.lightmapped", Program::kLightmapped, view( V::kOverRange ),
	    Constant( { 1, 0, 0 } ), "lab:page:bright" );
	add( "view.17.under.lightmapped", Program::kLightmapped, view( V::kOverRange ),
	    Constant( Scale( { 1, 1, 1 }, Luminance( final ) * 0.5f ) ) );

	// pbr: the material's MRAO.
	add( "view.1.albedo.pbr", Program::kPbr, view( V::kAlbedo ), Constant( in.pbrBase ) );
	add( "view.2.normal.pbr", Program::kPbr, view( V::kWorldNormal ),
	    Constant( Encoded( in.normal ) ) );
	add( "view.3.hatch.pbr-without-normal-map", Program::kPbr, view( V::kNormalMap ),
	    HatchExpected() );
	add( "view.4.roughness.pbr", Program::kPbr, view( V::kRoughness ),
	    Constant( { in.mrao[1], in.mrao[1], in.mrao[1] } ) );
	add( "view.6.metalness.pbr", Program::kPbr, view( V::kMetalness ),
	    Constant( { in.mrao[0], in.mrao[0], in.mrao[0] } ) );
	add( "view.7.ao.pbr", Program::kPbr, view( V::kAmbientOcclusion ),
	    Constant( { in.mrao[2], in.mrao[2], in.mrao[2] } ) );
	add( "view.8.hatch.pbr", Program::kPbr, view( V::kBakedLight ), HatchExpected() );
	add( "view.12.hatch.pbr-without-emission", Program::kPbr, view( V::kEmissive ),
	    HatchExpected() );
	add( "view.13.uv-checker.pbr", Program::kPbr, view( V::kUvChecker ), checker );
	add( "view.14.hatch.pbr-no-color-stream", Program::kPbr, view( V::kVertexColor ),
	    HatchExpected() );
	// The imported model has no MRAO image; its dielectric factors must reach
	// the same pixel program, even though the neutral material texture is white.
	add( "view.1.albedo.modern-mesh", Program::kModernMesh, view( V::kAlbedo ),
	    Constant( in.vertexLitBase ) );
	add( "view.4.roughness.modern-mesh", Program::kModernMesh, view( V::kRoughness ),
	    Constant( { 0.55f, 0.55f, 0.55f } ) );
	add( "view.6.metalness.modern-mesh", Program::kModernMesh, view( V::kMetalness ),
	    Constant( { 0.0f, 0.0f, 0.0f } ) );
	add( "view.7.ao.modern-mesh", Program::kModernMesh, view( V::kAmbientOcclusion ),
	    Constant( { 1.0f, 1.0f, 1.0f } ) );
	add( "view.1.albedo.instanced-mesh", Program::kInstancedMesh, view( V::kAlbedo ),
	    Constant( in.vertexLitBase ) );
	add( "view.4.roughness.instanced-mesh", Program::kInstancedMesh, view( V::kRoughness ),
	    Constant( { 0.55f, 0.55f, 0.55f } ) );

	// vertexlit: albedo, and no separable light term.
	add( "view.1.albedo.vertexlit", Program::kVertexLit, view( V::kAlbedo ),
	    Constant( Multiply( in.vertexLitBase,
	        { in.vertexLitColor[0], in.vertexLitColor[1], in.vertexLitColor[2] } ) ) );
	add( "view.9.hatch.vertexlit", Program::kVertexLit, view( V::kDirectLight ), HatchExpected() );
	add( "view.7.ao.vertexlit-neutral", Program::kVertexLit, view( V::kAmbientOcclusion ),
	    Constant( { 1, 1, 1 } ) );

	// lines: the vertex color (the lab's float target takes it undecoded).
	add( "view.14.vertex-color.lines", Program::kLines, view( V::kVertexColor ),
	    Constant(
	        { in.lineColor[0] / 255.0f, in.lineColor[1] / 255.0f, in.lineColor[2] / 255.0f } ) );
	add( "view.1.hatch.lines", Program::kLines, view( V::kAlbedo ), HatchExpected() );
	add(
	    "view.15.linear-depth.lines", Program::kLines,
	    [&]
	    {
		    frame::DebugControls depth = view( V::kLinearDepth );
		    depth.viewRange = 512.0f;
		    return depth;
	    }(),
	    Constant( { kDepth / 512.0f, kDepth / 512.0f, kDepth / 512.0f } ) );

	// The program filter: outside it, flat 18% grey; inside, the view.
	{
		frame::DebugControls filtered = view( V::kAlbedo );
		std::strcpy( filtered.program, "pbr" );
		add( "filter.outside-is-grey.lightmapped", Program::kLightmapped, filtered,
		    Constant( { 0.18f, 0.18f, 0.18f } ) );
		add( "filter.inside-is-the-view.pbr", Program::kPbr, filtered, Constant( in.pbrBase ) );
	}
	return cases;
}

// Renders the cases, eight lines-free cases per frame row, lines cases one
// per frame.
std::optional<std::string> RunCases( Lab &lab, std::span<const Case> cases, Results &results )
{
	std::vector<Case> batch;
	std::vector<int> cells;
	auto flush = [&]() -> std::optional<std::string>
	{
		if ( batch.empty() )
			return std::nullopt;
		CanvasImage image;
		if ( std::optional<std::string> why = DrawFrame( lab, batch, cells, image ) )
			return why;
		Judge( lab, batch, cells, image, results );
		batch.clear();
		cells.clear();
		return std::nullopt;
	};
	for ( const Case &c : cases )
	{
		if ( c.program == Program::kLines )
		{
			if ( std::optional<std::string> why = flush() )
				return why;
			batch.push_back( c );
			cells.push_back( 27 );
			if ( std::optional<std::string> why = flush() )
				return why;
			continue;
		}
		batch.push_back( c );
		cells.push_back( int( cells.size() ) );
		if ( batch.size() == 64 )
		{
			if ( std::optional<std::string> why = flush() )
				return why;
		}
	}
	return flush();
}

// The pbr direct-light and image-specular views equal the frame minus the
// frame without their term (one quad, same cell, in each frame).
std::optional<std::string> RelationalCases( Lab &lab, Results &results )
{
	using V = shaderlib::DebugView;
	auto render = [&]( frame::DebugControls debug, CanvasImage &image )
	{
		Case c;
		c.program = Program::kPbr;
		c.debug = debug;
		const int cell = 27;
		return DrawFrame( lab, std::span( &c, 1 ), std::span( &cell, 1 ), image );
	};
	CanvasImage full, noLights, noIbl, direct, image;
	frame::DebugControls controls;
	if ( auto why = render( controls, full ) )
		return why;
	controls.termsOff = shaderlib::kDebugTermClustered;
	if ( auto why = render( controls, noLights ) )
		return why;
	controls.termsOff = shaderlib::kDebugTermIbl;
	if ( auto why = render( controls, noIbl ) )
		return why;
	if ( auto why = render( View( std::uint32_t( V::kDirectLight ) ), direct ) )
		return why;
	if ( auto why = render( View( std::uint32_t( V::kImageSpecular ) ), image ) )
		return why;
	auto judge = [&]( const char *name, const CanvasImage &view, const CanvasImage &without )
	{
		std::string failure;
		std::size_t lit = 0;
		for ( auto [x, y] : lab.camera.Interior( 27 ) )
		{
			for ( int c = 0; c < 3; ++c )
			{
				const float expected = full.At( x, y )[c] - without.At( x, y )[c];
				lit += expected > 0.01f ? 1 : 0;
				if ( failure.empty() && !Near( expected, view.At( x, y )[c], 2.0f * kTolerance ) )
					failure = "pixel (" + std::to_string( x ) + ", " + std::to_string( y ) +
					          ") channel " + std::to_string( c ) + ": expected " +
					          std::to_string( expected ) + ", got " +
					          std::to_string( view.At( x, y )[c] );
			}
		}
		results.That( failure.empty() && lit > 0, name,
		    lit == 0 ? std::string( "the term adds nothing to the fixture" ) : failure );
	};
	judge( "view.9.direct.pbr-is-the-lights-share", direct, noLights );
	judge( "view.10.image-specular.pbr-is-the-ibl-share", image, noIbl );
	return std::nullopt;
}

// The controls' validation and specialization, without a device.
void ControlChecks( Results &results )
{
	using frame::DebugControls;
	using frame::DebugControlsStatus;
	const auto status = []( const DebugControls &controls ) -> int
	{
		auto valid = frame::ValidateDebugControls( controls, kLabPrograms );
		return valid ? 0 : int( valid.Error().status );
	};
	DebugControls controls;
	results.That( status( controls ) == 0, "controls.neutral-valid" );
	results.That( frame::DebugControlsNeutral( controls ), "controls.default-is-neutral" );
	bool installed = true;
	for ( std::uint32_t view = 1; view <= 17; ++view )
		installed = installed && status( View( view ) ) == 0;
	results.That( installed, "controls.views-1-17-valid" );
	results.That( status( View( 24 ) ) == 0 && status( View( 25 ) ) == 0 &&
	                  status( View( 26 ) ) == 0 && status( View( 27 ) ) == 0,
	    "controls.probe-views-24-27-valid" );
	results.That( status( View( 28 ) ) == int( DebugControlsStatus::kUnknownView ),
	    "controls.reserved-number-28-rejected" );
	results.That( status( View( 37 ) ) == int( DebugControlsStatus::kUnknownView ),
	    "controls.number-37-rejected" );
	results.That( status( View( 18 ) ) == int( DebugControlsStatus::kReservedView ),
	    "controls.view-18-reserved-until-its-term" );
	controls = View( 1 );
	std::strcpy( controls.program, "nope" );
	results.That( status( controls ) == int( DebugControlsStatus::kUnknownProgram ),
	    "controls.unknown-program-rejected" );
	std::strcpy( controls.program, "pbr" );
	results.That( status( controls ) == 0, "controls.known-program-accepted" );
	controls = DebugControls();
	std::memset( controls.program, 'x', sizeof( controls.program ) );
	results.That( status( controls ) == int( DebugControlsStatus::kUnterminatedProgram ),
	    "controls.unterminated-program-rejected" );
	controls = DebugControls();
	controls.brdf = 5;
	results.That( status( controls ) == int( DebugControlsStatus::kInvalidBrdf ),
	    "controls.brdf-5-rejected" );
	controls = DebugControls();
	controls.termsOff = ~shaderlib::kDebugTermAll;
	results.That( status( controls ) == int( DebugControlsStatus::kInvalidTerms ),
	    "controls.unknown-term-bit-rejected" );
	controls = DebugControls();
	controls.viewScale = 0.0f;
	results.That( status( controls ) == int( DebugControlsStatus::kInvalidNumber ),
	    "controls.zero-scale-rejected" );
	controls = DebugControls();
	controls.viewRange = std::nanf( "" );
	results.That( status( controls ) == int( DebugControlsStatus::kInvalidNumber ),
	    "controls.nan-range-rejected" );
	controls = DebugControls();
	controls.forceRoughness = 1.5f;
	results.That( status( controls ) == int( DebugControlsStatus::kInvalidOverride ),
	    "controls.roughness-1.5-rejected" );
	controls = DebugControls();
	controls.forceMetalness = -0.5f;
	results.That( status( controls ) == int( DebugControlsStatus::kInvalidOverride ),
	    "controls.metalness-minus-half-rejected" );
	controls = DebugControls();
	controls.legacy = frame::DebugLegacy( 3 );
	results.That( status( controls ) == int( DebugControlsStatus::kInvalidLegacy ),
	    "controls.legacy-3-rejected" );

	// Specialization.
	results.That( frame::DebugSpecializationFor( DebugControls(), "lightmapped" ).IsNeutral(),
	    "specialization.neutral-controls-are-the-shipped-program" );
	std::vector<SpecializationConstant> constants;
	shaderlib::AppendDebugConstants( {}, ShaderStage::kFragment, constants );
	results.That( constants.empty(), "specialization.neutral-appends-no-constant" );
	controls = View( 8 );
	controls.viewScale = 3.0f;
	const shaderlib::DebugSpecialization baked =
	    frame::DebugSpecializationFor( controls, "lightmapped" );
	shaderlib::AppendDebugConstants( baked, ShaderStage::kFragment, constants );
	results.That( constants.size() == 9 && baked.view == 8 && baked.scale == 3.0f,
	    "specialization.a-view-appends-every-constant" );
	controls.view = 2; // not radiometric: the scale stays neutral (one pipeline per view)
	results.That( frame::DebugSpecializationFor( controls, "lightmapped" ).scale == 1.0f,
	    "specialization.scale-only-for-radiometric-views" );
	controls = View( 1 );
	std::strcpy( controls.program, "pbr" );
	const auto outside = frame::DebugSpecializationFor( controls, "lightmapped" );
	const auto inside = frame::DebugSpecializationFor( controls, "pbr" );
	results.That( ( outside.flags & shaderlib::kDebugFlagFilteredOut ) != 0 && outside.view == 0 &&
	                  inside.view == 1 && inside.flags == 0,
	    "specialization.filter-greys-others" );
	controls = DebugControls();
	controls.termsOff = shaderlib::kDebugTermAo;
	std::strcpy( controls.program, "pbr" );
	results.That( frame::DebugSpecializationFor( controls, "lightmapped" ).termsOff ==
	                  shaderlib::kDebugTermAo,
	    "specialization.terms-apply-to-every-program" );

	// The lab's option parser (the ConVars' twin).
	DebugControls parsed;
	results.That( ParseDebugOption( "--debug-term", "ao,ibl", parsed ) &&
	                  parsed.termsOff == ( shaderlib::kDebugTermAo | shaderlib::kDebugTermIbl ),
	    "options.term-list" );
	DebugControls shadowParsed;
	results.That( ParseDebugOption( "--debug-term", "shadow_visibility", shadowParsed ) &&
	                  shadowParsed.termsOff == shaderlib::kDebugTermShadowVisibility &&
	                  status( shadowParsed ) == 0,
	    "options.shadow-visibility-term-is-valid" );
	results.That(
	    !ParseDebugOption( "--debug-term", "ao,sky", parsed ), "options.unknown-term-refused" );
	results.That( !ParseDebugOption( "--debug-view", "2x", parsed ), "options.bad-number-refused" );
}

// Shipped pipelines stay shipped.
void IdentityChecks( Lab &lab, Results &results )
{
	const material::ResolvedProgram &program = lab.programs.at( Program::kLightmapped );
	auto neutral = lab.resolver->DebugPipeline( program, {} );
	results.That( neutral && neutral.Value() == program.request.pipeline,
	    "identity.neutral-is-the-shipped-lightmapped-pipeline" );
	auto pbr = lab.pbr->DebugPipeline( lab.pbrRequest.pipeline, {} );
	results.That( pbr && pbr.Value() == lab.pbrRequest.pipeline,
	    "identity.neutral-is-the-shipped-pbr-pipeline" );
	auto vertexLit = lab.vertexLit->DebugPipeline( lab.vertexLitRequest.pipeline, {} );
	results.That( vertexLit && vertexLit.Value() == lab.vertexLitRequest.pipeline,
	    "identity.neutral-is-the-shipped-vertexlit-pipeline" );
	shaderlib::DebugSpecialization view;
	view.view = 1;
	auto variant = lab.resolver->DebugPipeline( program, view );
	auto again = lab.resolver->DebugPipeline( program, view );
	results.That( variant && again && variant.Value() != program.request.pipeline &&
	                  variant.Value() == again.Value(),
	    "identity.a-view-is-another-pipeline-made-once" );
	material::ResolvedProgram foreign = program;
	foreign.request.pipeline = PipelineId( 0xdead );
	results.That( !lab.resolver->DebugPipeline( foreign, view ),
	    "identity.a-foreign-pipeline-has-no-variant" );
}

// ---------------------------------------------------------------------------
// RFC 0014 D1: the lighting-model controls (render_lab suite lighting-controls)
// ---------------------------------------------------------------------------

constexpr int kControlCell = 27;

// One case alone in a frame, in the control cell.
std::optional<std::string> RenderOne( Lab &lab, const Case &c, CanvasImage &image )
{
	const int cell = kControlCell;
	return DrawFrame( lab, std::span( &c, 1 ), std::span( &cell, 1 ), image );
}

Case Make( Program program, frame::DebugControls debug = {} )
{
	Case c;
	c.program = program;
	c.debug = debug;
	return c;
}

frame::DebugControls TermsOff( std::uint32_t terms )
{
	frame::DebugControls debug;
	debug.termsOff = terms;
	return debug;
}

// Whether two frames are equal bit for bit, and whether they differ at all.
bool SameBits( const CanvasImage &a, const CanvasImage &b )
{
	return a.rgba.size() == b.rgba.size() &&
	       std::memcmp( a.rgba.data(), b.rgba.data(), a.rgba.size() * sizeof( float ) ) == 0;
}

// The pbr program's terms at a pixel of its quad, from pbr_brdf.h (the CPU
// copy of the BRDF) and the packed lighting and material: what
// cl_render_debug_brdf 1 (diffuse) and 2 (specular) must show.
struct PbrTerms
{
	Rgb diffuse;
	Rgb specular;
	Rgb splitSum; // A, B, 0
};

PbrTerms PbrOracle( const Lab &lab, const std::string &material, const std::string &lighting,
    float3 hit, bool compensate )
{
	const material::ModelLighting &l = lab.pbrLightingValues.at( lighting );
	const std::array<float, 4> &mrao = lab.pbrMrao.at( material );
	const Rgb base = lab.inputs.pbrBase;
	const float metal = std::clamp( mrao[0], 0.0f, 1.0f );
	const float rough = std::max( mrao[1], 0.02f );
	const float occlusion = std::clamp( mrao[2], 0.0f, 1.0f );
	const float3 view = math::Normalize( { l.eye[0] - hit.x, l.eye[1] - hit.y, l.eye[2] - hit.z } );
	const float3 n = math::Normalize( lab.inputs.normal );
	const float normalDotView = std::max( math::Dot( n, view ), 0.0f );
	const float baseChannels[3] = { base.r, base.g, base.b };
	float f0[3], compensation[3], albedo[3], diffuseColor[3];
	const pbr::SplitSumCoefficients split = pbr::SampleSplitSum( normalDotView, rough );
	for ( int c = 0; c < 3; ++c )
	{
		f0[c] = 0.04f + ( baseChannels[c] - 0.04f ) * metal;
		compensation[c] = compensate ? pbr::SpecularEnergyCompensation( f0[c], split ) : 1.0f;
		albedo[c] = compensate ? pbr::SpecularDirectionalAlbedo( f0[c], split )
		                       : std::min( 1.0f, f0[c] * split.a + split.b );
		diffuseColor[c] = baseChannels[c] * ( 1.0f - metal ) * ( 1.0f - albedo[c] );
	}
	const auto ambient = [&]( float3 d, int c )
	{
		const float sq[3] = { d.x * d.x, d.y * d.y, d.z * d.z };
		return sq[0] * l.cube[d.x >= 0 ? 0 : 1][c] + sq[1] * l.cube[d.y >= 0 ? 2 : 3][c] +
		       sq[2] * l.cube[d.z >= 0 ? 4 : 5][c];
	};
	float diffuse[3], specular[3];
	for ( int c = 0; c < 3; ++c )
		diffuse[c] = diffuseColor[c] * ambient( n, c ) * occlusion;
	for ( int c = 0; c < 3; ++c )
		specular[c] = 0.0f;
	const int count = int( l.eye[3] );
	for ( int i = 0; i < count && i < 4; ++i )
	{
		const auto &light = l.lights[i];
		const float3 toLight = light.color[3] > 0.5f
		                           ? math::Normalize( { -light.direction[0], -light.direction[1],
		                                 -light.direction[2] } )
		                           : math::Normalize( { light.position[0] - hit.x,
		                                 light.position[1] - hit.y, light.position[2] - hit.z } );
		const float normalDotLight = std::max( math::Dot( n, toLight ), 0.0f );
		if ( normalDotLight <= 0.0f )
			continue;
		const float3 half =
		    math::Normalize( { view.x + toLight.x, view.y + toLight.y, view.z + toLight.z } );
		const pbr::Color single = pbr::EvaluateSpecular( { f0[0], f0[1], f0[2] }, normalDotView,
		    normalDotLight, std::max( math::Dot( n, half ), 0.0f ),
		    std::max( math::Dot( view, half ), 0.0f ), rough );
		const float lobe[3] = { single.red, single.green, single.blue };
		for ( int c = 0; c < 3; ++c )
		{
			const float incident = light.color[c]; // a directional light's attenuation is 1
			diffuse[c] += diffuseColor[c] * incident * normalDotLight;
			specular[c] += pbr::kPi * incident * lobe[c] * compensation[c] * normalDotLight;
		}
	}
	const float d = math::Dot( view, n );
	const float3 reflected{
	    2.0f * d * n.x - view.x, 2.0f * d * n.y - view.y, 2.0f * d * n.z - view.z };
	for ( int c = 0; c < 3; ++c )
		specular[c] += ambient( reflected, c ) * albedo[c] * occlusion;
	return { { diffuse[0], diffuse[1], diffuse[2] }, { specular[0], specular[1], specular[2] },
	    { split.a, split.b, 0.0f } };
}

// A UV sphere of SurfaceModelVertex triangles.
std::vector<std::byte> SphereMesh( float3 centre, float radius )
{
	constexpr int kRings = 32, kSegments = 48;
	std::vector<std::byte> bytes;
	auto vertex = [&]( int ring, int segment )
	{
		const float theta = 3.14159265f * float( ring ) / float( kRings );
		const float phi = 2.0f * 3.14159265f * float( segment ) / float( kSegments );
		const float3 n{ std::sin( theta ) * std::cos( phi ), std::sin( theta ) * std::sin( phi ),
		    std::cos( theta ) };
		material::SurfaceModelVertex v;
		v.position[0] = centre.x + radius * n.x;
		v.position[1] = centre.y + radius * n.y;
		v.position[2] = centre.z + radius * n.z;
		v.normal[0] = n.x;
		v.normal[1] = n.y;
		v.normal[2] = n.z;
		v.tangent[0] = -std::sin( phi );
		v.tangent[1] = std::cos( phi );
		v.tangent[3] = 1.0f;
		v.uv[0] = float( segment ) / kSegments;
		v.uv[1] = float( ring ) / kRings;
		const auto b = Bytes( v );
		bytes.insert( bytes.end(), b.begin(), b.end() );
	};
	for ( int ring = 0; ring < kRings; ++ring )
	{
		for ( int segment = 0; segment < kSegments; ++segment )
		{
			vertex( ring, segment );
			vertex( ring + 1, segment );
			vertex( ring + 1, segment + 1 );
			vertex( ring, segment );
			vertex( ring + 1, segment + 1 );
			vertex( ring, segment + 1 );
		}
	}
	return bytes;
}

// The sphere's pixels away from its silhouette: the view ray's cosine to the
// surface normal is at least 0.4.
std::vector<std::pair<std::uint32_t, std::uint32_t>> SpherePixels(
    const Camera &camera, float3 centre, float radius )
{
	std::vector<std::pair<std::uint32_t, std::uint32_t>> pixels;
	for ( std::uint32_t y = 0; y < kSize; ++y )
	{
		for ( std::uint32_t x = 0; x < kSize; ++x )
		{
			const float3 d = math::Normalize( camera.Hit( x, y ) );
			const float b = math::Dot( d, centre );
			const float c = math::Dot( centre, centre ) - radius * radius;
			const float disc = b * b - c;
			if ( disc <= 0.0f )
				continue;
			const float tHit = b - std::sqrt( disc );
			const float3 p{ d.x * tHit, d.y * tHit, d.z * tHit };
			const float3 n = math::Normalize( { p.x - centre.x, p.y - centre.y, p.z - centre.z } );
			if ( -math::Dot( n, d ) >= 0.4f )
				pixels.emplace_back( x, y );
		}
	}
	return pixels;
}

std::optional<std::string> LightingControlChecks( Lab &lab, Results &results )
{
	// Terms: each term off is the frame without that input, bit for bit, and
	// the term on differs from it (the check is not vacuous).
	struct TermCase
	{
		const char *name;
		Case off;
		Case without;
		Case on;
	};
	auto pbrCase = []( const char *material, const char *lighting, frame::DebugControls debug )
	{
		Case c = Make( Program::kPbr, debug );
		c.pbrMaterial = material;
		c.pbrLighting = lighting;
		return c;
	};
	Case zeroPage = Make( Program::kLightmapped );
	zeroPage.page = "lab:page:zero";
	const TermCase terms[] = {
	    { "term.baked.lightmapped-is-a-zero-page",
	        Make( Program::kLightmapped, TermsOff( shaderlib::kDebugTermBaked ) ), zeroPage,
	        Make( Program::kLightmapped ) },
	    { "term.emission.selfillum-is-tint-zero",
	        Make( Program::kSelfIllum, TermsOff( shaderlib::kDebugTermEmission ) ),
	        Make( Program::kSelfIllumZero ), Make( Program::kSelfIllum ) },
	    { "term.ibl.envmapped-is-without-envmap",
	        Make( Program::kEnvmapped, TermsOff( shaderlib::kDebugTermIbl ) ),
	        Make( Program::kLightmapped ), Make( Program::kEnvmapped ) },
	    { "term.clustered.pbr-is-without-lights",
	        pbrCase( "default", "default", TermsOff( shaderlib::kDebugTermClustered ) ),
	        pbrCase( "default", "no-lights", {} ), pbrCase( "default", "default", {} ) },
	    { "term.probes-and-ibl.pbr-is-a-black-cube",
	        pbrCase( "default", "default",
	            TermsOff( shaderlib::kDebugTermProbes | shaderlib::kDebugTermIbl ) ),
	        pbrCase( "default", "cube-zero", {} ), pbrCase( "default", "default", {} ) },
	    { "term.ao.pbr-is-ao-one",
	        pbrCase( "default", "default", TermsOff( shaderlib::kDebugTermAo ) ),
	        pbrCase( "ao1", "default", {} ), pbrCase( "default", "default", {} ) },
	    { "term.emission.pbr-is-without-emission",
	        pbrCase( "emission", "default", TermsOff( shaderlib::kDebugTermEmission ) ),
	        pbrCase( "default", "default", {} ), pbrCase( "emission", "default", {} ) } };
	for ( const TermCase &term : terms )
	{
		CanvasImage off, without, on;
		if ( auto why = RenderOne( lab, term.off, off ) )
			return why;
		if ( auto why = RenderOne( lab, term.without, without ) )
			return why;
		if ( auto why = RenderOne( lab, term.on, on ) )
			return why;
		results.That( SameBits( off, without ), std::string( term.name ) + ".bitwise" );
		results.That( !SameBits( on, off ), std::string( term.name ) + ".the-term-contributes" );
	}

	// The BRDF modes against pbr_brdf.h, pixel by pixel.
	struct Mode
	{
		std::uint32_t brdf;
		const char *name;
	};
	CanvasImage full, diffuseOnly, specularOnly;
	for ( const Mode &mode : { Mode{ 0, "brdf.0.full" }, Mode{ 1, "brdf.1.diffuse-lobe" },
	          Mode{ 2, "brdf.2.specular-lobe" }, Mode{ 3, "brdf.3.no-energy-compensation" },
	          Mode{ 4, "brdf.4.split-sum-sample" } } )
	{
		frame::DebugControls debug;
		debug.brdf = mode.brdf;
		CanvasImage image;
		if ( auto why = RenderOne( lab, pbrCase( "default", "default", debug ), image ) )
			return why;
		if ( mode.brdf == 0 )
			full = image;
		if ( mode.brdf == 1 )
			diffuseOnly = image;
		if ( mode.brdf == 2 )
			specularOnly = image;
		std::string failure;
		std::size_t compared = 0;
		for ( auto [x, y] : lab.camera.Interior( kControlCell ) )
		{
			const PbrTerms oracle =
			    PbrOracle( lab, "default", "default", lab.camera.Hit( x, y ), mode.brdf != 3 );
			Rgb expected;
			if ( mode.brdf == 1 )
				expected = oracle.diffuse;
			else if ( mode.brdf == 2 )
				expected = oracle.specular;
			else if ( mode.brdf == 4 )
				expected = oracle.splitSum;
			else
				expected = { oracle.diffuse.r + oracle.specular.r,
				    oracle.diffuse.g + oracle.specular.g, oracle.diffuse.b + oracle.specular.b };
			const float *actual = image.At( x, y );
			++compared;
			const float tolerance = 3.0f * kTolerance;
			if ( failure.empty() && !( Near( expected.r, actual[0], tolerance ) &&
			                            Near( expected.g, actual[1], tolerance ) &&
			                            Near( expected.b, actual[2], tolerance ) ) )
				failure = "pixel (" + std::to_string( x ) + ", " + std::to_string( y ) +
				          "): " + Describe( expected, actual );
		}
		results.That( compared >= 8 && failure.empty(),
		    std::string( mode.name ) + ".matches-pbr_brdf.h", failure );
	}
	{
		// The lobes add up to the full frame.
		float worst = 0.0f;
		for ( auto [x, y] : lab.camera.Interior( kControlCell ) )
		{
			for ( int c = 0; c < 3; ++c )
				worst =
				    std::max( worst, std::fabs( diffuseOnly.At( x, y )[c] +
				                                specularOnly.At( x, y )[c] - full.At( x, y )[c] ) );
		}
		results.That( worst <= 2.0f * kTolerance, "brdf.lobes-sum-to-the-full-frame",
		    "worst " + std::to_string( worst ) );
	}

	// The furnace: an energy-compensated white metal sphere reads 1 at every
	// roughness; with compensation off (cl_render_debug_brdf 3) a rough one
	// reads below it. A white dielectric reads 1 too, as does a lightmapped
	// surface (albedo 1 under a uniform radiance of 1).
	const float3 centre{ kDepth, 0.0f, 0.0f };
	const float radius = 45.0f;
	const std::vector<std::byte> sphere = SphereMesh( centre, radius );
	const auto spherePixels = SpherePixels( lab.camera, centre, radius );
	auto furnace = [&]( float metal, float rough, std::uint32_t brdf, float &mean,
	                   float &worst ) -> std::optional<std::string>
	{
		frame::DebugControls debug;
		debug.furnace = true;
		debug.forceMetalness = metal;
		debug.forceRoughness = rough;
		debug.brdf = brdf;
		Case c = pbrCase( "ao1", "default", debug );
		c.mesh = sphere;
		CanvasImage image;
		if ( auto why = RenderOne( lab, c, image ) )
			return why;
		double sum = 0.0;
		worst = 0.0f;
		for ( auto [x, y] : spherePixels )
		{
			for ( int ch = 0; ch < 3; ++ch )
			{
				sum += image.At( x, y )[ch];
				worst = std::max( worst, std::fabs( image.At( x, y )[ch] - 1.0f ) );
			}
		}
		mean = spherePixels.empty() ? 0.0f : float( sum / double( spherePixels.size() * 3 ) );
		return std::nullopt;
	};
	results.That( spherePixels.size() > 2000, "furnace.sphere-covers-the-frame",
	    std::to_string( spherePixels.size() ) + " pixels" );
	for ( float rough : { 0.05f, 0.3f, 0.6f, 1.0f } )
	{
		float mean = 0, worst = 0;
		if ( auto why = furnace( 1.0f, rough, 0, mean, worst ) )
			return why;
		char name[96];
		std::snprintf(
		    name, sizeof( name ), "furnace.white-metal-reads-one.roughness-%.2f", rough );
		results.That( worst <= 2.0f * kTolerance, name,
		    "mean " + std::to_string( mean ) + ", worst " + std::to_string( worst ) );
	}
	{
		float mean = 0, worst = 0;
		if ( auto why = furnace( 0.0f, 0.5f, 0, mean, worst ) )
			return why;
		results.That( worst <= 2.0f * kTolerance, "furnace.white-dielectric-reads-one",
		    "mean " + std::to_string( mean ) + ", worst " + std::to_string( worst ) );
		if ( auto why = furnace( 1.0f, 1.0f, 3, mean, worst ) )
			return why;
		results.That( mean < 0.98f, "furnace.no-compensation-loses-energy-when-rough",
		    "mean " + std::to_string( mean ) );
		float smoothMean = 0;
		if ( auto why = furnace( 1.0f, 0.05f, 3, smoothMean, worst ) )
			return why;
		results.That( smoothMean > mean, "furnace.no-compensation-loses-more-when-rougher",
		    std::to_string( smoothMean ) + " at 0.05, " + std::to_string( mean ) + " at 1" );
	}
	{
		frame::DebugControls debug;
		debug.furnace = true;
		CanvasImage image;
		if ( auto why = RenderOne( lab, Make( Program::kLightmapped, debug ), image ) )
			return why;
		float worst = 0.0f;
		for ( auto [x, y] : lab.camera.Interior( kControlCell ) )
			for ( int c = 0; c < 3; ++c )
				worst = std::max( worst, std::fabs( image.At( x, y )[c] - 1.0f ) );
		results.That( worst <= kTolerance, "furnace.lightmapped-reads-one",
		    "worst " + std::to_string( worst ) );
	}

	// The overrides: a forced value is the material authored with it.
	struct Override
	{
		const char *name;
		const char *authored;
		bool roughness;
	};
	for ( const Override &override : { Override{ "force.roughness-is-authored", "rough102", true },
	          Override{ "force.metalness-is-authored", "metal102", false } } )
	{
		frame::DebugControls debug;
		( override.roughness ? debug.forceRoughness : debug.forceMetalness ) = 102.0f / 255.0f;
		CanvasImage forced, authored;
		if ( auto why = RenderOne( lab, pbrCase( "default", "default", debug ), forced ) )
			return why;
		if ( auto why = RenderOne( lab, pbrCase( override.authored, "default", {} ), authored ) )
			return why;
		float worst = 0.0f;
		for ( std::size_t i = 0; i < forced.rgba.size(); ++i )
			worst = std::max( worst, std::fabs( forced.rgba[i] - authored.rgba[i] ) );
		results.That( worst <= 1.0f / 1024.0f, override.name, "worst " + std::to_string( worst ) );
	}
	return std::nullopt;
}

const Seeded kSeeded[] = {
    { "swapped-normal", spirv::kLightmappedSwappedNormal, "view.2.normal.lightmapped" },
    { "tone-maps", spirv::kLightmappedToneMaps, "view.8.baked.scale-2" },
    { "misses-nan", spirv::kLightmappedMissesNan, "view.16.nan" },
    { "no-hatch", spirv::kLightmappedNoHatch, "view.4.hatch.lightmapped" } };

enum class SuiteKind
{
	kDebugViews,
	kLightingControls
};

const Seeded kLightingSeeded[] = {
    { "term-ignored", spirv::kLightmappedTermIgnored, "term.baked.lightmapped-is-a-zero-page" } };

// One run of a suite's checks with the lightmapped program's module.
std::optional<std::string> RunOnce( bool validate, std::span<const std::uint32_t> module,
    Results &results, std::uint64_t &messages, SuiteKind kind )
{
	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( std::optional<std::string> why = CreateLabDevice( validate, counter, device ) )
		return why;
	{
		Lab lab( *device );
		if ( std::optional<std::string> why = Prepare( lab, module ) )
			return why;
		// A warm-up frame makes every group resident.
		if ( std::optional<std::string> why =
		         lab.canvas->Render( lab.textures, lab.groups, {}, { 0, 0, 0, 1 }, nullptr ) )
			return why;
		if ( kind == SuiteKind::kLightingControls )
		{
			if ( std::optional<std::string> why = LightingControlChecks( lab, results ) )
				return why;
		}
		else
		{
			ControlChecks( results );
			IdentityChecks( lab, results );
			const std::vector<Case> cases = PixelCases( lab );
			if ( std::optional<std::string> why = RunCases( lab, cases, results ) )
				return why;
			if ( std::optional<std::string> why = RelationalCases( lab, results ) )
				return why;
		}
		(void)device->WaitIdle();
	}
	device.reset();
	messages = counter.load();
	return std::nullopt;
}

} // namespace

int RunDebugViewsSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "debug-views", kSeeded,
	    []( bool validate, std::span<const std::uint32_t> module, Results &results,
	        std::uint64_t &messages )
	    {
		    return RunOnce( validate, module, results, messages, SuiteKind::kDebugViews );
	    } );
}

int RunLightingControlsSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "lighting-controls", kLightingSeeded,
	    []( bool validate, std::span<const std::uint32_t> module, Results &results,
	        std::uint64_t &messages )
	    {
		    return RunOnce( validate, module, results, messages, SuiteKind::kLightingControls );
	    } );
}

} // namespace render::lab
