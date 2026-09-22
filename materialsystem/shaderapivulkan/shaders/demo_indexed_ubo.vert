#version 450
// Demo bring-up shader (roadmap R32 index-buffer + constant-buffer proof).
layout( location = 0 ) in vec2 inPos;
void main()
{
	gl_Position = vec4( inPos, 0.0, 1.0 );
}
