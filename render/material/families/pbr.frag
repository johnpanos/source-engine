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
// The debug views and lighting-model controls (RFC 0014) come from
// debug_view.glsl; at their neutral values they are dead code. The local
// lights answer to the `clustered` term, the ambient cube to `probes` and the
// cube in the reflected direction to `ibl`.
#version 450

#include "../../shaders/common/pbr_brdf.glsl"
#include "../../shaders/common/debug_view.glsl"
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

// PixelShaderAmbientLight: the faces weighted by the squared normal. In the
// furnace (RFC 0014) every face is a uniform radiance of 1.
vec3 AmbientCube( vec3 n )
{
	const vec3 squared = n * n;
	if ( DebugFurnace() )
		return vec3( squared.x + squared.y + squared.z );
	const bvec3 positive = greaterThanEqual( n, vec3( 0.0 ) );
	return squared.x * ( positive.x ? lighting.cube[0] : lighting.cube[1] ).rgb +
	       squared.y * ( positive.y ? lighting.cube[2] : lighting.cube[3] ).rgb +
	       squared.z * ( positive.z ? lighting.cube[4] : lighting.cube[5] ).rgb;
}

void main()
{
	const bool furnace = DebugFurnace();
	const vec4 baseSample = texture( sampler2D( baseTexture, baseSampler ), uv );
	const vec3 base = furnace ? vec3( 1.0 ) : baseSample.rgb;
	const vec3 mrao = texture( sampler2D( mraoTexture, mraoSampler ), uv ).rgb;
	const float metalness =
	    kDebugForceMetalness >= 0.0 ? kDebugForceMetalness : clamp( mrao.r, 0.0, 1.0 );
	const float roughness =
	    max( kDebugForceRoughness >= 0.0 ? kDebugForceRoughness : mrao.g, 0.02 );
	const float occlusion = DebugTermOn( kDebugTermAo ) ? clamp( mrao.b, 0.0, 1.0 ) : 1.0;

	const vec3 view = normalize( lighting.eye.xyz - worldPosition );
	vec3 normal = dot( worldNormal, worldNormal ) > 1e-12 ? normalize( worldNormal ) : view;
	vec3 mapped = vec3( 0.0, 0.0, 1.0 );
	if ( material.flags.x != 0.0 )
	{
		const vec2 xy = texture( sampler2D( normalTexture, normalSampler ), uv ).rg * 2.0 - 1.0;
		mapped = vec3( xy, sqrt( max( 0.0, 1.0 - dot( xy, xy ) ) ) );
		normal = normalize( normalize( tangentS ) * mapped.x + normalize( tangentT ) * mapped.y +
		                    normal * mapped.z );
	}
	const float normalDotView = max( dot( normal, view ), 0.0 );
	const vec3 f0 = mix( vec3( 0.04 ), base, metalness );
	const vec2 splitSum = texture( sampler2D( splitSumTexture, splitSumSampler ),
	    PbrSplitSumCoordinate( vec2( textureSize( sampler2D( splitSumTexture, splitSumSampler ), 0 ) ),
	        normalDotView, roughness ) )
	                          .rg;
	// cl_render_debug_brdf 3: multiple-scattering compensation off.
	const bool compensate = kDebugBrdf != kDebugBrdfNoEnergyCompensation;
	const vec3 compensation = compensate ? PbrEnergyCompensation( f0, splitSum ) : vec3( 1.0 );
	const vec3 directionalAlbedo = compensate
	                                   ? PbrDirectionalAlbedo( f0, splitSum )
	                                   : min( vec3( 1.0 ), f0 * splitSum.x + vec3( splitSum.y ) );
	const vec3 diffuseColor = base * ( 1.0 - metalness ) * ( vec3( 1.0 ) - directionalAlbedo );
	// cl_render_debug_brdf 1 and 2: one lobe.
	const bool diffuseLobe = kDebugBrdf != kDebugBrdfSpecularOnly;
	const bool specularLobe = kDebugBrdf != kDebugBrdfDiffuseOnly;

	vec3 color = diffuseLobe && DebugTermOn( kDebugTermProbes )
	                 ? diffuseColor * AmbientCube( normal ) * occlusion
	                 : vec3( 0.0 );
	vec3 direct = vec3( 0.0 );
	const int count = DebugTermOn( kDebugTermClustered ) && !furnace ? int( lighting.eye.w ) : 0;
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
		if ( diffuseLobe )
		{
			const vec3 diffuse = diffuseColor * incident * normalDotLight;
			color += diffuse;
			direct += diffuse;
		}
		if ( specularLobe )
		{
			const vec3 specular = kPi * incident * PbrSpecular( normal, view, light, f0, roughness ) *
			                      compensation * normalDotLight;
			color += specular;
			direct += specular;
		}
	}
	vec3 imageSpecular = vec3( 0.0 );
	if ( specularLobe && DebugTermOn( kDebugTermIbl ) )
	{
		imageSpecular = AmbientCube( reflect( -view, normal ) ) * directionalAlbedo * occlusion;
		color += imageSpecular;
	}
	vec3 emission = vec3( 0.0 );
	if ( material.flags.y != 0.0 && DebugTermOn( kDebugTermEmission ) && !furnace )
	{
		emission =
		    texture( sampler2D( emissionTexture, emissionSampler ), uv ).rgb * material.flags.z;
		color += emission;
	}

	if ( DebugViewActive() )
	{
		DebugInputs inputs = DebugInputsNone();
		inputs.mask = kDebugHasAlbedo | kDebugHasNormal | kDebugHasRoughness | kDebugHasMetalness |
		              kDebugHasAo | kDebugHasDirect | kDebugHasImageSpecular | kDebugHasUv0;
		inputs.albedo = base;
		inputs.normal = normal;
		if ( material.flags.x != 0.0 )
		{
			inputs.mask |= kDebugHasNormalMap;
			inputs.normalMap = mapped;
		}
		inputs.roughness = roughness;
		inputs.metalness = metalness;
		inputs.ao = occlusion;
		inputs.direct = direct;
		inputs.imageSpecular = imageSpecular;
		if ( material.flags.y != 0.0 )
		{
			inputs.mask |= kDebugHasEmission;
			inputs.emission = emission;
		}
		inputs.uv0 = uv;
		inputs.final = color;
		outColor = DebugViewOutput( inputs );
		return;
	}
	// cl_render_debug_brdf 4: the split-sum table's sample as red and green.
	if ( kDebugBrdf == kDebugBrdfSplitSumSample )
	{
		outColor = vec4( splitSum, 0.0, 1.0 );
		return;
	}
	outColor = vec4( color, baseSample.a );
}
