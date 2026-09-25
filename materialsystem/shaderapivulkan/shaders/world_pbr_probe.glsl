// Shared by world_pbr.frag, world_pbr_glass.frag and model_pbr.frag: the map
// reflection probes and the tangent-space normal. The includer declares
// `lightmapTexture` (frame set binding 1), the map's LMAP atlas, whose top
// rows carry a map built before RPRB's one direction-only probe; the map's
// RPRB probes are frame set binding 5 (R50-PARALLAX, reflection_probes.glsl).
// An includer that defines REFLECTION_PROBE_RELIGHT before including this
// file defines ReflectionProbeDiffuseChange (R50-RELIGHT; world_pbr.frag).

#include "pbr_brdf.glsl"

// The map's RPRB texture, or the built-in 2D texture when it has none (its
// first texel lacks the marker, so ReflectionProbesRadiance declines).
layout( set = 0, binding = 5 ) uniform sampler2D reflectionProbes;

vec4 ReflectionProbesFetch( ivec2 texel )
{
	return texelFetch( reflectionProbes, texel, 0 );
}

vec4 ReflectionProbesSample( vec2 texel )
{
	return textureLod( reflectionProbes, texel / vec2( textureSize( reflectionProbes, 0 ) ), 0.0 );
}

#include "reflection_probes.glsl"

// Reflection probe packed into the LMAP atlas's top rows by the map pipeline
// (tools/quality/reflection_probe.py): equirect mips side by side from x = 0,
// and a marker texel (mip count, mip-0 width, band rows, -1) at the atlas's
// top-right corner. Real lightmap texels never have negative alpha.
vec3 SampleProbeLevel( vec2 uv, float level, float width0, vec2 atlasSize )
{
	float width = width0 * exp2( -level );
	vec2 extent = vec2( width, width * 0.5 );
	float left = 2.0 * width0 * ( 1.0 - exp2( -level ) );
	vec2 texel = vec2( left, 0.0 ) + clamp( uv * extent, vec2( 0.5 ), extent - vec2( 0.5 ) );
	return textureLod( lightmapTexture, texel / atlasSize, 0.0 ).rgb;
}

bool ProbeRadiance( vec3 direction, float roughness, out vec3 radiance )
{
	ivec2 size = textureSize( lightmapTexture, 0 );
	vec4 marker = texelFetch( lightmapTexture, ivec2( size.x - 1, 0 ), 0 );
	radiance = vec3( 0.0 );
	if ( marker.a > -0.5 || marker.r < 1.0 )
		return false;
	float lod = clamp( roughness, 0.0, 1.0 ) * ( marker.r - 1.0 );
	float lower = floor( lod );
	float upper = min( lower + 1.0, marker.r - 1.0 );
	vec2 uv = vec2( 0.5 - atan( direction.y, direction.x ) / ( 2.0 * kPi ),
	    0.5 - asin( clamp( direction.z, -1.0, 1.0 ) ) / kPi );
	radiance = mix( SampleProbeLevel( uv, lower, marker.g, vec2( size ) ),
	    SampleProbeLevel( uv, upper, marker.g, vec2( size ) ), lod - lower );
	return true;
}

// The map's specular image light at `position` (geometric normal `normal`)
// along the unit ray `direction`: its blended, parallax-corrected RPRB
// probes, else the LMAP band's direction-only probe of an older map.
bool MapProbeRadiance(
    vec3 position, vec3 normal, vec3 direction, float roughness, out vec3 radiance )
{
	if ( ReflectionProbesRadiance( position, normal, direction, roughness, radiance ) )
		return true;
	return ProbeRadiance( direction, roughness, radiance );
}

// The WMSH interpolated frame applied to a two-channel tangent-space normal
// texel (x, y in 0..1).
vec3 MappedNormal( vec3 normal, vec4 tangent, vec2 texel )
{
	vec3 t = normalize( tangent.xyz - normal * dot( normal, tangent.xyz ) );
	vec3 bitangent = cross( normal, t ) * tangent.w;
	vec2 xy = texel * 2.0 - 1.0;
	vec3 mapped = vec3( xy, sqrt( max( 0.0, 1.0 - dot( xy, xy ) ) ) );
	return normalize( mat3( t, bitangent, normal ) * mapped );
}
