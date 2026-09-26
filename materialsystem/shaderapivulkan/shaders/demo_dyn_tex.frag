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
layout( location = 4 ) in vec3 fragReflection;
layout( location = 5 ) in vec2 fragScreenUv;
layout( location = 6 ) in vec4 fragEnvTint;
// The draw's pixel fog (CVulkanContext::DrawFog; see ApplyPixelFog).
layout( location = 7 ) flat in vec4 fragFogColor;
layout( location = 8 ) flat in vec4 fragFogParams;
layout( location = 9 ) flat in vec4 fragFogMisc;
layout( location = 10 ) in vec2 fragFogDepth;
layout( location = 0 ) out vec4 outColor;
layout( set = 0, binding = 0 ) uniform sampler2D baseTexture;
layout( set = 1, binding = 0 ) uniform sampler2D lightmapTexture;
layout( set = 2, binding = 0 ) uniform samplerCube envmapTexture;
layout( set = 3, binding = 0 ) uniform sampler2D normalMaskTexture;
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
		// sRGB base texture and fragVertexColor directional-light color), 65536
		// Sprite_DX9 vertex alpha, 16384
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
// common_ps_fxc.h's pixel fog, which FinalOutput applies after the tone-mapping
// scale and before the sRGB encode: CalcPixelFogFactor (CalcRangeFog for range
// fog, CalcWaterFogAlpha for height fog) with the pass's g_FogParams, then
// BlendPixelFog toward g_LinearFogColor, squaring a range factor. Fog type -1
// is a shader that does not fog. DecalModulate raises the factor to the 0.4.
vec3 ApplyPixelFog( vec3 color )
{
	const float fogType = fragFogColor.w;
	if ( fogType < -0.5 )
		return color;
	const float projZ = fragFogDepth.x;
	float factor;
	if ( fogType < 0.5 )
	{
		factor = clamp( min( fragFogParams.z, projZ * fragFogParams.w - fragFogParams.x ), 0.0, 1.0 );
	}
	else
	{
		const float worldZ = fragFogDepth.y;
		const float depthFromWater = fragFogParams.y - worldZ;
		const float depthFromEye = fragFogMisc.x - worldZ;
		const float f = clamp( depthFromWater * ( 1.0 / depthFromEye ), 0.0, 1.0 );
		factor = clamp( f * projZ * fragFogParams.w, 0.0, 1.0 );
	}
	if ( fragFogMisc.y > 0.5 )
		factor = pow( factor, 0.4 );
	if ( fogType < 0.5 )
		factor *= factor;
	return mix( color, fragFogColor.rgb, factor );
}
// shadow_ps2x.fxc (alphaParams.y == 2): the render-to-texture shadow projected
// onto a surface. The shadow texture is sampled at five jittered taps; the
// vertex color carries the jitter ( 1 / width, 1 / height ) in rg (the shader
// reads only the color's alpha) and the fade in a; the modulation is
// g_ShadowColor (c1). The result multiplies the frame (ZERO, SRC_COLOR), so it
// fades to white in fog instead of blending toward the fog color.
vec4 ShadowProjection( int flags )
{
	const vec2 jitter0 = fragVertexColor.rg;
	const vec2 jitter1 = vec2( jitter0.x, -jitter0.y );
	const float coverageSum = texture( baseTexture, fragUv ).a +
	                          texture( baseTexture, fragUv + jitter0 ).a +
	                          texture( baseTexture, fragUv - jitter0 ).a +
	                          texture( baseTexture, fragUv + jitter1 ).a +
	                          texture( baseTexture, fragUv - jitter1 ).a;
	const float shadowCoverage = clamp( coverageSum * 0.2 - fragVertexColor.a, 0.0, 1.0 );
	vec3 result = 1.0 + ( shadowCoverage * fragModulation.rgb - shadowCoverage );
	// CalcPixelFogFactor without BlendPixelFog's squaring.
	float fogFactor = 0.0;
	const float fogType = fragFogColor.w;
	if ( fogType > -0.5 )
	{
		const float projZ = fragFogDepth.x;
		if ( fogType < 0.5 )
		{
			fogFactor = clamp(
			    min( fragFogParams.z, projZ * fragFogParams.w - fragFogParams.x ), 0.0, 1.0 );
		}
		else
		{
			const float worldZ = fragFogDepth.y;
			const float f = clamp( ( fragFogParams.y - worldZ ) / ( fragFogMisc.x - worldZ ), 0.0, 1.0 );
			fogFactor = clamp( f * projZ * fragFogParams.w, 0.0, 1.0 );
		}
	}
	result = 1.0 - ( ( 1.0 - result ) * pow( 1.0 - fogFactor, 4.0 ) );
	if ( ( flags & 4 ) != 0 )
		result = LinearToSrgb( result );
	return vec4( result, 1.0 );
}
// Portal 2's refract_ps2x.fxc with LOCALREFRACT and CUBEMAP (flag 1048576 with
// 262144): sampler 0 is the base texture, refracted in texture space. The
// tangent-space vertex-to-eye vector (vertex color) offsets the lookup by the
// normal map, scaled by the base texture's aspect (c7.xy) and
// $localrefractdepth (c7.z, in the modulation's alpha); the result is tinted by
// c1 (modulation rgb) and darkened by the normal's z cubed. The shader
// recomputes that eye vector per pixel; the interpolated one is normalized here.
// The cubemap term adds c0 tint, c2 contrast, c3 saturation (the output scale's
// slot) and Portal 2's fresnel. BLUR does not apply; alpha is the normal map's.
vec4 LocalRefract( int flags, vec4 normal )
{
	const vec3 normalTs = normal.xyz * 2.0 - 1.0;
	const vec3 eyeTs = normalize( fragVertexColor.rgb );
	const float rDotN = -eyeTs.z;
	vec2 refractedUv = eyeTs.xy / rDotN;
	refractedUv += normalTs.xy;
	refractedUv += ( 1.0 - normalTs.z ) * eyeTs.xy / rDotN;
	const vec2 size = vec2( textureSize( baseTexture, 0 ) );
	refractedUv *= vec2( size.y / size.x, 1.0 ) * consts.modulation.a;
	refractedUv += fragUv;
	vec4 refracted = texture( baseTexture, clamp( refractedUv, 0.0, 1.0 ) );
	const float mask = texture( baseTexture, clamp( fragUv + normalTs.xy * 0.1, 0.0, 1.0 ) ).a;
	if ( ( flags & 1 ) != 0 )
		refracted.rgb = SrgbToLinear( refracted.rgb );
	refracted.rgb = mix( refracted.rgb, vec3( mask ), 0.025 );
	vec3 color = refracted.rgb * pow( max( normalTs.z, 0.0 ), 3.0 ) * consts.modulation.rgb;

	vec3 worldNormal = normalize( fragReflection );
	vec3 worldTangent = normalize( fragEnvTint.xyz );
	vec3 worldBinormal = normalize( cross( worldNormal, worldTangent ) ) * fragEnvTint.w;
	vec3 bumpedWorldNormal = normalTs.x * worldTangent + normalTs.y * worldBinormal +
	    normalTs.z * worldNormal;
	// As refract_ps2x does, the world-space normal reflects the interpolated
	// tangent-space eye vector (CalcReflectionVectorUnnormalized's inputs).
	const vec3 eyeTangent = fragVertexColor.rgb;
	vec3 reflectDirection = 2.0 * dot( bumpedWorldNormal, eyeTangent ) * bumpedWorldNormal -
	    dot( bumpedWorldNormal, bumpedWorldNormal ) * eyeTangent;
	vec3 envTint = vec3( fragLightmapUv, fragVertexColor.a );
	vec3 reflection = texture( envmapTexture, reflectDirection ).rgb * normal.a * envTint;
	reflection = mix( reflection, reflection * reflection, consts.alphaParams.x );
	const float luminance = dot( reflection, vec3( 0.299, 0.587, 0.114 ) );
	reflection = mix( vec3( luminance ), reflection, consts.alphaParams.w );
	const float nDotV = clamp( dot( normalTs, eyeTangent ), 0.0, 1.0 );
	color += reflection * ( 0.6 + 0.4 * ( 1.0 - nDotV ) );
	color = ApplyPixelFog( color );
	if ( ( flags & 4 ) != 0 )
		color = LinearToSrgb( color );
	return vec4( color, normal.a );
}
// spritecard_ps2x.fxc (alphaParams.y == 3): the frame, blended with the second
// animation frame (lightmap-uv slot) by the frame blend (fragReflection.x) with
// ANIMBLEND, times fOverbrightFactor (the modulation), then either times the
// vertex color or, with ADDSELF (fragReflection.y = 1 + weight), premultiplied
// by alpha and brightened by weight times itself before the vertex color.
vec4 SpriteCardColor( int flags )
{
	vec4 frame0 = texture( baseTexture, fragUv );
	vec4 frame1 = texture( baseTexture, fragLightmapUv );
	if ( ( flags & 1 ) != 0 )
	{
		frame0.rgb = SrgbToLinear( frame0.rgb );
		frame1.rgb = SrgbToLinear( frame1.rgb );
	}
	vec4 blended = mix( frame0, frame1, fragReflection.x );
	blended.rgb *= fragModulation.rgb;
	if ( fragReflection.y > 0.5 )
	{
		const float addSelf = fragReflection.y - 1.0;
		blended.a *= fragVertexColor.a;
		blended.rgb *= blended.a;
		blended.rgb += fragModulation.r * addSelf * fragVertexColor.a * blended.rgb;
		blended.rgb *= fragVertexColor.rgb;
		return blended;
	}
	return blended * fragVertexColor;
}
void main()
{
	const int flags = int( consts.alphaParams.z );
	if ( consts.alphaParams.y > 1.5 && consts.alphaParams.y < 2.5 )
	{
		outColor = ShadowProjection( flags );
		return;
	}
	if ( ( flags & 262144 ) != 0 )
	{
		// refract_ps2x.fxc: sampler 0 is the completed frame copy, sampler 1
		// is the normal map, and sampler 2 is the local cubemap. The authored
		// glass uses BLUR=1, CUBEMAP=1 and no optional masks or second normal.
		vec4 normal = texture( lightmapTexture, fragUv );
		if ( ( flags & 1048576 ) != 0 )
		{
			outColor = LocalRefract( flags, normal );
			return;
		}
		vec2 warped = fragScreenUv + ( normal.xy * 2.0 - 1.0 ) *
		    ( normal.a * consts.modulation.a );
		vec3 color;
		if ( ( flags & 524288 ) != 0 )
		{
			color = vec3( 0.0 );
			for ( int y = -1; y <= 1; ++y )
				for ( int x = -1; x <= 1; ++x )
					color += texture( baseTexture, warped + vec2( x, y ) / 512.0 ).rgb;
			color /= 9.0;
		}
		else
			color = texture( baseTexture, warped ).rgb;
		if ( ( flags & 1 ) != 0 )
			color = SrgbToLinear( color );
		color *= consts.modulation.rgb;
		vec3 worldNormal = normalize( fragReflection );
		vec3 worldTangent = normalize( fragEnvTint.xyz );
		vec3 worldBinormal = normalize( cross( worldNormal, worldTangent ) ) * fragEnvTint.w;
		vec3 tangentNormal = normal.xyz * 2.0 - 1.0;
		vec3 bumpedWorldNormal = tangentNormal.x * worldTangent +
		    tangentNormal.y * worldBinormal + tangentNormal.z * worldNormal;
		vec3 eyeTangent = fragVertexColor.rgb;
		vec3 reflectDirection = 2.0 * dot( bumpedWorldNormal, eyeTangent ) *
		    bumpedWorldNormal - dot( bumpedWorldNormal, bumpedWorldNormal ) * eyeTangent;
		vec3 envTint = vec3( fragLightmapUv, fragVertexColor.a );
		vec3 reflection = texture( envmapTexture, reflectDirection ).rgb * normal.a * envTint;
		reflection = mix( reflection, reflection * reflection, consts.alphaParams.x );
		// $envmapsaturation (c3, in the output scale's slot).
		const float luminance = dot( reflection, vec3( 0.299, 0.587, 0.114 ) );
		reflection = mix( vec3( luminance ), reflection, consts.alphaParams.w );
		color += reflection;
		color = ApplyPixelFog( color );
		if ( ( flags & 4 ) != 0 )
			color = LinearToSrgb( color );
		outColor = vec4( color, normal.a );
		return;
	}
	vec4 base = ( flags & 128 ) != 0 ? fragVertexColor : texture( baseTexture, fragUv );
	if ( ( flags & 1 ) != 0 )
		base.rgb = SrgbToLinear( base.rgb );
	vec4 result;
	if ( consts.alphaParams.y > 2.5 )
	{
		result = SpriteCardColor( flags );
	}
	else if ( ( flags & 16 ) != 0 )
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
		if ( ( flags & ( 1024 | 65536 ) ) != 0 )
			result.a *= fragVertexColor.a;
		if ( consts.alphaParams.y > 0.5 )
		{
			vec3 lightmap = texture( lightmapTexture, fragLightmapUv ).rgb;
			if ( ( flags & 2 ) != 0 )
				lightmap = SrgbToLinear( lightmap );
			result.rgb *= lightmap;
		}
		if ( ( flags & 131072 ) != 0 )
		{
			// lightmappedgeneric_ps2_3_x.h: base alpha masks the cubemap
			// with its inverse, and is not also used as output alpha.
			float specularFactor = ( flags & 1048576 ) != 0 ? 1.0 - base.a : 1.0;
			if ( ( flags & 2097152 ) != 0 )
				specularFactor *= texture( normalMaskTexture, fragUv ).a;
			if ( ( flags & 1048576 ) != 0 )
				result.a = fragModulation.a;
			vec3 specular = texture( envmapTexture, fragReflection ).rgb *
			    fragVertexColor.rgb * specularFactor;
			specular = mix( specular, specular * specular, fragEnvTint.x );
			float grey = dot( specular, vec3( 0.299, 0.587, 0.114 ) );
			specular = mix( vec3( grey ), specular, fragEnvTint.y );
			result.rgb += specular * fragEnvTint.z;
		}
	}
	// The D3D9 fixed-function alpha test: GREATEREQUAL ($alphatest) or GREATER
	// (screenspace_general), against the reference. Disabled when < 0.
	if ( ( flags & 16384 ) == 0 && consts.alphaParams.x >= 0.0 &&
	     ( ( flags & 8 ) != 0 ? result.a <= consts.alphaParams.x
	                          : result.a < consts.alphaParams.x ) )
		discard;
	result.rgb *= consts.alphaParams.w;
	result.rgb = ApplyPixelFog( result.rgb );
	if ( ( flags & 4 ) != 0 )
		result.rgb = LinearToSrgb( result.rgb );
	outColor = result;
}
