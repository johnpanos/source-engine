#version 450
// RFC 0007 transmissive PBRMetalRough (glass) on WMSH batches. The scene
// behind the glass was captured just before this draw
// (CVulkanContext::RecordSceneCapture): a color mip chain, sampled through its
// sRGB view so it reads linear, and the depth buffer. Light leaving the glass
// is the dielectric interface reflection (split-sum energy, map reflection
// probe) plus the refracted, tinted and roughness-blurred scene behind it.
// Scene depth rejects refracted samples that land on geometry in front of the
// glass. The output replaces the pixel; nothing is alpha blended.
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
layout( set = 5, binding = 0 ) uniform sampler2D sceneColor;
layout( set = 6, binding = 0 ) uniform sampler2D sceneDepth;

// The same size and vertex-visible layout as world_pbr's block (the vertex
// stage is world_pbr.vert, which reads only mvp and the clip planes).
layout( push_constant ) uniform Constants
{
	mat4 mvp;
	vec4 eyePosition;
	// x transmission (0..1), y index of refraction, z thickness in world units
	// (0 = a thin sheet), w 1 when sceneDepth holds this view's depth.
	vec4 glass;
	// xy 1 / the captured color size in pixels, zw half the viewport size.
	vec4 capture;
	// x alpha cutoff (<0 disables), y normal-map enable, z scene color mips.
	vec4 material;
#ifdef CLIP_PLANES
	vec4 clipPlanes[2];
#endif
}
consts;

#include "world_pbr_probe.glsl"

// Window position of a world point, relative to this fragment's.
vec2 ScreenOffset( vec3 from, vec3 to )
{
	vec4 a = consts.mvp * vec4( from, 1.0 );
	vec4 b = consts.mvp * vec4( to, 1.0 );
	if ( a.w <= 0.0 || b.w <= 0.0 )
		return vec2( 0.0 );
	// The draw viewport is flipped (negative height) to keep D3D's +Y up.
	return ( b.xy / b.w - a.xy / a.w ) * vec2( consts.capture.z, -consts.capture.w );
}

void main()
{
	vec4 baseSample = texture( baseTexture, fragUv );
	if ( consts.material.x >= 0.0 && baseSample.a < consts.material.x )
		discard;
	vec3 tint = baseSample.rgb;
	vec3 mrao = texture( mraoTexture, fragUv ).rgb;
	float roughness = max( mrao.g, 0.02 );
	float occlusion = clamp( mrao.b, 0.0, 1.0 );
	vec3 normal = normalize( fragNormal );
	if ( consts.material.y >= 0.5 )
		normal = MappedNormal( normal, fragTangent, texture( normalTexture, fragUv ).rg );
	vec3 view = normalize( consts.eyePosition.xyz - fragPosition );
	// Glass is seen from either side.
	if ( dot( normal, view ) < 0.0 )
		normal = -normal;
	float normalDotView = max( dot( normal, view ), 1e-4 );

	float ior = max( consts.glass.y, 1.0 );
	float f0 = ( ior - 1.0 ) / ( ior + 1.0 );
	f0 *= f0;
	vec2 splitSum = texture( splitSumTexture,
	    clamp( vec2( normalDotView, roughness ), vec2( 0.0 ), vec2( 1.0 ) ) ).rg;
	// Directional reflectance of one interface. A thin sheet has two parallel
	// interfaces; their incoherent inter-reflections reflect 2E / (1 + E).
	float single = min( 1.0, f0 * splitSum.x + splitSum.y );
	float thickness = max( consts.glass.z, 0.0 );
	float reflectance = thickness > 0.0 ? single : 2.0 * single / ( 1.0 + single );

	vec3 reflected = vec3( 0.0 );
	vec3 probe;
	if ( ProbeRadiance( reflect( -view, normal ), roughness, probe ) )
		reflected = probe * reflectance * occlusion;

	// A thin sheet passes light straight through. Solid glass bends it: the
	// refracted ray crosses `thickness` along the normal before it leaves.
	vec2 pixel = gl_FragCoord.xy;
	if ( thickness > 0.0 )
	{
		vec3 refracted = refract( -view, normal, 1.0 / ior );
		float cosine = max( dot( -normal, refracted ), 0.1 );
		pixel += ScreenOffset( fragPosition, fragPosition + refracted * ( thickness / cosine ) );
	}
	vec2 uv = clamp( pixel * consts.capture.xy, vec2( 0.0 ), vec2( 1.0 ) );
	// A refracted sample of something nearer than the glass would show an
	// object in front of it through it; take the undisplaced pixel instead.
	if ( consts.glass.w >= 0.5 && textureLod( sceneDepth, uv, 0.0 ).r < gl_FragCoord.z )
		uv = gl_FragCoord.xy * consts.capture.xy;
	// Rough glass scatters the transmitted light: blur by the GGX alpha, up to
	// a quarter of the view height at alpha 1.
	float alpha = roughness * roughness;
	float radius = alpha * consts.capture.w * 0.5;
	float lod = clamp( log2( max( radius, 1.0 ) ), 0.0, max( consts.material.z - 1.0, 0.0 ) );
	vec3 behind = textureLod( sceneColor, uv, lod ).rgb;
	vec3 transmitted = behind * tint * ( 1.0 - reflectance );

	// The untransmitted part is an ordinary baked dielectric.
	float transmission = clamp( consts.glass.x, 0.0, 1.0 );
	vec3 diffuse = vec3( 0.0 );
	if ( transmission < 1.0 )
		diffuse = tint * ( 1.0 - single ) *
		          texture( lightmapTexture, fragLightmapUv ).rgb * occlusion;
	outColor = vec4( reflected + mix( diffuse, transmitted, transmission ), 1.0 );
}
