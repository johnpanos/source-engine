#version 450
layout( location = 0 ) in vec2 fragUv;
layout( location = 0 ) out vec4 outColor;
layout( binding = 0 ) uniform sampler2D tex;
void LegacyColorMain()
{
	outColor = texture( tex, fragUv );
}

#include "linear_target.glsl"
