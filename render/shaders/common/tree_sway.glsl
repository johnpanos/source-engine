//========= Copyright Valve Corporation, All rights reserved. ============//
//
// VertexLitGeneric foliage deformation, adapted from Valve's public
// source-sdk-2013 src/materialsystem/stdshaders/tree_sway.h (b8cfb12c0e083a2ef5b2f9f9b50f3902fa034474).
// Object-space positions; world-space horizontal wind in Source units/s.
// geometry: height, start height fraction, radius, start radius fraction.
// motion: speed, strength, scrumble frequency, scrumble strength.
// curves: high-wind speed multiplier, scrumble exponent, trunk exponent, scrumble speed.
// windControls: speed lerp start/end, static wind, reserved.
// Mode 1 is upright foliage; mode 2 anchors hanging foliage above start height.
// Negative height is authored by hanging vines. The caller validates nonzero
// height, positive radius, start fractions below one, positive exponents and
// ordered wind lerp endpoints. At the model origin the scrumble direction is
// explicitly zero, avoiding the original normalize(0).

vec3 TreeSway( vec3 position, mat4 objectToWorld, vec2 wind, float time, int mode,
    vec4 geometry, vec4 motion, vec4 curves, vec4 windControls )
{
	if ( mode == 0 )
		return position;
#ifdef SEEDED_TREE_STATIC_IGNORED
	const vec2 effectiveWind = wind;
#else
	const vec2 effectiveWind = windControls.z != 0.0 ? vec2( 0.5 ) : wind;
#endif
	const float intensity = length( effectiveWind );
#ifdef SEEDED_TREE_WIND_UNROTATED
	const vec3 objectWind = vec3( effectiveWind, 0.0 );
#else
	const vec3 objectWind = transpose( mat3( objectToWorld ) ) * vec3( effectiveWind, 0.0 );
#endif
	const float heightScale = clamp( ( position.z - geometry.x * geometry.y ) /
	    ( ( 1.0 - geometry.y ) * geometry.x ), 0.0, 1.0 );
	const float radiusScale = clamp( length( position.xy - geometry.z * geometry.w ) /
	    ( ( 1.0 - geometry.w ) * geometry.z ), 0.0, 1.0 );
	float threshold = step( 0.0, position.z - geometry.x * geometry.y );
#ifndef SEEDED_TREE_HANGING_IGNORED
	if ( mode == 2 )
		threshold = step( position.z - geometry.x * geometry.y, 0.0 );
#endif
	const float orthogonal = 1.0 - clamp( abs( dot( objectWind, vec3( position.xy, 0.0 ) ) ) /
	    ( max( length( objectWind ), 0.0001 ) * max( length( position.xy ), 0.0001 ) ), 0.0, 1.0 );
	const float trunk = motion.y * pow( heightScale, curves.z );
	float branches = motion.y * orthogonal * radiusScale * threshold;
#ifndef SEEDED_TREE_HANGING_IGNORED
	if ( mode == 2 )
		branches = 0.0;
#endif
#ifdef SEEDED_TREE_ROOT_IGNORED
	const float phase = 0.0;
#else
	const float phase = dot( objectToWorld[3].xyz, vec3( 1.0 ) ) * 19.0;
#endif
	const float slowTime = ( time + phase ) * motion.x;
	const vec4 sines = sin( vec4( 1.0, 2.31, curves.x, 2.14 * curves.x ) * slowTime );
	const vec2 sway = mix( sines.xy, sines.zw,
	    smoothstep( windControls.x, windControls.y, intensity ) );
	vec3 offset = objectWind * ( trunk * ( sway.x + 0.1 ) + branches * ( sway.y + 0.4 ) );
	const vec3 direction = position / max( length( position ), 0.0001 );
	vec3 scrumble = vec3( pow( radiusScale, curves.y ) * motion.w * threshold );
	if ( mode == 2 )
		scrumble *= vec3( 0.5, 0.5, 1.0 );
	offset += intensity * scrumble * sin( curves.w * time + direction.yzx * motion.z + phase );
	return position + offset;
}
