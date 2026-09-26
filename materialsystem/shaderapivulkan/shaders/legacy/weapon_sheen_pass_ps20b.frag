#version 450
// VertexLitGeneric's $sheenpassenabled pass: a port of
// stdshaders/weapon_sheen_pass_ps2x.fxc (ps20b): an env-map sheen masked by a
// texture projected along a model-space axis. Combos
// (fxctmp9/weapon_sheen_pass_ps20b.inc): static CONVERT_TO_SRGB (1, always 0
// here), BUMPMAP (2). The shader reads neither its refraction sampler (s0) nor
// its bump map (s1): the bumped normal and effect 2's samples are unused, and
// fxc removes them.
// @legacy program=weapon_sheen_pass ps=weapon_sheen_pass_ps20b
//         vs=weapon_sheen_pass_vs20 vert=weapon_sheen_pass_vs20
//         samplers=2:cube,3:2d flags=object_position_extra
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform samplerCube EnvmapSampler;   // s2
layout( set = 0, binding = 1 ) uniform sampler2D EnvmapMaskSampler; // s3

layout( location = 0 ) in vec3 vWorldNormalIn;
layout( location = 1 ) in vec3 vProjPosForRefract;
layout( location = 2 ) in vec3 vWorldViewVector;
layout( location = 3 ) in vec3 mTangentSpaceTranspose0;
layout( location = 4 ) in vec3 mTangentSpaceTranspose1;
layout( location = 5 ) in vec3 mTangentSpaceTranspose2;
layout( location = 6 ) in vec2 vTexCoord0;
layout( location = 7 ) in vec4 vModelSpacePos;

#define g_flSheenMapMaskScaleX ( PS_C( 6 ).x )
#define g_flSheenMapMaskScaleY ( PS_C( 6 ).y )
#define g_flSheenMapMaskOffsetX ( PS_C( 6 ).z )
#define g_flSheenMapMaskOffsetY ( PS_C( 6 ).w )
#define g_flSheenDirection ( PS_C( 7 ).x )
#define g_flEffectIndex ( PS_C( 7 ).y )
#define g_cCloakColorTint PS_C( 8 )

// The mask coordinate: two model-space axes by $sheenmapmaskdirection, offset
// and scaled, y flipped.
vec2 SheenMaskCoord()
{
	const vec3 ppos = vModelSpacePos.xyz;
	vec2 temp;
	if ( g_flSheenDirection == 0.0 )
		temp = vec2( ppos.z, ppos.y );
	else if ( g_flSheenDirection == 1.0 )
		temp = vec2( ppos.z, ppos.x );
	else
		temp = vec2( ppos.y, ppos.x );
	temp.x -= g_flSheenMapMaskOffsetX;
	temp.y -= g_flSheenMapMaskOffsetY;
	temp.x /= g_flSheenMapMaskScaleX;
	temp.y /= g_flSheenMapMaskScaleY;
	temp.y = 1.0 - temp.y;
	return temp;
}

void main()
{
	vec4 result;
	if ( g_flEffectIndex == 2.0 )
	{
		result = vec4( 0.0 );
	}
	else
	{
		const vec3 vEyeDir = -normalize( vWorldViewVector );
		// mul( mTangentSpaceTranspose, float3( 0, 0, 1 ) ): its z column.
		const vec3 worldSpaceNormal = normalize(
		    vec3( mTangentSpaceTranspose0.z, mTangentSpaceTranspose1.z, mTangentSpaceTranspose2.z ) );
		const vec3 vReflect = 2.0 * worldSpaceNormal * dot( worldSpaceNormal, vEyeDir ) - vEyeDir;

		vec3 envMapColor = ENV_MAP_SCALE * texCUBE( 0, EnvmapSampler, vReflect ).rgb * g_cCloakColorTint.xyz;
		envMapColor *= 10.0;

		const vec4 envmapMaskTexel = tex2D( 1, EnvmapMaskSampler, SheenMaskCoord() );
		const float alpha = max( max( envMapColor.x, envMapColor.y ), envMapColor.z );
		result = vec4( envMapColor * envmapMaskTexel.xyz, alpha * envmapMaskTexel.x );
		if ( g_flEffectIndex == 1.0 )
			result *= 1.8;
	}

	LegacyWrite( FinalOutput( result, 0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE ) );
}
