#version 450
// Faithful UnlitGeneric pixel stage (matches stdshaders/unlitgeneric_ps2x.fxc):
//   result = i.vColor0 * tex2D( TextureSampler, i.vTexCoord0 )
// where vColor0 is cModulationColor ($color * $alpha) interpolated from the
// vertex stage. This is the exact D3D9 UnlitGeneric material output (before the
// FinalOutput sRGB/fog/tonemap stage, which is identity for an opaque, fog-off,
// linear material -- the equivalence oracle drives that case).
layout( location = 0 ) in vec2 fragUv;
layout( location = 1 ) in vec4 fragModulation;
layout( location = 0 ) out vec4 outColor;
layout( binding = 0 ) uniform sampler2D baseTexture;
layout( push_constant ) uniform Constants
{
	mat4 mvp;
	vec4 modulation;
	vec4 texXform0;
	vec4 texXform1;
	vec4 alphaParams; // x = alpha-test reference (<0 disables)
}
consts;
void main()
{
	vec4 result = fragModulation * texture( baseTexture, fragUv );
	// $alphatest: discard fragments below the reference alpha, matching the D3D9
	// fixed-function alpha test (ALPHAFUNC = GREATEREQUAL). Disabled when < 0.
	if ( consts.alphaParams.x >= 0.0 && result.a < consts.alphaParams.x )
		discard;
	outColor = result;
}
