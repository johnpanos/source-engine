#version 450
// ColorCorrection's pixel stage: a port of stdshaders/colorcorrection_ps2x.fxc
// (ps20b): the base texture weighted, plus up to four 32^3 lookup volumes read
// at the base color. Combos (fxctmp9/colorcorrection_ps20b.inc): static
// CONVERT_TO_SRGB (5, always 0 here); dynamic NUM_LOOKUPS (1, 0..4).
// @legacy program=colorcorrection ps=colorcorrection_ps20b vs=screenspaceeffect_vs20
//         vert=screenspaceeffect_vs20 samplers=0:2d,1:3d,2:3d,3:3d,4:3d flags=object_position
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler;            // s0
layout( set = 0, binding = 1 ) uniform sampler3D ColorCorrectionVolumeTexture0; // s1
layout( set = 0, binding = 2 ) uniform sampler3D ColorCorrectionVolumeTexture1; // s2
layout( set = 0, binding = 3 ) uniform sampler3D ColorCorrectionVolumeTexture2; // s3
layout( set = 0, binding = 4 ) uniform sampler3D ColorCorrectionVolumeTexture3; // s4

layout( location = 0 ) in vec2 baseTexCoord;

#define ColorCorrectionDefaultWeight ( PS_C( 0 ).x )
#define ColorCorrectionVolumeWeight0 ( PS_C( 1 ).x )
#define ColorCorrectionVolumeWeight1 ( PS_C( 2 ).x )
#define ColorCorrectionVolumeWeight2 ( PS_C( 3 ).x )
#define ColorCorrectionVolumeWeight3 ( PS_C( 4 ).x )

void main()
{
	const int NUM_LOOKUPS = DYNAMIC_PS_COMBO( 1, 5 );

	vec4 baseColor = tex2D( 0, BaseTextureSampler, baseTexCoord );

	// (0,0,0) reads the lookup at 0.5 / 32 and (1,1,1) at 31.5 / 32.
	vec4 offsetBaseColor = baseColor * ( 31.0 / 32.0 ) + ( 0.5 / 32.0 );

	vec4 outColor = vec4( 0.0, 0.0, 0.0, baseColor.a );
	outColor.rgb = baseColor.rgb * ColorCorrectionDefaultWeight;
	if ( NUM_LOOKUPS > 0 )
	{
		outColor.rgb += tex3D( 1, ColorCorrectionVolumeTexture0, offsetBaseColor.rgb ).rgb *
		                ColorCorrectionVolumeWeight0;
		if ( NUM_LOOKUPS > 1 )
		{
			outColor.rgb += tex3D( 2, ColorCorrectionVolumeTexture1, offsetBaseColor.rgb ).rgb *
			                ColorCorrectionVolumeWeight1;
			if ( NUM_LOOKUPS > 2 )
			{
				outColor.rgb += tex3D( 3, ColorCorrectionVolumeTexture2, offsetBaseColor.rgb ).rgb *
				                ColorCorrectionVolumeWeight2;
				if ( NUM_LOOKUPS > 3 )
				{
					outColor.rgb +=
					    tex3D( 4, ColorCorrectionVolumeTexture3, offsetBaseColor.rgb ).rgb *
					    ColorCorrectionVolumeWeight3;
				}
			}
		}
	}

	LegacyWrite( FinalOutput( outColor, 0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE ) );
}
