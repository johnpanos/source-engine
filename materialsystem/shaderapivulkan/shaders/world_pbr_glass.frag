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

// world_pbr.frag's grouped sets: the frame's, then the material's, where
// the scene capture takes the emission and depth slots.
layout( set = 0, binding = 0 ) uniform sampler2D splitSumTexture;
layout( set = 0, binding = 1 ) uniform sampler2D lightmapTexture;
layout( set = 1, binding = 0 ) uniform sampler2D baseTexture;
layout( set = 1, binding = 1 ) uniform sampler2D mraoTexture;
layout( set = 1, binding = 2 ) uniform sampler2D normalTexture;
layout( set = 1, binding = 3 ) uniform sampler2D sceneColor;
layout( set = 1, binding = 5 ) uniform sampler2D sceneDepth;

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

// World distance from this fragment to the surface behind it along the view
// ray, from that surface's window depth: solve z(Q) / w(Q) = depth for
// Q = fragment + s * direction with the mvp's z and w rows. Without depth, or
// behind the far plane, a nominal 128 units (about 3 m).
float BehindDistance( float depth, bool depthValid )
{
	const float kNominal = 128.0;
	if ( !depthValid || depth >= 1.0 )
		return kNominal;
	vec3 direction = normalize( fragPosition - consts.eyePosition.xyz );
	vec4 rowZ = vec4( consts.mvp[0][2], consts.mvp[1][2], consts.mvp[2][2], consts.mvp[3][2] );
	vec4 rowW = vec4( consts.mvp[0][3], consts.mvp[1][3], consts.mvp[2][3], consts.mvp[3][3] );
	float a = dot( rowZ, vec4( fragPosition, 1.0 ) );
	float b = dot( rowZ.xyz, direction );
	float c = dot( rowW, vec4( fragPosition, 1.0 ) );
	float e = dot( rowW.xyz, direction );
	float denominator = b - depth * e;
	if ( abs( denominator ) < 1e-8 )
		return kNominal;
	return clamp( ( depth * c - a ) / denominator, 0.0, 16384.0 );
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
	vec2 splitSum = PbrSplitSum( splitSumTexture, normalDotView, roughness );
	// Directional reflectance of one interface (multiple scattering,
	// pbr_brdf.glsl). A thin sheet has two parallel interfaces; their
	// incoherent inter-reflections reflect 2E / (1 + E).
	float single = PbrDirectionalAlbedo( vec3( f0 ), splitSum ).x;
	float thickness = max( consts.glass.z, 0.0 );
	float reflectance = thickness > 0.0 ? single : 2.0 * single / ( 1.0 + single );

	vec3 reflected = vec3( 0.0 );
	vec3 probe;
	if ( MapProbeRadiance( fragPosition, normalize( fragNormal ), reflect( -view, normal ),
	         roughness, probe ) )
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
	bool depthValid = consts.glass.w >= 0.5;
	float behindDepth = depthValid ? textureLod( sceneDepth, uv, 0.0 ).r : 1.0;
	if ( depthValid && behindDepth < gl_FragCoord.z )
	{
		uv = gl_FragCoord.xy * consts.capture.xy;
		behindDepth = textureLod( sceneDepth, uv, 0.0 ).r;
	}
	// Rough glass scatters what it transmits. The microfacet normals spread
	// about atan(alpha); by Snell's law (small angles) entering glass deflects
	// a ray by (1 - 1/n) of the tilt and leaving it by (n - 1), so a thin sheet
	// spreads light by (n - 1/n) and a solid's first surface by (1 - 1/n).
	// The blur is that angle over the distance from the glass to what is
	// behind it, as seen from the eye.
	float alpha = roughness * roughness;
	float spread = atan( alpha ) * ( thickness > 0.0 ? 1.0 - 1.0 / ior : ior - 1.0 / ior );
	float behindDistance = BehindDistance( behindDepth, depthValid );
	vec3 across = normalize( cross( view, abs( view.z ) < 0.9 ? vec3( 0, 0, 1 ) : vec3( 1, 0, 0 ) ) );
	vec3 behindPoint = fragPosition - view * behindDistance;
	float radius = length( ScreenOffset( behindPoint,
	    behindPoint + across * ( behindDistance * tan( min( spread, 1.2 ) ) ) ) );
	// A mip level averages 2^level pixels: the footprint is twice the radius.
	float lod = clamp( log2( max( 2.0 * radius, 1.0 ) ), 0.0, max( consts.material.z - 1.0, 0.0 ) );
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
