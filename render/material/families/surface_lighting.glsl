// render.material program `surface` (RFC 0016 K11 "Model assembly"): the
// draw group's model lighting (set 3, binding 2), as the legacy frontend
// supplies it until RFC 0016 K7's light set: Source's ambient cube and up to
// four sorted local lights, packed as CShaderAPIDx8::SetLight packs cLightInfo
// (PackSourceModelLighting in model_lighting.h mirrors this block). World
// draws bind a neutral block (no lights, a black cube); their terms read none
// of it. The model vertex stage and the pixel stage both read it.
#ifndef SURFACE_LIGHTING_GLSL
#define SURFACE_LIGHTING_GLSL

struct ModelLight
{
	vec4 color;       // w: 1 for a directional light
	vec4 direction;   // w: 1 for a spot light
	vec4 position;
	vec4 spot;        // exponent, cos(theta / 2), cos(phi / 2), 1 / their difference
	vec4 attenuation; // constant, linear, quadratic
};

layout( set = 3, binding = 2 ) uniform ModelLighting
{
	vec4 eye;     // xyz: the eye position; w: the number of lights
	vec4 cube[6]; // +x, -x, +y, -y, +z, -z
	ModelLight lights[4];
} lighting;

// PixelShaderAmbientLight (and the vertex term's AmbientLight): the faces
// weighted by the squared normal.
vec3 ModelAmbientCube( vec3 n )
{
	const vec3 squared = n * n;
	const bvec3 positive = greaterThanEqual( n, vec3( 0.0 ) );
	return squared.x * ( positive.x ? lighting.cube[0] : lighting.cube[1] ).rgb +
	       squared.y * ( positive.y ? lighting.cube[2] : lighting.cube[3] ).rgb +
	       squared.z * ( positive.z ? lighting.cube[4] : lighting.cube[5] ).rgb;
}

// GetVertexAttenForLight: distance falloff, the spot cone, and 1 for
// directional lights, per vertex as the legacy vertex shader evaluates it
// (the model and world vertex stages).
float ModelLightAttenuation( int i, vec3 position )
{
	const ModelLight light = lighting.lights[i];
	vec3 toLight = light.position.xyz - position;
	const float distanceSquared = dot( toLight, toLight );
	const float inverseDistance = inversesqrt( distanceSquared );
	toLight *= inverseDistance;
	const float distanceAtten = 1.0 / dot( light.attenuation.xyz,
	                                      vec3( 1.0, distanceSquared * inverseDistance, distanceSquared ) );
	const float cosTheta = -dot( light.direction.xyz, toLight );
	float spot = ( cosTheta - light.spot.z ) * light.spot.w;
	spot = clamp( pow( max( 0.0001, spot ), light.spot.x ), 0.0, 1.0 );
	const float atten = distanceAtten + ( distanceAtten * spot - distanceAtten ) * light.direction.w;
	return atten + ( 1.0 - atten ) * light.color.w;
}

#endif // SURFACE_LIGHTING_GLSL
