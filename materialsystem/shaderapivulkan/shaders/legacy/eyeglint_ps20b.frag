#version 450
// EyeGlint_dx9's pixel stage: a port of stdshaders/eyeglint_ps2x.fxc (ps20b, no
// combos), the procedural glint's gaussian spots.
// @legacy program=eyeglint ps=eyeglint_ps20b vs=eyeglint_vs20 vert=eyeglint_vs20
//         flags=object_position
#include "legacy_ps.glsl"

layout( location = 0 ) in vec2 tc;          // Interpolated coordinate of current texel
layout( location = 1 ) in vec2 glintCenter; // Uniform value containing center of glint
layout( location = 2 ) in vec3 glintColor;  // Uniform value of color of glint

float GlintGaussSpotCoefficient( vec2 d )
{
	return saturate( exp( -25.0 * dot( d, d ) ) );
}

void main()
{
	vec2 uv = tc - glintCenter; // This texel relative to glint center

	float intensity = GlintGaussSpotCoefficient( uv + vec2( -0.25, -0.25 ) ) +
	                  GlintGaussSpotCoefficient( uv + vec2( 0.25, -0.25 ) ) +
	                  5.0 * GlintGaussSpotCoefficient( uv ) +
	                  GlintGaussSpotCoefficient( uv + vec2( -0.25, 0.25 ) ) +
	                  GlintGaussSpotCoefficient( uv + vec2( 0.25, 0.25 ) );

	intensity *= 4.0 / 9.0;

	LegacyWrite( vec4( intensity * glintColor, 1.0 ) );
}
