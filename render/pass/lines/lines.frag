// render.pass.lines (RFC 0016): the vertex color, alpha-blended by the
// pipeline. The debug views (RFC 0014) come from debug_view.glsl.
#version 450

#include "../../shaders/common/debug_view.glsl"

layout( location = 0 ) in vec4 vertexColor;
layout( location = 0 ) out vec4 outColor;

void main()
{
	if ( DebugViewActive() )
	{
		DebugInputs inputs = DebugInputsNone();
		inputs.mask = kDebugHasVertexColor;
		inputs.vertexColor = vertexColor;
		inputs.final = vertexColor.rgb;
		outColor = DebugViewOutput( inputs );
		return;
	}
	outColor = vertexColor;
}
