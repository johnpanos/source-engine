#version 450
// BSP2 WMSH v1 vertex data: position, oct normals/tangents, handedness and UVs.
layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec2 inNormalOct;
layout( location = 2 ) in vec2 inUv;
layout( location = 3 ) in vec2 inLightmapUv;
layout( location = 4 ) in vec2 inTangentOct;
layout( location = 5 ) in int inHandedness;

layout( location = 0 ) out vec2 fragUv;
layout( location = 1 ) out vec2 fragLightmapUv;
layout( location = 2 ) out vec3 fragPosition;
layout( location = 3 ) out vec3 fragNormal;
layout( location = 4 ) out vec4 fragTangent;

layout( push_constant ) uniform Constants
{
	mat4 mvp;
	vec4 eyePosition;
	vec4 lightDirection;
	vec4 lightRadiance;
	vec4 material;
#ifdef CLIP_PLANES
	vec4 clipPlanes[2];
#endif
}
consts;
#ifdef CLIP_PLANES
out float gl_ClipDistance[2];
#endif

vec3 DecodeOct( vec2 packed )
{
	vec3 result = vec3( packed, 1.0 - abs( packed.x ) - abs( packed.y ) );
	if ( result.z < 0.0 )
		result.xy = ( 1.0 - abs( result.yx ) ) * sign( result.xy );
	return normalize( result );
}

void main()
{
	gl_Position = consts.mvp * vec4( inPosition, 1.0 );
	fragUv = inUv;
	fragLightmapUv = inLightmapUv;
	fragPosition = inPosition;
	fragNormal = DecodeOct( inNormalOct );
	fragTangent = vec4( DecodeOct( inTangentOct ), inHandedness < 0 ? -1.0 : 1.0 );
#ifdef CLIP_PLANES
	gl_ClipDistance[0] = dot( consts.clipPlanes[0], gl_Position );
	gl_ClipDistance[1] = dot( consts.clipPlanes[1], gl_Position );
#endif
}
