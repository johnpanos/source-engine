// render.material family `pbr` (RFC 0016 K4): PBRMetalRough on meshes. The
// draw constants are the FamilyDrawConstants prefix: the draw's object-to-clip
// and object-to-world matrices (row-major with column vectors). Normals and tangents are
// transformed by object-to-world's upper 3x3, as the model port does, and
// normalized per pixel. Each light's attenuation is Source's per-vertex term
// (common_vs_fxc.h GetVertexAttenForLight), interpolated as the port does.
#version 450

layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec3 normal;
layout( location = 2 ) in vec4 tangent; // w: the bitangent's sign
layout( location = 3 ) in vec2 uv0;

layout( push_constant ) uniform Draw
{
	layout( row_major ) mat4 toClip;
	layout( row_major ) mat4 world;
} draw;

#include "pbr_lighting.glsl"

layout( location = 0 ) out vec2 uv;
layout( location = 1 ) out vec4 lightAtten;
layout( location = 2 ) out vec3 worldPosition;
layout( location = 3 ) out vec3 worldNormal;
layout( location = 4 ) out vec3 tangentS;
layout( location = 5 ) out vec3 tangentT;

// GetVertexAttenForLight: distance falloff, the spot cone, and 1 for
// directional lights.
float VertexAttenuation( int i, vec3 position )
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

void main()
{
	const vec4 world = draw.world * vec4( position, 1.0 );
	gl_Position = draw.toClip * vec4( position, 1.0 );
	uv = uv0;
	worldPosition = world.xyz;
	const mat3 basis = mat3( draw.world );
	worldNormal = basis * normal;
	tangentS = basis * tangent.xyz;
	tangentT = cross( worldNormal, tangentS ) * tangent.w;
	const int count = int( lighting.eye.w );
	vec4 atten = vec4( 0.0 );
	for ( int i = 0; i < 4; ++i )
	{
		if ( i < count )
			atten[i] = VertexAttenuation( i, world.xyz );
	}
	lightAtten = atten;
}
