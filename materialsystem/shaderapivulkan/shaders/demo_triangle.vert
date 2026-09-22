#version 450
// Demo bring-up shader (roadmap R32 pipeline proof). Interleaved pos+color.
layout( location = 0 ) in vec2 inPos;
layout( location = 1 ) in vec3 inColor;
layout( location = 0 ) out vec3 fragColor;
void main()
{
	gl_Position = vec4( inPos, 0.0, 1.0 );
	fragColor = inColor;
}
