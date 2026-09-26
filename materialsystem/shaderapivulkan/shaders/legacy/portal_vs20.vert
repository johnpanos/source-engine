#version 450
// A port of stdshaders/portal_vs20.fxc (Portal_DX90's vertex stage). Combos
// (fxctmp9/portal_vs20.inc): static HASALPHAMASK (4), HASSTATICTEXTURE (8),
// USEALTERNATEVIEW (16); dynamic SKINNING (1, applied by the shader API's
// vertex record), ADDSTATIC (2). The portal texture coordinate is the screen
// position (through g_CustomViewProj with USEALTERNATEVIEW, stretched instead
// of clipped), divided by its w in the pixel stage.
#include "legacy_vs.glsl"

layout( location = 0 ) out vec3 vPortalTexCoord;
layout( location = 1 ) out vec2 vSecondaryTexCoord;
layout( location = 2 ) out vec2 vTertiaryTexCoord;
layout( location = 7 ) out vec4 worldPos_projPosZ;

#define g_CustomViewProj_0 VS_C( 48 )
#define g_CustomViewProj_1 VS_C( 49 )
#define g_CustomViewProj_2 VS_C( 50 )
#define g_CustomViewProj_3 VS_C( 51 )

void main()
{
	const bool HASALPHAMASK = STATIC_VS_COMBO( 4, 2 ) != 0;
	const bool HASSTATICTEXTURE = STATIC_VS_COMBO( 8, 2 ) != 0;
	const bool USEALTERNATEVIEW = STATIC_VS_COMBO( 16, 2 ) != 0;
	const bool ADDSTATIC = DYNAMIC_VS_COMBO( 2, 2 ) != 0;
	const bool USESTATICTEXTURE = ADDSTATIC && HASSTATICTEXTURE;

	const vec3 worldPos = inWorldPos;
	const vec4 vProjPos = LegacyProject( worldPos );
	gl_Position = vProjPos;

	vec4 vTextureProjectedPos = vProjPos;
	if ( USEALTERNATEVIEW )
	{
		const vec4 p = vec4( worldPos, 1.0 );
		vTextureProjectedPos = vec4( dot( p, g_CustomViewProj_0 ), dot( p, g_CustomViewProj_1 ),
		    dot( p, g_CustomViewProj_2 ), dot( p, g_CustomViewProj_3 ) );
	}

	worldPos_projPosZ = vec4( worldPos, vProjPos.z );

	// Screen coordinates mapped back to texture coordinates for the portal texture
	vPortalTexCoord.x = vTextureProjectedPos.x;
	vPortalTexCoord.y = -vTextureProjectedPos.y; // invert Y
	vPortalTexCoord.xy = ( vPortalTexCoord.xy + vTextureProjectedPos.w ) * 0.5;
	vPortalTexCoord.z = vTextureProjectedPos.w;
	if ( USEALTERNATEVIEW )
	{
		// stretch instead of clipping.
		vPortalTexCoord.xy =
		    saturate( vPortalTexCoord.xy / vTextureProjectedPos.w ) * vTextureProjectedPos.w;
	}

	vSecondaryTexCoord = vec2( 0.0 );
	vTertiaryTexCoord = vec2( 0.0 );
	if ( HASALPHAMASK || USESTATICTEXTURE )
	{
		vSecondaryTexCoord = inTexCoord0;
		if ( HASALPHAMASK && USESTATICTEXTURE )
			vTertiaryTexCoord = inTexCoord0;
	}
}
