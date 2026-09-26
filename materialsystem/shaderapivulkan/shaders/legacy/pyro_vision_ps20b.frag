#version 450
// pyro_vision's pixel stage: a port of stdshaders/pyro_vision_ps2x.fxc (ps20b):
// EFFECT 0 the posterized canvas look and EFFECT 1 the grey-stepped look of
// world and model surfaces (range fog, linear tone map scale), EFFECT 2 the
// depth of field blend and EFFECT 3 the vignette (with heat haze) of the
// screen. Combos (fxctmp9/pyro_vision_ps20b.inc): static EFFECT (8, 0..3),
// VERTEX_LIT (32), BASETEXTURE2 (64), FANCY_BLENDING (128), SELFILLUM (256),
// COLOR_BAR (512), STRIPES (1024), STRIPES_USE_NORMAL2 (2048); dynamic
// PIXELFOGTYPE (1; the shader fogs by range whatever it is), VISUALIZE_DOF (2),
// HEATHAZE (4).
// @legacy program=pyro_vision ps=pyro_vision_ps20b vs=pyro_vision_vs20 vert=pyro_vision_vs20
//         samplers=0:2d,1:2d,2:2d,3:2d,4:2d,5:2d,6:2d
#include "legacy_ps.glsl"
#include "legacy_color.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler; // s0
// s1: EFFECT 0, 1 LightmapSampler
layout( set = 0, binding = 1 ) uniform sampler2D LightmapSampler;
// s2: EFFECT 0 CanvasSampler, EFFECT 1 ColorBarSampler
layout( set = 0, binding = 2 ) uniform sampler2D Sampler2;
// s3: BlendModulationSampler (FANCY_BLENDING), EFFECT 3 NoiseSampler
layout( set = 0, binding = 3 ) uniform sampler2D Sampler3;
// s4: BaseTexture2Sampler, EFFECT 2 BlurredFrameSampler, EFFECT 3 WarpFrameSampler
layout( set = 0, binding = 4 ) uniform sampler2D Sampler4;
// s5: StripeSampler, EFFECT 3 VignetteSampler
layout( set = 0, binding = 5 ) uniform sampler2D Sampler5;
// s6: EFFECT 3 VignetteTileSampler
layout( set = 0, binding = 6 ) uniform sampler2D VignetteTileSampler;

layout( location = 0 ) in vec4 vBaseAndSeamlessTexCoord; // EFFECT 2, 3: vBaseTexCoord in xy
layout( location = 1 ) in vec2 vStripeSeamlessTexCoord;
layout( location = 2 ) in vec2 vLightmapBlendTexCoord;
layout( location = 3 ) in vec3 vBlendFactor;
layout( location = 4 ) in vec4 worldPos_projPosZ;
layout( location = 8 ) in vec4 vVertexColor;

#define g_vPyroParms1 PS_C( 0 )
#define g_vPyroParms2 PS_C( 1 )
#define g_vPyroParms3 PS_C( 2 )
#define g_vPyroParms4 PS_C( 3 )
#define g_vPyroParms5 PS_C( 4 )
#define g_vPyroParms6 PS_C( 5 )

// STRIPES
#define g_vStripeColor ( g_vPyroParms6.rgb )
#define g_flStripeLMScale ( g_vPyroParms6.a )

// EFFECT 0
#define g_vBaseStepRange ( g_vPyroParms1.xy )
#define g_vLightmapStepRange ( g_vPyroParms1.zw )
#define g_vColorModulation ( g_vPyroParms2.rgb )
#define g_vCanvasStepRange ( g_vPyroParms3.xy )
#define g_vCanvasColorStart ( g_vPyroParms4.rgb )
#define g_vCanvasColorEnd ( g_vPyroParms5.rgb )

// EFFECT 1
#define g_flGrayPower ( g_vPyroParms1.x )
#define g_flGrayStep ( g_vPyroParms1.yz )
#define g_flLightMapGradients ( g_vPyroParms1.w )
#define g_flDiffuseLighting ( g_vPyroParms2.w )
#define g_flDiffuseBase ( g_vPyroParms3.x )
#define g_vSelfIllumTint ( g_vPyroParms3.yzw )

// EFFECT 2
#define g_flDoFStartDistance ( g_vPyroParms1.x )
#define g_flDoFPower ( g_vPyroParms1.y )
#define g_flDoFMax ( g_vPyroParms1.z )

// EFFECT 3
#define g_flNoiseScale ( g_vPyroParms1.x )
#define g_flTimeScale ( g_vPyroParms1.y )
#define g_flHeatHazeScale ( g_vPyroParms1.z )

#define g_EyePos ( PS_C( 10 ).xyz )
#define g_FogParams PS_C( 11 )
#define g_vGeneralPyroParms1 PS_C( 12 )
#define g_flTime ( g_vGeneralPyroParms1.y )

bool BASETEXTURE2()
{
	return STATIC_PS_COMBO( 64, 2 ) != 0;
}

void HandleBlending( vec2 vBaseTextureCoord, vec3 blendFactor, inout vec4 vBaseColor )
{
	if ( !BASETEXTURE2() )
		return;
	const bool FANCY_BLENDING = STATIC_PS_COMBO( 128, 2 ) != 0;

	vec4 vBaseColor2 = tex2D( 4, Sampler4, vBaseTextureCoord );
	float flBlendFactor = blendFactor.z;

	if ( FANCY_BLENDING )
	{
		vec4 modt = tex2D( 3, Sampler3, blendFactor.xy );
		float minb = saturate( modt.g - modt.r );
		float maxb = saturate( modt.g + modt.r );
		flBlendFactor = smoothstep( minb, maxb, flBlendFactor );
	}

	vBaseColor = mix( vBaseColor, vBaseColor2, flBlendFactor );
}

void CalculateStripe( vec2 stripeSeamlessTexCoord, float flLMScale, inout vec3 vResult )
{
	vec4 vStripeColor = tex2D( 5, Sampler5, stripeSeamlessTexCoord );

	vStripeColor.rgb *= g_vStripeColor;
	vStripeColor.rgb *= mix( vec3( 1.0, 1.0, 1.0 ), vec3( flLMScale ), g_flStripeLMScale );

	vResult = mix( vResult, vStripeColor.rgb, vStripeColor.a );
}

vec4 Effect0()
{
	const bool VERTEX_LIT = STATIC_PS_COMBO( 32, 2 ) != 0;

	vec4 vBaseColor = tex2D( 0, BaseTextureSampler, vBaseAndSeamlessTexCoord.xy );
	HandleBlending( vBaseAndSeamlessTexCoord.xy, vBlendFactor, vBaseColor );

	vBaseColor.rgb *= vVertexColor.rgb;
	vBaseColor.rgb = ceil( vBaseColor.rgb * 16.0 ) / 16.0;

	vec4 vLightmapSample = vec4( 0.0 );
	if ( !VERTEX_LIT )
		vLightmapSample = tex2D( 1, LightmapSampler, vLightmapBlendTexCoord.xy ) *
		                  vec4( g_vColorModulation, 1.0 );
	vec4 vCanvas = tex2D( 2, Sampler2, vBaseAndSeamlessTexCoord.zw );

	float flFogFactor = CalcPixelFogFactor( PIXEL_FOG_TYPE_RANGE, g_FogParams, g_EyePos.z,
	    worldPos_projPosZ.z, worldPos_projPosZ.w );

	vec4 vResult;
	vResult.rgb = smoothstep( g_vBaseStepRange.xxx, g_vBaseStepRange.yyy, vBaseColor.rgb );
	vResult.a = vBaseColor.a;

	if ( !VERTEX_LIT )
	{
		vLightmapSample.rgb =
		    smoothstep( g_vLightmapStepRange.xxx, g_vLightmapStepRange.yyy, vLightmapSample.rgb );
		vResult.rgb *= vLightmapSample.rgb;
	}

	float flCanvasGray = dot( vCanvas.rgb, vec3( 0.30, 0.59, 0.11 ) );
	flCanvasGray = smoothstep( g_vCanvasStepRange.x, g_vCanvasStepRange.y, flCanvasGray );

	vCanvas.rgb = mix( g_vCanvasColorStart, g_vCanvasColorEnd, flCanvasGray );

	vResult *= vCanvas;

	return FinalOutput( vResult, flFogFactor, PIXEL_FOG_TYPE_RANGE, TONEMAP_SCALE_LINEAR, true,
	    worldPos_projPosZ.w );
}

vec4 Effect1()
{
	const bool VERTEX_LIT = STATIC_PS_COMBO( 32, 2 ) != 0;
	const bool SELFILLUM = STATIC_PS_COMBO( 256, 2 ) != 0;
	const bool COLOR_BAR = STATIC_PS_COMBO( 512, 2 ) != 0;
	const bool STRIPES = STATIC_PS_COMBO( 1024, 2 ) != 0;

	vec4 vResult;

	vec4 vBaseColor = tex2D( 0, BaseTextureSampler, vBaseAndSeamlessTexCoord.xy );
	HandleBlending( vBaseAndSeamlessTexCoord.xy, vBlendFactor, vBaseColor );

	float flFogFactor = CalcPixelFogFactor( PIXEL_FOG_TYPE_RANGE, g_FogParams, g_EyePos.z,
	    worldPos_projPosZ.z, worldPos_projPosZ.w );

	vBaseColor.rgb = mix( vBaseColor.rgb, vec3( 1.0 ), g_flDiffuseBase );
	vResult = vBaseColor * vVertexColor;

	if ( !VERTEX_LIT )
	{
		vec4 vLightmapSample = tex2D( 1, LightmapSampler, vLightmapBlendTexCoord.xy ) *
		                       vec4( g_vColorModulation, 1.0 );
		vResult.rgb = mix( vResult.rgb, vec3( 0.5 ), g_flDiffuseLighting );
		vResult.rgb *= vLightmapSample.rgb;
	}

	if ( SELFILLUM )
	{
		vec3 vSelfIllumComponent = g_vSelfIllumTint * vBaseColor.rgb;
		vResult.rgb = mix( vResult.rgb, vSelfIllumComponent, vBaseColor.a );
	}

	float flGray = dot( vResult.rgb, vec3( 0.30, 0.59, 0.11 ) );

	flGray = HlslPow( flGray, g_flGrayPower );
	flGray = smoothstep( g_flGrayStep.x, g_flGrayStep.y, flGray );

	flGray = ceil( flGray * g_flLightMapGradients ) / g_flLightMapGradients;

	if ( COLOR_BAR )
	{
		// tex1D reads the 2D color bar at ( flGray, flGray ).
		vec4 vCanvas = tex2D( 2, Sampler2, vec2( flGray ) );
		vResult.rgb = vCanvas.rgb * flGray;
	}

	if ( STRIPES )
		CalculateStripe( vStripeSeamlessTexCoord, flGray, vResult.rgb );

	return FinalOutput( vResult, flFogFactor, PIXEL_FOG_TYPE_RANGE, TONEMAP_SCALE_LINEAR, false,
	    worldPos_projPosZ.w );
}

vec4 Effect2()
{
	const bool VISUALIZE_DOF = DYNAMIC_PS_COMBO( 2, 2 ) != 0;

	float flDepth = tex2D( 0, BaseTextureSampler, vBaseAndSeamlessTexCoord.xy ).r;
	vec4 vBaseColor = tex2D( 4, Sampler4, vBaseAndSeamlessTexCoord.xy );

	float flAmount = saturate( flDepth - g_flDoFStartDistance );
	flAmount = HlslPow( flAmount, g_flDoFPower );
	flAmount *= g_flDoFMax;

	if ( VISUALIZE_DOF )
		return vec4( vec3( flAmount ), 1.0 );
	return vec4( vBaseColor.rgb, flAmount );
}

vec4 Effect3()
{
	const bool HEATHAZE = DYNAMIC_PS_COMBO( 4, 2 ) != 0;
	const vec2 vBaseTexCoord = vBaseAndSeamlessTexCoord.xy;

	float flVignetteAmount = tex2D( 5, Sampler5, vBaseTexCoord.xy ).x;
	vec4 vVignetteTile = tex2D( 6, VignetteTileSampler, vBaseTexCoord.xy * 50.0 );

	vec4 vOriginal;
	if ( HEATHAZE )
	{
		vec2 vOffset = tex2D( 4, Sampler4, vBaseTexCoord.xy ).aa;
		const vec2 noiseCoord = vec2(
		    vBaseTexCoord.x, ( vBaseTexCoord.y * g_flNoiseScale ) + ( g_flTime * g_flTimeScale ) );
		float flDynamicOffset = tex2D( 3, Sampler3, noiseCoord ).r;

		vOffset = smoothstep( 0.1, 1.0, vOffset );
		vOffset *= flDynamicOffset * g_flHeatHazeScale;
		vOriginal = tex2D( 0, BaseTextureSampler, vBaseTexCoord.xy + vec2( 0.0, vOffset.y ) );
	}
	else
	{
		vOriginal = tex2D( 0, BaseTextureSampler, vBaseTexCoord.xy );
	}

	float flDelta = flVignetteAmount - vVignetteTile.a;

	float flFinal =
	    flDelta <= 0.0 ? 0.0 : mix( flVignetteAmount, 1.0, flDelta / flVignetteAmount );
	vOriginal.rgb = mix( vOriginal.rgb, vVignetteTile.rgb, flFinal * flVignetteAmount );

	return vOriginal;
}

void main()
{
	const int EFFECT = STATIC_PS_COMBO( 8, 4 );
	vec4 result;
	if ( EFFECT == 0 )
		result = Effect0();
	else if ( EFFECT == 1 )
		result = Effect1();
	else if ( EFFECT == 2 )
		result = Effect2();
	else
		result = Effect3();
	LegacyWrite( result );
}
