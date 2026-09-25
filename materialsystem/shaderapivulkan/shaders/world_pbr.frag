#version 450
// Scene-derived WMSH PBR: Cycles diffuse-light bake plus a directional specular
// source, and split-sum specular from a map reflection probe when the LMAP
// atlas carries one, or from the material's $envmap cube. $emissiontexture
// (sRGB, decoded here) adds its color times $emissionscale. The native pixel
// fixture checks normal, metalness, roughness, emission and the environment.
//
// RUNTIME_INDIRECT (RFC 0011 render.indirect-policy.v1's RuntimeIndirect):
// the bound lightmap is the bake's direct layer, and the producer's indirect
// light is added from its atlas (set 8), sampled at the lightmap coordinate.
//
// DIRECT_LIGHTS (RFC 0011 G2) adds the frame's unbaked lights, from the
// engine's light set (render/light_set.h), through the legacy dlight falloff
// times the Lambert cosine and the layered BRDF (set 7). The push block's
// lightDirection / lightRadiance directional light is the pixel suite's
// hook; the engine never sets it.
//
// INDIRECT_VIEW (RFC 0011 debug view) replaces the shading with the indirect
// light alone, for comparison with Cycles' DiffInd pass: the bound lightmap
// is then the map's LMAP indirect layer, and the push block's lightDirection
// is ( view, exposure scale, 1 when that layer is bound, 0 ). View 1 writes
// the indirect diffuse light (irradiance / pi, no albedo), view 2 the
// indirect diffuse radiance (times the diffuse albedo and occlusion).
layout( location = 0 ) in vec2 fragUv;
layout( location = 1 ) in vec2 fragLightmapUv;
layout( location = 2 ) in vec3 fragPosition;
layout( location = 3 ) in vec3 fragNormal;
layout( location = 4 ) in vec4 fragTangent;
layout( location = 0 ) out vec4 outColor;

layout( set = 0, binding = 0 ) uniform sampler2D baseTexture;
layout( set = 1, binding = 0 ) uniform sampler2D mraoTexture;
layout( set = 2, binding = 0 ) uniform sampler2D normalTexture;
layout( set = 3, binding = 0 ) uniform sampler2D lightmapTexture;
layout( set = 4, binding = 0 ) uniform sampler2D splitSumTexture;
layout( set = 5, binding = 0 ) uniform sampler2D emissionTexture;
layout( set = 6, binding = 0 ) uniform samplerCube environmentTexture;

layout( push_constant ) uniform Constants
{
	mat4 mvp;
	vec4 eyePosition;   // w: $clearcoat (0: no coat)
	vec4 lightDirection;
	vec4 lightRadiance; // w: $clearcoatroughness
	// x alpha cutoff (<0 disables), y normal-map enable, z $emissionscale
	// (0: no emission), w the $envmap cube's mip count (0: no cube)
	vec4 material;
#ifdef CLIP_PLANES
	vec4 clipPlanes[2];
#endif
}
consts;

#include "world_pbr_probe.glsl"

vec3 SurfaceNormal()
{
	vec3 normal = normalize( fragNormal );
	if ( consts.material.y < 0.5 )
		return normal;
	return MappedNormal( normal, fragTangent, texture( normalTexture, fragUv ).rg );
}

// A 2:1 LMAP page is directional (tools/quality/lightmap_directional.py): the
// flat irradiance E0 baked on the smooth normal N on the left, and at the same
// texel on the right the world-space luminance gradient beta of the fitted
// irradiance E(n) = a + g.n, relative to E0. A normal-mapped normal n receives
// E0 * (1 + beta.(n - N)); n = N reproduces the flat bake exactly. Samples are
// clamped to their half so linear filtering never mixes the two.
vec3 BakedIrradiance( vec3 normal )
{
	ivec2 size = textureSize( lightmapTexture, 0 );
	if ( size.x != 2 * size.y )
		return texture( lightmapTexture, fragLightmapUv ).rgb;
	float halfTexel = 0.5 / float( size.x );
	float u = clamp( fragLightmapUv.x * 0.5, halfTexel, 0.5 - halfTexel );
	vec3 irradiance = texture( lightmapTexture, vec2( u, fragLightmapUv.y ) ).rgb;
	vec3 beta = texture( lightmapTexture, vec2( u + 0.5, fragLightmapUv.y ) ).rgb;
	float gain = 1.0 + dot( beta, normal - normalize( fragNormal ) );
	return irradiance * clamp( gain, 0.0, 4.0 );
}

// Specular image light: the material's $envmap cube, else the map probe.
bool ImageRadiance( vec3 direction, float roughness, out vec3 radiance )
{
	if ( consts.material.w >= 1.0 )
	{
		radiance = textureLod( environmentTexture, direction,
		    roughness * ( consts.material.w - 1.0 ) ).rgb;
		return true;
	}
	return ProbeRadiance( direction, roughness, radiance );
}

// GGX specular (height-correlated Smith visibility, Schlick Fresnel) for a
// light from `light`, without the cosine.
vec3 SpecularBrdf( vec3 normal, vec3 view, vec3 light, vec3 f0, float roughness )
{
	float normalDotView = max( dot( normal, view ), 0.0 );
	float normalDotLight = max( dot( normal, light ), 0.0 );
	vec3 halfVector = normalize( view + light );
	float normalDotHalf = max( dot( normal, halfVector ), 0.0 );
	float viewDotHalf = max( dot( view, halfVector ), 0.0 );
	float alpha = roughness * roughness;
	float alphaSquared = alpha * alpha;
	float denominator = normalDotHalf * normalDotHalf * ( alphaSquared - 1.0 ) + 1.0;
	float distribution = alphaSquared / ( kPi * denominator * denominator );
	float lambdaView =
	    sqrt( alphaSquared + ( 1.0 - alphaSquared ) * normalDotView * normalDotView );
	float lambdaLight =
	    sqrt( alphaSquared + ( 1.0 - alphaSquared ) * normalDotLight * normalDotLight );
	float visibility = 0.5 / ( normalDotView * lambdaLight + normalDotLight * lambdaView );
	float grazing = 1.0 - viewDotHalf;
	float grazing5 = grazing * grazing * grazing * grazing * grazing;
	vec3 fresnel = f0 + ( vec3( 1.0 ) - f0 ) * grazing5;
	return fresnel * distribution * visibility;
}

#ifdef RUNTIME_INDIRECT
layout( set = 8, binding = 0 ) uniform sampler2D producerIndirect;
#endif

#ifdef DIRECT_LIGHTS
// Four vec4 per light: position.xyz, radius; color.rgb, minLight;
// direction.xyz, outer cone cosine (below -1: no cone); inner cone cosine.
layout( set = 7, binding = 0 ) uniform DirectLights
{
	vec4 header; // x: the light count
	vec4 lights[28];
}
directLights;

// render/light_set.h Falloff(): the legacy dlight falloff.
float DynamicFalloff( float distanceSquared, float radius, float minLight )
{
	float radiusSquared = radius * radius;
	if ( !( radiusSquared > 0.0 ) || distanceSquared >= radiusSquared )
		return 0.0;
	float scale = distanceSquared > 0.0 ? radiusSquared * minLight / distanceSquared : 1.0;
	scale *= 1.0 - distanceSquared / radiusSquared;
	return min( scale, 2.0 );
}

// The direct light the frame's unbaked lights return toward the eye: the
// diffuse albedo times their diffuse light (the bake's unit), plus pi times
// their incident light through the specular lobe, as model_pbr.frag lights.
vec3 DirectLightRadiance( vec3 normal, vec3 view, vec3 diffuseAlbedo, vec3 f0, float roughness )
{
	vec3 total = vec3( 0.0 );
	int count = int( directLights.header.x );
	for ( int i = 0; i < 7; ++i )
	{
		if ( i >= count )
			break;
		vec4 positionRadius = directLights.lights[4 * i];
		vec4 colorMinLight = directLights.lights[4 * i + 1];
		vec4 directionOuter = directLights.lights[4 * i + 2];
		float innerCos = directLights.lights[4 * i + 3].x;
		vec3 toLight = positionRadius.xyz - fragPosition;
		float distanceSquared = dot( toLight, toLight );
		float falloff = DynamicFalloff( distanceSquared, positionRadius.w, colorMinLight.w );
		if ( falloff <= 0.0 )
			continue;
		vec3 light = toLight * inversesqrt( max( distanceSquared, 1e-8 ) );
		if ( directionOuter.w >= -1.0 )
		{
			float cosine = dot( -light, normalize( directionOuter.xyz ) );
			falloff *= innerCos > directionOuter.w + 1e-4
			               ? smoothstep( directionOuter.w, innerCos, cosine )
			               : step( directionOuter.w, cosine );
		}
		float normalDotLight = max( dot( normal, light ), 0.0 );
		if ( falloff <= 0.0 || normalDotLight <= 0.0 )
			continue;
		vec3 incident = colorMinLight.rgb * falloff;
		vec3 lit = diffuseAlbedo * incident * normalDotLight;
		if ( dot( normal, view ) > 0.0 )
			lit += kPi * incident * SpecularBrdf( normal, view, light, f0, roughness ) *
			       normalDotLight;
		total += lit;
	}
	return total;
}
#endif

// Clear coat, after Filament's standard model (Apache-2.0, google/filament
// shaders/src/surface_shading_model_standard.fs): IOR 1.5 (F0 0.04), GGX with
// Kelemen visibility, on the geometric normal; the base beneath is attenuated
// by 1 - Fc.
float SchlickF( float f0, float cosine )
{
	float grazing = 1.0 - cosine;
	float grazing2 = grazing * grazing;
	return f0 + ( 1.0 - f0 ) * grazing2 * grazing2 * grazing;
}

void main()
{
	vec4 baseSample = texture( baseTexture, fragUv );
	if ( consts.material.x >= 0.0 && baseSample.a < consts.material.x )
		discard;
#ifdef INDIRECT_VIEW
	{
		vec3 indirectLight =
		    consts.lightDirection.z > 0.5 ? BakedIrradiance( normalize( fragNormal ) ) : vec3( 0.0 );
		vec3 viewed = indirectLight;
		if ( consts.lightDirection.x > 1.5 )
		{
			vec3 viewMrao = texture( mraoTexture, fragUv ).rgb;
			viewed *= baseSample.rgb * ( 1.0 - clamp( viewMrao.r, 0.0, 1.0 ) ) *
			          clamp( viewMrao.b, 0.0, 1.0 );
		}
		outColor = vec4( viewed * consts.lightDirection.y, baseSample.a );
		return;
	}
#endif
	vec3 base = baseSample.rgb;
	vec3 mrao = texture( mraoTexture, fragUv ).rgb;
	float metalness = clamp( mrao.r, 0.0, 1.0 );
	float roughness = max( mrao.g, 0.02 );
	float occlusion = clamp( mrao.b, 0.0, 1.0 );
	vec3 normal = SurfaceNormal();
	vec3 view = normalize( consts.eyePosition.xyz - fragPosition );
	float normalDotView = max( dot( normal, view ), 0.0 );
	vec3 f0 = mix( vec3( 0.04 ), base, metalness );
	vec2 splitSum = texture( splitSumTexture,
	    clamp( vec2( normalDotView, roughness ), vec2( 0.0 ), vec2( 1.0 ) ) ).rg;
	vec3 directionalAlbedo = min( vec3( 1.0 ), f0 * splitSum.x + vec3( splitSum.y ) );
	// Cycles DIFFUSE DIRECT+INDIRECT with COLOR disabled already contains the
	// Lambertian 1/pi factor. Multiplying this bake by albedo must not divide
	// it by pi again.
	vec3 bakedDiffuse = BakedIrradiance( normal );
#ifdef RUNTIME_INDIRECT
	bakedDiffuse += texture( producerIndirect, fragLightmapUv ).rgb;
#endif
	vec3 diffuse = base * ( 1.0 - metalness ) *
	    ( vec3( 1.0 ) - directionalAlbedo ) * bakedDiffuse * occlusion;
	vec3 specular = vec3( 0.0 );
	vec3 light = normalize( -consts.lightDirection.xyz );
	float normalDotLight = max( dot( normal, light ), 0.0 );
	if ( normalDotView > 0.0 && normalDotLight > 0.0 )
		specular = consts.lightRadiance.rgb * SpecularBrdf( normal, view, light, f0, roughness ) *
		           normalDotLight;
	float coat = consts.eyePosition.w;
	vec3 coatNormal = normalize( fragNormal );
	if ( coat > 0.0 )
	{
		float coatRoughness = max( consts.lightRadiance.w, 0.02 );
		float coatAlpha = coatRoughness * coatRoughness;
		float coatAlphaSquared = coatAlpha * coatAlpha;
		vec3 halfVector = normalize( view + light );
		float lightDotHalf = max( dot( light, halfVector ), 0.0 );
		float coatNormalDotLight = max( dot( coatNormal, light ), 0.0 );
		float coatNormalDotHalf = max( dot( coatNormal, halfVector ), 0.0 );
		float denominator =
		    coatNormalDotHalf * coatNormalDotHalf * ( coatAlphaSquared - 1.0 ) + 1.0;
		float coatLightFresnel = SchlickF( 0.04, lightDotHalf ) * coat;
		specular = specular * ( 1.0 - coatLightFresnel ) +
		           consts.lightRadiance.rgb * coatAlphaSquared /
		               ( kPi * denominator * denominator ) *
		               ( 0.25 / max( lightDotHalf * lightDotHalf, 1e-6 ) ) * coatLightFresnel *
		               coatNormalDotLight;
	}
	vec3 probe;
	vec3 reflected = reflect( -view, normal );
	// Specular horizon occlusion: a normal-mapped normal that faces away from
	// the viewer reflects it below the geometric surface, where no light
	// arrives (Cycles bends such normals back; without this, every groove
	// facing away from the camera mirrored the probe at Fresnel 1).
	float horizon = clamp( 1.0 + 1.3 * dot( reflected, normalize( fragNormal ) ), 0.0, 1.0 );
	float probeWeight = occlusion * horizon * horizon;
	vec3 image = diffuse;
	if ( ImageRadiance( reflected, roughness, probe ) )
		image += probe * directionalAlbedo * probeWeight;
	if ( coat > 0.0 )
	{
		// The baked and image light beneath the coat, attenuated by its
		// view-angle Fresnel, then the coat's own image light (its geometric
		// normal needs no horizon term).
		float coatFresnel = SchlickF( 0.04, max( dot( coatNormal, view ), 0.0 ) ) * coat;
		image *= 1.0 - coatFresnel;
		if ( ImageRadiance( reflect( -view, coatNormal ), max( consts.lightRadiance.w, 0.02 ),
		         probe ) )
			image += probe * coatFresnel * occlusion;
	}
	vec3 emission = vec3( 0.0 );
	if ( consts.material.z > 0.0 )
	{
		vec3 encoded = texture( emissionTexture, fragUv ).rgb;
		emission = mix( encoded / 12.92, pow( ( encoded + 0.055 ) / 1.055, vec3( 2.4 ) ),
		               step( 0.04045, encoded ) ) *
		           consts.material.z;
	}
#ifdef DIRECT_LIGHTS
	specular += DirectLightRadiance(
	    normal, view, base * ( 1.0 - metalness ) * ( vec3( 1.0 ) - directionalAlbedo ), f0, roughness );
#endif
	outColor = vec4( image + specular + emission, baseSample.a );
}
