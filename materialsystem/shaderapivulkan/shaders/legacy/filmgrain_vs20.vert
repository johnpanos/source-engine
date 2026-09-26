#version 450
// A port of stdshaders/filmgrain_vs20.fxc (the hsl_filmgrain passes): the
// position passes through; TEXCOORD1 is TEXCOORD0 scaled and biased by
// cFilmGrainOffset.
#include "legacy_screen_vs.glsl"

layout( location = 0 ) out vec2 InputImageCoord;
layout( location = 1 ) out vec2 FilmGrainCoord;

#define cFilmGrainOffset VS_C( 48 )

void main()
{
	gl_Position = LegacyClipSpace( vec4( inWorldPos, 1.0 ) );
	InputImageCoord = inTexCoord0;
	FilmGrainCoord = inTexCoord0 * cFilmGrainOffset.xy + cFilmGrainOffset.zw;
}
