#version 450
// Faithful UnlitGeneric pixel stage (matches stdshaders/unlitgeneric_ps2x.fxc):
//   result = i.vColor0 * tex2D( TextureSampler, i.vTexCoord0 )
// where vColor0 is cModulationColor ($color * $alpha) interpolated from the
// vertex stage. This is the exact D3D9 UnlitGeneric material output (before the
// FinalOutput sRGB/fog/tonemap stage, which is identity for an opaque, fog-off,
// linear material -- the equivalence oracle drives that case).
//
// With a lightmap bound it is LightmappedGeneric's lightmap term
// (lightmappedgeneric_ps2_3_x.h): result = albedo * lightmap * c12, where the
// material folds GetLightMapScaleFactor() into the modulation. Inputs the
// material reads as sRGB are decoded first and the result is encoded when it
// writes sRGB, as EnableSRGBRead/EnableSRGBWrite select on D3D9.
layout( location = 0 ) in vec2 fragUv;
layout( location = 1 ) in vec4 fragModulation;
layout( location = 2 ) in vec2 fragLightmapUv;
layout( location = 0 ) out vec4 outColor;
layout( set = 0, binding = 0 ) uniform sampler2D baseTexture;
layout( set = 1, binding = 0 ) uniform sampler2D lightmapTexture;
layout( push_constant ) uniform Constants
{
	mat4 mvp;
	vec4 modulation;
	vec4 texXform0;
	vec4 texXform1;
	// x = alpha-test reference (<0 disables); y = 1 to multiply by the lightmap;
	// z = color-space flags (1 sRGB base, 2 sRGB lightmap, 4 sRGB output).
	vec4 alphaParams;
}
consts;
// The exact sRGB transfer functions D3D9's sRGB sampling and output apply.
vec3 SrgbToLinear( vec3 c )
{
	return mix( c / 12.92, pow( ( c + 0.055 ) / 1.055, vec3( 2.4 ) ), step( 0.04045, c ) );
}
vec3 LinearToSrgb( vec3 c )
{
	c = clamp( c, 0.0, 1.0 );
	return mix( c * 12.92, 1.055 * pow( c, vec3( 1.0 / 2.4 ) ) - 0.055, step( 0.0031308, c ) );
}
void main()
{
	const int flags = int( consts.alphaParams.z );
	vec4 base = texture( baseTexture, fragUv );
	if ( ( flags & 1 ) != 0 )
		base.rgb = SrgbToLinear( base.rgb );
	vec4 result = fragModulation * base;
	if ( consts.alphaParams.y > 0.5 )
	{
		vec3 lightmap = texture( lightmapTexture, fragLightmapUv ).rgb;
		if ( ( flags & 2 ) != 0 )
			lightmap = SrgbToLinear( lightmap );
		result.rgb *= lightmap;
	}
	// $alphatest: discard fragments below the reference alpha, matching the D3D9
	// fixed-function alpha test (ALPHAFUNC = GREATEREQUAL). Disabled when < 0.
	if ( consts.alphaParams.x >= 0.0 && result.a < consts.alphaParams.x )
		discard;
	if ( ( flags & 4 ) != 0 )
		result.rgb = LinearToSrgb( result.rgb );
	outColor = result;
}
