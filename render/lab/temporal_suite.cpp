//========= Copyright Valve Corporation, All rights reserved. ============//
// RFC 0019 initial input/lifetime fixture and real FSR dispatch measurements.
#include "suites.h"
#include "lab_support.h"
#include "render/device/vulkan/fsr.h"
#include "render/device/vulkan/provider.h"
#include "render/pass/temporal/temporal.h"
#include "render/graph/executor.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <limits>
#include <thread>
namespace render::lab
{
namespace
{
using namespace device;
using namespace pass::temporal;
struct Checks
{
	int count = 0, failures = 0, resetFailures = 0;
	void Check( bool result, const char *name )
	{
		++count;
		if ( !result )
		{
			++failures;
			if ( std::strcmp( name, "camera cut equals independent fresh history" ) == 0 ||
			     std::strcmp( name, "view histories remain isolated" ) == 0 ||
			     std::strcmp( name, "failed recording recovery" ) == 0 )
				++resetFailures;
			std::fprintf( stderr, "FAIL %s\n", name );
		}
	}
};
void HistoryChecks( Checks &checks )
{
	History history;
	Frame f;
	f.view = { 1, 1, 0, 0, 0 };
	f.sequence = 1;
	f.render = { 64, 48 };
	f.output = { 96, 72 };
	f.motionComplete = true;
	f.deltaMilliseconds = 16;
	auto first = history.Prepare( f, 1 );
	checks.Check( first && first.Value().reset, "first-use resets" );
	checks.Check( !history.Prepare( f, 1 ), "pending metadata refuses overwrite" );
	history.Commit();
	checks.Check( !history.Prepare( f, 1 ), "stale sequence rejected" );
	++f.sequence;
	auto next = history.Prepare( f, 1 );
	checks.Check( next && !next.Value().reset, "consecutive frame accumulates" );
	history.Commit();
	f.sequence += 2;
	auto skipped = history.Prepare( f, 1 );
	checks.Check( skipped && skipped.Value().reset, "skipped frame resets" );
	history.Commit();
	++f.sequence;
	f.view.chain = 42;
	auto portal = history.Prepare( f, 1 );
	checks.Check( portal && portal.Value().reset, "portal chain isolates history" );
	history.Commit();
	++f.sequence;
	f.preExposure = 2;
	auto exposure = history.Prepare( f, 1 );
	checks.Check( exposure && exposure.Value().reset, "exposure change resets" );
	history.Abort();
	auto aborted = history.Prepare( f, 1 );
	checks.Check( aborted && aborted.Value().reset, "failed frame discards candidate" );
	history.Commit();
	++f.sequence;
	f.motionComplete = false;
	checks.Check( !history.Prepare( f, 1 ), "missing cohort rejected" );
	f.motionComplete = true;
	f.jitterX = std::numeric_limits<float>::quiet_NaN();
	checks.Check( !history.Prepare( f, 1 ), "nonfinite jitter rejected" );
	f.jitterX = 0;
	f.deltaMilliseconds = 0;
	checks.Check( !history.Prepare( f, 1 ), "invalid time rejected" );
	f.deltaMilliseconds = 16;
	auto recovery = history.Prepare( f, 2 );
	checks.Check( recovery && recovery.Value().reset, "device recovery resets" );
	history.Commit();
	++f.sequence;
	f.output.width += 8;
	auto resize = history.Prepare( f, 2 );
	checks.Check( resize && resize.Value().reset, "resize resets" );
	history.Abort();
}
float Halton( unsigned i, unsigned base )
{
	float result = 0, fraction = 1;
	while ( i )
	{
		fraction /= base;
		result += fraction * ( i % base );
		i /= base;
	}
	return result - 0.5f;
}
// Analytic layered image. Camera translates 0.25 render pixels per frame;
// a foreground rectangle moves 0.75 pixels. Motion is current -> previous,
// unjittered, in render pixels. No missing geometry pretends to be stationary.
void InputsFor( unsigned width, unsigned height, unsigned sequence, float jx, float jy,
    std::vector<std::uint16_t> &color, std::vector<float> &depth,
    std::vector<std::uint16_t> &motion )
{
	color.resize( std::size_t( width ) * height * 4 );
	depth.resize( std::size_t( width ) * height );
	motion.resize( depth.size() * 2 );
	const float camera = sequence * 0.25f, object = sequence * 0.75f;
	for ( unsigned y = 0; y < height; ++y )
		for ( unsigned x = 0; x < width; ++x )
		{
			const float px = x + 0.5f + jx, py = y + 0.5f + jy;
			const bool foreground = px > width * 0.3f + object - camera &&
			                        px < width * 0.6f + object - camera && py > height * 0.2f &&
			                        py < height * 0.8f;
			const int checker = ( int( std::floor( ( px + camera + py ) / 12 ) ) & 1 );
			const float base = checker ? 0.8f : 0.15f;
			const std::size_t pixel = std::size_t( y ) * width + x;
			const float rgb[] = { foreground ? 1.0f : base, foreground ? 0.15f : base,
			    foreground ? 0.05f : base, 1.0f };
			for ( unsigned c = 0; c < 4; ++c )
				color[pixel * 4 + c] = FloatToHalf( rgb[c] );
			depth[pixel] = foreground ? 0.25f : 0.8f;
			motion[pixel * 2] = FloatToHalf( foreground ? -0.5f : 0.25f );
			motion[pixel * 2 + 1] = 0;
		}
}
}
int RunTemporalSuite( int argc, char **argv )
{
	Checks checks;
	HistoryChecks( checks );
	unsigned rw = 1280, rh = 720, ow = 1920, oh = 1080, frames = 48;
	bool validate = false, seededReset = false;
	std::string assets = "external/fsr411/assets", dump;
	for ( int i = 0; i < argc; ++i )
	{
		const std::string arg = argv[i];
		if ( arg == "--seeded" && i + 1 < argc &&
		     std::string( argv[i + 1] ) == "retain-reset-history" )
		{
			seededReset = true;
			++i;
		}
		else if ( arg == "--validate" )
			validate = true;
		else if ( arg == "--assets" && i + 1 < argc )
			assets = argv[++i];
		else if ( arg == "--dump" && i + 1 < argc )
			dump = argv[++i];
		else if ( arg == "--render" && i + 1 < argc )
		{
			if ( std::sscanf( argv[++i], "%ux%u", &rw, &rh ) != 2 )
				return 2;
		}
		else if ( arg == "--output" && i + 1 < argc )
		{
			if ( std::sscanf( argv[++i], "%ux%u", &ow, &oh ) != 2 )
				return 2;
		}
		else if ( arg == "--frames" && i + 1 < argc )
		{
			if ( std::sscanf( argv[++i], "%u", &frames ) != 1 )
				return 2;
		}
		else
			return 2;
	}
	if ( frames < 10 || frames > 600 || !rw || !rh || rw > ow || rh > oh || ow > 3840 || oh > 2160 )
		return 2;
	std::atomic<std::uint64_t> messages{ 0 };
	if ( validate && !vulkan::ValidationLayerAvailable() )
	{
		std::fprintf( stderr, "required validation layer unavailable\n" );
		return 1;
	}
	vulkan::VulkanAdapterOptions options;
	options.validation = validate;
	options.validationCounter = &messages;
	options.fsr411 = true;
	options.sensitivity.fsrRetainResetHistory = seededReset;
	auto created = vulkan::Create( options );
	if ( !created )
	{
		std::fprintf( stderr, "FSR device: %s\n", DescribeStatus( created.Error().status ) );
		return 1;
	}
	auto device = std::move( created ).Value();
	auto providerResult = vulkan::CreateFsr411( *device, assets );
	if ( !providerResult )
		return 1;
	auto provider = std::move( providerResult ).Value();
	std::printf( "GPU %s; provider %s\n", std::string( device->Facts().adapterName ).c_str(),
	    provider->Description().c_str() );
	std::array<TextureId, 4> textures;
	std::array<TextureDesc, 4> descriptions;
	const Format formats[] = {
	    Format::kRGBA16Float, Format::kD32Float, Format::kRG16Float, Format::kRGBA16Float };
	for ( unsigned i = 0; i < 4; ++i )
	{
		auto &d = descriptions[i];
		d.format = formats[i];
		d.width = i == 3 ? ow : rw;
		d.height = i == 3 ? oh : rh;
		d.usages = i == 3 ? UsageSet{ ResourceUsage::kStorageWrite, ResourceUsage::kCopySource }
		                  : UsageSet{ ResourceUsage::kSampled, ResourceUsage::kCopyDestination };
		auto texture = device->CreateTexture( d );
		if ( !texture )
			return 1;
		textures[i] = texture.Value();
	}
	BufferDesc bd;
	bd.size = std::uint64_t( ow ) * oh * 8;
	bd.memory = MemoryKind::kReadback;
	bd.usages = { ResourceUsage::kCopyDestination };
	auto pixels = device->CreateBuffer( bd );
	bd.size = 16;
	auto timestamps = device->CreateBuffer( bd );
	if ( !pixels || !timestamps )
		return 1;
	History history;
	std::vector<std::uint16_t> color, motion, output( std::size_t( ow ) * oh * 4 );
	std::vector<float> depth;
	std::vector<double> gpu, cpu;
	CompletionToken last;
	for ( unsigned n = 0; n < frames; ++n )
	{
		const float jx = Halton( n % 16 + 1, 2 ), jy = Halton( n % 16 + 1, 3 );
		InputsFor( rw, rh, n, jx, jy, color, depth, motion );
		auto e = device->BeginEncoder( QueueKind::kGraphics );
		if ( !e )
			return 1;
		auto encoder = std::move( e ).Value();
		std::array<BufferId, 3> uploads;
		const std::span<const std::byte> bytes[] = { std::as_bytes( std::span( color ) ),
		    std::as_bytes( std::span( depth ) ), std::as_bytes( std::span( motion ) ) };
		for ( unsigned i = 0; i < 3; ++i )
		{
			auto upload = device->CreateUploadBuffer( bytes[i] );
			if ( !upload )
				return 1;
			uploads[i] = upload.Value();
			encoder.TransitionTexture( textures[i],
			    n ? ResourceUsage::kSampled : ResourceUsage::kUndefined,
			    ResourceUsage::kCopyDestination );
			encoder.TransitionBuffer(
			    uploads[i], ResourceUsage::kUndefined, ResourceUsage::kCopySource );
			TextureBufferCopy copy;
			copy.width = rw;
			copy.height = rh;
			encoder.CopyBufferToTexture( uploads[i], textures[i], copy );
			encoder.TransitionTexture(
			    textures[i], ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
		}
		encoder.TransitionBuffer( timestamps.Value(),
		    n ? ResourceUsage::kCopyDestination : ResourceUsage::kUndefined,
		    ResourceUsage::kCopyDestination );
		encoder.WriteTimestamp( timestamps.Value(), 0 );
		graph::GraphBuilder graph;
		std::array<graph::ResourceRef, 4> refs;
		for ( unsigned i = 0; i < 4; ++i )
			refs[i] = graph.ImportTexture( "temporal", textures[i], descriptions[i],
			    i == 3 ? ( n ? ResourceUsage::kCopySource : ResourceUsage::kUndefined )
			           : ResourceUsage::kSampled,
			    i == 3 ? ResourceUsage::kCopySource : ResourceUsage::kSampled );
		Frame frame;
		frame.view = { 1, 1, 0, 0, 0 };
		frame.sequence = n + 1;
		frame.render = { rw, rh };
		frame.output = { ow, oh };
		frame.jitterX = jx;
		frame.jitterY = jy;
		frame.motionComplete = true;
		frame.deltaMilliseconds = 1000.0f / 60;
		frame.discontinuity = n == frames / 2;
		auto added = AddPass( graph, history, *provider, frame, device->Epoch(),
		    { refs[0], refs[1], refs[2], refs[3] } );
		if ( !added )
			return 1;
		auto compiled = graph::CompileGraph( std::move( graph ) );
		if ( !compiled )
			return 1;
		const auto start = std::chrono::steady_clock::now();
		auto recorded = graph::RecordInline( compiled.Value(), *device, encoder );
		if ( !recorded )
			return 1;
		encoder.WriteTimestamp( timestamps.Value(), 8 );
		encoder.TransitionBuffer( pixels.Value(),
		    n ? ResourceUsage::kCopyDestination : ResourceUsage::kUndefined,
		    ResourceUsage::kCopyDestination );
		TextureBufferCopy copy;
		copy.width = ow;
		copy.height = oh;
		encoder.CopyTextureToBuffer( textures[3], pixels.Value(), copy );
		auto submitted = device->Submit( QueueKind::kGraphics, std::span( &encoder, 1 ), {} );
		if ( !submitted )
		{
			history.Abort();
			std::fprintf( stderr, "FSR submit: %s\n", DescribeStatus( submitted.Error().status ) );
			return 1;
		}
		const double cpuMs =
		    std::chrono::duration<double, std::milli>( std::chrono::steady_clock::now() - start )
		        .count();
		last = submitted.Value();
		history.Commit();
		recorded.Value().Release( *device, last );
		for ( auto upload : uploads )
			(void)device->Release( ResourceId( upload ), last );
		const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 30 );
		while ( !device->IsComplete( last ) && std::chrono::steady_clock::now() < deadline )
			std::this_thread::yield();
		if ( !device->IsComplete( last ) )
		{
			std::fprintf( stderr, "GPU timeout\n" );
			return 1;
		}
		std::array<std::uint64_t, 2> ticks{};
		if ( !device->ReadBuffer(
		         timestamps.Value(), 0, std::as_writable_bytes( std::span( ticks ) ) ) ||
		     !device->ReadBuffer(
		         pixels.Value(), 0, std::as_writable_bytes( std::span( output ) ) ) )
			return 1;
		bool finite = true;
		double sum = 0;
		for ( auto half : output )
		{
			const float value = HalfToFloat( half );
			finite &= std::isfinite( value );
			sum += value;
		}
		checks.Check( finite && sum > 0, "finite nonempty FSR output" );
		checks.Check( ticks[1] > ticks[0], "GPU timestamp interval" );
		if ( n >= 8 )
		{
			gpu.push_back( ( ticks[1] - ticks[0] ) * device->Facts().timestampPeriodNs / 1e6 );
			cpu.push_back( cpuMs );
		}
		device->Poll();
	}

	// Same inputs, independent contexts: reset must erase accumulated history
	// and another view must not inherit it. Exact comparison is meaningful
	// here because every input and provider implementation is identical.
	auto resetFrame = [&]( ITemporalUpscaler &selected, bool reset ) -> bool
	{
		auto begun = device->BeginEncoder( QueueKind::kGraphics );
		if ( !begun )
			return false;
		auto encoder = std::move( begun ).Value();
		encoder.TransitionTexture(
		    textures[3], ResourceUsage::kCopySource, ResourceUsage::kStorageWrite );
		TemporalDispatch dispatch;
		dispatch.images = { textures[0], textures[1], textures[2], textures[3] };
		dispatch.render = { rw, rh };
		dispatch.output = { ow, oh };
		dispatch.jitterX = Halton( ( frames - 1 ) % 16 + 1, 2 );
		dispatch.jitterY = Halton( ( frames - 1 ) % 16 + 1, 3 );
		dispatch.reset = reset;
		if ( !selected.Record( encoder, dispatch ) )
			return false;
		encoder.TransitionTexture(
		    textures[3], ResourceUsage::kStorageWrite, ResourceUsage::kCopySource );
		TextureBufferCopy copy;
		copy.width = ow;
		copy.height = oh;
		encoder.CopyTextureToBuffer( textures[3], pixels.Value(), copy );
		auto submitted = device->Submit( QueueKind::kGraphics, std::span( &encoder, 1 ), {} );
		if ( !submitted )
			return false;
		last = submitted.Value();
		const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 30 );
		while ( !device->IsComplete( last ) && std::chrono::steady_clock::now() < deadline )
			std::this_thread::yield();
		if ( !device->IsComplete( last ) )
			return false;
		device->Poll();
		return bool( device->ReadBuffer(
		    pixels.Value(), 0, std::as_writable_bytes( std::span( output ) ) ) );
	};
	auto second = vulkan::CreateFsr411( *device, assets );
	if ( !second )
		return 1;
	checks.Check( resetFrame( *second.Value(), true ), "second view first use" );
	const auto independent = output;
	checks.Check( resetFrame( *provider, true ), "camera cut reset dispatch" );
	checks.Check( output == independent, "camera cut equals independent fresh history" );
	checks.Check( resetFrame( *second.Value(), true ), "second view reuse" );
	checks.Check( output == independent, "view histories remain isolated" );
	// Deliberately invalid input is refused before recording and poisons the
	// submission. The existing history must remain usable afterward.
	{
		auto begun = device->BeginEncoder( QueueKind::kGraphics );
		if ( !begun )
			return 1;
		auto encoder = std::move( begun ).Value();
		TemporalDispatch invalid;
		invalid.images = { textures[0], textures[1], textures[2], textures[3] };
		invalid.render = { rw, rh };
		invalid.output = { ow, oh };
		invalid.preExposure = std::numeric_limits<float>::quiet_NaN();
		checks.Check( !provider->Record( encoder, invalid ), "nonfinite provider input refused" );
		checks.Check( !device->Submit( QueueKind::kGraphics, std::span( &encoder, 1 ), {} ),
		    "failed recording cannot submit" );
		checks.Check(
		    resetFrame( *provider, true ) && output == independent, "failed recording recovery" );
	}
	// Adversarial GPU schedule: the first eight submissions cannot execute.
	// The ninth must refuse constant-buffer slot reuse; reset gets independent
	// storage, so it can be queued while the old context is still in flight.
	if ( !seededReset )
	{
		const bool held = vulkan::HoldSubmissions( *device, true );
		checks.Check( held, "hold GPU submissions" );
		if ( !held )
			return 1;
		auto enqueue = [&]( bool reset ) -> bool
		{
			auto begun = device->BeginEncoder( QueueKind::kGraphics );
			if ( !begun )
				return false;
			auto encoder = std::move( begun ).Value();
			encoder.TransitionTexture(
			    textures[3], ResourceUsage::kCopySource, ResourceUsage::kStorageWrite );
			TemporalDispatch f;
			f.render = { rw, rh };
			f.output = { ow, oh };
			f.images = { textures[0], textures[1], textures[2], textures[3] };
			f.reset = reset;
			if ( !provider->Record( encoder, f ) )
			{
				checks.Check( !device->Submit( QueueKind::kGraphics, std::span( &encoder, 1 ), {} ),
				    "ring refusal cannot submit" );
				return false;
			}
			encoder.TransitionTexture(
			    textures[3], ResourceUsage::kStorageWrite, ResourceUsage::kCopySource );
			auto submitted = device->Submit( QueueKind::kGraphics, std::span( &encoder, 1 ), {} );
			if ( !submitted )
				return false;
			last = submitted.Value();
			return true;
		};
		bool queued = true;
		for ( unsigned i = 0; i < 8; ++i )
			queued &= enqueue( false );
		checks.Check( queued && !device->IsComplete( last ), "eight outstanding frames" );
		checks.Check( !enqueue( false ), "constant ring cannot overwrite held GPU work" );
		checks.Check( enqueue( true ), "reset replaces context with GPU work outstanding" );
		provider.reset(); // submission payloads own the outstanding histories
		checks.Check( vulkan::HoldSubmissions( *device, false ), "release GPU submissions" );
		const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 30 );
		while ( !device->IsComplete( last ) && std::chrono::steady_clock::now() < deadline )
			std::this_thread::yield();
		checks.Check( device->IsComplete( last ), "provider teardown retains in-flight resources" );
		device->Poll();
	}

	checks.Check( bool( device->Recover() ), "healthy device recovery is a no-op" );

	second.Value().reset();
	if ( !dump.empty() )
	{
		std::ofstream file( dump, std::ios::binary );
		file << "PF\n" << ow << " " << oh << "\n-1.0\n";
		for ( unsigned y = oh; y-- > 0; )
			for ( unsigned x = 0; x < ow; ++x )
				for ( unsigned c = 0; c < 3; ++c )
				{
					float value = HalfToFloat( output[( std::size_t( y ) * ow + x ) * 4 + c] );
					file.write( reinterpret_cast<const char *>( &value ), 4 );
				}
		checks.Check( bool( file ), "image saved" );
	}
	std::sort( gpu.begin(), gpu.end() );
	std::sort( cpu.begin(), cpu.end() );
	std::printf(
	    "FSR_TIMING "
	    "{\"render\":[%u,%u],\"output\":[%u,%u],\"samples\":%zu,\"gpu_median_ms\":%.6f,\"gpu_max_"
	    "ms\":%.6f,\"cpu_record_submit_median_ms\":%.6f,\"cpu_record_submit_max_ms\":%.6f,\"target_"
	    "fps\":120,\"timing_policy\":\"advisory\",\"complete_product_frame\":false}\n",
	    rw, rh, ow, oh, gpu.size(), gpu[gpu.size() / 2], gpu.back(), cpu[cpu.size() / 2],
	    cpu.back() );
	provider.reset();
	for ( auto texture : textures )
		(void)device->Release( ResourceId( texture ), last );
	(void)device->Release( ResourceId( pixels.Value() ), last );
	(void)device->Release( ResourceId( timestamps.Value() ), last );
	device->Poll();
	device.reset();
	checks.Check( messages.load() == 0, "zero validation messages including teardown" );
	if ( seededReset )
	{
		const bool caught = checks.resetFailures == 3 && checks.failures == 3;
		std::printf( "SENSITIVITY retained-reset-history %s\n", caught ? "caught" : "not caught" );
		std::printf( "CONFORMANCE 1 %d\n", caught ? 0 : 1 );
		return caught ? 0 : 1;
	}
	std::printf( "CONFORMANCE %d %d\n", checks.count, checks.failures );
	return checks.failures ? 1 : 0;
}
}
