#version 450
// VertexLitGeneric's $phong vertex stage: a port of stdshaders/skin_vs20.fxc.
//
// The shader API evaluates on the CPU what the D3D9 shader derives per vertex
// from its registers, as it does for every native model draw: the skinned or
// MODEL-transformed world position, world normal and tangent S (not yet
// normalized), and GetVertexAttenForLight of the four lights in SortLights
// order. This stage does the rest of skin_vs20: the tangent frame (T from
// cross( N, S ) times the TANGENT flip, all three normalized), the eye vector
// and the base texture coordinate transform.
//
// Inputs (the shared dynamic vertex): 0 world position, 1 light attenuation of
// lights 0..2, 2 texture coordinate, 4 world normal, 5 world tangent S with the
// flip in w, 6 light attenuation of light 3.
layout( location = 0 ) in vec3 inPos;
layout( location = 1 ) in vec3 inLightAtten;
layout( location = 2 ) in vec2 inUv;
layout( location = 4 ) in vec3 inNormal;
layout( location = 5 ) in vec4 inTangent;
layout( location = 6 ) in float inAtten3;
layout( location = 0 ) out vec2 vBaseUv;
layout( location = 1 ) out vec4 vLightAtten;
layout( location = 2 ) out vec3 vWorldVertToEye;
layout( location = 3 ) out vec3 vTangentS;
layout( location = 4 ) out vec3 vTangentT;
layout( location = 5 ) out vec3 vNormal;
layout( location = 6 ) out vec3 vWorldPos;
layout( push_constant ) uniform Constants
{
	mat4 viewProj;   // cViewProj (laid out like demo_dyn.vert's mvp)
	vec4 texXform0;  // cBaseTexCoordTransform, SHADER_SPECIFIC_CONST_0
	vec4 texXform1;
	vec4 eyePos;     // cEyePos
	vec4 params;     // see skin.frag
	vec4 params2;
	vec4 clipPlanes[2];
}
consts;
out float gl_ClipDistance[2];
void main()
{
	vec3 worldNormal = inNormal;
	vec3 worldTangentS = inTangent.xyz;
	vec3 worldTangentT = cross( worldNormal, worldTangentS ) * inTangent.w;
	worldNormal = normalize( worldNormal );
	worldTangentS = normalize( worldTangentS );
	worldTangentT = normalize( worldTangentT );

	vec4 projPos = consts.viewProj * vec4( inPos, 1.0 );
	gl_Position = projPos;
	vWorldPos = inPos;
	vWorldVertToEye = consts.eyePos.xyz - inPos;
	vLightAtten = vec4( inLightAtten, inAtten3 );

	// dot( v.vTexCoord0, cBaseTexCoordTransform[i] ) with the texture
	// coordinate expanded to ( u, v, 0, 1 ).
	const vec4 uv = vec4( inUv, 0.0, 1.0 );
	vBaseUv = vec2( dot( uv, consts.texXform0 ), dot( uv, consts.texXform1 ) );

	vTangentS = worldTangentS;
	vTangentT = worldTangentT;
	vNormal = worldNormal;

	gl_ClipDistance[0] = dot( consts.clipPlanes[0], projPos );
	gl_ClipDistance[1] = dot( consts.clipPlanes[1], projPos );
}
