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

#endif // SURFACE_LIGHTING_GLSL
