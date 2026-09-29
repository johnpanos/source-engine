// The map's RPRB reflection probes (RFC 0007 "Image-based lighting"; RFC 0016
// render.lighting.v1, "Image-based specular"), blended, parallax-corrected
// and relit, with distance-based roughness: the specular image light the
// includer weights by the split-sum directional albedo (pbr_brdf.glsl). The
// core's one copy, a line-for-line mirror of mapcontainer::
// ReflectionProbesView, its oracle (render.lab.reflection-probes); the
// native backend's materialsystem/shaderapivulkan/shaders/
// reflection_probes.glsl is frozen and goes at K12. The includer includes
// pbr_brdf.glsl first (kPi) and defines how the probe texture
// (mapcontainer::WriteReflectionProbeTexture's GPU form) is read:
//
//   vec4 ReflectionProbesFetch( ivec2 texel )   the exact texel
//   vec4 ReflectionProbesSample( vec2 texel )   bilinear, texel coordinates
//                                               (texel centres at + 0.5)
//
// and, when it defines REFLECTION_PROBE_RELIGHT (R50-RELIGHT), the scene's
// diffuse light (irradiance / pi, the lightmap's unit) at a world point with a
// unit normal, now and as baked, and the moving occluders (axis-aligned boxes
// in Source units with a diffuse reflectance, at most
// kReflectionProbeMaxOccluders):
//
//   void ReflectionProbeDiffuseLight( vec3 position, vec3 normal,
//                                     out vec3 now, out vec3 baked )
//   int  ReflectionProbeOccluderCount()
//   void ReflectionProbeOccluder( int k, out vec3 lo, out vec3 hi,
//                                 out float reflectance )
//
// A surface program reads a sampler; render/lab/reflection_probes_check.comp
// filters texel fetches explicitly, so the lab suite runs this file's blend,
// parallax and lod arithmetic unchanged.
//
// The C++ reference is mapcontainer::ReflectionProbesView (Radiance and
// Weights), which the world.reflection-probes suite checks against the
// Python oracle (tools/quality/reflection_probe_set.py); keep this file its
// line-for-line mirror.
//
//   * Parallax: the reflected ray from the shaded point is intersected with
//     the probe's proxy box, and the probe is sampled toward that hit from
//     its capture point (Lagarde and Zanuttini, "Local Image-based Lighting
//     With Parallax-corrected Cubemap", SIGGRAPH 2012).
//   * Distance-based roughness: the lobe's footprint on the proxy seen from
//     the capture point, clamped to the authored roughness, and the
//     corrected direction eased back to the ray as roughness grows (Lagarde
//     and de Rousiers, "Moving Frostbite to PBR", SIGGRAPH 2014 course notes,
//     Listing 25 and Listing F.1).
//   * Relighting (R50-RELIGHT, RPRB v2), when the mode texel's y is 1: at its
//     lookup direction and lod, a probe with relight bands saw the point
//     capture + direction * distance (the band's albedo and normal). Per
//     channel, light removed there since the bake scales the capture
//     (radiance * now / baked: visibility is multiplicative, so the capture's
//     detail is kept) and light added is albedo * (now - baked), clamped at
//     zero (ReflectionProbeRelit; the traced producers' rule for a probe
//     texel, indirect_sdf.h TracedProducer::Change). A moving occluder on the
//     segment from the capture to that point hides it: the capture sees the
//     occluder's face, its reflectance times the light now at the entry
//     point (McAuley, "Rendering the World of Far Cry 4", GDC 2015, relights
//     a G-buffer cubemap; Lazarov, SIGGRAPH 2013, and Unreal Engine instead
//     scale a capture by the diffuse light at the shaded point).
//   * Blending: by rank (influence volume ascending, the global probe last),
//     each probe takes its weight times what is still unassigned; a weight is
//     1 inside the influence box, smoothstep to 0 at `fade` outside, times a
//     facing term that drops a capture behind the surface. The two largest
//     shares are sampled, each less the third, renormalized: continuous as
//     the viewer moves (Source 1 switches cubemaps instead; 3kliksphilip,
//     "Advanced Reflections in CS:GO... and for Source 2?", 2019).
//
// Texture layout: row 0 texel 0 = (count, mips, mip-0 width, -3), texel 1 =
// (mode, 0, 0, 0); row 1 + rank = the probe's five vec4, each stored as a hi
// and a lo texel; the atlas from row 1 + count, probe i's band at its band
// row: mip l of W_l x W_l / 2 at x = 2 W0 (1 - 2^-l). With relight bands,
// influence min's w is the probe's albedo band row (rgb albedo, a distance;
// the normal band follows at + W0 / 2), and texel 1's y is the relight
// switch.

#ifdef REFLECTION_PROBE_RELIGHT
void ReflectionProbeDiffuseLight( vec3 position, vec3 normal, out vec3 now, out vec3 baked );
int ReflectionProbeOccluderCount();
void ReflectionProbeOccluder( int k, out vec3 lo, out vec3 hi, out float reflectance );
#endif

const float kReflectionProbesMarker = -3.0;
const float kReflectionProbeFacingEdge = 0.1;
const int kReflectionProbesMaxProbes = 16;
// mapcontainer::kReflectionProbeRelightFloor and kReflectionProbeMaxOccluders.
const float kReflectionProbeRelightFloor = 1e-4;
const int kReflectionProbeMaxOccluders = 16;
// The weight view's colours by rank (mapcontainer::kReflectionProbeWeightPalette).
const vec3 kReflectionProbeWeightPalette[6] = vec3[6]( vec3( 1.0, 0.0, 0.0 ),
    vec3( 0.0, 0.0, 1.0 ), vec3( 0.0, 1.0, 0.0 ), vec3( 1.0, 1.0, 0.0 ), vec3( 1.0, 0.0, 1.0 ),
    vec3( 0.0, 1.0, 1.0 ) );

vec4 ReflectionProbeRecord( int rank, int field )
{
	return ReflectionProbesFetch( ivec2( 2 * field, 1 + rank ) ) +
	       ReflectionProbesFetch( ivec2( 2 * field + 1, 1 + rank ) );
}

vec4 ReflectionProbeLevel( vec2 uv, float level, float width0, float top )
{
	float width = width0 * exp2( -level );
	vec2 extent = vec2( width, width * 0.5 );
	float left = 2.0 * width0 * ( 1.0 - exp2( -level ) );
	return ReflectionProbesSample(
	    vec2( left, top ) + clamp( uv * extent, vec2( 0.5 ), extent - vec2( 0.5 ) ) );
}

// One probe's split-sum fetch along the reflected ray from `position`,
// relit when `relight` and the probe carries relight bands.
#ifdef REFLECTION_PROBE_RELIGHT
// The relight rule per channel (mapcontainer::ReflectionProbeRelit).
vec3 ReflectionProbeRelit( vec3 radiance, vec3 albedo, vec3 now, vec3 baked )
{
	bvec3 relative = bvec3( vec3( lessThan( now, baked ) ) *
	                        vec3( greaterThan( baked, vec3( kReflectionProbeRelightFloor ) ) ) );
#ifdef SEEDED_RPRB_RELIGHT_ADDED_ONLY
	relative = bvec3( false );
#endif
	vec3 scaled = radiance * ( max( now, vec3( 0.0 ) ) / mix( vec3( 1.0 ), baked, relative ) );
	return max( mix( radiance + albedo * ( now - baked ), scaled, relative ), vec3( 0.0 ) );
}

// The nearest moving occluder on origin + t * direction, 0 <= t < length
// (mapcontainer::ReflectionProbeOccluded): its entry t, the entered face's
// outward normal and reflectance; false when none.
bool ReflectionProbeOccluded( vec3 origin, vec3 direction, float length, out float t,
    out vec3 faceNormal, out float reflectance )
{
	bool found = false;
	t = length;
	faceNormal = -direction;
	reflectance = 0.0;
	vec3 safe = vec3( direction.x >= 0.0 ? max( direction.x, 1e-12 ) : min( direction.x, -1e-12 ),
	    direction.y >= 0.0 ? max( direction.y, 1e-12 ) : min( direction.y, -1e-12 ),
	    direction.z >= 0.0 ? max( direction.z, 1e-12 ) : min( direction.z, -1e-12 ) );
	int count = min( ReflectionProbeOccluderCount(), kReflectionProbeMaxOccluders );
	for ( int k = 0; k < count; ++k )
	{
		vec3 lo, hi;
		float value;
		ReflectionProbeOccluder( k, lo, hi, value );
		vec3 first = ( lo - origin ) / safe;
		vec3 second = ( hi - origin ) / safe;
		vec3 nearAxes = min( first, second );
		vec3 farAxes = max( first, second );
		float near = max( nearAxes.x, max( nearAxes.y, nearAxes.z ) );
		float far = min( farAxes.x, min( farAxes.y, farAxes.z ) );
		float entry = max( near, 0.0 );
		if ( !( far >= entry ) || !( entry < t ) )
			continue;
		found = true;
		t = entry;
		reflectance = value;
		// The first axis on ties, as the C++ reference.
		int axis = nearAxes.x == near ? 0 : nearAxes.y == near ? 1 : 2;
		vec3 face = vec3( 0.0 );
		face[axis] = direction[axis] >= 0.0 ? -1.0 : 1.0;
		faceNormal = near < 0.0 ? -direction : face;
	}
	return found;
}
#endif

vec3 ReflectionProbeSample( int rank, int count, vec3 header, vec3 position, vec3 reflected,
    float roughness, bool parallax, bool relight )
{
	vec4 captureFade = ReflectionProbeRecord( rank, 0 );
	vec4 boxMinBand = ReflectionProbeRecord( rank, 1 );
	vec4 boxMaxGlobal = ReflectionProbeRecord( rank, 2 );
	vec3 direction = reflected;
	float lookupRoughness = roughness;
	if ( parallax )
	{
		vec3 safe = vec3( reflected.x >= 0.0 ? max( reflected.x, 1e-12 ) : min( reflected.x, -1e-12 ),
		    reflected.y >= 0.0 ? max( reflected.y, 1e-12 ) : min( reflected.y, -1e-12 ),
		    reflected.z >= 0.0 ? max( reflected.z, 1e-12 ) : min( reflected.z, -1e-12 ) );
		vec3 first = ( boxMaxGlobal.xyz - position ) / safe;
		vec3 second = ( boxMinBand.xyz - position ) / safe;
		vec3 farSide = max( first, second );
		vec3 nearSide = min( first, second );
		float far = min( farSide.x, min( farSide.y, farSide.z ) );
		float near = max( nearSide.x, max( nearSide.y, nearSide.z ) );
		vec3 lookup = reflected;
		if ( far > 0.0 && near <= far )
		{
			vec3 local = position + far * reflected - captureFade.xyz;
			float captured = length( local );
			lookup = local / max( captured, 1e-12 );
			float ratio = captured > 0.0 ? far / max( captured, 1e-12 ) : 1.0;
			float sharpened = clamp( ratio * roughness, 0.0, roughness );
#ifndef SEEDED_RPRB_NO_DISTANCE_ROUGHNESS
			lookupRoughness = sharpened + ( roughness - sharpened ) * roughness;
#endif
		}
		direction = lookup + ( reflected - lookup ) * roughness;
		direction /= max( length( direction ), 1e-12 );
	}
	float mips = header.y;
	vec2 uv = vec2( 0.5 - atan( direction.y, direction.x ) / ( 2.0 * kPi ),
	    0.5 - asin( clamp( direction.z, -1.0, 1.0 ) ) / kPi );
	float lod = clamp( lookupRoughness, 0.0, 1.0 ) * ( mips - 1.0 );
	float lower = floor( lod );
	float upper = min( lower + 1.0, mips - 1.0 );
	float top = float( 1 + count ) + boxMinBand.w;
	vec3 radiance = mix( ReflectionProbeLevel( uv, lower, header.z, top ),
	    ReflectionProbeLevel( uv, upper, header.z, top ), lod - lower ).rgb;
#ifdef REFLECTION_PROBE_RELIGHT
	float relightRow = ReflectionProbeRecord( rank, 3 ).w;
	if ( relight && relightRow > 0.5 )
	{
		float albedoTop = float( 1 + count ) + relightRow;
		float normalTop = albedoTop + 0.5 * header.z;
		vec4 albedo = mix( ReflectionProbeLevel( uv, lower, header.z, albedoTop ),
		    ReflectionProbeLevel( uv, upper, header.z, albedoTop ), lod - lower );
		vec3 normal = mix( ReflectionProbeLevel( uv, lower, header.z, normalTop ),
		    ReflectionProbeLevel( uv, upper, header.z, normalTop ), lod - lower ).rgb;
		normal /= max( length( normal ), 1e-12 );
		float t, reflectance;
		vec3 face;
		bool hidden =
		    ReflectionProbeOccluded( captureFade.xyz, direction, albedo.w, t, face, reflectance );
		vec3 now, baked;
		ReflectionProbeDiffuseLight(
		    captureFade.xyz + direction * ( hidden ? t : albedo.w ), hidden ? face : normal, now,
		    baked );
		radiance = hidden ? max( reflectance * now, vec3( 0.0 ) )
		                  : ReflectionProbeRelit( radiance, albedo.rgb, now, baked );
	}
#endif
	return radiance;
}

// Specular image light at `position` (geometric normal `normal`) along the
// unit reflected ray; false when the texture carries no reflection probes
// (the fallback texture) or the mode is off.
bool ReflectionProbesRadiance( vec3 position, vec3 normal, vec3 reflected, float roughness,
    out vec3 radiance )
{
	radiance = vec3( 0.0 );
	vec4 header = ReflectionProbesFetch( ivec2( 0, 0 ) );
	if ( header.w != kReflectionProbesMarker || header.x < 1.0 )
		return false;
	int count = min( int( header.x + 0.5 ), kReflectionProbesMaxProbes );
	vec4 modeTexel = ReflectionProbesFetch( ivec2( 1, 0 ) );
	int mode = int( modeTexel.x + 0.5 );
	bool relight = modeTexel.y > 0.5;
	int selection = mode & 3;
	if ( selection == 0 )
		return false;
	// The ranks of the (up to) two sampled probes and their weights.
	int ranks[2] = int[2]( 0, 0 );
	float weights[2] = float[2]( 0.0, 0.0 );
	if ( selection == 2 )
	{
		// Nearest capture (first on ties): Source 1's switch.
		float best = 1e30;
		for ( int rank = 0; rank < count; ++rank )
		{
			float d = length( position - ReflectionProbeRecord( rank, 0 ).xyz );
			if ( d < best )
			{
				best = d;
				ranks[0] = rank;
			}
		}
		weights[0] = 1.0;
	}
	else
	{
		// Shares by rank; keep the largest three (the third only as a value).
		float remaining = 1.0;
		float top0 = -1.0, top1 = -1.0, third = 0.0;
		int rank0 = 0, rank1 = 0;
		for ( int rank = 0; rank < count; ++rank )
		{
			vec4 captureFade = ReflectionProbeRecord( rank, 0 );
			float weight = 1.0;
			if ( ReflectionProbeRecord( rank, 2 ).w < 0.5 )
			{
				vec3 influenceMin = ReflectionProbeRecord( rank, 3 ).xyz;
				vec3 influenceMax = ReflectionProbeRecord( rank, 4 ).xyz;
				vec3 outside = max( max( influenceMin - position, position - influenceMax ),
				    vec3( 0.0 ) );
				vec3 toward = captureFade.xyz - position;
				weight = ( 1.0 - smoothstep( 0.0, captureFade.w, length( outside ) ) ) *
#ifdef SEEDED_RPRB_NO_FACING
				         1.0;
#else
				         smoothstep( -kReflectionProbeFacingEdge, kReflectionProbeFacingEdge,
				             dot( normal, toward ) / max( length( toward ), 1e-9 ) );
#endif
			}
			float share = weight * remaining;
			remaining -= share;
			// Ties keep the earlier rank (the reference keeps the lower
			// record index). A tie decides which probe is sampled only
			// when its weight is zero (it equals the third share), except
			// for three exactly equal shares, split evenly in both.
			if ( share > top0 )
			{
				third = max( third, top1 );
				top1 = top0;
				rank1 = rank0;
				top0 = share;
				rank0 = rank;
			}
			else if ( share > top1 )
			{
				third = max( third, top1 );
				top1 = share;
				rank1 = rank;
			}
			else
				third = max( third, share );
		}
		ranks[0] = rank0;
		ranks[1] = rank1;
		weights[0] = top0 - third;
		weights[1] = count > 1 ? top1 - third : 0.0;
		float total = weights[0] + weights[1];
		if ( total <= 1e-12 )
		{
			weights[0] = 1.0;
			weights[1] = count > 1 ? 1.0 : 0.0;
			total = weights[0] + weights[1];
		}
		weights[0] /= total;
		weights[1] /= total;
	}
	for ( int i = 0; i < 2; ++i )
		if ( weights[i] > 0.0 )
			radiance += weights[i] * ( ( mode & 4 ) != 0
			                               ? kReflectionProbeWeightPalette[ranks[i] % 6]
			                               : ReflectionProbeSample( ranks[i], count, header.xyz,
			                                     position, reflected, roughness,
			                                     selection != 3, relight ) );
	return true;
}
