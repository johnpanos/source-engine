#version 450
// Demo bring-up shader (roadmap R32 texture-path proof): pos + uv quad.
layout( location = 0 ) in vec2 inPos;
layout( location = 1 ) in vec2 inUv;
layout( location = 0 ) out vec2 fragUv;
void main()
{
	gl_Position = vec4( inPos, 0.0, 1.0 );
	fragUv = inUv;
}
