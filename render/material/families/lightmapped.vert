// render.material family `lightmapped` (RFC 0016 K4): LightmappedGeneric's
// claimed subset. The draw constants carry the draw's world-to-clip matrix
// (row-major with column vectors, as render.math stores them).
#version 450

layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec2 uv0;
layout( location = 2 ) in vec2 uv1;
layout( location = 3 ) in vec4 vertexColor;

layout( push_constant ) uniform Draw
{
	layout( row_major ) mat4 toClip;
} draw;

layout( location = 0 ) out vec2 baseUv;
layout( location = 1 ) out vec2 lightmapUv;
layout( location = 2 ) out vec4 color;

void main()
{
	gl_Position = draw.toClip * vec4( position, 1.0 );
	baseUv = uv0;
	lightmapUv = uv1;
	// The port passes vertex colors through unconverted, unlike unlit's.
	color = vertexColor;
}
