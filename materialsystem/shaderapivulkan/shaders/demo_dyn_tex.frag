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
//
// With the self-illumination flag it is vertexlit_and_unlit_generic_ps2x's
// SELFILLUM blend (see main).
//
// With the luminance-compare flag it is luminance_compare_ps2x.fxc, the pixel
// shader of dev/lumcompare that auto-exposure's histogram counts with:
//   result = step( c0.x, L ) * step( L, c0.y ),  L = dot( base * c0.z, NTSC )
// with c0 carried in the modulation slot, which that shader does not use.
layout( location = 0 ) in vec2 fragUv;
layout( location = 1 ) in vec4 fragModulation;
layout( location = 2 ) in vec2 fragLightmapUv;
layout( location = 3 ) in vec4 fragVertexColor;
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
	// z = flags: 1 sRGB base, 2 sRGB lightmap, 4 sRGB output, 8 alpha test
	// passes only above the reference (GREATER; else GREATEREQUAL), 16 luminance
	// compare, 32 screen-space vertex stage (see demo_dyn_tex.vert), 128 the
	// vertex color in place of the base texture (bufferclearobeystencil_ps2x:
	// result = vColor, the clear color the quad's vertices carry), 256 times
	// the vertex color and 1024 times the vertex alpha
		// (vertexlit_and_unlit_generic_ps2x with VERTEXCOLOR / g_fVertexAlpha;
		// 512 is read by the vertex stage), 2048 its SELFILLUM (see main), 4096
		// the Cable_DX9 normal-map half-Lambert term (sampler 0 normal, sampler 1
		// sRGB base texture and fragVertexColor directional-light color), 16384
		// MonitorScreen_DX9 (contrast in alphaParams.x, saturation and tint in
		// fragVertexColor), 32768 its optional second texture;
	// w = linear output scale (FinalOutput's LINEAR_LIGHT_SCALE).
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
	vec4 base = ( flags & 128 ) != 0 ? fragVertexColor : texture( baseTexture, fragUv );
	if ( ( flags & 1 ) != 0 )
		base.rgb = SrgbToLinear( base.rgb );
	vec4 result;
	if ( ( flags & 16 ) != 0 )
	{
		const float luminance =
		    dot( base.rgb * consts.modulation.z, vec3( 0.2125, 0.7154, 0.0721 ) );
		result = vec4(
		    step( consts.modulation.x, luminance ) * step( luminance, consts.modulation.y ) );
	}
	else if ( ( flags & 2048 ) != 0 )
	{
		// vertexlit_and_unlit_generic_ps2x.fxc with SELFILLUM (no mask texture):
		//   diffuse = lerp( albedo * diffuseLighting, g_SelfIllumTint * albedo, base.a )
		//   alpha   = g_DiffuseModulation.a
		// with albedo = base * c1. Factored as base * lerp( c1 * L, c1 * tint, a):
		// the vertex color carries c1 * L (L = 1 when unlit) and the modulation
		// carries ( c1 * tint, c1.a ), so the tint needs no push-constant slot.
		result = vec4( base.rgb * mix( fragVertexColor.rgb, fragModulation.rgb, base.a ),
		    fragModulation.a );
		if ( ( flags & 1024 ) != 0 )
			result.a *= fragVertexColor.a;
	}
	else if ( ( flags & 4096 ) != 0 )
	{
		// Cable_DX9's cable_ps2x.fxc decodes the normal-map texel from [0,1]
		// to tangent-space [-1,1], then applies a squared half-Lambert from +Z.
		// The cable shader binds its normal map at s0 and its authored color at
		// s1; sampler 1 is supplied by the material pipeline's second descriptor.
		vec3 normal = texture( baseTexture, fragUv ).xyz * 2.0 - 1.0;
		float halfLambert = normal.z * 0.5 + 0.5;
		float lighting = halfLambert * halfLambert;
		vec4 cableBase = texture( lightmapTexture, fragLightmapUv );
		result = vec4( cableBase.rgb * fragVertexColor.rgb * lighting,
			cableBase.a * fragVertexColor.a );
	}
	else if ( ( flags & 16384 ) != 0 )
	{
		// monitorscreen_ps2x.fxc: base times cModulationColor and, when its
		// TEXTURE2 combo is set, sampler 1; then contrast, saturation and tint.
		result = base * fragModulation;
		if ( ( flags & 32768 ) != 0 )
		{
			vec4 second = texture( lightmapTexture, fragLightmapUv );
			if ( ( flags & 2 ) != 0 )
				second.rgb = SrgbToLinear( second.rgb );
			result *= second;
		}
		result.rgb = mix( result.rgb, result.rgb * result.rgb, consts.alphaParams.x );
		float grey = dot( result.rgb, vec3( 0.33333 ) );
		result.rgb = mix( vec3( grey ), result.rgb, fragVertexColor.a );
		result.rgb *= fragVertexColor.rgb;
	}
	else
	{
		result = fragModulation * base;
		// vertexlit_and_unlit_generic_ps2x.fxc, unlit: diffuseLighting =
		// i.color.rgb with VERTEXCOLOR, and
		// alpha = lerp( alpha, alpha * i.color.a, g_fVertexAlpha ).
		if ( ( flags & 256 ) != 0 )
			result.rgb *= fragVertexColor.rgb;
		if ( ( flags & 1024 ) != 0 )
			result.a *= fragVertexColor.a;
		if ( consts.alphaParams.y > 0.5 )
		{
			vec3 lightmap = texture( lightmapTexture, fragLightmapUv ).rgb;
			if ( ( flags & 2 ) != 0 )
				lightmap = SrgbToLinear( lightmap );
			result.rgb *= lightmap;
		}
	}
	// The D3D9 fixed-function alpha test: GREATEREQUAL ($alphatest) or GREATER
	// (screenspace_general), against the reference. Disabled when < 0.
	if ( ( flags & 16384 ) == 0 && consts.alphaParams.x >= 0.0 &&
	     ( ( flags & 8 ) != 0 ? result.a <= consts.alphaParams.x
	                          : result.a < consts.alphaParams.x ) )
		discard;
	result.rgb *= consts.alphaParams.w;
	if ( ( flags & 4 ) != 0 )
		result.rgb = LinearToSrgb( result.rgb );
	outColor = result;
}
