// render.material family `pbr` (RFC 0016 K4): the view group's model
// lighting, as the legacy frontend supplies it until RFC 0016 K7's light set:
// Source's ambient cube and up to four sorted local lights, packed as
// CShaderAPIDx8::SetLight packs cLightInfo (PackSourceModelLighting in
// pbr_family.h mirrors this block).
#ifndef PBR_LIGHTING_GLSL
#define PBR_LIGHTING_GLSL

struct ModelLight
{
	vec4 color;       // w: 1 for a directional light
	vec4 direction;   // w: 1 for a spot light
	vec4 position;
	vec4 spot;        // exponent, cos(theta / 2), cos(phi / 2), 1 / their difference
	vec4 attenuation; // constant, linear, quadratic
};

layout( set = 1, binding = 0 ) uniform ModelLighting
{
	vec4 eye;     // xyz: the eye position; w: the number of lights
	vec4 cube[6]; // +x, -x, +y, -y, +z, -z
	ModelLight lights[4];
} lighting;

#endif // PBR_LIGHTING_GLSL
