#version 450
// The screen-space vertex stages of the bloom and color-correction passes:
// screenspaceeffect_vs20.fxc, Downsample_vs20.fxc and BlurFilter_vs20.fxc. Their
// positions are already in clip space (DrawScreenSpaceRectangle builds them) and
// each derives its taps from TEXCOORD0 and SHADER_SPECIFIC_CONST_0..3 by
// addition, which screenspace_post.frag does per pixel with the same result.
layout( location = 0 ) in vec3 inPos;
layout( location = 2 ) in vec2 inUv;
layout( location = 0 ) out vec2 vUv;
void main()
{
	gl_Position = vec4( inPos, 1.0 );
	vUv = inUv;
}
