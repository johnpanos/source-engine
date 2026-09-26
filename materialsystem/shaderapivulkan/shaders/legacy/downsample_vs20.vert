#version 450
// A port of stdshaders/Downsample_vs20.fxc (Downsample, sfm_downsample and the
// Sample4x4 family): the position passes through, TEXCOORD0..3 are TEXCOORD0
// offset by vsTapOffs[0..3].
#include "legacy_screen_vs.glsl"

layout( location = 0 ) out vec2 coordTap0;
layout( location = 1 ) out vec2 coordTap1;
layout( location = 2 ) out vec2 coordTap2;
layout( location = 3 ) out vec2 coordTap3;

#define vsTapOffs_0 VS_C( 48 )
#define vsTapOffs_1 VS_C( 49 )
#define vsTapOffs_2 VS_C( 50 )
#define vsTapOffs_3 VS_C( 51 )

void main()
{
	gl_Position = LegacyClipSpace( vec4( inWorldPos, 1.0 ) );
	coordTap0 = inTexCoord0 + vsTapOffs_0.xy;
	coordTap1 = inTexCoord0 + vsTapOffs_1.xy;
	coordTap2 = inTexCoord0 + vsTapOffs_2.xy;
	coordTap3 = inTexCoord0 + vsTapOffs_3.xy;
}
