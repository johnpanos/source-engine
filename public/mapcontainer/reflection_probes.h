//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RPRB reflection probes (RFC 0008's reflection probe lump, R50):
//          bounded validation, the GPU texture form and the C++ reference
//          blend.
//
// The encoding is documented once, in tools/quality/reflection_probe_set.py
// (the pipeline writer and independent reader); this reader must accept
// exactly what that writer produces and reject each malformation of its
// shared corpus with the same error. The reference blend here is the oracle
// for shaders/world_pbr_probe.glsl: parallax-corrected lookups against each
// probe's proxy box (Lagarde and Zanuttini, SIGGRAPH 2012), distance-based
// roughness (Lagarde and de Rousiers, SIGGRAPH 2014, Listing 25) and a
// continuous blend of the probes whose influence contains the shaded point.
// The design record is RFC/0007-progress.md, R50-PARALLAX.
//
// Units: positions and distances are Source units; radiance texels are the
// lightmap's linear units (the map's exposure gain applied).
//
// RPRB v2 (R50-RELIGHT) adds relight bands: per probe, the albedo and ray
// distance, and the normal, of what its capture saw, so a consumer can relight
// the point seen (McAuley, "Rendering the World of Far Cry 4", GDC 2015,
// relights a G-buffer cubemap). The rule (reflection_probe_set.py `relight`):
// light removed since the bake scales the capture (now / baked), light added
// is albedo * (now - baked); a moving occluder on the capture's line of sight
// replaces the point seen by the occluder's face.
//
//=============================================================================//

#ifndef MAPCONTAINER_REFLECTION_PROBES_H
#define MAPCONTAINER_REFLECTION_PROBES_H

#include <cstddef>
#include <cstdint>

namespace mapcontainer
{

static const uint32_t kLumpReflectionProbes = 0x42525052u; // "RPRB"
// RPRB v6 is the one version (2026-10-05); every earlier version is refused.
// Its conservative spatial rank masks (encoding owner: reflection_probe_set.py)
// are always present: a dim^3 grid of ceil(count / 64) uint64 words per cell.
static const uint32_t kReflectionProbesVersion = 6;
static const uint32_t kReflectionProbeCandidateDim = 32;
static const uint32_t kReflectionProbeCandidateCells =
    kReflectionProbeCandidateDim * kReflectionProbeCandidateDim * kReflectionProbeCandidateDim;
static const uint32_t kReflectionProbeCandidateHeaderBytes = 32;
static const uint32_t kReflectionProbesFlagRelight = 1; // header flags
static const float kReflectionProbesMaxDistance = 60000.0f;
static const float kReflectionProbesNormalLimit = 1.001f;
static const uint32_t kReflectionProbesHeaderBytes = 64;
static const uint32_t kReflectionProbeRecordBytes = 80;
static const uint32_t kReflectionProbesMaxProbes = 256;
static const uint32_t kReflectionProbesMaxMips = 12;
static const uint32_t kReflectionProbesMinWidth = 8;
static const uint32_t kReflectionProbesMaxWidth = 2048;
static const uint32_t kReflectionProbesPrefilterVersion = 1;
static const uint32_t kReflectionProbeGlobal = 1; // record flag
// The GPU table stores each value as a float16 hi/lo pair.
static const float kReflectionProbesMaxCoordinate = 65504.0f;
static const uint64_t kReflectionProbesMaxBytes =
    kReflectionProbesHeaderBytes + kReflectionProbeCandidateHeaderBytes +
    4 * 8 * uint64_t( kReflectionProbeCandidateCells ) +
    uint64_t( kReflectionProbesMaxProbes ) *
        ( kReflectionProbeRecordBytes + 16 +
            3 * uint64_t( kReflectionProbesMaxWidth ) * kReflectionProbesMaxWidth * 8 );
// Blend constants shared with reflection_probe_set.py and the shader.
static const float kReflectionProbeFacingEdge = 0.1f;

// `mat_reflection_probes`: off, blended and parallax-corrected, the nearest
// capture alone (Source 1's switch; the blend check's negative control), and
// blended but direction-only (the parallax check's negative control). The
// weight view (+4) replaces radiance with each selected probe's
// kReflectionProbeWeightPalette colour by rank, times its weight, so a
// capture of a mirror floor maps the blend.
enum class ReflectionProbeMode : uint32_t
{
	Off = 0,
	Blend = 1,
	Nearest = 2,
	DirectionOnly = 3,
	BlendWeights = 5,
	NearestWeights = 6,
	DirectionOnlyWeights = 7,
};
static const uint32_t kReflectionProbeModeSelection = 3;
static const uint32_t kReflectionProbeModeWeights = 4;
static const float kReflectionProbeWeightPalette[6][3] = { { 1.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 1.0f },
    { 0.0f, 1.0f, 1.0f } };
// True for a mode the shaders accept (0..3, 5..7).
bool ReflectionProbeModeValid( uint32_t mode ) noexcept;
// The candidate section's bytes for `count` probes.
uint64_t ReflectionProbeCandidateBytes( uint32_t count ) noexcept;

enum class ReflectionProbesError
{
	Ok = 0,
	Truncated,
	BadMagic,
	UnsupportedVersion,
	InvalidCounts,
	InvalidAtlas,
	InvalidSections,
	InvalidRecord,
	InvalidRanks,
	InvalidGlobal,
	InvalidTexels,
	InvalidCandidates,
};

struct ReflectionProbeRecord
{
	float capture[3];
	float fade;
	float boxMin[3];
	float boxMax[3];
	float influenceMin[3];
	float influenceMax[3];
	uint32_t rank;
	uint32_t flags;
	uint32_t bandRow;
	uint32_t relightRow; // the albedo band's first row (v2), the normal band's is + width / 2
};

struct ReflectionProbesLayout
{
	uint32_t count;
	uint32_t mipCount;
	uint32_t width; // mip 0 of every probe is width x width / 2
	uint32_t atlasWidth;
	uint32_t atlasHeight;
	uint32_t globalIndex;
	bool relight;         // v2: relight bands follow the radiance bands
	uint64_t atlasOffset; // RGBA16F texels, rows top-left first
	uint64_t atlasBytes;
	ReflectionProbeRecord probes[kReflectionProbesMaxProbes];
	// Immutable, validated conservative rank masks in the original payload.
	uint64_t candidateOffset = 0;
	uint32_t candidateWords = 0; // uint64 words per spatial cell: ceil(count / 64)
	float candidateOrigin[3] = {};
	float candidateStep = 0;
};

// Validates complete RPRB bytes (header, records, sections, and every atlas
// texel: finite, non-negative colour, alpha 1 inside a mip and 0 outside)
// without retaining them or allocating.
ReflectionProbesError ValidateReflectionProbes(
    const void *pData, size_t size, ReflectionProbesLayout *pLayout = nullptr ) noexcept;
const char *ReflectionProbesErrorName( ReflectionProbesError error ) noexcept;

// The GPU form the shaders read: one RGBA16F texture 2 * width wide and
// ReflectionProbeTextureRows() tall. Row 0: texel 0 = (count, mips, width,
// kReflectionProbeTextureMarker), texel 1 = (mode, 0, 0, 0). Row 1 + rank:
// that probe's five vec4 (capture.xyz, fade | box min.xyz, band row | box
// max.xyz, global | influence min.xyz, 0 | influence max.xyz, 0), each as two
// texels hi = half(v) and lo = half(v - hi). Then the atlas. With relight
// bands, influence min's w is the probe's relight row and texel 1's y is 1
// when `relight` asks for them to be applied.
static const float kReflectionProbeTextureMarker = -3.0f;
static const uint32_t kReflectionProbeTableVec4 = 5;
uint32_t ReflectionProbeTextureColumns( const ReflectionProbesLayout &layout ) noexcept;
uint32_t ReflectionProbeTextureWidth( const ReflectionProbesLayout &layout ) noexcept;
uint32_t ReflectionProbeTextureRows( const ReflectionProbesLayout &layout ) noexcept;
// Writes the texture (ReflectionProbeTextureWidth() * ReflectionProbeTextureRows() * 4 halves)
// from validated bytes.
void WriteReflectionProbeTexture( const void *pData, const ReflectionProbesLayout &layout,
    ReflectionProbeMode mode, uint16_t *pOut, bool relight = true ) noexcept;

// Relighting (RPRB v2): below this baked diffuse light a removal is absolute
// (indirect_sdf.h's kRelativeFloor), and at most this many moving occluders
// are tested (the traced producers' proxy limit).
static const float kReflectionProbeRelightFloor = 1e-4f;
static const uint32_t kReflectionProbeMaxOccluders = 16;

// Moving geometry no bake contains (a closed door): an axis-aligned box,
// Source units, and the diffuse reflectance of its faces.
struct ReflectionProbeOccluder
{
	float lo[3];
	float hi[3];
	float reflectance;
};

// What relights a probe's relight bands: the scene's diffuse light
// (irradiance / pi, the lightmap's unit) at a world point with a unit normal,
// now and as baked, and the moving occluders.
struct ReflectionProbeRelight
{
	void ( *evaluate )( void *context, const float position[3], const float normal[3],
	    float outNow[3], float outBaked[3] );
	void *context;
	const ReflectionProbeOccluder *occluders = nullptr;
	uint32_t occluderCount = 0;
};

// The relight rule per channel: removed light relative, added light
// absolute, clamped at zero.
float ReflectionProbeRelit( float radiance, float albedo, float now, float baked ) noexcept;

// The nearest occluder on the segment origin + t * direction, 0 <= t <
// length: false when none; else its entry t, the entered face's outward
// normal (a segment starting inside faces back along itself, t = 0) and
// its index.
bool ReflectionProbeOccluded( const ReflectionProbeOccluder *occluders, uint32_t count,
    const float origin[3], const float direction[3], float length, float *outT,
    float outNormal[3], uint32_t *outIndex ) noexcept;

// A read-only view of validated bytes; the caller keeps them alive.
class ReflectionProbesView
{
public:
	ReflectionProbesView( const void *pData, const ReflectionProbesLayout &layout ) noexcept;
	// Specular image light arriving at `position` (geometric normal
	// `normal`) along the unit reflected ray at perceptual `roughness`.
	// With `relight`, probes carrying relight bands are relit by it.
	void Radiance( const float position[3], const float normal[3], const float reflected[3],
	    float roughness, ReflectionProbeMode mode, float outRadiance[3],
	    const ReflectionProbeRelight *relight = nullptr ) const noexcept;
	// The per-probe sample weights at a point (at most two nonzero, sum 1).
	void Weights( const float position[3], const float normal[3], ReflectionProbeMode mode,
	    float outWeights[kReflectionProbesMaxProbes] ) const noexcept;
	const ReflectionProbesLayout &Layout() const noexcept { return m_layout; }

private:
	void ProbeRadiance( uint32_t probe, const float position[3], const float reflected[3],
	    float roughness, bool parallax, const ReflectionProbeRelight *relight,
	    float out[3] ) const noexcept;
	// Bilinear, `channels` of the band starting at atlas row `top`.
	void SampleLevel( uint32_t top, uint32_t level, const float uv[2], int channels,
	    float *out ) const noexcept;

	const unsigned char *m_bytes;
	ReflectionProbesLayout m_layout;
};

} // namespace mapcontainer

#endif // MAPCONTAINER_REFLECTION_PROBES_H
