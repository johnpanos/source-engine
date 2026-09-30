#version 450
// render.pass.shadows receiver pass (RFC 0016 K7, render.shadows.v1): the
// shadowed light at each receiver pixel, through shadow_sample.glsl.
//
// A spot light (lightPositionKind.w 0) lights the points inside its cone and
// range, by the visibility from tile 0. The sun (w 1) lights every point, by
// the visibility from the first cascade whose split holds the point's view
// distance (cascadeSplits holds each cascade's far distance); beyond the last
// split the point is lit. Output: red = ambient + ( 1 - ambient ) * light,
// green 1 (a receiver), blue = the cascade used / 4 (4 beyond the last).

#include "../../shaders/common/shadow_sample.glsl"

layout( set = 0, binding = 0 ) uniform texture2D shadowAtlas;
layout( set = 0, binding = 1 ) uniform sampler shadowSampler;

layout( std140, set = 1, binding = 0, row_major ) uniform Receiver
{
	mat4 viewProjection;
	mat4 view;
	vec4 lightPositionKind; // xyz spot position, w 0 spot, 1 sun
	vec4 lightAxisCos;      // spot axis, outer cosine
	vec4 lightRange;        // spot range, cascade count, ambient, 0
	vec4 cascadeSplits;
	ShadowTile tiles[4];
}
receiver;

layout( location = 0 ) in vec3 worldPosition;
layout( location = 1 ) in float viewDistance;

layout( location = 0 ) out vec4 outColor;

void main()
{
	const float ambient = receiver.lightRange.z;
	float light = 0.0;
	uint cascade = 0u;
	if ( receiver.lightPositionKind.w == 0.0 )
	{
		const vec3 toPoint = worldPosition - receiver.lightPositionKind.xyz;
		const float distance = length( toPoint );
		if ( distance <= receiver.lightRange.x &&
		     dot( toPoint, receiver.lightAxisCos.xyz ) >= receiver.lightAxisCos.w * distance )
			light = ShadowVisibility( shadowAtlas, shadowSampler, receiver.tiles[0], worldPosition );
	}
	else
	{
		const uint count = uint( receiver.lightRange.y );
		while ( cascade < count && viewDistance > receiver.cascadeSplits[cascade] )
			++cascade;
		light = cascade < count ? ShadowVisibility( shadowAtlas, shadowSampler,
		                              receiver.tiles[cascade], worldPosition )
		                        : 1.0;
	}
	outColor = vec4( ambient + ( 1.0 - ambient ) * light, 1.0, float( cascade ) * 0.25, 1.0 );
}
