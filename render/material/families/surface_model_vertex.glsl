// render.material program `surface` (RFC 0016 K4, K11), the model vertex
// (SurfaceModelVertex): position, normal, tangent (w: the bitangent's sign) and uv0 of
// a mesh. The draw constants are the FamilyDrawConstants prefix: the draw's
// object-to-clip and object-to-world matrices (row-major with column
// vectors). Normals and tangents are transformed by object-to-world's upper
// 3x3, as the model port does, and normalized per pixel. Each model light's
// attenuation is Source's per-vertex term (common_vs_fxc.h
// GetVertexAttenForLight), interpolated as the port does. A mesh has no
// lightmap coordinates and a white vertex color. The vertexlit point
// (kVertexLit) also evaluates the port's lit, non-bumped vertex lighting
// (common_vs_fxc.h DoLighting with static control flow): each light's color
// times its cosine term (Lambert, or half-Lambert squared with
// kHalfLambert) times its attenuation, in order, then the ambient cube.

layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec3 normal;
layout( location = 2 ) in vec4 tangent; // w: the bitangent's sign
layout( location = 3 ) in vec2 uv0;

// The program's terms (surface.frag's kTerms).
layout( constant_id = 0 ) const int kTerms = 0;
layout( constant_id = 1 ) const int kTreeSwayMode = 0;
const int kHalfLambert = 16;
const int kVertexLit = 8192;
const int kMeshDirect = 8388608;

layout( push_constant ) uniform Draw
{
	layout( row_major ) mat4 toClip;
	layout( row_major ) mat4 world;
} draw;

#ifdef SURFACE_INSTANCED
// GPU-driven draws (SurfaceVariant::instanced): the same two matrices per
// instance, as rows, from the per-instance buffer the indirect command's
// first instance selects.
layout( location = 8 ) in vec4 instanceToClip[4];
layout( location = 12 ) in vec4 instanceWorld[4];
mat4 DrawToClip()
{
	return transpose( mat4( instanceToClip[0], instanceToClip[1], instanceToClip[2],
	    instanceToClip[3] ) );
}
mat4 DrawWorld()
{
	return transpose( mat4( instanceWorld[0], instanceWorld[1], instanceWorld[2],
	    instanceWorld[3] ) );
}
#else
mat4 DrawToClip()
{
	return draw.toClip;
}
mat4 DrawWorld()
{
	return draw.world;
}
#endif

#include "surface_lighting.glsl"
#include "surface_material.glsl"
#include "surface_frame.glsl"
#include "../../shaders/common/tree_sway.glsl"

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
layout( location = 9 ) out vec4 lightAtten;
layout( location = 10 ) out vec3 vertexLighting;

// CosineTermInternal: Lambert, or half-Lambert squared.
float CosineTerm( int i, vec3 position, vec3 normal )
{
	vec3 toLight = normalize( lighting.lights[i].position.xyz - position );
	toLight = mix( toLight, -lighting.lights[i].direction.xyz, lighting.lights[i].color.w );
	const float normalDotLight = dot( normal, toLight );
	if ( ( kTerms & kHalfLambert ) == 0 )
		return max( 0.0, normalDotLight );
	const float halfLambert = normalDotLight * 0.5 + 0.5;
	return halfLambert * halfLambert;
}

void main()
{
	const vec3 animated = TreeSway( position, DrawWorld(), frame.foliage[0].xy, frame.foliage[0].z,
	    kTreeSwayMode, material.treeGeometry, material.treeMotion, material.treeCurves, material.treeWind );
	const vec4 world = DrawWorld() * vec4( animated, 1.0 );
	gl_Position = DrawToClip() * vec4( animated, 1.0 );
#ifdef SURFACE_TEMPORAL
    motionCurrent = frame.motionCurrentToClip * world;
    const vec3 previous = TreeSway( previousPosition, DrawWorld(), frame.foliage[1].xy, frame.foliage[1].z,
        kTreeSwayMode, material.treeGeometry, material.treeMotion, material.treeCurves, material.treeWind );
    motionPrevious = frame.motionPreviousToClip * ( DrawWorld() * vec4( previous, 1.0 ) );
#endif
	baseUv = uv0;
	lightmapUv = vec2( 0.0 );
	color = vec4( 1.0 );
	fogDepth = vec2( gl_Position.z, world.z );
	worldPosition = world.xyz;
	const mat3 basis = mat3( DrawWorld() );
	worldNormal = basis * normal;
	tangentS = basis * tangent.xyz;
	tangentT = cross( worldNormal, tangentS ) * tangent.w;
	lightmapOffset = 0.0;
	// Modern meshes take their direct light from the view's clustered set.
	// The old per-draw lights belong only to the legacy model points.
	const int count = ( kTerms & kMeshDirect ) != 0 ? 0 : int( lighting.eye.w );
	vec4 atten = vec4( 0.0 );
	for ( int i = 0; i < 4; ++i )
	{
		if ( i < count )
			atten[i] = ModelLightAttenuation( i, world.xyz );
	}
	lightAtten = atten;
	vertexLighting = vec3( 0.0 );
	if ( ( kTerms & kVertexLit ) != 0 )
	{
		const vec3 n = normalize( worldNormal );
		vec3 sum = vec3( 0.0 );
		for ( int i = 0; i < 4; ++i )
		{
			if ( i < count )
				sum += lighting.lights[i].color.xyz * CosineTerm( i, world.xyz, n ) * atten[i];
		}
		vertexLighting = sum + ModelAmbientCube( n );
	}
}
