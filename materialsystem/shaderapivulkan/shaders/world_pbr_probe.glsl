// Shared by world_pbr.frag and world_pbr_glass.frag: the map reflection probe
// and the tangent-space normal. The includer declares `lightmapTexture`, the
// map's LMAP atlas, whose top rows carry the probe.

const float kPi = 3.14159265358979323846;

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
