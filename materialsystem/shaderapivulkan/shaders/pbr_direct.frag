#version 450
// RFC 0007 R47 synthetic direct-light PBR path. The sampled base color is
// linear after an sRGB texture view (set 0); MRAO is linear data (set 1).
// The draw supplies incident radiance and
// the four geometric dot products through the push block. This is the GPU
// counterpart of public/render/pbr_brdf.h; the pixel suite compares them.
// AO affects indirect lighting, so the MRAO B channel is intentionally absent
// from this direct-light term. Real scene lights, normal maps, and baked
// lighting follow in the full PBRMetalRough material path.

layout( location = 0 ) in vec2 fragUv;
layout( location = 0 ) out vec4 outColor;
layout( set = 0, binding = 0 ) uniform sampler2D baseTexture;
layout( set = 1, binding = 0 ) uniform sampler2D mraoTexture;
layout( set = 2, binding = 0 ) uniform sampler2D splitSumTexture;

// The vertex stage uses the same first five vectors. angles.z is in [0,1],
// so its legacy screen-space flag bit (32) is always clear in this variant.
layout( push_constant ) uniform Constants
{
	mat4 mvp;
	vec4 radiance;
	vec4 texXform0;
	vec4 texXform1;
	vec4 angles; // N.V, N.L, N.H, V.H
#ifdef CLIP_PLANES
	vec4 clipPlanes[2];
#endif
}
consts;

#include "pbr_brdf.glsl"

void main()
{
	float normalDotView = consts.angles.x;
	float normalDotLight = consts.angles.y;
	if ( normalDotView <= 0.0 || normalDotLight <= 0.0 )
	{
		outColor = vec4( 0.0, 0.0, 0.0, 1.0 );
		return;
	}
	vec3 base = texture( baseTexture, fragUv ).rgb;
	vec3 mrao = texture( mraoTexture, fragUv ).rgb;
	float roughness = max( mrao.g, 0.02 );
	vec3 f0 = mix( vec3( 0.04 ), base, mrao.r );
	vec2 splitSum = PbrSplitSum( splitSumTexture, normalDotView, roughness );
	vec3 diffuse =
	    base * ( 1.0 - mrao.r ) * ( vec3( 1.0 ) - PbrDirectionalAlbedo( f0, splitSum ) ) / kPi;
	vec3 specular = PbrSpecular( f0, normalDotView, normalDotLight, consts.angles.z,
	                    consts.angles.w, roughness ) *
	                PbrEnergyCompensation( f0, splitSum );
	outColor = vec4( consts.radiance.rgb * ( diffuse + specular ) * normalDotLight, 1.0 );
}
