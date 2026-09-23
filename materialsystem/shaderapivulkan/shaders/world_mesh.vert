#version 450
// WMSH v1 corners are uploaded unchanged: position, octahedral normal and
// tangent, handedness, material UV and lightmap-atlas UV in a 40-byte record.
// This stage shares the textured/PBR material push block and fragment outputs.
layout( location = 0 ) in vec3 inPos;
layout( location = 1 ) in vec2 inNormalOct;
layout( location = 2 ) in vec2 inUv;
layout( location = 3 ) in vec2 inLightmapUv;
layout( location = 0 ) out vec2 fragUv;
layout( location = 1 ) out vec4 fragModulation;
layout( location = 2 ) out vec2 fragLightmapUv;
layout( location = 3 ) out vec4 fragVertexColor;
layout( location = 4 ) out vec3 fragReflection;
layout( location = 5 ) out vec2 fragScreenUv;
layout( location = 6 ) out vec4 fragEnvTint;
layout( push_constant ) uniform Constants
{
	mat4 mvp;
	vec4 modulation;
	vec4 texXform0;
	vec4 texXform1;
	vec4 alphaParams;
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
	vec3 normal = vec3( packed, 1.0 - abs( packed.x ) - abs( packed.y ) );
	if ( normal.z < 0.0 )
		normal.xy = ( 1.0 - abs( normal.yx ) ) * sign( normal.xy );
	return normalize( normal );
}

void main()
{
	gl_Position = consts.mvp * vec4( inPos, 1.0 );
	fragUv = vec2( dot( consts.texXform0, vec4( inUv, 0.0, 1.0 ) ),
	    dot( consts.texXform1, vec4( inUv, 0.0, 1.0 ) ) );
	fragModulation = consts.modulation;
	fragLightmapUv = inLightmapUv;
	fragVertexColor = vec4( 1.0 );
	fragReflection = DecodeOct( inNormalOct );
	fragScreenUv = ( gl_Position.xy / gl_Position.w * vec2( 1.0, -1.0 ) + 1.0 ) * 0.5;
	fragEnvTint = vec4( 1.0 );
#ifdef CLIP_PLANES
	gl_ClipDistance[0] = dot( consts.clipPlanes[0], gl_Position );
	gl_ClipDistance[1] = dot( consts.clipPlanes[1], gl_Position );
#endif
}
