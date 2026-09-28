// render.material family `vertexlit` (RFC 0016 K4): VertexLitGeneric's
// claimed subset under Source's vertex lighting. The draw constants carry
// object-to-clip and object-to-world (row-major with column vectors, the
// FamilyDrawConstants prefix, as the opaque pass pushes them); the draw group
// carries the draw's lighting in world space (VertexLitLighting, PackSourceModelLighting's packing). The
// lighting is the port's common_vs_fxc.h DoLighting with static control flow:
// the lights in order, then the ambient cube, in the port's expressions.
#version 450

layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec3 normal;
layout( location = 2 ) in vec2 uv0;

layout( push_constant ) uniform Draw
{
	layout( row_major ) mat4 toClip;
	layout( row_major ) mat4 world;
} draw;

layout( set = 2, binding = 0 ) uniform Material
{
	vec4 color; // rgb: $color, linear; a: $alpha
	vec4 flags; // x: $halflambert, y: $alphatest, z: $alphatestreference
} material;

struct ModelLight
{
	vec4 color;       // w: 1 for a directional light
	vec4 direction;   // w: 1 for a spot light
	vec4 position;
	vec4 spot;        // exponent, cos(theta / 2), cos(phi / 2), 1 / their difference
	vec4 attenuation; // constant, linear, quadratic
};

layout( set = 3, binding = 0 ) uniform Lighting
{
	vec4 eye;     // w: the number of lights
	vec4 cube[6]; // +x, -x, +y, -y, +z, -z
	ModelLight lights[4];
} lighting;

layout( location = 0 ) out vec2 uv;
layout( location = 1 ) out vec3 diffuseLighting;

vec3 AmbientLight( vec3 worldNormal )
{
	const vec3 nSquared = worldNormal * worldNormal;
	const ivec3 isNegative = ivec3( lessThan( worldNormal, vec3( 0.0 ) ) );
	return nSquared.x * lighting.cube[isNegative.x].xyz +
	       nSquared.y * lighting.cube[2 + isNegative.y].xyz +
	       nSquared.z * lighting.cube[4 + isNegative.z].xyz;
}

// VertexAttenInternal: distance falloff, the spot cone, and 1 for directional
// lights.
float VertexAtten( vec3 worldPos, int i )
{
	vec3 lightDir = lighting.lights[i].position.xyz - worldPos;
	const float lightDistSquared = dot( lightDir, lightDir );
	const float ooLightDist = inversesqrt( lightDistSquared );
	lightDir *= ooLightDist;
	const vec3 vDist = vec3( 1.0, lightDistSquared * ooLightDist, lightDistSquared );
	const float flDistanceAtten = 1.0 / dot( lighting.lights[i].attenuation.xyz, vDist );
	const float flCosTheta = dot( lighting.lights[i].direction.xyz, -lightDir );
	float flSpotAtten = ( flCosTheta - lighting.lights[i].spot.z ) * lighting.lights[i].spot.w;
	flSpotAtten = max( 0.0001, flSpotAtten );
	flSpotAtten = pow( flSpotAtten, lighting.lights[i].spot.x );
	flSpotAtten = clamp( flSpotAtten, 0.0, 1.0 );
	const float flAtten =
	    mix( flDistanceAtten, flDistanceAtten * flSpotAtten, lighting.lights[i].direction.w );
	return mix( flAtten, 1.0, lighting.lights[i].color.w );
}

// CosineTermInternal: Lambert, or half-Lambert squared.
float CosineTerm( vec3 worldPos, vec3 worldNormal, int i, bool halfLambert )
{
	vec3 lightDir = normalize( lighting.lights[i].position.xyz - worldPos );
	lightDir = mix( lightDir, -lighting.lights[i].direction.xyz, lighting.lights[i].color.w );
	float NDotL = dot( worldNormal, lightDir );
	if ( !halfLambert )
	{
		NDotL = max( 0.0, NDotL );
	}
	else
	{
		NDotL = NDotL * 0.5 + 0.5;
		NDotL = NDotL * NDotL;
	}
	return NDotL;
}

void main()
{
	const vec4 world = draw.world * vec4( position, 1.0 );
	gl_Position = draw.toClip * vec4( position, 1.0 );
	uv = uv0;
	const vec3 worldNormal = normalize( mat3( draw.world ) * normal );
	const bool halfLambert = material.flags.x != 0.0;
	const int count = int( lighting.eye.w );
	vec3 linearColor = vec3( 0.0 );
	for ( int i = 0; i < 4; ++i )
	{
		if ( i < count )
			linearColor += lighting.lights[i].color.xyz *
			               CosineTerm( world.xyz, worldNormal, i, halfLambert ) *
			               VertexAtten( world.xyz, i );
	}
	linearColor += AmbientLight( worldNormal );
	diffuseLighting = linearColor;
}
