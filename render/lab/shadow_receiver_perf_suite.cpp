//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Complete shadow receiver microbenchmark. GPU dispatch timestamps
//          exclude uploads and readback; immutable inputs and output comparison
//          accompany every sample. This is diagnostic, not a frame-floor gate.
//
//=============================================================================//

#include "suites.h"

#include "lab_compute.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "render/shadow_tile.h"
#include "spv/shadow_receiver_perf_spv.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace render::lab
{
namespace
{

using namespace device;
constexpr std::uint32_t kAtlasSize = 1024;
constexpr std::uint32_t kSide = 512;
constexpr std::uint32_t kCases = kSide * kSide * 3;
constexpr float kNear = 2.0f;
constexpr float kFar = 512.0f;

struct Case
{
	float worldSize[4];
	float normalRotation[4];
	std::uint32_t mode[4];
};
static_assert( sizeof( Case ) == 48 );

std::optional<std::string> Run(
    bool validate, std::span<const std::uint32_t> seeded, Results &checks, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( auto why = CreateLabDevice( validate, counter, device ) )
		return why;
	{
		struct Fixture
		{
			IRenderDevice2 &device;
			std::vector<ResourceId> resources;
			CompletionToken token;
			~Fixture()
			{
				(void)device.WaitIdle();
				for ( ResourceId id : resources )
					(void)device.Release( id, token );
				device.Poll();
			}
		} fixture{ *device, {}, {} };
		TextureDesc desc;
		desc.format = Format::kD32Float;
		desc.width = desc.height = kAtlasSize;
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
		auto atlas = device->CreateTexture( desc );
		if ( !atlas )
			return std::string( "the benchmark atlas was refused" );
		fixture.resources.push_back( atlas.Value() );
		std::vector<float> depths( kAtlasSize * kAtlasSize );
		for ( std::uint32_t y = 0; y < kAtlasSize; ++y )
			for ( std::uint32_t x = 0; x < kAtlasSize; ++x )
			{
				// Constant lit/shadow regions, small blockers and a depth ramp.
				depths[y * kAtlasSize + x] =
				    x < kAtlasSize / 2
				        ? ( y < kAtlasSize / 2 ? 1.0f : 0.25f )
				        : ( y < kAtlasSize / 2
				                  ? ( ( ( x / 16 ) ^ ( y / 16 ) ) & 1 ? 0.95f : 1.0f )
				                  : 0.7f + 0.3f * float( x ) / float( kAtlasSize - 1 ) );
			}
		BufferDesc uploadDesc;
		uploadDesc.size = depths.size() * sizeof( float );
		uploadDesc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource };
		auto upload = device->CreateBuffer( uploadDesc );
		if ( !upload )
			return std::string( "the benchmark atlas staging buffer was refused" );
		fixture.resources.push_back( upload.Value() );
		auto encoded = device->BeginEncoder( QueueKind::kGraphics );
		if ( !encoded )
			return std::string( "no benchmark upload encoder" );
		auto &e = encoded.Value();
		e.TransitionBuffer(
		    upload.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		e.WriteBuffer( upload.Value(), 0, std::as_bytes( std::span( depths ) ) );
		e.TransitionBuffer(
		    upload.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
		e.TransitionTexture(
		    atlas.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		e.CopyBufferToTexture( upload.Value(), atlas.Value(), { 0, 0, 0, kAtlasSize, kAtlasSize } );
		e.TransitionTexture(
		    atlas.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
		auto submitted = device->Submit( QueueKind::kGraphics, { &e, 1 }, {} );
		if ( !submitted )
			return std::string( "the atlas upload did not submit" );
		fixture.token = submitted.Value();
		if ( !device->WaitIdle() )
			return std::string( "the atlas upload did not complete" );

		ShadowTileGpu tiles[3];
		for ( auto &tile : tiles )
		{
			tile.viewProjection[0][0] = tile.viewProjection[1][1] = 1.0f;
			tile.viewProjection[2][2] = kFar / ( kFar - kNear );
			tile.viewProjection[2][3] = -kFar * kNear / ( kFar - kNear );
			tile.viewProjection[3][2] = 1.0f;
			tile.transform[0] = tile.transform[1] = tile.transform[2] = tile.transform[3] = 0.5f;
			tile.bounds[2] = tile.bounds[3] = 1.0f;
			tile.params[0] = 5e-5f;
			tile.params[1] = float( kAtlasSize );
			tile.params[2] = kNear;
			tile.params[3] = kFar;
		}
		tiles[1].viewProjection[0][0] = tiles[1].viewProjection[1][1] = 1.0f / 256.0f;
		tiles[1].viewProjection[2][2] = 1.0f / kFar;
		tiles[1].viewProjection[2][3] = tiles[1].viewProjection[3][2] = 0.0f;
		tiles[1].viewProjection[3][3] = 1.0f;
		tiles[1].params[2] = -1.0f;
		tiles[2].params[2] = tiles[2].params[3] = 0.0f;
		std::vector<Case> points( kCases );
		for ( std::uint32_t mode = 0; mode < 3; ++mode )
			for ( std::uint32_t y = 0; y < kSide; ++y )
				for ( std::uint32_t x = 0; x < kSide; ++x )
				{
					const float distance = 32.0f + 448.0f * float( y ) / float( kSide - 1 );
					const float scale = mode == 1 ? 256.0f : distance;
					Case &p = points[( mode * kSide + y ) * kSide + x];
					p = { { ( float( x ) / float( kSide - 1 ) * 2.02f - 1.01f ) * scale,
					          ( float( y ) / float( kSide - 1 ) * 2.02f - 1.01f ) * scale, distance,
					          mode == 1 ? 0.05f
					                    : ( x % 3 == 0     ? 0.5f
					                          : x % 3 == 1 ? 4.0f
					                                       : 32.0f ) },
					    { 0.0f, 0.0f, -1.0f, float( ( x * 73 + y * 37 ) % 997 ) * 0.0063f },
					    { mode, 0, 0, 0 } };
				}
		std::vector<std::byte> cases( sizeof( tiles ) + points.size() * sizeof( Case ) );
		std::memcpy( cases.data(), tiles, sizeof( tiles ) );
		std::memcpy(
		    cases.data() + sizeof( tiles ), points.data(), points.size() * sizeof( Case ) );
		SamplerDesc samplers[2];
		samplers[0].minFilter = samplers[0].magFilter = samplers[0].mipFilter = Filter::kNearest;
		samplers[0].address = samplers[1].address = AddressMode::kClampToEdge;
		samplers[1].mipFilter = Filter::kNearest;
		samplers[1].comparison = CompareOp::kLessEqual;
		resources::TextureCache cache( *device );
		CheckKernel control( *device ), candidate( *device );
		if ( auto why = control.Create(
		         spirv::kShadowReceiverPerfControl, 1, 2, "shadow receiver control", samplers ) )
			return why;
		if ( auto why = candidate.Create(
		         seeded.empty() ? std::span<const std::uint32_t>( spirv::kShadowReceiverPerf )
		                        : seeded,
		         1, 2, "shadow receiver candidate", samplers ) )
			return why;
		const TextureId textures[] = { atlas.Value() };
		std::vector<std::byte> expected, actual;
		std::array<CheckKernel::GpuTime, 32> calibrationTimes;
		CheckKernel::Comparison calibration{ control, expected, calibrationTimes };
		if ( auto why = control.Run(
		         cache, textures, kCases, cases, kCases * sizeof( float ), actual, &calibration ) )
			return why;
		std::array<double, 32> calibrationRatios;
		for ( std::size_t i = 0; i < calibrationTimes.size(); ++i )
		{
			calibrationRatios[i] = calibrationTimes[i].candidateMs / calibrationTimes[i].controlMs;
			std::printf( "GPU_MICRO\tshadow_receiver_calibration\tcontrol\t0\t%zu\t%u\t%.6f\n", i,
			    kCases, calibrationTimes[i].controlMs );
			std::printf( "GPU_MICRO\tshadow_receiver_calibration\tcandidate\t0\t%zu\t%u\t%.6f\n", i,
			    kCases, calibrationTimes[i].candidateMs );
		}
		std::sort( calibrationRatios.begin(), calibrationRatios.end() );
		checks.That( std::fabs( calibrationRatios[16] - 1.0 ) <= 0.03,
		    "receiver.timestamp-calibration",
		    "identical-code ratio " + std::to_string( calibrationRatios[16] ) );
		checks.That( actual == expected, "receiver.calibration-bitwise" );
		for ( int round = 0; round < 4; ++round )
		{
			std::array<CheckKernel::GpuTime, 32> times;
			CheckKernel::Comparison comparison{ control, expected, times };
			if ( auto why = candidate.Run( cache, textures, kCases, cases, kCases * sizeof( float ),
			         actual, &comparison ) )
				return why;
			for ( std::size_t i = 0; i < times.size(); ++i )
			{
				std::printf( "GPU_MICRO\tshadow_receiver\tcontrol\t%d\t%zu\t%u\t%.6f\n", round, i,
				    kCases, times[i].controlMs );
				std::printf( "GPU_MICRO\tshadow_receiver\tcandidate\t%d\t%zu\t%u\t%.6f\n", round, i,
				    kCases, times[i].candidateMs );
			}
			std::size_t bad = 0, lit = 0, dark = 0, partial = 0;
			float worst = 0;
			for ( std::uint32_t i = 0; i < kCases; ++i )
			{
				float a, b;
				std::memcpy( &a, actual.data() + i * sizeof( float ), sizeof( float ) );
				std::memcpy( &b, expected.data() + i * sizeof( float ), sizeof( float ) );
				worst = std::max( worst, std::fabs( a - b ) );
				bad += !std::isfinite( a ) || !std::isfinite( b ) || a != b;
				lit += b == 1.0f;
				dark += b == 0.0f;
				partial += b > 0.0f && b < 1.0f;
			}
			checks.That( bad == 0, "receiver.matches-control." + std::to_string( round ),
			    std::to_string( bad ) + " mismatches; max error " + std::to_string( worst ) );
			checks.That( lit > 100 && dark > 100 && partial > 100,
			    "receiver.coverage." + std::to_string( round ),
			    std::to_string( lit ) + " lit, " + std::to_string( dark ) + " dark, " +
			        std::to_string( partial ) + " partial" );
		}
	}
	checks.That( device->LiveResourceCount() == 0, "receiver.resources-retired" );
	device.reset();
	messages = counter.load();
	return std::nullopt;
}

} // namespace

int RunShadowReceiverPerfSuite( int argc, char **argv )
{
	const Seeded seeded[] = { { "missing-filter-tap", spirv::kShadowReceiverPerfMissingTap,
	    "receiver.matches-control." } };
	return RunSeededSuite( argc, argv, "shadow-receiver-perf", seeded, Run );
}

} // namespace render::lab
