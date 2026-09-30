// render.pass.panels (RFC 0016): a panel's draw list into its image. The
// quads are in the panel's layout units (origin top-left, y down); the draw
// constants carry units-to-clip (render.math's PixelToClip over the panel's
// units, so image row 0 is the panel's top) and the quad's modes.
#version 450

layout( location = 0 ) in vec2 position;
layout( location = 1 ) in vec2 uv;
layout( location = 2 ) in vec4 color; // gamma RGBA, straight alpha

layout( push_constant ) uniform Constants
{
	layout( row_major ) mat4 toClip;
	vec4 params; // x: 1 when the texture's alpha is not read; y: 1 to premultiply (additive)
}
constants;

layout( location = 0 ) out vec2 vertexUv;
layout( location = 1 ) out vec4 vertexColor;

void main()
{
	gl_Position = constants.toClip * vec4( position, 0.0, 1.0 );
	vertexUv = uv;
	vertexColor = color;
}
