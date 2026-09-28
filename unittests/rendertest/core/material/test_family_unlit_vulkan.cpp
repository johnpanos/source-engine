//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.family.unlit (RFC 0016 K4 "Families match ports", the
//			`unlit` family) on render.device.vulkan.
//
//			Each case of quality/fixtures/legacy-shaders/families/unlit.vdf
//			(an UnlitGeneric material, its textures and a clip-space quad) is
//			imported with render.material's VMT importer into an `unlit`
//			parameter block, claimed by the family (ClaimUnlit) and drawn with
//			the family's pipeline into a 256x256 sRGB target, as the material
//			pixel harness draws it. The sampled pixels must match the legacy
//			port's within kTolerance levels per channel: the port's pixels are
//			the versioned fixture quality/fixtures/render-families/
//			unlit-port-v1.vdf, recorded from a legacy_shader_conformance.py run
//			that judged the port against the retail D3D9 bytecode
//			(tools/render/family_port_pixels.py). The fixture must name the
//			case file with its current sha256, so a changed case needs a new
//			recording. With the Khronos validation layer installed, the run
//			must report no message.
//
//			Seeded defect (sensitivity row): RENDER_MATERIAL_UNLIT_SEEDED_
//			IGNORE_VERTEX_COLOR (unlit_family.cpp) packs $vertexcolor as off.
//
//=============================================================================//

#include "kvtext/keyvalues.h"
#include "render/device/vulkan/provider.h"
#include "render/graph/executor.h"
#include "render/graph/graph_builder.h"
#include "render/material/registry.h"
#include "render/material/unlit_family.h"
#include "render/material/vmt_import.h"
#include "render/material/vmt_mapping.h"
#include "testing/checks.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace
{

using namespace render;
using namespace render::material;
namespace vulkan = render::device::vulkan;

constexpr std::uint32_t kSize = 256; // the material pixel harness's target
constexpr int kTolerance = 2;        // levels per channel, against the port
const char *const kCaseFile = "quality/fixtures/legacy-shaders/families/unlit.vdf";
const char *const kFixture = "quality/fixtures/render-families/unlit-port-v1.vdf";

std::string ReadFile( const std::string &path )
{
	std::ifstream file( path, std::ios::binary );
	std::stringstream text;
	text << file.rdbuf();
	return text.str();
}

std::vector<float> Numbers( const std::string *text )
{
	std::vector<float> numbers;
	if ( !text )
		return numbers;
	std::istringstream stream( *text );
	float value = 0.0f;
	while ( stream >> value )
		numbers.push_back( value );
	return numbers;
}

// FIPS 180-4 SHA-256, for the fixture's case-file digest.
std::string Sha256( const std::string &data )
{
	static const std::uint32_t k[64] = { 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b,
	    0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6,
	    0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d,
	    0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, 0x27b70a85,
	    0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585,
	    0x106aa070, 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
	    0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa,
	    0xa4506ceb, 0xbef9a3f7, 0xc67178f2 };
	std::uint32_t h[8] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c,
	    0x1f83d9ab, 0x5be0cd19 };
	std::string message = data;
	const std::uint64_t bits = std::uint64_t( data.size() ) * 8;
	message.push_back( char( 0x80 ) );
	while ( message.size() % 64 != 56 )
		message.push_back( 0 );
	for ( int i = 7; i >= 0; --i )
		message.push_back( char( ( bits >> ( i * 8 ) ) & 0xff ) );
	auto rotr = []( std::uint32_t x, int n )
	{
		return ( x >> n ) | ( x << ( 32 - n ) );
	};
	for ( std::size_t chunk = 0; chunk < message.size(); chunk += 64 )
	{
		std::uint32_t w[64];
		for ( int i = 0; i < 16; ++i )
		{
			w[i] = 0;
			for ( int b = 0; b < 4; ++b )
				w[i] = ( w[i] << 8 ) | std::uint8_t( message[chunk + i * 4 + b] );
		}
		for ( int i = 16; i < 64; ++i )
		{
			const std::uint32_t s0 =
			    rotr( w[i - 15], 7 ) ^ rotr( w[i - 15], 18 ) ^ ( w[i - 15] >> 3 );
			const std::uint32_t s1 =
			    rotr( w[i - 2], 17 ) ^ rotr( w[i - 2], 19 ) ^ ( w[i - 2] >> 10 );
			w[i] = w[i - 16] + s0 + w[i - 7] + s1;
		}
		std::uint32_t a[8];
		std::copy( h, h + 8, a );
		for ( int i = 0; i < 64; ++i )
		{
			const std::uint32_t s1 = rotr( a[4], 6 ) ^ rotr( a[4], 11 ) ^ rotr( a[4], 25 );
			const std::uint32_t ch = ( a[4] & a[5] ) ^ ( ~a[4] & a[6] );
			const std::uint32_t t1 = a[7] + s1 + ch + k[i] + w[i];
			const std::uint32_t s0 = rotr( a[0], 2 ) ^ rotr( a[0], 13 ) ^ rotr( a[0], 22 );
			const std::uint32_t maj = ( a[0] & a[1] ) ^ ( a[0] & a[2] ) ^ ( a[1] & a[2] );
			const std::uint32_t t2 = s0 + maj;
			std::copy_backward( a, a + 7, a + 8 );
			a[4] += t1;
			a[0] = t1 + t2;
		}
		for ( int i = 0; i < 8; ++i )
			h[i] += a[i];
	}
	char hex[65];
	for ( int i = 0; i < 8; ++i )
		std::snprintf( hex + i * 8, 9, "%08x", h[i] );
	return std::string( hex, 64 );
}

struct CaseTexture
{
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	bool clamp = false;
	std::vector<std::uint8_t> texels;
};

struct PortPixel
{
	int x = 0;
	int y = 0;
	int rgba[4] = {};
};

// A check whose failure prints why.
bool That(
    testing::Checks &checks, bool condition, const std::string &what, const std::string &detail )
{
	if ( !condition && !detail.empty() )
		std::printf( "INFO %s: %s\n", what.c_str(), detail.c_str() );
	return checks.That( condition, what );
}

bool Wait( device::IRenderDevice2 &device, device::CompletionToken token )
{
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 20 );
	while ( !device.IsComplete( token ) )
	{
		if ( std::chrono::steady_clock::now() > deadline )
			return false;
		(void)device.Poll();
		std::this_thread::yield();
	}
	(void)device.Poll();
	return true;
}

float SrgbToLinear( float value )
{
	return value <= 0.04045f ? value / 12.92f : std::pow( ( value + 0.055f ) / 1.055f, 2.4f );
}

std::string Normalized( std::string name )
{
	std::transform( name.begin(), name.end(), name.begin(),
	    []( unsigned char c )
	    {
		    return char( std::tolower( c ) );
	    } );
	std::replace( name.begin(), name.end(), '\\', '/' );
	if ( name.rfind( "materials/", 0 ) == 0 )
		name = name.substr( 10 );
	if ( name.size() > 4 && name.compare( name.size() - 4, 4, ".vtf" ) == 0 )
		name.resize( name.size() - 4 );
	return name;
}

struct Drawn
{
	bool ok = false;
	std::vector<std::uint8_t> rgba; // kSize x kSize, row 0 at the top
};

// One case: its texture, constants and quad, drawn with the family.
Drawn DrawCase( device::IRenderDevice2 &device, UnlitFamily &family, const UnlitClaim &claim,
    const CaseTexture &texture, const std::vector<UnlitVertex> &quad, const int clear[4] )
{
	Drawn drawn;
	using device::ResourceUsage;
	device::TextureDesc baseDesc;
	baseDesc.format = device::Format::kRGBA8Srgb;
	baseDesc.width = texture.width;
	baseDesc.height = texture.height;
	baseDesc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	device::TextureDesc colorDesc;
	colorDesc.format = device::Format::kRGBA8Srgb;
	colorDesc.width = colorDesc.height = kSize;
	colorDesc.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kCopySource };
	device::BufferDesc stagingDesc;
	stagingDesc.size = texture.texels.size();
	stagingDesc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource };
	device::BufferDesc constantsDesc;
	constantsDesc.size = sizeof( UnlitConstants );
	constantsDesc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kUniform };
	device::BufferDesc verticesDesc;
	verticesDesc.size = quad.size() * sizeof( UnlitVertex );
	verticesDesc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kVertex };
	device::BufferDesc readbackDesc;
	readbackDesc.size = kSize * kSize * 4;
	readbackDesc.usages = { ResourceUsage::kCopyDestination };
	readbackDesc.memory = device::MemoryKind::kReadback;
	device::SamplerDesc samplerDesc;
	samplerDesc.address =
	    texture.clamp ? device::AddressMode::kClampToEdge : device::AddressMode::kRepeat;

	auto base = device.CreateTexture( baseDesc );
	auto color = device.CreateTexture( colorDesc );
	auto staging = device.CreateBuffer( stagingDesc );
	auto constants = device.CreateBuffer( constantsDesc );
	auto vertices = device.CreateBuffer( verticesDesc );
	auto readback = device.CreateBuffer( readbackDesc );
	auto sampler = device.CreateSampler( samplerDesc );
	auto pipeline = family.Pipeline( claim );
	if ( base && color && staging && constants && vertices && readback && sampler && pipeline )
	{
		const device::BindGroupEntry entries[] = {
		    { 0, constants.Value(), 0, sizeof( UnlitConstants ), {}, {} },
		    { 1, {}, 0, 0, base.Value(), {} }, { 2, {}, 0, 0, {}, sampler.Value() } };
		auto group = device.CreateBindGroup( { family.MaterialLayout(), entries } );
		graph::GraphBuilder builder;
		auto import = [&builder]( const char *name, device::BufferId id,
		                  const device::BufferDesc &desc, ResourceUsage final )
		{
			return builder.ImportBuffer( name, id, desc, ResourceUsage::kUndefined, final );
		};
		const graph::ResourceRef stagingRef =
		    import( "staging", staging.Value(), stagingDesc, ResourceUsage::kCopySource );
		const graph::ResourceRef constantsRef =
		    import( "constants", constants.Value(), constantsDesc, ResourceUsage::kUniform );
		const graph::ResourceRef verticesRef =
		    import( "vertices", vertices.Value(), verticesDesc, ResourceUsage::kVertex );
		const graph::ResourceRef readbackRef =
		    import( "readback", readback.Value(), readbackDesc, ResourceUsage::kCopyDestination );
		const graph::ResourceRef baseRef = builder.ImportTexture(
		    "base", base.Value(), baseDesc, ResourceUsage::kUndefined, ResourceUsage::kSampled );
		const graph::ResourceRef colorRef = builder.ImportTexture( "color", color.Value(),
		    colorDesc, ResourceUsage::kUndefined, ResourceUsage::kCopySource );
		builder.AddPass( "upload", graph::PassKind::kCopy )
		    .Write( stagingRef, ResourceUsage::kCopyDestination )
		    .Write( constantsRef, ResourceUsage::kCopyDestination )
		    .Write( verticesRef, ResourceUsage::kCopyDestination )
		    .Execute(
		        [&]( graph::RecordContext &context )
		        {
			        device::CommandEncoder &encoder = context.Encoder();
			        encoder.WriteBuffer( context.Buffer( stagingRef ), 0,
			            std::as_bytes( std::span( texture.texels ) ) );
			        encoder.WriteBuffer( context.Buffer( constantsRef ), 0,
			            std::as_bytes( std::span( &claim.constants, 1 ) ) );
			        encoder.WriteBuffer(
			            context.Buffer( verticesRef ), 0, std::as_bytes( std::span( quad ) ) );
		        } );
		builder.AddPass( "texture", graph::PassKind::kCopy )
		    .Read( stagingRef, ResourceUsage::kCopySource )
		    .Write( baseRef, ResourceUsage::kCopyDestination )
		    .Execute(
		        [&]( graph::RecordContext &context )
		        {
			        context.Encoder().CopyBufferToTexture( context.Buffer( stagingRef ),
			            context.Texture( baseRef ), { 0, 0, 0, texture.width, texture.height } );
		        } );
		builder.AddPass( "draw", graph::PassKind::kRender )
		    .Read( baseRef, ResourceUsage::kSampled )
		    .Read( constantsRef, ResourceUsage::kUniform )
		    .Read( verticesRef, ResourceUsage::kVertex )
		    .Write( colorRef, ResourceUsage::kColorAttachment )
		    .Execute(
		        [&]( graph::RecordContext &context )
		        {
			        device::CommandEncoder &encoder = context.Encoder();
			        device::ColorAttachment attachment;
			        attachment.texture = context.Texture( colorRef );
			        attachment.clear = { SrgbToLinear( clear[0] / 255.0f ),
			            SrgbToLinear( clear[1] / 255.0f ), SrgbToLinear( clear[2] / 255.0f ),
			            clear[3] / 255.0f };
			        const device::ColorAttachment attachments[] = { attachment };
			        device::RenderingDesc rendering;
			        rendering.colors = attachments;
			        rendering.width = rendering.height = kSize;
			        encoder.BeginRendering( rendering );
			        encoder.SetViewport(
			            { 0.0f, 0.0f, float( kSize ), float( kSize ), 0.0f, 1.0f } );
			        encoder.SetPipeline( pipeline.Value() );
			        if ( group )
				        encoder.SetBindGroup( device::BindGroupRole::kMaterial, group.Value() );
			        encoder.SetVertexBuffer( 0, context.Buffer( verticesRef ) );
			        // The cases are clip-space quads in D3D9's convention, where
			        // pixel centers sit on integer coordinates: the half-pixel
			        // shift the legacy frontend applies to a D3D9 transform
			        // (content right and down by half a pixel), then the identity.
			        UnlitDrawConstants draw;
			        for ( int i = 0; i < 4; ++i )
				        draw.toClip[i * 5] = 1.0f;
			        draw.toClip[3] = 1.0f / float( kSize );
			        draw.toClip[7] = -1.0f / float( kSize );
			        encoder.SetDrawConstants( 0, std::as_bytes( std::span( &draw, 1 ) ) );
			        encoder.Draw( std::uint32_t( quad.size() ) );
			        encoder.EndRendering();
		        } );
		builder.AddPass( "readback", graph::PassKind::kCopy )
		    .Read( colorRef, ResourceUsage::kCopySource )
		    .Write( readbackRef, ResourceUsage::kCopyDestination )
		    .SideEffect()
		    .Execute(
		        [&]( graph::RecordContext &context )
		        {
			        context.Encoder().CopyTextureToBuffer( context.Texture( colorRef ),
			            context.Buffer( readbackRef ), { 0, 0, 0, kSize, kSize } );
		        } );
		auto compiled = graph::CompileGraph( std::move( builder ) );
		if ( group && compiled )
		{
			graph::SerialGraphExecutor executor;
			auto executed = executor.Execute( compiled.Value(), device );
			if ( executed && Wait( device, executed.Value().token ) )
			{
				drawn.rgba.resize( kSize * kSize * 4 );
				drawn.ok = device
				               .ReadBuffer( readback.Value(), 0,
				                   std::as_writable_bytes( std::span<std::uint8_t>( drawn.rgba ) ) )
				               .HasValue();
			}
		}
		if ( group )
			(void)device.Release( group.Value(), device::CompletionToken() );
	}
	for ( device::ResourceId id : { device::ResourceId( base.Value() ),
	          device::ResourceId( color.Value() ), device::ResourceId( staging.Value() ),
	          device::ResourceId( constants.Value() ), device::ResourceId( vertices.Value() ),
	          device::ResourceId( readback.Value() ), device::ResourceId( sampler.Value() ) } )
		(void)device.Release( id, device::CompletionToken() );
	(void)device.Poll();
	return drawn;
}

} // namespace

int main()
{
	testing::Checks checks;
	const std::string caseText = ReadFile( kCaseFile );
	const kvtext::ParseResult cases = kvtext::ParseKeyValues( caseText );
	const kvtext::ParseResult fixture = kvtext::ParseKeyValues( ReadFile( kFixture ) );
	if ( !checks.That( cases.ok && !cases.root.children.empty(), "setup.case-file-parses" ) ||
	     !checks.That( fixture.ok && !fixture.root.children.empty(), "setup.fixture-parses" ) )
		return checks.Report();
	const kvtext::KeyValueNode &caseRoot = cases.root.children[0];
	const kvtext::KeyValueNode &fixtureRoot = fixture.root.children[0];
	const std::string *digest = fixtureRoot.Find( "case_file_sha256" );
	That( checks, digest && *digest == Sha256( caseText ), "fixture.records-the-current-case-file",
	    "record it again: tools/render/family_port_pixels.py" );

	std::map<std::string, CaseTexture> textures;
	for ( const kvtext::KeyValueNode &node : caseRoot.children )
	{
		if ( node.name != "texture" || !node.Find( "name" ) )
			continue;
		CaseTexture texture;
		texture.width = std::uint32_t( Numbers( node.Find( "width" ) ).at( 0 ) );
		texture.height = std::uint32_t( Numbers( node.Find( "height" ) ).at( 0 ) );
		texture.clamp = node.Find( "clamp" ) && *node.Find( "clamp" ) == "1";
		for ( float value : Numbers( node.Find( "texels" ) ) )
			texture.texels.push_back( std::uint8_t( value ) );
		textures[Normalized( *node.Find( "name" ) )] = texture;
	}
	std::map<std::string, std::vector<PortPixel>> port;
	for ( const kvtext::KeyValueNode &node : fixtureRoot.children )
	{
		if ( node.name != "case" || !node.Find( "name" ) )
			continue;
		for ( const kvtext::KeyValue &pair : node.pairs )
		{
			if ( pair.key != "pixel" )
				continue;
			const std::vector<float> n = Numbers( &pair.value );
			if ( n.size() == 6 )
				port[*node.Find( "name" )].push_back( { int( n[0] ), int( n[1] ),
				    { int( n[2] ), int( n[3] ), int( n[4] ), int( n[5] ) } } );
		}
	}

	FamilyRegistry registry;
	for ( const FamilyDesc &desc : FamiliesFromMapping( BuiltinVmtMapping() ) )
		(void)registry.Register( desc );
	const FamilySchema *unlit = registry.Find( "unlit" );
	if ( !checks.That( unlit != nullptr, "setup.unlit-family-registers" ) )
		return checks.Report();

	// The family refuses what it does not draw, naming it.
	{
		auto refused = [&]( const char *vmt, const char *named )
		{
			VmtImportContext context;
			auto imported = ImportVmt( vmt, context );
			if ( !imported )
				return false;
			ParameterBlock block( *unlit );
			if ( !ApplyValues( imported.Value(), block ) )
				return false;
			(void)block.SetTexture( "basetexture", device::TextureId( 1 ) );
			if ( const std::optional<std::size_t> index = unlit->IndexOf( named );
			    index && unlit->layout[*index].type == ParameterType::kTexture )
				(void)block.SetTexture( named, device::TextureId( 2 ) );
			const UnlitClaim claim = ClaimUnlit( block );
			return !claim.claimed && claim.reason.find( named ) != std::string::npos;
		};
		checks.That(
		    refused( "\"UnlitGeneric\" { \"$basetexture\" \"a\" \"$detail\" \"b\" }", "detail" ),
		    "claim.refuses-a-detail-texture-by-name" );
		checks.That(
		    refused( "\"UnlitGeneric\" { \"$basetexture\" \"a\" \"$envmapcontrast\" \"0.5\" }",
		        "envmapcontrast" ),
		    "claim.refuses-an-env-map-parameter-by-name" );
		checks.That( refused( "\"UnlitGeneric\" { \"$basetexture\" \"a\" \"$translucent\" \"1\" "
		                      "\"$additive\" \"1\" }",
		                 "additive" ),
		    "claim.refuses-translucent-additive" );
	}

	const bool layer = vulkan::ValidationLayerAvailable();
	std::atomic<std::uint64_t> messages{ 0 };
	vulkan::VulkanAdapterOptions options;
	options.validation = layer;
	options.validationCounter = &messages;
	if ( const char *adapter = std::getenv( "RENDER_VK_ADAPTER" ) )
		options.adapterIndex = std::atoi( adapter );
	int drawnCases = 0;
	{
		auto created = vulkan::Create( options );
		if ( !checks.That( created.HasValue(), "device.a-vulkan-device-is-created" ) )
			return checks.Report();
		std::unique_ptr<device::IRenderDevice2> device = std::move( created ).Value();
		auto family =
		    UnlitFamily::Create( *device, device::Format::kRGBA8Srgb, device::Format::kUnknown );
		if ( !checks.That( family.HasValue(), "family.creates" ) )
			return checks.Report();

		for ( const kvtext::KeyValueNode &node : caseRoot.children )
		{
			if ( node.name != "case" || !node.Find( "name" ) )
				continue;
			const std::string name = *node.Find( "name" );
			const kvtext::KeyValueNode *material = nullptr;
			for ( const kvtext::KeyValueNode &child : node.children )
			{
				if ( child.name == "material" )
					material = &child;
			}
			std::string vmt =
			    "\"" + ( node.Find( "shader" ) ? *node.Find( "shader" ) : "" ) + "\"\n{\n";
			if ( material )
			{
				for ( const kvtext::KeyValue &pair : material->pairs )
					vmt += "\t\"" + pair.key + "\" \"" + pair.value + "\"\n";
			}
			vmt += "}\n";
			VmtImportContext context;
			auto imported = ImportVmt( vmt, context );
			if ( !checks.That( imported.HasValue() && imported.Value().family == "unlit",
			         "import." + name + ".is-unlit" ) )
				continue;
			ParameterBlock block( *unlit );
			checks.That(
			    ApplyValues( imported.Value(), block ).HasValue(), "import." + name + ".applies" );
			const CaseTexture *texture = nullptr;
			for ( const MaterialValue &value : imported.Value().values )
			{
				if ( value.parameter == "basetexture" )
				{
					auto found = textures.find( Normalized( value.text ) );
					texture = found == textures.end() ? nullptr : &found->second;
				}
			}
			if ( !checks.That( texture != nullptr, "import." + name + ".base-texture-resolves" ) )
				continue;
			// A stand-in id: the claim only asks whether a texture is bound.
			(void)block.SetTexture( "basetexture", device::TextureId( 1 ) );
			const UnlitClaim claim = ClaimUnlit( block );
			if ( !That( checks, claim.claimed, "claim." + name, claim.reason ) )
				continue;

			std::vector<UnlitVertex> corners;
			for ( const kvtext::KeyValueNode &child : node.children )
			{
				if ( child.name != "vertex" )
					continue;
				UnlitVertex vertex;
				const std::vector<float> pos = Numbers( child.Find( "pos" ) );
				const std::vector<float> uv = Numbers( child.Find( "uv0" ) );
				const std::vector<float> rgba = Numbers( child.Find( "color" ) );
				for ( std::size_t i = 0; i < 3 && i < pos.size(); ++i )
					vertex.position[i] = pos[i];
				for ( std::size_t i = 0; i < 2 && i < uv.size(); ++i )
					vertex.uv[i] = uv[i];
				for ( std::size_t i = 0; i < 4 && i < rgba.size(); ++i )
					vertex.color[i] = std::uint8_t( rgba[i] );
				corners.push_back( vertex );
			}
			if ( !checks.That( corners.size() == 4, "case." + name + ".is-a-quad" ) )
				continue;
			const std::vector<UnlitVertex> quad = {
			    corners[0], corners[1], corners[2], corners[0], corners[2], corners[3] };
			int clear[4] = { 0, 0, 0, 255 };
			const std::vector<float> clearValues = Numbers( node.Find( "clear" ) );
			for ( std::size_t i = 0; i < 4 && i < clearValues.size(); ++i )
				clear[i] = int( clearValues[i] );

			const Drawn drawn = DrawCase( *device, *family.Value(), claim, *texture, quad, clear );
			if ( !checks.That( drawn.ok, "draw." + name ) )
				continue;
			++drawnCases;
			const std::vector<PortPixel> &expected = port[name];
			int worst = 0;
			std::string worstAt;
			for ( const PortPixel &pixel : expected )
			{
				const std::size_t i =
				    ( std::size_t( pixel.y ) * kSize + std::size_t( pixel.x ) ) * 4;
				for ( int c = 0; c < 4; ++c )
				{
					const int difference = std::abs( int( drawn.rgba[i + c] ) - pixel.rgba[c] );
					if ( difference > worst )
					{
						worst = difference;
						worstAt = std::to_string( pixel.x ) + "," + std::to_string( pixel.y ) +
						          " channel " + std::to_string( c ) + ": family " +
						          std::to_string( drawn.rgba[i + c] ) + ", port " +
						          std::to_string( pixel.rgba[c] );
					}
				}
			}
			std::printf( "INFO %s: %zu port pixels, worst difference %d\n", name.c_str(),
			    expected.size(), worst );
			checks.That( !expected.empty(), "fixture." + name + ".has-port-pixels" );
			That( checks, worst <= kTolerance, "pixels." + name + ".within-tolerance-of-the-port",
			    worstAt );
		}
		checks.That( drawnCases == 6, "cases.every-case-drew" );
		(void)device->WaitIdle();
	}
	if ( layer )
		checks.Equal( messages.load(), std::uint64_t( 0 ), "validation.no-messages" );
	else
		std::printf( "SKIP validation: the Khronos validation layer is not installed\n" );
	return checks.Report();
}
