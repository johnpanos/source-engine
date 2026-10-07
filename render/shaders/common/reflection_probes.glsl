// The map's RPRB reflection probes (RFC 0007 "Image-based lighting"; RFC 0016
// render.lighting.v1, "Image-based specular"), blended, parallax-corrected
// and relit, with distance-based roughness: the specular image light the
// includer weights by the split-sum directional albedo (pbr_brdf.glsl). The
// core's one copy, also consumed by the legacy frontend's PBR wrappers.
// Its oracle is mapcontainer::ReflectionProbesView. The includer defines
// the probe buffer and cube-array adapters (RPRB v8):
//
//   uint ReflectionProbesWord( uint index )   a 32-bit word of the probe buffer
//                                             (mapcontainer::WriteReflectionProbeBuffer)
//   uvec4 ReflectionProbesVec4( uint index )  optional, under REFLECTION_PROBES_VEC4:
//                                             16-byte element `index` of the same buffer
//                                             (record fields in one load)
//   vec4 ReflectionProbesRadianceFetch( vec3 direction, float layer, float lod )
//                                             the BC6H radiance cube array, trilinear
//                                             (textureLod of a samplerCubeArray), sampled
//                                             by the world direction
//
// and, when it defines REFLECTION_PROBE_RELIGHT (R50-RELIGHT), the relight
// cube arrays,
//
//   vec4 ReflectionProbesAlbedoFetch( vec3 direction, float layer, float lod )
//   vec4 ReflectionProbesNormalFetch( vec3 direction, float layer, float lod )
//
// and the scene's
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
// A surface program reads a samplerCubeArray; render/lab/reflection_probes_check.comp
// reads the same arrays, so the lab suite runs this file's blend, parallax and
// lod arithmetic unchanged.
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
//   * Blending: by serialized rank (authored priority, then influence volume,
//     the global probe last),
//     each probe takes its weight times what is still unassigned; a weight is
//     1 inside the influence box, smoothstep to 0 at `fade` outside, times a
//     facing term that drops a capture behind the surface. The two largest
//     shares are sampled, each less the third, renormalized: continuous as
//     the viewer moves (Source 1 switches cubemaps instead; 3kliksphilip,
//     "Advanced Reflections in CS:GO... and for Source 2?", 2019).
//
// Probe buffer layout (mapcontainer/reflection_probes.h): word 0 count, 1 mips,
// 2 face size, 3 candidate words per cell, 4 mode, 5 relight switch, 6 candidate
// dimension, 7 base mip (the radiance array's first lump mip), 8..10 candidate
// origin and 11 step (float); from word 16 one
// 20-float record per rank (capture.xyz, fade | box min.xyz, layer | box max.xyz,
// global | influence min.xyz, relight layer | influence max.xyz, 0); from word
// kReflectionProbeMasksWord the candidate masks as lo/hi word pairs. Radiance is
// cube array layer = the record's layer at lod = roughness * (mips - 1);
// relight layers are the same arrays' albedo/normal (RPRB v8 is the only form).

#ifdef REFLECTION_PROBE_RELIGHT
void ReflectionProbeDiffuseLight( vec3 position, vec3 normal, out vec3 now, out vec3 baked );
int ReflectionProbeOccluderCount();
void ReflectionProbeOccluder( int k, out vec3 lo, out vec3 hi, out float reflectance );
#endif

const float kReflectionProbeFacingEdge = 0.1;
const int kReflectionProbesMaxProbes = 256;
// mapcontainer::kReflectionProbeBufferProbesWord, RecordWords and MasksWord.
const uint kReflectionProbeProbesWord = 16u;
const uint kReflectionProbeRecordWords = 20u;
const uint kReflectionProbeMasksWord = 5136u;
// mapcontainer::kReflectionProbeRelightFloor and kReflectionProbeMaxOccluders.
const float kReflectionProbeRelightFloor = 1e-4;
const int kReflectionProbeMaxOccluders = 16;
// The weight view's colours by rank (mapcontainer::kReflectionProbeWeightPalette).
const vec3 kReflectionProbeWeightPalette[6] = vec3[6]( vec3( 1.0, 0.0, 0.0 ),
    vec3( 0.0, 0.0, 1.0 ), vec3( 0.0, 1.0, 0.0 ), vec3( 1.0, 1.0, 0.0 ), vec3( 1.0, 0.0, 1.0 ),
    vec3( 0.0, 1.0, 1.0 ) );

vec4 ReflectionProbeRecord( int rank, int field )
{
#ifdef REFLECTION_PROBES_VEC4
	// One 16-byte load: records start at word 16 and are 20 words, so every
	// field is 16-byte aligned (vec4 index 4 + rank * 5 + field).
	return uintBitsToFloat( ReflectionProbesVec4(
	    kReflectionProbeProbesWord / 4u + uint( rank ) * ( kReflectionProbeRecordWords / 4u ) +
	    uint( field ) ) );
#else
	uint at = kReflectionProbeProbesWord + uint( rank ) * kReflectionProbeRecordWords +
	          uint( field ) * 4u;
	return vec4( uintBitsToFloat( ReflectionProbesWord( at ) ),
	    uintBitsToFloat( ReflectionProbesWord( at + 1u ) ),
	    uintBitsToFloat( ReflectionProbesWord( at + 2u ) ),
	    uintBitsToFloat( ReflectionProbesWord( at + 3u ) ) );
#endif
}

// The direction a cube array is read along: the world direction. The seeded
// wrong-face defect swaps x and y (a rotated cube).
vec3 ReflectionProbeCubeDirection( vec3 direction )
{
#ifdef SEEDED_RPRB_WRONG_FACE
	return direction.yxz;
#else
	return direction;
#endif
}

// The lod and layer a probe is read at; the seeded defects read the wrong mip
// (roughness inverted) and the wrong probe (the next layer).
float ReflectionProbeLod( float roughness, float mips )
{
#ifdef SEEDED_RPRB_WRONG_LOD
	return ( 1.0 - clamp( roughness, 0.0, 1.0 ) ) * ( mips - 1.0 );
#else
	return clamp( roughness, 0.0, 1.0 ) * ( mips - 1.0 );
#endif
}

// The first mip the radiance array holds (probe buffer word 7): a graphics
// setting that drops the top mips of the cube array at upload. The array's
// level j is the lump's level j + base, so a lookup at lod l reads the
// array at max(l, base) - base; the seeded defect ignores the offset.
float ReflectionProbeBaseMip()
{
#ifdef SEEDED_RPRB_BASE_MIP_IGNORED
	return 0.0;
#else
	return float( ReflectionProbesWord( 7u ) );
#endif
}

float ReflectionProbeLayer( float layer, int count )
{
#ifdef SEEDED_RPRB_WRONG_LAYER
	return mod( layer + 1.0, float( count ) );
#else
	return layer;
#endif
}

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
	float lod = ReflectionProbeLod( lookupRoughness, mips );
	vec3 cubeDirection = ReflectionProbeCubeDirection( direction );
	float layer = ReflectionProbeLayer( boxMinBand.w, count );
	vec3 radiance = ReflectionProbesRadianceFetch(
	    cubeDirection, layer, max( lod, ReflectionProbeBaseMip() ) - ReflectionProbeBaseMip() )
	                        .rgb;
#ifdef REFLECTION_PROBE_RELIGHT
	float relightLayer = ReflectionProbeRecord( rank, 3 ).w;
	if ( relight )
	{
		vec4 albedo = ReflectionProbesAlbedoFetch( cubeDirection, relightLayer, lod );
		vec3 normal = ReflectionProbesNormalFetch( cubeDirection, relightLayer, lod ).rgb;
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

// v4 masks are validated for conservative spatial coverage when loaded.
// Facing, rank, weights and radiance remain runtime computations.
uvec2 ReflectionCandidates( vec3 position, int count, int group )
{
	int remaining = clamp( count - 64 * group, 0, 64 );
	uvec2 allRanks = uvec2( remaining >= 32 ? 0xffffffffu : ( 1u << uint( remaining ) ) - 1u,
	    remaining <= 32   ? 0u
	    : remaining == 64 ? 0xffffffffu
	                      : ( 1u << uint( remaining - 32 ) ) - 1u );
	int dim = int( ReflectionProbesWord( 6u ) );
	int groups = int( ReflectionProbesWord( 3u ) );
	if ( groups < 1 || groups > 4 || dim < 1 )
		return allRanks;
	vec3 origin = vec3( uintBitsToFloat( ReflectionProbesWord( 8u ) ),
	    uintBitsToFloat( ReflectionProbesWord( 9u ) ),
	    uintBitsToFloat( ReflectionProbesWord( 10u ) ) );
	float step = uintBitsToFloat( ReflectionProbesWord( 11u ) );
	vec3 cell = floor( ( position - origin ) / step );
	if ( any( lessThan( cell, vec3( 0.0 ) ) ) || any( greaterThanEqual( cell, vec3( float( dim ) ) ) ) )
		return allRanks;
	ivec3 index = ivec3( cell );
	uint offset = kReflectionProbeMasksWord +
	              2u * uint( groups * ( index.x + dim * ( index.y + dim * index.z ) ) + group );
	return uvec2( ReflectionProbesWord( offset ), ReflectionProbesWord( offset + 1u ) );
}

// Specular image light at `position` (geometric normal `normal`) along the
// unit reflected ray; false when the buffer carries no reflection probes
// (the neutral buffer: count 0) or the mode is off.
bool ReflectionProbesRadianceDebug( vec3 position, vec3 normal, vec3 reflected, float roughness,
    out vec3 radiance, out vec4 selectionInfo )
{
	radiance = vec3( 0.0 );
	selectionInfo = vec4( -1.0, -1.0, 0.0, 0.0 );
	uint words = ReflectionProbesWord( 0u );
	if ( words < 1u )
		return false;
	int count = min( int( words ), kReflectionProbesMaxProbes );
	vec3 header = vec3( float( count ), float( ReflectionProbesWord( 1u ) ),
	    float( ReflectionProbesWord( 2u ) ) );
	int mode = int( ReflectionProbesWord( 4u ) );
	bool relight = ReflectionProbesWord( 5u ) != 0u;
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
		for ( int group = 0; group < ( count + 63 ) / 64; ++group )
		{
			uvec2 candidates = ReflectionCandidates( position, count, group );
			while ( any( notEqual( candidates, uvec2( 0u ) ) ) )
			{
				int word = candidates.x != 0u ? 0 : 1;
				int rank = 64 * group + 32 * word + findLSB( candidates[word] );
				candidates[word] &= candidates[word] - 1u;
				vec4 captureFade = ReflectionProbeRecord( rank, 0 );
				float weight = 1.0;
				if ( ReflectionProbeRecord( rank, 2 ).w < 0.5 )
				{
					vec3 influenceMin = ReflectionProbeRecord( rank, 3 ).xyz;
					vec3 influenceMax = ReflectionProbeRecord( rank, 4 ).xyz;
					vec3 outside =
					    max( max( influenceMin - position, position - influenceMax ), vec3( 0.0 ) );
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
	selectionInfo = vec4(
	    float( ranks[0] ), weights[1] > 0.0 ? float( ranks[1] ) : -1.0, weights[0], weights[1] );
	for ( int i = 0; i < 2; ++i )
		if ( weights[i] > 0.0 )
			radiance +=
			    weights[i] * ( ( mode & 4 ) != 0
			                         ? kReflectionProbeWeightPalette[ranks[i] % 6]
			                         : ReflectionProbeSample( ranks[i], count, header, position,
			                               reflected, roughness, selection != 3, relight ) );
	return true;
}

bool ReflectionProbesRadiance( vec3 position, vec3 normal, vec3 reflected, float roughness,
    out vec3 radiance )
{
	vec4 unusedSelection;
	return ReflectionProbesRadianceDebug(
	    position, normal, reflected, roughness, radiance, unusedSelection );
}
