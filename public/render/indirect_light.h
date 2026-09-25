//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The indirect-light producer contract (render.indirect-light.v1,
//          RFC 0011): what a baked, radiosity, SDF or ray-query producer
//          promises, so producers substitute behind one switcher and one
//          shared suite (unittests/rendertest/test_indirect_light.cpp).
//
//          Obligations (each checked by the suite, each seeded by a bad
//          producer it must reject):
//          - Begin validates the scene and either succeeds or leaves no
//            resources behind, reporting an IndirectError.
//          - A published volume is complete, immutable, in the scene's grid
//            topology, with a strictly increasing epoch.
//          - The first publication after Begin is not darker than the seed by
//            more than the producer's seed tolerance (no black flash).
//          - A published volume keeps the seed's visibility (no leaks).
//          - Total minus indirect stays the seed's direct light: a producer
//            publishes indirect light, never counts the direct twice.
//          - Within convergenceFrames of a change a producer claims to
//            respond to, its volume is within tolerance of that change's
//            steady state; a producer claims no response it lacks.
//          - Resources are released behind the completion serial End's
//            ticket names; nothing a submitted frame may read is freed.
//          - Schedule adds CPU jobs to the frame's executor and returns; it
//            never waits for the GPU and never creates threads.
//
//===========================================================================//

#ifndef RENDER_INDIRECT_LIGHT_H
#define RENDER_INDIRECT_LIGHT_H

#include "foundation/expected.h"
#include "mapcontainer/probe_volume.h"
#include "mapcontainer/radiosity_transfer.h"
#include "mapcontainer/sdf_volume.h"
#include "render/gpu_compute.h"
#include "render/indirect_policy.h"
#include "render/light_set.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <vector>

namespace indirect_light
{
enum class ProducerKind : uint8_t
{
	Baked,
	PrecomputedRadiosity,
	SdfTraced,
	RayQuery,
	ScriptedFake, // the contract's test producer; never offered to players
};

// r_indirect_producer's values.
[[nodiscard]] inline const char *ProducerName( ProducerKind kind )
{
	switch ( kind )
	{
	case ProducerKind::Baked:
		return "baked";
	case ProducerKind::PrecomputedRadiosity:
		return "radiosity";
	case ProducerKind::SdfTraced:
		return "sdf";
	case ProducerKind::RayQuery:
		return "rayquery";
	case ProducerKind::ScriptedFake:
		return "fake";
	}
	return "unknown";
}

[[nodiscard]] inline bool ParseProducer( const char *name, ProducerKind *out )
{
	for ( ProducerKind kind : { ProducerKind::Baked, ProducerKind::PrecomputedRadiosity,
	          ProducerKind::SdfTraced, ProducerKind::RayQuery, ProducerKind::ScriptedFake } )
	{
		if ( name && std::strcmp( name, ProducerName( kind ) ) == 0 )
		{
			*out = kind;
			return true;
		}
	}
	return false;
}

// The scene changes a producer can respond to.
enum Response : uint32_t
{
	kLightIntensity = 1u << 0, // style scalars, toggles, dimming, color
	kLightMotion = 1u << 1,    // moved or unbaked lights
	kGeometryMotion = 1u << 2, // doors, panels, props occlude and bounce
	kEmission = 1u << 3,       // emissive material changes
	kPortalTransport = 1u << 4, // light through open portal pairs (RFC 0011 G10)
};

[[nodiscard]] constexpr uint32_t PolicyBit( indirect_policy::Policy policy )
{
	return 1u << uint32_t( policy );
}

struct ProducerCaps
{
	ProducerKind kind = ProducerKind::Baked;
	uint32_t responds = 0;
	uint32_t policies = PolicyBit( indirect_policy::Policy::Baked );
	uint32_t requiredFeatures = 0; // device feature bits (RFC 0011 G5)
	uint32_t convergenceFrames = 0;
	// Frames after Begin before responses count (a producer that first
	// converges its own estimate of the baked scene); the seed stays
	// published meanwhile.
	uint32_t warmupFrames = 0;
	bool publishesSurfaceAtlas = false;
	float seedTolerance = 0.05f;     // first publication: at least (1 - this) x the seed
	float responseTolerance = 0.05f; // converged: within this of the steady state
};

enum class IndirectError : uint8_t
{
	MissingSceneData = 1, // e.g. no probe volume, no RTRN, no SDF input
	UnsupportedPolicy,
	MissingFeature,
	InvalidSeed,
	Unavailable, // not offered on this profile or map
};

[[nodiscard]] inline const char *IndirectErrorName( IndirectError error )
{
	switch ( error )
	{
	case IndirectError::MissingSceneData:
		return "missing-scene-data";
	case IndirectError::UnsupportedPolicy:
		return "unsupported-policy";
	case IndirectError::MissingFeature:
		return "missing-feature";
	case IndirectError::InvalidSeed:
		return "invalid-seed";
	case IndirectError::Unavailable:
		return "unavailable";
	}
	return "unknown";
}

// IEEE binary16 from float, round to nearest even (the PRBV texel encoding;
// mapcontainer owns it).
[[nodiscard]] inline uint16_t FloatToHalf( float value )
{
	return mapcontainer::FloatToHalf( value );
}

// A complete, immutable probe volume: validated PRBV bytes and their layout.
struct Volume
{
	std::vector<unsigned char> bytes;
	mapcontainer::ProbeVolumeLayout layout{};

	// Null when the bytes are not a valid volume.
	[[nodiscard]] static std::shared_ptr<const Volume> FromBytes( std::vector<unsigned char> bytes )
	{
		auto volume = std::make_shared<Volume>();
		if ( mapcontainer::ValidateProbeVolume( bytes.data(), bytes.size(), &volume->layout ) !=
		     mapcontainer::ProbeVolumeError::Ok )
			return nullptr;
		volume->bytes = std::move( bytes );
		return volume;
	}

	[[nodiscard]] mapcontainer::ProbeVolumeView View() const
	{
		return mapcontainer::ProbeVolumeView( bytes.data(), layout );
	}

	// Same header, grids and atlas: volumes a fade can blend texel by texel.
	[[nodiscard]] bool SameTopology( const Volume &other ) const
	{
		return bytes.size() == other.bytes.size() &&
		       layout.atlasOffset == other.layout.atlasOffset &&
		       std::memcmp( bytes.data(), other.bytes.data(), size_t( layout.atlasOffset ) ) == 0;
	}

	// The mean luminance of every probe's layer-`layer` irradiance tile (the
	// measure a switch's "no black frame" check compares).
	[[nodiscard]] float MeanIrradiance( uint32_t layer = 0 ) const
	{
		double sum = 0.0;
		uint64_t count = 0;
		for ( uint32_t g = 0; g < layout.gridCount; ++g )
		{
			const mapcontainer::ProbeGridLayout &grid = layout.grids[g];
			if ( layer >= layout.layerCount )
				return 0.0f;
			const uint32_t rows = ( grid.probeCount + grid.tilesPerRow - 1 ) / grid.tilesPerRow;
			const uint32_t x0 = grid.irradianceOrigin[layer][0];
			const uint32_t y0 = grid.irradianceOrigin[layer][1];
			for ( uint32_t y = y0; y < y0 + rows * mapcontainer::kProbeIrradianceTile; ++y )
			{
				for ( uint32_t x = x0;
				    x < x0 + grid.tilesPerRow * mapcontainer::kProbeIrradianceTile; ++x )
				{
					const unsigned char *texel = bytes.data() + layout.atlasOffset +
					                             ( uint64_t( y ) * layout.atlasWidth + x ) * 8;
					uint16_t rgb[3];
					std::memcpy( rgb, texel, sizeof( rgb ) );
					sum += 0.2126 * mapcontainer::HalfToFloat( rgb[0] ) +
					       0.7152 * mapcontainer::HalfToFloat( rgb[1] ) +
					       0.0722 * mapcontainer::HalfToFloat( rgb[2] );
					++count;
				}
			}
		}
		return count ? float( sum / double( count ) ) : 0.0f;
	}
};

// Texel-by-texel blend of two volumes of one topology: a fade's frame.
// Null when the topologies differ.
[[nodiscard]] inline std::shared_ptr<const Volume> Blend(
    const Volume &from, const Volume &to, float weight )
{
	if ( !from.SameTopology( to ) )
		return nullptr;
	auto blended = std::make_shared<Volume>( from );
	const size_t begin = size_t( from.layout.atlasOffset );
	for ( size_t i = begin; i + 1 < from.bytes.size(); i += 2 )
	{
		uint16_t a, b;
		std::memcpy( &a, from.bytes.data() + i, 2 );
		std::memcpy( &b, to.bytes.data() + i, 2 );
		const float value = mapcontainer::HalfToFloat( a ) * ( 1.0f - weight ) +
		                    mapcontainer::HalfToFloat( b ) * weight;
		const uint16_t half = FloatToHalf( value );
		std::memcpy( blended->bytes.data() + i, &half, 2 );
	}
	return blended;
}

// A validated RTRN radiosity transfer paired with the map's volume (the
// precomputed radiosity producer's input).
struct Transfer
{
	std::vector<unsigned char> bytes;
	mapcontainer::RadiosityTransferLayout layout{};

	// Null when the bytes are not a valid transfer baked for `volume`.
	[[nodiscard]] static std::shared_ptr<const Transfer> FromBytes(
	    std::vector<unsigned char> bytes, const Volume &volume )
	{
		auto transfer = std::make_shared<Transfer>();
		if ( mapcontainer::ValidateRadiosityTransfer( bytes.data(), bytes.size(), &transfer->layout,
		         volume.bytes.data(), &volume.layout ) != mapcontainer::RadiosityTransferError::Ok )
			return nullptr;
		transfer->bytes = std::move( bytes );
		return transfer;
	}

	[[nodiscard]] mapcontainer::RadiosityTransferView View() const
	{
		return mapcontainer::RadiosityTransferView( bytes.data(), layout );
	}
};

// A validated SDFV signed distance volume (the SDF-traced producer's input).
struct SdfData
{
	std::vector<unsigned char> bytes;
	mapcontainer::SdfVolumeLayout layout{};

	[[nodiscard]] static std::shared_ptr<const SdfData> FromBytes( std::vector<unsigned char> bytes )
	{
		auto sdf = std::make_shared<SdfData>();
		if ( mapcontainer::ValidateSdfVolume( bytes.data(), bytes.size(), &sdf->layout ) !=
		     mapcontainer::SdfVolumeError::Ok )
			return nullptr;
		sdf->bytes = std::move( bytes );
		return sdf;
	}
};

// The static world's triangles (Source units), for producers that trace
// geometry (the ray-query producer): xyz positions and 32-bit indices.
struct WorldGeometry
{
	std::vector<float> positions;
	std::vector<uint32_t> indices;
};

// A moving occluder this frame (a closed door, a pushed crate): its world
// box (Source units) and reflectance. Producers that claim kGeometryMotion
// include it; the others ignore it.
struct Proxy
{
	float lo[3] = {};
	float hi[3] = {};
	float reflectance = 0.5f;

	bool operator==( const Proxy & ) const = default;
};

// An open portal (RFC 0011 G10): a rectangle on a wall that shows its linked
// portal's side of the map. Source units; the unit axes of its frame
// (forward out of the wall), its half extents along right and up, and the
// row-major 3x4 transform from a point in front of it to the point the
// linked portal shows it at. Producers claiming kPortalTransport carry light
// through it; the others ignore it. Both portals of an open pair are listed.
struct Portal
{
	float origin[3] = {};
	float forward[3] = {};
	float right[3] = {};
	float up[3] = {};
	float halfWidth = 0.0f;
	float halfHeight = 0.0f;
	float toLinked[12] = {};

	bool operator==( const Portal & ) const = default;
};

// Each probe's world position (xyz) and whether it is active (w 1 or 0), from
// a volume's first grid and its relocation offsets.
[[nodiscard]] inline std::vector<float> ProbePositions( const Volume &volume )
{
	const mapcontainer::ProbeVolumeLayout &layout = volume.layout;
	const mapcontainer::ProbeGridLayout &grid = layout.grids[0];
	std::vector<float> out( size_t( grid.probeCount ) * 4 );
	const uint32_t row = grid.tilesPerRow * mapcontainer::kProbeVisibilityTile;
	for ( uint32_t i = 0; i < grid.probeCount; ++i )
	{
		const uint32_t index[3] = { i % grid.dims[0], ( i / grid.dims[0] ) % grid.dims[1],
		    i / ( grid.dims[0] * grid.dims[1] ) };
		uint16_t state[4];
		std::memcpy( state,
		    volume.bytes.data() + layout.atlasOffset +
		        ( uint64_t( grid.stateOrigin[1] + i / row ) * layout.atlasWidth +
		            grid.stateOrigin[0] + i % row ) *
		            8,
		    sizeof( state ) );
		for ( int k = 0; k < 3; ++k )
			out[i * 4 + k] = grid.origin[k] + float( index[k] ) * grid.spacing[k] +
			                 mapcontainer::HalfToFloat( state[k] );
		out[i * 4 + 3] = mapcontainer::HalfToFloat( state[3] );
	}
	return out;
}

// A scene light's current direction, by light style (a moved sun).
struct LightOverride
{
	int32_t style = -1;
	float direction[3] = { 0, 0, -1 };
};

struct PublishedVolume
{
	uint64_t epoch = 0;
	// Shared by the producer, the switcher's fade and the frames that sample
	// it until the host retires them behind the completion serial.
	std::shared_ptr<const Volume> volume;
};

// A scripted scene change (a door opening, a light toggled): the scenario
// inputs a producer's response is judged on. A change persists until another
// replaces it. For kLightIntensity and kEmission, `target` is a transfer
// source index (or kEverySource) and `value` its scalar, which overrides the
// light set's style scalar for that source.
struct SceneChange
{
	uint32_t response = 0; // the Response bit it exercises
	uint32_t target = 0;
	float value = 0.0f;
};
constexpr uint32_t kEverySource = 0xFFFFFFFFu;

struct IndirectScene
{
	uint64_t mapSerial = 0;
	std::shared_ptr<const Volume> baked; // the map's PRBV: every producer's seed
	indirect_policy::Policy policy = indirect_policy::Policy::Baked;
	uint32_t deviceFeatures = 0;
	std::shared_ptr<const Transfer> transfer; // RTRN: the radiosity producer's input
	std::shared_ptr<const SdfData> sdf;       // SDFV: the SDF-traced producer's input
	std::shared_ptr<const WorldGeometry> geometry; // the ray-query producer's triangles
	gpu_compute::IGpuCompute *gpu = nullptr;  // the renderer's compute service, if any
};

// GPU resources a producer owns, released behind the provider's completion
// serial (the native provider's retirement rule).
class IResourceTracker
{
public:
	virtual ~IResourceTracker() = default;
	virtual uint64_t Acquire( size_t bytes ) = 0;
	// Frees `id` once the completion serial reaches `afterSerial`.
	virtual void Release( uint64_t id, uint64_t afterSerial ) = 0;
	[[nodiscard]] virtual uint64_t SubmittedSerial() const = 0;
	[[nodiscard]] virtual uint64_t CompletedSerial() const = 0;
};

// The frame executor's data-parallel form (RFC 0003): runs body( context, i )
// for every i in [0, count), possibly concurrently on the engine's workers,
// and returns once all have completed, so their writes happen-before the
// return. Items must be independent and must not wait on this executor. The
// serial mode, the oracle for the pooled one, runs them in ascending order
// on the caller.
class IBatchExecutor
{
public:
	virtual ~IBatchExecutor() = default;
	virtual void ParallelFor(
	    const char *name, uint32_t count, void ( *body )( void *, uint32_t ), void *context ) = 0;
};

// Composes a published volume: a base volume plus per-probe changes of its
// two irradiance layers (36 rgb per probe, the interior's order), clamped at
// zero, borders rewritten (probe_volume.with_border). A probe whose change is
// zero keeps the base's tiles. Batches of probes run on the executor (null:
// inline); the base's interior texels are decoded once per base.
class ChangeComposer
{
public:
	static constexpr uint32_t kInterior = mapcontainer::kProbeIrradianceTile - 2;
	static constexpr uint32_t kTexels = kInterior * kInterior;
	static constexpr uint32_t kBlock = 8;

	[[nodiscard]] std::shared_ptr<const Volume> Compose( const Volume &base, const float *total,
	    const float *indirect, uint32_t probeCount, IBatchExecutor *executor ) const
	{
		if ( m_baseBytes != base.bytes.data() || m_baseSize != base.bytes.size() )
			DecodeBase( base, probeCount );
		auto volume = std::make_shared<Volume>( base );
		Context context{ this, volume.get(), total, indirect, probeCount };
		const uint32_t blocks = ( probeCount + kBlock - 1 ) / kBlock;
		if ( executor )
			executor->ParallelFor( "indirect.compose", blocks, &Block, &context );
		else
			for ( uint32_t b = 0; b < blocks; ++b )
				Block( &context, b );
		return volume;
	}

	// `base`'s interior texels, decoded once per base: kTexels RGB values of
	// `probe`'s layer 0 (total) or 1 (indirect).
	[[nodiscard]] const float *BaseInterior(
	    const Volume &base, uint32_t probeCount, uint32_t probe, uint32_t layer ) const
	{
		if ( m_baseBytes != base.bytes.data() || m_baseSize != base.bytes.size() )
			DecodeBase( base, probeCount );
		return &m_baseInterior[( size_t( probe ) * 2 + layer ) * kTexels * 3];
	}

private:
	struct Context
	{
		const ChangeComposer *self;
		Volume *volume;
		const float *total;
		const float *indirect;
		uint32_t probeCount;
	};

	void DecodeBase( const Volume &base, uint32_t probeCount ) const
	{
		const mapcontainer::ProbeVolumeLayout &layout = base.layout;
		const uint32_t tile = mapcontainer::kProbeIrradianceTile;
		m_baseInterior.assign( size_t( probeCount ) * 2 * kTexels * 3, 0.0f );
		uint32_t first = 0;
		for ( uint32_t g = 0; g < layout.gridCount; ++g )
		{
			const mapcontainer::ProbeGridLayout &grid = layout.grids[g];
			for ( uint32_t local = 0; local < grid.probeCount && first + local < probeCount; ++local )
			{
				for ( uint32_t layer = 0; layer < layout.layerCount && layer < 2; ++layer )
				{
					const uint32_t x0 =
					    grid.irradianceOrigin[layer][0] + ( local % grid.tilesPerRow ) * tile + 1;
					const uint32_t y0 =
					    grid.irradianceOrigin[layer][1] + ( local / grid.tilesPerRow ) * tile + 1;
					float *out =
					    &m_baseInterior[( size_t( first + local ) * 2 + layer ) * kTexels * 3];
					for ( uint32_t v = 0; v < kInterior; ++v )
						for ( uint32_t u = 0; u < kInterior; ++u )
							for ( int c = 0; c < 3; ++c )
							{
								uint16_t half;
								std::memcpy( &half,
								    base.bytes.data() + layout.atlasOffset +
								        ( uint64_t( y0 + v ) * layout.atlasWidth + x0 + u ) * 8 +
								        2 * c,
								    2 );
								out[( v * kInterior + u ) * 3 + c] = mapcontainer::HalfToFloat( half );
							}
				}
			}
			first += grid.probeCount;
		}
		m_baseBytes = base.bytes.data();
		m_baseSize = base.bytes.size();
	}

	static void Block( void *context, uint32_t block )
	{
		const Context &compose = *static_cast<const Context *>( context );
		const ChangeComposer &self = *compose.self;
		Volume &volume = *compose.volume;
		const mapcontainer::ProbeVolumeLayout &layout = volume.layout;
		const uint32_t begin = block * kBlock;
		const uint32_t end = std::min( begin + kBlock, compose.probeCount );
		uint32_t first = 0; // the global index of the grid's first probe
		for ( uint32_t g = 0; g < layout.gridCount; ++g )
		{
			const mapcontainer::ProbeGridLayout &grid = layout.grids[g];
			for ( uint32_t i = std::max( begin, first ); i < std::min( end, first + grid.probeCount );
			      ++i )
			{
				const float *total = compose.total + size_t( i ) * kTexels * 3;
				const float *indirect = compose.indirect + size_t( i ) * kTexels * 3;
				bool changed = false;
				for ( uint32_t k = 0; k < kTexels * 3 && !changed; ++k )
					changed = total[k] != 0.0f || indirect[k] != 0.0f;
				if ( !changed )
					continue;
				for ( uint32_t layer = 0; layer < layout.layerCount && layer < 2; ++layer )
					WriteTile( volume, grid.irradianceOrigin[layer], i - first, grid.tilesPerRow,
					    &self.m_baseInterior[( size_t( i ) * 2 + layer ) * kTexels * 3],
					    layer == 0 ? total : indirect );
			}
			first += grid.probeCount;
		}
	}

	static void WriteTile( Volume &volume, const uint32_t origin[2], uint32_t probe,
	    uint32_t tilesPerRow, const float *base, const float *change )
	{
		const uint32_t tile = mapcontainer::kProbeIrradianceTile;
		const uint32_t x0 = origin[0] + ( probe % tilesPerRow ) * tile;
		const uint32_t y0 = origin[1] + ( probe / tilesPerRow ) * tile;
		// Each interior texel is encoded once; the border repeats the halves.
		uint16_t interior[kInterior][kInterior][3];
		for ( uint32_t v = 0; v < kInterior; ++v )
			for ( uint32_t u = 0; u < kInterior; ++u )
				for ( int c = 0; c < 3; ++c )
				{
					const uint32_t k = ( v * kInterior + u ) * 3 + c;
					interior[v][u][c] = FloatToHalf( std::max( 0.0f, base[k] + change[k] ) );
				}
		const auto put = [&]( uint32_t x, uint32_t y, const uint16_t rgb[3] )
		{
			std::memcpy( volume.bytes.data() + volume.layout.atlasOffset +
			                 ( uint64_t( y0 + y ) * volume.layout.atlasWidth + x0 + x ) * 8,
			    rgb, 6 );
		};
		const uint32_t n = kInterior;
		for ( uint32_t v = 0; v < n; ++v )
			for ( uint32_t u = 0; u < n; ++u )
				put( 1 + u, 1 + v, interior[v][u] );
		for ( uint32_t k = 0; k < n; ++k )
		{
			put( 1 + k, 0, interior[0][n - 1 - k] );
			put( 1 + k, n + 1, interior[n - 1][n - 1 - k] );
			put( 0, 1 + k, interior[n - 1 - k][0] );
			put( n + 1, 1 + k, interior[n - 1 - k][n - 1] );
		}
		put( 0, 0, interior[n - 1][n - 1] );
		put( n + 1, 0, interior[n - 1][0] );
		put( 0, n + 1, interior[0][n - 1] );
		put( n + 1, n + 1, interior[0][0] );
	}

	mutable std::vector<float> m_baseInterior;
	mutable const unsigned char *m_baseBytes = nullptr;
	mutable size_t m_baseSize = 0;
};

// The world's change atlas under BakedPlusDelta: `published`'s atlas with its
// indirect layer's irradiance texels replaced by published minus baked.
[[nodiscard]] inline bool ChangeAtlas(
    const Volume &published, const Volume &baked, std::vector<unsigned char> *out )
{
	const mapcontainer::ProbeVolumeLayout &layout = published.layout;
	if ( layout.layerCount < 2 || !published.SameTopology( baked ) )
		return false;
	out->assign( published.bytes.begin() + layout.atlasOffset, published.bytes.end() );
	for ( uint32_t g = 0; g < layout.gridCount; ++g )
	{
		const mapcontainer::ProbeGridLayout &grid = layout.grids[g];
		const uint32_t rows = ( grid.probeCount + grid.tilesPerRow - 1 ) / grid.tilesPerRow;
		for ( uint32_t y = 0; y < rows * mapcontainer::kProbeIrradianceTile; ++y )
		{
			const uint64_t row = uint64_t( grid.irradianceOrigin[1][1] + y ) * layout.atlasWidth;
			for ( uint32_t x = 0; x < grid.tilesPerRow * mapcontainer::kProbeIrradianceTile; ++x )
			{
				const uint64_t texel = ( row + grid.irradianceOrigin[1][0] + x ) * 8;
				for ( int c = 0; c < 3; ++c )
				{
					uint16_t now, then;
					std::memcpy(
					    &now, published.bytes.data() + layout.atlasOffset + texel + 2 * c, 2 );
					std::memcpy(
					    &then, baked.bytes.data() + layout.atlasOffset + texel + 2 * c, 2 );
					const uint16_t change = FloatToHalf(
					    mapcontainer::HalfToFloat( now ) - mapcontainer::HalfToFloat( then ) );
					std::memcpy( out->data() + texel + 2 * c, &change, 2 );
				}
			}
		}
	}
	return true;
}

// One frame's work: CPU jobs the frame's executor runs (a producer never
// creates threads; a CPU producer's serial mode runs its jobs in order), the
// executor those jobs may fan out on (null: inline and serial), the frame's
// scene changes, and read-only GPU progress. There is no GPU wait.
struct FrameWork
{
	uint64_t frameSerial = 0;
	std::vector<std::function<void()>> jobs;
	IBatchExecutor *executor = nullptr;
	IResourceTracker *resources = nullptr;
	std::span<const SceneChange> changes;
	std::span<const Proxy> proxies;
	std::span<const LightOverride> lightOverrides;
	std::span<const Portal> portals; // this frame's open portals (G10)
	// Traced producers: the probes (indices into the volume's first grid) to
	// update every update, the camera's surroundings; empty means every
	// probe. The others take turns, `probeBudget` probes an update.
	std::span<const uint32_t> focusProbes;
	uint32_t probeBudget = 128;
};

struct RetireTicket
{
	uint64_t afterSerial = 0; // resources are free once the completion serial reaches this
};

class IProducer
{
public:
	virtual ~IProducer() = default;
	[[nodiscard]] virtual ProducerCaps Caps() const = 0;
	// Validates scene data and allocates. Fails without side effects.
	[[nodiscard]] virtual foundation::Expected<void, IndirectError> Begin(
	    const IndirectScene &scene, const PublishedVolume &seed, IResourceTracker &resources ) = 0;
	// Adds this frame's CPU jobs; never blocks on the GPU.
	virtual void Schedule( FrameWork &work, const light_set::Snapshot &lights ) = 0;
	// The newest complete volume, or none before the first publication.
	[[nodiscard]] virtual std::optional<PublishedVolume> Published() const = 0;
	// Stops scheduling; resources are released once the ticket completes.
	[[nodiscard]] virtual RetireTicket End() = 0;
};

// The Baked producer: publishes the map's PRBV once, as its seed. It
// responds to nothing, runs under the Baked policy only, needs no device
// feature, and is every other producer's seed and fallback.
class BakedProducer final : public IProducer
{
public:
	[[nodiscard]] ProducerCaps Caps() const override
	{
		ProducerCaps caps;
		caps.kind = ProducerKind::Baked;
		caps.seedTolerance = 0.0f;
		return caps;
	}
	[[nodiscard]] foundation::Expected<void, IndirectError> Begin(
	    const IndirectScene &scene, const PublishedVolume &, IResourceTracker & ) override
	{
		if ( !scene.baked )
			return foundation::MakeUnexpected( IndirectError::MissingSceneData );
		if ( scene.policy != indirect_policy::Policy::Baked )
			return foundation::MakeUnexpected( IndirectError::UnsupportedPolicy );
		m_published = PublishedVolume{ ++m_epoch, scene.baked };
		return {};
	}
	void Schedule( FrameWork &, const light_set::Snapshot & ) override {}
	[[nodiscard]] std::optional<PublishedVolume> Published() const override { return m_published; }
	[[nodiscard]] RetireTicket End() override
	{
		m_published.reset(); // CPU data only: the host retires its uploads
		return {};
	}

private:
	std::optional<PublishedVolume> m_published;
	uint64_t m_epoch = 0;
};

// The contract's scripted fake as the product composes it (offered only
// under -indirect_test_fake, never to players): after `delayFrames` it
// publishes the seed with its indirect light scaled by `indirectScale` and
// its total recomposed (the seed's direct light plus that indirect). It
// responds to nothing and runs under Baked and RuntimeIndirect. The native
// switching suite uses it for a visible, deterministic switch.
class ScriptedFakeProducer final : public IProducer
{
public:
	explicit ScriptedFakeProducer( float indirectScale = 1.2f, uint32_t delayFrames = 2 )
	    : m_indirectScale( indirectScale ), m_delayFrames( delayFrames )
	{
	}
	[[nodiscard]] ProducerCaps Caps() const override
	{
		ProducerCaps caps;
		caps.kind = ProducerKind::ScriptedFake;
		caps.policies = PolicyBit( indirect_policy::Policy::Baked ) |
		                PolicyBit( indirect_policy::Policy::RuntimeIndirect );
		caps.convergenceFrames = m_delayFrames;
		return caps;
	}
	[[nodiscard]] foundation::Expected<void, IndirectError> Begin(
	    const IndirectScene &scene, const PublishedVolume &seed, IResourceTracker & ) override
	{
		if ( !scene.baked )
			return foundation::MakeUnexpected( IndirectError::MissingSceneData );
		if ( !( Caps().policies & PolicyBit( scene.policy ) ) )
			return foundation::MakeUnexpected( IndirectError::UnsupportedPolicy );
		m_seed =
		    seed.volume && seed.volume->SameTopology( *scene.baked ) ? seed.volume : scene.baked;
		m_frames = 0;
		m_published.reset();
		return {};
	}
	void Schedule( FrameWork &work, const light_set::Snapshot & ) override
	{
		if ( m_published || !m_seed )
			return;
		work.jobs.push_back(
		    [this]
		    {
			    if ( ++m_frames >= m_delayFrames )
				    m_published = PublishedVolume{ ++m_epoch, Rescaled( *m_seed ) };
		    } );
	}
	[[nodiscard]] std::optional<PublishedVolume> Published() const override { return m_published; }
	[[nodiscard]] RetireTicket End() override
	{
		m_published.reset();
		m_seed.reset();
		return {};
	}

private:
	std::shared_ptr<const Volume> Rescaled( const Volume &seed ) const
	{
		auto volume = std::make_shared<Volume>( seed );
		if ( seed.layout.layerCount < 2 )
			return volume;
		for ( uint32_t g = 0; g < seed.layout.gridCount; ++g )
		{
			const mapcontainer::ProbeGridLayout &grid = seed.layout.grids[g];
			const uint32_t rows = ( grid.probeCount + grid.tilesPerRow - 1 ) / grid.tilesPerRow;
			for ( uint32_t y = 0; y < rows * mapcontainer::kProbeIrradianceTile; ++y )
			{
				for ( uint32_t x = 0; x < grid.tilesPerRow * mapcontainer::kProbeIrradianceTile;
				    ++x )
				{
					const auto offset = [&]( uint32_t layer )
					{
						return size_t( seed.layout.atlasOffset +
						               ( uint64_t( grid.irradianceOrigin[layer][1] + y ) *
						                       seed.layout.atlasWidth +
						                   grid.irradianceOrigin[layer][0] + x ) *
						                   8 );
					};
					for ( int c = 0; c < 3; ++c )
					{
						uint16_t total, indirect;
						std::memcpy( &total, seed.bytes.data() + offset( 0 ) + 2 * c, 2 );
						std::memcpy( &indirect, seed.bytes.data() + offset( 1 ) + 2 * c, 2 );
						const float seedIndirect = mapcontainer::HalfToFloat( indirect );
						const float scaled = seedIndirect * m_indirectScale;
						const uint16_t newIndirect = FloatToHalf( scaled );
						const uint16_t newTotal = FloatToHalf(
						    mapcontainer::HalfToFloat( total ) - seedIndirect + scaled );
						std::memcpy( volume->bytes.data() + offset( 1 ) + 2 * c, &newIndirect, 2 );
						std::memcpy( volume->bytes.data() + offset( 0 ) + 2 * c, &newTotal, 2 );
					}
				}
			}
		}
		return volume;
	}

	float m_indirectScale;
	uint32_t m_delayFrames;
	std::shared_ptr<const Volume> m_seed;
	std::optional<PublishedVolume> m_published;
	uint32_t m_frames = 0;
	uint64_t m_epoch = 0;
};

} // namespace indirect_light

#endif // RENDER_INDIRECT_LIGHT_H
