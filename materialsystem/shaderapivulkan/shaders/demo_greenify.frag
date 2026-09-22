#version 450
// A second "material shader": ignores vertex color and outputs green. Selecting
// it vs. the passthrough shader proves shader binding routes to a distinct
// native Vulkan pipeline.
layout( location = 0 ) out vec4 outColor;
void main()
{
	outColor = vec4( 0.0, 1.0, 0.0, 1.0 );
}
