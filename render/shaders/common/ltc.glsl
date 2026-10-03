// render.lighting.v1 area lights (RFC 0016 K11). RFC 0011's render.area-light.v1
// (public/render/area_light.h) defines the rectangle, its radiance, its
// one-sidedness and its window; this is the one GLSL copy of its per-pixel
// evaluation. Both lobes integrate a clamped cosine over the rectangle,
// clipped to the surface's horizon: the diffuse lobe the cosine itself (the
// exact Lambert form factor), the GGX lobe the cosine transformed by the
// fitted inverse matrix (linearly transformed cosines, Heitz, Dupuy, Hill and
// Neubelt 2016; the table is public/render/pbr_ltc_table.h). The edge
// integral is exact: each edge contributes the angle it subtends (atan2 of
// |a x b| and a . b) along its plane's unit normal.
#ifndef LTC_GLSL
#define LTC_GLSL

const float kLtcInverseTwoPi = 0.15915494309189535;
const int kLtcSize = 64;

// The table's entry at a perceptual roughness and N.V: roughness along v,
// sqrt(1 - N.V) along u, the texels on the end points, filtered bilinearly
// in full precision here (a device's filtering weights are only required to
// have a few bits, and a narrow lobe's entries change fast between texels).
vec4 LtcLookup( texture2D table, sampler point, float roughness, float normalDotView )
{
	const vec2 at = vec2( sqrt( max( 1.0 - normalDotView, 0.0 ) ), clamp( roughness, 0.0, 1.0 ) ) *
	                float( kLtcSize - 1 );
	const ivec2 low = clamp( ivec2( floor( at ) ), ivec2( 0 ), ivec2( kLtcSize - 1 ) );
	const ivec2 high = min( low + 1, ivec2( kLtcSize - 1 ) );
	const vec2 f = at - vec2( low );
	const vec4 a = texelFetch( sampler2D( table, point ), low, 0 );
	const vec4 b = texelFetch( sampler2D( table, point ), ivec2( high.x, low.y ), 0 );
	const vec4 c = texelFetch( sampler2D( table, point ), ivec2( low.x, high.y ), 0 );
	const vec4 d = texelFetch( sampler2D( table, point ), high, 0 );
	return mix( mix( a, b, f.x ), mix( c, d, f.x ), f.y );
}

// The inverse matrix from a table texel (m00, m02, m20, m22).
mat3 LtcInverse( vec4 texel )
{
	return mat3( vec3( texel.x, 0.0, texel.y ), vec3( 0.0, 1.0, 0.0 ),
	    vec3( texel.z, 0.0, texel.w ) );
}

// theta_k g_k for the edge from a to b (unit vectors).
vec3 LtcEdge( vec3 a, vec3 b )
{
	const vec3 c = cross( a, b );
	const float s = length( c );
	if ( s <= 1e-8 )
		return vec3( 0.0 );
	return c * ( atan( s, dot( a, b ) ) / s );
}

// Append one normalized vertex to an edge integral. Streaming the final
// horizon clip avoids a second dynamically indexed six-vertex array and keeps
// the clip's live range separate from the shadow evaluator in the caller.
void LtcAppend( vec3 p, inout vec3 first, inout vec3 previous, inout int count,
    inout float sum )
{
	const vec3 point = normalize( p );
	if ( count == 0 )
		first = point;
	else
		sum += LtcEdge( previous, point ).z;
	previous = point;
	++count;
}

// The integral over a rectangle (its four corners, counterclockwise seen
// from its front) of the clamped cosine transformed by `inverse`, in the
// frame at p whose z is n and whose x is the view projected on the surface.
// The rectangle is clipped twice: to the surface's horizon, below which the
// BRDF is zero, and to the transformed cosine's own horizon. A one-sided
// rectangle gives nothing from behind.
float LtcRectangle( vec3 n, vec3 v, vec3 p, mat3 inverse, vec3 corners[4], bool twoSided )
{
#ifndef SEEDED_LTC_NO_FRONT_TEST
	// Match area_light::Faces before the tangent-space integral: its sign
	// is numerically unstable when the receiver is coplanar with the emitter.
	const vec3 emitterNormal = cross( corners[1] - corners[0], corners[3] - corners[0] );
	if ( !twoSided && !( dot( emitterNormal, p - corners[0] ) > 0.0 ) )
		return 0.0;
#endif
	vec3 t1 = v - n * dot( v, n );
	if ( dot( t1, t1 ) < 1e-12 )
		t1 = abs( n.x ) < 0.9 ? vec3( 1.0, 0.0, 0.0 ) : vec3( 0.0, 1.0, 0.0 );
	t1 = normalize( t1 - n * dot( t1, n ) );
	const vec3 t2 = cross( n, t1 );
	const mat3 toTangent = transpose( mat3( t1, t2, n ) );
#ifdef SEEDED_LTC_TRANSPOSED
	const mat3 toCosine = transpose( inverse );
#else
	const mat3 toCosine = inverse;
#endif
	vec3 polygon[6];
	int count = 0;
	// Clip the original four edges to the surface horizon, transforming
	// each survivor directly. No intermediate tangent-space polygon survives.
	for ( int i = 0; i < 4; ++i )
	{
		const vec3 a = toTangent * ( corners[i] - p );
		const vec3 b = toTangent * ( corners[( i + 1 ) % 4] - p );
#ifdef SEEDED_LTC_NO_HORIZON_CLIP
		polygon[count++] = toCosine * a;
#else
		if ( a.z >= 0.0 )
			polygon[count++] = toCosine * a;
		if ( ( a.z >= 0.0 ) != ( b.z >= 0.0 ) )
			polygon[count++] = toCosine * mix( a, b, a.z / ( a.z - b.z ) );
#endif
	}
	if ( count < 3 )
		return 0.0;
	vec3 first = vec3( 0.0 ), previous = vec3( 0.0 );
	float sum = 0.0;
	int emitted = 0;
	for ( int i = 0; i < count; ++i )
	{
		const vec3 a = polygon[i];
		const vec3 b = polygon[( i + 1 ) % count];
#ifdef SEEDED_LTC_NO_HORIZON_CLIP
		LtcAppend( a, first, previous, emitted, sum );
#else
		if ( a.z >= 0.0 )
			LtcAppend( a, first, previous, emitted, sum );
		if ( ( a.z >= 0.0 ) != ( b.z >= 0.0 ) )
			LtcAppend( mix( a, b, a.z / ( a.z - b.z ) ), first, previous, emitted, sum );
#endif
	}
	if ( emitted < 3 )
		return 0.0;
	sum += LtcEdge( previous, first ).z;
	// Counterclockwise from the front gives a negative z.
	const float integral = -sum * kLtcInverseTwoPi;
#ifdef SEEDED_LTC_NO_HORIZON_CLIP
	return abs( integral );
#else
	return twoSided ? abs( integral ) : max( integral, 0.0 );
#endif
}

// RFC 0011's window at p (area_light::Window of area_light::DistanceTo).
float AreaLightWindow( vec3 center, vec3 halfU, vec3 halfV, float reach, vec3 p )
{
	const vec3 d = p - center;
	const float uu = dot( halfU, halfU );
	const float vv = dot( halfV, halfV );
	const float s = clamp( uu > 0.0 ? dot( d, halfU ) / uu : 0.0, -1.0, 1.0 );
	const float t = clamp( vv > 0.0 ? dot( d, halfV ) / vv : 0.0, -1.0, 1.0 );
	const float distance = length( p - ( center + s * halfU + t * halfV ) );
	if ( !( reach > 0.0 ) || distance >= reach )
		return 0.0;
	const float ratio = distance * distance / ( reach * reach );
	const float edge = 1.0 - ratio * ratio;
	return edge * edge;
}

#endif // LTC_GLSL
