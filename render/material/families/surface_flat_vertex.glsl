// render.material program `surface` (RFC 0016 K4, K5, K11), the flat vertex:
// position, base and lightmap coordinates and color (SurfaceFlatVertex).
// Terms that read the surface's normal and tangents take the world vertex
// (surface_world.vert); here they are zero, as are the model lights'
// attenuations (surface_model.vert). The draw constants carry the
// draw's world-to-clip matrix (row-major with column vectors, as render.math
// stores them).

layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec2 uv0;
layout( location = 2 ) in vec2 uv1;
layout( location = 3 ) in vec4 vertexColor;

#include "surface_material.glsl"

layout( push_constant ) uniform Draw
{
	layout( row_major ) mat4 toClip;
} draw;

#ifdef SURFACE_TEMPORAL
#include "surface_frame.glsl"
layout( location = 7 ) in vec3 previousPosition;
layout( location = 11 ) out vec4 motionCurrent;
layout( location = 12 ) out vec4 motionPrevious;
#endif
invariant gl_Position;

layout( location = 0 ) out vec2 baseUv;
layout( location = 1 ) out vec2 lightmapUv;
layout( location = 2 ) out vec4 color;
layout( location = 3 ) out vec2 fogDepth; // the clip-space z (D3D9's projPos.z) and world z
layout( location = 4 ) out vec3 worldPosition;
layout( location = 5 ) out vec3 worldNormal;
layout( location = 6 ) out vec3 tangentS;
layout( location = 7 ) out vec3 tangentT;
layout( location = 8 ) out float lightmapOffset;
layout( location = 9 ) out vec4 lightAtten; // the model lights' attenuations (none here)
layout( location = 10 ) out vec3 vertexLighting; // the vertexlit point's (none here)

void main()
{
	gl_Position = draw.toClip * vec4( position, 1.0 );
#ifdef SURFACE_TEMPORAL
    motionCurrent = frame.motionCurrentToClip * vec4( position, 1.0 );
    motionPrevious = frame.motionPreviousToClip * vec4( previousPosition, 1.0 );
#endif
	baseUv = uv0;
	lightmapUv = uv1;
	// LightmappedGeneric's port passes vertex colors through unconverted;
	// UnlitGeneric's decodes them per vertex (GammaToLinear, pow 2.2).
	color = material.state.y != 0.0 ? vec4( pow( vertexColor.rgb, vec3( 2.2 ) ), vertexColor.a )
	                                 : vertexColor;
	fogDepth = vec2( gl_Position.z, position.z );
	worldPosition = position;
	worldNormal = vec3( 0.0 );
	tangentS = vec3( 0.0 );
	tangentT = vec3( 0.0 );
	lightmapOffset = 0.0;
	lightAtten = vec4( 0.0 );
	vertexLighting = vec3( 0.0 );
}
