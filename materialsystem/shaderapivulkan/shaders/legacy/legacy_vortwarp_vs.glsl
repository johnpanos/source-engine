// vortwarp_vs20_helper.h's world-space warp (the ep1 intro's "INTRO" combo of
// the eyes, eye refract, teeth and vortwarp vertex shaders). Include from a
// vertex stage.
#ifndef LEGACY_VORTWARP_VS_GLSL
#define LEGACY_VORTWARP_VS_GLSL

float Sine( float min, float max, float t )
{
	return ( sin( t ) * 0.5 + 0.5 ) * ( max - min ) + min;
}

vec3 QuadraticBezier( vec3 A, vec3 B, vec3 C, float t )
{
	return mix( mix( A, B, t ), mix( B, C, t ), t );
}

vec3 CubicBezier( vec3 A, vec3 B, vec3 C, vec3 D, float t )
{
	return QuadraticBezier( mix( A, B, t ), mix( B, C, t ), mix( C, D, t ), t );
}

void WorldSpaceVertexProcess( float time, vec3 modelOrigin, inout vec3 worldPos,
    inout vec3 worldNormal, inout vec3 worldTangentS, inout vec3 worldTangentT )
{
	float myTime = time;
	myTime = saturate( 1.0 - myTime );
	myTime *= myTime;
	myTime *= myTime;
	myTime *= myTime;

	// end
	vec3 A = vec3( 0.0, 0.0, 1.0 );
	vec3 B = vec3( 1.0, 1.0, 1.0 );
	vec3 C = vec3( 0.0, 0.0, 1.0 );
	vec3 D = vec3( 0.0, 0.0, 1.0 );
	// start

	float t = worldPos.z * ( 1.0 / ( 72.0 ) ); // about 72 inches tall
	t = saturate( t );
	vec3 worldPosDelta = ( worldPos - modelOrigin ) * CubicBezier( A, B, C, D, t );
	worldPosDelta.z += Sine( 0.0, 10.0, worldPos.z );
	worldPos = mix( worldPos, worldPosDelta + modelOrigin, myTime );
}

#endif // LEGACY_VORTWARP_VS_GLSL
