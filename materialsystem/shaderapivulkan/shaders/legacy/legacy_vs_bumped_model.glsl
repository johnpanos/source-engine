// common_vs_fxc.h helpers the bumped model vertex shaders share (skin_vs20,
// vertexlit_and_unlit_generic_bump_vs20): the tangent frame
// SkinPositionNormalAndTangentSpace leaves (GetVertexAttenForLight, the
// per-vertex light attenuation their pixel shaders light with, is in
// legacy_vs_lighting.glsl). Include after legacy_vs_lighting.glsl.
#ifndef LEGACY_VS_BUMPED_MODEL_GLSL
#define LEGACY_VS_BUMPED_MODEL_GLSL

// The world-space tangent frame of a model vertex: the record's normal and
// TANGENT (user data) stream arrive skinned or through the MODEL matrix and not
// normalized; tangent T is cross( N, S ) times the TANGENT w flip, as
// SkinPositionNormalAndTangentSpace computes it before the vertex shaders
// normalize all three ("always normalize since flex path is controlled by
// runtime constant").
void LegacyModelTangentFrame( out vec3 worldNormal, out vec3 worldTangentS, out vec3 worldTangentT )
{
	worldNormal = inWorldNormal;
	worldTangentS = inWorldTangentS.xyz;
	worldTangentT = cross( worldNormal, worldTangentS ) * LegacyTangentSign();
	worldNormal = normalize( worldNormal );
	worldTangentS = normalize( worldTangentS );
	worldTangentT = normalize( worldTangentT );
}

#endif // LEGACY_VS_BUMPED_MODEL_GLSL
