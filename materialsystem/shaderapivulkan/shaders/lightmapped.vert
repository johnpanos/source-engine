#version 450
// LightmappedGeneric's vertex stage: a port of stdshaders/lightmappedgeneric_vs20.fxc
// (not SEAMLESS, not RELIEF_MAPPING, not LIGHTING_PREVIEW).
//
// The shader API evaluates on the CPU what the D3D9 shader derives from cModel:
// the world position, and the world normal and tangent S and T (mul by cModel,
// not normalized), as it does for every native model draw. This stage projects
// the position with cViewProj and passes the rest on. The texture coordinate
// transforms (cBaseTexCoordTransform, cDetailOrBumpTexCoordTransform,
// cEnvmapMaskTexCoordTransform) are affine, so lightmapped.frag applies them
// per pixel to the interpolated raw coordinate with the same result; its
// uniform block carries them.
//
// Inputs (the shared dynamic vertex): 0 world position, 1 vertex color, 2
// TEXCOORD0, 3 TEXCOORD1 (lightmap), 4 world normal, 5 world tangent S, 6
// vertex color alpha, 7..10 the draw's pixel fog (CVulkanContext::DrawFog),
// 11 world tangent T, 12 TEXCOORD2.x (the bumped lightmap page offset).
layout( location = 0 ) in vec3 inPos;
layout( location = 1 ) in vec3 inColor;
layout( location = 2 ) in vec2 inUv;
layout( location = 3 ) in vec2 inLightmapUv;
layout( location = 4 ) in vec3 inNormal;
layout( location = 5 ) in vec3 inTangentS;
layout( location = 6 ) in float inAlpha;
layout( location = 7 ) in vec4 inFogColor;
layout( location = 8 ) in vec4 inFogParams;
layout( location = 9 ) in vec4 inFogWorldZ;
layout( location = 10 ) in vec4 inFogMisc;
layout( location = 11 ) in vec3 inTangentT;
layout( location = 12 ) in float inLightmapOffset;
layout( location = 0 ) out vec2 vUv;
layout( location = 1 ) out vec4 vColor;
layout( location = 2 ) out vec2 vLightmapUv;
layout( location = 3 ) out float vLightmapOffset;
layout( location = 4 ) out vec3 vWorldPos;
layout( location = 5 ) out vec3 vTangentS;
layout( location = 6 ) out vec3 vTangentT;
layout( location = 7 ) out vec3 vNormal;
layout( location = 8 ) flat out vec4 fragFogColor;
layout( location = 9 ) flat out vec4 fragFogParams;
layout( location = 10 ) flat out vec4 fragFogMisc;
layout( location = 11 ) out vec2 fragFogDepth;
layout( push_constant ) uniform Constants
{
	mat4 viewProj; // cViewProj (laid out like demo_dyn.vert's mvp)
	vec4 unused0;
	vec4 unused1;
	vec4 eyePos;
	vec4 params; // see lightmapped.frag
	vec4 params2;
	vec4 clipPlanes[2];
}
consts;
out float gl_ClipDistance[2];
void main()
{
	vec4 projPos = consts.viewProj * vec4( inPos, 1.0 );
	gl_Position = projPos;
	vUv = inUv;
	vColor = vec4( inColor, inAlpha );
	vLightmapUv = inLightmapUv;
	vLightmapOffset = inLightmapOffset;
	vWorldPos = inPos;
	vTangentS = inTangentS;
	vTangentT = inTangentT;
	vNormal = inNormal;
	fragFogColor = inFogColor;
	fragFogParams = inFogParams;
	fragFogMisc = inFogMisc;
	fragFogDepth = vec2( projPos.z, dot( inFogWorldZ, vec4( inPos, 1.0 ) ) );
	gl_ClipDistance[0] = dot( consts.clipPlanes[0], projPos );
	gl_ClipDistance[1] = dot( consts.clipPlanes[1], projPos );
}
