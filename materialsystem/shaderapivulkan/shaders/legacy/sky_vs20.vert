#version 450
// A port of stdshaders/sky_vs20.fxc (Sky_HDR_DX9's and Sky_DX9's vertex
// stage; no combos): the transformed base coordinate and its four texel
// neighbours (g_vTextureSizeInfo's half-texel steps) for
// sky_hdr_compressed_rgbs' manual bilinear filter, and the first one in
// texels.
#include "legacy_vs.glsl"

layout( location = 0 ) out vec2 baseTexCoord00;
layout( location = 1 ) out vec2 baseTexCoord01;
layout( location = 2 ) out vec2 baseTexCoord10;
layout( location = 3 ) out vec2 baseTexCoord11;
layout( location = 4 ) out vec2 baseTexCoord_In_Pixels;

#define g_vTextureSizeInfo VS_C( 48 )
#define g_mBaseTexCoordTransform_0 VS_C( 49 )
#define g_mBaseTexCoordTransform_1 VS_C( 50 )
#define TEXEL_XINCR ( g_vTextureSizeInfo.x )
#define TEXEL_YINCR ( g_vTextureSizeInfo.y )
#define U_TO_PIXEL_COORD_SCALE ( g_vTextureSizeInfo.z )
#define V_TO_PIXEL_COORD_SCALE ( g_vTextureSizeInfo.w )

void main()
{
	gl_Position = LegacyProject( inWorldPos );

	const vec2 vTexCoord =
	    DotTexTransform( inTexCoord0, g_mBaseTexCoordTransform_0, g_mBaseTexCoordTransform_1 );

	// Compute quantities needed for pixel shader texture lerping
	baseTexCoord00 = vec2( vTexCoord.x - TEXEL_XINCR, vTexCoord.y - TEXEL_YINCR );
	baseTexCoord10 = vec2( vTexCoord.x + TEXEL_XINCR, vTexCoord.y - TEXEL_YINCR );
	baseTexCoord01 = vec2( vTexCoord.x - TEXEL_XINCR, vTexCoord.y + TEXEL_YINCR );
	baseTexCoord11 = vec2( vTexCoord.x + TEXEL_XINCR, vTexCoord.y + TEXEL_YINCR );

	baseTexCoord_In_Pixels = baseTexCoord00 * vec2( U_TO_PIXEL_COORD_SCALE, V_TO_PIXEL_COORD_SCALE );
}
