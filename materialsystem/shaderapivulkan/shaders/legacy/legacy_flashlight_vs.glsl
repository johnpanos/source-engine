// The vertex stages' flashlight helpers (eyes_flashlight_vs20.fxc and
// teeth_flashlight_vs20.fxc define RemapValClamped_01 each). Include from a
// vertex stage.
#ifndef LEGACY_FLASHLIGHT_VS_GLSL
#define LEGACY_FLASHLIGHT_VS_GLSL

float RemapValClamped_01( float val, float A, float B )
{
	float cVal = ( val - A ) / ( B - A );
	cVal = saturate( cVal );
	return cVal;
}

#endif // LEGACY_FLASHLIGHT_VS_GLSL
