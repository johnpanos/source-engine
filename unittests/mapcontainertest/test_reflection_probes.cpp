//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance for RPRB reflection probes (R50-PARALLAX, Q-CONTENT).
//
//  - The checked-in fixture (tools/quality/reflection_probe_set.py fixture)
//    validates; its GPU buffer is byte-identical to the Python writer's.
//  - Every edit of the shared malformation corpus fails with the error the
//    independent Python reader reports; texel corruptions are rejected.
//  - The C++ reference blend reproduces every shading sample the Python
//    oracle recorded (blended, nearest and direction-only modes).
//  - Blend invariants: weights sum to one on at most two probes, walking
//    across an influence boundary is continuous while the nearest-capture
//    mode jumps, and a capture behind the surface gets no weight.
//  - Seeded mutation fuzzing never crashes the validator.
//  - RPRB v2 (R50-RELIGHT): the relight fixture validates and its GPU buffer
//    matches the Python writer's; its malformation corpus fails as in
//    Python; with the fixture's analytic light (now and baked) and its
//    moving occluder the reference reproduces the Python oracle's relit
//    samples; unchanged light reproduces the baked radiance exactly, the
//    occluder changes the lookups it hides and relighting is ignored without
//    a relight. The rule (removed light relative, added light absolute) and
//    the occluder test (entry, face normal, a start inside, a miss) are
//    checked on their own values.
//
//=============================================================================//

#include "mapcontainer/reflection_probes.h"
#include "mapcontainer/probe_volume.h"
#include "testing/conformance_result.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <random>
#include <sstream>
#include <string>
#include <vector>

using namespace mapcontainer;

namespace
{
unsigned long g_checks = 0;
unsigned long g_failures = 0;

void Check( bool condition, const std::string &what )
{
	++g_checks;
	if ( !condition )
	{
		++g_failures;
		std::printf( "FAIL %s\n", what.c_str() );
	}
}

const char *kFixtures = "quality/fixtures/reflection/rprb/";

std::vector<char> Load( const std::string &name )
{
	std::ifstream file( kFixtures + name, std::ios::binary );
	return std::vector<char>(
	    std::istreambuf_iterator<char>{ file }, std::istreambuf_iterator<char>{} );
}

void Put( std::vector<char> &bytes, size_t offset, const void *value, size_t size )
{
	std::memcpy( bytes.data() + offset, value, size );
}

ReflectionProbesError Validate( const std::vector<char> &bytes, ReflectionProbesLayout *layout )
{
	return ValidateReflectionProbes( bytes.data(), bytes.size(), layout );
}

// The decoded form (RPRB v8's radiance blocks expanded) and its layout, which
// the reference view takes; empty when invalid.
std::vector<std::byte> Decoded( const std::vector<char> &bytes, ReflectionProbesLayout *layout )
{
	std::vector<std::byte> decoded;
	if ( DecodeReflectionProbes( bytes.data(), bytes.size(), &decoded, layout ) !=
	     ReflectionProbesError::Ok )
		decoded.clear();
	return decoded;
}

void CheckFixture( const std::vector<char> &valid, ReflectionProbesLayout *layout )
{
	Check( !valid.empty(), "fixture valid.rprb is present" );
	Check( Validate( valid, layout ) == ReflectionProbesError::Ok, "fixture validates" );
	Check( layout->count == 2 && layout->faceSize == 16 && layout->mipCount == 2,
	    "fixture layout: two probes, face 16, two mips" );
	Check( layout->globalIndex == 1 && layout->probes[1].rank == 1 && layout->probes[0].rank == 0,
	    "the global probe ranks last" );
}

int ApplyCorpus( const std::vector<char> &valid, const char *name )
{
	std::ifstream corpus( std::string( kFixtures ) + name );
	std::string line;
	int cases = 0;
	while ( std::getline( corpus, line ) )
	{
		if ( line.empty() || line[0] == '#' )
			continue;
		std::istringstream fields( line );
		size_t offset;
		std::string format, value, expected;
		fields >> offset >> format >> value >> expected;
		std::vector<char> bytes = valid;
		if ( format == "I" )
		{
			const uint32_t v = uint32_t( std::stoul( value ) );
			Put( bytes, offset, &v, 4 );
		}
		else if ( format == "Q" )
		{
			const uint64_t v = std::stoull( value );
			Put( bytes, offset, &v, 8 );
		}
		else if ( format == "e" )
		{
			const uint16_t v = FloatToHalf( std::stof( value ) );
			Put( bytes, offset, &v, 2 );
		}
		else
		{
			const float v = value == "nan" ? NAN : std::stof( value );
			Put( bytes, offset, &v, 4 );
		}
		const ReflectionProbesError error = Validate( bytes, nullptr );
		Check( expected == ReflectionProbesErrorName( error ),
		    "malformation at " + std::to_string( offset ) + " (" + value + ") is " + expected +
		        ", got " + ReflectionProbesErrorName( error ) );
		++cases;
	}
	return cases;
}

void CheckMalformations( const std::vector<char> &valid )
{
	const int cases = ApplyCorpus( valid, "malformations.txt" );
	Check( cases >= 20, "the shared corpus has its cases" );
	Check(
	    ValidateReflectionProbes( valid.data(), 63, nullptr ) == ReflectionProbesError::Truncated,
	    "a short header is Truncated" );
	std::vector<char> shorter( valid.begin(), valid.end() - 2 );
	Check( Validate( shorter, nullptr ) == ReflectionProbesError::InvalidSections,
	    "a short atlas is InvalidSections" );
}

// Texel checks run on the relight cubes (RGBA16F; any BC6H radiance block
// decodes to finite, non-negative light). The albedo array's first texel is
// probe 0's, face +X (alpha its distance); the normal array's alpha is 1.
void CheckTexels()
{
	const std::vector<char> relit = Load( "valid-relight.rprb" );
	ReflectionProbesLayout layout = {};
	if ( Validate( relit, &layout ) != ReflectionProbesError::Ok )
	{
		Check( false, "the relight fixture validates for the texel checks" );
		return;
	}
	uint64_t albedoAt, normalAt, bytes;
	ReflectionProbeMipRange( layout, ReflectionProbeArray::kAlbedo, 0, &albedoAt, &bytes );
	ReflectionProbeMipRange( layout, ReflectionProbeArray::kNormal, 0, &normalAt, &bytes );
	albedoAt += layout.dataOffset;
	normalAt += layout.dataOffset;
	const auto corrupt = [&]( size_t at, uint16_t half, const char *what )
	{
		std::vector<char> edited = relit;
		Put( edited, at, &half, 2 );
		Check( Validate( edited, nullptr ) == ReflectionProbesError::InvalidTexels, what );
	};
	corrupt( albedoAt + 3 * 8 + 0, 0x7c00u, "an infinite texel is rejected" );
	corrupt( albedoAt + 3 * 8 + 2, 0x7e00u, "a NaN texel is rejected" );
	corrupt( albedoAt + 3 * 8 + 4, 0xbc00u, "a negative albedo is rejected" );
	corrupt( normalAt + 3 * 8 + 6, 0x3800u, "a normal texel's alpha must be 1" );
	std::vector<char> edited = relit;
	const uint16_t negativeZero = 0x8000u;
	Put( edited, albedoAt + 3 * 8, &negativeZero, 2 );
	Check( Validate( edited, nullptr ) == ReflectionProbesError::Ok, "negative zero is zero" );
}

void CheckDecode( const std::vector<char> &valid, const ReflectionProbesLayout &layout )
{
	ReflectionProbesLayout decodedLayout = {};
	const std::vector<std::byte> decoded = Decoded( valid, &decodedLayout );
	// Two probes, six faces: mip 0 is 16 x 16 (4 x 4 blocks), mip 1 8 x 8 (2 x 2).
	const uint64_t blocks = 2 * 6 * ( 16 + 4 ) * 16;
	const uint64_t texels = 2 * 6 * ( 256 + 64 );
	Check( layout.radianceBlockBytes == blocks && layout.dataBytes == blocks &&
	           decodedLayout.decoded && decodedLayout.radianceBlockBytes == 0 &&
	           decodedLayout.dataBytes == texels * 8 &&
	           decoded.size() == layout.dataOffset + texels * 8,
	    "the decoded radiance is RGBA16F; the lump's is BC6H, 1 byte per texel" );
	uint64_t at, bytes;
	ReflectionProbeMipRange( decodedLayout, ReflectionProbeArray::kRadianceHalf, 1, &at, &bytes );
	Check( at == 2 * 6 * 256 * 8 && bytes == 2 * 6 * 64 * 8,
	    "a mip's range follows the earlier mips, mip-major" );
	uint16_t alpha;
	std::memcpy( &alpha, decoded.data() + layout.dataOffset + 3 * 8 + 6, 2 );
	Check( alpha == 0x3c00u, "decoded alpha is 1" );
	Check( DecodeReflectionProbes( valid.data(), 63, nullptr, nullptr ) ==
	           ReflectionProbesError::Truncated,
	    "decoding validates first" );
}

// The GPU buffer: byte-identical to the Python writer's, probe records in rank
// order as exact float32, the candidate masks as lo/hi word pairs.
void CheckGpuBuffer( const std::vector<char> &valid, const ReflectionProbesLayout &layout )
{
	const std::vector<char> expected = Load( "gpu-mode1.u32" );
	const uint32_t words = ReflectionProbeBufferWords( layout );
	Check( words == kReflectionProbeBufferMasksWord + 2 * kReflectionProbeCandidateCells * 1,
	    "buffer words: header, probe records, candidate masks" );
	std::vector<uint32_t> buffer( words + 4, 0x12345678u );
	WriteReflectionProbeBuffer( valid.data(), layout, ReflectionProbeMode::Blend, buffer.data() );
	Check( expected.size() == size_t( words ) * 4 &&
	           std::memcmp( expected.data(), buffer.data(), expected.size() ) == 0,
	    "GPU buffer is byte-identical to reflection_probe_set.gpu_buffer" );
	Check( buffer[words] == 0x12345678u && buffer[words + 3] == 0x12345678u,
	    "the buffer writer stays within ReflectionProbeBufferWords" );
	Check( buffer[0] == 2 && buffer[1] == 2 && buffer[2] == 16 && buffer[4] == 1 && buffer[5] == 0,
	    "header: count, mips, face, mode, no relight" );
	for ( uint32_t i = 0; i < layout.count; ++i )
	{
		float record[20];
		std::memcpy( record,
		    buffer.data() + kReflectionProbeBufferProbesWord +
		        size_t( layout.probes[i].rank ) * kReflectionProbeBufferRecordWords,
		    sizeof( record ) );
		for ( int axis = 0; axis < 3; ++axis )
			Check( record[axis] == layout.probes[i].capture[axis], "a capture is exact float32" );
		Check( record[7] == float( i ), "the record names its cube layer" );
	}
	WriteReflectionProbeBuffer( valid.data(), layout, ReflectionProbeMode::Nearest, buffer.data() );
	Check( buffer[4] == 2, "the mode word follows the mode" );
}

ReflectionProbeMode ModeOf( float value )
{
	return static_cast<ReflectionProbeMode>( uint32_t( value ) );
}

// A conservative 150-probe subset of the independent 256-probe fixture.
// All cells include all retained ranks; the captured texels are unchanged.
std::vector<char> PartialCandidateFixture()
{
	const auto source = Load( "candidates256.rprb" );
	ReflectionProbesLayout original{};
	if ( Validate( source, &original ) != ReflectionProbesError::Ok || original.relight )
		return {};
	const uint32_t count = 150;
	const uint64_t gridOffset =
	    ( kReflectionProbesHeaderBytes + count * kReflectionProbeRecordBytes + 15 ) &
	    ~uint64_t( 15 );
	const uint32_t words = ( count + 63 ) / 64;
	const uint64_t dataOffset = gridOffset + ReflectionProbeCandidateBytes( count );
	// v8 radiance: BC6H, mip-major, then probe, then face.
	uint64_t perProbe[kReflectionProbesMaxMips] = {};
	uint64_t dataBytes = 0;
	for ( uint32_t level = 0; level < original.mipCount; ++level )
	{
		const uint64_t blocks = ( original.faceSize >> level ) / 4;
		perProbe[level] = 6 * blocks * blocks * 16;
		dataBytes += count * perProbe[level];
	}
	std::vector<char> bytes( dataOffset + dataBytes );
	std::copy_n( source.begin(), kReflectionProbesHeaderBytes, bytes.begin() );
	Put( bytes, 8, &count, 4 );
	Put( bytes, 32, &dataOffset, 8 );
	Put( bytes, 40, &dataBytes, 8 );
	const uint32_t global = count - 1;
	Put( bytes, 56, &global, 4 );
	for ( uint32_t rank = 0; rank < count; ++rank )
	{
		uint32_t index = original.globalIndex;
		if ( rank != global )
			for ( uint32_t i = 0; i < original.count; ++i )
				if ( original.probes[i].rank == rank )
					index = i;
		const size_t record = kReflectionProbesHeaderBytes + rank * kReflectionProbeRecordBytes;
		std::copy_n(
		    source.begin() + kReflectionProbesHeaderBytes + index * kReflectionProbeRecordBytes,
		    kReflectionProbeRecordBytes, bytes.begin() + record );
		Put( bytes, record + 64, &rank, 4 );
		Put( bytes, record + 72, &rank, 4 ); // the layer is the record's index
		uint64_t from = original.dataOffset, to = dataOffset;
		for ( uint32_t level = 0; level < original.mipCount; ++level )
		{
			std::copy_n( source.begin() + from + index * perProbe[level], perProbe[level],
			    bytes.begin() + to + rank * perProbe[level] );
			from += original.count * perProbe[level];
			to += count * perProbe[level];
		}
	}
	std::copy_n( source.begin() + original.candidateOffset, kReflectionProbeCandidateHeaderBytes,
	    bytes.begin() + gridOffset );
	Put( bytes, gridOffset + 20, &words, 4 );
	for ( uint32_t cell = 0; cell < kReflectionProbeCandidateCells; ++cell )
		for ( uint32_t rank = 0; rank < count; ++rank )
			bytes[gridOffset + kReflectionProbeCandidateHeaderBytes + size_t( cell ) * words * 8 +
			      rank / 8] |= char( 1u << ( rank % 8 ) );
	return bytes;
}

void CheckCandidates()
{
	{
		auto old = Load( "candidates64.rprb" );
		for ( uint32_t version : { 0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 9u } )
		{
			auto changed = old;
			Put( changed, 4, &version, 4 );
			Check( Validate( changed, nullptr ) == ReflectionProbesError::UnsupportedVersion,
			    "RPRB v" + std::to_string( version ) + " is refused (v8 only)" );
		}
	}
	for ( const char *name : { "candidates64.rprb", "candidates150", "candidates256.rprb" } )
	{
		const auto bytes =
		    std::string( name ) == "candidates150" ? PartialCandidateFixture() : Load( name );
		ReflectionProbesLayout layout{};
		const bool valid = Validate( bytes, &layout ) == ReflectionProbesError::Ok;
		Check( valid, std::string( name ) + " validates" );
		if ( !valid )
			continue;
		const uint32_t words = ( layout.count + 63 ) / 64;
		Check( layout.candidateOffset && layout.candidateWords == words,
		    "candidate word count follows the probe count" );
		std::vector<uint32_t> buffer( ReflectionProbeBufferWords( layout ) + 16, 0x1234u );
		WriteReflectionProbeBuffer(
		    bytes.data(), layout, ReflectionProbeMode::Blend, buffer.data() );
		Check( buffer[3] == words, "GPU header declares every candidate word" );
		const size_t maskWords = size_t( kReflectionProbeCandidateCells ) * words * 2;
		bool matches = true;
		for ( size_t i = 0; matches && i < maskWords; ++i )
		{
			uint32_t expected;
			std::memcpy( &expected,
			    bytes.data() + size_t( layout.candidateOffset ) +
			        kReflectionProbeCandidateHeaderBytes + 4 * i,
			    4 );
			matches = buffer[kReflectionProbeBufferMasksWord + i] == expected;
		}
		Check( matches, "all serialized candidate bytes survive GPU packing in cell order" );
		Check( std::all_of( buffer.end() - 16, buffer.end(),
		           []( uint32_t value )
		           {
			           return value == 0x1234u;
		           } ),
		    "GPU packing stays within the declared buffer" );
		for ( uint32_t rank : { 0u, 63u, 64u, 127u, 128u, 191u, 192u, 255u } )
		{
			if ( rank >= layout.count )
				continue;
			auto missing = bytes;
			// Remove this rank from every cell, independently of spatial indexing.
			for ( uint32_t cell = 0; cell < kReflectionProbeCandidateCells; ++cell )
				missing[size_t( layout.candidateOffset ) + kReflectionProbeCandidateHeaderBytes +
				        ( size_t( cell ) * words * 8 ) +
				        rank / 8] &= char( ~( 1u << ( rank % 8 ) ) );
			auto unchanged = layout;
			Check( Validate( missing, &unchanged ) == ReflectionProbesError::InvalidCandidates,
			    "missing candidate rank " + std::to_string( rank ) + " is rejected" );
			Check( std::memcmp( &layout, &unchanged, sizeof( layout ) ) == 0,
			    "failed validation preserves the caller's layout" );
		}
		if ( layout.count == 150 )
		{
			for ( uint32_t rank : { 150u, 191u } )
			{
				auto extra = bytes;
				extra[size_t( layout.candidateOffset ) + kReflectionProbeCandidateHeaderBytes +
				      rank / 8] |= char( 1u << ( rank % 8 ) );
				Check( Validate( extra, nullptr ) == ReflectionProbesError::InvalidCandidates,
				    "partial mask rejects undeclared rank " + std::to_string( rank ) );
			}
		}
		{
			auto wrong = bytes;
			const uint32_t badWords = words + 1;
			Put( wrong, layout.candidateOffset + 20, &badWords, 4 );
			Check( Validate( wrong, nullptr ) == ReflectionProbesError::InvalidCandidates,
			    "a grid header naming the wrong word count is rejected" );
		}
	}
}

void CheckSamples( const ReflectionProbesView &view )
{
	std::ifstream samples( std::string( kFixtures ) + "samples.txt" );
	std::string line;
	int count = 0;
	double worst = 0.0;
	while ( std::getline( samples, line ) )
	{
		if ( line.empty() || line[0] == '#' )
			continue;
		std::istringstream fields( line );
		float mode, position[3], normal[3], reflected[3], roughness, expected[3];
		fields >> mode;
		for ( float &v : position )
			fields >> v;
		for ( float &v : normal )
			fields >> v;
		for ( float &v : reflected )
			fields >> v;
		fields >> roughness;
		for ( float &v : expected )
			fields >> v;
		float radiance[3];
		view.Radiance( position, normal, reflected, roughness, ModeOf( mode ), radiance );
		for ( int c = 0; c < 3; ++c )
			worst = std::max( worst, double( std::fabs( radiance[c] - expected[c] ) ) );
		++count;
	}
	Check( count == 192, "samples.txt has 48 samples per mode (blend, nearest, direction-only, "
	                     "blend weights)" );
	// The Python oracle runs in double precision; float32 lookups can land on
	// the other side of a stripe edge by a rounding, so the bound is the
	// largest texel step (0.8) times the chance of that, not zero: judged
	// here as a worst-case absolute error.
	std::printf( "reference blend worst absolute error %.6f over %d samples\n", worst, count );
	Check( worst < 0.02, "the C++ reference blend reproduces the Python oracle's samples" );
}

void CheckBlend( const ReflectionProbesView &view )
{
	const float up[3] = { 0.0f, 0.0f, 1.0f };
	const float scale = 39.37007874015748f;
	float previous[kReflectionProbesMaxProbes] = {};
	float previousNearest[kReflectionProbesMaxProbes] = {};
	float largestStep = 0.0f, largestNearestStep = 0.0f;
	bool sums = true, pairs = true;
	const int steps = 241;
	const float stepLength = ( 5.8f - 0.2f ) / float( steps - 1 ) * scale;
	for ( int i = 0; i < steps; ++i )
	{
		const float position[3] = {
		    ( 0.2f + ( 5.8f - 0.2f ) * float( i ) / float( steps - 1 ) ) * scale, 2.0f * scale,
		    0.0f };
		float weights[kReflectionProbesMaxProbes], nearest[kReflectionProbesMaxProbes];
		view.Weights( position, up, ReflectionProbeMode::Blend, weights );
		view.Weights( position, up, ReflectionProbeMode::Nearest, nearest );
		float sum = 0.0f;
		int used = 0;
		for ( float w : weights )
		{
			sum += w;
			used += w > 0.0f ? 1 : 0;
		}
		sums = sums && std::fabs( sum - 1.0f ) < 1e-5f;
		pairs = pairs && used <= 2;
		if ( i > 0 )
			for ( uint32_t k = 0; k < kReflectionProbesMaxProbes; ++k )
			{
				largestStep = std::max( largestStep, std::fabs( weights[k] - previous[k] ) );
				largestNearestStep =
				    std::max( largestNearestStep, std::fabs( nearest[k] - previousNearest[k] ) );
			}
		std::memcpy( previous, weights, sizeof( previous ) );
		std::memcpy( previousNearest, nearest, sizeof( previousNearest ) );
	}
	Check( sums, "blend weights sum to one" );
	Check( pairs, "at most two probes are sampled" );
	// The smoothstep's steepest slope is 1.5 / fade (fade 0.5 m).
	Check( largestStep <= 1.5f / ( 0.5f * scale ) * stepLength * 1.01f,
	    "walking across the influence boundary is continuous" );
	Check( largestNearestStep == 1.0f, "the nearest-capture mode jumps (negative control)" );
	// A wall at x = 2.9 m inside probe 0's influence: facing -x its capture
	// (x = 1.5 m) is in front, facing +x it is behind.
	const float wall[3] = { 2.9f * scale, 2.0f * scale, 1.0f * scale };
	const float towardProbe[3] = { -1.0f, 0.0f, 0.0f };
	const float awayFromProbe[3] = { 1.0f, 0.0f, 0.0f };
	float weights[kReflectionProbesMaxProbes];
	view.Weights( wall, towardProbe, ReflectionProbeMode::Blend, weights );
	Check( weights[0] == 1.0f, "a surface facing the capture takes its probe" );
	view.Weights( wall, awayFromProbe, ReflectionProbeMode::Blend, weights );
	Check(
	    weights[0] == 0.0f && weights[1] == 1.0f, "a capture behind the surface gets no weight" );
	float radiance[3];
	view.Radiance( wall, towardProbe, towardProbe, 0.3f, ReflectionProbeMode::Off, radiance );
	Check( radiance[0] == 0.0f && radiance[1] == 0.0f && radiance[2] == 0.0f,
	    "the off mode samples nothing" );
	// The weight view paints the rank's palette colour: probe 0 (rank 0) red
	// where it takes the whole weight, the global probe (rank 1) blue.
	view.Radiance(
	    wall, towardProbe, towardProbe, 0.3f, ReflectionProbeMode::BlendWeights, radiance );
	Check( radiance[0] == 1.0f && radiance[1] == 0.0f && radiance[2] == 0.0f,
	    "the weight view paints a whole weight in its rank's colour" );
	view.Radiance(
	    wall, awayFromProbe, towardProbe, 0.3f, ReflectionProbeMode::BlendWeights, radiance );
	Check( radiance[0] == 0.0f && radiance[2] == 1.0f, "the global probe paints blue" );
	Check( ReflectionProbeModeValid( 5 ) && !ReflectionProbeModeValid( 4 ) &&
	           !ReflectionProbeModeValid( 8 ),
	    "valid modes are 0..3 and 5..7" );
}

// reflection_probe_set.fixture_light: the analytic diffuse light (now and
// baked) the relit samples were computed with (Source units).
void FixtureLight( void *, const float p[3], const float n[3], float now[3], float baked[3] )
{
	baked[0] = 0.3f + 0.001f * p[0];
	baked[1] = 0.25f + 0.1f * std::fabs( n[2] );
	baked[2] = 0.2f + 0.0005f * p[2];
	now[0] = baked[0] + 0.002f * p[0] + 0.5f * std::max( n[2], 0.0f ) - 0.45f;
	now[1] = baked[1] + 0.3f - 0.003f * p[1];
	now[2] = baked[2] + 0.25f * n[0] - 0.1f;
}

// The light as baked: nothing changed.
void UnchangedLight( void *, const float p[3], const float n[3], float now[3], float baked[3] )
{
	float ignored[3];
	FixtureLight( nullptr, p, n, ignored, baked );
	for ( int c = 0; c < 3; ++c )
		now[c] = baked[c];
}

// reflection_probe_set.FIXTURE_OCCLUDER in Source units.
const float kUnitsPerMeter = 39.37007874015748f;
const ReflectionProbeOccluder kFixtureOccluder = {
    { 2.8f * kUnitsPerMeter, 1.2f * kUnitsPerMeter, 0.0f },
    { 3.2f * kUnitsPerMeter, 2.8f * kUnitsPerMeter, 2.2f * kUnitsPerMeter }, 0.4f };

void CheckRelightRule()
{
	Check( ReflectionProbeRelit( 0.2f, 0.5f, 0.1f, 0.4f ) == 0.05f,
	    "light removed scales the capture: 0.2 * 0.1 / 0.4" );
	Check( ReflectionProbeRelit( 0.2f, 0.5f, 0.4f, 0.2f ) == 0.2f + 0.5f * 0.2f,
	    "light added is albedo times the increase" );
	Check( ReflectionProbeRelit( 0.2f, 0.5f, -1.0f, 0.4f ) == 0.0f &&
	           ReflectionProbeRelit( 0.2f, 0.5f, 0.0f, 1e-5f ) == 0.2f - 0.5f * 1e-5f,
	    "relit light clamps at zero; below the floor a removal is absolute" );
	const ReflectionProbeOccluder box = { { 10, -5, -5 }, { 12, 5, 5 }, 0.3f };
	const float origin[3] = { 0, 0, 0 }, along[3] = { 1, 0, 0 };
	float t = -1, normal[3] = {};
	uint32_t index = 99;
	Check( ReflectionProbeOccluded( &box, 1, origin, along, 100, &t, normal, &index ) &&
	           t == 10 && normal[0] == -1 && normal[1] == 0 && normal[2] == 0 && index == 0,
	    "an occluder on the segment: its entry and the entered face's outward normal" );
	Check( !ReflectionProbeOccluded( &box, 1, origin, along, 9.5f, &t, normal, &index ),
	    "an occluder beyond the point seen hides nothing" );
	const float inside[3] = { 11, 0, 0 }, back[3] = { -1, 0, 0 };
	Check( ReflectionProbeOccluded( &box, 1, inside, back, 50, &t, normal, &index ) && t == 0 &&
	           normal[0] == 1,
	    "a segment starting inside an occluder is hidden at once, facing back along itself" );
	const float up[3] = { 0, 0, 1 };
	Check( !ReflectionProbeOccluded( &box, 1, origin, up, 100, &t, normal, &index ),
	    "a segment that misses the box is not hidden" );
}

void CheckRelight()
{
	const std::vector<char> relit = Load( "valid-relight.rprb" );
	ReflectionProbesLayout layout = {};
	Check( !relit.empty() && Validate( relit, &layout ) == ReflectionProbesError::Ok,
	    "the relight fixture (v2) validates" );
	if ( !layout.relight )
	{
		Check( false, "the relight fixture carries relight bands" );
		return;
	}
	Check( layout.probes[0].relightLayer == 0 && layout.probes[1].relightLayer == 1 &&
	           layout.dataBytes == layout.radianceBlockBytes + 2 * 2 * 6 * ( 256 + 64 ) * 8,
	    "relight cubes follow the radiance cubes: albedo array, then normal array" );
	const int cases = ApplyCorpus( relit, "relight-malformations.txt" );
	Check( cases >= 9, "the relight corpus has its cases" );
	// The GPU form: relight layers in the records, the switch in word 5.
	const std::vector<char> expected = Load( "gpu-relight-mode1.u32" );
	std::vector<uint32_t> buffer( ReflectionProbeBufferWords( layout ) );
	ReflectionProbesLayout decodedLayout = {};
	const std::vector<std::byte> decoded = Decoded( relit, &decodedLayout );
	WriteReflectionProbeBuffer( relit.data(), layout, ReflectionProbeMode::Blend, buffer.data() );
	Check( expected.size() == buffer.size() * 4 &&
	           std::memcmp( expected.data(), buffer.data(), expected.size() ) == 0,
	    "the relight GPU buffer is byte-identical to reflection_probe_set.gpu_buffer" );
	Check( buffer[5] == 1, "word 5 turns relighting on" );
	WriteReflectionProbeBuffer(
	    relit.data(), layout, ReflectionProbeMode::Blend, buffer.data(), false );
	Check( buffer[5] == 0, "relight false turns it off" );
	// The reference against the Python oracle's relit samples.
	const ReflectionProbesView view( decoded.data(), decodedLayout );
	ReflectionProbeRelight relight = { FixtureLight, nullptr, &kFixtureOccluder, 1 };
	ReflectionProbeRelight open = { FixtureLight, nullptr, nullptr, 0 };
	const ReflectionProbeRelight unchanged = { UnchangedLight, nullptr, nullptr, 0 };
	std::ifstream samples( std::string( kFixtures ) + "relight-samples.txt" );
	std::string line;
	int count = 0;
	double worst = 0.0, identity = 0.0, moved = 0.0;
	int occluded = 0;
	while ( std::getline( samples, line ) )
	{
		if ( line.empty() || line[0] == '#' )
			continue;
		std::istringstream fields( line );
		float mode, position[3], normal[3], reflected[3], roughness, expectedRadiance[3];
		fields >> mode;
		for ( float &v : position )
			fields >> v;
		for ( float &v : normal )
			fields >> v;
		for ( float &v : reflected )
			fields >> v;
		fields >> roughness;
		for ( float &v : expectedRadiance )
			fields >> v;
		float radiance[3], baked[3], same[3], unoccluded[3];
		view.Radiance( position, normal, reflected, roughness, ModeOf( mode ), radiance, &relight );
		view.Radiance( position, normal, reflected, roughness, ModeOf( mode ), baked );
		view.Radiance(
		    position, normal, reflected, roughness, ModeOf( mode ), same, &unchanged );
		view.Radiance(
		    position, normal, reflected, roughness, ModeOf( mode ), unoccluded, &open );
		bool differs = false;
		for ( int c = 0; c < 3; ++c )
		{
			worst = std::max( worst, double( std::fabs( radiance[c] - expectedRadiance[c] ) ) );
			identity = std::max( identity, double( std::fabs( same[c] - baked[c] ) ) );
			moved = std::max( moved, double( std::fabs( radiance[c] - baked[c] ) ) );
			differs |= std::fabs( radiance[c] - unoccluded[c] ) > 1e-4f;
		}
		occluded += differs ? 1 : 0;
		++count;
	}
	std::printf( "relit reference worst absolute error %.6f over %d samples; unchanged light "
	             "%.6g; largest relight %.4f; %d samples see the occluder\n",
	    worst, count, identity, moved, occluded );
	Check( count == 144, "relight-samples.txt has 48 samples per mode" );
	Check( worst < 0.02, "the C++ relit reference reproduces the Python oracle's samples" );
	Check( identity == 0.0, "unchanged light reproduces the baked radiance exactly" );
	Check( moved > 0.1, "the fixture's light moves the radiance (the check can fail)" );
	Check( occluded >= 5, "the moving occluder changes the lookups it hides" );
}

void CheckFuzz( const std::vector<char> &valid )
{
	std::mt19937 rng( 20260925 );
	int accepted = 0;
	bool finite = true;
	for ( int round = 0; round < 3000; ++round )
	{
		std::vector<char> bytes = valid;
		const int edits = 1 + int( rng() % 4 );
		for ( int e = 0; e < edits; ++e )
		{
			// Mostly the header and records, where structure lives.
			const size_t limit = rng() % 4 == 0 ? bytes.size() : size_t( 64 + 2 * 80 );
			bytes[rng() % limit] = char( rng() );
		}
		ReflectionProbesLayout layout;
		const std::vector<std::byte> decoded = Decoded( bytes, &layout );
		if ( decoded.empty() )
			continue;
		++accepted;
		const ReflectionProbesView view( decoded.data(), layout );
		const float position[3] = { 100.0f, 80.0f, 10.0f };
		const float normal[3] = { 0.0f, 0.0f, 1.0f };
		const float reflected[3] = { 0.6f, 0.0f, 0.8f };
		float radiance[3];
		view.Radiance( position, normal, reflected, 0.4f, ReflectionProbeMode::Blend, radiance );
		for ( float value : radiance )
			finite = finite && std::isfinite( value ) && value >= 0.0f;
	}
	std::printf( "fuzz: %d of 3000 mutants accepted\n", accepted );
	Check( true, "mutation fuzzing completed without a crash" );
	Check( finite, "accepted mutants shade to finite, non-negative radiance" );
}

} // namespace

int main()
{
	const std::vector<char> valid = Load( "valid.rprb" );
	ReflectionProbesLayout layout = {};
	CheckFixture( valid, &layout );
	if ( g_failures == 0 )
	{
		CheckMalformations( valid );
		CheckTexels();
		CheckDecode( valid, layout );
		CheckGpuBuffer( valid, layout );
		CheckCandidates();
		ReflectionProbesLayout decodedLayout = {};
		const std::vector<std::byte> decoded = Decoded( valid, &decodedLayout );
		const ReflectionProbesView view( decoded.data(), decodedLayout );
		CheckSamples( view );
		CheckBlend( view );
		CheckFuzz( valid );
		CheckRelight();
		CheckRelightRule();
		CheckFuzz( Load( "valid-relight.rprb" ) );
	}
	return testing::ReportConformance( g_checks, g_failures );
}
