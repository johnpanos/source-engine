#version 450
// Portal 2's paint on world surfaces: a port of stdshaders/lightmappedpaint_ps2x.fxc
// (the ps20b build, from the CS:GO-era source) for the LightmappedPaint pass the
// engine draws over each painted surface (engine/paint_render.cpp). It runs after
// lightmapped.vert on LightmappedGeneric's layout:
//
//   s0 the paint page (per lightmap luxel: gel color, coverage in alpha),
//   s1 the lightmap page, s2 the paint reflection cube, s4 the bubble layout,
//   s5 the bubble normals, s7 the splat normals (alpha: splat height).
//
// params.y holds the combos (kLightmapped* in vulkan_device.h): 32 CUBEMAP,
// 32768 the paint pass, 65536 THICKPAINT. params.z the kColor* flags as in
// lightmapped.frag (2 sRGB lightmap, 4 sRGB output, 64 sRGB envmap), params.w
// FinalOutput's linear light scale. Registers (lightmappedpaint_dx9.cpp): c10
// the eye, c12 the tint times the lightmap scale, c22/c23 the camera's
// horizontal right and forward, c30.z ENV_MAP_SCALE. (c0, the reflection tint,
// is set but, as in the fxc, not read.)
//
// The splat coordinates are the fxc's paintCoord * 40 with v halved on its
// 512x256 pages, written here in luxels so every page size tiles alike.
layout( location = 0 ) in vec2 vUv;
layout( location = 1 ) in vec4 vColor;
layout( location = 2 ) in vec2 vLightmapUv;
layout( location = 3 ) in float vLightmapOffset;
layout( location = 4 ) in vec3 vWorldPos;
layout( location = 5 ) in vec3 vTangentS;
layout( location = 6 ) in vec3 vTangentT;
layout( location = 7 ) in vec3 vWorldNormal;
layout( location = 8 ) flat in vec4 fragFogColor;
layout( location = 9 ) flat in vec4 fragFogParams;
layout( location = 10 ) flat in vec4 fragFogMisc;
layout( location = 11 ) in vec2 fragFogDepth;
layout( location = 0 ) out vec4 outColor;
layout( set = 0, binding = 0 ) uniform sampler2D PaintSampler;        // s0
layout( set = 1, binding = 0 ) uniform sampler2D LightmapSampler;     // s1
layout( set = 2, binding = 0 ) uniform samplerCube EnvmapSampler;     // s2
layout( set = 3, binding = 0 ) uniform sampler2D BubbleLayoutSampler; // s4
layout( set = 4, binding = 0 ) uniform sampler2D BubbleSampler;       // s5
layout( set = 5, binding = 0 ) uniform sampler2D SplatNormalSampler;  // s7
layout( set = 6, binding = 0 ) uniform PixelShaderConstants
{
	vec4 c[32];
}
ps;
layout( set = 7, binding = 0 ) uniform sampler2D UnusedSampler8;  // s8
layout( set = 8, binding = 0 ) uniform sampler2D UnusedSampler12; // s12
layout( push_constant ) uniform Constants
{
	mat4 viewProj;
	vec4 unused0;
	vec4 unused1;
	vec4 eyePos;
	vec4 params;
	vec4 params2;
	vec4 clipPlanes[2];
}
consts;

const int kCubemap = 32;
const int kThickPaint = 65536;

const int kSrgbLightmap = 2;
const int kSrgbOutput = 4;
const int kSrgbEnvmap = 64;

// common_fxc.h
const float OO_SQRT_3 = 0.57735025882720947;
const vec3 bumpBasis[3] = vec3[3]( vec3( 0.81649661064147949, 0.0, OO_SQRT_3 ),
    vec3( -0.40824833512306213, 0.70710676908493042, OO_SQRT_3 ),
    vec3( -0.40824821591377258, -0.7071068286895752, OO_SQRT_3 ) );

vec3 SrgbToLinear( vec3 c )
{
	return mix( c / 12.92, pow( ( c + 0.055 ) / 1.055, vec3( 2.4 ) ), step( 0.04045, c ) );
}
vec3 LinearToSrgb( vec3 c )
{
	c = clamp( c, 0.0, 1.0 );
	return mix( c * 12.92, 1.055 * pow( c, vec3( 1.0 / 2.4 ) ) - 0.055, step( 0.0031308, c ) );
}

vec3 LightMapSample( vec2 uv )
{
	vec3 texel = texture( LightmapSampler, uv ).rgb;
	if ( ( int( consts.params.z ) & kSrgbLightmap ) != 0 )
		texel = SrgbToLinear( texel );
	return texel;
}

// common_ps_fxc.h CalcPixelFogFactor and BlendPixelFog, as lightmapped.frag.
float PixelFogFactor()
{
	const float fogType = fragFogColor.w;
	if ( fogType < -0.5 )
		return 0.0;
	const float projZ = fragFogDepth.x;
	if ( fogType < 0.5 )
		return clamp( min( fragFogParams.z, projZ * fragFogParams.w - fragFogParams.x ), 0.0, 1.0 );
	const float worldZ = fragFogDepth.y;
	const float depthFromWater = fragFogParams.y - worldZ;
	const float depthFromEye = fragFogMisc.x - worldZ;
	const float f = clamp( depthFromWater * ( 1.0 / depthFromEye ), 0.0, 1.0 );
	return clamp( f * projZ * fragFogParams.w, 0.0, 1.0 );
}

vec3 BlendPixelFog( vec3 color, float factor )
{
	const float fogType = fragFogColor.w;
	if ( fogType < -0.5 )
		return color;
	if ( fogType < 0.5 )
		factor *= factor;
	return mix( color, fragFogColor.rgb, factor );
}

// common_fxc.h CalcReflectionVectorUnnormalized.
vec3 Reflect( vec3 normal, vec3 eyeVector )
{
	return 2.0 * dot( normal, eyeVector ) * normal - dot( normal, normal ) * eyeVector;
}

void main()
{
	const int combos = int( consts.params.y );
	const int flags = int( consts.params.z );
	const bool bThickPaint = ( combos & kThickPaint ) != 0;
	const bool bCubemap = ( combos & kCubemap ) != 0;

	// The fxc's tangent-space basis: rows S, T, N (tangentSpaceTranspose).
	const mat3 tangentToWorld = mat3( vTangentS, vTangentT, vWorldNormal );
	const vec3 worldVertToEyeVector = ps.c[10].xyz - vWorldPos;

	// paintCoord: the flat lightmap coordinate (the bumped ones minus their offset).
	const vec2 paintCoord = vLightmapUv;
	const vec4 paintColor = texture( PaintSampler, paintCoord );

	vec3 lightmapColor1 = vec3( 1.0 );
	vec3 lightmapColor2 = vec3( 1.0 );
	vec3 lightmapColor3 = vec3( 1.0 );
	const vec2 lightmapOffset = vec2( vLightmapOffset, 0.0 );
	if ( bThickPaint )
	{
		lightmapColor1 = LightMapSample( paintCoord + lightmapOffset );
		lightmapColor2 = LightMapSample( paintCoord + 2.0 * lightmapOffset );
		lightmapColor3 = LightMapSample( paintCoord + 3.0 * lightmapOffset );
	}
	else
	{
		lightmapColor1 = LightMapSample( paintCoord );
	}

	const vec2 coords = paintCoord * vec2( textureSize( PaintSampler, 0 ) ) * ( 40.0 / 512.0 );

	// The fxc's clip( alpha - 0.01 ): the pixel is killed at the end, so the
	// texture fetches below keep their quad's derivatives (after OpKill they
	// are undefined, which picks random mips).
	const bool bClipped = paintColor.a - 0.01 < 0.0;

	// Paint splat guts.
	vec3 fvNormalTs = vec3( 0.0, 0.0, 1.0 );
	const vec4 fvSplats = texture( SplatNormalSampler, coords );

	const vec3 fvViewDirectionTs = normalize( worldVertToEyeVector * tangentToWorld );

	float alpha = ( paintColor.a + fvSplats.a ) * 0.5; // the combined alpha
	const float fPaintThickness = smoothstep( 0.34, 0.63, alpha );
	vec4 cMixedPaint = paintColor;

	if ( bThickPaint )
	{
		const vec4 fvSplatsPushed = texture( SplatNormalSampler, coords + fvViewDirectionTs.xy * fvSplats.a * 0.01 );
		float bumpedPaintArea = smoothstep( 0.6, 0.3, alpha ); // the edges: super-bumped
		bumpedPaintArea *= fPaintThickness;

		fvNormalTs = fvSplatsPushed.xyz * 2.0 - 1.0;
		fvNormalTs.xy *= mix( 0.1, 4.0, bumpedPaintArea );
		// Distorted coordinates, so the blend between paint colors is not blocky.
		const vec2 fvPaintCoordsDistorted = paintCoord + ( 0.003 - 0.006 * alpha );
		cMixedPaint = texture( PaintSampler, fvPaintCoordsDistorted );
		// Fade the distortion out toward the edges, where it would fetch too far.
		cMixedPaint.rgb = mix( paintColor.rgb, cMixedPaint.rgb, smoothstep( 0.9, 1.0, cMixedPaint.a ) );
	}
	else
	{
		fvNormalTs = fvSplats.xyz * 2.0 - 1.0;
	}

	// The paint textures are filtered in gamma space: read them as sRGB here.
	cMixedPaint.rgb = SrgbToLinear( cMixedPaint.rgb );

	vec3 vNormalWs = tangentToWorld * fvNormalTs;
	vec4 fvBubbles = vec4( 0.0 );
	vec4 fvDeepBubbles = vec4( 0.0 );
	vec4 worldSpaceBubblesNormal = vec4( 0.0 );
	if ( bThickPaint )
	{
		vec4 vN;
		vN.x = dot( ps.c[22].xyz, vNormalWs );
		vN.y = dot( ps.c[23].xyz, vNormalWs );
		vN.wz = vN.xy;

		float flD = paintColor.a + fvSplats.a;
		flD *= smoothstep( 1.0, 2.0, flD );

		const vec4 vDependentTexCoords = vN * flD;
		const vec2 vRefractTexCoord = vDependentTexCoords.wz;

		vec2 fvLayoutCoords = coords * 6.5 + fvViewDirectionTs.xy * 0.0014;
		const vec2 fvDeepLayoutCoords = fvLayoutCoords * 1.76 + fvViewDirectionTs.xy * 0.0007;
		// Single-step parallax, so the bubbles look higher than the surface they are on.
		fvLayoutCoords += fvViewDirectionTs.xy * 0.03;
		vec4 fvLayout = texture( BubbleLayoutSampler, fvLayoutCoords );
		vec4 fvDeepLayout = texture( BubbleLayoutSampler, fvDeepLayoutCoords );
		fvLayout.xy -= 0.5;
		fvLayout.z *= smoothstep( 0.750, 0.80, alpha );
		fvDeepLayout.xy -= 0.5;
		fvDeepLayout.z *= smoothstep( 0.40, 0.75, alpha );

		// Camera-facing bubble coordinates, so the bubbles always look round.
		mat3 matCameraFaceUVs;
		matCameraFaceUVs[2] = fvViewDirectionTs;
		matCameraFaceUVs[1] = vec3( 0.0, 1.0, 0.0 );
		matCameraFaceUVs[0] = normalize( cross( matCameraFaceUVs[1], matCameraFaceUVs[2] ) );
		matCameraFaceUVs[1] = normalize( cross( matCameraFaceUVs[2], matCameraFaceUVs[0] ) );

		vec2 fvBubbleCoords;
		fvBubbleCoords.x = dot( fvLayout.xy, matCameraFaceUVs[0].xy );
		fvBubbleCoords.y = dot( fvLayout.xy, matCameraFaceUVs[1].xy );

		vec2 fvDeepBubbleCoords;
		fvDeepBubbleCoords.x = dot( fvDeepLayout.xy, matCameraFaceUVs[0].xy );
		fvDeepBubbleCoords.y = dot( fvDeepLayout.xy, matCameraFaceUVs[1].xy );

		const vec2 fvBubbleSurfaceCoords = fvLayout.xy * 0.5 + 0.5;
		fvBubbleCoords = fvBubbleCoords + 0.5 + vRefractTexCoord * 0.05;
		fvDeepBubbleCoords = fvDeepBubbleCoords + 0.5 + vRefractTexCoord * 0.2;

		fvDeepBubbles = texture( BubbleSampler, fvDeepBubbleCoords );
		fvDeepBubbles = mix( vec4( 0.5, 0.5, 1.0, 0.0 ), fvDeepBubbles, fvDeepLayout.z );
		fvBubbles = texture( BubbleSampler, fvBubbleCoords );
		fvBubbles = mix( vec4( 0.5, 0.5, 1.0, 0.0 ), fvBubbles, fvLayout.z );
		vec3 fvSurfaceBubbles = texture( BubbleSampler, fvBubbleSurfaceCoords ).xyz;
		fvSurfaceBubbles = fvSurfaceBubbles * 2.0 - 1.0;
		fvSurfaceBubbles.xy *= -2.0;

		fvNormalTs += fvSurfaceBubbles * pow( fvLayout.z, 3.5 ) * 0.4;
		fvNormalTs = normalize( fvNormalTs );

		worldSpaceBubblesNormal = mix( fvDeepBubbles * vec4( 1.0, 1.0, 1.0, 0.4 ), fvBubbles, fvBubbles.a );
		worldSpaceBubblesNormal.xyz = tangentToWorld * ( worldSpaceBubblesNormal.xyz * 2.0 - 1.0 );
		worldSpaceBubblesNormal = normalize( worldSpaceBubblesNormal );
	}

	vec3 diffuseLighting;
	if ( bThickPaint )
	{
		vec3 dp;
		dp.x = clamp( dot( fvNormalTs, bumpBasis[0] ), 0.0, 1.0 );
		dp.y = clamp( dot( fvNormalTs, bumpBasis[1] ), 0.0, 1.0 );
		dp.z = clamp( dot( fvNormalTs, bumpBasis[2] ), 0.0, 1.0 );
		dp *= dp;
		diffuseLighting = dp.x * lightmapColor1 + dp.y * lightmapColor2 + dp.z * lightmapColor3;
		const float sum = dot( dp, vec3( 1.0 ) );
		diffuseLighting *= ps.c[12].rgb / sum;
	}
	else
	{
		diffuseLighting = lightmapColor1 * ps.c[12].rgb;
	}
	const vec3 diffuseComponent = diffuseLighting;

	// Less thick areas are less saturated, and darker.
	const float fLum = dot( cMixedPaint.rgb, vec3( 0.299, 0.587, 0.114 ) ) * 0.5;
	const vec3 cPaint = mix( vec3( fLum ), cMixedPaint.rgb, fPaintThickness );

	vNormalWs = tangentToWorld * fvNormalTs;

	vec3 specularLighting = vec3( 0.0 );
	const vec3 eyeVect = normalize( worldVertToEyeVector );
	const float fresnel = dot( vNormalWs, eyeVect );
	if ( bCubemap )
	{
		// ENV_MAP_SCALE / 16: the code was tuned where env_map_scale is 16.
		vec3 envmap = texture( EnvmapSampler, Reflect( vNormalWs, worldVertToEyeVector ) ).rgb;
		if ( ( flags & kSrgbEnvmap ) != 0 )
			envmap = SrgbToLinear( envmap );
		specularLighting += ( ps.c[30].z / 16.0 ) * envmap * ( 1.5 - fresnel ) * 0.67;
		if ( bThickPaint )
		{
			vec3 bubbleEnvmap = texture( EnvmapSampler, Reflect( worldSpaceBubblesNormal.xyz, worldVertToEyeVector ) ).rgb;
			if ( ( flags & kSrgbEnvmap ) != 0 )
				bubbleEnvmap = SrgbToLinear( bubbleEnvmap );
			specularLighting += ( ps.c[30].z / 16.0 ) * bubbleEnvmap * cPaint * worldSpaceBubblesNormal.a * fresnel * 2.0;
		}
		specularLighting *= ( 1.0 + diffuseComponent );
	}

	alpha = smoothstep( 0.40, 0.45, alpha );
	if ( bThickPaint )
	{
		const float alphaBubbles = fvBubbles.a * 0.3 + fvDeepBubbles.a * 0.2;
		alpha = alpha - alphaBubbles * 1.25;
	}
	alpha *= 0.85;

	// Slightly emissive, so the paint color shows on dark surfaces; the tint
	// matches paintblob_ps20b.
	vec3 fvTotalDiffuse = ( diffuseComponent * 0.82 + 0.04 ) * cPaint;
	fvTotalDiffuse *= vec3( 0.8, 0.85, 1.0 );
	vec3 result = fvTotalDiffuse + specularLighting;

	// FinalOutput( ..., TONEMAP_SCALE_LINEAR ).
	result *= consts.params.w;
	result = BlendPixelFog( result, PixelFogFactor() );
	if ( ( flags & kSrgbOutput ) != 0 )
		result = LinearToSrgb( result );
	if ( bClipped )
		discard;
	outColor = vec4( result, clamp( alpha, 0.0, 1.0 ) );
}
