#version 450
// PBRMetalRough on dynamic meshes (studio models, props and other non-WMSH
// draws): the RFC 0007 layered metal/roughness BRDF (public/render/pbr_brdf.h)
// lit by Source's model lighting. It runs on the skin pipeline's layout and
// vertex stage (skin.vert): the shader API supplies world-space positions,
// normals and tangents, and each vertex's attenuation of the four sorted
// lights, as it does for skin_vs20.
//
// Units follow Source's model lighting, where a Lambertian surface of albedo
// a returns a * color * attenuation * N.L. A local light is therefore incident
// radiance pi * color * attenuation through the BRDF, so a rough dielectric
// matches VertexLitGeneric's brightness. The ambient cube is treated the same
// way: a * cube( n ) is its Lambertian return.
//
// Specular image lighting comes from ENV_CUBE's $envmap (mip level chosen by
// roughness), else the map reflection probe carried in the WMSH LMAP atlas
// (world_pbr_probe.glsl), else the ambient cube in the reflected direction.
//
// Pixel constants (the material's dynamic state, c0..c31):
//   c2  x $emissionscale, y $clearcoat, z $clearcoatroughness, w the
//       environment map's mip count (ENV_CUBE)
//   c4..c9   cAmbientCube (PSREG_AMBIENT_CUBE)
//   c20..c25 cLightInfo (PSREG_LIGHT_INFO_ARRAY)
// Push block (skin.vert's): params x alpha-test reference (< 0 disables),
// y flags (1 normal map, 2 emission, 4 environment map, 8 map probe, 32 clear
// coat),
// z kColor* flags (1 sRGB base, 4 sRGB output), w linear light scale;
// params2.x the number of lights.
//
// PROBE_VOLUME (RFC 0011) takes the diffuse light from the map's PRBV probe
// volume, sampled at each pixel's world position and normal (probe_volume.glsl,
// sets 7 and 8), wherever the volume covers it; elsewhere, and without it, the
// ambient cube. params2.w is 0 without a resident volume, 1 to sample with the
// visibility test and 2 without it (r_probevolume_visibility 0).
//
// INDIRECT_VIEW (RFC 0011 debug view) writes only the indirect light, for
// comparison with Cycles' DiffInd pass: params2.y is the view (1 the diffuse
// light cube( n ), or the probe volume's indirect layer, no albedo; 2 the
// diffuse radiance, times the diffuse
// albedo and occlusion) and params2.z the exposure scale. Local lights,
// specular, emission and the tone-map scale are left out.
layout( location = 0 ) in vec2 vBaseUv;
layout( location = 1 ) in vec4 vLightAtten;
layout( location = 2 ) in vec3 vWorldVertToEye;
layout( location = 3 ) in vec3 vTangentS;
layout( location = 4 ) in vec3 vTangentT;
layout( location = 5 ) in vec3 vNormal;
layout( location = 6 ) in vec3 vWorldPos;
layout( location = 0 ) out vec4 outColor;
layout( set = 0, binding = 0 ) uniform sampler2D baseTexture;     // s0, sRGB
layout( set = 1, binding = 0 ) uniform sampler2D mraoTexture;     // s10, linear
layout( set = 2, binding = 0 ) uniform sampler2D normalTexture;   // s1, linear
layout( set = 3, binding = 0 ) uniform sampler2D emissionTexture; // s2, sRGB
layout( set = 4, binding = 0 ) uniform sampler2D splitSumTexture;
#ifdef ENV_CUBE
layout( set = 5, binding = 0 ) uniform samplerCube envTexture; // s3 $envmap
#else
layout( set = 5, binding = 0 ) uniform sampler2D lightmapTexture; // map LMAP atlas
#endif
layout( set = 6, binding = 0 ) uniform PixelShaderConstants
{
	vec4 c[32];
}
ps;
layout( push_constant ) uniform Constants
{
	mat4 viewProj;
	vec4 texXform0;
	vec4 texXform1;
	vec4 eyePos;
	vec4 params;
	vec4 params2;
	vec4 clipPlanes[2];
}
consts;

#include "pbr_brdf.glsl"
#ifndef ENV_CUBE
#include "world_pbr_probe.glsl"
#endif
#ifdef PROBE_VOLUME
layout( set = 7, binding = 0 ) uniform sampler2D probeAtlas; // PRBV atlas, RGBA16F
layout( set = 8, binding = 0 ) uniform sampler2D probeGrids; // grid table, RGBA32F
#include "probe_volume.glsl"
#endif

const int kNormalMap = 1;
const int kEmission = 2;
const int kEnvMap = 4;
const int kMapProbe = 8;
const int kClearCoat = 32;

vec3 SrgbToLinear( vec3 c )
{
	return mix( c / 12.92, pow( ( c + 0.055 ) / 1.055, vec3( 2.4 ) ), step( 0.04045, c ) );
}
vec3 LinearToSrgb( vec3 c )
{
	c = clamp( c, 0.0, 1.0 );
	return mix( c * 12.92, 1.055 * pow( c, vec3( 1.0 / 2.4 ) ) - 0.055, step( 0.0031308, c ) );
}

// cAmbientCube: c4..c9, as skin.frag's PixelShaderAmbientLight.
vec3 AmbientCube( vec3 n )
{
	vec3 nSquared = n * n;
	vec3 isNegative = mix( nSquared, vec3( 0.0 ), greaterThanEqual( n, vec3( 0.0 ) ) );
	vec3 isPositive = mix( vec3( 0.0 ), nSquared, greaterThanEqual( n, vec3( 0.0 ) ) );
	return isPositive.x * ps.c[4].xyz + isNegative.x * ps.c[5].xyz + isPositive.y * ps.c[6].xyz +
	       isNegative.y * ps.c[7].xyz + isPositive.z * ps.c[8].xyz + isNegative.z * ps.c[9].xyz;
}

// The diffuse light arriving along n: the probe volume's `layer` (0 total,
// 1 indirect) where it covers this pixel, else the ambient cube.
vec3 DiffuseLight( vec3 n, int layer )
{
#ifdef PROBE_VOLUME
	vec3 probe;
	if ( consts.params2.w > 0.5 &&
	     ProbeIrradiance( vWorldPos, n, layer, consts.params2.w < 1.5, probe ) )
		return probe;
#endif
	return AmbientCube( n );
}

// cLightInfo: c20..c25, the fourth light spread across the w components.
vec3 LightColor( int n )
{
	if ( n == 3 )
		return vec3( ps.c[20].w, ps.c[21].w, ps.c[22].w );
	return ps.c[20 + 2 * n].xyz;
}
vec3 LightPosition( int n )
{
	if ( n == 3 )
		return vec3( ps.c[23].w, ps.c[24].w, ps.c[25].w );
	return ps.c[21 + 2 * n].xyz;
}

// Specular image light in `direction` at `roughness`.
vec3 EnvironmentRadiance( vec3 direction, float roughness, int flags )
{
	vec3 environment;
#ifdef ENV_CUBE
	float mips = max( ps.c[2].w, 1.0 );
	environment = textureLod( envTexture, direction, roughness * ( mips - 1.0 ) ).rgb;
#else
	if ( ( flags & kMapProbe ) == 0 || !ProbeRadiance( direction, roughness, environment ) )
		environment = AmbientCube( direction );
#endif
	return environment;
}

void main()
{
	const int flags = int( consts.params.y );
	const int colorFlags = int( consts.params.z );
	const int numLights = int( consts.params2.x );
	const float lightAtten[4] =
	    float[4]( vLightAtten.x, vLightAtten.y, vLightAtten.z, vLightAtten.w );

	vec4 baseSample = texture( baseTexture, vBaseUv );
	// The D3D9 fixed-function alpha test, GREATEREQUAL.
	if ( consts.params.x >= 0.0 && baseSample.a < consts.params.x )
		discard;
	vec3 base = baseSample.rgb;
	if ( ( colorFlags & 1 ) != 0 )
		base = SrgbToLinear( base );
	vec3 mrao = texture( mraoTexture, vBaseUv ).rgb;
	float metalness = clamp( mrao.r, 0.0, 1.0 );
	float roughness = max( mrao.g, 0.02 );
	float occlusion = clamp( mrao.b, 0.0, 1.0 );

	vec3 view = normalize( vWorldVertToEye );
	// A mesh without normals (a screen-space rectangle) faces the eye rather
	// than shading with a NaN normal.
	vec3 normal = dot( vNormal, vNormal ) > 1e-12 ? normalize( vNormal ) : view;
	if ( ( flags & kNormalMap ) != 0 )
	{
		vec2 xy = texture( normalTexture, vBaseUv ).rg * 2.0 - 1.0;
		vec3 mapped = vec3( xy, sqrt( max( 0.0, 1.0 - dot( xy, xy ) ) ) );
		normal = normalize( normalize( vTangentS ) * mapped.x +
		                    normalize( vTangentT ) * mapped.y + normal * mapped.z );
	}
	float normalDotView = max( dot( normal, view ), 0.0 );
	vec3 f0 = mix( vec3( 0.04 ), base, metalness );
	vec2 splitSum = PbrSplitSum( splitSumTexture, normalDotView, roughness );
	// Multiple scattering (pbr_brdf.glsl): the compensated lobe's albedo
	// weights image light and leaves the rest to the diffuse layer.
	vec3 compensation = PbrEnergyCompensation( f0, splitSum );
	vec3 directionalAlbedo = PbrDirectionalAlbedo( f0, splitSum );
	vec3 diffuseColor = base * ( 1.0 - metalness ) * ( vec3( 1.0 ) - directionalAlbedo );
#ifdef INDIRECT_VIEW
	{
		vec3 viewed = DiffuseLight( normal, 1 );
		if ( consts.params2.y > 1.5 )
			viewed *= base * ( 1.0 - metalness ) * occlusion;
		viewed *= consts.params2.z;
		if ( ( colorFlags & 4 ) != 0 )
			viewed = LinearToSrgb( viewed );
		outColor = vec4( viewed, baseSample.a );
		return;
	}
#endif

	const bool clearCoat = ( flags & kClearCoat ) != 0;
	const float coat = clearCoat ? ps.c[2].y : 0.0;
	const float coatRoughness = max( ps.c[2].z, 0.02 );
	vec3 coatNormal = dot( vNormal, vNormal ) > 1e-12 ? normalize( vNormal ) : view;
	const float coatNormalDotView = max( dot( coatNormal, view ), 0.0 );

	// Image light (the ambient cube's Lambertian return, occluded, and the
	// specular environment below) and the local lights accumulate apart: the
	// coat attenuates each once, with its own Fresnel.
	vec3 indirect = diffuseColor * DiffuseLight( normal, 0 ) * occlusion;
	vec3 direct = vec3( 0.0 );

	// Local lights through the full BRDF: radiance pi * color * attenuation.
	for ( int i = 0; i < 4; ++i )
	{
		if ( i >= numLights )
			break;
		vec3 light = normalize( LightPosition( i ) - vWorldPos );
		float normalDotLight = max( dot( normal, light ), 0.0 );
		if ( normalDotLight <= 0.0 )
			continue;
		vec3 incident = LightColor( i ) * lightAtten[i];
		vec3 lighting = diffuseColor * incident * normalDotLight;
		lighting += kPi * incident * PbrSpecular( normal, view, light, f0, roughness ) *
		            compensation * normalDotLight;
		if ( clearCoat )
		{
			// The coat (pbr_brdf.glsl, after Filament's standard model) on the
			// geometric normal; the base beneath is attenuated by 1 - Fc.
			vec3 halfVector = normalize( view + light );
			vec2 coatLobe = PbrClearCoat( coat, max( dot( coatNormal, halfVector ), 0.0 ),
			    max( dot( light, halfVector ), 0.0 ), coatRoughness );
			lighting = lighting * ( 1.0 - coatLobe.y ) +
			           kPi * incident * coatLobe.x * max( dot( coatNormal, light ), 0.0 );
		}
		direct += lighting;
	}

	// Specular image lighting (split sum).
	vec3 environment = EnvironmentRadiance( reflect( -view, normal ), roughness, flags );
	indirect += environment * directionalAlbedo * occlusion;
	if ( clearCoat )
	{
		// The coat's image light over the attenuated base image light.
		float coatFresnel = PbrFresnelSchlick( 0.04, coatNormalDotView ) * coat;
		indirect = indirect * ( 1.0 - coatFresnel ) +
		           EnvironmentRadiance( reflect( -view, coatNormal ), coatRoughness, flags ) *
		               coatFresnel * occlusion;
	}
	vec3 color = direct + indirect;

	if ( ( flags & kEmission ) != 0 )
		color += SrgbToLinear( texture( emissionTexture, vBaseUv ).rgb ) * ps.c[2].x;

	// FinalOutput( ..., TONEMAP_SCALE_LINEAR ).
	color *= consts.params.w;
	if ( ( colorFlags & 4 ) != 0 )
		color = LinearToSrgb( color );
	outColor = vec4( color, baseSample.a );
}
