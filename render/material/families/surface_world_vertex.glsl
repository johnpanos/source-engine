// render.material program `surface` (RFC 0016 K5, K11), the world vertex
// (SurfaceWorldVertex): the flat vertex's position, coordinates and
// color, and the world normal, tangents S and T and the bumped lightmap
// pages' offset (lightmappedgeneric_vs20's TEXCOORD2.x) the bump and env map
// terms read. Positions are in world space (the world pass draws the BSP
// world); the draw constants carry world-to-clip.

layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec2 uv0;
layout( location = 2 ) in vec2 uv1;
layout( location = 3 ) in vec4 vertexColor;
layout( location = 4 ) in vec3 normal;
layout( location = 5 ) in vec3 inTangentS;
// Tangent T in xyz and the bumped pages' offset in w (adjacent in the vertex).
layout( location = 6 ) in vec4 inTangentTOffset;

#ifndef SURFACE_SHADOW_DEPTH
#include "surface_lighting.glsl"
#endif
#include "surface_material.glsl"
#include "surface_frame.glsl"
#include "../../shaders/common/tree_sway.glsl"
layout( constant_id = 1 ) const int kTreeSwayMode = 0;
// A static prop's baked vertex lighting in the vertex color
// (SurfaceVariant::staticVertexLight).
layout( constant_id = 9 ) const bool kStaticVertexLight = false;

layout( push_constant ) uniform Draw
{
	layout( row_major ) mat4 toClip;
	layout( row_major ) mat4 world;
} draw;

#ifdef SURFACE_TEMPORAL
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
#ifdef SURFACE_SHADOW_DEPTH
float lightmapOffset; // Lighting-only varying, absent from the depth interface.
#else
layout( location = 8 ) out float lightmapOffset;
#endif
layout( location = 9 ) out vec4 lightAtten; // the model lights' attenuations
layout( location = 10 ) out vec3 vertexLighting; // the vertexlit point's (none here)

// Dynamic meshes are captured after the frontend transforms them. Animate in
// the authored root coordinates, then return to world space for every view.
vec3 AnimateWorld( vec3 worldPosition, vec4 windTime )
{
	if ( kTreeSwayMode == 0 )
		return worldPosition;
	const vec3 objectPosition = ( inverse( draw.world ) * vec4( worldPosition, 1.0 ) ).xyz;
	return ( draw.world * vec4( TreeSway( objectPosition, draw.world, windTime.xy, windTime.z,
	    kTreeSwayMode, material.treeGeometry, material.treeMotion, material.treeCurves,
	    material.treeWind ), 1.0 ) ).xyz;
}

void main()
{
	const vec3 animated = AnimateWorld( position, frame.foliage[0] );
	gl_Position = draw.toClip * vec4( animated, 1.0 );
#ifdef SURFACE_TEMPORAL
    motionCurrent = frame.motionCurrentToClip * vec4( animated, 1.0 );
    motionPrevious = frame.motionPreviousToClip *
        vec4( AnimateWorld( previousPosition, frame.foliage[1] ), 1.0 );
#endif
	baseUv = uv0;
	lightmapUv = uv1;
	// LightmappedGeneric's port passes vertex colors through unconverted;
	// UnlitGeneric's decodes them per vertex (GammaToLinear, pow 2.2).
	color = material.state.y != 0.0 ? vec4( pow( vertexColor.rgb, vec3( 2.2 ) ), vertexColor.a )
	                                 : vertexColor;
	fogDepth = vec2( gl_Position.z, animated.z );
	worldPosition = animated;
	worldNormal = normal;
	tangentS = inTangentS;
	tangentT = inTangentTOffset.xyz;
	lightmapOffset = inTangentTOffset.w;
	// A dynamic mesh draw's model lights (the mesh handoff's, R91); the world
	// pass's own surfaces bind the neutral block, which has none.
	vec4 atten = vec4( 0.0 );
#ifndef SURFACE_SHADOW_DEPTH
	const int count = int( lighting.eye.w );
	for ( int i = 0; i < 4; ++i )
	{
		if ( i < count )
			atten[i] = ModelLightAttenuation( i, animated );
	}
#endif
	lightAtten = atten;
	vertexLighting = vec3( 0.0 );
	// vertexlit_and_unlit_generic's STATIC_LIGHT: GammaToLinear( color *
	// cOverbright ), the static-prop color lump's baked light. The color is
	// that light, not a tint.
	if ( kStaticVertexLight )
	{
		vertexLighting = pow( vertexColor.rgb * 2.0, vec3( 2.2 ) );
		color = vec4( 1.0, 1.0, 1.0, vertexColor.a );
	}
}
