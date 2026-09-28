// render.material family `pbr` (RFC 0016 K4): the RFC 0007 layered
// metal/roughness BRDF (render/shaders/common/pbr_brdf.glsl, the one GLSL
// copy of public/render/pbr_brdf.h) under the view's model lighting: the
// model port's arithmetic (model_pbr.frag) without map probes, environment
// maps, the probe volume or clear coat, which the family does not claim yet.
// Units follow Source's model lighting: a local light is incident radiance
// pi * color * attenuation, the ambient cube a Lambertian return, and with
// no probe the cube in the reflected direction is the specular image light.
// The base and emission textures are sampled as sRGB and the target is sRGB;
// tone mapping belongs to the frame's post pass.
#version 450

#include "../../shaders/common/pbr_brdf.glsl"
#include "pbr_lighting.glsl"

layout( set = 0, binding = 0 ) uniform texture2D splitSumTexture;
layout( set = 0, binding = 1 ) uniform sampler splitSumSampler;
layout( set = 2, binding = 0 ) uniform Material
{
	vec4 flags; // x: $bumpmap, y: $emissiontexture, z: $emissionscale
} material;
layout( set = 2, binding = 1 ) uniform texture2D baseTexture;
layout( set = 2, binding = 2 ) uniform sampler baseSampler;
layout( set = 2, binding = 3 ) uniform texture2D mraoTexture;
layout( set = 2, binding = 4 ) uniform sampler mraoSampler;
layout( set = 2, binding = 5 ) uniform texture2D normalTexture;
layout( set = 2, binding = 6 ) uniform sampler normalSampler;
layout( set = 2, binding = 7 ) uniform texture2D emissionTexture;
layout( set = 2, binding = 8 ) uniform sampler emissionSampler;

layout( location = 0 ) in vec2 uv;
layout( location = 1 ) in vec4 lightAtten;
layout( location = 2 ) in vec3 worldPosition;
layout( location = 3 ) in vec3 worldNormal;
layout( location = 4 ) in vec3 tangentS;
layout( location = 5 ) in vec3 tangentT;
layout( location = 0 ) out vec4 outColor;

// PixelShaderAmbientLight: the faces weighted by the squared normal.
vec3 AmbientCube( vec3 n )
{
	const vec3 squared = n * n;
	const bvec3 positive = greaterThanEqual( n, vec3( 0.0 ) );
	return squared.x * ( positive.x ? lighting.cube[0] : lighting.cube[1] ).rgb +
	       squared.y * ( positive.y ? lighting.cube[2] : lighting.cube[3] ).rgb +
	       squared.z * ( positive.z ? lighting.cube[4] : lighting.cube[5] ).rgb;
}

void main()
{
	const vec4 baseSample = texture( sampler2D( baseTexture, baseSampler ), uv );
	const vec3 base = baseSample.rgb;
	const vec3 mrao = texture( sampler2D( mraoTexture, mraoSampler ), uv ).rgb;
	const float metalness = clamp( mrao.r, 0.0, 1.0 );
	const float roughness = max( mrao.g, 0.02 );
	const float occlusion = clamp( mrao.b, 0.0, 1.0 );

	const vec3 view = normalize( lighting.eye.xyz - worldPosition );
	vec3 normal = dot( worldNormal, worldNormal ) > 1e-12 ? normalize( worldNormal ) : view;
	if ( material.flags.x != 0.0 )
	{
		const vec2 xy = texture( sampler2D( normalTexture, normalSampler ), uv ).rg * 2.0 - 1.0;
		const vec3 mapped = vec3( xy, sqrt( max( 0.0, 1.0 - dot( xy, xy ) ) ) );
		normal = normalize( normalize( tangentS ) * mapped.x + normalize( tangentT ) * mapped.y +
		                    normal * mapped.z );
	}
	const float normalDotView = max( dot( normal, view ), 0.0 );
	const vec3 f0 = mix( vec3( 0.04 ), base, metalness );
	const vec2 splitSum = texture( sampler2D( splitSumTexture, splitSumSampler ),
	    PbrSplitSumCoordinate( vec2( textureSize( sampler2D( splitSumTexture, splitSumSampler ), 0 ) ),
	        normalDotView, roughness ) )
	                          .rg;
	const vec3 compensation = PbrEnergyCompensation( f0, splitSum );
	const vec3 directionalAlbedo = PbrDirectionalAlbedo( f0, splitSum );
	const vec3 diffuseColor = base * ( 1.0 - metalness ) * ( vec3( 1.0 ) - directionalAlbedo );

	vec3 color = diffuseColor * AmbientCube( normal ) * occlusion;
	const int count = int( lighting.eye.w );
	for ( int i = 0; i < 4; ++i )
	{
		if ( i >= count )
			break;
		// A directional light shines along its direction. (The port's pixel
		// constants place it 10,000 units from the lighting origin against
		// that direction, CommitPixelShaderLighting; the vertex term reads
		// the light's own position, which is 1 for it.)
		const vec3 light = lighting.lights[i].color.w > 0.5
		                       ? -normalize( lighting.lights[i].direction.xyz )
		                       : normalize( lighting.lights[i].position.xyz - worldPosition );
		const float normalDotLight = max( dot( normal, light ), 0.0 );
		if ( normalDotLight <= 0.0 )
			continue;
		const vec3 incident = lighting.lights[i].color.rgb * lightAtten[i];
		color += diffuseColor * incident * normalDotLight;
		color += kPi * incident * PbrSpecular( normal, view, light, f0, roughness ) *
		         compensation * normalDotLight;
	}
	color += AmbientCube( reflect( -view, normal ) ) * directionalAlbedo * occlusion;
	if ( material.flags.y != 0.0 )
		color += texture( sampler2D( emissionTexture, emissionSampler ), uv ).rgb * material.flags.z;
	outColor = vec4( color, baseSample.a );
}
