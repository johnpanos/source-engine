// The vertex stage VertexLitGeneric's cloak and weapon sheen passes share
// (stdshaders/cloak_blended_pass_vs20.fxc and weapon_sheen_pass_vs20.fxc, whose
// bodies are the same but for the sheen's model-space position): the
// normalized world normal, the frame-buffer coordinates of the position, the
// world view vector and, with BUMPMAP, the tangent frame and the bump texture
// coordinate. Include after legacy_vs.glsl.
#ifndef LEGACY_BLENDED_PASS_VS_GLSL
#define LEGACY_BLENDED_PASS_VS_GLSL

layout( location = 0 ) out vec3 vWorldNormal;
layout( location = 1 ) out vec3 vProjPosForRefract;
layout( location = 2 ) out vec3 vWorldViewVector;
layout( location = 3 ) out vec3 mTangentSpaceTranspose0;
layout( location = 4 ) out vec3 mTangentSpaceTranspose1;
layout( location = 5 ) out vec3 mTangentSpaceTranspose2;
layout( location = 6 ) out vec2 vTexCoord0;

#define cBaseTexCoordTransform_0 VS_C( 48 )
#define cBaseTexCoordTransform_1 VS_C( 49 )

// bTangentFrame: BUMPMAP (SkinPositionNormalAndTangentSpace); without it the
// tangent and binormal stay zero, as the HLSL initializes them.
void BlendedPassVertex( bool bTangentFrame )
{
	const vec3 vWorldPosition = inWorldPos;
	const vec3 worldNormal = inWorldNormal;
	vec3 vWorldTangent = vec3( 0.0 );
	vec3 vWorldBinormal = vec3( 0.0 );
	if ( bTangentFrame )
	{
		vWorldTangent = inWorldTangentS.xyz;
		vWorldBinormal = cross( worldNormal, vWorldTangent ) * LegacyTangentSign();
	}
	vWorldNormal = normalize( worldNormal );

	const vec4 vProjPosition = LegacyProject( vWorldPosition );
	gl_Position = vProjPosition;

	vec2 vRefractPos = vec2( vProjPosition.x, -vProjPosition.y ); // Invert Y
	vRefractPos = ( vRefractPos + vProjPosition.w ) * 0.5;
	vProjPosForRefract = vec3( vRefractPos, vProjPosition.w );

	vWorldViewVector = normalize( vWorldPosition - cEyePos );

	mTangentSpaceTranspose0 = vec3( vWorldTangent.x, vWorldBinormal.x, worldNormal.x );
	mTangentSpaceTranspose1 = vec3( vWorldTangent.y, vWorldBinormal.y, worldNormal.y );
	mTangentSpaceTranspose2 = vec3( vWorldTangent.z, vWorldBinormal.z, worldNormal.z );

	// dot( i.vTexCoord0.xy, cBaseTexCoordTransform[n] ): the float2 dot product.
	vTexCoord0 = vec2( dot( inTexCoord0, cBaseTexCoordTransform_0.xy ),
	    dot( inTexCoord0, cBaseTexCoordTransform_1.xy ) );
}

#endif // LEGACY_BLENDED_PASS_VS_GLSL
