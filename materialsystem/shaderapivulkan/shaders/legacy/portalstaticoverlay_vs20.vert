#version 450
// A port of stdshaders/portalstaticoverlay_vs20.fxc (PortalStaticOverlay's
// vertex stage). Combos (fxctmp9/portalstaticoverlay_vs20.inc): static MODEL
// (2), PORTALGHOSTOVERLAY (4, 0..2); dynamic SKINNING (1, applied by the
// shader API's vertex record). Portal 2's ghost moves the portal one unit off
// the wall (a reverse z-test then draws it only where it is hidden) and passes
// its tint and fade in COLOR. A
// model's position is skinned; a brush's is mul( v.vPos.xyz, cModel[0] ),
// which fxc compiles to the rotation rows alone (no translation), so the
// record's MODEL translation (cModel[0]'s w column) is taken back out.
#include "legacy_vs.glsl"

layout( location = 0 ) out vec2 vTexCoord1;
layout( location = 1 ) out vec2 vTexCoord2;
layout( location = 7 ) out vec4 worldPos_projPosZ;
layout( location = 8 ) out vec4 vColor; // COLOR0, the ghost's

#define cModel0_0 VS_C( 58 )
#define cModel0_1 VS_C( 59 )
#define cModel0_2 VS_C( 60 )

void main()
{
	const bool MODEL = STATIC_VS_COMBO( 2, 2 ) != 0;
	const int PORTALGHOSTOVERLAY = STATIC_VS_COMBO( 4, 3 );

	vec3 worldPos = inWorldPos;
	if ( !MODEL )
		worldPos -= vec3( cModel0_0.w, cModel0_1.w, cModel0_2.w );
	if ( PORTALGHOSTOVERLAY != 0 )
		worldPos += inWorldNormal;

	const vec4 vProjPos = LegacyProject( worldPos );
	gl_Position = vProjPos;
	worldPos_projPosZ = vec4( worldPos, vProjPos.z );
	vTexCoord1 = inTexCoord0;
	vTexCoord2 = inTexCoord0;

	vColor = vec4( 1.0 );
	if ( PORTALGHOSTOVERLAY != 0 )
	{
		if ( PORTALGHOSTOVERLAY == 1 )
			vColor.rgb = inColor;
		// Facing the viewer, the ghost fades in from 120 to 240 units away.
		const vec3 vViewRayWs = worldPos - cEyePos;
		if ( dot( inWorldNormal, vViewRayWs ) <= 0.0 )
		{
			const float flDistSqr = dot( vViewRayWs, vViewRayWs );
			vColor.a = clamp( ( flDistSqr - 120.0 * 120.0 ) / ( 240.0 * 240.0 - 120.0 * 120.0 ), 0.0, 1.0 );
		}
		vColor.a *= inAlpha; // how open the portal is
	}
}
