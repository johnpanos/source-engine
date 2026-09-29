//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: What the render.family.* suites share (RFC 0016 K4); see
//			family_pixel_cases.h.
//
//=============================================================================//

#include "family_pixel_cases.h"

#include "kvtext/keyvalues.h"
#include "render/graph/executor.h"
#include "render/graph/graph_builder.h"
#include "render/material/vmt_mapping.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <thread>

namespace rendertest::families
{

namespace
{

using namespace render;

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

// A case's "light" block, with material_pixel_legacy.cpp's defaults
// (ApplyCaseLighting) for the keys it leaves out.
ModelLight CaseLight( const kvtext::KeyValueNode &node )
{
	ModelLight light;
	light.type = node.Find( "type" ) ? *node.Find( "type" ) : "point";
	const float white[3] = { 1.0f, 1.0f, 1.0f };
	const float down[3] = { 0.0f, 0.0f, -1.0f };
	const float constant[3] = { 1.0f, 0.0f, 0.0f };
	auto copy = [&node]( const char *key, float ( &out )[3], const float *fallback )
	{
		const std::vector<float> n = Numbers( node.Find( key ) );
		for ( std::size_t i = 0; i < 3; ++i )
			out[i] = i < n.size() ? n[i] : ( n.empty() && fallback ? fallback[i] : 0.0f );
	};
	copy( "color", light.color, white );
	copy( "position", light.position, nullptr );
	copy( "direction", light.direction, down );
	copy( "attenuation", light.attenuation, constant );
	auto scalar = [&node]( const char *key, float fallback )
	{
		const std::vector<float> n = Numbers( node.Find( key ) );
		return n.empty() ? fallback : n[0];
	};
	light.theta = scalar( "theta", 0.5f );
	light.phi = scalar( "phi", 1.0f );
	light.falloff = scalar( "falloff", 1.0f );
	return light;
}

} // namespace

std::optional<CaseSet> LoadCases(
    testing::Checks &checks, const char *caseFile, const char *fixtureFile )
{
	const std::string caseText = ReadFile( caseFile );
	const kvtext::ParseResult cases = kvtext::ParseKeyValues( caseText );
	const kvtext::ParseResult fixture = kvtext::ParseKeyValues( ReadFile( fixtureFile ) );
	if ( !checks.That( cases.ok && !cases.root.children.empty(), "setup.case-file-parses" ) ||
	     !checks.That( fixture.ok && !fixture.root.children.empty(), "setup.fixture-parses" ) )
		return std::nullopt;
	const kvtext::KeyValueNode &caseRoot = cases.root.children[0];
	const kvtext::KeyValueNode &fixtureRoot = fixture.root.children[0];
	const std::string *digest = fixtureRoot.Find( "case_file_sha256" );
	That( checks, digest && *digest == Sha256( caseText ), "fixture.records-the-current-case-file",
	    "record it again: tools/render/family_port_pixels.py" );

	CaseSet set;
	for ( const kvtext::KeyValueNode &node : caseRoot.children )
	{
		if ( node.name != "texture" || !node.Find( "name" ) )
			continue;
		CaseTexture texture;
		texture.width = std::uint32_t( Numbers( node.Find( "width" ) ).at( 0 ) );
		texture.height = std::uint32_t( Numbers( node.Find( "height" ) ).at( 0 ) );
		texture.clamp = node.Find( "clamp" ) && *node.Find( "clamp" ) == "1";
		texture.point = node.Find( "point" ) && *node.Find( "point" ) == "1";
		texture.cube = node.Find( "cube" ) && *node.Find( "cube" ) == "1";
		for ( float value : Numbers( node.Find( "texels" ) ) )
			texture.texels.push_back( std::uint8_t( value ) );
		set.textures[Normalized( *node.Find( "name" ) )] = texture;
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
	for ( const kvtext::KeyValueNode &node : caseRoot.children )
	{
		if ( node.name != "case" || !node.Find( "name" ) )
			continue;
		FamilyCase testCase;
		testCase.name = *node.Find( "name" );
		const kvtext::KeyValueNode *material = nullptr;
		std::vector<CaseVertex> corners;
		for ( const kvtext::KeyValueNode &child : node.children )
		{
			if ( child.name == "material" )
				material = &child;
			if ( child.name == "light" )
				testCase.lights.push_back( CaseLight( child ) );
			if ( child.name != "vertex" )
				continue;
			CaseVertex vertex;
			const std::vector<float> pos = Numbers( child.Find( "pos" ) );
			const std::vector<float> uv0 = Numbers( child.Find( "uv0" ) );
			const std::vector<float> uv1 = Numbers( child.Find( "uv1" ) );
			const std::vector<float> rgba = Numbers( child.Find( "color" ) );
			const std::vector<float> normal = Numbers( child.Find( "normal" ) );
			const std::vector<float> tangentS = Numbers( child.Find( "tangents" ) );
			const std::vector<float> tangentT = Numbers( child.Find( "tangentt" ) );
			const std::vector<float> uv2 = Numbers( child.Find( "uv2" ) );
			for ( std::size_t i = 0; i < 3 && i < pos.size(); ++i )
				vertex.position[i] = pos[i];
			for ( std::size_t i = 0; i < 3 && i < normal.size(); ++i )
				vertex.normal[i] = normal[i];
			for ( std::size_t i = 0; i < 3 && i < tangentS.size(); ++i )
				vertex.tangentS[i] = tangentS[i];
			for ( std::size_t i = 0; i < 3 && i < tangentT.size(); ++i )
				vertex.tangentT[i] = tangentT[i];
			for ( std::size_t i = 0; i < 2 && i < uv2.size(); ++i )
				vertex.uv2[i] = uv2[i];
			for ( std::size_t i = 0; i < 2 && i < uv0.size(); ++i )
				vertex.uv0[i] = uv0[i];
			for ( std::size_t i = 0; i < 2 && i < uv1.size(); ++i )
				vertex.uv1[i] = uv1[i];
			for ( std::size_t i = 0; i < 4 && i < rgba.size(); ++i )
				vertex.color[i] = std::uint8_t( rgba[i] );
			corners.push_back( vertex );
		}
		testCase.vmt = "\"" + ( node.Find( "shader" ) ? *node.Find( "shader" ) : "" ) + "\"\n{\n";
		if ( material )
		{
			for ( const kvtext::KeyValue &pair : material->pairs )
				testCase.vmt += "\t\"" + pair.key + "\" \"" + pair.value + "\"\n";
		}
		testCase.vmt += "}\n";
		if ( const std::string *lightmap = node.Find( "lightmap" ) )
			testCase.lightmap = FindTexture( set, *lightmap );
		if ( checks.That( corners.size() == 4, "case." + testCase.name + ".is-a-quad" ) )
			testCase.triangles = {
			    corners[0], corners[1], corners[2], corners[0], corners[2], corners[3] };
		const std::vector<float> clearValues = Numbers( node.Find( "clear" ) );
		for ( std::size_t i = 0; i < 4 && i < clearValues.size(); ++i )
			testCase.clear[i] = int( clearValues[i] );
		const std::vector<float> ambient = Numbers( node.Find( "ambient" ) );
		for ( std::size_t i = 0; i < 18 && i < ambient.size(); ++i )
			testCase.cube[i / 3][i % 3] = ambient[i];
		testCase.port = port[testCase.name];
		set.cases.push_back( std::move( testCase ) );
	}
	// FindTexture pointers stay valid: the map is complete before any case.
	return set;
}

const CaseTexture *FindTexture( const CaseSet &set, const std::string &name )
{
	return FindTexture( set.textures, name );
}

const CaseTexture *FindTexture(
    const std::map<std::string, CaseTexture> &textures, const std::string &name )
{
	auto found = textures.find( Normalized( name ) );
	return found == textures.end() ? nullptr : &found->second;
}

namespace
{

template <std::size_t N>
void CopyNumbers( const kvtext::KeyValueNode &node, const char *key, float ( &out )[N] )
{
	const std::vector<float> values = Numbers( node.Find( key ) );
	for ( std::size_t i = 0; i < N && i < values.size(); ++i )
		out[i] = values[i];
}

} // namespace

std::optional<ModelCaseSet> LoadModelCases( testing::Checks &checks, const char *fixtureFile )
{
	const kvtext::ParseResult fixture = kvtext::ParseKeyValues( ReadFile( fixtureFile ) );
	if ( !checks.That( fixture.ok && !fixture.root.children.empty(), "setup.fixture-parses" ) )
		return std::nullopt;
	const kvtext::KeyValueNode &root = fixture.root.children[0];
	ModelCaseSet set;
	set.quadZ = Numbers( root.Find( "quad_z" ) ).at( 0 );
	CopyNumbers( root, "eye", set.eye );
	int clear[4] = { 0, 0, 0, 255 };
	const std::vector<float> clearValues = Numbers( root.Find( "clear" ) );
	for ( std::size_t i = 0; i < 4 && i < clearValues.size(); ++i )
		clear[i] = int( clearValues[i] );
	std::map<std::string, std::string> materials; // name -> VMT text
	for ( const kvtext::KeyValueNode &node : root.children )
	{
		if ( node.name == "texture" && node.Find( "name" ) )
		{
			CaseTexture texture;
			texture.width = std::uint32_t( Numbers( node.Find( "width" ) ).at( 0 ) );
			texture.height = std::uint32_t( Numbers( node.Find( "height" ) ).at( 0 ) );
			for ( float value : Numbers( node.Find( "texels" ) ) )
				texture.texels.push_back( std::uint8_t( value ) );
			set.textures[Normalized( *node.Find( "name" ) )] = texture;
		}
		else if ( node.name == "material" && node.Find( "name" ) )
		{
			std::string vmt = "\"PBRMetalRough\"\n{\n";
			for ( const kvtext::KeyValue &pair : node.pairs )
			{
				if ( pair.key != "name" )
					vmt += "\t\"" + pair.key + "\" \"" + pair.value + "\"\n";
			}
			materials[*node.Find( "name" )] = vmt + "}\n";
		}
		else if ( node.name == "quad" )
		{
			ModelQuad quad;
			const std::vector<float> corners = Numbers( node.Find( "corners" ) );
			for ( std::size_t i = 0; i < 8 && i < corners.size(); ++i )
				quad.corners[i / 2][i % 2] = corners[i];
			CopyNumbers( node, "normal", quad.normal );
			CopyNumbers( node, "tangent", quad.tangent );
			set.quads.push_back( quad );
		}
		else if ( node.name == "case" && node.Find( "name" ) )
		{
			ModelCase modelCase;
			FamilyCase &common = modelCase.common;
			common.name = *node.Find( "name" );
			std::copy( clear, clear + 4, common.clear );
			const std::string *material = node.Find( "material" );
			auto found = material ? materials.find( *material ) : materials.end();
			if ( checks.That(
			         found != materials.end(), "case." + common.name + ".has-a-material" ) )
				common.vmt = found->second;
			CopyNumbers( node, "model_matrix", modelCase.modelMatrix );
			float cube[18] = {};
			CopyNumbers( node, "cube", cube );
			for ( int i = 0; i < 18; ++i )
				modelCase.cube[i / 3][i % 3] = cube[i];
			for ( const kvtext::KeyValueNode &child : node.children )
			{
				if ( child.name != "light" )
					continue;
				ModelLight light;
				light.type = child.Find( "type" ) ? *child.Find( "type" ) : "";
				CopyNumbers( child, "color", light.color );
				CopyNumbers( child, "position", light.position );
				CopyNumbers( child, "direction", light.direction );
				CopyNumbers( child, "attenuation", light.attenuation );
				light.theta = Numbers( child.Find( "theta" ) ).at( 0 );
				light.phi = Numbers( child.Find( "phi" ) ).at( 0 );
				light.falloff = Numbers( child.Find( "falloff" ) ).at( 0 );
				modelCase.lights.push_back( light );
			}
			for ( const kvtext::KeyValue &pair : node.pairs )
			{
				const std::vector<float> n =
				    pair.key == "pixel" ? Numbers( &pair.value ) : std::vector<float>();
				if ( n.size() == 5 )
					common.port.push_back( { int( n[0] ), int( n[1] ),
					    { int( n[2] ), int( n[3] ), int( n[4] ), 255 }, 3 } );
			}
			set.cases.push_back( std::move( modelCase ) );
		}
	}
	return set;
}

material::FamilyRegistry BuiltinFamilies()
{
	material::FamilyRegistry registry;
	for ( const material::FamilyDesc &desc :
	    material::FamiliesFromMapping( material::BuiltinVmtMapping() ) )
		(void)registry.Register( desc );
	return registry;
}

ImportedCase ImportCase(
    testing::Checks &checks, const FamilyCase &testCase, const material::FamilySchema &family )
{
	ImportedCase imported;
	material::VmtImportContext context;
	// A PBR material's $fallbackmaterial must resolve; other profiles draw
	// it, so the cases do not carry it.
	context.resolve = []( std::string_view ) -> std::optional<std::string>
	{
		return std::string();
	};
	auto desc = material::ImportVmt( testCase.vmt, context );
	if ( !checks.That( desc.HasValue() && desc.Value().family == family.desc.name,
	         "import." + testCase.name + ".is-" + family.desc.name ) )
		return imported;
	imported.material = desc.Value();
	imported.block.emplace( family );
	checks.That( material::ApplyValues( imported.material, *imported.block ).HasValue(),
	    "import." + testCase.name + ".applies" );
	for ( const material::MaterialValue &value : imported.material.values )
	{
		if ( value.kind == material::ValueKind::kTexture && !value.text.empty() )
			(void)imported.block->SetTexture( value.parameter, device::TextureId( 1 ) );
	}
	return imported;
}

bool That(
    testing::Checks &checks, bool condition, const std::string &what, const std::string &detail )
{
	if ( !condition && !detail.empty() )
		std::printf( "INFO %s: %s\n", what.c_str(), detail.c_str() );
	return checks.That( condition, what );
}

std::array<float, 16> CaseToClip()
{
	std::array<float, 16> toClip = {};
	for ( int i = 0; i < 4; ++i )
		toClip[i * 5] = 1.0f;
	toClip[3] = 1.0f / float( kSize );
	toClip[7] = -1.0f / float( kSize );
	return toClip;
}

Drawn DrawCase( device::IRenderDevice2 &device, const CaseDraw &draw )
{
	Drawn drawn;
	using device::ResourceUsage;
	struct GroupResources
	{
		device::BufferDesc constantsDesc;
		device::BufferId constants;
		std::vector<device::TextureDesc> textureDescs;
		std::vector<device::TextureId> textures;
		std::vector<device::BufferDesc> stagingDescs;
		std::vector<device::BufferId> staging;
		std::vector<device::SamplerId> samplers;
		device::BindGroupId group;
		graph::ResourceRef constantsRef;
		std::vector<graph::ResourceRef> textureRefs;
		std::vector<graph::ResourceRef> stagingRefs;
	};
	std::vector<GroupResources> groups( draw.groups.size() );
	std::vector<device::ResourceId> owned;
	bool ok = true;
	auto keep = [&]( auto created, auto &id )
	{
		if ( !created )
		{
			ok = false;
			return;
		}
		id = created.Value();
		owned.push_back( device::ResourceId( id ) );
	};

	device::TextureDesc colorDesc;
	colorDesc.format = device::Format::kRGBA8Srgb;
	colorDesc.width = colorDesc.height = kSize;
	colorDesc.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kCopySource };
	device::BufferDesc verticesDesc;
	verticesDesc.size = draw.vertices.size();
	verticesDesc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kVertex };
	device::BufferDesc readbackDesc;
	readbackDesc.size = kSize * kSize * 4;
	readbackDesc.usages = { ResourceUsage::kCopyDestination };
	readbackDesc.memory = device::MemoryKind::kReadback;
	device::TextureId color;
	device::BufferId vertices;
	device::BufferId readback;
	keep( device.CreateTexture( colorDesc ), color );
	keep( device.CreateBuffer( verticesDesc ), vertices );
	keep( device.CreateBuffer( readbackDesc ), readback );

	for ( std::size_t g = 0; g < draw.groups.size() && ok; ++g )
	{
		const CaseGroup &request = draw.groups[g];
		GroupResources &resources = groups[g];
		std::vector<device::BindGroupEntry> entries;
		std::uint32_t binding = 0;
		const auto addConstants = [&]
		{
			resources.constantsDesc.size = request.constants.size();
			resources.constantsDesc.usages = {
			    ResourceUsage::kCopyDestination, ResourceUsage::kUniform };
			keep( device.CreateBuffer( resources.constantsDesc ), resources.constants );
			entries.push_back(
			    { binding++, resources.constants, 0, request.constants.size(), {}, {} } );
		};
		if ( !request.constants.empty() && !request.constantsLast )
			addConstants();
		for ( const CaseTexture *texture : request.textures )
		{
			if ( !texture )
			{
				ok = false;
				break;
			}
			device::TextureDesc desc;
			desc.format = texture->format;
			desc.width = texture->width;
			desc.height = texture->height;
			if ( texture->cube )
			{
				desc.dimension = device::TextureDimension::kCube;
				desc.depthOrLayers = 6;
			}
			desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
			device::BufferDesc stagingDesc;
			stagingDesc.size = texture->texels.size();
			stagingDesc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource };
			device::SamplerDesc samplerDesc;
			samplerDesc.address =
			    texture->clamp ? device::AddressMode::kClampToEdge : device::AddressMode::kRepeat;
			if ( texture->point )
				samplerDesc.minFilter = samplerDesc.magFilter = samplerDesc.mipFilter =
				    device::Filter::kNearest;
			device::TextureId id;
			device::BufferId staging;
			device::SamplerId sampler;
			keep( device.CreateTexture( desc ), id );
			keep( device.CreateBuffer( stagingDesc ), staging );
			keep( device.CreateSampler( samplerDesc ), sampler );
			resources.textureDescs.push_back( desc );
			resources.textures.push_back( id );
			resources.stagingDescs.push_back( stagingDesc );
			resources.staging.push_back( staging );
			resources.samplers.push_back( sampler );
			entries.push_back( { binding++, {}, 0, 0, id, {} } );
			entries.push_back( { binding++, {}, 0, 0, {}, sampler } );
		}
		if ( !ok )
			break;
		if ( !request.constants.empty() && request.constantsLast )
			addConstants();
		auto group = device.CreateBindGroup( { request.layout, entries } );
		if ( !group )
		{
			ok = false;
			break;
		}
		resources.group = group.Value();
	}

	if ( ok )
	{
		graph::GraphBuilder builder;
		auto import = [&builder]( const char *name, device::BufferId id,
		                  const device::BufferDesc &desc, ResourceUsage final )
		{
			return builder.ImportBuffer( name, id, desc, ResourceUsage::kUndefined, final );
		};
		const graph::ResourceRef verticesRef =
		    import( "vertices", vertices, verticesDesc, ResourceUsage::kVertex );
		const graph::ResourceRef readbackRef =
		    import( "readback", readback, readbackDesc, ResourceUsage::kCopyDestination );
		const graph::ResourceRef colorRef = builder.ImportTexture(
		    "color", color, colorDesc, ResourceUsage::kUndefined, ResourceUsage::kCopySource );
		for ( GroupResources &resources : groups )
		{
			if ( resources.constants.IsValid() )
				resources.constantsRef = import( "constants", resources.constants,
				    resources.constantsDesc, ResourceUsage::kUniform );
			for ( std::size_t t = 0; t < resources.textures.size(); ++t )
			{
				resources.stagingRefs.push_back( import( "staging", resources.staging[t],
				    resources.stagingDescs[t], ResourceUsage::kCopySource ) );
				resources.textureRefs.push_back( builder.ImportTexture( "texture",
				    resources.textures[t], resources.textureDescs[t], ResourceUsage::kUndefined,
				    ResourceUsage::kSampled ) );
			}
		}
		{
			graph::PassBuilder upload = builder.AddPass( "upload", graph::PassKind::kCopy );
			upload.Write( verticesRef, ResourceUsage::kCopyDestination );
			for ( GroupResources &resources : groups )
			{
				if ( resources.constants.IsValid() )
					upload.Write( resources.constantsRef, ResourceUsage::kCopyDestination );
				for ( graph::ResourceRef ref : resources.stagingRefs )
					upload.Write( ref, ResourceUsage::kCopyDestination );
			}
			upload.Execute(
			    [&]( graph::RecordContext &context )
			    {
				    device::CommandEncoder &encoder = context.Encoder();
				    encoder.WriteBuffer( context.Buffer( verticesRef ), 0, draw.vertices );
				    for ( std::size_t g = 0; g < groups.size(); ++g )
				    {
					    if ( groups[g].constants.IsValid() )
						    encoder.WriteBuffer( context.Buffer( groups[g].constantsRef ), 0,
						        draw.groups[g].constants );
					    for ( std::size_t t = 0; t < groups[g].stagingRefs.size(); ++t )
						    encoder.WriteBuffer( context.Buffer( groups[g].stagingRefs[t] ), 0,
						        std::as_bytes( std::span( draw.groups[g].textures[t]->texels ) ) );
				    }
			    } );
		}
		{
			graph::PassBuilder textures = builder.AddPass( "textures", graph::PassKind::kCopy );
			for ( GroupResources &resources : groups )
			{
				for ( std::size_t t = 0; t < resources.textureRefs.size(); ++t )
				{
					textures.Read( resources.stagingRefs[t], ResourceUsage::kCopySource );
					textures.Write( resources.textureRefs[t], ResourceUsage::kCopyDestination );
				}
			}
			textures.Execute(
			    [&]( graph::RecordContext &context )
			    {
				    for ( std::size_t g = 0; g < groups.size(); ++g )
				    {
					    for ( std::size_t t = 0; t < groups[g].textureRefs.size(); ++t )
					    {
						    const CaseTexture &texture = *draw.groups[g].textures[t];
						    // A cube's faces follow one another in its texels.
						    const std::uint32_t layers = texture.cube ? 6 : 1;
						    const std::uint64_t faceBytes =
						        texture.texels.size() / std::max<std::uint32_t>( layers, 1 );
						    for ( std::uint32_t layer = 0; layer < layers; ++layer )
							    context.Encoder().CopyBufferToTexture(
							        context.Buffer( groups[g].stagingRefs[t] ),
							        context.Texture( groups[g].textureRefs[t] ),
							        { layer * faceBytes, 0, layer, texture.width,
							            texture.height } );
					    }
				    }
			    } );
		}
		{
			graph::PassBuilder pass = builder.AddPass( "draw", graph::PassKind::kRender );
			pass.Read( verticesRef, ResourceUsage::kVertex );
			for ( GroupResources &resources : groups )
			{
				if ( resources.constants.IsValid() )
					pass.Read( resources.constantsRef, ResourceUsage::kUniform );
				for ( graph::ResourceRef ref : resources.textureRefs )
					pass.Read( ref, ResourceUsage::kSampled );
			}
			pass.Write( colorRef, ResourceUsage::kColorAttachment );
			pass.Execute(
			    [&]( graph::RecordContext &context )
			    {
				    device::CommandEncoder &encoder = context.Encoder();
				    device::ColorAttachment attachment;
				    attachment.texture = context.Texture( colorRef );
				    attachment.clear = { SrgbToLinear( draw.clear[0] / 255.0f ),
				        SrgbToLinear( draw.clear[1] / 255.0f ),
				        SrgbToLinear( draw.clear[2] / 255.0f ), draw.clear[3] / 255.0f };
				    const device::ColorAttachment attachments[] = { attachment };
				    device::RenderingDesc rendering;
				    rendering.colors = attachments;
				    rendering.width = rendering.height = kSize;
				    encoder.BeginRendering( rendering );
				    encoder.SetViewport(
				        { 0.0f, 0.0f, float( kSize ), float( kSize ), 0.0f, 1.0f } );
				    encoder.SetPipeline( draw.pipeline );
				    for ( std::size_t g = 0; g < groups.size(); ++g )
					    encoder.SetBindGroup( draw.groups[g].role, groups[g].group );
				    encoder.SetVertexBuffer( 0, context.Buffer( verticesRef ) );
				    const std::array<float, 16> toClip = CaseToClip();
				    encoder.SetDrawConstants( 0, draw.drawConstants.empty()
				                                     ? std::as_bytes( std::span( toClip ) )
				                                     : draw.drawConstants );
				    encoder.Draw( draw.vertexCount );
				    encoder.EndRendering();
			    } );
		}
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
		if ( compiled )
		{
			graph::SerialGraphExecutor executor;
			auto executed = executor.Execute( compiled.Value(), device );
			if ( executed && Wait( device, executed.Value().token ) )
			{
				drawn.rgba.resize( kSize * kSize * 4 );
				drawn.ok = device
				               .ReadBuffer( readback, 0,
				                   std::as_writable_bytes( std::span<std::uint8_t>( drawn.rgba ) ) )
				               .HasValue();
			}
		}
	}
	for ( GroupResources &resources : groups )
	{
		if ( resources.group.IsValid() )
			(void)device.Release( resources.group, device::CompletionToken() );
	}
	for ( device::ResourceId id : owned )
		(void)device.Release( id, device::CompletionToken() );
	(void)device.Poll();
	return drawn;
}

void JudgeCase( testing::Checks &checks, const FamilyCase &testCase, const Drawn &drawn )
{
	// RENDER_FAMILY_DUMP_DIR: the drawn frame as <case>.ppm, for diagnosis.
	if ( const char *directory = std::getenv( "RENDER_FAMILY_DUMP_DIR" ) )
	{
		std::ofstream file(
		    std::string( directory ) + "/" + testCase.name + ".ppm", std::ios::binary );
		file << "P6\n" << kSize << " " << kSize << "\n255\n";
		for ( std::size_t i = 0; i < std::size_t( kSize ) * kSize; ++i )
			file.write( reinterpret_cast<const char *>( &drawn.rgba[i * 4] ), 3 );
	}
	int worst = 0;
	std::string worstAt;
	for ( const PortPixel &pixel : testCase.port )
	{
		const std::size_t i = ( std::size_t( pixel.y ) * kSize + std::size_t( pixel.x ) ) * 4;
		for ( int c = 0; c < pixel.channels; ++c )
		{
			const int difference = std::abs( int( drawn.rgba[i + c] ) - pixel.rgba[c] );
			if ( difference > worst )
			{
				worst = difference;
				worstAt = std::to_string( pixel.x ) + "," + std::to_string( pixel.y ) +
				          " channel " + std::to_string( c ) + ": family";
				for ( int k = 0; k < pixel.channels; ++k )
					worstAt += " " + std::to_string( drawn.rgba[i + k] );
				worstAt += ", port";
				for ( int k = 0; k < pixel.channels; ++k )
					worstAt += " " + std::to_string( pixel.rgba[k] );
			}
		}
	}
	std::printf( "INFO %s: %zu port pixels, worst difference %d\n", testCase.name.c_str(),
	    testCase.port.size(), worst );
	checks.That( !testCase.port.empty(), "fixture." + testCase.name + ".has-port-pixels" );
	That( checks, worst <= kTolerance, "pixels." + testCase.name + ".within-tolerance-of-the-port",
	    worstAt );
}

} // namespace rendertest::families
