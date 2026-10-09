// Shared by world_pbr.frag, world_pbr_glass.frag and model_pbr.frag: the map
// reflection probes and the tangent-space normal. `lightmapTexture` (frame set
// binding 1) is the LMAP atlas, whose top rows carry an older map's one
// direction-only probe. The RPRB v8 probes are frame binding 5 (the radiance
// cube array) and 7 (the probe words, an R32_UINT texture). No relight here.

#include "../../../render/shaders/common/pbr_brdf.glsl"

// Neutral inputs (a count of 0) when the map has no probes.
layout( set = 0, binding = 5 ) uniform samplerCubeArray reflectionProbes;
layout( set = 0, binding = 7 ) uniform usampler2D reflectionTable;

uint ReflectionProbesWord( uint index )
{
	const ivec2 size = textureSize( reflectionTable, 0 );
	const ivec2 texel = ivec2( int( index % uint( size.x ) ), int( index / uint( size.x ) ) );
	return texel.y < size.y ? texelFetch( reflectionTable, texel, 0 ).x : 0u;
}

vec4 ReflectionProbesRadianceFetch( vec3 direction, float layer, float lod )
{
	return textureLod( reflectionProbes, vec4( direction, layer ), lod );
}

#include "../../../render/shaders/common/reflection_probes.glsl"

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
