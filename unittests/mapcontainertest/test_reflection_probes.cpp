//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance for RPRB reflection probes (R50-PARALLAX, Q-CONTENT).
//
//  - The checked-in fixture (tools/quality/reflection_probe_set.py fixture)
//    validates; its GPU texture is byte-identical to the Python writer's.
//  - Every edit of the shared malformation corpus fails with the error the
//    independent Python reader reports; texel corruptions are rejected.
//  - The C++ reference blend reproduces every shading sample the Python
//    oracle recorded (blended, nearest and direction-only modes).
//  - Blend invariants: weights sum to one on at most two probes, walking
//    across an influence boundary is continuous while the nearest-capture
//    mode jumps, and a capture behind the surface gets no weight.
//  - Seeded mutation fuzzing never crashes the validator.
//  - RPRB v2 (R50-RELIGHT): the relight fixture validates and its GPU texture
//    matches the Python writer's; its malformation corpus fails as in
//    Python; with the fixture's analytic change the reference reproduces
//    the Python oracle's relit samples; a zero change reproduces the baked
//    radiance exactly, and relighting is ignored without a change.
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

void CheckFixture( const std::vector<char> &valid, ReflectionProbesLayout *layout )
{
	Check( !valid.empty(), "fixture valid.rprb is present" );
	Check( Validate( valid, layout ) == ReflectionProbesError::Ok, "fixture validates" );
	Check( layout->count == 2 && layout->width == 32 && layout->atlasWidth == 64 &&
	           layout->atlasHeight == 32,
	    "fixture layout: two probes, width 32" );
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

void CheckTexels( const std::vector<char> &valid, const ReflectionProbesLayout &layout )
{
	const size_t atlas = size_t( layout.atlasOffset );
	const auto corrupt = [&]( size_t texel, int channel, uint16_t half, const char *what )
	{
		std::vector<char> bytes = valid;
		Put( bytes, atlas + texel * 8 + 2 * channel, &half, 2 );
		Check( Validate( bytes, nullptr ) == ReflectionProbesError::InvalidTexels, what );
	};
	corrupt( 3, 0, 0x7c00u, "an infinite texel is rejected" );
	corrupt( 3, 1, 0x7e00u, "a NaN texel is rejected" );
	corrupt( 3, 2, 0xbc00u, "a negative colour is rejected" );
	corrupt( 3, 3, 0x3800u, "a mip texel's alpha must be 1" );
	corrupt( layout.atlasWidth - 1, 0, 0x3c00u, "a texel outside every mip must be 0" );
	std::vector<char> bytes = valid;
	const uint16_t negativeZero = 0x8000u;
	Put( bytes, atlas + 3 * 8, &negativeZero, 2 );
	Check( Validate( bytes, nullptr ) == ReflectionProbesError::Ok, "negative zero is zero" );
}

void CheckGpuTexture( const std::vector<char> &valid, const ReflectionProbesLayout &layout )
{
	const std::vector<char> expected = Load( "gpu-mode1.rgba16f" );
	const uint32_t rows = ReflectionProbeTextureRows( layout );
	Check( rows == 1 + 2 + 32, "texture rows: header, table, atlas" );
	std::vector<uint16_t> texture( size_t( layout.atlasWidth ) * rows * 4 );
	WriteReflectionProbeTexture( valid.data(), layout, ReflectionProbeMode::Blend, texture.data() );
	Check( expected.size() == texture.size() * 2 &&
	           std::memcmp( expected.data(), texture.data(), expected.size() ) == 0,
	    "GPU texture is byte-identical to reflection_probe_set.gpu_texture" );
	Check( HalfToFloat( texture[3] ) == kReflectionProbeTextureMarker,
	    "the header texel carries the marker" );
	// hi + lo keeps capture positions within 0.02 units.
	for ( uint32_t i = 0; i < layout.count; ++i )
	{
		const uint16_t *row =
		    texture.data() + size_t( 1 + layout.probes[i].rank ) * layout.atlasWidth * 4;
		for ( int axis = 0; axis < 3; ++axis )
		{
			const float decoded = HalfToFloat( row[axis] ) + HalfToFloat( row[4 + axis] );
			Check( std::fabs( decoded - layout.probes[i].capture[axis] ) < 0.02f,
			    "capture survives hi/lo encoding" );
		}
	}
	WriteReflectionProbeTexture(
	    valid.data(), layout, ReflectionProbeMode::Nearest, texture.data() );
	Check( HalfToFloat( texture[4] ) == 2.0f, "the mode texel follows the mode" );
}

ReflectionProbeMode ModeOf( float value )
{
	return static_cast<ReflectionProbeMode>( uint32_t( value ) );
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

// reflection_probe_set.fixture_change: the analytic change the relit
// samples were computed with (Source units).
void FixtureChange( void *, const float p[3], const float n[3], float out[3] )
{
	out[0] = 0.002f * p[0] + 0.5f * std::max( n[2], 0.0f );
	out[1] = 0.3f - 0.003f * p[1];
	out[2] = 0.25f * n[0] - 0.1f;
}

void ZeroChange( void *, const float *, const float *, float out[3] )
{
	out[0] = out[1] = out[2] = 0.0f;
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
	Check( layout.atlasHeight == 3 * 2 * 16 && layout.probes[0].relightRow == 32 &&
	           layout.probes[1].relightRow == 64,
	    "relight bands follow the radiance bands: albedo rows 32 and 64" );
	const int cases = ApplyCorpus( relit, "relight-malformations.txt" );
	Check( cases >= 11, "the relight corpus has its cases" );
	// The GPU form: relight rows in the table, the switch in texel 1.
	const std::vector<char> expected = Load( "gpu-relight-mode1.rgba16f" );
	const uint32_t rows = ReflectionProbeTextureRows( layout );
	std::vector<uint16_t> texture( size_t( layout.atlasWidth ) * rows * 4 );
	WriteReflectionProbeTexture( relit.data(), layout, ReflectionProbeMode::Blend, texture.data() );
	Check( rows == 1 + 2 + 96 && expected.size() == texture.size() * 2 &&
	           std::memcmp( expected.data(), texture.data(), expected.size() ) == 0,
	    "the relight GPU texture is byte-identical to reflection_probe_set.gpu_texture" );
	Check( HalfToFloat( texture[5] ) == 1.0f, "texel 1's y turns relighting on" );
	WriteReflectionProbeTexture(
	    relit.data(), layout, ReflectionProbeMode::Blend, texture.data(), false );
	Check( HalfToFloat( texture[5] ) == 0.0f, "relight false turns it off" );
	// The reference against the Python oracle's relit samples.
	const ReflectionProbesView view( relit.data(), layout );
	const ReflectionProbeDiffuseChange change = { FixtureChange, nullptr };
	const ReflectionProbeDiffuseChange zero = { ZeroChange, nullptr };
	std::ifstream samples( std::string( kFixtures ) + "relight-samples.txt" );
	std::string line;
	int count = 0;
	double worst = 0.0, identity = 0.0, moved = 0.0;
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
		float radiance[3], baked[3], unchanged[3];
		view.Radiance( position, normal, reflected, roughness, ModeOf( mode ), radiance, &change );
		view.Radiance( position, normal, reflected, roughness, ModeOf( mode ), baked );
		view.Radiance( position, normal, reflected, roughness, ModeOf( mode ), unchanged, &zero );
		for ( int c = 0; c < 3; ++c )
		{
			worst = std::max( worst, double( std::fabs( radiance[c] - expectedRadiance[c] ) ) );
			identity = std::max( identity, double( std::fabs( unchanged[c] - baked[c] ) ) );
			moved = std::max( moved, double( std::fabs( radiance[c] - baked[c] ) ) );
		}
		++count;
	}
	std::printf( "relit reference worst absolute error %.6f over %d samples; zero change %.6g; "
	             "largest relight %.4f\n",
	    worst, count, identity, moved );
	Check( count == 144, "relight-samples.txt has 48 samples per mode" );
	Check( worst < 0.02, "the C++ relit reference reproduces the Python oracle's samples" );
	Check( identity == 0.0, "a zero change reproduces the baked radiance exactly" );
	Check( moved > 0.1, "the fixture's change moves the radiance (the check can fail)" );
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
		if ( Validate( bytes, &layout ) != ReflectionProbesError::Ok )
			continue;
		++accepted;
		const ReflectionProbesView view( bytes.data(), layout );
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
		CheckTexels( valid, layout );
		CheckGpuTexture( valid, layout );
		const ReflectionProbesView view( valid.data(), layout );
		CheckSamples( view );
		CheckBlend( view );
		CheckFuzz( valid );
		CheckRelight();
		CheckFuzz( Load( "valid-relight.rprb" ) );
	}
	return testing::ReportConformance( g_checks, g_failures );
}
