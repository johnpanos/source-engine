// render.pass.ssr (render.ssr.v1): the pass's bindings and view constants,
// shared by its kernels. The one definition of the term is
// public/render/pass/ssr/ssr.h; SsrViewConstants in ssr.cpp mirrors this
// block (std140).

const int kSsrMaxLevels = 16;

layout( set = 3, binding = 0, std140 ) uniform SsrView
{
	layout( row_major ) mat4 toClip; // world to clip
	layout( row_major ) mat4 fromClip;
	vec4 eye;    // xyz: the world position
	vec4 extent; // x width, y height, z depth levels, w colour levels
	vec4 params; // x cutoff, y roughness fade start (absolute), z thickness, w edge fade
	vec4 limits; // x the highest colour level sampled, y max steps (texels)
	// Per level: the first element's index, width, height.
	uvec4 depthLevels[kSsrMaxLevels];
	uvec4 colorLevels[kSsrMaxLevels];
}
view;

layout( set = 3, binding = 1 ) uniform texture2D depthTexture;
layout( set = 3, binding = 2 ) uniform texture2D normalRoughnessTexture;
layout( set = 3, binding = 3 ) uniform texture2D iblRadianceTexture;
layout( set = 3, binding = 4 ) uniform texture2D specularWeightTexture;
layout( set = 3, binding = 5 ) uniform texture2D litTexture;
layout( set = 3, binding = 6 ) uniform sampler pointSampler;

// The min-depth pyramid (level 0 the depth buffer) and the lit pyramid.
layout( std430, set = 3, binding = 7 ) buffer DepthPyramid
{
	float depthPyramid[];
};
layout( std430, set = 3, binding = 8 ) buffer ColorPyramid
{
	vec4 colorPyramid[];
};

float PyramidDepth( int level, ivec2 cell )
{
	const uvec4 l = view.depthLevels[level];
	return depthPyramid[l.x + uint( cell.y ) * l.y + uint( cell.x )];
}

vec4 PyramidColor( int level, ivec2 texel )
{
	const uvec4 l = view.colorLevels[level];
	return colorPyramid[l.x + uint( texel.y ) * l.y + uint( texel.x )];
}
