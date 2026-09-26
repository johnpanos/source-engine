#version 450
// A port of stdshaders/portalstaticoverlay_vs20.fxc (PortalStaticOverlay's
// vertex stage). Combos (fxctmp9/portalstaticoverlay_vs20.inc): static MODEL
// (2); dynamic SKINNING (1, applied by the shader API's vertex record). A
// model's position is skinned; a brush's is mul( v.vPos.xyz, cModel[0] ),
// which fxc compiles to the rotation rows alone (no translation), so the
// record's MODEL translation (cModel[0]'s w column) is taken back out.
#include "legacy_vs.glsl"

layout( location = 0 ) out vec2 vTexCoord1;
layout( location = 1 ) out vec2 vTexCoord2;
layout( location = 7 ) out vec4 worldPos_projPosZ;

#define cModel0_0 VS_C( 58 )
#define cModel0_1 VS_C( 59 )
#define cModel0_2 VS_C( 60 )

void main()
{
	const bool MODEL = STATIC_VS_COMBO( 2, 2 ) != 0;

	vec3 worldPos = inWorldPos;
	if ( !MODEL )
		worldPos -= vec3( cModel0_0.w, cModel0_1.w, cModel0_2.w );

	const vec4 vProjPos = LegacyProject( worldPos );
	gl_Position = vProjPos;
	worldPos_projPosZ = vec4( worldPos, vProjPos.z );
	vTexCoord1 = inTexCoord0;
	vTexCoord2 = inTexCoord0;
}
