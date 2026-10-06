// One frame block shared by surface stages.
// An area light (render.area-light.v1, area_light::AreaLight): its
// rectangle, its radiance and its reach.
struct AreaLight
{
	vec4 center; // w: 1 when two-sided
	vec4 halfU;  // w: the reach
	vec4 halfV;
	vec4 radiance;
};
const int kMaxAreaLights = 64;

layout( set = 0, binding = 0 ) uniform Frame
{
	// x: the lightmap scale for how the pages encode light (2^2.2 for LDR
	// gamma pages, 16 for integer-HDR pages); y: the output's linear scale
	// (the frame's tone-mapping scale, 1 without HDR); z: 1 when the target
	// has no sRGB view and the shader encodes the output itself; w: 1 when
	// specular shows (mat_specular), else the env map's tint is zero.
	vec4 light;
	// The view's fog (common_ps_fxc.h CalcPixelFogFactor, BlendPixelFog):
	// color with its type in w (-1 none, 0 range, 1 height), parameters
	// (range: start / range, water z, max density, 1 / range; height: 0,
	// water z, 1, 1 / range), and the eye's world z in misc.x.
	vec4 fogColor;
	vec4 fogParams;
	vec4 fogMisc; // x: the eye's world z, y: 1 when the game scales every ssbump by 1/sqrt(3)
	// xyz: the eye's world position (c10); w: ENV_MAP_SCALE (16 in integer
	// HDR, where cube maps hold light / 16, else 1).
	vec4 eye;
	vec4 areaCount; // x: the frame's area lights (the pbr point reads them)
	// The sun: towards it (w the tangent of its disc's angular radius), its
	// diffuse light (w 1 when that is in the baked light) and its shadow (x
	// the first cascade tile or -1, y the cascades, z 1 to read the baked
	// mask in the gradient page's alpha).
	vec4 sunDirection;
	vec4 sunColor;
	vec4 sunShadow;
	// The water point: x the shaders' time in seconds, y its reflection
	// tint's scale (4 in integer HDR), zw the camera's right in the water
	// plane.
	vec4 water;
	vec4 viewport; // the view's x, y, 1 / width, 1 / height in the target
	AreaLight areas[kMaxAreaLights];
	vec4 clipPlanes[6];
	layout( row_major ) mat4 motionCurrentToClip;
	layout( row_major ) mat4 motionPreviousToClip;
	vec4 motionExtent; // width, height, valid previous view, reserved
	vec4 foliage[2]; // xy horizontal wind, z animation time, current/previous frame
} frame;
