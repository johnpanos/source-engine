// render.material family `lightmapped` (RFC 0016 K4): the base texture times
// the lightmap times the material tint, with the vertex color and alpha when
// the material asks for them and alpha test; the non-bumped path of the
// lightmappedgeneric port, in linear light. Blending is pipeline state.
#version 450

layout( set = 2, binding = 0 ) uniform Material
{
	vec4 tint;  // rgb: $color times the lightmap scale, a: $alpha
	vec4 flags; // x: $vertexcolor, y: $alphatest, z: $alphatestreference
} material;
layout( set = 2, binding = 1 ) uniform texture2D baseTexture;
layout( set = 2, binding = 2 ) uniform sampler baseSampler;
// The lightmap page is the draw's: surfaces of one material share pages
// with others.
layout( set = 3, binding = 0 ) uniform texture2D lightmap;
layout( set = 3, binding = 1 ) uniform sampler lightmapSampler;

layout( location = 0 ) in vec2 baseUv;
layout( location = 1 ) in vec2 lightmapUv;
layout( location = 2 ) in vec4 color;
layout( location = 0 ) out vec4 outColor;

void main()
{
	const vec4 base = texture( sampler2D( baseTexture, baseSampler ), baseUv );
	const vec3 light = texture( sampler2D( lightmap, lightmapSampler ), lightmapUv ).rgb;
	vec3 albedo = base.rgb;
	// The port's vertex fast path (no texture transform, no detail): the
	// vertex color replaces the modulation alpha, which otherwise applies
	// $alpha a second time.
	float alpha = base.a * material.tint.a;
	if ( material.flags.x != 0.0 )
	{
		albedo *= color.rgb;
		alpha *= color.a;
	}
	else
	{
		alpha *= material.tint.a;
	}
	if ( material.flags.y != 0.0 && alpha < material.flags.z )
		discard;
	outColor = vec4( albedo * light * material.tint.rgb, alpha );
}
