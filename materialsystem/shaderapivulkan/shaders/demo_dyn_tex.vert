#version 450
// Faithful UnlitGeneric vertex stage (matches stdshaders/unlitgeneric_vs20.fxc):
//   o.vProjPos   = mul( float4( pos, 1 ), cModelViewProj )   -- push mvp
//   o.vTexCoord0 = mul( v.vTexCoord0, (float2x4)cBaseTextureTransform )
//   o.vColor     = cModulationColor   ( $color * $alpha )
// The push block carries the exact Source constants the material system commits:
// cModelViewProj (from the matrix stack), cModulationColor (c37), and the two
// rows of cBaseTextureTransform (SHADER_SPECIFIC_CONST_0/1, c38-c39).
layout( location = 0 ) in vec3 inPos;
layout( location = 1 ) in vec3 inColor; // vertex color (VERTEXCOLOR path)
layout( location = 2 ) in vec2 inUv;
layout( location = 3 ) in vec2 inLightmapUv; // TEXCOORD1: lightmap page coordinates
layout( location = 0 ) out vec2 fragUv;
layout( location = 1 ) out vec4 fragModulation;
layout( location = 2 ) out vec2 fragLightmapUv;
layout( push_constant ) uniform Constants
{
	mat4 mvp;        // cModelViewProj
	vec4 modulation; // cModulationColor
	vec4 texXform0;  // cBaseTextureTransform[0]
	vec4 texXform1;  // cBaseTextureTransform[1]
	vec4 alphaParams; // fragment stage only; see demo_dyn_tex.frag
}
consts;
void main()
{
	gl_Position = consts.mvp * vec4( inPos, 1.0 );
	// D3D9 fills a 2D texcoord to (u, v, 0, 1); the base-texture transform is a
	// 2x4 affine matrix, so u' = dot(row0, uv4), v' = dot(row1, uv4). Identity
	// (row0 = 1,0,0,0 / row1 = 0,1,0,0) passes the coordinate through unchanged.
	vec4 uv4 = vec4( inUv, 0.0, 1.0 );
	fragUv = vec2( dot( consts.texXform0, uv4 ), dot( consts.texXform1, uv4 ) );
	fragModulation = consts.modulation;
	// Lightmap coordinates are used as given (lightmappedgeneric_vs20.fxc).
	fragLightmapUv = inLightmapUv;
}
