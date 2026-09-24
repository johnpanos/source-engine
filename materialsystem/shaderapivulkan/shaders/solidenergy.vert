#version 450
// SolidEnergy's vertex stage: a port of stdshaders/solidenergy_vs20.fxc.
//
// The shader API evaluates on the CPU what the D3D9 shader derives per vertex
// from cModel and the tangent streams, as it does for the skin shader: the
// world position, world normal and world tangent S with the sign that makes
// tangent T = cross( N, S ) * sign (a brush's tangent S/T streams, or a model's
// normal and TANGENT). This stage passes the world frame on; solidenergy.frag
// does the vortex and view-dependent terms per pixel, where D3D9 interpolates
// them (the vortex offsets are linear in the position, so they agree).
//
// Inputs (the shared dynamic vertex): 0 world position, 1 vertex color rgb,
// 2 texture coordinate, 4 world normal, 5 world tangent S with the sign in w,
// 6 vertex alpha.
layout( location = 0 ) in vec3 inPos;
layout( location = 1 ) in vec3 inColor;
layout( location = 2 ) in vec2 inUv;
layout( location = 4 ) in vec3 inNormal;
layout( location = 5 ) in vec4 inTangent;
layout( location = 6 ) in float inAlpha;
layout( location = 0 ) out vec2 vBaseUv;      // vUV0_UV1.xy
layout( location = 1 ) out vec2 vTexCoord;    // TEXCOORD0, for the detail and model flow UVs
layout( location = 2 ) out vec4 vProjPos;     // vIteratedProjPos
layout( location = 3 ) out vec3 vWorldPos;
layout( location = 4 ) out vec3 vWorldNormal;
layout( location = 5 ) out vec3 vWorldTangentS;
layout( location = 6 ) out vec3 vWorldTangentT;
layout( location = 7 ) out vec4 vColor;
layout( push_constant ) uniform Constants
{
	mat4 viewProj;  // cViewProj (laid out like demo_dyn.vert's mvp)
	vec4 texXform0; // cBaseTextureTransform, SHADER_SPECIFIC_CONST_0
	vec4 texXform1;
	vec4 eyePos;    // g_vEyePosition, SHADER_SPECIFIC_CONST_4
	vec4 params;    // see solidenergy.frag
	vec4 params2;
	vec4 clipPlanes[2];
}
consts;
out float gl_ClipDistance[2];
void main()
{
	vec4 projPos = consts.viewProj * vec4( inPos, 1.0 );
	gl_Position = projPos;
	vProjPos = projPos;
	vWorldPos = inPos;
	vWorldNormal = inNormal;
	vWorldTangentS = inTangent.xyz;
	vWorldTangentT = cross( inNormal, inTangent.xyz ) * inTangent.w;
	vColor = vec4( inColor, inAlpha );
	vTexCoord = inUv;

	// dot( i.vTexCoord.xy, cBaseTextureTransform[i].xy ) + cBaseTextureTransform[i].w
	vBaseUv = vec2( dot( inUv, consts.texXform0.xy ) + consts.texXform0.w,
	    dot( inUv, consts.texXform1.xy ) + consts.texXform1.w );

	gl_ClipDistance[0] = dot( consts.clipPlanes[0], projPos );
	gl_ClipDistance[1] = dot( consts.clipPlanes[1], projPos );
}
