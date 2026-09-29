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

// A check's outcome, named so the sensitivity runs can say which failed.
struct Outcome
{
	std::string name;
	bool passed = false;
	std::string detail;
};

class Results
{
public:
	void That( bool condition, std::string name, std::string detail = {} )
	{
		m_Outcomes.push_back( { std::move( name ), condition, std::move( detail ) } );
	}
	const std::vector<Outcome> &Outcomes() const { return m_Outcomes; }
	bool Failed( std::string_view prefix ) const
	{
		for ( const Outcome &outcome : m_Outcomes )
		{
			if ( !outcome.passed && std::string_view( outcome.name ).starts_with( prefix ) )
				return true;
		}
		return false;
	}
	std::size_t FailureCount() const
	{
		return std::size_t( std::count_if( m_Outcomes.begin(), m_Outcomes.end(),
		    []( const Outcome &o )
		    {
			    return !o.passed;
		    } ) );
	}

private:
	std::vector<Outcome> m_Outcomes;
};

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
bool StageConstant( resources::TextureCache &cache, const std::string &name, Format format,
    std::span<const std::byte> texel, bool cube = false )
{
	TextureDesc desc;
	desc.format = format;
	desc.width = desc.height = 4;
	if ( cube )
	{
		desc.dimension = TextureDimension::kCube;
		desc.depthOrLayers = 6;
	}
	std::vector<std::byte> pixels;
	for ( int i = 0; i < 16 * ( cube ? 6 : 1 ); ++i )
		pixels.insert( pixels.end(), texel.begin(), texel.end() );
	return cache.Stage( name, desc, pixels ).HasValue();
}

std::vector<std::byte> HalfTexel( float r, float g, float b, float a = 1.0f )
{
	const std::uint16_t half[4] = {
	    FloatToHalf( r ), FloatToHalf( g ), FloatToHalf( b ), FloatToHalf( a ) };
	return Bytes( half );
}

std::vector<std::byte> ByteTexel( int r, int g, int b, int a )
{
	const std::uint8_t texel[4] = {
	    std::uint8_t( r ), std::uint8_t( g ), std::uint8_t( b ), std::uint8_t( a ) };
	return Bytes( texel );
}

enum class Program
{
	kLightmapped, // LightmappedGeneric, the base texture
	kBumped,      // + $bumpmap
	kEnvmapped,   // + $envmap with $envmaptint
	kSelfIllum,   // + $selfillum with $selfillumtint
	kUnlit,       // UnlitGeneric with $vertexcolor
	kPbr,
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
	std::unique_ptr<material::PbrFamily> pbr;
	std::unique_ptr<material::VertexLitFamily> vertexLit;
	std::unique_ptr<pass::lines::LinesRenderer> lines;
	resources::TextureCache textures;
	material::GroupResidency groups;
	std::unique_ptr<Canvas> canvas;
	Camera camera;
	Inputs inputs;
	std::map<Program, material::ResolvedProgram> programs;
	std::map<std::string, std::uint64_t> drawGroups; // lightmap page -> group id
	std::uint64_t frameGroup = 0;
	std::uint64_t pbrFrameGroup = 0;
	std::uint64_t pbrViewGroup = 0;
	std::uint64_t vertexLitLighting = 0;
	std::uint64_t nextGroup = 1;
	std::map<Program, std::uint64_t> materialGroups;
	material::ProgramRequest pbrRequest;
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
	    StageConstant( cache, "lab/vl/base", Format::kRGBA8Srgb, ByteTexel( 100, 200, 50, 255 ) );
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
	          "lab:page:negative", "lab:page:bright" } )
	{
		const auto group = lab.resolver->DrawGroup( lab.programs[Program::kLightmapped], { page } );
		const std::uint64_t id = lab.nextGroup++;
		if ( !group || !lab.groups.Set( id, *group ) )
			return std::string( "a lightmap group was refused" );
		lab.drawGroups[page] = id;
	}

	// pbr: an ambient cube of 0.2 and one directional light.
	material::PbrClaim claim;
	claim.claimed = true;
	material::PbrTextures pbrTextures;
	pbrTextures.base = "lab/pbr/base";
	pbrTextures.mrao = "lab/pbr/mrao";
	pbrTextures.placeholder = "lab/pbr/mrao";
	auto pbrRequest = lab.pbr->Request( claim, pbrTextures );
	if ( !pbrRequest )
		return std::string( "the pbr program was refused" );
	lab.pbrRequest = pbrRequest.Value();
	lab.materialGroups[Program::kPbr] = lab.nextGroup++;
	lab.pbrFrameGroup = lab.nextGroup++;
	lab.pbrViewGroup = lab.nextGroup++;
	const float eye[3] = { 0, 0, 0 };
	const float cube[6][3] = { { 0.2f, 0.2f, 0.2f }, { 0.2f, 0.2f, 0.2f }, { 0.2f, 0.2f, 0.2f },
	    { 0.2f, 0.2f, 0.2f }, { 0.2f, 0.2f, 0.2f }, { 0.2f, 0.2f, 0.2f } };
	material::PbrLightDesc sun;
	sun.type = material::PbrLightType::kDirectional;
	sun.color[0] = 1.0f;
	sun.color[1] = 0.9f;
	sun.color[2] = 0.8f;
	sun.direction[0] = 0.8f;
	sun.direction[1] = -0.3f;
	sun.direction[2] = -0.52f;
	const material::PbrModelLighting lighting =
	    material::PackSourceModelLighting( eye, cube, std::span( &sun, 1 ) );
	if ( !lab.groups.Set( lab.materialGroups[Program::kPbr], lab.pbrRequest.material ) ||
	     !lab.groups.Set( lab.pbrFrameGroup, lab.pbr->FrameGroup( "lab/pbr/splitsum" ) ) ||
	     !lab.groups.Set( lab.pbrViewGroup, lab.pbr->ViewGroup( lighting ) ) )
		return std::string( "a pbr group was refused" );

	// vertexlit: the same lighting.
	material::VertexLitClaim vertexLitClaim;
	vertexLitClaim.claimed = true;
	std::copy( in.vertexLitColor, in.vertexLitColor + 3, vertexLitClaim.constants.color );
	auto vertexLitRequest = lab.vertexLit->Request( vertexLitClaim, "lab/vl/base" );
	if ( !vertexLitRequest )
		return std::string( "the vertexlit program was refused" );
	lab.vertexLitRequest = vertexLitRequest.Value();
	lab.materialGroups[Program::kVertexLit] = lab.nextGroup++;
	lab.vertexLitLighting = lab.nextGroup++;
	if ( !lab.groups.Set(
	         lab.materialGroups[Program::kVertexLit], lab.vertexLitRequest.material ) ||
	     !lab.groups.Set( lab.vertexLitLighting, lab.vertexLit->LightingGroup( lighting ) ) )
		return std::string( "a vertexlit group was refused" );
	return std::nullopt;
}

// The quad of one case as the program's vertex.
std::vector<std::byte> QuadVertices( const Lab &lab, Program program, int cell )
{
	const Inputs &in = lab.inputs;
	std::vector<std::byte> bytes;
	for ( const Corner &corner : QuadCorners( cell ) )
	{
		if ( program == Program::kPbr )
		{
			material::PbrVertex v;
			std::memcpy( v.position, &corner.position, sizeof( v.position ) );
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
			material::VertexLitVertex v;
			std::memcpy( v.position, &corner.position, sizeof( v.position ) );
			std::memcpy( v.normal, &in.normal, sizeof( v.normal ) );
			v.uv[0] = corner.u;
			v.uv[1] = corner.v;
			const auto b = Bytes( v );
			bytes.insert( bytes.end(), b.begin(), b.end() );
		}
		else
		{
			material::LightmappedSurfaceVertex v;
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
			auto pipeline = lab.pbr->DebugPipeline( lab.pbrRequest.pipeline, debug );
			if ( !pipeline )
				return std::string( "no pbr debug pipeline" );
			draw.pipeline = *pipeline;
			draw.groups[std::size_t( BindGroupRole::kFrame )] = group( lab.pbrFrameGroup );
			draw.groups[std::size_t( BindGroupRole::kView )] = group( lab.pbrViewGroup );
			draw.groups[std::size_t( BindGroupRole::kMaterial )] =
			    group( lab.materialGroups[Program::kPbr] );
			draw.constants = Bytes( constants );
		}
		else if ( c.program == Program::kVertexLit )
		{
			auto pipeline = lab.vertexLit->DebugPipeline( lab.vertexLitRequest.pipeline, debug );
			if ( !pipeline )
				return std::string( "no vertexlit debug pipeline" );
			draw.pipeline = *pipeline;
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
			draw.groups[std::size_t( BindGroupRole::kMaterial )] =
			    group( lab.materialGroups.at( c.program ) );
			draw.groups[std::size_t( BindGroupRole::kDraw )] = group( lab.drawGroups.at( c.page ) );
			material::LightmappedDrawConstants lightmapped;
			std::memcpy( lightmapped.toClip, constants.toClip, sizeof( lightmapped.toClip ) );
			draw.constants = Bytes( lightmapped );
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
	material::LightmappedConstants lightmapped;
	std::memcpy( &lightmapped,
	    lab.programs.at( Program::kLightmapped ).request.material.constants.data(),
	    sizeof( lightmapped ) );
	material::LightmappedConstants envmapped;
	std::memcpy( &envmapped,
	    lab.programs.at( Program::kEnvmapped ).request.material.constants.data(),
	    sizeof( envmapped ) );
	material::LightmappedConstants selfIllum;
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
	results.That( status( View( 24 ) ) == int( DebugControlsStatus::kUnknownView ),
	    "controls.reserved-number-24-rejected" );
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
	controls.termsOff = 1u << 12;
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
	results.That(
	    pbr && *pbr == lab.pbrRequest.pipeline, "identity.neutral-is-the-shipped-pbr-pipeline" );
	auto vertexLit = lab.vertexLit->DebugPipeline( lab.vertexLitRequest.pipeline, {} );
	results.That( vertexLit && *vertexLit == lab.vertexLitRequest.pipeline,
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

struct Seeded
{
	const char *name;
	std::span<const std::uint32_t> module;
	const char *breaks; // the check prefix the defect must fail
};

const Seeded kSeeded[] = {
    { "swapped-normal", spirv::kLightmappedSwappedNormal, "view.2.normal.lightmapped" },
    { "tone-maps", spirv::kLightmappedToneMaps, "view.8.baked.scale-2" },
    { "misses-nan", spirv::kLightmappedMissesNan, "view.16.nan" },
    { "no-hatch", spirv::kLightmappedNoHatch, "view.4.hatch.lightmapped" } };

// One run of every check with the lightmapped program's module.
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
		// A warm-up frame makes every group resident.
		if ( std::optional<std::string> why =
		         lab.canvas->Render( lab.textures, lab.groups, {}, { 0, 0, 0, 1 }, nullptr ) )
			return why;
		ControlChecks( results );
		IdentityChecks( lab, results );
		const std::vector<Case> cases = PixelCases( lab );
		if ( std::optional<std::string> why = RunCases( lab, cases, results ) )
			return why;
		if ( std::optional<std::string> why = RelationalCases( lab, results ) )
			return why;
		(void)device->WaitIdle();
	}
	device.reset();
	messages = counter.load();
	return std::nullopt;
}

} // namespace

int RunDebugViewsSuite( int argc, char **argv )
{
	bool validate = false;
	bool sensitivity = false;
	std::string seeded;
	for ( int i = 0; i < argc; ++i )
	{
		const std::string arg = argv[i];
		if ( arg == "--validate" )
			validate = true;
		else if ( arg == "--sensitivity" )
			sensitivity = true;
		else if ( arg == "--seeded" && i + 1 < argc )
			seeded = argv[++i];
		else
		{
			std::fprintf(
			    stderr, "render_lab suite debug-views: unknown option %s\n", arg.c_str() );
			return 2;
		}
	}
	unsigned long checks = 0;
	unsigned long failures = 0;
	auto report = [&]( const Results &results, const char *prefix )
	{
		for ( const Outcome &outcome : results.Outcomes() )
		{
			++checks;
			if ( !outcome.passed )
			{
				++failures;
				std::fprintf( stderr, "FAIL %s%s%s%s\n", prefix, outcome.name.c_str(),
				    outcome.detail.empty() ? "" : ": ", outcome.detail.c_str() );
			}
		}
	};
	auto fail = [&]( const std::string &why )
	{
		std::fprintf( stderr, "FAIL render_lab debug-views: %s\n", why.c_str() );
		return testing::ReportConformance( checks + 1, failures + 1 );
	};

	if ( !sensitivity )
	{
		std::span<const std::uint32_t> module;
		if ( !seeded.empty() )
		{
			const Seeded *found = nullptr;
			for ( const Seeded &s : kSeeded )
				found = seeded == s.name ? &s : found;
			if ( !found )
				return fail( "no seeded defect " + seeded );
			module = found->module;
		}
		Results results;
		std::uint64_t messages = 0;
		if ( std::optional<std::string> why = RunOnce( validate, module, results, messages ) )
			return fail( *why );
		if ( validate )
			results.That( messages == 0, "validation.silent",
			    std::to_string( messages ) + " validation messages" );
		report( results, "" );
		std::printf( "render_lab debug-views: %zu checks, %zu failed\n", results.Outcomes().size(),
		    results.FailureCount() );
		return testing::ReportConformance( checks, failures );
	}

	// Sensitivity: the control passes, and each seeded program fails the
	// checks its defect breaks.
	Results control;
	std::uint64_t messages = 0;
	if ( std::optional<std::string> why = RunOnce( validate, {}, control, messages ) )
		return fail( *why );
	Results verdicts;
	verdicts.That( control.FailureCount() == 0, "sensitivity.control-passes",
	    std::to_string( control.FailureCount() ) + " control failures" );
	for ( const Seeded &s : kSeeded )
	{
		Results results;
		if ( std::optional<std::string> why = RunOnce( validate, s.module, results, messages ) )
			return fail( *why );
		verdicts.That( results.Failed( s.breaks ), std::string( "sensitivity.detects." ) + s.name,
		    std::string( "no failure of " ) + s.breaks );
	}
	report( verdicts, "" );
	return testing::ReportConformance( checks, failures );
}

} // namespace render::lab
