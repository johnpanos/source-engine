//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RFC 0007 R47 pixel oracle for PBRMetalRough on dynamic meshes
//          (models and props). Draws shaders/model_pbr.frag through the skin
//          pipeline's layout with Source's model lighting inputs (ambient cube,
//          a sorted local light with per-vertex attenuation, $emissiontexture,
//          $envmap) and compares each captured pixel with an independent CPU
//          evaluation built on public/render/pbr_brdf.h. Seeded controls show the
//          oracle separates plausible wrong implementations from the expected
//          pixel. Requires a native Vulkan device and a display; exit 77 means
//          the gate is unverified.
//
//===========================================================================//

#include "mapcontainer/probe_volume.h"
#include "render/pbr_brdf.h"
#include "../../materialsystem/shaderapivulkan/sdl3/sdl3_vulkan_surface_host.h"
#include "../../materialsystem/shaderapivulkan/vulkan_device.h"
#include "../../materialsystem/shaderapivulkan/vulkan_world_lightmap.h"
#include "testing/conformance_result.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

namespace
{

using render_vulkan::CVulkanContext;

int g_checks = 0;
int g_failures = 0;

void Check( bool condition, const char *message )
{
	++g_checks;
	if ( !condition )
	{
		++g_failures;
		std::fprintf( stderr, "FAIL: %s\n", message );
	}
}

float DecodeSrgb( float value )
{
	return value <= 0.04045f ? value / 12.92f : std::pow( ( value + 0.055f ) / 1.055f, 2.4f );
}

float EncodeSrgb( float value )
{
	value = std::clamp( value, 0.0f, 1.0f );
	return value <= 0.0031308f ? value * 12.92f : 1.055f * std::pow( value, 1.0f / 2.4f ) - 0.055f;
}

struct Vec3
{
	float x, y, z;
};

Vec3 Normalize( Vec3 v )
{
	const float length = std::sqrt( v.x * v.x + v.y * v.y + v.z * v.z );
	return { v.x / length, v.y / length, v.z / length };
}

float Dot( Vec3 a, Vec3 b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Everything one case feeds the shader, and what the CPU model reads.
struct Scene
{
	std::array<uint8_t, 4> base = { 200, 120, 60, 255 };      // sRGB
	std::array<uint8_t, 4> mrao = { 0, 128, 255, 255 };       // linear
	std::array<uint8_t, 4> normalTexel = { 128, 128, 255, 255 }; // tangent-space
	std::array<uint8_t, 4> emission = { 0, 0, 0, 255 };       // sRGB
	float emissionScale = 1.0f;
	float ambient[6][3] = {};    // +X -X +Y -Y +Z -Z
	float environment[6][3] = {}; // $envmap cube faces (linear)
	int lights = 0;
	Vec3 lightDirection = { 0, 0, -1 }; // toward the light
	float lightColor[3] = { 0, 0, 0 };
	float attenuation = 1.0f;
	float coat = 0.0f;
	float coatRoughness = 0.03f;
	int flags = 0; // model_pbr.frag's
	// The diffuse light the model uses instead of the ambient cube (a probe
	// volume's irradiance at the pixel); the ambient cube still lights the
	// specular fallback.
	bool diffuseOverride = false;
	float diffuseLight[3] = {};
};

// The quad faces the eye: normal and view are -Z (Source's model lighting sees
// the surface from the eye at -Z; clip space equals world space here).
constexpr Vec3 kNormal = { 0, 0, -1 };
constexpr Vec3 kView = { 0, 0, -1 };
constexpr float kDistance = 1000.0f;

Vec3 AmbientCubeAt( const float cube[6][3], Vec3 n, int channel )
{
	const float nx2 = n.x * n.x, ny2 = n.y * n.y, nz2 = n.z * n.z;
	const float value = nx2 * cube[n.x >= 0 ? 0 : 1][channel] +
	                    ny2 * cube[n.y >= 0 ? 2 : 3][channel] +
	                    nz2 * cube[n.z >= 0 ? 4 : 5][channel];
	return { value, 0, 0 };
}

// Deliberate defects the controls seed into the CPU model.
enum Defect
{
	kNone,
	kSpecularWithoutPi,    // local light specular missing Source's pi radiance scale
	kIgnoreAttenuation,    // per-vertex light attenuation not applied
	kMraoAsSrgb,           // linear MRAO decoded as sRGB
	kNoOcclusion,          // ambient not multiplied by MRAO occlusion
	kEnvIgnoresFresnel,    // environment added without the split-sum albedo
	kEmissionLinearBytes,  // emission bytes used without sRGB decode
	kNoClearCoat,          // $clearcoat ignored
};

std::array<float, 3> Expected( const Scene &scene, Defect defect = kNone )
{
	using namespace render::pbr;
	std::array<float, 3> result = {};
	const float mraoR = scene.mrao[0] / 255.0f;
	const float mraoG = scene.mrao[1] / 255.0f;
	const float mraoB = scene.mrao[2] / 255.0f;
	const float metalness = defect == kMraoAsSrgb ? DecodeSrgb( mraoR ) : mraoR;
	const float roughness = std::max( defect == kMraoAsSrgb ? DecodeSrgb( mraoG ) : mraoG, 0.02f );
	const float occlusion = defect == kNoOcclusion ? 1.0f : mraoB;
	const Color base = { DecodeSrgb( scene.base[0] / 255.0f ), DecodeSrgb( scene.base[1] / 255.0f ),
	    DecodeSrgb( scene.base[2] / 255.0f ) };
	const float baseChannel[3] = { base.red, base.green, base.blue };
	Vec3 normal = kNormal;
	if ( scene.flags & CVulkanContext::kPbrModelNormalMap )
	{
		// Tangent S = +X, T = cross( N, S ) * +1, as the vertices below.
		const float x = scene.normalTexel[0] / 255.0f * 2.0f - 1.0f;
		const float y = scene.normalTexel[1] / 255.0f * 2.0f - 1.0f;
		const float z = std::sqrt( std::max( 0.0f, 1.0f - x * x - y * y ) );
		const Vec3 s = { 1, 0, 0 };
		const Vec3 t = { kNormal.y * s.z - kNormal.z * s.y, kNormal.z * s.x - kNormal.x * s.z,
			kNormal.x * s.y - kNormal.y * s.x };
		normal = Normalize( { s.x * x + t.x * y + kNormal.x * z, s.y * x + t.y * y + kNormal.y * z,
		    s.z * x + t.z * y + kNormal.z * z } );
	}
	const float normalDotView = std::max( Dot( normal, kView ), 0.0f );
	const SplitSumCoefficients albedo = SampleSplitSum( normalDotView, roughness );
	const Vec3 light = Normalize( scene.lightDirection );
	const float normalDotLight = std::max( Dot( normal, light ), 0.0f );
	const Vec3 half = Normalize( { light.x + kView.x, light.y + kView.y, light.z + kView.z } );
	const float normalDotHalf = std::max( Dot( normal, half ), 0.0f );
	const float viewDotHalf = std::max( Dot( kView, half ), 0.0f );
	const Color direct = EvaluateLayeredDirect(
	    base, metalness, normalDotView, normalDotLight, normalDotHalf, viewDotHalf, roughness );
	const float directChannel[3] = { direct.red, direct.green, direct.blue };
	const float f0Base = 0.04f * ( 1.0f - metalness );
	const Color f0 = { f0Base + base.red * metalness, f0Base + base.green * metalness,
	    f0Base + base.blue * metalness };
	const Color specular = EvaluateSpecular(
	    f0, normalDotView, normalDotLight, normalDotHalf, viewDotHalf, roughness );
	const float specularChannel[3] = { specular.red, specular.green, specular.blue };
	const float f0Channel[3] = { f0.red, f0.green, f0.blue };
	// reflect( -view, normal )
	const float vn = Dot( kView, normal );
	const Vec3 reflected = { 2.0f * vn * normal.x - kView.x, 2.0f * vn * normal.y - kView.y,
		2.0f * vn * normal.z - kView.z };
	// Clear coat (Filament's model): IOR 1.5 layer on the geometric normal.
	const bool coated = ( scene.flags & CVulkanContext::kPbrModelClearCoat ) && defect != kNoClearCoat;
	const auto schlick = []( float f0, float cosine )
	{
		const float g = 1.0f - cosine;
		return f0 + ( 1.0f - f0 ) * g * g * g * g * g;
	};
	const float coatRoughness = std::max( scene.coatRoughness, 0.02f );
	const float coatLightDotHalf = std::max( Dot( light, half ), 0.0f );
	const float coatLightFresnel = coated ? schlick( 0.04f, coatLightDotHalf ) * scene.coat : 0.0f;
	const float coatDirect = coated ? kPi * GgxDistribution( std::max( Dot( kNormal, half ), 0.0f ),
	                                          coatRoughness ) *
	                                      ( 0.25f / std::max( coatLightDotHalf * coatLightDotHalf,
	                                                   1e-6f ) ) *
	                                      coatLightFresnel * std::max( Dot( kNormal, light ), 0.0f )
	                                : 0.0f;
	const float coatViewFresnel = coated ? schlick( 0.04f, std::max( Dot( kNormal, kView ), 0.0f ) ) *
	                                           scene.coat
	                                     : 0.0f;
	for ( int c = 0; c < 3; ++c )
	{
		const float directionalAlbedo = std::min( 1.0f, f0Channel[c] * albedo.a + albedo.b );
		const float diffuseColor = baseChannel[c] * ( 1.0f - metalness ) * ( 1.0f - directionalAlbedo );
		const float diffuseLight = scene.diffuseOverride
		                               ? scene.diffuseLight[c]
		                               : AmbientCubeAt( scene.ambient, normal, c ).x;
		float value = diffuseColor * diffuseLight * occlusion;
		float direct = 0.0f;
		if ( scene.lights > 0 && normalDotLight > 0.0f )
		{
			const float incident = scene.lightColor[c] *
			                       ( defect == kIgnoreAttenuation ? 1.0f : scene.attenuation );
			float lit = kPi * directChannel[c] * incident * normalDotLight;
			if ( defect == kSpecularWithoutPi )
				lit -= ( kPi - 1.0f ) * specularChannel[c] * incident * normalDotLight;
			direct = lit * ( 1.0f - coatLightFresnel ) + coatDirect * incident;
		}
		float environment;
		if ( scene.flags & CVulkanContext::kPbrModelEnvMap )
		{
			// A 1x1 cube: the face the reflected direction selects.
			const float ax = std::fabs( reflected.x ), ay = std::fabs( reflected.y ),
			            az = std::fabs( reflected.z );
			const int face = ax >= ay && ax >= az ? ( reflected.x >= 0 ? 0 : 1 )
			                 : ay >= az           ? ( reflected.y >= 0 ? 2 : 3 )
			                                      : ( reflected.z >= 0 ? 4 : 5 );
			environment = scene.environment[face][c];
		}
		else
		{
			environment = AmbientCubeAt( scene.ambient, Normalize( reflected ), c ).x;
		}
		value += environment * ( defect == kEnvIgnoresFresnel ? 1.0f : directionalAlbedo ) *
		         occlusion;
		// The coat reflects the environment in the geometric normal's mirror
		// direction (-Z here, like the base), attenuating the image light.
		value = value * ( 1.0f - coatViewFresnel ) + environment * coatViewFresnel * occlusion +
		        direct;
		if ( scene.flags & CVulkanContext::kPbrModelEmission )
		{
			const float byte = scene.emission[c] / 255.0f;
			value += ( defect == kEmissionLinearBytes ? byte : DecodeSrgb( byte ) ) *
			         scene.emissionScale;
		}
		result[c] = value;
	}
	return result;
}

struct Handles
{
	int base, mrao, normal, emission, env;
};

// Draws the quad; returns its centre pixel and, when `frame` is given, the
// whole capture (RGBA rows, top first).
bool Draw( CVulkanContext &context, const Handles &handles, const Scene &scene,
    std::array<uint8_t, 4> *pixel, std::string *error, std::vector<uint8_t> *frame = nullptr,
    int *frameWidth = nullptr, int *frameHeight = nullptr )
{
	context.ClearDynamicQueue();
	context.SetClearColor( 0, 0, 0, 1 );
	if ( !context.UploadManagedTexture( handles.base, scene.base.data(), 4, error ) ||
	     !context.UploadManagedTexture( handles.mrao, scene.mrao.data(), 4, error ) ||
	     !context.UploadManagedTexture( handles.normal, scene.normalTexel.data(), 4, error ) ||
	     !context.UploadManagedTexture( handles.emission, scene.emission.data(), 4, error ) )
		return false;
	for ( uint32_t face = 0; face < 6; ++face )
	{
		// Linear RGBA16F faces.
		uint16_t texel[4];
		for ( int c = 0; c < 3; ++c )
		{
			// Half-float encode of values in [0, 2).
			float value = scene.environment[face][c];
			uint32_t bits;
			std::memcpy( &bits, &value, 4 );
			const int exponent = int( ( bits >> 23 ) & 0xff ) - 127 + 15;
			texel[c] = value <= 0.0f
			               ? 0
			               : uint16_t( ( exponent << 10 ) | ( ( bits >> 13 ) & 0x3ff ) );
		}
		texel[3] = 0x3c00;
		if ( !context.UploadManagedTexture( handles.env,
		         reinterpret_cast<const uint8_t *>( texel ), sizeof( texel ), error, 0, face ) )
			return false;
	}
	CVulkanContext::SkinConstants constants;
	std::memset( constants.ps, 0, sizeof( constants.ps ) );
	for ( int face = 0; face < 6; ++face )
		for ( int c = 0; c < 3; ++c )
			constants.ps[4 + face][c] = scene.ambient[face][c];
	const Vec3 light = Normalize( scene.lightDirection );
	for ( int c = 0; c < 3; ++c )
		constants.ps[20][c] = scene.lightColor[c];
	constants.ps[21][0] = light.x * kDistance;
	constants.ps[21][1] = light.y * kDistance;
	constants.ps[21][2] = 0.5f + light.z * kDistance;
	constants.ps[2][0] = scene.emissionScale;
	constants.ps[2][1] = scene.coat;
	constants.ps[2][2] = scene.coatRoughness;
	constants.ps[2][3] = 1.0f;
	const float identity[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	std::memcpy( constants.viewProj, identity, sizeof( identity ) );
	const float row0[4] = { 1, 0, 0, 0 };
	const float row1[4] = { 0, 1, 0, 0 };
	std::memcpy( constants.texXform0, row0, sizeof( row0 ) );
	std::memcpy( constants.texXform1, row1, sizeof( row1 ) );
	constants.eyePos[0] = kView.x * kDistance;
	constants.eyePos[1] = kView.y * kDistance;
	constants.eyePos[2] = 0.5f + kView.z * kDistance;
	constants.eyePos[3] = 0.0f;
	constants.combos = scene.flags;
	constants.numLights = scene.lights;
	context.SetDynamicSkinConstants( constants );
	context.SelectDynamicShader( CVulkanContext::kDynShaderPbrModel );
	context.SelectDynamicColorSpace(
	    CVulkanContext::kColorSrgbReadBase | CVulkanContext::kColorSrgbWrite );
	context.BindManagedTexture( handles.base );
	context.BindManagedSampler( 10, handles.mrao );
	context.BindManagedSampler( 1, handles.normal );
	context.BindManagedSampler( 2, handles.emission );
	context.BindManagedSampler( 3, handles.env );
	// The skin record: world position, the light attenuation in the color slot,
	// uv; normal, tangent S with the binormal sign; the fourth attenuation.
	const float a = scene.attenuation;
	const float quad[6][8] = {
	    { -0.5f, -0.5f, 0.5f, a, 0, 0, 0, 0 },
	    { 0.5f, 0.5f, 0.5f, a, 0, 0, 1, 1 },
	    { 0.5f, -0.5f, 0.5f, a, 0, 0, 1, 0 },
	    { -0.5f, -0.5f, 0.5f, a, 0, 0, 0, 0 },
	    { -0.5f, 0.5f, 0.5f, a, 0, 0, 0, 1 },
	    { 0.5f, 0.5f, 0.5f, a, 0, 0, 1, 1 },
	};
	float normalTangent[6][7];
	float alpha[6];
	for ( int v = 0; v < 6; ++v )
	{
		const float frame[7] = { kNormal.x, kNormal.y, kNormal.z, 1, 0, 0, 1 };
		std::memcpy( normalTangent[v], frame, sizeof( frame ) );
		alpha[v] = 0.0f;
	}
	context.QueueDynamicTriangles( &quad[0][0], 6, nullptr, &normalTangent[0][0], alpha );
	context.RequestCapture();
	bool skip = false;
	if ( !context.BeginFrame( &skip, error ) || skip || !context.EndFrame( error ) )
	{
		if ( skip )
			*error = "surface unexpectedly skipped a model PBR frame";
		return false;
	}
	int width = 0;
	int height = 0;
	const std::vector<uint8_t> &pixels = context.GetCapturedPixels( &width, &height );
	if ( width < 4 || height < 4 || pixels.size() < size_t( width * height * 4 ) )
	{
		*error = "model PBR capture is incomplete";
		return false;
	}
	std::memcpy( pixel->data(), &pixels[( size_t( height / 2 ) * width + width / 2 ) * 4], 4 );
	if ( frame )
	{
		frame->assign( pixels.begin(), pixels.begin() + size_t( width ) * height * 4 );
		*frameWidth = width;
		*frameHeight = height;
	}
	Check( pixels[0] <= 2 && pixels[1] <= 2 && pixels[2] <= 2, "frame corner keeps the clear" );
	return true;
}

// Compares the captured sRGB pixel with the model (tolerance in 8-bit steps),
// and requires every seeded defect to miss it by more than `separation`.
void Judge( const char *name, const std::array<uint8_t, 4> &pixel, const Scene &scene,
    std::initializer_list<Defect> controls, float separation = 6.0f )
{
	const std::array<float, 3> expected = Expected( scene );
	std::fprintf( stderr, "%s: GPU=(%u,%u,%u) CPU=(%.1f,%.1f,%.1f)\n", name, pixel[0], pixel[1],
	    pixel[2], EncodeSrgb( expected[0] ) * 255.0f, EncodeSrgb( expected[1] ) * 255.0f,
	    EncodeSrgb( expected[2] ) * 255.0f );
	bool match = true;
	for ( int c = 0; c < 3; ++c )
		match &= std::fabs( pixel[c] - EncodeSrgb( expected[c] ) * 255.0f ) <= 3.0f;
	std::string message = std::string( name ) + ": native pixel matches the CPU model";
	Check( match, message.c_str() );
	for ( Defect defect : controls )
	{
		const std::array<float, 3> wrong = Expected( scene, defect );
		float distance = 0.0f;
		for ( int c = 0; c < 3; ++c )
			distance = std::max( distance,
			    std::fabs( pixel[c] - EncodeSrgb( wrong[c] ) * 255.0f ) );
		std::string control =
		    std::string( name ) + ": seeded defect " + std::to_string( int( defect ) ) +
		    " is separated from the captured pixel";
		Check( distance > separation, control.c_str() );
	}
}

// RFC 0011 G1: model_pbr.frag -DPROBE_VOLUME samples the map's probe volume
// per pixel exactly as the C++ reference sampler (mapcontainer::
// ProbeVolumeView, itself checked against the Python reader) does, and falls
// back to the ambient cube outside it. The fixture's grid covers the quad's
// x <= 0.2; clip space equals world space, so a pixel's world position is its
// clip-space position at z = 0.5.
void ProbeVolumeCases( CVulkanContext &context, const Handles &handles )
{
	using mapcontainer::ProbeVolumeLayer;
	std::string error;
	std::ifstream file( "quality/fixtures/gi/prbv/gpu.prbv", std::ios::binary );
	const std::vector<unsigned char> bytes(
	    std::istreambuf_iterator<char>{ file }, std::istreambuf_iterator<char>{} );
	mapcontainer::ProbeVolumeLayout layout{};
	const bool valid = !bytes.empty() &&
	                   mapcontainer::ValidateProbeVolume( bytes.data(), bytes.size(), &layout ) ==
	                       mapcontainer::ProbeVolumeError::Ok;
	Check( valid, "probe volume fixture gpu.prbv validates" );
	if ( !valid )
		return;
	// The volume is map-scoped: it needs a resident world mesh.
	const std::array<uint8_t, 3 * 40> vertices = {};
	const std::array<uint32_t, 3> indices = { 0, 1, 2 };
	Check( context.UploadWorldMesh(
	           vertices.data(), vertices.size(), indices.data(), sizeof( indices ), &error ),
	    "a world mesh is resident for the probe volume" );
	std::vector<float> table( layout.gridCount * mapcontainer::kProbeGridTableFloats );
	mapcontainer::WriteProbeGridTable( layout, table.data() );
	world_mesh_gpu::ProbeVolumeUploadRequest request;
	request.atlasWidth = layout.atlasWidth;
	request.atlasHeight = layout.atlasHeight;
	request.atlas = bytes.data() + layout.atlasOffset;
	request.gridCount = layout.gridCount;
	request.tableFloats = mapcontainer::kProbeGridTableFloats;
	request.gridTable = table.data();
	Check( render_vulkan::UploadWorldProbeVolume( context, request, &error ) &&
	           context.ProbeVolumeResident(),
	    "the probe volume uploads" );
	Check( context.ProbeVolumeSamplingSupported(),
	    "this device samples the probe volume per pixel (nine descriptor sets)" );
	if ( !context.ProbeVolumeResident() || !context.ProbeVolumeSamplingSupported() )
		return;
	const mapcontainer::ProbeVolumeView view( bytes.data(), layout );

	struct Run
	{
		const char *name;
		int indirectView;
		int sampling;
		ProbeVolumeLayer layer;
		bool visibility;
		bool normalMap;
	};
	const Run runs[] = {
	    { "indirect view, indirect layer", 1, 1, ProbeVolumeLayer::Indirect, true, false },
	    { "indirect view, tilted normal", 1, 1, ProbeVolumeLayer::Indirect, true, true },
	    { "shaded, total layer", 0, 1, ProbeVolumeLayer::Total, true, false },
	    { "shaded, visibility off", 0, 2, ProbeVolumeLayer::Total, false, false },
	};
	const float kScale = 2.0f; // the indirect view's exposure
	for ( const Run &run : runs )
	{
		Scene scene;
		scene.base = { 190, 190, 190, 255 };
		scene.mrao = { 0, 230, 255, 255 };
		scene.normalTexel = { 170, 110, 230, 255 };
		scene.flags = run.normalMap ? CVulkanContext::kPbrModelNormalMap : 0;
		for ( int face = 0; face < 6; ++face )
			for ( int c = 0; c < 3; ++c )
				scene.ambient[face][c] = 0.3f; // the fallback, distinct from the volume
		// The normal the shader shades with (Expected()'s tangent frame).
		Vec3 normal = kNormal;
		if ( run.normalMap )
		{
			const float x = scene.normalTexel[0] / 255.0f * 2.0f - 1.0f;
			const float y = scene.normalTexel[1] / 255.0f * 2.0f - 1.0f;
			const float z = std::sqrt( std::max( 0.0f, 1.0f - x * x - y * y ) );
			normal = Normalize( { x, -y, -z } ); // S = +X, T = N x S = -Y, N = -Z
		}
		context.SetIndirectLightView( run.indirectView, kScale );
		context.SetProbeVolumeSampling( run.sampling );
		std::array<uint8_t, 4> centre = {};
		std::vector<uint8_t> frame;
		int width = 0, height = 0;
		const bool drawn =
		    Draw( context, handles, scene, &centre, &error, &frame, &width, &height );
		Check( drawn, ( std::string( run.name ) + ": probe case renders" ).c_str() );
		if ( !drawn )
			continue;
		int inside = 0, outside = 0, mismatches = 0, visibilityMatters = 0, layerMatters = 0;
		std::string first;
		for ( int py = 0; py < height; py += 3 )
		{
			for ( int px = 0; px < width; px += 3 )
			{
				// The backend keeps D3D9's clip space (+Y at the top) and pixel
				// centres at integer coordinates (vulkan_device.cpp's draw viewport).
				const float x = float( px ) / width * 2.0f - 1.0f;
				const float y = 1.0f - float( py ) / height * 2.0f;
				const float margin = 2.5f * 2.0f / std::min( width, height );
				// Inside the quad, away from its edges and the grid's x = 0.2 face.
				if ( std::fabs( x ) > 0.5f - margin || std::fabs( y ) > 0.5f - margin ||
				     std::fabs( x - 0.2f ) < margin )
					continue;
				const float position[3] = { x, y, 0.5f };
				const float n[3] = { normal.x, normal.y, normal.z };
				float irradiance[3];
				const bool covered =
				    view.Sample( position, n, run.layer, run.visibility, irradiance );
				covered ? ++inside : ++outside;
				float other[3], otherLayer[3];
				if ( covered && view.Sample( position, n, run.layer, !run.visibility, other ) &&
				     view.Sample( position, n,
				         run.layer == ProbeVolumeLayer::Total ? ProbeVolumeLayer::Indirect
				                                              : ProbeVolumeLayer::Total,
				         run.visibility, otherLayer ) )
				{
					visibilityMatters += std::fabs( other[0] - irradiance[0] ) > 0.01f;
					layerMatters += std::fabs( otherLayer[0] - irradiance[0] ) > 0.01f;
				}
				std::array<float, 3> expected;
				if ( run.indirectView )
				{
					for ( int c = 0; c < 3; ++c )
						expected[c] = ( covered ? irradiance[c]
						                        : AmbientCubeAt( scene.ambient, normal, c ).x ) *
						              kScale;
				}
				else
				{
					Scene lit = scene;
					lit.diffuseOverride = covered;
					for ( int c = 0; c < 3; ++c )
						lit.diffuseLight[c] = irradiance[c];
					expected = Expected( lit );
				}
				const uint8_t *got = &frame[( size_t( py ) * width + px ) * 4];
				bool match = true;
				for ( int c = 0; c < 3; ++c )
					match &= std::fabs( got[c] - EncodeSrgb( expected[c] ) * 255.0f ) <= 3.0f;
				if ( !match && mismatches++ == 0 )
				{
					char text[256];
					std::snprintf( text, sizeof( text ),
					    " (first at %.3f %.3f: GPU %u %u %u, C++ %.1f %.1f %.1f)", x, y, got[0],
					    got[1], got[2], EncodeSrgb( expected[0] ) * 255.0f,
					    EncodeSrgb( expected[1] ) * 255.0f, EncodeSrgb( expected[2] ) * 255.0f );
					first = text;
				}
			}
		}
		std::fprintf( stderr, "%s: %d inside, %d outside, %d mismatches%s\n", run.name, inside,
		    outside, mismatches, first.c_str() );
		Check( mismatches == 0 && inside >= 100 && outside >= 20,
		    ( std::string( run.name ) +
		        ": every pixel matches the C++ sampler inside the volume and the ambient cube "
		        "outside it" )
		        .c_str() );
		// Controls: this fixture distinguishes visibility on/off and the layers.
		Check( visibilityMatters >= 20,
		    ( std::string( run.name ) + ": the visibility test changes the sampled light" )
		        .c_str() );
		Check( layerMatters >= 100,
		    ( std::string( run.name ) + ": the other layer differs from the sampled one" )
		        .c_str() );
	}
	context.SetIndirectLightView( 0, 1.0f );
	context.SetProbeVolumeSampling( 1 );
	context.ReleaseWorldMesh();
	Check( !context.ProbeVolumeResident(), "releasing the world mesh releases the probe volume" );
}

} // namespace

int main()
{
	if ( !SDL_Init( SDL_INIT_VIDEO ) )
	{
		std::fprintf( stderr, "SKIP: SDL video unavailable: %s\n", SDL_GetError() );
		return 77;
	}
	SDL_Window *window = SDL_CreateWindow( "model-pbr-pixels", 128, 128, SDL_WINDOW_VULKAN );
	if ( !window )
	{
		std::fprintf( stderr, "SKIP: Vulkan window unavailable: %s\n", SDL_GetError() );
		SDL_Quit();
		return 77;
	}
	std::string error;
	std::unique_ptr<render_vulkan::IVulkanSurfaceHost> host =
	    render_vulkan::MakeSdl3LegacySurfaceHost( window, &error );
	CVulkanContext context;
	render_vulkan::VulkanContextConfig config;
	config.appName = "model-pbr-pixels";
	config.enableValidation = true;
	if ( !host || !context.Init( *host, config, &error ) )
	{
		std::fprintf( stderr, "SKIP: native Vulkan context unavailable: %s\n", error.c_str() );
		SDL_DestroyWindow( window );
		SDL_Quit();
		return 77;
	}
	Check( context.InitDynamicMesh( &error ) && context.PbrModelPipelineSupported(),
	    "model PBR pipeline initializes" );
	Handles handles;
	// The material system's base textures: UNORM storage with an sRGB view,
	// which the sampler reads for a material that enables sRGB reads.
	handles.base = context.CreateManagedTexture(
	    1, 1, VK_FORMAT_R8G8B8A8_UNORM, &error, 0, 1, VK_FORMAT_R8G8B8A8_SRGB );
	// A base stored in an sRGB format (KTX2 packages) must not decode twice.
	const int srgbStoredBase = context.CreateManagedTexture( 1, 1, VK_FORMAT_R8G8B8A8_SRGB, &error );
	handles.mrao = context.CreateManagedTexture( 1, 1, VK_FORMAT_R8G8B8A8_UNORM, &error );
	handles.normal = context.CreateManagedTexture( 1, 1, VK_FORMAT_R8G8B8A8_UNORM, &error );
	handles.emission = context.CreateManagedTexture( 1, 1, VK_FORMAT_R8G8B8A8_UNORM, &error );
	handles.env = context.CreateManagedTexture(
	    1, 1, VK_FORMAT_R16G16B16A16_SFLOAT, &error, 0, 1, VK_FORMAT_UNDEFINED, true );
	const bool imagesReady = handles.base >= 0 && handles.mrao >= 0 && handles.normal >= 0 &&
	                         handles.emission >= 0 && handles.env >= 0;
	Check( imagesReady, "model PBR images, including an HDR cube, are supported" );
	if ( imagesReady && context.PbrModelPipelineSupported() )
	{
		std::array<uint8_t, 4> pixel = {};

		// 1. Ambient cube only: diffuse through the layered BRDF, occluded,
		// plus the cube in the reflected direction as specular.
		Scene ambient;
		for ( int face = 0; face < 6; ++face )
			for ( int c = 0; c < 3; ++c )
				ambient.ambient[face][c] = face == 5 ? 0.6f : 0.15f;
		ambient.mrao = { 0, 128, 200, 255 };
		Check( Draw( context, handles, ambient, &pixel, &error ), "ambient case renders" );
		Judge( "ambient dielectric", pixel, ambient, { kNoOcclusion } );

		// RFC 0011 indirect view: the ambient cube alone (view 1: light, no
		// albedo; view 2: times the diffuse albedo and occlusion). The quad
		// faces -Z, whose cube face is 0.6.
		{
			const auto srgbByte = []( float linear )
			{
				linear = std::min( 1.0f, std::max( 0.0f, linear ) );
				return 255.0f * ( linear <= 0.0031308f
				                        ? linear * 12.92f
				                        : 1.055f * std::pow( linear, 1.0f / 2.4f ) - 0.055f );
			};
			context.SetIndirectLightView( 1, 1.0f );
			Check( Draw( context, handles, ambient, &pixel, &error ), "indirect view renders" );
			bool lightOnly = true;
			for ( int c = 0; c < 3; ++c )
				lightOnly = lightOnly && std::abs( pixel[c] - srgbByte( 0.6f ) ) <= 1.5f;
			Check( lightOnly, "model indirect view 1 writes the ambient cube's light" );
			context.SetIndirectLightView( 2, 1.0f );
			Check( Draw( context, handles, ambient, &pixel, &error ), "indirect view 2 renders" );
			bool radiance = true;
			for ( int c = 0; c < 3; ++c )
			{
				const float base = std::pow( ( ambient.base[c] / 255.0f + 0.055f ) / 1.055f, 2.4f );
				radiance = radiance &&
				           std::abs( pixel[c] - srgbByte( base * 0.6f * 200.0f / 255.0f ) ) <= 1.5f;
			}
			Check( radiance, "model indirect view 2 writes albedo x occlusion x ambient light" );
			// A local light is direct light: the view leaves it out.
			Scene direct = ambient;
			for ( auto &face : direct.ambient )
				face[0] = face[1] = face[2] = 0.0f;
			direct.lights = 1;
			direct.lightColor[0] = direct.lightColor[1] = direct.lightColor[2] = 1.0f;
			context.SetIndirectLightView( 1, 1.0f );
			Check( Draw( context, handles, direct, &pixel, &error ) && pixel[0] <= 1 &&
			           pixel[1] <= 1 && pixel[2] <= 1,
			    "model indirect view excludes local (direct) lights" );
			context.SetIndirectLightView( 0, 1.0f );
		}

		// 2. One local light at 40 degrees, attenuated at the vertices.
		Scene lit;
		lit.lights = 1;
		lit.lightDirection = { std::sin( 0.7f ), 0, -std::cos( 0.7f ) };
		lit.lightColor[0] = lit.lightColor[1] = lit.lightColor[2] = 0.9f;
		lit.attenuation = 0.5f;
		lit.mrao = { 0, 90, 255, 255 };
		Check( Draw( context, handles, lit, &pixel, &error ), "lit dielectric renders" );
		Judge( "lit dielectric", pixel, lit, { kIgnoreAttenuation } );
		if ( srgbStoredBase >= 0 )
		{
			Handles srgbHandles = handles;
			srgbHandles.base = srgbStoredBase;
			std::array<uint8_t, 4> srgbPixel = {};
			Check( Draw( context, srgbHandles, lit, &srgbPixel, &error ),
			    "sRGB-stored base renders" );
			Judge( "lit dielectric, sRGB-stored base", srgbPixel, lit, {} );
		}

		// 3. A smooth colored metal near the mirror direction: the specular
		// lobe dominates and its pi radiance scale is visible.
		Scene metal = lit;
		metal.mrao = { 255, 110, 255, 255 };
		metal.lightDirection = { std::sin( 0.15f ), 0, -std::cos( 0.15f ) };
		metal.lightColor[0] = metal.lightColor[1] = metal.lightColor[2] = 0.2f;
		metal.attenuation = 1.0f;
		Check( Draw( context, handles, metal, &pixel, &error ), "lit metal renders" );
		Judge( "lit metal", pixel, metal, { kSpecularWithoutPi, kMraoAsSrgb } );

		// 4. Tangent-space normal map tilts the lit response.
		Scene bumped = lit;
		bumped.flags = CVulkanContext::kPbrModelNormalMap;
		bumped.normalTexel = { 200, 128, 255, 255 };
		Check( Draw( context, handles, bumped, &pixel, &error ), "normal-mapped case renders" );
		Judge( "normal-mapped dielectric", pixel, bumped, {} );
		Check( std::fabs( pixel[0] - EncodeSrgb( Expected( lit )[0] ) * 255.0f ) > 6.0f,
		    "normal map changes the lit pixel" );

		// 5. $emissiontexture decoded from sRGB, times $emissionscale.
		Scene emissive;
		emissive.flags = CVulkanContext::kPbrModelEmission;
		emissive.emission = { 100, 40, 10, 255 };
		emissive.emissionScale = 1.5f;
		Check( Draw( context, handles, emissive, &pixel, &error ), "emissive case renders" );
		Judge( "emission", pixel, emissive, { kEmissionLinearBytes } );

		// 6. $envmap: a cube whose faces differ; the smooth metal reflects
		// the -Z face through the split-sum albedo.
		Scene env;
		env.flags = CVulkanContext::kPbrModelEnvMap;
		env.mrao = { 255, 20, 255, 255 };
		env.base = { 230, 230, 230, 255 };
		for ( int face = 0; face < 6; ++face )
		{
			env.environment[face][0] = face == 5 ? 0.2f : 0.9f;
			env.environment[face][1] = face == 5 ? 0.5f : 0.0f;
			env.environment[face][2] = face == 5 ? 0.8f : 0.0f;
		}
		Check( Draw( context, handles, env, &pixel, &error ), "environment map case renders" );
		Judge( "envmap metal", pixel, env, { kEnvIgnoresFresnel } );

		// 7. $clearcoat over a rough, dark base: a light along the normal puts
		// the coat's GGX peak (roughness 0.3) on the pixel.
		Scene coated;
		coated.flags = CVulkanContext::kPbrModelClearCoat;
		coated.coat = 1.0f;
		coated.coatRoughness = 0.3f;
		coated.base = { 40, 40, 40, 255 };
		coated.mrao = { 0, 255, 255, 255 };
		coated.lights = 1;
		coated.lightDirection = kNormal;
		coated.lightColor[0] = coated.lightColor[1] = coated.lightColor[2] = 0.2f;
		for ( int face = 0; face < 6; ++face )
			for ( int c = 0; c < 3; ++c )
				coated.ambient[face][c] = 0.05f;
		Check( Draw( context, handles, coated, &pixel, &error ), "clear coat case renders" );
		Judge( "clear coat", pixel, coated, { kNoClearCoat } );

		ProbeVolumeCases( context, handles );
	}
	if ( handles.base >= 0 )
		context.DestroyManagedTexture( handles.base );
	if ( srgbStoredBase >= 0 )
		context.DestroyManagedTexture( srgbStoredBase );
	if ( handles.mrao >= 0 )
		context.DestroyManagedTexture( handles.mrao );
	if ( handles.normal >= 0 )
		context.DestroyManagedTexture( handles.normal );
	if ( handles.emission >= 0 )
		context.DestroyManagedTexture( handles.emission );
	if ( handles.env >= 0 )
		context.DestroyManagedTexture( handles.env );
	context.Shutdown();
	SDL_DestroyWindow( window );
	SDL_Quit();
	if ( !error.empty() && g_failures )
		std::fprintf( stderr, "last error: %s\n", error.c_str() );
	return testing::ReportConformance( g_checks, g_failures );
}
