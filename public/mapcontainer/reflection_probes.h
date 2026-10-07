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
// RPRB v2 (R50-RELIGHT) adds relight cubes: per probe, the albedo and ray
// distance, and the normal, of what its capture saw, so a consumer can relight
// the point seen (McAuley, "Rendering the World of Far Cry 4", GDC 2015,
// relights a G-buffer cubemap). The rule (reflection_probe_set.py `relight`):
// light removed since the bake scales the capture (now / baked), light added
// is albedo * (now - baked); a moving occluder on the capture's line of sight
// replaces the point seen by the occluder's face.
//
// RPRB v8 (2026-10-06) is the one version: each probe is a cube (layer =
// probe index) in the Vulkan cube convention, the radiance cube array
// BC6H-compressed with a full mip chain, sampled by the world direction. The
// layout is documented once, in reflection_probe_set.py.
//
//=============================================================================//

#ifndef MAPCONTAINER_REFLECTION_PROBES_H
#define MAPCONTAINER_REFLECTION_PROBES_H

#include <cstddef>
#include <cstdint>
#include <vector>

namespace mapcontainer
{

static const uint32_t kLumpReflectionProbes = 0x42525052u; // "RPRB"
// RPRB v8 is the one version (2026-10-06); every earlier version, the v7
// equirect strip atlas included, is refused. Each probe is a cube in the
// Vulkan convention (reflection_probe.py CUBE_FACES), layer = probe index;
// the radiance cube array is BC6H blocks, mip-major, then probe, then face,
// and DecodeReflectionProbes expands it to RGBA16F for the reference view.
// Its conservative spatial rank masks (encoding owner:
// reflection_probe_set.py) are always present: a dim^3 grid of
// ceil(count / 64) uint64 words per cell.
static const uint32_t kReflectionProbesVersion = 8;
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
static const uint32_t kReflectionProbesMinFace = 8;
static const uint32_t kReflectionProbesMaxFace = 1024;
static const uint32_t kReflectionProbesPrefilterVersion = 1;
static const uint32_t kReflectionProbeGlobal = 1; // record flag
// The shaders read each coordinate as a float32 (the probe buffer), so only
// the writer's finiteness bound applies.
static const float kReflectionProbesMaxCoordinate = 65504.0f;
// The largest conforming lump: every probe at the largest face with a full
// chain and relight cubes (a sanity bound for loaders, not a budget).
static const uint64_t kReflectionProbesMaxBytes =
    kReflectionProbesHeaderBytes + kReflectionProbeCandidateHeaderBytes +
    4 * 8 * uint64_t( kReflectionProbeCandidateCells ) +
    uint64_t( kReflectionProbesMaxProbes ) *
        ( kReflectionProbeRecordBytes + 6 * uint64_t( kReflectionProbesMaxFace ) *
                                            kReflectionProbesMaxFace * 4 / 3 * ( 1 + 2 * 8 ) );
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
	InvalidAtlas, // the cube geometry (face size, mips, record size)
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
	uint32_t layer;        // the radiance cube array layer (= the probe's index)
	uint32_t relightLayer; // the relight cube arrays' layer (= index with relight cubes, else 0)
};

struct ReflectionProbesLayout
{
	uint32_t count;
	uint32_t mipCount;
	uint32_t faceSize; // mip 0 of every probe is faceSize x faceSize per face
	uint32_t globalIndex;
	bool relight;         // relight cubes follow the radiance cubes
	bool decoded = false; // a layout from DecodeReflectionProbes (RGBA16F radiance)
	uint64_t dataOffset;  // the lump's cube data (BC6H radiance, then the relight cubes)
	uint64_t dataBytes;
	// In the lump: the radiance cubes' BC6H bytes, before the relight cubes;
	// 0 in a layout from DecodeReflectionProbes, whose radiance is RGBA16F.
	uint64_t radianceBlockBytes = 0;
	ReflectionProbeRecord probes[kReflectionProbesMaxProbes];
	// Immutable, validated conservative rank masks in the original payload.
	uint64_t candidateOffset = 0;
	uint32_t candidateWords = 0; // uint64 words per spatial cell: ceil(count / 64)
	float candidateOrigin[3] = {};
	float candidateStep = 0;
};

// Validates complete RPRB bytes (header, records, sections, the radiance
// blocks' size, and every relight texel: finite, albedo in [0, 1] with a
// positive distance, normals within the limit, alpha 1) without retaining
// them or allocating.
ReflectionProbesError ValidateReflectionProbes(
    const void *pData, size_t size, ReflectionProbesLayout *pLayout = nullptr ) noexcept;
// Validates, then expands the lump into `pOut`: its bytes up to the cube data
// unchanged, then the radiance cubes as RGBA16F (decoded from BC6H, alpha 1)
// and the relight cubes as stored. `pLayout` describes `pOut`, so the
// consumers below take `pOut->data()` and it. Errors are
// ValidateReflectionProbes'; a failed allocation reports Truncated.
ReflectionProbesError DecodeReflectionProbes( const void *pData, size_t size,
    std::vector<std::byte> *pOut, ReflectionProbesLayout *pLayout );
const char *ReflectionProbesErrorName( ReflectionProbesError error ) noexcept;
// Decodes BC6H radiance faces as the lump stores them (mip-major, then
// slice), mips firstMip to mipCount - 1 of a faceSize cube with `slices`
// faces per mip, to RGBA16F texels (alpha 1) in the same order: a device
// without BC formats samples these. False when `size` does not hold them.
bool DecodeReflectionProbeRadiance( const void *pBlocks, size_t size, uint32_t faceSize,
    uint32_t firstMip, uint32_t mipCount, uint32_t slices, std::vector<std::byte> *pOut );

// The cube arrays' subresource geometry, in the order the lump stores them:
// mip-major, then probe (layer), then face. A face-mip is size x size, size =
// faceSize >> level.
uint32_t ReflectionProbeMipSize( const ReflectionProbesLayout &layout, uint32_t level ) noexcept;
// A mip's byte offset from the start of the cube data and its byte length (all
// layers and faces), for the BC6H radiance (`kRadiance`, in a lump layout)
// and the RGBA16F arrays (`kRadianceHalf` in a decoded layout, `kAlbedo`,
// `kNormal`, in either).
enum class ReflectionProbeArray
{
	kRadiance,     // BC6H blocks, 16 bytes per 4 x 4
	kRadianceHalf, // RGBA16F, 8 bytes per texel (a decoded layout)
	kAlbedo,       // RGBA16F: albedo rgb, distance (Source units)
	kNormal,       // RGBA16F: normal xyz, 1
};
void ReflectionProbeMipRange( const ReflectionProbesLayout &layout, ReflectionProbeArray array,
    uint32_t level, uint64_t *pOffset, uint64_t *pBytes ) noexcept;

// The GPU probe buffer the shaders read: little-endian 32-bit words.
// Header, 16 words: count, mips, face size, candidate words per cell, mode,
// relight switch (1 when `relight` asks for relight cubes and the lump has
// them), candidate dimension, the base mip (the first lump mip the radiance
// cube array holds: a texture setting drops the top mips at upload, and a
// lookup at lod l reads max(l, base) - base of the array), candidate origin
// xyz and step (float), four
// zeros. From kReflectionProbeBufferProbesWord, one 20-float record per rank
// (capture.xyz, fade | box min.xyz, layer | box max.xyz, global | influence
// min.xyz, relight layer | influence max.xyz, 0); from
// kReflectionProbeBufferMasksWord the candidate masks as lo/hi word pairs.
// Radiance and relight live in the cube-array textures, not here.
static const uint32_t kReflectionProbeBufferHeaderWords = 16;
static const uint32_t kReflectionProbeBufferProbesWord = 16;
static const uint32_t kReflectionProbeBufferRecordWords = 20;
static const uint32_t kReflectionProbeBufferMasksWord =
    kReflectionProbeBufferProbesWord +
    kReflectionProbeBufferRecordWords * kReflectionProbesMaxProbes;
uint32_t ReflectionProbeBufferWords( const ReflectionProbesLayout &layout ) noexcept;
// Writes ReflectionProbeBufferWords() words from validated bytes.
void WriteReflectionProbeBuffer( const void *pData, const ReflectionProbesLayout &layout,
    ReflectionProbeMode mode, uint32_t *pOut, bool relight = true, uint32_t baseMip = 0 ) noexcept;
// The probe buffer for a Source BSP's cubemaps (its env_cubemap samples) in
// place of RPRB: one probe per sample in sample order (layer = index), each
// selected by nearest capture (ReflectionProbeMode::Nearest, Source 1's rule)
// and looked up by direction alone (a box of kReflectionProbesMaxCoordinate
// makes the parallax term vanish, as Source's cubemaps are at infinity). No
// candidate masks, no relight. `pOut` holds ReflectionProbeBufferWords() of
// the returned layout's words. False for a count of 0 or above the maximum.
bool WriteCubemapProbeBuffer( const float ( *pOrigins )[3], uint32_t count, uint32_t mips,
    uint32_t faceSize, uint32_t baseMip, std::vector<uint32_t> *pOut );

// The base mip a texture setting asks of a lump with `mips` levels: `drop`
// levels dropped, but at least four mips (and a face of 8) remain.
uint32_t ReflectionProbeBaseMip( uint32_t mips, uint32_t drop ) noexcept;

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

// A read-only view of validated, decoded bytes (DecodeReflectionProbes'
// output); the caller keeps them alive.
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
	// Bilinear taps beyond a face's edge since the last reset: those read the
	// neighbouring face, which the Vulkan specification leaves to the
	// implementation within one texel, so a consumer compares a result that
	// used one with a looser bound. Not thread-safe (the view is a test
	// oracle's).
	uint32_t SeamTaps() const noexcept { return m_seamTaps; }
	// The radiance reads at lod >= `baseMip`, as a cube array that dropped the
	// top mips does (the relight cubes are read at the lookup's own lod).
	void SetBaseMip( uint32_t baseMip ) noexcept { m_baseMip = baseMip; }
	void ResetSeamTaps() const noexcept { m_seamTaps = 0; }

private:
	void ProbeRadiance( uint32_t probe, const float position[3], const float reflected[3],
	    float roughness, bool parallax, const ReflectionProbeRelight *relight,
	    float out[3] ) const noexcept;
	// Bilinear and seamless, `channels` of one cube of the decoded `array`
	// at `layer` and mip `level`, along the unit `direction`.
	void SampleLevel( ReflectionProbeArray array, uint32_t layer, uint32_t level,
	    const float direction[3], int channels, float *out ) const noexcept;

	const unsigned char *m_bytes;
	ReflectionProbesLayout m_layout;
	mutable uint32_t m_seamTaps = 0;
	uint32_t m_baseMip = 0;
};

} // namespace mapcontainer

#endif // MAPCONTAINER_REFLECTION_PROBES_H
