// common_flashlight_fxc.h's DoFlashlight for the legacy shader ports, without
// its shadow term: this backend has no shadow depth textures, so no pass it
// draws selects flashlight shadows. Include from a pixel stage after
// legacy_ps.glsl.
#ifndef LEGACY_FLASHLIGHT_GLSL
#define LEGACY_FLASHLIGHT_GLSL

#define flFlashlightNoLambertValue ( cFlashlightColor.w ) // This is either 0.0 or 2.0

// common_ps_fxc.h's RemapValClamped.
float RemapValClamped( float val, float A, float B, float C, float D )
{
	float cVal = ( val - A ) / ( B - A );
	cVal = saturate( cVal );
	return C + ( D - C ) * cVal;
}

vec3 DoFlashlight( vec3 flashlightPos, vec3 worldPos, vec4 flashlightSpacePosition, vec3 worldNormal,
    vec3 attenuationFactors, float farZ, int flashlightSet, sampler2D FlashlightSampler, bool bHasNormal )
{
	vec3 vProjCoords = flashlightSpacePosition.xyz / flashlightSpacePosition.w;
	vec3 flashlightColor = tex2D( flashlightSet, FlashlightSampler, vProjCoords.xy ).rgb;
	flashlightColor *= cFlashlightColor.xyz; // Flashlight color

	vec3 delta = flashlightPos - worldPos;
	vec3 L = normalize( delta );
	float distSquared = dot( delta, delta );
	float dist = sqrt( distSquared );

	float endFalloffFactor = RemapValClamped( dist, farZ, 0.6 * farZ, 0.0, 1.0 );

	// Attenuation for light and to fade out shadow over distance
	float fAtten = saturate( dot( attenuationFactors, vec3( 1.0, 1.0 / dist, 1.0 / distSquared ) ) );

	vec3 diffuseLighting = vec3( fAtten );

	float flLDotWorldNormal = bHasNormal ? dot( L.xyz, worldNormal.xyz ) : 1.0;
	diffuseLighting *= saturate( flLDotWorldNormal + flFlashlightNoLambertValue ); // Lambertian term

	diffuseLighting *= flashlightColor;
	diffuseLighting *= endFalloffFactor;
	return diffuseLighting;
}

#endif // LEGACY_FLASHLIGHT_GLSL
