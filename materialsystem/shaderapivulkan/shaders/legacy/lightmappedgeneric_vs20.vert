#version 450
// A port of stdshaders/lightmappedgeneric_vs20.fxc, the vertex stage of the
// lightmapped world shaders (LightmappedGeneric, WorldVertexTransition,
// WorldTwoTextureBlend, DecalBaseTimesLightmapAlphaBlendSelfIllum's second
// pass). Combos (fxctmp9/lightmappedgeneric_vs20.inc): static ENVMAP_MASK (8),
// TANGENTSPACE (16), BUMPMAP (32), DIFFUSEBUMPMAP (64), VERTEXCOLOR (128),
// VERTEXALPHATEXBLENDFACTOR (256), RELIEF_MAPPING (512, always 0), SEAMLESS
// (512), BUMPMASK (1024); dynamic FASTPATH (1), DOWATERFOG (2),
// LIGHTING_PREVIEW (4). vs_2_0's water fog is done in the pixel shader, so
// DOWATERFOG's fog factor is 1. The tangent frame TANGENTSPACE and
// LIGHTING_PREVIEW output is the brush tangents S and T when the pass's
// format has them (LightmappedGeneric with an env map); a format without
// them has no TANGENT or BINORMAL stream D3D9 could bind, and the frame
// holds whatever the record's slots carry (legacy_vs.glsl).
#include "legacy_vs.glsl"

layout( location = 0 ) out vec4 baseTexCoord;       // xy, or SEAMLESS's xyz
layout( location = 1 ) out vec4 detailOrBumpAndEnvmapMaskTexCoord;
layout( location = 2 ) out vec4 lightmapTexCoord1And2;
layout( location = 3 ) out vec4 lightmapTexCoord3;
layout( location = 4 ) out vec4 worldPos_projPosZ;
layout( location = 5 ) out vec3 tangentSpaceTranspose0;
layout( location = 6 ) out vec3 tangentSpaceTranspose1;
layout( location = 7 ) out vec3 tangentSpaceTranspose2;
layout( location = 8 ) out vec4 vertexColor;
layout( location = 9 ) out vec4 vertexBlendX_fogFactorW;

#define SeamlessScale VS_C( 48 )
#define cBaseTexCoordTransform_0 VS_C( 48 )
#define cBaseTexCoordTransform_1 VS_C( 49 )
#define cDetailOrBumpTexCoordTransform_0 VS_C( 50 )
#define cDetailOrBumpTexCoordTransform_1 VS_C( 51 )
#define cEnvmapMaskTexCoordTransform_0 VS_C( 52 )
#define cEnvmapMaskTexCoordTransform_1 VS_C( 53 )
#define cBlendMaskTexCoordTransform_0 VS_C( 14 )
#define cBlendMaskTexCoordTransform_1 VS_C( 15 )

// dot( float2 uv, float4 row ) + row.w: fxc truncates the dot to two terms.
float Dot2PlusW( vec2 uv, vec4 row )
{
	return dot( uv, row.xy ) + row.w;
}

void main()
{
	const bool ENVMAP_MASK = STATIC_VS_COMBO( 8, 2 ) != 0;
	const bool TANGENTSPACE = STATIC_VS_COMBO( 16, 2 ) != 0;
	const bool BUMPMAP = STATIC_VS_COMBO( 32, 2 ) != 0;
	const bool DIFFUSEBUMPMAP = STATIC_VS_COMBO( 64, 2 ) != 0;
	const bool VERTEXCOLOR = STATIC_VS_COMBO( 128, 2 ) != 0;
	const bool VERTEXALPHATEXBLENDFACTOR = STATIC_VS_COMBO( 256, 2 ) != 0;
	const bool SEAMLESS = STATIC_VS_COMBO( 512, 2 ) != 0;
	const bool BUMPMASK = STATIC_VS_COMBO( 1024, 2 ) != 0;
	const bool FASTPATH = DYNAMIC_VS_COMBO( 1, 2 ) != 0;
	const bool DOWATERFOG = DYNAMIC_VS_COMBO( 2, 2 ) != 0;
	const bool LIGHTING_PREVIEW = DYNAMIC_VS_COMBO( 4, 2 ) != 0;

	const vec3 worldPos = inWorldPos;
	vec4 vProjPos = LegacyProject( worldPos );
	gl_Position = vProjPos;
	vProjPos.z = dot( vec4( worldPos, 1.0 ), VS_C( 13 ) ); // cViewProjZ
	worldPos_projPosZ = vec4( worldPos, vProjPos.z );

	const vec3 worldNormal = inWorldNormal;
	tangentSpaceTranspose0 = vec3( 0.0 );
	tangentSpaceTranspose1 = vec3( 0.0 );
	tangentSpaceTranspose2 = vec3( 0.0 );
	if ( TANGENTSPACE || LIGHTING_PREVIEW )
	{
		tangentSpaceTranspose0 = inWorldTangentS.xyz;
		tangentSpaceTranspose1 = LegacyTangentT();
		tangentSpaceTranspose2 = worldNormal;
	}

	const vec2 vBaseTexCoord = inTexCoord0;
	baseTexCoord = vec4( 0.0 );
	detailOrBumpAndEnvmapMaskTexCoord = vec4( 0.0 );
	lightmapTexCoord1And2 = vec4( 0.0 );
	lightmapTexCoord3 = vec4( 0.0 );
	if ( SEAMLESS )
	{
		baseTexCoord.xyz = SeamlessScale.x * worldPos;
	}
	else
	{
		if ( FASTPATH )
		{
			baseTexCoord.xy = vBaseTexCoord;
			detailOrBumpAndEnvmapMaskTexCoord.xy = vBaseTexCoord;
		}
		else
		{
			baseTexCoord.x = Dot2PlusW( vBaseTexCoord, cBaseTexCoordTransform_0 );
			baseTexCoord.y = Dot2PlusW( vBaseTexCoord, cBaseTexCoordTransform_1 );
			detailOrBumpAndEnvmapMaskTexCoord.x =
			    Dot2PlusW( vBaseTexCoord, cDetailOrBumpTexCoordTransform_0 );
			detailOrBumpAndEnvmapMaskTexCoord.y =
			    Dot2PlusW( vBaseTexCoord, cDetailOrBumpTexCoordTransform_1 );
		}
	}
	if ( FASTPATH )
	{
		lightmapTexCoord3.zw = vBaseTexCoord;
	}
	else
	{
		lightmapTexCoord3.z = Dot2PlusW( vBaseTexCoord, cBlendMaskTexCoordTransform_0 );
		lightmapTexCoord3.w = Dot2PlusW( vBaseTexCoord, cBlendMaskTexCoordTransform_1 );
	}

	// The lightmap coordinates; bumped lightmaps step by the TEXCOORD2 offset.
	const vec2 vLightmapTexCoord = inTexCoord1;
	const vec2 vLightmapTexCoordOffset = LegacyTexCoord2();
	if ( BUMPMAP && DIFFUSEBUMPMAP )
	{
		lightmapTexCoord1And2.xy = vLightmapTexCoord + vLightmapTexCoordOffset;
		const vec2 lightmapTexCoord2 = lightmapTexCoord1And2.xy + vLightmapTexCoordOffset;
		const vec2 lightmapTexCoord3xy = lightmapTexCoord2 + vLightmapTexCoordOffset;
		// reversed component order
		lightmapTexCoord1And2.w = lightmapTexCoord2.x;
		lightmapTexCoord1And2.z = lightmapTexCoord2.y;
		lightmapTexCoord3.xy = lightmapTexCoord3xy;
	}
	else
	{
		lightmapTexCoord1And2.xy = vLightmapTexCoord;
	}

	if ( ENVMAP_MASK || BUMPMASK )
	{
		// reversed component order
		if ( FASTPATH )
		{
			detailOrBumpAndEnvmapMaskTexCoord.wz = vBaseTexCoord.xy;
		}
		else
		{
			detailOrBumpAndEnvmapMaskTexCoord.w =
			    Dot2PlusW( vBaseTexCoord, cEnvmapMaskTexCoordTransform_0 );
			detailOrBumpAndEnvmapMaskTexCoord.z =
			    Dot2PlusW( vBaseTexCoord, cEnvmapMaskTexCoordTransform_1 );
		}
	}

	vertexBlendX_fogFactorW = vec4( DOWATERFOG ? 1.0 : RangeFog( vProjPos.xyz ) );

	const vec4 vColor = vec4( inColor, inAlpha );
	if ( !VERTEXCOLOR )
	{
		vertexColor = vec4( 1.0, 1.0, 1.0, cModulationColor.a );
	}
	else if ( FASTPATH )
	{
		vertexColor = vColor;
	}
	else if ( VERTEXALPHATEXBLENDFACTOR )
	{
		vertexColor = vec4( vColor.rgb, cModulationColor.a );
	}
	else
	{
		vertexColor = vec4( vColor.rgb, vColor.a * cModulationColor.a );
	}
	if ( SEAMLESS )
	{
		// blend weights in rgb
		const vec3 vNormal = normalize( worldNormal );
		vertexColor.xyz = vNormal * vNormal;
	}

	if ( VERTEXALPHATEXBLENDFACTOR )
		vertexBlendX_fogFactorW.r = vColor.a;
}
