// The one full SurfaceConstants layout shared by the surface stages.
layout( set = 2, binding = 0 ) uniform Material
{
	vec4 tint;  // rgb: $color, a: $alpha
	vec4 flags; // x: $vertexcolor, y: $alphatest, z: $alphatestreference
	// x: 1 when fully opaque (height fog's factor is the output alpha), y:
	// gamma vertex color (vertex stage), z: the ssbump weights' scale
	// (0.57735 with $ssbumpmathfix, else 1), w: $vertexalpha (the unlit point)
	vec4 state;
	vec4 envTint;       // rgb: $envmaptint, a: $fresnelreflection
	vec4 envContrast;   // rgb: the contrast in effect, a: 1 - $fresnelreflection
	vec4 envSaturation; // legacy saturation; PBR rgb: probe tint, a: inverted Phong mask
	vec4 selfIllumTint; // rgb: $selfillumtint
	vec4 detailTint;    // rgb: $detailtint, a: $detailblendfactor
	vec4 detailScale;   // xy: $detailscale
	vec4 envLightScale; // x: min, y: min + max, z: $envmaplightscale (Portal 2)
	vec4 emission;      // x: $emissionscale, y: $emissiononesided (the pbr point)
	// The water point (SurfaceConstants::water*, water_family.h).
	vec4 waterFlow;      // 1/$flow_worlduvscale, 1/$flow_normaluvscale, bump strength, displacement
	vec4 waterFlowTime;  // interval, scroll distance, noise scale, 1 with a flow map
	vec4 waterColorFlow; // 1/$color_flow_uvscale, interval, scroll distance, lerp exponent
	vec4 waterReflect;   // rgb: $reflecttint (linear); a: $waterblendfactor
	vec4 waterFog;       // rgb: $fogcolor (linear); a: $reflectamount
	vec4 waterMode;      // x: reflection target, y: sludge, z: $lightmapwaterfog, w: $forcefresnel
	vec4 pbrFactors;     // rgb: metalness, roughness, AO without MRAO texture
	vec4 meshControls;   // VertexLit mesh: base-alpha Phong mask, albedo tint, rim boost/exponent
	vec4 meshProbeMasks; // VertexLit mesh: base-alpha and normal-alpha probe masks
	vec4 meshProbeFresnel; // VertexLit mesh: env Fresnel weight and min/max/exp
	vec4 meshModes; // light warp, authored env map, Phong enable
	vec4 meshProbeColor; // contrast, saturation, Phong warp, RGB probe mask
	vec4 transmission; // thin: fraction/IOR; Refract: amount/blur/mode/fade
	vec4 emissionCone; // inner/outer cosine, exponent, enabled
	vec4 texture2Transform[2];
	vec4 surfaceControls;
	vec4 selfIllumFresnel; // $selfillumfresnelminmaxexp; w: 1 with $selfillumfresnel
	vec4 treeGeometry;
	vec4 treeMotion;
	vec4 treeCurves;
	vec4 treeWind;
	vec4 baseTransform[2];
} material;
