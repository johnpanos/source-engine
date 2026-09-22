#version 450
// Faithful UnlitGeneric vertex stage (matches stdshaders/unlitgeneric_vs20.fxc):
//   o.vProjPos   = mul( float4( pos, 1 ), cModelViewProj )   -- push mvp
//   o.vTexCoord0 = mul( v.vTexCoord0, (float2x4)cBaseTextureTransform )
//   o.vColor     = cModulationColor   ( $color * $alpha )
// The push block carries the exact Source constants the material system commits:
// cModelViewProj (from the matrix stack), cModulationColor (c37), and the two
// rows of cBaseTextureTransform (SHADER_SPECIFIC_CONST_0/1, c38-c39).
//
// With the screen-space flag (alphaParams.z & 32) it is screenspaceeffect_vs20.fxc
// instead: the position is already in clip space and the texture coordinate is
// used as given, as DrawScreenSpaceRectangle's vertices are built.
//
// Compiled with -DCLIP_PLANES it also applies D3D9's user clip planes: planes in
// clip space (SetClipPlane under a vertex shader) that keep a vertex where
// dot( plane, position ) >= 0. They follow the block; unused planes are zero.
layout( location = 0 ) in vec3 inPos;
layout( location = 1 ) in vec3 inColor; // vertex color (VERTEXCOLOR path)
layout( location = 2 ) in vec2 inUv;
layout( location = 3 ) in vec2 inLightmapUv; // TEXCOORD1: lightmap page coordinates
layout( location = 6 ) in float inAlpha;     // vertex color alpha
layout( location = 0 ) out vec2 fragUv;
layout( location = 1 ) out vec4 fragModulation;
layout( location = 2 ) out vec2 fragLightmapUv;
layout( location = 3 ) out vec4 fragVertexColor; // read with flags 128, 256, 1024
layout( push_constant ) uniform Constants
{
	mat4 mvp;        // cModelViewProj
	vec4 modulation; // cModulationColor
	vec4 texXform0;  // cBaseTextureTransform[0]
	vec4 texXform1;  // cBaseTextureTransform[1]
	vec4 alphaParams; // .z flags (32: screen space); see demo_dyn_tex.frag
#ifdef CLIP_PLANES
	vec4 clipPlanes[2];
#endif
}
consts;
#ifdef CLIP_PLANES
out float gl_ClipDistance[2];
void ApplyClipPlanes()
{
	gl_ClipDistance[0] = dot( consts.clipPlanes[0], gl_Position );
	gl_ClipDistance[1] = dot( consts.clipPlanes[1], gl_Position );
}
#else
void ApplyClipPlanes()
{
}
#endif
void main()
{
	fragModulation = consts.modulation;
	fragLightmapUv = inLightmapUv;
	// vertexlit_and_unlit_generic_vs20.fxc with VERTEXCOLOR: the vertex color
	// is GammaToLinear'd (pow 2.2) unless DONT_GAMMA_CONVERT_VERTEX_COLOR (512).
	const int flags = int( consts.alphaParams.z );
	fragVertexColor = vec4( inColor, inAlpha );
	if ( ( flags & 256 ) != 0 && ( flags & 512 ) == 0 )
		fragVertexColor.rgb = pow( inColor, vec3( 2.2 ) );
	if ( ( flags & 32 ) != 0 )
	{
		gl_Position = vec4( inPos, 1.0 );
		fragUv = inUv;
		ApplyClipPlanes();
		return;
	}
	gl_Position = consts.mvp * vec4( inPos, 1.0 );
	ApplyClipPlanes();
	// D3D9 fills a 2D texcoord to (u, v, 0, 1); the base-texture transform is a
	// 2x4 affine matrix, so u' = dot(row0, uv4), v' = dot(row1, uv4). Identity
	// (row0 = 1,0,0,0 / row1 = 0,1,0,0) passes the coordinate through unchanged.
	vec4 uv4 = vec4( inUv, 0.0, 1.0 );
	fragUv = vec2( dot( consts.texXform0, uv4 ), dot( consts.texXform1, uv4 ) );
	// Lightmap coordinates are used as given (lightmappedgeneric_vs20.fxc).
}
