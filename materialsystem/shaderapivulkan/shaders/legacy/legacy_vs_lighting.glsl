// common_vs_fxc.h's vertex lighting for the legacy shader ports: the ambient
// cube (cAmbientCubeX/Y/Z, c21..c26), the four local lights (cLightInfo,
// c27..c46: color, dir, pos, spotParams, atten), the static-control-flow loop
// over i0 and the unrolled NUM_LIGHTS variant, and GetVertexAttenForLight (the
// enabled lights b0..b3). Include after legacy_vs.glsl.
#ifndef LEGACY_VS_LIGHTING_GLSL
#define LEGACY_VS_LIGHTING_GLSL

#define cOverbright 2.0
#define LIGHT_INFO( n, k ) VS_C( 27 + 5 * ( n ) + ( k ) )
#define LIGHT_COLOR( n ) LIGHT_INFO( n, 0 )
#define LIGHT_DIR( n ) LIGHT_INFO( n, 1 )
#define LIGHT_POS( n ) LIGHT_INFO( n, 2 )
#define LIGHT_SPOT_PARAMS( n ) LIGHT_INFO( n, 3 )
#define LIGHT_ATTEN( n ) LIGHT_INFO( n, 4 )
#define g_nLightCount ( lc.vsLoop.x )

vec3 AmbientLight( vec3 worldNormal )
{
	const vec3 nSquared = worldNormal * worldNormal;
	const ivec3 isNegative = ivec3( lessThan( worldNormal, vec3( 0.0 ) ) );
	return nSquared.x * VS_C( 21 + isNegative.x ).xyz + nSquared.y * VS_C( 23 + isNegative.y ).xyz +
	       nSquared.z * VS_C( 25 + isNegative.z ).xyz;
}

float VertexAttenInternal( vec3 worldPos, int lightNum )
{
	// Get light direction
	vec3 lightDir = LIGHT_POS( lightNum ).xyz - worldPos;

	// Get light distance squared.
	const float lightDistSquared = dot( lightDir, lightDir );

	// Get 1/lightDistance
	const float ooLightDist = inversesqrt( lightDistSquared );

	// Normalize light direction
	lightDir *= ooLightDist;

	// dst( lightDistSquared, ooLightDist ).xyz: ( 1, distance, distance squared )
	const vec3 vDist = vec3( 1.0, lightDistSquared * ooLightDist, lightDistSquared );

	const float flDistanceAtten = 1.0 / dot( LIGHT_ATTEN( lightNum ).xyz, vDist );

	// Spot attenuation
	const float flCosTheta = dot( LIGHT_DIR( lightNum ).xyz, -lightDir );
	float flSpotAtten =
	    ( flCosTheta - LIGHT_SPOT_PARAMS( lightNum ).z ) * LIGHT_SPOT_PARAMS( lightNum ).w;
	flSpotAtten = max( 0.0001, flSpotAtten );
	flSpotAtten = pow( flSpotAtten, LIGHT_SPOT_PARAMS( lightNum ).x );
	flSpotAtten = saturate( flSpotAtten );

	// Select between point and spot
	const float flAtten =
	    mix( flDistanceAtten, flDistanceAtten * flSpotAtten, LIGHT_DIR( lightNum ).w );

	// Select between above and directional (no attenuation)
	return mix( flAtten, 1.0, LIGHT_COLOR( lightNum ).w );
}

float CosineTermInternal( vec3 worldPos, vec3 worldNormal, int lightNum, bool bHalfLambert )
{
	// Calculate light direction assuming this is a point or spot
	vec3 lightDir = normalize( LIGHT_POS( lightNum ).xyz - worldPos );

	// Select the above direction or the one in the structure, based upon light type
	lightDir = mix( lightDir, -LIGHT_DIR( lightNum ).xyz, LIGHT_COLOR( lightNum ).w );

	// compute N dot L
	float NDotL = dot( worldNormal, lightDir );
	if ( !bHalfLambert )
	{
		NDotL = max( 0.0, NDotL );
	}
	else // Half-Lambert
	{
		NDotL = NDotL * 0.5 + 0.5;
		NDotL = NDotL * NDotL;
	}
	return NDotL;
}

// With static control flow a light counts only when g_bLightEnabled (b0..b3)
// says so.
float GetVertexAttenForLight( vec3 worldPos, int lightNum, bool bUseStaticControlFlow )
{
	float result = 0.0;
	if ( bUseStaticControlFlow )
	{
		if ( VS_BOOL( lightNum ) )
			result = VertexAttenInternal( worldPos, lightNum );
	}
	else
	{
		result = VertexAttenInternal( worldPos, lightNum );
	}
	return result;
}

vec3 DoLightInternal( vec3 worldPos, vec3 worldNormal, int lightNum, bool bHalfLambert )
{
	return LIGHT_COLOR( lightNum ).xyz *
	       CosineTermInternal( worldPos, worldNormal, lightNum, bHalfLambert ) *
	       VertexAttenInternal( worldPos, lightNum );
}

vec3 DoLighting( vec3 worldPos, vec3 worldNormal, vec3 staticLightingColor, bool bStaticLight,
    bool bDynamicLight, bool bHalfLambert )
{
	vec3 linearColor = vec3( 0.0 );
	if ( bStaticLight ) // Static light
		linearColor += GammaToLinear( staticLightingColor * cOverbright );
	if ( bDynamicLight ) // Dynamic light
	{
		for ( int i = 0; i < g_nLightCount; i++ )
			linearColor += DoLightInternal( worldPos, worldNormal, i, bHalfLambert );
	}
	if ( bDynamicLight )
		linearColor += AmbientLight( worldNormal ); // ambient light is already remapped
	return linearColor;
}

vec3 DoLightingUnrolled( vec3 worldPos, vec3 worldNormal, vec3 staticLightingColor,
    bool bStaticLight, bool bDynamicLight, bool bHalfLambert, int nNumLights )
{
	vec3 linearColor = vec3( 0.0 );
	if ( bStaticLight ) // Static light
		linearColor += GammaToLinear( staticLightingColor * cOverbright );
	if ( bDynamicLight )
	{
		for ( int i = 0; i < 4; i++ )
		{
			if ( nNumLights > i )
				linearColor += DoLightInternal( worldPos, worldNormal, i, bHalfLambert );
		}
	}
	if ( bDynamicLight )
		linearColor += AmbientLight( worldNormal ); // ambient light is already remapped
	return linearColor;
}

#endif // LEGACY_VS_LIGHTING_GLSL
