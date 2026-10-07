//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.v2 on render.device.gl (RFC 0016 K10): the shared
//			suite and its raster clauses on an OpenGL 4.5 core context (EGL
//			surfaceless: no window reaches a desktop), with the GLSL 4.50
//			artifacts of the suite's fixtures (spv/device_fixtures_glsl.h), in
//			the default and a small-ring configuration and under GL debug
//			output; plus the adapter's own clauses: facts (unclaimed
//			capabilities are reported, not failed), the ring, a sub-viewport's
//			placement, calls from another thread, the caller's current context
//			kept, and a masked capability.
//
//			Built with RENDER_DEVICE_GL_ES it is render.device.v2.gles: the
//			same clauses on the adapter's OpenGL ES 3.1 dialect (RFC 0022), with
//			the fixtures' GLSL ES 3.10 artifacts (spv/device_fixtures_gles.h).
//
//			Built with RENDER_DEVICE_GL_SENSITIVITY it is
//			render.device.v2.gl.sensitivity instead: each bad GL adapter
//			(GlAdapterOptions::Sensitivity) must fail the clause it breaks and
//			no other, while the undecorated control passes.
//
//			LIBGL_ALWAYS_SOFTWARE=1 selects Mesa's llvmpipe instead of the GPU.
//
//=============================================================================//

#include "device_conformance.h"
#include "render/device/gl/provider.h"
#include "spv/device_fixtures_gles.h"
#include "spv/device_fixtures_glsl.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>

namespace
{

using namespace render::device;

struct Fixture
{
	const std::uint32_t *spirv;
	std::string_view glsl;
};

#if defined( RENDER_DEVICE_GL_ES )
namespace text = rendertest::gles;
constexpr gl::GlApiKind kApi = gl::GlApiKind::kEs31;
[[maybe_unused]] constexpr ArtifactFormat kFormat = ArtifactFormat::kGlslEs310;
[[maybe_unused]] constexpr const char *kName = "gles";
[[maybe_unused]] constexpr EGLenum kEglApi = EGL_OPENGL_ES_API;
#else
namespace text = rendertest::glsl;
constexpr gl::GlApiKind kApi = gl::GlApiKind::kDesktop45;
[[maybe_unused]] constexpr ArtifactFormat kFormat = ArtifactFormat::kGlsl450;
[[maybe_unused]] constexpr const char *kName = "gl";
[[maybe_unused]] constexpr EGLenum kEglApi = EGL_OPENGL_API;
#endif

// The adapter's options for this build's dialect.
gl::GlAdapterOptions Options()
{
	gl::GlAdapterOptions options;
	options.api = kApi;
	return options;
}

// Each suite fixture's SPIR-V and its GLSL 4.50 (GLSL ES 3.10) artifact.
const Fixture kFixtures[] = {
    { rendertest::shaders::kFullScreenVertex, text::kFullScreenVertex },
    { rendertest::shaders::kTopHalfVertex, text::kTopHalfVertex },
    { rendertest::shaders::kColorFragment, text::kColorFragment },
    { rendertest::shaders::kConstantFragment, text::kConstantFragment },
    { rendertest::shaders::kSpecializedFragment, text::kSpecializedFragment },
    { rendertest::shaders::kDoubleCompute, text::kDoubleCompute },
    { rendertest::shaders::kSampledFragment, text::kSampledFragment },
    { rendertest::shaders::kComparisonFragment, text::kComparisonFragment },
    { rendertest::shaders::kPositionVertex, text::kPositionVertex },
};

std::span<const std::byte> Artifact( std::span<const std::uint32_t> spirv )
{
	for ( const Fixture &fixture : kFixtures )
	{
		if ( fixture.spirv == spirv.data() )
			return std::as_bytes( std::span<const char>( fixture.glsl ) );
	}
	return {};
}

bool WaitFor( IRenderDevice2 &device, CompletionToken token )
{
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 10 );
	while ( !device.IsComplete( token ) )
	{
		if ( std::chrono::steady_clock::now() > deadline )
			return false;
		std::this_thread::sleep_for( std::chrono::microseconds( 200 ) );
	}
	return true;
}

rendertest::DeviceDriver Driver( const char *name, gl::GlAdapterOptions options )
{
	rendertest::DeviceDriver driver;
	driver.name = name;
	driver.create = [options]() -> std::unique_ptr<IRenderDevice2>
	{
		auto device = gl::Create( options );
		return device ? std::move( device ).Value() : nullptr;
	};
	driver.complete = WaitFor;
	driver.lose = []( IRenderDevice2 &device )
	{
		(void)gl::SimulateContextLoss( device );
	};
	driver.hold = []( IRenderDevice2 &device, bool held )
	{
		(void)gl::HoldSubmissions( device, held );
	};
	driver.rasterizes = true;
	driver.doubleCompute = rendertest::shaders::kDoubleCompute;
	driver.artifact = Artifact;
	return driver;
}

} // namespace

#if defined( RENDER_DEVICE_GL_SENSITIVITY )

namespace
{

std::string FailuresOf( const rendertest::DeviceDriver &driver )
{
	char *buffer = nullptr;
	size_t size = 0;
	std::FILE *stream = open_memstream( &buffer, &size );
	{
		testing::Checks inner( stream );
		rendertest::RunDeviceConformance( inner, driver );
	}
	std::fclose( stream );
	std::string out( buffer ? buffer : "", size );
	std::free( buffer );
	return out;
}

// Every failing check line names only the expected clause.
bool OnlyClause( const std::string &failures, const char *clause )
{
	std::size_t lines = 0;
	std::size_t start = 0;
	while ( start < failures.size() )
	{
		std::size_t end = failures.find( '\n', start );
		if ( end == std::string::npos )
			end = failures.size();
		const std::string line = failures.substr( start, end - start );
		if ( line.find( "under-test." ) != std::string::npos )
		{
			++lines;
			if ( line.find( clause ) == std::string::npos )
				return false;
		}
		start = end + 1;
	}
	return lines > 0;
}

} // namespace

// render.device.v2.gl.sensitivity: the shared suite must fail each bad GL
// adapter on the clause it breaks, and on no other, while the undecorated
// control passes. The knobs are never set by a product.
int main()
{
	testing::Checks checks;
	const gl::GlAdapterOptions options = Options();
	{
		const std::string failures = FailuresOf( Driver( "under-test", options ) );
		checks.That( failures.empty(), "control-passes" );
		if ( !failures.empty() )
			std::printf( "%s", failures.c_str() );
	}
	struct Case
	{
		const char *name;
		gl::GlAdapterOptions::Sensitivity defect;
		const char *clause;
	};
	gl::GlAdapterOptions::Sensitivity lowerLeft;
	lowerLeft.lowerLeftOrigin = true;
	gl::GlAdapterOptions::Sensitivity glDepth;
	glDepth.negativeOneToOneDepth = true;
	gl::GlAdapterOptions::Sensitivity asyncCompute;
	asyncCompute.falseClaims.Add( Capability::kAsyncCompute );
	gl::GlAdapterOptions::Sensitivity aliasing;
	aliasing.falseClaims.Add( Capability::kTransientAliasing );
	gl::GlAdapterOptions::Sensitivity writeMasks;
	writeMasks.ignoreColorWriteMasks = true;
	gl::GlAdapterOptions::Sensitivity specialization;
	specialization.dropSpecialization = true;
	gl::GlAdapterOptions::Sensitivity uploads;
	uploads.unsafeUploadReuse = true;
	gl::GlAdapterOptions::Sensitivity transmittance;
	transmittance.transmittanceAsPremultiplied = true;
	gl::GlAdapterOptions::Sensitivity comparison;
	comparison.reverseSamplerComparison = true;
	const Case cases[] = {
	    { "lower-left-origin", lowerLeft, "under-test.D13 clip y" },
	    { "gl-depth-range", glDepth, "under-test.D13 clip z" },
	    { "false-async-compute", asyncCompute, "under-test.D15 claimed async compute" },
	    { "false-aliasing", aliasing, "under-test.D15 claims transient-aliasing" },
	    { "ignored-write-masks", writeMasks, "under-test.D17 a red-and-alpha mask" },
	    { "dropped-specialization", specialization, "under-test.D20 the constant's value" },
	    { "early-upload-reuse", uploads, "under-test.D10 " },
	    { "reversed-sampler-compare", comparison, "under-test.D24 " },
	    { "transmittance-as-premultiplied", transmittance,
	        "under-test.D21 the independent color equation" },
	};
	// D10 records its uploads with the queue held (Driver's hold), so early
	// reuse shows on every driver, including one that copies at submission
	// (llvmpipe).
	for ( const Case &c : cases )
	{
		gl::GlAdapterOptions broken = options;
		broken.sensitivity = c.defect;
		const std::string failures = FailuresOf( Driver( "under-test", broken ) );
		const bool detected = failures.find( c.clause ) != std::string::npos;
		checks.That( detected, std::string( "detects " ) + c.name + " on " + c.clause );
		checks.That(
		    OnlyClause( failures, c.clause ), std::string( c.name ) + " fails no other clause" );
		if ( !detected || !OnlyClause( failures, c.clause ) )
			std::printf( "%s:\n%s", c.name, failures.c_str() );
	}
	return checks.Report();
}

#else

namespace
{

// Facts: the GLSL 4.50 artifact format and the backend's name; every
// capability not claimed is listed, not failed.
void FactsClauses( testing::Checks &checks, const IRenderDevice2 &device )
{
	const DeviceFacts &facts = device.Facts();
	checks.That( facts.artifactFormat == kFormat,
	    "gl.facts the device accepts its dialect's GLSL artifacts" );
	checks.That( facts.diagnosticBackend == kName && !facts.adapterName.empty(),
	    "gl.facts the device names its backend and adapter" );
	std::string claimed, unclaimed;
	for ( std::uint32_t bit = 0; bit < static_cast<std::uint32_t>( Capability::kCount ); ++bit )
	{
		const Capability capability = static_cast<Capability>( bit );
		std::string &list = facts.capabilities.Has( capability ) ? claimed : unclaimed;
		list += list.empty() ? "" : ", ";
		list += CapabilityName( capability );
	}
	std::printf( "INFO gl adapter: %.*s\nINFO gl claims: %s\nINFO gl does not claim: %s\n",
	    static_cast<int>( facts.adapterName.size() ), facts.adapterName.data(), claimed.c_str(),
	    unclaimed.c_str() );
}

// The ring: an encoder destroyed unsubmitted returns its range, a full ring
// copies from the command's own storage and counts it, and ranges are
// reused once their token completes.
void RingClauses( testing::Checks &checks, const rendertest::DeviceDriver &driver )
{
	gl::GlAdapterOptions options = Options();
	options.uploadRingBytes = 64 * 1024;
	auto created = gl::Create( options );
	checks.That( created.HasValue(), "gl.ring creates a device with a 64 KiB ring" );
	if ( !created )
		return;
	IRenderDevice2 &device = *created.Value();
	rendertest::detail::Suite suite( checks, driver );
	constexpr std::size_t kSize = 40 * 1024;
	const UsageSet usages{ ResourceUsage::kCopyDestination, ResourceUsage::kCopySource };
	const BufferId a = suite.Buffer( device, kSize, usages );
	const BufferId b = suite.Buffer( device, kSize, usages );
	const BufferId c = suite.Buffer( device, kSize, usages );
	auto upload = [&]( CommandEncoder &e, BufferId buffer, std::uint8_t seed )
	{
		e.TransitionBuffer( buffer, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		e.WriteBuffer( buffer, 0, rendertest::detail::Pattern( kSize, seed ) );
		e.TransitionBuffer( buffer, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	};
	{
		auto abandoned = device.BeginEncoder( QueueKind::kGraphics );
		if ( abandoned )
			upload( abandoned.Value(), a, 9 );
	}
	auto both = device.BeginEncoder( QueueKind::kGraphics );
	if ( !both )
		return;
	upload( both.Value(), a, 1 );
	checks.Equal( gl::DeferredUploads( device ), std::uint64_t( 0 ),
	    "gl.ring an encoder destroyed unsubmitted returns its range" );
	upload( both.Value(), b, 2 );
	checks.Equal( gl::DeferredUploads( device ), std::uint64_t( 1 ),
	    "gl.ring a full ring copies from the command's own storage and counts it" );
	const std::optional<CompletionToken> token = suite.Run( device, both.Value() );
	checks.That( token && WaitFor( device, *token ), "gl.ring the uploads complete" );
	(void)device.Poll();
	auto after = device.BeginEncoder( QueueKind::kGraphics );
	if ( !after )
		return;
	upload( after.Value(), c, 3 );
	checks.Equal( gl::DeferredUploads( device ), std::uint64_t( 1 ),
	    "gl.ring ranges are reused once their token completes" );
	const std::optional<CompletionToken> last = suite.Run( device, after.Value() );
	checks.That( last && WaitFor( device, *last ), "gl.ring the reused range completes" );
	checks.That(
	    suite.ReadBack( device, a, kSize ) == rendertest::detail::Pattern( kSize, 1 ) &&
	        suite.ReadBack( device, b, kSize ) == rendertest::detail::Pattern( kSize, 2 ) &&
	        suite.ReadBack( device, c, kSize ) == rendertest::detail::Pattern( kSize, 3 ),
	    "gl.ring ring and deferred uploads land unchanged" );
}

// A viewport's origin is its top-left corner (row 0 at the top): a
// full-screen draw into the top-left quarter viewport covers exactly the top
// left quarter of the target.
void ViewportClauses(
    testing::Checks &checks, rendertest::detail::Suite &suite, IRenderDevice2 &device )
{
	constexpr std::uint32_t kSize = 8;
	const rendertest::detail::ColorPipeline pipeline = rendertest::detail::MakeColorPipeline(
	    suite, device, rendertest::shaders::kFullScreenVertex, false );
	const float color[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
	const BufferId uniform =
	    suite.Buffer( device, 256, { ResourceUsage::kCopyDestination, ResourceUsage::kUniform } );
	const BindGroupEntry entry[] = { { 0, uniform, 0, 16, {}, {} } };
	auto group = device.CreateBindGroup( { pipeline.layouts[2], entry } );
	TextureDesc desc;
	desc.format = Format::kRGBA8Unorm;
	desc.width = kSize;
	desc.height = kSize;
	desc.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kCopySource };
	auto target = device.CreateTexture( desc );
	auto encoder = device.BeginEncoder( QueueKind::kGraphics );
	if ( !checks.That( pipeline.ok && group && target && encoder,
	         "gl.viewport the pipeline, target and group are created" ) )
		return;
	CommandEncoder &e = encoder.Value();
	e.TransitionBuffer( uniform, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.WriteBuffer( uniform, 0, std::as_bytes( std::span( color ) ) );
	e.TransitionBuffer( uniform, ResourceUsage::kCopyDestination, ResourceUsage::kUniform );
	e.TransitionTexture(
	    target.Value(), ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
	const ColorAttachment attachments[] = {
	    { target.Value(), LoadOp::kClear, StoreOp::kStore, { 0, 0, 0, 1 }, {} } };
	RenderingDesc rendering;
	rendering.colors = attachments;
	rendering.width = kSize;
	rendering.height = kSize;
	e.BeginRendering( rendering );
	e.SetPipeline( pipeline.pipeline );
	e.SetBindGroup( BindGroupRole::kMaterial, group.Value() );
	e.SetViewport( { 0, 0, kSize / 2, kSize / 2, 0, 1 } );
	e.Draw( 3 );
	e.EndRendering();
	e.TransitionTexture(
	    target.Value(), ResourceUsage::kColorAttachment, ResourceUsage::kCopySource );
	const std::vector<std::byte> pixels =
	    rendertest::detail::ReadTexture( suite, device, e, target.Value(), kSize );
	bool placed = pixels.size() == kSize * kSize * 4;
	for ( std::uint32_t y = 0; placed && y < kSize; ++y )
	{
		for ( std::uint32_t x = 0; placed && x < kSize; ++x )
		{
			const bool inside = x < kSize / 2 && y < kSize / 2;
			placed = pixels[( y * kSize + x ) * 4 + 1] == std::byte( inside ? 255 : 0 );
		}
	}
	checks.That( placed, "gl.viewport a viewport at the origin covers the top-left quarter" );
}

// The render sequence may move between threads (the main thread, then the
// material system's queue thread): a device made on one thread submits and
// completes from another, and a device call leaves the caller's own current
// context current (or none, if it had none).
void ThreadClauses( testing::Checks &checks, const rendertest::DeviceDriver &driver )
{
	rendertest::detail::Suite suite( checks, driver );
	std::unique_ptr<IRenderDevice2> device = suite.Create();
	if ( !device )
		return;
	const BufferId buffer = suite.Buffer(
	    *device, 256, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	const std::vector<std::byte> bytes = rendertest::detail::Pattern( 256, 21 );
	bool completed = false;
	std::thread other(
	    [&]
	    {
		    auto encoder = device->BeginEncoder( QueueKind::kGraphics );
		    if ( !encoder )
			    return;
		    encoder.Value().TransitionBuffer(
		        buffer, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		    encoder.Value().WriteBuffer( buffer, 0, bytes );
		    encoder.Value().TransitionBuffer(
		        buffer, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
		    const std::optional<CompletionToken> token = suite.Run( *device, encoder.Value() );
		    completed = token && WaitFor( *device, *token );
	    } );
	other.join();
	checks.That( completed, "gl.threads a submission from another thread completes" );
	checks.That( suite.ReadBack( *device, buffer, 256 ) == bytes,
	    "gl.threads this thread reads what the other thread's submission wrote" );
	checks.That( eglGetCurrentContext() == EGL_NO_CONTEXT,
	    "gl.threads device calls leave no context current on a thread that had none" );

	// A context of the caller's own (a host's), current around device calls.
	EGLDisplay display =
	    eglGetPlatformDisplay( EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, nullptr );
	EGLint major = 0, minor = 0;
	if ( !checks.That( display != EGL_NO_DISPLAY && eglInitialize( display, &major, &minor ) &&
	                       eglBindAPI( kEglApi ),
	         "gl.threads a host context's display initializes" ) )
		return;
	const EGLint attributes[] = { EGL_CONTEXT_MAJOR_VERSION, 3, EGL_NONE };
	EGLContext host = eglCreateContext( display, EGL_NO_CONFIG_KHR, EGL_NO_CONTEXT, attributes );
	const bool current =
	    host != EGL_NO_CONTEXT && eglMakeCurrent( display, EGL_NO_SURFACE, EGL_NO_SURFACE, host );
	if ( checks.That( current, "gl.threads a host context is current" ) )
	{
		const BufferId more = suite.Buffer( *device, 64, { ResourceUsage::kCopyDestination } );
		(void)device->Poll();
		checks.That( more.IsValid() && eglGetCurrentContext() == host,
		    "gl.threads device calls restore the caller's current context" );
		eglMakeCurrent( display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
	}
	if ( host != EGL_NO_CONTEXT )
		eglDestroyContext( display, host );
}

// A profile masks compute: the device claims neither compute nor storage
// buffers, and refuses a compute pipeline and a storage layout by status.
void MaskClauses( testing::Checks &checks )
{
	gl::GlAdapterOptions options = Options();
	options.allowed.Remove( Capability::kCompute ).Remove( Capability::kStorageBuffers );
	auto created = gl::Create( options );
	if ( !checks.That( created.HasValue(), "gl.mask a device without compute is created" ) )
		return;
	IRenderDevice2 &device = *created.Value();
	checks.That( !device.Facts().capabilities.Has( Capability::kCompute ) &&
	                 !device.Facts().capabilities.Has( Capability::kStorageBuffers ),
	    "gl.mask the masked capabilities are not claimed" );
	static const BindingDesc storage[] = {
	    { 0, BindingKind::kStorageBuffer, 1, { ShaderStage::kCompute } } };
	auto layout = device.CreateBindGroupLayout( { BindGroupRole::kDraw, storage } );
	checks.That( !layout && layout.Error().status == DeviceStatus::kUnsupported,
	    "gl.mask a storage binding fails kUnsupported" );
	const ShaderArtifactView stage[] = { { ShaderStage::kCompute, kFormat,
	    Artifact( rendertest::shaders::kDoubleCompute ), "main", {} } };
	PipelineDesc desc;
	desc.kind = PipelineKind::kCompute;
	desc.stages = stage;
	auto pipeline = device.CreatePipeline( desc );
	checks.That( !pipeline && pipeline.Error().status == DeviceStatus::kUnsupported,
	    "gl.mask a compute pipeline fails kUnsupported" );
	DeviceRequest request;
	request.required = { Capability::kCompute };
	const DeviceProviderDescriptor &descriptor =
	    kApi == gl::GlApiKind::kEs31 ? gl::DescribeEs() : gl::Describe();
	checks.That( descriptor.id == kName, "gl.mask the descriptor is named for its dialect" );
	auto described = descriptor.create( request );
	checks.That( described.HasValue(), "gl.mask the descriptor makes a device with compute" );
	request.required = { Capability::kRayQuery };
	auto missing = descriptor.create( request );
	checks.That( !missing && missing.Error().status == DeviceStatus::kUnsupported,
	    "gl.mask the descriptor refuses a required capability the adapter lacks" );
}

} // namespace

int main()
{
	testing::Checks checks;
	std::atomic<std::uint64_t> messages{ 0 };
	gl::GlAdapterOptions options = Options();
	options.validationCounter = &messages;
	{
		auto probe = gl::Create( options );
		if ( !checks.That(
		         probe.HasValue(), "gl.setup an OpenGL 4.5 core (ES 3.1) context is available" ) )
			return checks.Report();
		FactsClauses( checks, *probe.Value() );
	}

	const rendertest::DeviceDriver driver = Driver( kName, options );
	rendertest::RunDeviceConformance( checks, driver );
	rendertest::RunRasterConformance( checks, driver );
	gl::GlAdapterOptions small = options;
	small.uploadRingBytes = 256 * 1024;
	rendertest::RunDeviceConformance(
	    checks, Driver( ( std::string( kName ) + "-small-ring" ).c_str(), small ) );
	RingClauses( checks, driver );
	{
		rendertest::detail::Suite suite( checks, driver );
		std::unique_ptr<IRenderDevice2> device = suite.Create();
		if ( device )
			ViewportClauses( checks, suite, *device );
	}
	ThreadClauses( checks, driver );
	MaskClauses( checks );

	// Once more under GL debug output: no error or high/medium message.
	gl::GlAdapterOptions validated = options;
	validated.validation = true;
	const std::string validatedName = std::string( kName ) + "-validated";
	rendertest::RunDeviceConformance( checks, Driver( validatedName.c_str(), validated ) );
	rendertest::RunRasterConformance( checks, Driver( validatedName.c_str(), validated ) );
	std::printf(
	    "INFO gl.validation messages: %llu\n", static_cast<unsigned long long>( messages.load() ) );
	checks.That( messages.load() == 0,
	    "gl.validation the suite runs without a GL error or debug-output message" );
	return checks.Report();
}

#endif // RENDER_DEVICE_GL_SENSITIVITY
