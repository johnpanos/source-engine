#version 450
// Portal 2's paint blob surface (the blobulator isosurface of flowing gel): a
// port of stdshaders/paintblob_ps20b.fxc, drawn after skin.vert on the skin
// pipeline's layout.
//
// The pixel shader constants are the D3D9 registers paintblob_helper.cpp's
// dynamic state writes, c0..c31 as the shader API holds them, read under their
// register names below; c27 carries the combos (see paintblob_helper.cpp).
// skin.vert supplies the world position, the world normal, the eye vector and
// the four lights' attenuation; the three planar projections are computed here
// from the world position with paintblob_vs20's SHADER_SPECIFIC_CONST_0/1
// (UV scale, projection offset), which arrive as texXform0.x and texXform1.xyz.
//
// params: x alpha-test reference (unused), y combo flags (c27.x), z kColor*
// flags (1 sRGB base, 4 sRGB output), w FinalOutput's linear light scale.
// params2.x: NUM_LIGHTS. The environment cube's sRGB read is left to its view:
// Portal 2's paint cube is RGBA16F, which D3D9's SRGBTEXTURE does not decode.
//
// FRESNEL_WARP, OPACITY_TEXTURE, CONTACT_SHADOW and the flashlight are not
// ported (the shader API reports those combos unimplemented); the flashlight
// term is disabled in paintblob_ps20b as well. BACK_SURFACE writes the frame
// copy, without the destination-alpha depth.
layout( location = 0 ) in vec2 vBaseUv;
layout( location = 1 ) in vec4 vLightAtten;
layout( location = 2 ) in vec3 vWorldVertToEye;
layout( location = 3 ) in vec3 vTangentS;
layout( location = 4 ) in vec3 vTangentT;
layout( location = 5 ) in vec3 vNormal;
layout( location = 6 ) in vec3 vWorldPos;
layout( location = 0 ) out vec4 outColor;
layout( set = 0, binding = 0 ) uniform sampler2D g_tBase;             // s0
layout( set = 1, binding = 0 ) uniform sampler2D g_tBump;             // s1
layout( set = 2, binding = 0 ) uniform sampler2D g_tSpecMask;         // s3
layout( set = 3, binding = 0 ) uniform samplerCube g_tEnvironment;    // s7
layout( set = 4, binding = 0 ) uniform sampler2D g_tLightWarp;        // s4
layout( set = 5, binding = 0 ) uniform sampler2D g_tScreen;           // s2
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

#define g_flBumpStrength ps.c[0].x
#define g_flRefractStrength ps.c[0].w
#define g_vTranslucentFresnelParams ps.c[1].xyz
#define g_flInteriorBoost ps.c[1].w
#define g_flRimLightExp ps.c[3].x
#define g_flRimLightScale ps.c[3].y
#define g_flSpecScale ps.c[3].z
#define g_flSpecExp2 ps.c[3].w
#define g_flSpecScale2 ps.c[10].x
#define g_flFresnelBumpStrength ps.c[10].y
#define g_flDiffuseScale ps.c[10].z
#define g_flInteriorLightScale ps.c[10].w
#define g_vEyePos ps.c[11].xyz
#define g_flSpecExp ps.c[11].w
#define g_cBaseTint ps.c[19].rgb
#define g_flInteriorBackLightScale ps.c[19].w
#define ENV_MAP_SCALE ps.c[30].z

// c27.x (paintblob_helper.cpp's PaintBlobStaticFlags_t).
const int kBackSurface = 1;
const int kLightWarp = 2;
const int kInteriorLayer = 16;
const int kEnvMap = 128;

const int kColorSrgbReadBase = 1;
const int kColorSrgbWrite = 4;

const vec3 g_cEnvMapTint = vec3( 1.0 );

vec3 SrgbToLinear( vec3 c )
{
	return mix( c / 12.92, pow( ( c + 0.055 ) / 1.055, vec3( 2.4 ) ), step( 0.04045, c ) );
}
vec3 LinearToSrgb( vec3 c )
{
	c = clamp( c, 0.0, 1.0 );
	return mix( c * 12.92, 1.055 * pow( c, vec3( 1.0 / 2.4 ) ) - 0.055, step( 0.0031308, c ) );
}

float Luminance( vec3 colour )
{
	return dot( colour, vec3( 0.3, 0.59, 0.11 ) );
}

// cAmbientCube: c4..c9.
vec3 PixelShaderAmbientLight( vec3 worldNormal )
{
	vec3 nSquared = worldNormal * worldNormal;
	vec3 isNegative = mix( nSquared, vec3( 0.0 ), greaterThanEqual( worldNormal, vec3( 0.0 ) ) );
	vec3 isPositive = mix( vec3( 0.0 ), nSquared, greaterThanEqual( worldNormal, vec3( 0.0 ) ) );
	return isPositive.x * ps.c[4].xyz + isNegative.x * ps.c[5].xyz + isPositive.y * ps.c[6].xyz +
	       isNegative.y * ps.c[7].xyz + isPositive.z * ps.c[8].xyz + isNegative.z * ps.c[9].xyz;
}

vec3 CubeAverage()
{
	vec3 cAvgLight = vec3( 0.0 );
	for ( int j = 0; j < 6; j++ )
		cAvgLight += ps.c[4 + j].xyz / 6.0;
	return cAvgLight;
}

// cLightInfo: c20..c25, the fourth light spread across the w components.
vec3 PixelShaderGetLightColor( int n )
{
	if ( n == 3 )
		return vec3( ps.c[20].w, ps.c[21].w, ps.c[22].w );
	return ps.c[20 + 2 * n].xyz;
}
vec3 PixelShaderGetLightVector( vec3 worldPos, int n )
{
	vec3 position =
	    n == 3 ? vec3( ps.c[23].w, ps.c[24].w, ps.c[25].w ) : ps.c[21 + 2 * n].xyz;
	return normalize( position - worldPos );
}

// DiffuseTerm with bHalfLambert (paintblob always passes it).
vec3 DiffuseTerm( vec3 worldNormal, vec3 lightDir, bool bDoLightingWarp )
{
	float fResult = clamp( dot( worldNormal, lightDir ) * 0.5 + 0.5, 0.0, 1.0 );
	if ( !bDoLightingWarp )
		fResult *= fResult;
	vec3 fOut = vec3( fResult );
	if ( bDoLightingWarp )
		fOut = 2.0 * texture( g_tLightWarp, vec2( fResult, 0.5 ) ).rgb; // tex1D
	return fOut;
}

// SpecularAndRimTerms without the specular warp (paintblob passes none).
void SpecularAndRimTerms( vec3 vWorldNormal, vec3 vLightDir, float fSpecularExponent,
    vec3 vEyeDir, vec3 color, bool bDoRimLighting, float fRimExponent,
    out vec3 specularLighting, out vec3 rimLighting )
{
	vec3 vHalfAngle = normalize( vEyeDir + vLightDir );
	float flNDotH = clamp( dot( vWorldNormal, vHalfAngle ), 0.0, 1.0 );
	specularLighting = vec3( pow( flNDotH, fSpecularExponent ) );
	specularLighting *= pow( clamp( dot( vWorldNormal, vLightDir ), 0.0, 1.0 ), 0.5 );
	specularLighting *= color;
	rimLighting = vec3( 0.0 );
	if ( bDoRimLighting )
	{
		rimLighting = vec3( pow( flNDotH, fRimExponent ) );
		rimLighting *= clamp( dot( vWorldNormal, vLightDir ), 0.0, 1.0 );
		rimLighting *= color;
	}
}

vec3 ComputeTextureBlendWeights( vec3 vWorldNormal )
{
	vec3 vBlendWeights = max( ( abs( vWorldNormal ) - 0.2 ) * 7.0, 0.0 );
	vBlendWeights /= dot( vBlendWeights, vec3( 1.0 ) );
	return vBlendWeights;
}

vec3 BumpedToWorldNormal( vec3 vBumpedNormal, vec3 vVertexNormal, vec3 vTangentDir )
{
	// mul( row vector, mTanToWorld ) with the rows ( T, B, N ).
	vec3 t = normalize( vTangentDir - dot( vTangentDir, vVertexNormal ) * vVertexNormal );
	vec3 b = cross( t, vVertexNormal );
	return normalize( vBumpedNormal.x * t + vBumpedNormal.y * b + vBumpedNormal.z * vVertexNormal );
}

void main()
{
	const int combos = int( consts.params.y );
	const int flags = int( consts.params.z );
	const int nNumLights = int( consts.params2.x );
	const bool bLightWarp = ( combos & kLightWarp ) != 0;

	vec2 vScreenPos = gl_FragCoord.xy / vec2( textureSize( g_tScreen, 0 ) );
	vec3 vEyeDir = g_vEyePos - vWorldPos;
	float flCamDist = length( vEyeDir );
	vEyeDir /= flCamDist;

	if ( ( combos & kBackSurface ) != 0 )
	{
		// The frame copy (FinalOutput without the depth in alpha).
		vec3 cBack = texture( g_tScreen, vScreenPos ).rgb;
		cBack *= consts.params.w;
		if ( ( flags & kColorSrgbWrite ) != 0 )
			cBack = LinearToSrgb( cBack );
		outColor = vec4( cBack, 1.0 );
		return;
	}

	// paintblob_vs20's three planar projections of the world position.
	vec3 vUVSpacePos = vWorldPos - consts.texXform1.xyz;
	float flUVScale = consts.texXform0.x;
	vec2 uv0 = flUVScale * vUVSpacePos.yz; // vUV0.xy
	vec2 uv1 = flUVScale * vUVSpacePos.xz; // vUV0.wz
	vec2 uv2 = flUVScale * vUVSpacePos.xy; // vUV1.xy
	// The bump fetch reads vUV0.zw, the second projection's axes swapped.
	vec2 uv1Swapped = uv1.yx;

	vec3 vWorldNormal = normalize( vNormal );
	vec3 vBlendWeights = ComputeTextureBlendWeights( vWorldNormal );

	// Base and spec mask
	vec4 cBase = vBlendWeights.x * texture( g_tBase, uv0 ) +
	             vBlendWeights.y * texture( g_tBase, uv1 ) +
	             vBlendWeights.z * texture( g_tBase, uv2 );
	if ( ( flags & kColorSrgbReadBase ) != 0 )
		cBase.rgb = SrgbToLinear( cBase.rgb );
	float flSpecMask = vBlendWeights.x * texture( g_tSpecMask, uv0 ).x +
	                   vBlendWeights.y * texture( g_tSpecMask, uv1 ).x +
	                   vBlendWeights.z * texture( g_tSpecMask, uv2 ).x;

	// Normal mapping (BlendedTexFetchNormal)
	vec3 vNormalTS1 = 2.0 * texture( g_tBump, uv0 ).xyz - 1.0;
	vec3 vNormalTS2 = 2.0 * texture( g_tBump, uv1Swapped ).xyz - 1.0;
	vec3 vNormalTS3 = 2.0 * texture( g_tBump, uv2 ).xyz - 1.0;
	vec2 vBumpedTSNormal = vBlendWeights.x * vNormalTS1.xy + vBlendWeights.y * vNormalTS2.xy +
	                       vBlendWeights.z * vNormalTS3.xy;
	vec3 vBumpedWorldNormal =
	    vBlendWeights.x * BumpedToWorldNormal( vNormalTS1, vWorldNormal, vec3( 0.0, 1.0, 0.0 ) ) +
	    vBlendWeights.y * BumpedToWorldNormal( vNormalTS2, vWorldNormal, vec3( 1.0, 0.0, 0.0 ) ) +
	    vBlendWeights.z * BumpedToWorldNormal( vNormalTS3, vWorldNormal, vec3( 1.0, 0.0, 0.0 ) );
	vec3 vBumpStrengthDir =
	    vBumpedWorldNormal - dot( vBumpedWorldNormal, vWorldNormal ) * vWorldNormal;
	vec3 vFresnelWorldNormal =
	    normalize( vBumpedWorldNormal + ( g_flFresnelBumpStrength - 1.0 ) * vBumpStrengthDir );
	vBumpedWorldNormal =
	    normalize( vBumpedWorldNormal + ( g_flBumpStrength - 1.0 ) * vBumpStrengthDir );

	// Opacity and fresnel (no FRESNEL_WARP or OPACITY_TEXTURE)
	float flFresnel = clamp( 1.0 - dot( vEyeDir, vFresnelWorldNormal ), 0.0, 1.0 );
	float flSkinOpacity = mix( g_vTranslucentFresnelParams.x, g_vTranslucentFresnelParams.y,
	    pow( flFresnel, g_vTranslucentFresnelParams.z ) );

	// Ambient light (paintblob_vs20's AmbientLight of the vertex normal)
	vec3 cAmbient = PixelShaderAmbientLight( vWorldNormal );
	vec3 cAvgAmbient = CubeAverage();

	// Dynamic lights
	vec3 cDiffuse = vec3( 0.0 ), cSpec = vec3( 0.0 ), cSpec2 = vec3( 0.0 ), cRim = vec3( 0.0 );
	for ( int l = 0; l < nNumLights && l < 4; l++ )
	{
		vec3 lightColor = PixelShaderGetLightColor( l );
		vec3 lightDir = PixelShaderGetLightVector( vWorldPos, l );
		cDiffuse += vLightAtten[l] * lightColor *
		            DiffuseTerm( vBumpedWorldNormal, lightDir, bLightWarp );

		vec3 cCurrSpec, cCurrRim;
		SpecularAndRimTerms( vBumpedWorldNormal, lightDir, g_flSpecExp, vEyeDir, lightColor, true,
		    g_flRimLightExp, cCurrSpec, cCurrRim );
		cSpec += vLightAtten[l] * cCurrSpec;
		cRim += vLightAtten[l] * cCurrRim;

		vec3 cCurrSpec2, cDummy;
		SpecularAndRimTerms( vBumpedWorldNormal, lightDir, g_flSpecExp2, vEyeDir, lightColor,
		    false, g_flRimLightExp, cCurrSpec2, cDummy );
		cSpec2 += vLightAtten[l] * cCurrSpec2;
	}
	float flFresnel2 = flFresnel * flFresnel;
	cRim *= flFresnel2 * flFresnel2;

	// Environment reflection
	vec3 cEnvironment = vec3( 0.0 );
	if ( ( combos & kEnvMap ) != 0 )
	{
		vec3 vViewDirReflected = reflect( -vEyeDir, vBumpedWorldNormal );
		vec3 cEnvMap = texture( g_tEnvironment, vViewDirReflected ).rgb;
		cEnvironment = flFresnel * 0.67 * g_cEnvMapTint * cEnvMap * ( ENV_MAP_SCALE / 16.0 );
		cEnvironment *= ( 1.0 + cAmbient + cDiffuse );
	}

	// Scale light terms
	cDiffuse *= g_flDiffuseScale;
	cSpec *= g_flSpecScale;
	cSpec2 *= g_flSpecScale2;
	cRim *= g_flRimLightScale;

	// Slight emissive term so paint shows in the dark (matches lightmappedpaint).
	cDiffuse = ( cDiffuse * 0.82 ) + 0.04;
	cDiffuse *= vec3( 0.8, 0.85, 1.0 );

	vec3 result;
	if ( ( combos & kInteriorLayer ) == 0 )
	{
		// Outer layer only
		result = cBase.rgb * ( cAmbient + cDiffuse ) + flSpecMask * ( cSpec + cSpec2 ) + cRim +
		         cEnvironment;
	}
	else
	{
		// Outer layer blended over the inner (refracted background) colour
		vec3 cExterior = cBase.rgb * ( cAmbient + cDiffuse ) + flSpecMask * ( cSpec2 + cRim );

		float flRefractStrength = ( 2.0 - flFresnel ) * 80.0 * g_flRefractStrength / flCamDist;
		vec3 cBackground =
		    texture( g_tScreen, flRefractStrength * vBumpedTSNormal + vScreenPos ).rgb;
		float flLuminance = Luminance( cBackground );
		cBackground *= 1.0 + g_flInteriorBoost * flLuminance * flLuminance;

		vec3 vRefract = normalize( -( vEyeDir + vWorldNormal ) );
		vec3 cBackLight = PixelShaderAmbientLight( vRefract );
		cBackLight = max( g_flInteriorLightScale * cAvgAmbient,
		    g_flInteriorBackLightScale * cBackLight );
		float flRimEffect = clamp( dot( vEyeDir, vWorldNormal ), 0.0, 1.0 );
		vec3 cInterior = mix( cBackLight, cBackground, sqrt( flRimEffect ) );

		cInterior *= cBase.rgb;
		cInterior += cSpec + cEnvironment;
		result = mix( cInterior, cExterior, flSkinOpacity );
	}

	// FinalOutput( ..., TONEMAP_SCALE_LINEAR ), opaque.
	result *= consts.params.w;
	if ( ( flags & kColorSrgbWrite ) != 0 )
		result = LinearToSrgb( result );
	outColor = vec4( result, 1.0 );
}
