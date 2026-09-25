// R50-PARALLAX: the map's RPRB reflection probes, blended and
// parallax-corrected. The includer defines how the probe texture
// (mapcontainer::WriteReflectionProbeTexture's GPU form) is read:
//
//   vec4 ReflectionProbesFetch( ivec2 texel )   the exact texel
//   vec4 ReflectionProbesSample( vec2 texel )   bilinear, texel coordinates
//                                               (texel centres at + 0.5)
//
// and, when it defines REFLECTION_PROBE_RELIGHT (R50-RELIGHT), the scene's
// diffuse-light change since the bake (irradiance / pi, the lightmap's unit)
// at a world point with a unit normal:
//
//   vec3 ReflectionProbeDiffuseChange( vec3 position, vec3 normal )
//
// world_pbr_probe.glsl reads a sampler; reflection_probes_check.comp reads a
// storage buffer with explicit bilinear filtering, so the GPU suite runs
// this file's blend, parallax and lod arithmetic unchanged.
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
//   * Relighting (R50-RELIGHT, RPRB v2): a probe with relight bands gains, at
//     its lookup direction and lod, albedo * change at the point its capture
//     saw there (capture + direction * distance, with the band's normal),
//     clamped at zero, when the mode texel's y is 1 (McAuley, "Rendering the
//     World of Far Cry 4", GDC 2015, relights a G-buffer cubemap).
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
vec3 ReflectionProbeDiffuseChange( vec3 position, vec3 normal );
#endif

const float kReflectionProbesMarker = -3.0;
const float kReflectionProbeFacingEdge = 0.1;
const int kReflectionProbesMaxProbes = 16;
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
			lookupRoughness = sharpened + ( roughness - sharpened ) * roughness;
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
		vec3 seen = captureFade.xyz + direction * albedo.w;
		radiance = max( radiance + albedo.rgb * ReflectionProbeDiffuseChange( seen, normal ),
		    vec3( 0.0 ) );
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
				         smoothstep( -kReflectionProbeFacingEdge, kReflectionProbeFacingEdge,
				             dot( normal, toward ) / max( length( toward ), 1e-9 ) );
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
