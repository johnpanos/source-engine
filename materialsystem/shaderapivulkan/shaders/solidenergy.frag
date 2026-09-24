#version 450
// SolidEnergy's pixel stage: a port of stdshaders/solidenergy_ps20b.fxc, with
// the vertex-stage terms that depend on the tangent frame (flow UVs, vortex
// offsets, the view-aligned tangent) evaluated here per pixel.
//
// Constants (ps.c, the skin layout's uniform buffer), as solidenergy_dx9_helper
// writes them: c0-c2 the opacity ranges, c3 ( depth-to-alpha, time, $powerup,
// $flow_color_intensity ), c6-c9 the flow parameters, c10 the dynamic combos
// ( ACTIVE, POWERUP, VORTEX1, VORTEX2 ), c11 ( static flags, DETAIL1BLENDMODE,
// DETAIL2BLENDMODE, FLOW_CHEAP ). The shader API adds the vertex registers:
// c12/c13 and c14/c15 the detail 1 and 2 texture transforms, c16 ( vortex 1,
// noise scale ), c17 ( vortex 2, normal UV scale ).
// params: x alpha-test reference (unused), y static flags, z kColor* flags
// (1 sRGB base, 4 sRGB output), w saturate( LINEAR_LIGHT_SCALE ).
layout( location = 0 ) in vec2 vBaseUv;
layout( location = 1 ) in vec2 vTexCoord;
layout( location = 2 ) in vec4 vProjPos;
layout( location = 3 ) in vec3 vWorldPos;
layout( location = 4 ) in vec3 vWorldNormal;
layout( location = 5 ) in vec3 vWorldTangentS;
layout( location = 6 ) in vec3 vWorldTangentT;
layout( location = 7 ) in vec4 vColor;
layout( location = 0 ) out vec4 outColor;
layout( set = 0, binding = 0 ) uniform sampler2D g_tBase;       // s0
layout( set = 1, binding = 0 ) uniform sampler2D g_tDetail1;    // s1
layout( set = 2, binding = 0 ) uniform sampler2D g_tDetail2;    // s4
layout( set = 3, binding = 0 ) uniform sampler2D g_tFlowMap;    // s5
layout( set = 4, binding = 0 ) uniform sampler2D g_tFlowNoise;  // s6
layout( set = 5, binding = 0 ) uniform sampler2D g_tFlowBounds; // s7
layout( set = 6, binding = 0 ) uniform PixelShaderConstants
{
	vec4 c[32];
}
ps;
layout( push_constant ) uniform Constants
{
	mat4 viewProj;
	vec4 texXform0;
	vec4 texXform1;
	vec4 eyePos;
	vec4 params;
	vec4 params2;
	vec4 clipPlanes[2];
}
consts;

// Static flags (solidenergy_dx9_helper.cpp's SolidEnergyStaticFlags_t).
const int ADDITIVE = 1;
const int DETAIL1 = 2;
const int DETAIL2 = 4;
const int TANGENTTOPACITY = 8;
const int TANGENTSOPACITY = 16;
const int FRESNELOPACITY = 32;
const int VERTEXCOLOR = 64;
const int FLOWMAP = 128;
const int MODELFORMAT = 256;

#define g_vTangentTOpacityRanges ps.c[0]
#define g_vTangentSOpacityRanges ps.c[1]
#define g_vFresnelOpacityRanges ps.c[2]
#define g_flTime ( ps.c[3].y )
#define g_flPowerUp ( ps.c[3].z )
#define g_flIntensity ( ps.c[3].w )
#define g_flWorldUvScale ( ps.c[6].x )
#define g_flOutputIntensity ( ps.c[6].w )
#define g_flFlowTimeIntervalInSeconds ( ps.c[7].x )
#define g_flFlowUvScrollDistance ( ps.c[7].y )
#define g_flColorFlowLerpExp ( ps.c[7].w )
#define g_cFlow ( ps.c[8].xyz )
#define g_cVortex ( ps.c[9].xyz )
#define g_flVortexSize ( ps.c[9].w )

vec3 SrgbToLinear( vec3 c )
{
	return mix( c / 12.92, pow( ( c + 0.055 ) / 1.055, vec3( 2.4 ) ), step( 0.04045, c ) );
}
vec3 LinearToSrgb( vec3 c )
{
	c = clamp( c, 0.0, 1.0 );
	return mix( c * 12.92, 1.055 * pow( c, vec3( 1.0 / 2.4 ) ) - 0.055, step( 0.0031308, c ) );
}
// HLSL smoothstep, which (unlike GLSL's) is defined for a reversed range.
float HlslSmoothstep( float edge0, float edge1, float x )
{
	float t = clamp( ( x - edge0 ) / ( edge1 - edge0 ), 0.0, 1.0 );
	return t * t * ( 3.0 - 2.0 * t );
}
// A 2-row texture transform as the vertex shader applies it.
vec2 Transform( vec4 row0, vec4 row1, vec2 uv )
{
	return vec2( dot( uv, row0.xy ) + row0.w, dot( uv, row1.xy ) + row1.w );
}
// ComputeCameraFade (PC): fade out geometry right at the near plane.
float ComputeCameraFade( vec4 projPos )
{
	return HlslSmoothstep( 0.0, 1.0, clamp( projPos.z * 0.025, 0.0, 1.0 ) );
}
// The vortex's position in the tangent frame, as solidenergy_vs20 computes it.
vec3 VortexPosition( vec3 vortex )
{
	vec3 v = vortex - vWorldPos;
	return -vec3( dot( v, vWorldTangentS ), dot( v, vWorldTangentT ), dot( v, vWorldNormal ) );
}

void main()
{
	const int statics = int( ps.c[11].x );
	const int colorFlags = int( consts.params.z );
	const bool bActive = ps.c[10].x != 0.0;
	const bool bPowerUp = ps.c[10].y != 0.0;
	const bool bVortex1 = ps.c[10].z != 0.0;
	const bool bVortex2 = ps.c[10].w != 0.0;
	const bool bFlowMap = ( statics & FLOWMAP ) != 0;
	const bool bFlowCheap = ps.c[11].w != 0.0;
	const bool bDetail1 = ( statics & DETAIL1 ) != 0;
	const bool bDetail2 = ( statics & DETAIL2 ) != 0;
	const int nDetail1BlendMode = int( ps.c[11].y );
	const int nDetail2BlendMode = int( ps.c[11].z );
	const bool bTangentT = ( statics & TANGENTTOPACITY ) != 0;
	const bool bTangentS = ( statics & TANGENTSOPACITY ) != 0;
	const bool bFresnel = ( statics & FRESNELOPACITY ) != 0;

	// solidenergy_vs20's texture coordinates.
	vec4 vUV0_UV1 = vec4( vBaseUv, 0.0, 0.0 );
	vec4 vFlowUV_UV2 = vec4( 0.0 );
	if ( bDetail1 )
		vUV0_UV1.zw = Transform( ps.c[12], ps.c[13], vTexCoord );
	if ( bDetail2 )
		vFlowUV_UV2.zw = Transform( ps.c[14], ps.c[15], vTexCoord );
	if ( bFlowMap )
	{
		vec2 vFlowUV = vTexCoord;
		if ( ( statics & MODELFORMAT ) == 0 )
			vFlowUV = vec2( dot( vWorldPos, vWorldTangentS ), dot( vWorldPos, vWorldTangentT ) );
		vFlowUV_UV2.xy = vFlowUV;
		vFlowUV_UV2.zw = vFlowUV * ps.c[16].w;
		vUV0_UV1.zw = vFlowUV * ps.c[17].w;
	}

	vec4 cOut = vec4( 0.0, 0.0, 0.0, 1.0 );
	vec4 cBase = vec4( 0.0, 0.0, 0.0, 1.0 );
	float alpha = 1.0;

	if ( bActive )
	{
		if ( bFlowMap )
		{
			vec4 vBoundsTexel = texture( g_tFlowBounds, vUV0_UV1.xy );
			vec2 vFlowVectorTs = vec2( 0.0 );
			if ( !bFlowCheap )
			{
				vec2 vFlowUV = vFlowUV_UV2.xy * g_flWorldUvScale;
				vec4 vFlowTexel = texture( g_tFlowMap, vFlowUV );
				vFlowVectorTs = ( vFlowTexel.rg * 2.0 ) - 1.0;
				vFlowVectorTs *= vBoundsTexel.r; // slow flow
			}
			float flVortexIntensity = 0.0;
			if ( bVortex1 )
			{
				vec3 vVortex = VortexPosition( ps.c[16].xyz );
				float flVortex1Intensity =
				    clamp( g_flVortexSize / length( vVortex ) - 0.5, 0.0, 1.0 );
				if ( !bFlowCheap )
					vFlowVectorTs = mix( vFlowVectorTs, normalize( vVortex.xy ), flVortex1Intensity * 0.5 );
				flVortexIntensity += flVortex1Intensity;
			}
			if ( bVortex2 )
			{
				vec3 vVortex = VortexPosition( ps.c[17].xyz );
				float flVortex2Intensity =
				    clamp( g_flVortexSize / length( vVortex ) - 0.5, 0.0, 1.0 );
				if ( !bFlowCheap )
					vFlowVectorTs = mix( vFlowVectorTs, normalize( vVortex.xy ), flVortex2Intensity * 0.5 );
				flVortexIntensity += flVortex2Intensity;
			}

			float flNoise = texture( g_tFlowNoise, vFlowUV_UV2.zw ).g;

			// Every interval has a unique offset so the same bump texels do not
			// repeat continuously.
			float flTimeInIntervals = ( g_flTime / ( g_flFlowTimeIntervalInSeconds * 2.0 ) ) + flNoise;
			float flScrollTime1 = fract( flTimeInIntervals ) - 0.5;
			float flScrollTime2 = fract( flTimeInIntervals + 0.5 ) - 0.5; // half an interval off

			float flOffset1 = 0.0;
			float flOffset2 = 0.5;
			if ( !bFlowCheap )
			{
				flOffset1 = floor( flTimeInIntervals ) * 0.311;
				flOffset2 = floor( flTimeInIntervals + 0.5 ) * 0.311 + 0.5;
			}

			float flWeight1 = abs( ( 2.0 * fract( flTimeInIntervals + 0.5 ) ) - 1.0 );
			float flWeight2 = abs( ( 2.0 * fract( flTimeInIntervals ) ) - 1.0 );
			if ( !bFlowCheap )
			{
				flWeight1 = pow( flWeight1, g_flColorFlowLerpExp );
				flWeight2 = pow( flWeight2, g_flColorFlowLerpExp );
			}
			else
			{
				flWeight1 *= flWeight1;
				flWeight2 *= flWeight2;
			}

			float flFlowUvScrollDistance = g_flFlowUvScrollDistance;
			vec2 vFlowUV0 = vUV0_UV1.zw + flOffset1;
			vec2 vFlowUV1 = vUV0_UV1.zw + flOffset2;
			if ( !bFlowCheap )
			{
				flFlowUvScrollDistance *= ( 1.0 + flVortexIntensity );
				vFlowUV0 += ( flScrollTime1 * ( flFlowUvScrollDistance * vFlowVectorTs.xy ) );
				vFlowUV1 += ( flScrollTime2 * ( flFlowUvScrollDistance * vFlowVectorTs.xy ) );
			}

			// $basetexture is sRGB (decoded by its view or here).
			vec4 cBase0 = texture( g_tBase, vFlowUV0 );
			vec4 cBase1 = texture( g_tBase, vFlowUV1 );
			if ( ( colorFlags & 1 ) != 0 )
			{
				cBase0.rgb = SrgbToLinear( cBase0.rgb );
				cBase1.rgb = SrgbToLinear( cBase1.rgb );
			}
			cBase = cBase0 * flWeight1 + cBase1 * flWeight2;

			if ( bPowerUp )
			{
				float flNoiseReveal = ( flNoise + ( 1.0 - vBoundsTexel.g ) ) * 0.5;
				float flPowerStage2 = clamp( g_flPowerUp * 3.0, 0.0, 1.0 );
				float flPowerUpRange1 = HlslSmoothstep( 0.02, 0.0, abs( flNoiseReveal - g_flPowerUp ) );
				float flPowerUpRange2 = HlslSmoothstep( 0.02, 0.0, ( flNoiseReveal - g_flPowerUp ) );

				cBase.ag += flPowerUpRange1 * g_flPowerUp * ( 1.0 - g_flPowerUp );
				cBase.ag *= flPowerStage2 * flPowerUpRange2;
				cBase.ag += vBoundsTexel.g * flPowerStage2;
			}
			else
			{
				cBase.ag += vBoundsTexel.g;
			}

			vec3 cFlowField = cBase.a * g_cFlow;
			if ( bVortex1 || bVortex2 )
			{
				vec3 cVortex = cBase.g * g_cVortex;
				cBase.rgb = mix( cFlowField, cVortex, flVortexIntensity );
			}
			else
			{
				cBase.rgb = cFlowField;
			}
			cBase.rgb *= vBoundsTexel.b * g_flIntensity;
		}
		else
		{
			cBase = texture( g_tBase, vUV0_UV1.xy );
			if ( ( colorFlags & 1 ) != 0 )
				cBase.rgb = SrgbToLinear( cBase.rgb );
		}

		// The view-dependent opacity terms.
		vec3 vWorldEyeDir = normalize( consts.eyePos.xyz - vWorldPos );
		vec3 vNormal = normalize( vWorldNormal );
		float fBackfaceRatio = 0.0;
		if ( bTangentT || bTangentS || bFresnel )
		{
			fBackfaceRatio = dot( vWorldEyeDir, vNormal ) * 0.5 + 0.5;
			fBackfaceRatio *= fBackfaceRatio;
		}

		float fBackfaceAlpha = 1.0;
		if ( bTangentT )
		{
			vec3 vTangent = normalize( vWorldTangentT );
			vec3 vRightT = cross( vWorldTangentS, vWorldEyeDir );
			vec3 vAlignedViewT = normalize( cross( vWorldTangentS, vRightT ) );
			float fTTFacing = abs( dot( vTangent, vAlignedViewT ) );
			alpha *= mix( g_vTangentTOpacityRanges.x, g_vTangentTOpacityRanges.y,
			    pow( fTTFacing, g_vTangentTOpacityRanges.z ) );
			fBackfaceAlpha = mix( 1.0, g_vTangentTOpacityRanges.w, fBackfaceRatio );
		}
		if ( bTangentS )
		{
			vec3 vTangent = normalize( vWorldTangentS );
			vec3 vRightS = cross( vWorldTangentT, vWorldEyeDir );
			vec3 vAlignedViewS = normalize( cross( vWorldTangentT, vRightS ) );
			float fTSFacing = abs( dot( vTangent, vAlignedViewS ) );
			alpha *= mix( g_vTangentSOpacityRanges.x, g_vTangentSOpacityRanges.y,
			    pow( fTSFacing, g_vTangentTOpacityRanges.z ) );
			fBackfaceAlpha = min( fBackfaceAlpha, mix( g_vTangentSOpacityRanges.w, 1.0, fBackfaceRatio ) );
		}
		if ( bFresnel )
		{
			float tNFacing = abs( dot( vNormal, vWorldEyeDir ) );
			alpha *= mix( g_vFresnelOpacityRanges.x, g_vFresnelOpacityRanges.y,
			    pow( tNFacing, g_vFresnelOpacityRanges.z ) );
			fBackfaceAlpha = min( fBackfaceAlpha, mix( g_vFresnelOpacityRanges.w, 1.0,
			                                          dot( vWorldEyeDir, vNormal ) * 0.5 + 0.5 ) );
		}
		alpha *= fBackfaceAlpha;

		if ( !bFlowMap && !bTangentT && !bTangentS && !bFresnel && !( bDetail1 && nDetail1BlendMode == 1 ) )
			alpha *= cBase.a;

		float flCameraFade = ComputeCameraFade( vProjPos );

		vec4 cDetail1 = vec4( 0.0 );
		vec4 cDetail2 = vec4( 0.0 );
		if ( bDetail2 )
		{
			cDetail2 = texture( g_tDetail2, vFlowUV_UV2.zw );
			cDetail2.rgb = SrgbToLinear( cDetail2.rgb );
		}
		if ( bDetail1 )
		{
			cDetail1 = texture( g_tDetail1, vUV0_UV1.zw );
			cDetail1.rgb = SrgbToLinear( cDetail1.rgb );
			if ( nDetail1BlendMode == 0 )
				cBase.rgb *= 2.0 * cDetail1.rgb;
			else
				cBase.rgb = mix( cBase.rgb * cDetail1.rgb, cBase.rgb, cBase.a );
		}
		if ( bDetail2 )
		{
			if ( nDetail2BlendMode == 0 )
			{
				if ( bDetail1 )
					cDetail2.rgb *= cDetail1.rgb;
				cBase.rgb += cDetail2.rgb;
			}
			else
			{
				cBase.rgb *= cDetail2.rgb;
			}
		}

		cOut.a = alpha;
		cOut.rgb = cBase.rgb;
		if ( bPowerUp && !bFlowMap )
			cOut.rgb *= g_flPowerUp;

		if ( ( statics & ADDITIVE ) != 0 )
		{
			cOut.rgb *= ( 1.0 + alpha ) * flCameraFade;
			cOut.a = 1.0;
		}

		if ( ( statics & VERTEXCOLOR ) != 0 )
		{
			// fun with saturation:
			vec3 vVertexColor = pow( vColor.rgb, vec3( ( 2.0 - alpha ) * 2.0 ) );
			cOut.rgb *= vVertexColor;
		}
	}

	// FinalOutput( cOut, 0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE )
	//     * saturate( LINEAR_LIGHT_SCALE ) * g_flOutputIntensity,
	// clamped as the fixed-point render target stores it before blending.
	vec4 result = clamp( cOut * consts.params.w * g_flOutputIntensity, 0.0, 1.0 );
	if ( ( colorFlags & 4 ) != 0 )
		result.rgb = LinearToSrgb( result.rgb );
	outColor = result;
}
