#version 450
// The PBR shader's pixel stage (materialsystem/stdshaders/pbr_dx9.cpp): a port
// of stdshaders/pbr_ps30.fxc and pbr_common_ps2_3_x.h. Combos
// (fxctmp9/pbr_ps30.inc): static FLASHLIGHT (80), FLASHLIGHTDEPTHFILTERMODE
// (160, 0..2), LIGHTMAPPED (480), USEENVAMBIENT (960), EMISSIVE (1920),
// SPECULAR (3840), PARALLAXOCCLUSION (7680); dynamic WRITEWATERFOGTODESTALPHA
// (1), PIXELFOGTYPE (2), NUM_LIGHTS (4, 0..4), WRITE_DEPTH_TO_DESTALPHA (20),
// FLASHLIGHTSHADOWS (40). FLASHLIGHT and its shadow combos are never selected
// here (the backend reports no flashlight mode), so the flashlight pass is not
// ported. The tangent frame comes from the screen-space derivatives of the
// world position and texture coordinate, as the HLSL computes it.
// @legacy program=pbr ps=pbr_ps30 vs=pbr_vs30 vert=pbr_vs30
//         samplers=0:2d,1:2d,2:cube,11:2d,12:2d,7:2d,10:2d
#include "legacy_ps.glsl"
#include "legacy_bumpbasis.glsl"
#include "legacy_ps_lighting.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler;     // s0
layout( set = 0, binding = 1 ) uniform sampler2D NormalTextureSampler;   // s1
layout( set = 0, binding = 2 ) uniform samplerCube EnvmapSampler;        // s2
layout( set = 0, binding = 3 ) uniform sampler2D EmissionTextureSampler; // s11
layout( set = 0, binding = 4 ) uniform sampler2D SpecularTextureSampler; // s12
layout( set = 0, binding = 5 ) uniform sampler2D LightmapSampler;        // s7
layout( set = 0, binding = 6 ) uniform sampler2D MRAOTextureSampler;     // s10

layout( location = 0 ) in vec2 baseTexCoord;
layout( location = 1 ) in vec4 lightAtten;
layout( location = 2 ) in vec3 worldNormal;
layout( location = 3 ) in vec3 worldPos;
layout( location = 4 ) in vec3 projPos;
layout( location = 5 ) in vec4 lightmapTexCoord1And2;
layout( location = 6 ) in vec4 lightmapTexCoord3;

#define g_DiffuseModulation PS_C( PSREG_DIFFUSE_MODULATION )
#define g_EyePos PS_C( PSREG_EYEPOS_SPEC_EXPONENT )
#define g_FogParams PS_C( PSREG_FOG_PARAMS )
#define g_BaseColor PS_C( PSREG_SELFILLUMTINT )
#define PARALLAX_DEPTH ( PS_C( 27 ).r )
#define PARALLAX_CENTER ( PS_C( 27 ).g )
#define ENVMAPLOD ( g_EyePos.a )

#define texCUBElod( set, s, dir, lod ) LegacyTexel( ( set ), textureLod( ( s ), ( dir ), ( lod ) ) )

const float PI = 3.141592;
const float EPSILON = 0.00001;

bool LIGHTMAPPED;
bool SPECULAR;

// pow( x, 5 ), which fxc compiles as products.
float Pow5( float x )
{
	const float x2 = x * x;
	return x2 * x2 * x;
}

// Shlick's approximation of the Fresnel factor
vec3 fresnelSchlick( vec3 F0, float cosTheta )
{
	return F0 + ( 1.0 - F0 ) * Pow5( 1.0 - cosTheta );
}

// GGX/Towbridge-Reitz normal distribution function (alpha = roughness^2)
float ndfGGX( float cosLh, float roughness )
{
	const float alpha = roughness * roughness;
	const float alphaSq = alpha * alpha;
	const float denom = ( cosLh * cosLh ) * ( alphaSq - 1.0 ) + 1.0;
	return alphaSq / ( PI * denom * denom );
}

float gaSchlickG1( float cosTheta, float k )
{
	return cosTheta / ( cosTheta * ( 1.0 - k ) + k );
}

// Schlick-GGX geometric attenuation with Smith's method
float gaSchlickGGX( float cosLi, float cosLo, float roughness )
{
	const float r = roughness + 1.0;
	const float k = ( r * r ) / 8.0;
	return gaSchlickG1( cosLi, k ) * gaSchlickG1( cosLo, k );
}

vec3 EnvBRDFApprox( vec3 SpecularColor, float Roughness, float NoV )
{
	const vec4 c0 = vec4( -1.0, -0.0275, -0.572, 0.022 );
	const vec4 c1 = vec4( 1.0, 0.0425, 1.04, -0.04 );
	const vec4 r = Roughness * c0 + c1;
	const float a004 = min( r.x * r.x, exp2( -9.28 * NoV ) ) * r.x + r.y;
	const vec2 AB = vec2( -1.04, 1.04 ) * a004 + r.zw;
	return SpecularColor * AB.x + AB.y;
}

// The matrix taking tangent-space normals to world space, from the screen-space
// derivatives of the position and texture coordinate (rows T, B, N).
void compute_tangent_frame( vec3 N, vec3 P, vec2 uv, out vec3 T, out vec3 B, out float sign_det )
{
	const vec3 dp1 = dFdx( P );
	const vec3 dp2 = dFdy( P );
	const vec2 duv1 = dFdx( uv );
	const vec2 duv2 = dFdy( uv );

	sign_det = dot( dp2, cross( N, dp1 ) ) > 0.0 ? -1.0 : 1.0;

	const vec3 M1 = dp2;
	const vec3 M2 = cross( dp1, dp2 );
	const vec3 inverseM0 = cross( M1, M2 );
	const vec3 inverseM1 = cross( M2, dp1 );
	T = normalize( duv1.x * inverseM0 + duv2.x * inverseM1 );
	B = normalize( duv1.y * inverseM0 + duv2.y * inverseM1 );
}

// Direct light for one source
vec3 calculateLight( vec3 lightIn, vec3 lightIntensity, vec3 lightOut, vec3 normal,
    vec3 fresnelReflectance, float roughness, float metalness, float lightDirectionAngle, vec3 albedo )
{
	const vec3 HalfAngle = normalize( lightIn + lightOut );
	const float cosLightIn = max( 0.0, dot( normal, lightIn ) );
	const float cosHalfAngle = max( 0.0, dot( normal, HalfAngle ) );

	const vec3 F = fresnelSchlick( fresnelReflectance, max( 0.0, dot( HalfAngle, lightOut ) ) );
	const float D = ndfGGX( cosHalfAngle, roughness );
	const float G = gaSchlickGGX( cosLightIn, lightDirectionAngle, roughness );

	vec3 kd;
	if ( SPECULAR )
		kd = vec3( 1.0 ) - F; // Metalness is not used if F0 map is available
	else
		kd = mix( vec3( 1.0 ) - F, vec3( 0.0 ), metalness );

	const vec3 diffuseBRDF = kd * albedo;
	const vec3 specularBRDF = ( F * D * G ) / max( EPSILON, 4.0 * cosLightIn * lightDirectionAngle );
	// Static lights are already in the lightmap.
	if ( LIGHTMAPPED )
		return specularBRDF * lightIntensity * cosLightIn;
	return ( diffuseBRDF + specularBRDF ) * lightIntensity * cosLightIn;
}

// common_vertexlitgeneric_dx9.h's PixelShaderAmbientLight on a cube in registers
// or from the env map.
vec3 AmbientCubeLight( vec3 worldNormal, vec3 cube[6] )
{
	const vec3 nSquared = worldNormal * worldNormal;
	const vec3 isNegative = mix( nSquared, vec3( 0.0 ), greaterThanEqual( worldNormal, vec3( 0.0 ) ) );
	const vec3 isPositive = mix( vec3( 0.0 ), nSquared, greaterThanEqual( worldNormal, vec3( 0.0 ) ) );
	return isPositive.x * cube[0] + isNegative.x * cube[1] + isPositive.y * cube[2] +
	       isNegative.y * cube[3] + isPositive.z * cube[4] + isNegative.z * cube[5];
}

vec3 ambientLookupLightmap( vec3 textureNormal )
{
	vec2 bumpCoord1, bumpCoord2, bumpCoord3;
	ComputeBumpedLightmapCoordinates(
	    lightmapTexCoord1And2, lightmapTexCoord3.xy, bumpCoord1, bumpCoord2, bumpCoord3 );

	const vec3 lightmapColor1 = tex2D( 5, LightmapSampler, bumpCoord1 ).rgb;
	const vec3 lightmapColor2 = tex2D( 5, LightmapSampler, bumpCoord2 ).rgb;
	const vec3 lightmapColor3 = tex2D( 5, LightmapSampler, bumpCoord3 ).rgb;

	vec3 dp;
	dp.x = saturate( dot( textureNormal, bumpBasis[0] ) );
	dp.y = saturate( dot( textureNormal, bumpBasis[1] ) );
	dp.z = saturate( dot( textureNormal, bumpBasis[2] ) );
	dp *= dp;

	vec3 diffuseLighting = dp.x * lightmapColor1 + dp.y * lightmapColor2 + dp.z * lightmapColor3;
	const float sum = dot( dp, vec3( 1.0 ) );
	diffuseLighting *= g_DiffuseModulation.xyz / sum;
	return diffuseLighting;
}

vec2 parallaxCorrect( vec2 texCoord, vec3 viewRelativeDir, float parallaxDepth, float parallaxCenter )
{
	const float fLength = length( viewRelativeDir );
	const float fParallaxLength =
	    sqrt( fLength * fLength - viewRelativeDir.z * viewRelativeDir.z ) / viewRelativeDir.z;
	const vec2 vParallaxDirection = normalize( viewRelativeDir.xy );
	vec2 vParallaxOffsetTS = vParallaxDirection * fParallaxLength;
	vParallaxOffsetTS *= parallaxDepth;

	const vec2 dx = dFdx( texCoord );
	const vec2 dy = dFdy( texCoord );

	const int nNumSteps = 20;
	float fCurrHeight = 0.0;
	const float fStepSize = 1.0 / float( nNumSteps );
	float fPrevHeight = 1.0;

	int nStepIndex = 0;
	const vec2 vTexOffsetPerStep = fStepSize * vParallaxOffsetTS;
	vec2 vTexCurrentOffset = texCoord;
	float fCurrentBound = 1.0;

	vec2 pt1 = vec2( 0.0 );
	vec2 pt2 = vec2( 0.0 );

	while ( nStepIndex < nNumSteps )
	{
		vTexCurrentOffset -= vTexOffsetPerStep;

		// The height map is the normal map's alpha.
		fCurrHeight = parallaxCenter +
		              LegacyTexel( 1, textureGrad( NormalTextureSampler, vTexCurrentOffset, dx, dy ) ).a;

		fCurrentBound -= fStepSize;

		if ( fCurrHeight > fCurrentBound )
		{
			pt1 = vec2( fCurrentBound, fCurrHeight );
			pt2 = vec2( fCurrentBound + fStepSize, fPrevHeight );
			nStepIndex = nNumSteps + 1;
		}
		else
		{
			nStepIndex++;
			fPrevHeight = fCurrHeight;
		}
	}

	const float fDelta2 = pt2.x - pt2.y;
	const float fDelta1 = pt1.x - pt1.y;
	const float fParallaxAmount = ( pt1.x * fDelta2 - pt2.x * fDelta1 ) / ( fDelta2 - fDelta1 );
	const vec2 vParallaxOffset = vParallaxOffsetTS * ( 1.0 - fParallaxAmount );
	return texCoord - vParallaxOffset;
}

void main()
{
	LIGHTMAPPED = STATIC_PS_COMBO( 480, 2 ) != 0;
	const bool USEENVAMBIENT = STATIC_PS_COMBO( 960, 2 ) != 0;
	const bool EMISSIVE = STATIC_PS_COMBO( 1920, 2 ) != 0;
	SPECULAR = STATIC_PS_COMBO( 3840, 2 ) != 0;
	const bool PARALLAXOCCLUSION = STATIC_PS_COMBO( 7680, 2 ) != 0;
	const int WRITEWATERFOGTODESTALPHA = DYNAMIC_PS_COMBO( 1, 2 );
	const int PIXELFOGTYPE = DYNAMIC_PS_COMBO( 2, 2 );
	const int NUM_LIGHTS = DYNAMIC_PS_COMBO( 4, 5 );
	const int WRITE_DEPTH_TO_DESTALPHA = DYNAMIC_PS_COMBO( 20, 2 );

	vec3 EnvAmbientCube[6];
	if ( USEENVAMBIENT )
	{
		EnvAmbientCube[0] = ENV_MAP_SCALE * texCUBElod( 2, EnvmapSampler, vec3( 1, 0, 0 ), 12.0 ).rgb;
		EnvAmbientCube[1] = ENV_MAP_SCALE * texCUBElod( 2, EnvmapSampler, vec3( -1, 0, 0 ), 12.0 ).rgb;
		EnvAmbientCube[2] = ENV_MAP_SCALE * texCUBElod( 2, EnvmapSampler, vec3( 0, 1, 0 ), 12.0 ).rgb;
		EnvAmbientCube[3] = ENV_MAP_SCALE * texCUBElod( 2, EnvmapSampler, vec3( 0, -1, 0 ), 12.0 ).rgb;
		EnvAmbientCube[4] = ENV_MAP_SCALE * texCUBElod( 2, EnvmapSampler, vec3( 0, 0, 1 ), 12.0 ).rgb;
		EnvAmbientCube[5] = ENV_MAP_SCALE * texCUBElod( 2, EnvmapSampler, vec3( 0, 0, -1 ), 12.0 ).rgb;
	}
	else
	{
		for ( int i = 0; i < 6; ++i )
			EnvAmbientCube[i] = PS_C( PSREG_AMBIENT_CUBE + i ).rgb;
	}

	const vec3 surfNormal = normalize( worldNormal );
	vec3 surfTangent, surfBase;
	float flipSign;
	compute_tangent_frame( surfNormal, worldPos, baseTexCoord, surfTangent, surfBase, flipSign );

	vec2 correctedTexCoord = baseTexCoord;
	if ( PARALLAXOCCLUSION )
	{
		const vec3 outgoingLightRay = g_EyePos.xyz - worldPos;
		const vec3 outgoingLightDirectionTS = vec3( dot( outgoingLightRay, surfTangent ),
		    dot( outgoingLightRay, surfBase ), dot( outgoingLightRay, surfNormal ) );
		correctedTexCoord =
		    parallaxCorrect( baseTexCoord, outgoingLightDirectionTS, PARALLAX_DEPTH, PARALLAX_CENTER );
	}

	vec3 textureNormal =
	    normalize( ( tex2D( 1, NormalTextureSampler, correctedTexCoord ).xyz - vec3( 0.5 ) ) * 2.0 );
	// mul( textureNormal, float3x3( T, B, N ) ): World Normal
	const vec3 normal = normalize(
	    textureNormal.x * surfTangent + textureNormal.y * surfBase + textureNormal.z * surfNormal );

	vec4 albedo = tex2D( 0, BaseTextureSampler, correctedTexCoord );
	albedo.xyz *= g_BaseColor.xyz;

	const vec3 mrao = tex2D( 6, MRAOTextureSampler, correctedTexCoord ).xyz;
	const float metalness = mrao.x, roughness = mrao.y, ambientOcclusion = mrao.z;

	textureNormal.y *= flipSign; // Fixup textureNormal for ambient lighting

	const vec3 outgoingLightDirection = normalize( g_EyePos.xyz - worldPos );         // Lo
	const float lightDirectionAngle = max( 0.0, dot( normal, outgoingLightDirection ) ); // cosLo
	const vec3 specularReflectionVector =
	    2.0 * lightDirectionAngle * normal - outgoingLightDirection; // Lr

	vec3 fresnelReflectance; // F0
	if ( SPECULAR )
		fresnelReflectance = tex2D( 4, SpecularTextureSampler, correctedTexCoord ).xyz;
	else
		fresnelReflectance = mix( vec3( 0.04 ), albedo.rgb, metalness );

	// Ambient
	const vec3 diffuseIrradiance =
	    LIGHTMAPPED ? ambientLookupLightmap( textureNormal ) : AmbientCubeLight( normal, EnvAmbientCube );
	const vec3 ambientLightingFresnelTerm = fresnelSchlick( fresnelReflectance, lightDirectionAngle );
	const vec3 diffuseContributionFactor = SPECULAR
	                                           ? 1.0 - ambientLightingFresnelTerm
	                                           : mix( 1.0 - ambientLightingFresnelTerm, vec3( 0.0 ), metalness );
	const vec3 diffuseIBL = diffuseContributionFactor * albedo.rgb * diffuseIrradiance;

	const vec3 lookupHigh =
	    ENV_MAP_SCALE * texCUBElod( 2, EnvmapSampler, specularReflectionVector, roughness * ENVMAPLOD ).xyz;
	const vec3 lookupLow = AmbientCubeLight( specularReflectionVector, EnvAmbientCube );
	const vec3 specularIrradiance = mix( lookupHigh, lookupLow, roughness * roughness );
	const vec3 specularIBL =
	    specularIrradiance * EnvBRDFApprox( fresnelReflectance, roughness, lightDirectionAngle );
	const vec3 ambientLighting = ( diffuseIBL + specularIBL ) * ambientOcclusion;

	// Direct
	vec3 directLighting = vec3( 0.0 );
	for ( int n = 0; n < NUM_LIGHTS; ++n )
	{
		const vec3 LightIn = normalize( PixelShaderGetLightVector( worldPos, PSREG_LIGHT_INFO_ARRAY, n ) );
		const vec3 LightColor = PixelShaderGetLightColor( PSREG_LIGHT_INFO_ARRAY, n ) * lightAtten[n]; // Li
		directLighting += calculateLight( LightIn, LightColor, outgoingLightDirection, normal,
		    fresnelReflectance, roughness, metalness, lightDirectionAngle, albedo.rgb );
	}

	const float fogFactor =
	    CalcPixelFogFactor( PIXELFOGTYPE, g_FogParams, g_EyePos.z, worldPos.z, projPos.z );

	float alpha = albedo.a;
	if ( WRITEWATERFOGTODESTALPHA != 0 && PIXELFOGTYPE == PIXEL_FOG_TYPE_HEIGHT )
		alpha = fogFactor;

	const bool bWriteDepthToAlpha = WRITE_DEPTH_TO_DESTALPHA != 0 && WRITEWATERFOGTODESTALPHA == 0;

	vec3 combinedLighting = directLighting + ambientLighting;
	if ( EMISSIVE )
		combinedLighting += tex2D( 3, EmissionTextureSampler, correctedTexCoord ).xyz;

	LegacyWrite( FinalOutput( vec4( combinedLighting, alpha ), fogFactor, PIXELFOGTYPE,
	    TONEMAP_SCALE_LINEAR, bWriteDepthToAlpha, projPos.z ) );
}
