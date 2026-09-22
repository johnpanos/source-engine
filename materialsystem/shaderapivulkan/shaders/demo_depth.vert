#version 450
// Demo bring-up shader (roadmap R32 depth/occlusion proof): 3D position so
// triangles occupy different depths in the depth buffer.
layout( location = 0 ) in vec3 inPos;
layout( location = 1 ) in vec3 inColor;
layout( location = 0 ) out vec3 fragColor;
void main()
{
	gl_Position = vec4( inPos, 1.0 );
	fragColor = inColor;
}
