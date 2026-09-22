#version 450
// PortalRefract's pixel stage: a line-for-line port of
// stdshaders/portal_refract_ps2x.fxc, STAGE selected by alphaParams.y.
//   stage 0: warp the frame copy (s0, _rt_PowerOfTwoFB) around the opening hole
//   stage 1: the stencil cutout (black, alpha 1 inside the hole)
//   stage 2: the flame rim from noise (s1) and a 1D color ramp (s2)
// HLSL intrinsics keep their HLSL definitions: smoothstep with a reversed range
// is defined there (and not in GLSL), and tex1D reads the ramp's single row.
//
// alphaParams: x alpha-test reference (< 0 disables), y STAGE, z flags (1 sRGB s0,
// 4 sRGB output, 8 GREATER alpha test, 64 sRGB s2), w FinalOutput's linear light
// scale (TONEMAP_SCALE_LINEAR, stage 2 only). Fog is not applied here.
layout( location = 0 ) in vec2 vUv0;
layout( location = 1 ) in vec3 vWorldTangent;
layout( location = 2 ) in vec3 vWorldBinormal;
layout( location = 3 ) in vec4 vWorldPosition;
layout( location = 4 ) in vec3 vProjPosForRefract;
layout( location = 5 ) in vec4 vNoiseTexCoord;
layout( location = 0 ) out vec4 outColor;
layout( set = 0, binding = 0 ) uniform sampler2D g_tRefractionSampler;
layout( set = 1, binding = 0 ) uniform sampler2D g_tPortalNoiseSampler;
layout( set = 2, binding = 0 ) uniform sampler2D g_tPortalColorSampler;
layout( push_constant ) uniform Constants
{
	mat4 model;
	mat4 viewProj; // c0-c3: g_mViewProj rows
	vec4 texXform0;
	vec4 texXform1;
	vec4 params; // x time, y g_flPortalOpenAmount, z g_flPortalActive, w g_flPortalColorScale
	vec4 alphaParams;
	vec4 clipPlanes[2];
}
consts;

float HlslSmoothstep( float edge0, float edge1, float x )
{
	float t = clamp( ( x - edge0 ) / ( edge1 - edge0 ), 0.0, 1.0 );
	return t * t * ( 3.0 - 2.0 * t );
}
// This is the equilavent of smoothstep built into HLSL but linear
float linearstep( float iMin, float iMax, float iValue )
{
	return clamp( ( iValue - iMin ) / ( iMax - iMin ), 0.0, 1.0 );
}
vec3 SrgbToLinear( vec3 c )
{
	return mix( c / 12.92, pow( ( c + 0.055 ) / 1.055, vec3( 2.4 ) ), step( 0.04045, c ) );
}
vec3 LinearToSrgb( vec3 c )
{
	c = clamp( c, 0.0, 1.0 );
	return mix( c * 12.92, 1.055 * pow( c, vec3( 1.0 / 2.4 ) ) - 0.055, step( 0.0031308, c ) );
}
// dot( float4( p, 1 ), g_mViewProjN ) for rows 0, 1 and 3.
vec3 ProjectRows( vec3 p )
{
	vec4 clip = consts.viewProj * vec4( p, 1.0 );
	return vec3( clip.x, -clip.y, clip.w );
}

void main()
{
	const int flags = int( consts.alphaParams.z );
	const int stage = int( consts.alphaParams.y );
	const float kFlPortalOuterBorder = 0.075; // Must match VS!
	const float kFlPortalInnerBorder = kFlPortalOuterBorder * 4.0;

	// Portal open time
	float flPortalOpenAmount = HlslSmoothstep( 0.0, 1.0, clamp( consts.params.y, 0.0, 1.0 ) );
	float flPortalOpenAmountSquared = flPortalOpenAmount * flPortalOpenAmount;

	// Stretch values
	vec2 vStretchVector = ( vUv0 * 2.0 ) - 1.0;
	float flDistFromCenter = length( vStretchVector );
	vec2 vStretchVectorNormalized = normalize( vStretchVector );

	// Stencil cutout (1.0 in hole)
	float flStencilCutout = step( flDistFromCenter, flPortalOpenAmountSquared );

	vec4 result;
	bool scaleLinear = false;
	if ( stage == 0 )
	{
		// Unrefracted tex coords
		vec2 vRefractTexCoordNoWarp = vProjPosForRefract.xy / vProjPosForRefract.z;

		// Projected screen-space tangent
		vec3 vProjTangent = ProjectRows( vWorldTangent );
		vProjTangent.xy += vProjTangent.z;
		vProjTangent.xy *= 0.5;
		vProjTangent.xy /= vProjTangent.z;
		vProjTangent.xy -= vRefractTexCoordNoWarp.xy;

		// Projected screen-space binormal
		vec3 vProjBinormal = ProjectRows( vWorldBinormal );
		vProjBinormal.xy += vProjBinormal.z;
		vProjBinormal.xy *= 0.5;
		vProjBinormal.xy /= vProjBinormal.z;
		vProjBinormal.xy -= vRefractTexCoordNoWarp.xy;

		// Tangent-space uv offset
		vec2 vTangentRefract = -vStretchVectorNormalized * flPortalOpenAmountSquared *
		                       ( 1.0 - pow( clamp( flDistFromCenter, 0.0, 1.0 ), 64.0 ) );
		vTangentRefract *=
		    HlslSmoothstep( ( flPortalOpenAmount * 1.5 ), flPortalOpenAmount, flDistFromCenter );

		const vec2 kPortalRadius = vec2( 32.0, 32.0 );
		vTangentRefract *= kPortalRadius;

		// Generate refracteds screen-space uv
		vec2 vRefractTexCoord = vRefractTexCoordNoWarp;
		vRefractTexCoord += vTangentRefract.x * vProjTangent.xy;
		vRefractTexCoord -= vTangentRefract.y * vProjBinormal.xy;

		// Fetch color from texture
		vec3 cRefract = texture( g_tRefractionSampler, vRefractTexCoord ).rgb;
		if ( ( flags & 1 ) != 0 )
			cRefract = SrgbToLinear( cRefract );

		// Darken the ring around the portal as it's opening
		float flHoleEdge = flPortalOpenAmountSquared;
		float flDimEdge = clamp( flPortalOpenAmount * 2.0, 0.0, 1.0 );
		float flDarkeningRing = linearstep( flHoleEdge - 0.01, flDimEdge, flDistFromCenter );
		flDarkeningRing = ( abs( flDarkeningRing * 2.0 - 1.0 ) * 0.15 ) + 0.85;

		result.rgb = cRefract * flDarkeningRing;
		// Alpha test away outside the portal oval
		result.a = step( flDistFromCenter, 1.0 );
	}
	else if ( stage == 1 )
	{
		result = vec4( 0.0, 0.0, 0.0, flStencilCutout );
	}
	else
	{
		// Outer effect mask
		float flOuterEffectMask =
		    ( 1.0 - linearstep( flPortalOpenAmountSquared,
		                flPortalOpenAmountSquared + kFlPortalOuterBorder, flDistFromCenter ) ) *
		    ( 1.0 - flStencilCutout );

		// Inner effect mask
		float flInnerEffectMask = ( linearstep( flPortalOpenAmountSquared - kFlPortalInnerBorder,
		                              flPortalOpenAmountSquared, flDistFromCenter ) ) *
		                          ( flStencilCutout );

		// Fade it in as the portal is opening
		float flPortalActive = clamp( consts.params.z, 0.0, 1.0 );
		float flEffectFadeIn =
		    max( clamp( flPortalOpenAmount * 2.5, 0.0, 1.0 ), ( 1.0 - flPortalActive ) );

		// Combine mask terms
		float flEffectMask = ( flInnerEffectMask + flOuterEffectMask ) * flEffectFadeIn;

		vec4 cNoiseTexel1 = texture( g_tPortalNoiseSampler, vNoiseTexCoord.xy );
		vec4 cNoiseTexel2 =
		    texture( g_tPortalNoiseSampler, vNoiseTexCoord.wz - cNoiseTexel1.rg * 0.02 );
		cNoiseTexel1 = texture( g_tPortalNoiseSampler, vNoiseTexCoord.xy - cNoiseTexel2.rg * 0.02 );

		float flNoise = ( ( cNoiseTexel1.g + cNoiseTexel2.g ) * 0.5 );
		float flPortalActiveWithNoise = HlslSmoothstep( 0.0, flNoise, flPortalActive );

		const float kFlBorderSoftness = 0.875;
		float flBorderMaskWithNoise = ( 1.0 - HlslSmoothstep( flEffectMask - kFlBorderSoftness,
		                                          flEffectMask + kFlBorderSoftness, flNoise ) );
		flNoise = flBorderMaskWithNoise;
		flEffectMask *= flBorderMaskWithNoise;

		// This will get stuffed in alpha
		float flTransparancy =
		    clamp( flEffectMask + ( flStencilCutout * ( 1.0 - flPortalActiveWithNoise ) ), 0.0,
		        1.0 ) *
		    1.5;

		// Portals shift in color from bottom to top
		float flBottomToTopBrightnessShift = ( pow( abs( vUv0.y ), 1.5 ) * 0.8 ) + 0.2;

		// Fetch color from 1D texture
		float rampCoord = pow( flNoise, 0.5 ) * flBottomToTopBrightnessShift * flTransparancy;
		vec4 cFlameColor = texture( g_tPortalColorSampler, vec2( rampCoord, 0.5 ) );
		if ( ( flags & 64 ) != 0 )
			cFlameColor.rgb = SrgbToLinear( cFlameColor.rgb );
		cFlameColor.rgb *= consts.params.w;

		result.rgb = cFlameColor.rgb;
		result.a = flTransparancy;
		scaleLinear = true;
	}

	// The D3D9 fixed-function alpha test (GREATER for every stage).
	if ( consts.alphaParams.x >= 0.0 &&
	     ( ( flags & 8 ) != 0 ? result.a <= consts.alphaParams.x
	                          : result.a < consts.alphaParams.x ) )
		discard;
	if ( scaleLinear )
		result.rgb *= consts.alphaParams.w;
	if ( ( flags & 4 ) != 0 )
		result.rgb = LinearToSrgb( result.rgb );
	outColor = result;
}
