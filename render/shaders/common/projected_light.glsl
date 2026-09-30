// render.projected-light.v1 (RFC 0011, public/render/projected_light.h): the
// render core's one GLSL copy of a projector's frustum and falloff, read by
// the surface program's projected-light term and render.pass.volumetric.
// ProjectedLight is projected_light::LightGpu (its std430/std140 layout).
// ProjectorProject mirrors projected_light::Project and ProjectorAttenuation
// projected_light::Attenuation; neither has the Lambert term (the surface
// program applies it, a medium has no normal).
#ifndef PROJECTED_LIGHT_GLSL
#define PROJECTED_LIGHT_GLSL

struct ProjectedLight
{
	vec4 origin; // w cookie layer
	vec4 forward;
	vec4 right;
	vec4 up;
	vec4 frustum; // tan half horizontal, tan half vertical, near, far
	vec4 color;   // rgb; w the shadow tile, or -1
	vec4 atten;   // constant, linear, quadratic; w the lens radius (soft shadows)
};

// projected_light::Project: the cookie coordinates and distance of `p`, or false.
bool ProjectorProject( ProjectedLight projector, vec3 p, out vec2 uv, out float distance )
{
	const vec3 d = p - projector.origin.xyz;
	const float z = dot( d, projector.forward.xyz );
	uv = vec2( 0.0 );
	distance = length( d );
	if ( !( z > projector.frustum.z ) || z > projector.frustum.w )
		return false;
	const float x = dot( d, projector.right.xyz ) / ( z * projector.frustum.x );
	const float y = dot( d, projector.up.xyz ) / ( z * projector.frustum.y );
	if ( x < -1.0 || x > 1.0 || y < -1.0 || y > 1.0 )
		return false;
	uv = vec2( 0.5 + 0.5 * x, 0.5 - 0.5 * y );
	return true;
}

// projected_light::Attenuation.
float ProjectorAttenuation( ProjectedLight projector, float distance )
{
	if ( !( distance > 0.0 ) )
		return 0.0;
	const vec3 a = projector.atten.xyz;
	const float atten = clamp( a.x + a.y / distance + a.z / ( distance * distance ), 0.0, 1.0 );
	const float far = projector.frustum.w;
	const float end = far > 0.0 ? clamp( ( distance - far ) / ( 0.6 * far - far ), 0.0, 1.0 ) : 0.0;
	return atten * end;
}

#endif // PROJECTED_LIGHT_GLSL
