#version 450
// Scene-derived WMSH PBR: Cycles diffuse-light bake plus a directional specular
// source, and split-sum specular from a map reflection probe when the LMAP
// atlas carries one. The native pixel fixture checks normal, metalness and
// roughness.
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

layout( push_constant ) uniform Constants
{
	mat4 mvp;
	vec4 eyePosition;
	vec4 lightDirection;
	vec4 lightRadiance;
	vec4 material; // x alpha cutoff (<0 disables), y normal-map enable
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

void main()
{
	vec4 baseSample = texture( baseTexture, fragUv );
	if ( consts.material.x >= 0.0 && baseSample.a < consts.material.x )
		discard;
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
	vec3 bakedDiffuse = texture( lightmapTexture, fragLightmapUv ).rgb;
	vec3 diffuse = base * ( 1.0 - metalness ) *
	    ( vec3( 1.0 ) - directionalAlbedo ) * bakedDiffuse * occlusion;
	vec3 specular = vec3( 0.0 );
	vec3 light = normalize( -consts.lightDirection.xyz );
	float normalDotLight = max( dot( normal, light ), 0.0 );
	if ( normalDotView > 0.0 && normalDotLight > 0.0 )
	{
		vec3 halfVector = normalize( view + light );
		float normalDotHalf = max( dot( normal, halfVector ), 0.0 );
		float viewDotHalf = max( dot( view, halfVector ), 0.0 );
		float alpha = roughness * roughness;
		float alphaSquared = alpha * alpha;
		float denominator = normalDotHalf * normalDotHalf * ( alphaSquared - 1.0 ) + 1.0;
		float distribution = alphaSquared / ( kPi * denominator * denominator );
		float lambdaView = sqrt( alphaSquared +
		    ( 1.0 - alphaSquared ) * normalDotView * normalDotView );
		float lambdaLight = sqrt( alphaSquared +
		    ( 1.0 - alphaSquared ) * normalDotLight * normalDotLight );
		float visibility = 0.5 /
		    ( normalDotView * lambdaLight + normalDotLight * lambdaView );
		float grazing = 1.0 - viewDotHalf;
		float grazing5 = grazing * grazing * grazing * grazing * grazing;
		vec3 fresnel = f0 + ( vec3( 1.0 ) - f0 ) * grazing5;
		specular = consts.lightRadiance.rgb * fresnel * distribution *
		    visibility * normalDotLight;
	}
	vec3 probe;
	if ( ProbeRadiance( reflect( -view, normal ), roughness, probe ) )
		specular += probe * directionalAlbedo * occlusion;
	outColor = vec4( diffuse + specular, baseSample.a );
}
