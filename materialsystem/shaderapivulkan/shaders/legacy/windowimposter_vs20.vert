#version 450
// A port of stdshaders/windowimposter_vs20.fxc. Combos
// (fxctmp9/windowimposter_vs20.inc): dynamic DOWATERFOG (1), which only
// changes the fixed-function fog output. The eye-to-vertex vector for the
// cube map and the modulation color.
#include "legacy_vs.glsl"

layout( location = 0 ) out vec3 eyeToVertVector;
layout( location = 7 ) out vec4 worldPos_projPosZ;
layout( location = 8 ) out vec4 vertexColor;

void main()
{
	const vec3 worldPos = inWorldPos;
	const vec4 projPos = LegacyProject( worldPos );
	gl_Position = projPos;
	worldPos_projPosZ = vec4( worldPos, projPos.z );
	eyeToVertVector = worldPos - cEyePos;
	vertexColor = cModulationColor;
}
