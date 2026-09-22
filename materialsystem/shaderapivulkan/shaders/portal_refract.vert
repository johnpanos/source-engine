#version 450
// PortalRefract's vertex stage: a line-for-line port of
// stdshaders/portal_refract_vs20.fxc (all three STAGE combos share it).
//
// The push block holds its constants in their registers' terms:
//   model, viewProj   cModel[0] and cViewProj (laid out like demo_dyn.vert's mvp)
//   texXform0/1       cBaseTexCoordTransform, SHADER_SPECIFIC_CONST_1/2
//   params.x          g_flTime, SHADER_SPECIFIC_CONST_0.x (already mod 1000)
//   params.y          g_flPortalOpenAmount, SHADER_SPECIFIC_CONST_3.x (= ps c4.x)
//   params.z, .w      ps c4.y (portal active) and c4.z (color scale)
//   alphaParams       see portal_refract.frag
//   clipPlanes        D3D9 user clip planes in clip space (0 = unused)
layout( location = 0 ) in vec3 inPos;
layout( location = 2 ) in vec2 inUv;
layout( location = 4 ) in vec3 inNormal;
layout( location = 5 ) in vec4 inTangent; // TANGENT: flip in w
layout( location = 0 ) out vec2 vUv0;
layout( location = 1 ) out vec3 vWorldTangent;
layout( location = 2 ) out vec3 vWorldBinormal;
layout( location = 3 ) out vec4 vWorldPosition; // proj pos z in w
layout( location = 4 ) out vec3 vProjPosForRefract;
layout( location = 5 ) out vec4 vNoiseTexCoord;
layout( push_constant ) uniform Constants
{
	mat4 model;
	mat4 viewProj;
	vec4 texXform0;
	vec4 texXform1;
	vec4 params;
	vec4 alphaParams;
	vec4 clipPlanes[2];
}
consts;
out float gl_ClipDistance[2];
void main()
{
	const float kFlPortalOuterBorder = 0.075; // Must match PS!

	// DecompressVertex_Normal: uncompressed normals pass through.
	vec3 vObjNormal = inNormal;

	// mul( v, (float3x3)cModel[0] ): the rotation part of the model matrix.
	const mat3 model3 = mat3( consts.model );
	vec3 worldPosition = ( consts.model * vec4( inPos, 1.0 ) ).xyz;
	vec3 worldNormal = normalize( model3 * vObjNormal );
	vec3 worldTangent = normalize( model3 * inTangent.xyz );
	vec3 worldBinormal = normalize( cross( worldNormal, worldTangent ) * inTangent.w );

	vWorldPosition.xyz = worldPosition;
	vWorldTangent = worldTangent + worldPosition;
	vWorldBinormal = worldBinormal + worldPosition;

	// Transform into projection space
	vec4 projPosition = consts.viewProj * vec4( worldPosition, 1.0 );
	gl_Position = projPosition;
	vWorldPosition.w = projPosition.z;

	// Map projected position to the refraction texture
	vec2 refractPos;
	refractPos.x = projPosition.x;
	refractPos.y = -projPosition.y; // Invert Y
	refractPos = ( refractPos + projPosition.w ) * 0.5;
	vProjPosForRefract = vec3( refractPos.x, refractPos.y, projPosition.w );

	// dot( float2, float4 ) in HLSL uses the first two components: the transform's
	// translation column is not applied.
	vec2 baseUv;
	baseUv.x = dot( inUv, consts.texXform0.xy );
	baseUv.y = dot( inUv, consts.texXform1.xy );
	vUv0 = baseUv * ( 1.0 + kFlPortalOuterBorder ) - ( kFlPortalOuterBorder * 0.5 );

	// Portal open time
	float portalOpenAmount = clamp( consts.params.y + 0.001, 0.0, 1.0 );

	// Noise UV
	const float kFlBorderNoiseScale = 0.3;
	float noiseUvScroll = consts.params.x * 0.0275;
	vec2 noiseUv = ( ( baseUv - 0.5 ) / portalOpenAmount ) + 0.5;
	vNoiseTexCoord.xy = ( noiseUv * kFlBorderNoiseScale ) + vec2( noiseUvScroll, 0.0 );
	vNoiseTexCoord.zw = ( noiseUv * kFlBorderNoiseScale ) - vec2( noiseUvScroll, 0.0 );

	gl_ClipDistance[0] = dot( consts.clipPlanes[0], gl_Position );
	gl_ClipDistance[1] = dot( consts.clipPlanes[1], gl_Position );
}
