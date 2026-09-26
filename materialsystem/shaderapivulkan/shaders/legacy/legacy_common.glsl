// The inputs every legacy shader port shares (vulkan_legacy_programs.h): the
// registers the material's shader wrote, the pass's combo indices and the push
// block. Included by both stages of each port.
//
// A port reads its registers under the names its HLSL gives them, e.g.
//   #define g_DiffuseModulation PS_C( 1 )
//   #define cModulationColor VS_C( 47 )
// and decodes its combos with the strides of its fxctmp9/*.inc header:
//   const int CUBEMAP = STATIC_PS_COMBO( 4, 2 );
#ifndef LEGACY_COMMON_GLSL
#define LEGACY_COMMON_GLSL

layout( set = 1, binding = 0 ) uniform LegacyConstants
{
	vec4 ps[32];
	vec4 vs[72]; // c0..c63, then c217..c224
	ivec4 combos; // ps static, ps dynamic, vs static, vs dynamic combo index
	ivec4 bools;  // ps b0..b15 bits, vs b0..b15 bits, D3D9 samplers read as sRGB,
	              // the pass's vertex format (VERTEX_* flags)
	ivec4 vsLoop; // the vertex shader's i0 (the light loop: count, 0, 1, 0)
	vec4 target;  // render target width, height, 1 / width, 1 / height
}
lc;

// params: x the alpha-test reference (< 0 disables), y 1 when the test is
// GREATER (else GREATEREQUAL), z the sampler slots (bits 0..15) the shader must
// decode from sRGB itself and bit 16 when it must encode its output to sRGB
// itself, w 0. params2: 0.
layout( push_constant ) uniform LegacyPush
{
	mat4 viewProj; // cViewProj
	vec4 unused0;
	vec4 unused1;
	vec4 eyePos;   // the world-space camera position
	vec4 params;
	vec4 params2;
	vec4 clipPlanes[2];
}
pc;

#define PS_C( n ) lc.ps[( n )]
#define VS_C( n ) lc.vs[( n ) < 64 ? ( n ) : ( n ) - 217 + 64]
#define PS_BOOL( n ) ( ( lc.bools.x & ( 1 << ( n ) ) ) != 0 )
#define VS_BOOL( n ) ( ( lc.bools.y & ( 1 << ( n ) ) ) != 0 )

// A combo's value from its stride and count in the pass's static or dynamic
// index (the fxctmp9 .inc headers' m_n<COMBO> * stride sums).
#define STATIC_PS_COMBO( stride, count ) ( ( lc.combos.x / ( stride ) ) % ( count ) )
#define DYNAMIC_PS_COMBO( stride, count ) ( ( lc.combos.y / ( stride ) ) % ( count ) )
#define STATIC_VS_COMBO( stride, count ) ( ( lc.combos.z / ( stride ) ) % ( count ) )
#define DYNAMIC_VS_COMBO( stride, count ) ( ( lc.combos.w / ( stride ) ) % ( count ) )

// HLSL's saturate and the D3D9 matrix products Source's shaders use: mul( v, M )
// with M given as rows in consecutive registers.
float saturate( float x )
{
	return clamp( x, 0.0, 1.0 );
}
vec2 saturate( vec2 x )
{
	return clamp( x, 0.0, 1.0 );
}
vec3 saturate( vec3 x )
{
	return clamp( x, 0.0, 1.0 );
}
vec4 saturate( vec4 x )
{
	return clamp( x, 0.0, 1.0 );
}

// common_fxc.h's GammaToLinear / LinearToGamma: D3D9's pow is exp2( y *
// log2( |x| ) ), so the base's sign is dropped as the compiled shaders drop it.
vec3 GammaToLinear( vec3 gamma )
{
	return pow( abs( gamma ), vec3( 2.2 ) );
}
vec3 LinearToGamma( vec3 linear )
{
	return pow( abs( linear ), vec3( 1.0 / 2.2 ) );
}

#endif // LEGACY_COMMON_GLSL
