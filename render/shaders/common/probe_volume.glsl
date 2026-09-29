// The probe volume (RFC 0011 render.probe-volume.v1; RFC 0016 render.lighting.v1,
// "Indirect diffuse, dynamic surfaces"): the PRBV sampler, a line-for-line port
// of mapcontainer/probe_volume.cpp (ProbeVolumeView::SampleGrid), which is its
// oracle (render.lab.probe-volume). The core's one copy; the native backend's
// materialsystem/shaderapivulkan/shaders/probe_volume.glsl is frozen and goes
// at K12.
//
// The grid records arrive as the table WriteProbeGridTable writes
// (public/mapcontainer/probe_volume.h), one row of six RGBA32F texels per
// grid; the atlas is the lump's RGBA16F atlas, sampled with clamped bilinear
// filtering, which matches the C++ Bilinear() texel-centre convention.
//
// The includer declares, in its own group and bindings:
//   texture2D probeAtlas;       the lump's atlas
//   sampler probeAtlasSampler;  linear, clamped to edge
//   texture2D probeGrids;       the grid table (texelFetch only)
//   sampler probeGridsSampler;  any (the table is only fetched)
// With PROBE_VOLUME_SECOND it also declares texture2D probeSecondAtlas, an
// atlas of the same layout (the runtime volume beside a change volume, or
// the reverse), sampled with the first atlas's weights and visibility by
// ProbeIrradiancePair.
//
// Units: irradiance / pi (the lightmap's diffuse light), Source units.

const float kProbeNormalBias = 0.1;
const float kProbeCrushThreshold = 0.2;

vec2 ProbeOctEncode( vec3 d )
{
	d /= abs( d.x ) + abs( d.y ) + abs( d.z );
	vec2 p = d.xy;
	if ( d.z < 0.0 )
		p = ( vec2( 1.0 ) - abs( p.yx ) ) *
		    vec2( p.x >= 0.0 ? 1.0 : -1.0, p.y >= 0.0 ? 1.0 : -1.0 );
	return p;
}

vec2 ProbeTileTexel( vec2 origin, float tile, uint probe, uint tilesPerRow, vec3 direction )
{
	const vec2 oct = ProbeOctEncode( direction );
	const float interior = tile - 2.0;
	const vec2 corner =
	    origin + vec2( float( probe % tilesPerRow ), float( probe / tilesPerRow ) ) * tile;
	return corner + 1.0 + ( oct * 0.5 + 0.5 ) * interior;
}

vec4 ProbeAtlasSample( vec2 texel )
{
	return textureLod( sampler2D( probeAtlas, probeAtlasSampler ),
	    texel / vec2( textureSize( sampler2D( probeAtlas, probeAtlasSampler ), 0 ) ), 0.0 );
}

#ifdef PROBE_VOLUME_SECOND
vec4 ProbeSecondAtlasSample( vec2 texel )
{
	return textureLod( sampler2D( probeSecondAtlas, probeAtlasSampler ),
	    texel / vec2( textureSize( sampler2D( probeSecondAtlas, probeAtlasSampler ), 0 ) ), 0.0 );
}
#endif

vec4 ProbeTile( vec2 origin, float tile, uint probe, uint tilesPerRow, vec3 direction )
{
	return ProbeAtlasSample( ProbeTileTexel( origin, tile, probe, tilesPerRow, direction ) );
}

vec4 ProbeGridRow( int texel, int grid )
{
	return texelFetch( sampler2D( probeGrids, probeGridsSampler ), ivec2( texel, grid ), 0 );
}

// Irradiance / pi from `layer` at a surface point with unit normal `normal`;
// false outside grid `g`. `second` is the same from probeSecondAtlas
// (PROBE_VOLUME_SECOND), else 0.
bool ProbeSampleGridPair( int g, vec3 position, vec3 normal, int layer, bool useVisibility,
    out vec3 result, out vec3 second )
{
	// Zero outside the grid (the native copy left the out parameters
	// undefined there, so ProbeIrradiance's zero was not kept).
	result = vec3( 0.0 );
	second = vec3( 0.0 );
	const vec4 row0 = ProbeGridRow( 0, g );
	const vec4 row1 = ProbeGridRow( 1, g );
	const vec4 row2 = ProbeGridRow( 2, g );
	const vec4 row3 = ProbeGridRow( 3, g );
	const vec4 row4 = ProbeGridRow( 4, g );
	const vec3 origin = row0.xyz;
	const uint tilesPerRow = uint( row0.w );
	const vec3 spacing = row1.xyz;
	const float maxDistance = row1.w;
	const vec3 dimsF = row2.xyz;
	const uvec3 dims = uvec3( dimsF );
	const vec2 irradianceOrigin = layer == 0 ? row3.xy : row3.zw;
	const vec2 visibilityOrigin = row4.xy;
	const uvec2 stateOrigin = uvec2( row4.zw );

#ifdef SEEDED_PROBE_NO_NORMAL_BIAS
	const vec3 biased = position;
#else
	const vec3 biased = position + normal * kProbeNormalBias * row2.w;
#endif
	const vec3 g3 = ( biased - origin ) / spacing;
	if ( any( lessThan( g3, vec3( 0.0 ) ) ) || any( greaterThan( g3, dimsF - 1.0 ) ) )
		return false;
	const ivec3 base = min( ivec3( floor( g3 ) ), ivec3( dims ) - 2 );
	const vec3 alpha = g3 - vec3( base );
	const uint stateRow = tilesPerRow * 16u;
	vec3 total = vec3( 0.0 );
	vec3 totalSecond = vec3( 0.0 );
	float weights = 0.0;
	for ( int corner = 0; corner < 8; ++corner )
	{
		const ivec3 bits = ivec3( corner & 1, ( corner >> 1 ) & 1, ( corner >> 2 ) & 1 );
		const ivec3 index3 = base + bits;
		const vec3 t = mix( vec3( 1.0 ) - alpha, alpha, vec3( bits ) );
		const float trilinear = t.x * t.y * t.z;
		const uint probe =
		    uint( index3.x ) + dims.x * ( uint( index3.y ) + dims.y * uint( index3.z ) );
		const vec4 state = texelFetch( sampler2D( probeAtlas, probeAtlasSampler ),
		    ivec2( stateOrigin.x + probe % stateRow, stateOrigin.y + probe / stateRow ), 0 );
#ifndef SEEDED_PROBE_STATE_IGNORED
		if ( state.w < 0.5 )
			continue;
#endif
		const vec3 probePosition = origin + vec3( index3 ) * spacing + state.xyz;
		const vec3 toProbe = probePosition - position;
		const float toProbeLength = sqrt( dot( toProbe, toProbe ) );
		const float dotDirection =
		    toProbeLength > 1.0e-9 ? dot( toProbe, normal ) / toProbeLength : 1.0;
		const float backface = ( dotDirection + 1.0 ) * 0.5;
		float weight = backface * backface + 0.2;
#ifndef SEEDED_PROBE_VISIBILITY_IGNORED
		if ( useVisibility )
		{
			const vec3 toPoint = biased - probePosition;
			const float distance = sqrt( dot( toPoint, toPoint ) );
			if ( distance > 1.0e-9 )
			{
				const vec4 moments =
				    ProbeTile( visibilityOrigin, 16.0, probe, tilesPerRow, toPoint / distance );
				// Moments are stored as fractions of the max distance.
				const float mean = moments.x * maxDistance;
				const float meanSquared = moments.y * maxDistance * maxDistance;
				if ( distance > mean )
				{
					const float variance = abs( mean * mean - meanSquared );
					const float excess = distance - mean;
					const float chebyshev = variance / ( variance + excess * excess );
					weight *= chebyshev * chebyshev * chebyshev;
				}
			}
		}
#endif
		weight = max( weight, 1.0e-6 );
#ifndef SEEDED_PROBE_NO_CRUSH
		if ( weight < kProbeCrushThreshold )
			weight *= weight * weight / ( kProbeCrushThreshold * kProbeCrushThreshold );
#endif
		weight *= trilinear;
		const vec2 texel = ProbeTileTexel( irradianceOrigin, 8.0, probe, tilesPerRow, normal );
		total += weight * ProbeAtlasSample( texel ).rgb;
#ifdef PROBE_VOLUME_SECOND
		totalSecond += weight * ProbeSecondAtlasSample( texel ).rgb;
#endif
		weights += weight;
	}
	result = weights > 0.0 ? total / weights : vec3( 0.0 );
	second = weights > 0.0 ? totalSecond / weights : vec3( 0.0 );
	return true;
}

bool ProbeSampleGrid(
    int g, vec3 position, vec3 normal, int layer, bool useVisibility, out vec3 result )
{
	vec3 unused;
	return ProbeSampleGridPair( g, position, normal, layer, useVisibility, result, unused );
}

// The first grid holding the point; false outside every grid, or when the
// volume lacks `layer`.
bool ProbeIrradiance( vec3 position, vec3 normal, int layer, bool useVisibility, out vec3 result )
{
	const vec4 counts = ProbeGridRow( 5, 0 );
	result = vec3( 0.0 );
	if ( float( layer ) >= counts.x )
		return false;
	const int grids = int( counts.y );
	for ( int g = 0; g < grids; ++g )
	{
		if ( ProbeSampleGrid( g, position, normal, layer, useVisibility, result ) )
			return true;
	}
	return false;
}

#ifdef PROBE_VOLUME_SECOND
// ProbeIrradiance from both atlases with the first's weights.
bool ProbeIrradiancePair( vec3 position, vec3 normal, int layer, bool useVisibility,
    out vec3 result, out vec3 second )
{
	const vec4 counts = ProbeGridRow( 5, 0 );
	result = vec3( 0.0 );
	second = vec3( 0.0 );
	if ( float( layer ) >= counts.x )
		return false;
	const int grids = int( counts.y );
	for ( int g = 0; g < grids; ++g )
	{
		if ( ProbeSampleGridPair( g, position, normal, layer, useVisibility, result, second ) )
			return true;
	}
	return false;
}
#endif
