// common_vertexlitgeneric_dx9.h's pixel-shader lighting for the legacy shader
// ports: the ambient cube read per pixel (cAmbientCube[6]), the four local
// lights packed by CommitPixelShaderLighting into cLightInfo[3] (two registers
// a light, the fourth light spread across the w components), the diffuse term
// with half-Lambert and the light warp, the specular and rim terms and the
// Fresnel approximations, and common_fxc.h's tangent-space helpers. The
// shaders place cAmbientCube and cLightInfo at different pixel registers, so
// the functions take the first register of each. Include after legacy_ps.glsl.
#ifndef LEGACY_PS_LIGHTING_GLSL
#define LEGACY_PS_LIGHTING_GLSL

#include "legacy_color.glsl"

#define cOverbright 2.0
#define cOOOverbright 0.5

// tex1D on a 2D texture: D3D9 samples at ( x, x ) (fxc replicates the scalar).
#define tex1D( set, s, x ) tex2D( ( set ), ( s ), vec2( x ) )

vec3 Vec3WorldToTangent( vec3 iWorldVector, vec3 iWorldNormal, vec3 iWorldTangent, vec3 iWorldBinormal )
{
	vec3 vTangentVector;
	vTangentVector.x = dot( iWorldVector.xyz, iWorldTangent.xyz );
	vTangentVector.y = dot( iWorldVector.xyz, iWorldBinormal.xyz );
	vTangentVector.z = dot( iWorldVector.xyz, iWorldNormal.xyz );
	return vTangentVector.xyz; // Return without normalizing
}

vec3 Vec3TangentToWorld( vec3 iTangentVector, vec3 iWorldNormal, vec3 iWorldTangent, vec3 iWorldBinormal )
{
	vec3 vWorldVector;
	vWorldVector.xyz = iTangentVector.x * iWorldTangent.xyz;
	vWorldVector.xyz += iTangentVector.y * iWorldBinormal.xyz;
	vWorldVector.xyz += iTangentVector.z * iWorldNormal.xyz;
	return vWorldVector.xyz; // Return without normalizing
}

vec3 Vec3TangentToWorldNormalized(
    vec3 iTangentVector, vec3 iWorldNormal, vec3 iWorldTangent, vec3 iWorldBinormal )
{
	return normalize( Vec3TangentToWorld( iTangentVector, iWorldNormal, iWorldTangent, iWorldBinormal ) );
}


// cLightInfo[n].color and .pos, cLightInfo being the registers from lightReg.
#define PS_LIGHT_COLOR( lightReg, n ) PS_C( ( lightReg ) + 2 * ( n ) )
#define PS_LIGHT_POS( lightReg, n ) PS_C( ( lightReg ) + 2 * ( n ) + 1 )

// cAmbientCube[6] in pixel registers cubeReg..cubeReg + 5.
vec3 PixelShaderAmbientLight( vec3 worldNormal, int cubeReg )
{
	const vec3 nSquared = worldNormal * worldNormal;
	const bvec3 nonNegative = greaterThanEqual( worldNormal, vec3( 0.0 ) );
	const vec3 isNegative = mix( nSquared, vec3( 0.0 ), nonNegative );
	const vec3 isPositive = mix( vec3( 0.0 ), nSquared, nonNegative );
	return isPositive.x * PS_C( cubeReg ).xyz + isNegative.x * PS_C( cubeReg + 1 ).xyz +
	       isPositive.y * PS_C( cubeReg + 2 ).xyz + isNegative.y * PS_C( cubeReg + 3 ).xyz +
	       isPositive.z * PS_C( cubeReg + 4 ).xyz + isNegative.z * PS_C( cubeReg + 5 ).xyz;
}

// The scalar diffuse term with half-Lambert, ambient occlusion and the light
// warp (a 1D lookup of the term, times two), warpSlot being the warp sampler's
// binding.
vec3 DiffuseTerm( bool bHalfLambert, vec3 worldNormal, vec3 lightDir, bool bDoAmbientOcclusion,
    float fAmbientOcclusion, bool bDoLightingWarp, int warpSlot, sampler2D lightWarpSampler )
{
	float fResult;

	const float NDotL = dot( worldNormal, lightDir ); // Unsaturated dot (-1 to 1 range)

	if ( bHalfLambert )
	{
		fResult = saturate( NDotL * 0.5 + 0.5 ); // Scale and bias to 0 to 1 range

		if ( !bDoLightingWarp )
			fResult *= fResult; // Square
	}
	else
	{
		fResult = saturate( NDotL ); // Saturate pure Lambertian term
	}

	if ( bDoAmbientOcclusion )
		fResult *= fAmbientOcclusion;

	vec3 fOut = vec3( fResult );
	if ( bDoLightingWarp )
		fOut = 2.0 * tex1D( warpSlot, lightWarpSampler, fResult ).rgb;

	return fOut;
}

vec3 PixelShaderDoGeneralDiffuseLight( float fAtten, vec3 worldPos, vec3 worldNormal,
    vec3 vPosition, vec3 vColor, bool bHalfLambert, bool bDoAmbientOcclusion,
    float fAmbientOcclusion, bool bDoLightingWarp, int warpSlot, sampler2D lightWarpSampler )
{
	// ps_2_b and later normalize in math (earlier models used the normalization cube).
	const vec3 lightDir = normalize( vPosition - worldPos );
	return vColor * fAtten *
	       DiffuseTerm( bHalfLambert, worldNormal, lightDir, bDoAmbientOcclusion, fAmbientOcclusion,
	           bDoLightingWarp, warpSlot, lightWarpSampler );
}

vec3 PixelShaderGetLightVector( vec3 worldPos, int lightReg, int nLightIndex )
{
	if ( nLightIndex == 3 )
	{
		// Unpack light 3 from w components...
		const vec3 vLight3Pos = vec3( PS_LIGHT_POS( lightReg, 1 ).w,
		    PS_LIGHT_COLOR( lightReg, 2 ).w, PS_LIGHT_POS( lightReg, 2 ).w );
		return normalize( vLight3Pos - worldPos );
	}
	return normalize( PS_LIGHT_POS( lightReg, nLightIndex ).xyz - worldPos );
}

vec3 PixelShaderGetLightColor( int lightReg, int nLightIndex )
{
	if ( nLightIndex == 3 )
	{
		// Unpack light 3 from w components...
		return vec3( PS_LIGHT_COLOR( lightReg, 0 ).w, PS_LIGHT_POS( lightReg, 0 ).w,
		    PS_LIGHT_COLOR( lightReg, 1 ).w );
	}
	return PS_LIGHT_COLOR( lightReg, nLightIndex ).rgb;
}

void SpecularAndRimTerms( vec3 vWorldNormal, vec3 vLightDir, float fSpecularExponent,
    vec3 vEyeDir, bool bDoAmbientOcclusion, float fAmbientOcclusion, bool bDoSpecularWarp,
    int warpSlot, sampler2D specularWarpSampler, float fFresnel, vec3 color, bool bDoRimLighting,
    float fRimExponent, out vec3 specularLighting, out vec3 rimLighting )
{
	rimLighting = vec3( 0.0 );

	// Reflect view through normal
	const vec3 vReflect = 2.0 * vWorldNormal * dot( vWorldNormal, vEyeDir ) - vEyeDir;
	const float LdotR = saturate( dot( vReflect, vLightDir ) ); // L.R
	specularLighting = vec3( HlslPow( LdotR, fSpecularExponent ) ); // Raise to specular exponent

	// Optionally warp as function of scalar specular and fresnel
	if ( bDoSpecularWarp )
		specularLighting *=
		    tex2D( warpSlot, specularWarpSampler, vec2( specularLighting.x, fFresnel ) ).rgb;

	specularLighting *= saturate( dot( vWorldNormal, vLightDir ) ); // Mask with N.L
	specularLighting *= color;                                        // Modulate with light color

	if ( bDoAmbientOcclusion ) // Optionally modulate with ambient occlusion
		specularLighting *= fAmbientOcclusion;

	if ( bDoRimLighting ) // Optionally do rim lighting
	{
		rimLighting = vec3( HlslPow( LdotR, fRimExponent ) );     // Raise to rim exponent
		rimLighting *= saturate( dot( vWorldNormal, vLightDir ) ); // Mask with N.L
		rimLighting *= color;                                       // Modulate with light color
	}
}

// Traditional fresnel term approximation
float Fresnel( vec3 vNormal, vec3 vEyeDir )
{
	const float fresnel = saturate( 1.0 - dot( vNormal, vEyeDir ) ); // 1-(N.V) for Fresnel term
	return fresnel * fresnel; // Square for a more subtle look
}

// Traditional fresnel term approximation which uses 4th power (square twice)
float Fresnel4( vec3 vNormal, vec3 vEyeDir )
{
	float fresnel = saturate( 1.0 - dot( vNormal, vEyeDir ) ); // 1-(N.V) for Fresnel term
	fresnel = fresnel * fresnel;                                // Square
	return fresnel * fresnel; // Square again for a more subtle look
}

// Custom Fresnel with low, mid and high parameters, encoded as
// ( ( mid - min ) * 2, mid, ( max - mid ) * 2 ).
float Fresnel( vec3 vNormal, vec3 vEyeDir, vec3 vRanges )
{
	float f = saturate( 1.0 - dot( vNormal, vEyeDir ) );
	f = f * f - 0.5;
	return vRanges.y + ( f >= 0.0 ? vRanges.z : vRanges.x ) * f;
}

void PixelShaderDoSpecularLight( vec3 vWorldPos, vec3 vWorldNormal, float fSpecularExponent,
    vec3 vEyeDir, float fAtten, vec3 vLightColor, vec3 vLightDir, bool bDoAmbientOcclusion,
    float fAmbientOcclusion, bool bDoSpecularWarp, int warpSlot, sampler2D specularWarpSampler,
    float fFresnel, bool bDoRimLighting, float fRimExponent, out vec3 specularLighting,
    out vec3 rimLighting )
{
	// Compute Specular and rim terms
	SpecularAndRimTerms( vWorldNormal, vLightDir, fSpecularExponent, vEyeDir, bDoAmbientOcclusion,
	    fAmbientOcclusion, bDoSpecularWarp, warpSlot, specularWarpSampler, fFresnel,
	    vLightColor * fAtten, bDoRimLighting, fRimExponent, specularLighting, rimLighting );
}

vec3 PixelShaderDoLightingLinear( vec3 worldPos, vec3 worldNormal, vec3 staticLightingColor,
    bool bStaticLight, bool bAmbientLight, vec4 lightAtten, int cubeReg, int nNumLights,
    int lightReg, bool bHalfLambert, bool bDoAmbientOcclusion, float fAmbientOcclusion,
    bool bDoLightingWarp, int warpSlot, sampler2D lightWarpSampler )
{
	vec3 linearColor = vec3( 0.0 );

	if ( bStaticLight )
	{
		// The static lighting comes in in gamma space and has also been
		// premultiplied by $cOOOverbright.
		linearColor += GammaToLinear( staticLightingColor * cOverbright );
	}

	if ( bAmbientLight )
	{
		vec3 ambient = PixelShaderAmbientLight( worldNormal, cubeReg );

		if ( bDoAmbientOcclusion )
			ambient *= fAmbientOcclusion * fAmbientOcclusion; // Note squaring...

		linearColor += ambient;
	}

	if ( nNumLights > 0 )
	{
		linearColor += PixelShaderDoGeneralDiffuseLight( lightAtten.x, worldPos, worldNormal,
		    PS_LIGHT_POS( lightReg, 0 ).xyz, PS_LIGHT_COLOR( lightReg, 0 ).xyz, bHalfLambert,
		    bDoAmbientOcclusion, fAmbientOcclusion, bDoLightingWarp, warpSlot, lightWarpSampler );
		if ( nNumLights > 1 )
		{
			linearColor += PixelShaderDoGeneralDiffuseLight( lightAtten.y, worldPos, worldNormal,
			    PS_LIGHT_POS( lightReg, 1 ).xyz, PS_LIGHT_COLOR( lightReg, 1 ).xyz, bHalfLambert,
			    bDoAmbientOcclusion, fAmbientOcclusion, bDoLightingWarp, warpSlot,
			    lightWarpSampler );
			if ( nNumLights > 2 )
			{
				linearColor += PixelShaderDoGeneralDiffuseLight( lightAtten.z, worldPos,
				    worldNormal, PS_LIGHT_POS( lightReg, 2 ).xyz, PS_LIGHT_COLOR( lightReg, 2 ).xyz,
				    bHalfLambert, bDoAmbientOcclusion, fAmbientOcclusion, bDoLightingWarp,
				    warpSlot, lightWarpSampler );
				if ( nNumLights > 3 )
				{
					// Unpack the 4th light's data from tight constant packing
					const vec3 vLight3Color = PixelShaderGetLightColor( lightReg, 3 );
					const vec3 vLight3Pos = vec3( PS_LIGHT_POS( lightReg, 1 ).w,
					    PS_LIGHT_COLOR( lightReg, 2 ).w, PS_LIGHT_POS( lightReg, 2 ).w );
					linearColor += PixelShaderDoGeneralDiffuseLight( lightAtten.w, worldPos,
					    worldNormal, vLight3Pos, vLight3Color, bHalfLambert, bDoAmbientOcclusion,
					    fAmbientOcclusion, bDoLightingWarp, warpSlot, lightWarpSampler );
				}
			}
		}
	}

	return linearColor;
}

void PixelShaderDoSpecularLighting( vec3 worldPos, vec3 worldNormal, float fSpecularExponent,
    vec3 vEyeDir, vec4 lightAtten, int nNumLights, int lightReg, bool bDoAmbientOcclusion,
    float fAmbientOcclusion, bool bDoSpecularWarp, int warpSlot, sampler2D specularWarpSampler,
    float fFresnel, bool bDoRimLighting, float fRimExponent, out vec3 specularLighting,
    out vec3 rimLighting )
{
	specularLighting = rimLighting = vec3( 0.0 );
	vec3 localSpecularTerm, localRimTerm;

	for ( int i = 0; i < 4; ++i )
	{
		if ( nNumLights > i )
		{
			PixelShaderDoSpecularLight( worldPos, worldNormal, fSpecularExponent, vEyeDir,
			    lightAtten[i], PixelShaderGetLightColor( lightReg, i ),
			    PixelShaderGetLightVector( worldPos, lightReg, i ), bDoAmbientOcclusion,
			    fAmbientOcclusion, bDoSpecularWarp, warpSlot, specularWarpSampler, fFresnel,
			    bDoRimLighting, fRimExponent, localSpecularTerm, localRimTerm );

			specularLighting += localSpecularTerm; // Accumulate specular and rim terms
			rimLighting += localRimTerm;
		}
	}
}

// PixelShaderDoLighting: the wrapper the shaders call.
vec3 PixelShaderDoLighting( vec3 worldPos, vec3 worldNormal, vec3 staticLightingColor,
    bool bStaticLight, bool bAmbientLight, vec4 lightAtten, int cubeReg, int nNumLights,
    int lightReg, bool bHalfLambert, bool bDoAmbientOcclusion, float fAmbientOcclusion,
    bool bDoLightingWarp, int warpSlot, sampler2D lightWarpSampler )
{
	return PixelShaderDoLightingLinear( worldPos, worldNormal, staticLightingColor, bStaticLight,
	    bAmbientLight, lightAtten, cubeReg, nNumLights, lightReg, bHalfLambert, bDoAmbientOcclusion,
	    fAmbientOcclusion, bDoLightingWarp, warpSlot, lightWarpSampler );
}

#endif // LEGACY_PS_LIGHTING_GLSL
