// RFC 0011 render.probe-volume.v1: the PRBV sampler, a line-for-line port of
// mapcontainer/probe_volume.cpp (ProbeVolumeView::SampleGrid), which is its
// oracle. The grid records arrive as the table WriteProbeGridTable writes
// (public/mapcontainer/probe_volume.h), one row of six RGBA32F texels per
// grid; the atlas is the lump's RGBA16F atlas, sampled with clamped bilinear
// filtering, which matches the C++ Bilinear() texel-centre convention.
//
// The includer declares probeAtlas and probeGrids.

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

vec4 ProbeTile( vec2 origin, float tile, uint probe, uint tilesPerRow, vec3 direction )
{
	const vec2 oct = ProbeOctEncode( direction );
	const float interior = tile - 2.0;
	const vec2 corner =
	    origin + vec2( float( probe % tilesPerRow ), float( probe / tilesPerRow ) ) * tile;
	const vec2 texel = corner + 1.0 + ( oct * 0.5 + 0.5 ) * interior;
	return textureLod( probeAtlas, texel / vec2( textureSize( probeAtlas, 0 ) ), 0.0 );
}

// Irradiance / pi from `layer` at a surface point; false outside grid `g`.
bool ProbeSampleGrid(
    int g, vec3 position, vec3 normal, int layer, bool useVisibility, out vec3 result )
{
	const vec4 row0 = texelFetch( probeGrids, ivec2( 0, g ), 0 );
	const vec4 row1 = texelFetch( probeGrids, ivec2( 1, g ), 0 );
	const vec4 row2 = texelFetch( probeGrids, ivec2( 2, g ), 0 );
	const vec4 row3 = texelFetch( probeGrids, ivec2( 3, g ), 0 );
	const vec4 row4 = texelFetch( probeGrids, ivec2( 4, g ), 0 );
	const vec3 origin = row0.xyz;
	const uint tilesPerRow = uint( row0.w );
	const vec3 spacing = row1.xyz;
	const float maxDistance = row1.w;
	const vec3 dimsF = row2.xyz;
	const uvec3 dims = uvec3( dimsF );
	const vec2 irradianceOrigin = layer == 0 ? row3.xy : row3.zw;
	const vec2 visibilityOrigin = row4.xy;
	const uvec2 stateOrigin = uvec2( row4.zw );

	const vec3 biased = position + normal * kProbeNormalBias * row2.w;
	const vec3 g3 = ( biased - origin ) / spacing;
	if ( any( lessThan( g3, vec3( 0.0 ) ) ) || any( greaterThan( g3, dimsF - 1.0 ) ) )
		return false;
	const ivec3 base = min( ivec3( floor( g3 ) ), ivec3( dims ) - 2 );
	const vec3 alpha = g3 - vec3( base );
	const uint stateRow = tilesPerRow * 16u;
	vec3 total = vec3( 0.0 );
	float weights = 0.0;
	for ( int corner = 0; corner < 8; ++corner )
	{
		const ivec3 bits = ivec3( corner & 1, ( corner >> 1 ) & 1, ( corner >> 2 ) & 1 );
		const ivec3 index3 = base + bits;
		const vec3 t = mix( vec3( 1.0 ) - alpha, alpha, vec3( bits ) );
		const float trilinear = t.x * t.y * t.z;
		const uint probe = uint( index3.x ) + dims.x * ( uint( index3.y ) + dims.y * uint( index3.z ) );
		const vec4 state = texelFetch( probeAtlas,
		    ivec2( stateOrigin.x + probe % stateRow, stateOrigin.y + probe / stateRow ), 0 );
		if ( state.w < 0.5 )
			continue;
		const vec3 probePosition = origin + vec3( index3 ) * spacing + state.xyz;
		const vec3 toProbe = probePosition - position;
		const float toProbeLength = sqrt( dot( toProbe, toProbe ) );
		const float dotDirection =
		    toProbeLength > 1.0e-9 ? dot( toProbe, normal ) / toProbeLength : 1.0;
		const float backface = ( dotDirection + 1.0 ) * 0.5;
		float weight = backface * backface + 0.2;
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
		weight = max( weight, 1.0e-6 );
		if ( weight < kProbeCrushThreshold )
			weight *= weight * weight / ( kProbeCrushThreshold * kProbeCrushThreshold );
		weight *= trilinear;
		total += weight * ProbeTile( irradianceOrigin, 8.0, probe, tilesPerRow, normal ).rgb;
		weights += weight;
	}
	result = weights > 0.0 ? total / weights : vec3( 0.0 );
	return true;
}

// The first grid holding the point; false outside every grid, or when the
// volume lacks `layer`.
bool ProbeIrradiance(
    vec3 position, vec3 normal, int layer, bool useVisibility, out vec3 result )
{
	const vec4 counts = texelFetch( probeGrids, ivec2( 5, 0 ), 0 );
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
