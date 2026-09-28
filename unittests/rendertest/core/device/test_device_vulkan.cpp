//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.v2 on render.device.vulkan (RFC 0016 K1): the shared
//			suite against a real, headless Vulkan device (no surface, no
//			window), then the adapter's own clauses:
//
//			- vulkan-small-ring: the suite again with a 256 KiB upload ring, so
//			  upload ranges are reused and a full ring takes staging buffers;
//			- vulkan.ring: a full ring defers exactly the upload that cannot
//			  fit, an abandoned encoder returns its range, and ranges are reused
//			  once their token completes;
//			- vulkan.compute, vulkan.sampled, vulkan.multisample: the paths the
//			  shared suite does not reach (dispatch ordering on a storage
//			  buffer, sampled textures and samplers, vertex and index input,
//			  blending, back-face culling and a 4x resolve), with pixels;
//			- vulkan.validation: with the Khronos validation layer (including
//			  synchronization validation) the suite reports no warning or
//			  error, counted through teardown. Without the layer installed the
//			  clause prints SKIP and certifies nothing.
//
//			Device loss cannot be forced on Vulkan, so D7 is reported as
//			skipped here; the null adapter's suite covers the clause.
//			RENDER_VK_VALIDATION=1 turns validation on for every run;
//			RENDER_VK_ADAPTER=<n> picks the physical device by index.
//			A missing Vulkan device fails the run: required runs fail on
//			missing providers.
//
//=============================================================================//

#include "device_conformance.h"
#include "render/device/vulkan/provider.h"
#include "../../../../render/device/vulkan/host_device.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <thread>

namespace
{

using namespace render::device;

// Adapter-local SPIR-V fixtures, built from
// unittests/rendertest/core/device/shaders/ by the pinned compiler and
// verified by `tools/render/shader_toolchain.py check`, like test_shaders.h.
//
// double.comp:
//   layout( local_size_x = 64 ) in;
//   layout( set = 3, binding = 0 ) buffer Data { uint values[]; };
//   values[i] = values[i] * 2u + 1u; (i = gl_GlobalInvocationID.x)
// sampled.frag:
//   layout( set = 2, binding = 0 ) uniform texture2D image;
//   layout( set = 2, binding = 1 ) uniform sampler point;
//   outColor = texture( sampler2D( image, point ), gl_FragCoord.xy / textureSize( ... ) );
// position.vert:
//   layout( location = 0 ) in vec2 position;
//   gl_Position = vec4( position, 0.5, 1.0 );
namespace spirv
{

inline constexpr std::uint32_t kDoubleCompute[] = { 0x07230203, 0x00010300, 0x000d000b, 0x00000022,
    0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
    0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0006000f, 0x00000005, 0x00000004, 0x6e69616d,
    0x00000000, 0x0000000b, 0x00060010, 0x00000004, 0x00000011, 0x00000040, 0x00000001, 0x00000001,
    0x00040047, 0x0000000b, 0x0000000b, 0x0000001c, 0x00040047, 0x00000010, 0x00000006, 0x00000004,
    0x00030047, 0x00000011, 0x00000002, 0x00050048, 0x00000011, 0x00000000, 0x00000023, 0x00000000,
    0x00040047, 0x00000013, 0x00000021, 0x00000000, 0x00040047, 0x00000013, 0x00000022, 0x00000003,
    0x00040047, 0x00000021, 0x0000000b, 0x00000019, 0x00020013, 0x00000002, 0x00030021, 0x00000003,
    0x00000002, 0x00040015, 0x00000006, 0x00000020, 0x00000000, 0x00040017, 0x00000009, 0x00000006,
    0x00000003, 0x00040020, 0x0000000a, 0x00000001, 0x00000009, 0x0004003b, 0x0000000a, 0x0000000b,
    0x00000001, 0x0004002b, 0x00000006, 0x0000000c, 0x00000000, 0x00040020, 0x0000000d, 0x00000001,
    0x00000006, 0x0003001d, 0x00000010, 0x00000006, 0x0003001e, 0x00000011, 0x00000010, 0x00040020,
    0x00000012, 0x0000000c, 0x00000011, 0x0004003b, 0x00000012, 0x00000013, 0x0000000c, 0x00040015,
    0x00000014, 0x00000020, 0x00000001, 0x0004002b, 0x00000014, 0x00000015, 0x00000000, 0x00040020,
    0x00000018, 0x0000000c, 0x00000006, 0x0004002b, 0x00000006, 0x0000001b, 0x00000002, 0x0004002b,
    0x00000006, 0x0000001d, 0x00000001, 0x0004002b, 0x00000006, 0x00000020, 0x00000040, 0x0006002c,
    0x00000009, 0x00000021, 0x00000020, 0x0000001d, 0x0000001d, 0x00050036, 0x00000002, 0x00000004,
    0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x00050041, 0x0000000d, 0x0000000e, 0x0000000b,
    0x0000000c, 0x0004003d, 0x00000006, 0x0000000f, 0x0000000e, 0x00060041, 0x00000018, 0x00000019,
    0x00000013, 0x00000015, 0x0000000f, 0x0004003d, 0x00000006, 0x0000001a, 0x00000019, 0x00050084,
    0x00000006, 0x0000001c, 0x0000001a, 0x0000001b, 0x00050080, 0x00000006, 0x0000001e, 0x0000001c,
    0x0000001d, 0x0003003e, 0x00000019, 0x0000001e, 0x000100fd, 0x00010038 };

#if !defined( RENDER_DEVICE_VULKAN_SENSITIVITY )
// Used by the suite's own clauses only, not by the sensitivity build.
inline constexpr std::uint32_t kSampledFragment[] = { 0x07230203, 0x00010300, 0x000d000b,
    0x00000024, 0x00000000, 0x00020011, 0x00000001, 0x00020011, 0x00000032, 0x0006000b, 0x00000001,
    0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0007000f,
    0x00000004, 0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x00000015, 0x00030010, 0x00000004,
    0x00000007, 0x00040047, 0x00000009, 0x0000001e, 0x00000000, 0x00040047, 0x0000000c, 0x00000021,
    0x00000000, 0x00040047, 0x0000000c, 0x00000022, 0x00000002, 0x00040047, 0x00000010, 0x00000021,
    0x00000001, 0x00040047, 0x00000010, 0x00000022, 0x00000002, 0x00040047, 0x00000015, 0x0000000b,
    0x0000000f, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006,
    0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040020, 0x00000008, 0x00000003,
    0x00000007, 0x0004003b, 0x00000008, 0x00000009, 0x00000003, 0x00090019, 0x0000000a, 0x00000006,
    0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000001, 0x00000000, 0x00040020, 0x0000000b,
    0x00000000, 0x0000000a, 0x0004003b, 0x0000000b, 0x0000000c, 0x00000000, 0x0002001a, 0x0000000e,
    0x00040020, 0x0000000f, 0x00000000, 0x0000000e, 0x0004003b, 0x0000000f, 0x00000010, 0x00000000,
    0x0003001b, 0x00000012, 0x0000000a, 0x00040020, 0x00000014, 0x00000001, 0x00000007, 0x0004003b,
    0x00000014, 0x00000015, 0x00000001, 0x00040017, 0x00000016, 0x00000006, 0x00000002, 0x00040015,
    0x0000001c, 0x00000020, 0x00000001, 0x0004002b, 0x0000001c, 0x0000001d, 0x00000000, 0x00040017,
    0x0000001f, 0x0000001c, 0x00000002, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003,
    0x000200f8, 0x00000005, 0x0004003d, 0x0000000a, 0x0000000d, 0x0000000c, 0x0004003d, 0x0000000e,
    0x00000011, 0x00000010, 0x00050056, 0x00000012, 0x00000013, 0x0000000d, 0x00000011, 0x0004003d,
    0x00000007, 0x00000017, 0x00000015, 0x0007004f, 0x00000016, 0x00000018, 0x00000017, 0x00000017,
    0x00000000, 0x00000001, 0x0004003d, 0x0000000a, 0x00000019, 0x0000000c, 0x0004003d, 0x0000000e,
    0x0000001a, 0x00000010, 0x00050056, 0x00000012, 0x0000001b, 0x00000019, 0x0000001a, 0x00040064,
    0x0000000a, 0x0000001e, 0x0000001b, 0x00050067, 0x0000001f, 0x00000020, 0x0000001e, 0x0000001d,
    0x0004006f, 0x00000016, 0x00000021, 0x00000020, 0x00050088, 0x00000016, 0x00000022, 0x00000018,
    0x00000021, 0x00050057, 0x00000007, 0x00000023, 0x00000013, 0x00000022, 0x0003003e, 0x00000009,
    0x00000023, 0x000100fd, 0x00010038 };

inline constexpr std::uint32_t kPositionVertex[] = { 0x07230203, 0x00010300, 0x000d000b, 0x0000001b,
    0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
    0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0007000f, 0x00000000, 0x00000004, 0x6e69616d,
    0x00000000, 0x0000000d, 0x00000012, 0x00030047, 0x0000000b, 0x00000002, 0x00050048, 0x0000000b,
    0x00000000, 0x0000000b, 0x00000000, 0x00050048, 0x0000000b, 0x00000001, 0x0000000b, 0x00000001,
    0x00050048, 0x0000000b, 0x00000002, 0x0000000b, 0x00000003, 0x00050048, 0x0000000b, 0x00000003,
    0x0000000b, 0x00000004, 0x00040047, 0x00000012, 0x0000001e, 0x00000000, 0x00020013, 0x00000002,
    0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007,
    0x00000006, 0x00000004, 0x00040015, 0x00000008, 0x00000020, 0x00000000, 0x0004002b, 0x00000008,
    0x00000009, 0x00000001, 0x0004001c, 0x0000000a, 0x00000006, 0x00000009, 0x0006001e, 0x0000000b,
    0x00000007, 0x00000006, 0x0000000a, 0x0000000a, 0x00040020, 0x0000000c, 0x00000003, 0x0000000b,
    0x0004003b, 0x0000000c, 0x0000000d, 0x00000003, 0x00040015, 0x0000000e, 0x00000020, 0x00000001,
    0x0004002b, 0x0000000e, 0x0000000f, 0x00000000, 0x00040017, 0x00000010, 0x00000006, 0x00000002,
    0x00040020, 0x00000011, 0x00000001, 0x00000010, 0x0004003b, 0x00000011, 0x00000012, 0x00000001,
    0x0004002b, 0x00000006, 0x00000014, 0x3f000000, 0x0004002b, 0x00000006, 0x00000015, 0x3f800000,
    0x00040020, 0x00000019, 0x00000003, 0x00000007, 0x00050036, 0x00000002, 0x00000004, 0x00000000,
    0x00000003, 0x000200f8, 0x00000005, 0x0004003d, 0x00000010, 0x00000013, 0x00000012, 0x00050051,
    0x00000006, 0x00000016, 0x00000013, 0x00000000, 0x00050051, 0x00000006, 0x00000017, 0x00000013,
    0x00000001, 0x00070050, 0x00000007, 0x00000018, 0x00000016, 0x00000017, 0x00000014, 0x00000015,
    0x00050041, 0x00000019, 0x0000001a, 0x0000000d, 0x0000000f, 0x0003003e, 0x0000001a, 0x00000018,
    0x000100fd, 0x00010038 };
#endif

} // namespace spirv

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

rendertest::DeviceDriver Driver( const char *name, vulkan::VulkanAdapterOptions options )
{
	rendertest::DeviceDriver driver;
	driver.name = name;
	driver.create = [options]() -> std::unique_ptr<IRenderDevice2>
	{
		auto device = vulkan::Create( options );
		if ( !device )
		{
			std::fprintf( stderr, "render.device.vulkan: no device: %s in %s (VkResult %d)\n",
			    DescribeStatus( device.Error().status ),
			    DescribeOperation( device.Error().operation ), device.Error().nativeCode );
			return nullptr;
		}
		return std::move( device ).Value();
	};
	driver.complete = WaitFor;
	driver.holdsCompletion = false;
	driver.rasterizes = true;
	driver.doubleCompute = spirv::kDoubleCompute;
	return driver;
}

#if !defined( RENDER_DEVICE_VULKAN_SENSITIVITY )

// Signals value on the host device's timeline from its graphics queue, as a
// host's own submission does.
bool SignalFromQueue( vulkan::IHostDevice &host, std::uint64_t value )
{
	VkSemaphore timeline = host.Timeline();
	VkTimelineSemaphoreSubmitInfo values{};
	values.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
	values.signalSemaphoreValueCount = 1;
	values.pSignalSemaphoreValues = &value;
	VkSubmitInfo submit{};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.pNext = &values;
	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &timeline;
	host.LockQueue();
	const VkResult result = vkQueueSubmit( host.Info().graphicsQueue, 1, &submit, VK_NULL_HANDLE );
	host.UnlockQueue();
	return result == VK_SUCCESS;
}

// vulkan.host: the interop a legacy host borrows the device through
// (render/device/vulkan/host_device.h). The adapter creates the device, its
// memory and its completion; the host's resources are released only behind
// timeline values, never before, and the port runs on the same device.
void HostClauses( testing::Checks &checks, bool validation )
{
	vulkan::HostDeviceRequest request;
	request.applicationName = "render.device.v2 host clauses";
	request.validation = validation;
	char error[256] = {};
	std::unique_ptr<vulkan::IHostDevice> host =
	    vulkan::HostDeviceFactory().Create( request, error, sizeof( error ) );
	checks.That( host != nullptr, "vulkan.host the factory creates a device without a surface" );
	if ( !host )
	{
		std::fprintf( stderr, "vulkan.host: %s\n", error );
		return;
	}
	const vulkan::HostDeviceInfo &info = host->Info();
	checks.That( info.instance != VK_NULL_HANDLE && info.physical != VK_NULL_HANDLE &&
	                 info.device != VK_NULL_HANDLE && info.graphicsQueue != VK_NULL_HANDLE &&
	                 info.surface == VK_NULL_HANDLE,
	    "vulkan.host exposes the instance, device and queue it created" );
	checks.That( host->Port().Facts().diagnosticBackend == "vulkan",
	    "vulkan.host the same device serves render.device.v2" );

	// The adapter's own allocations (its upload ring) are counted too.
	const std::uint64_t base = host->LiveAllocations();
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = 4096;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	vulkan::HostMemory upload;
	upload.required = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	upload.mapped = true;
	VkBuffer buffer = VK_NULL_HANDLE;
	vulkan::HostAllocation bufferMemory = nullptr;
	void *mapped = nullptr;
	const bool bufferMade =
	    host->CreateBuffer( bufferInfo, upload, &buffer, &bufferMemory, &mapped ) == VK_SUCCESS &&
	    mapped != nullptr &&
	    ( host->Properties( bufferMemory ) & upload.required ) == upload.required;
	checks.That( bufferMade, "vulkan.host a mapped host-visible buffer is created" );
	if ( mapped )
		std::memset( mapped, 0x5a, 4096 );
	void *again = nullptr;
	checks.That( host->Map( bufferMemory, &again ) == VK_SUCCESS && again == mapped,
	    "vulkan.host mapping a mapped allocation returns the same address" );
	host->Unmap( bufferMemory );

	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageInfo.extent = { 64, 64, 1 };
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	vulkan::HostMemory local;
	local.required = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	VkImage image = VK_NULL_HANDLE;
	vulkan::HostAllocation imageMemory = nullptr;
	checks.That( host->CreateImage( imageInfo, local, &image, &imageMemory ) == VK_SUCCESS &&
	                 host->LiveAllocations() == base + 2,
	    "vulkan.host a device-local image is created and both allocations are counted" );

	// Releases wait for their value, whichever order values complete in.
	const std::uint64_t first = host->NextSubmitValue();
	const std::uint64_t second = host->NextSubmitValue();
	checks.That( second == first + 1 && host->SubmittedValue() == second,
	    "vulkan.host reserved values rise by one" );
	host->ReleaseBufferAfter( second, buffer, bufferMemory );
	host->ReleaseImageAfter( first, image, imageMemory );
	static int destroyed = 0;
	destroyed = 0;
	host->ReleaseAfter(
	    second,
	    []( void * )
	    {
		    ++destroyed;
	    },
	    nullptr );
	checks.That(
	    host->Collect() == 0 && host->PendingReleases() == 3 && host->LiveAllocations() == base + 2,
	    "vulkan.host nothing is released before its value is signaled" );
	checks.That(
	    SignalFromQueue( *host, first ) && host->WaitValue( first, 5000000000ull ) == VK_SUCCESS,
	    "vulkan.host the first value completes" );
	const std::size_t early = host->Collect();
	checks.That( early == 1 && host->LiveAllocations() == base + 1 && destroyed == 0,
	    "vulkan.host only the release behind the completed value runs" );
	// A reserved value its host never signals is abandoned, not waited on.
	host->AbandonValue( second );
	checks.That( host->WaitValue( second, 5000000000ull ) == VK_SUCCESS,
	    "vulkan.host an abandoned value still completes in order" );
	checks.That( host->Collect() == 2 && host->LiveAllocations() == base && destroyed == 1 &&
	                 host->PendingReleases() == 0,
	    "vulkan.host every release runs once its value completes" );

	// The port's submissions and the host's share one timeline.
	IRenderDevice2 &port = host->Port();
	auto encoder = port.BeginEncoder( QueueKind::kGraphics );
	std::optional<CompletionToken> token;
	if ( encoder )
	{
		auto submitted = port.Submit( QueueKind::kGraphics, { &encoder.Value(), 1 }, {} );
		if ( submitted )
			token = submitted.Value();
	}
	checks.That( token && token->value == second + 1 && WaitFor( port, *token ),
	    "vulkan.host a port submission takes the next timeline value" );
	checks.That( host->NextSubmitValue() == second + 2,
	    "vulkan.host a host reservation follows the port's submission" );
	host->AbandonValue( second + 2 );
	(void)host->WaitValue( second + 2, 5000000000ull );

	// RFC 0016 K3: host work inside a port encoder, and binary semaphores on
	// port submissions (the legacy frame's acquire and present).
	VkBufferCreateInfo fillInfo{};
	fillInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	fillInfo.size = 256;
	fillInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	VkBuffer fill = VK_NULL_HANDLE;
	vulkan::HostAllocation fillMemory = nullptr;
	void *fillMapped = nullptr;
	const bool fillMade =
	    host->CreateBuffer( fillInfo, upload, &fill, &fillMemory, &fillMapped ) == VK_SUCCESS;
	if ( fillMapped )
		std::memset( fillMapped, 0, 256 );
	struct Native
	{
		VkBuffer buffer;
		int calls = 0;
	} native{ fill };
	auto nativeEncoder = port.BeginEncoder( QueueKind::kGraphics );
	std::optional<CompletionToken> nativeToken;
	if ( fillMade && nativeEncoder )
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VkSemaphore binary = VK_NULL_HANDLE;
		vkCreateSemaphore( host->Info().device, &semaphoreInfo, nullptr, &binary );
		host->RecordNative(
		    nativeEncoder.Value(),
		    []( void *user, VkCommandBuffer cmd )
		    {
			    Native &state = *static_cast<Native *>( user );
			    ++state.calls;
			    vkCmdFillBuffer( cmd, state.buffer, 0, 256, 0x5043A5A5u );
		    },
		    &native );
		host->AddSubmitSignal( nativeEncoder.Value(), binary );
		auto submitted = port.Submit( QueueKind::kGraphics, { &nativeEncoder.Value(), 1 }, {} );
		// The next submission waits on the binary semaphore the first signaled.
		auto waiting = port.BeginEncoder( QueueKind::kGraphics );
		std::optional<CompletionToken> waitToken;
		if ( submitted && waiting )
		{
			host->AddSubmitWait( waiting.Value(), binary, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT );
			auto second = port.Submit( QueueKind::kGraphics, { &waiting.Value(), 1 }, {} );
			if ( second )
				waitToken = second.Value();
		}
		if ( submitted )
			nativeToken = submitted.Value();
		checks.That( waitToken && WaitFor( port, *waitToken ),
		    "vulkan.host a submission waits on a binary semaphore another signaled" );
		(void)host->WaitValue( host->SubmittedValue(), 5000000000ull );
		vkDestroySemaphore( host->Info().device, binary, nullptr );
	}
	bool filled = nativeToken && WaitFor( port, *nativeToken ) && native.calls == 1 && fillMapped;
	for ( std::size_t i = 0; filled && i < 64; ++i )
		filled = static_cast<const std::uint32_t *>( fillMapped )[i] == 0x5043A5A5u;
	checks.That( filled, "vulkan.host native work runs once, in its encoder's submission" );
	auto nullNative = port.BeginEncoder( QueueKind::kGraphics );
	if ( nullNative )
	{
		host->RecordNative( nullNative.Value(), nullptr, nullptr );
		auto refused = port.Submit( QueueKind::kGraphics, { &nullNative.Value(), 1 }, {} );
		checks.That( !refused && refused.Error().status == DeviceStatus::kInvalidState,
		    "vulkan.host native work without a recorder fails its submission" );
	}
	host->ReleaseBufferAfter( host->SubmittedValue(), fill, fillMemory );
	(void)host->Collect();

	// A host whose surface cannot be created gets a named reason.
	vulkan::HostDeviceRequest noSurface;
	noSurface.createSurface = []( void *, VkInstance, VkSurfaceKHR * )
	{
		return false;
	};
	char reason[256] = {};
	const bool refused =
	    vulkan::HostDeviceFactory().Create( noSurface, reason, sizeof( reason ) ) == nullptr;
	checks.That( refused && std::strstr( reason, "surface" ) != nullptr,
	    "vulkan.host a failed surface fails creation with a named reason" );
}

void DescriptorClauses( testing::Checks &checks )
{
	auto device = vulkan::Describe().create( DeviceRequest{ { Capability::kCompute }, false } );
	checks.That( vulkan::Describe().id == "vulkan" && device.HasValue(),
	    "vulkan.descriptor creates a device with the capabilities it claims" );
	if ( device )
	{
		const DeviceFacts &facts = device.Value()->Facts();
		checks.That(
		    facts.diagnosticBackend == "vulkan" && !facts.adapterName.empty() &&
		        facts.artifactFormat == ArtifactFormat::kSpirv &&
		        CapabilitySet( facts.capabilities ).Remove( Capability::kExternalImages ) ==
		            CapabilitySet{ Capability::kCompute, Capability::kStorageBuffers },
		    "vulkan.facts name the backend and adapter and claim only compute and storage "
		    "(and external images where the driver exports dmabufs, D18)" );
		auto compute = device.Value()->BeginEncoder( QueueKind::kCompute );
		checks.That( !compute && compute.Error().status == DeviceStatus::kUnsupported,
		    "vulkan.queues a compute-queue encoder is unsupported (no async compute)" );
	}
	auto missing =
	    vulkan::Describe().create( DeviceRequest{ { Capability::kTransientAliasing }, false } );
	checks.That( !missing && missing.Error().status == DeviceStatus::kUnsupported &&
	                 missing.Error().operation == DeviceOperation::kCreateDevice,
	    "vulkan.descriptor a required capability it lacks fails creation with kUnsupported" );
}

// A 64 KiB ring and 40 KiB uploads: two in one encoder cannot both fit, since
// the first is not submitted yet, so exactly one takes a staging buffer.
void RingClauses( testing::Checks &checks, const rendertest::DeviceDriver &driver,
    const vulkan::VulkanAdapterOptions &driverOptions )
{
	vulkan::VulkanAdapterOptions options;
	options.uploadRingBytes = 64 * 1024;
	options.adapterIndex = driverOptions.adapterIndex;
	options.validation = driverOptions.validation;
	options.validationCounter = driverOptions.validationCounter;
	auto created = vulkan::Create( options );
	checks.That( created.HasValue(), "vulkan.ring creates a device with a 64 KiB ring" );
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
	checks.Equal( vulkan::DeferredUploads( device ), std::uint64_t( 0 ),
	    "vulkan.ring an encoder destroyed unsubmitted returns its range" );
	upload( both.Value(), b, 2 );
	checks.Equal( vulkan::DeferredUploads( device ), std::uint64_t( 1 ),
	    "vulkan.ring a full ring gives the upload a staging buffer and counts it" );
	const std::optional<CompletionToken> token = suite.Run( device, both.Value() );
	checks.That( token && WaitFor( device, *token ), "vulkan.ring the uploads complete" );
	(void)device.Poll();

	auto after = device.BeginEncoder( QueueKind::kGraphics );
	if ( !after )
		return;
	upload( after.Value(), c, 3 );
	checks.Equal( vulkan::DeferredUploads( device ), std::uint64_t( 1 ),
	    "vulkan.ring ranges are reused once their token completes" );
	const std::optional<CompletionToken> last = suite.Run( device, after.Value() );
	checks.That( last && WaitFor( device, *last ), "vulkan.ring the reused range completes" );
	checks.That(
	    suite.ReadBack( device, a, kSize ) == rendertest::detail::Pattern( kSize, 1 ) &&
	        suite.ReadBack( device, b, kSize ) == rendertest::detail::Pattern( kSize, 2 ) &&
	        suite.ReadBack( device, c, kSize ) == rendertest::detail::Pattern( kSize, 3 ),
	    "vulkan.ring ring and staging uploads land unchanged" );
}

// Copies a texture in kCopySource out through a buffer and reads it back.
std::vector<std::byte> ReadTexture( rendertest::detail::Suite &suite, IRenderDevice2 &device,
    CommandEncoder &e, TextureId texture, std::uint32_t size )
{
	const std::uint64_t bytes = std::uint64_t( size ) * size * 4;
	const BufferId out = suite.Buffer(
	    device, bytes, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	e.TransitionBuffer( out, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.CopyTextureToBuffer( texture, out, { 0, 0, 0, size, size } );
	e.TransitionBuffer( out, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	const std::optional<CompletionToken> token = suite.Run( device, e );
	if ( !token || !WaitFor( device, *token ) )
		return {};
	return suite.ReadBack( device, out, bytes );
}

bool Near( std::byte actual, int expected )
{
	const int value = static_cast<int>( actual );
	return value >= expected - 1 && value <= expected + 1;
}

// Two dispatches on one storage buffer (the second reads what the first
// wrote, in the same usage, so the adapter must order them).
void ComputeClauses(
    testing::Checks &checks, rendertest::detail::Suite &suite, IRenderDevice2 &device )
{
	using rendertest::detail::Code;
	constexpr std::uint32_t kCount = 256;
	static const BindingDesc storage[] = {
	    { 0, BindingKind::kStorageBuffer, 1, { ShaderStage::kCompute } } };
	auto layout = device.CreateBindGroupLayout( { BindGroupRole::kDraw, storage } );
	if ( !layout )
		return;
	const BindGroupLayoutId layouts[] = { {}, {}, {}, layout.Value() };
	static const ReflectedBinding used[] = { { 3, 0, BindingKind::kStorageBuffer } };
	const ShaderArtifactView stage[] = { { ShaderStage::kCompute, ArtifactFormat::kSpirv,
	    Code( spirv::kDoubleCompute ), "main", used } };
	PipelineDesc desc;
	desc.kind = PipelineKind::kCompute;
	desc.stages = stage;
	desc.layouts = layouts;
	auto pipeline = device.CreatePipeline( desc );
	checks.That( pipeline.HasValue(), "vulkan.compute a compute pipeline is created" );
	const BufferId data = suite.Buffer( device, kCount * 4,
	    { ResourceUsage::kCopyDestination, ResourceUsage::kStorageWrite,
	        ResourceUsage::kCopySource } );
	const BindGroupEntry entry[] = { { 0, data, 0, 0, {}, {} } };
	auto group = device.CreateBindGroup( { layout.Value(), entry } );
	auto encoder = device.BeginEncoder( QueueKind::kGraphics );
	if ( !pipeline || !group || !encoder )
		return;
	std::vector<std::uint32_t> values( kCount );
	for ( std::uint32_t i = 0; i < kCount; ++i )
		values[i] = i;
	CommandEncoder &e = encoder.Value();
	e.TransitionBuffer( data, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.WriteBuffer( data, 0, std::as_bytes( std::span<const std::uint32_t>( values ) ) );
	e.TransitionBuffer( data, ResourceUsage::kCopyDestination, ResourceUsage::kStorageWrite );
	e.SetPipeline( pipeline.Value() );
	e.SetBindGroup( BindGroupRole::kDraw, group.Value() );
	e.Dispatch( kCount / 64 );
	e.Dispatch( kCount / 64 );
	e.TransitionBuffer( data, ResourceUsage::kStorageWrite, ResourceUsage::kCopySource );
	const std::optional<CompletionToken> token = suite.Run( device, e );
	checks.That( token && WaitFor( device, *token ), "vulkan.compute two dispatches submit" );
	const std::vector<std::byte> read = suite.ReadBack( device, data, kCount * 4 );
	bool ordered = read.size() == kCount * 4;
	for ( std::uint32_t i = 0; ordered && i < kCount; ++i )
	{
		std::uint32_t value = 0;
		std::memcpy( &value, read.data() + i * 4, 4 );
		ordered = value == 4 * i + 3;
	}
	checks.That( ordered, "vulkan.compute the second dispatch reads the first one's writes" );
}

// A texture uploaded through a buffer, sampled with a nearest sampler at
// texel centers: every texel lands on the pixel with the same coordinates
// (row 0 at the top for textures and framebuffers alike).
void SampledClauses(
    testing::Checks &checks, rendertest::detail::Suite &suite, IRenderDevice2 &device )
{
	using rendertest::detail::Code;
	constexpr std::uint32_t kSize = 4;
	static const BindingDesc material[] = {
	    { 0, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 1, BindingKind::kSampler, 1, { ShaderStage::kFragment } } };
	auto layout = device.CreateBindGroupLayout( { BindGroupRole::kMaterial, material } );
	if ( !layout )
		return;
	const BindGroupLayoutId layouts[] = { {}, {}, layout.Value() };
	static const ReflectedBinding used[] = {
	    { 2, 0, BindingKind::kSampledTexture }, { 2, 1, BindingKind::kSampler } };
	const ShaderArtifactView stages[] = {
	    { ShaderStage::kVertex, ArtifactFormat::kSpirv,
	        Code( rendertest::shaders::kFullScreenVertex ), "main", {} },
	    { ShaderStage::kFragment, ArtifactFormat::kSpirv, Code( spirv::kSampledFragment ), "main",
	        used } };
	const Format colors[] = { Format::kRGBA8Unorm };
	PipelineDesc desc;
	desc.stages = stages;
	desc.layouts = layouts;
	desc.colorFormats = colors;
	desc.raster.cull = CullMode::kNone;
	auto pipeline = device.CreatePipeline( desc );

	TextureDesc image;
	image.format = Format::kRGBA8Unorm;
	image.width = kSize;
	image.height = kSize;
	image.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	auto texture = device.CreateTexture( image );
	image.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kCopySource };
	auto target = device.CreateTexture( image );
	SamplerDesc point;
	point.minFilter = point.magFilter = point.mipFilter = Filter::kNearest;
	point.address = AddressMode::kClampToEdge;
	auto sampler = device.CreateSampler( point );
	checks.That( pipeline && texture && target && sampler,
	    "vulkan.sampled a sampling pipeline, texture and sampler are created" );
	if ( !pipeline || !texture || !target || !sampler )
		return;
	const BindGroupEntry entries[] = {
	    { 0, {}, 0, 0, texture.Value(), {} }, { 1, {}, 0, 0, {}, sampler.Value() } };
	auto group = device.CreateBindGroup( { layout.Value(), entries } );
	const std::vector<std::byte> texels = rendertest::detail::Pattern( kSize * kSize * 4, 5 );
	const BufferId staging = suite.Buffer(
	    device, texels.size(), { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	auto encoder = device.BeginEncoder( QueueKind::kGraphics );
	if ( !group || !encoder )
		return;
	CommandEncoder &e = encoder.Value();
	e.TransitionBuffer( staging, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.WriteBuffer( staging, 0, texels );
	e.TransitionBuffer( staging, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	e.TransitionTexture(
	    texture.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.CopyBufferToTexture( staging, texture.Value(), { 0, 0, 0, kSize, kSize } );
	e.TransitionTexture(
	    texture.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
	e.TransitionTexture(
	    target.Value(), ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
	const ColorAttachment attachment[] = {
	    { target.Value(), LoadOp::kClear, StoreOp::kStore, {}, {} } };
	RenderingDesc rendering;
	rendering.colors = attachment;
	rendering.width = kSize;
	rendering.height = kSize;
	e.BeginRendering( rendering );
	e.SetPipeline( pipeline.Value() );
	e.SetBindGroup( BindGroupRole::kMaterial, group.Value() );
	e.Draw( 3 );
	e.EndRendering();
	e.TransitionTexture(
	    target.Value(), ResourceUsage::kColorAttachment, ResourceUsage::kCopySource );
	const std::vector<std::byte> pixels = ReadTexture( suite, device, e, target.Value(), kSize );
	checks.That( pixels == texels,
	    "vulkan.sampled texels sampled at their centers land on the same pixels (row 0 on top)" );
}

// An indexed draw from a vertex buffer into a 4x multisampled target that
// resolves: a counter-clockwise quad on the left half blends additively over
// the clear color; a clockwise quad on the right half is culled as a back face.
void MultisampleClauses(
    testing::Checks &checks, rendertest::detail::Suite &suite, IRenderDevice2 &device )
{
	using rendertest::detail::Code;
	constexpr std::uint32_t kSize = 8;
	if ( !( device.Facts().limits.sampleCounts & 4u ) )
	{
		std::printf( "SKIP vulkan.multisample the adapter has no 4x sample count\n" );
		return;
	}
	static const BindingDesc material[] = {
	    { 0, BindingKind::kUniformBuffer, 1, { ShaderStage::kFragment } } };
	auto layout = device.CreateBindGroupLayout( { BindGroupRole::kMaterial, material } );
	if ( !layout )
		return;
	const BindGroupLayoutId layouts[] = { {}, {}, layout.Value() };
	static const ReflectedBinding used[] = { { 2, 0, BindingKind::kUniformBuffer } };
	const ShaderArtifactView stages[] = { { ShaderStage::kVertex, ArtifactFormat::kSpirv,
	                                          Code( spirv::kPositionVertex ), "main", {} },
	    { ShaderStage::kFragment, ArtifactFormat::kSpirv,
	        Code( rendertest::shaders::kColorFragment ), "main", used } };
	const VertexAttribute attributes[] = { { 0, VertexFormat::kFloat2, 0, 0 } };
	const VertexBufferLayout buffers[] = { { 8, false } };
	const Format colors[] = { Format::kRGBA8Unorm };
	const BlendMode blends[] = { BlendMode::kAdditive };
	PipelineDesc desc;
	desc.stages = stages;
	desc.layouts = layouts;
	desc.vertex = { attributes, buffers };
	desc.colorFormats = colors;
	desc.blends = blends;
	desc.raster = { CullMode::kBack, true };
	desc.sampleCount = 4;
	auto pipeline = device.CreatePipeline( desc );

	TextureDesc target;
	target.format = Format::kRGBA8Unorm;
	target.width = kSize;
	target.height = kSize;
	target.sampleCount = 4;
	target.usages = { ResourceUsage::kColorAttachment };
	auto multisampled = device.CreateTexture( target );
	target.sampleCount = 1;
	target.usages = { ResourceUsage::kResolveDestination, ResourceUsage::kCopySource };
	auto resolved = device.CreateTexture( target );
	checks.That( pipeline && multisampled && resolved,
	    "vulkan.multisample a 4x pipeline and its targets are created" );
	if ( !pipeline || !multisampled || !resolved )
		return;

	// Left half counter-clockwise (front), right half clockwise (back), clip Y up.
	const float vertices[] = { -1, -1, 0, -1, 0, 1, -1, 1, 0, -1, 0, 1, 1, 1, 1, -1 };
	const std::uint16_t indices[] = { 0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7 };
	const float color[4] = { 0.4f, 0.2f, 0.0f, 1.0f };
	const UsageSet upload{ ResourceUsage::kCopyDestination };
	const BufferId vertexBuffer = suite.Buffer(
	    device, sizeof( vertices ), UsageSet( upload ).Add( ResourceUsage::kVertex ) );
	const BufferId indexBuffer =
	    suite.Buffer( device, sizeof( indices ), UsageSet( upload ).Add( ResourceUsage::kIndex ) );
	const BufferId uniform =
	    suite.Buffer( device, 256, UsageSet( upload ).Add( ResourceUsage::kUniform ) );
	const BindGroupEntry entry[] = { { 0, uniform, 0, 16, {}, {} } };
	auto group = device.CreateBindGroup( { layout.Value(), entry } );
	auto encoder = device.BeginEncoder( QueueKind::kGraphics );
	if ( !group || !encoder )
		return;
	CommandEncoder &e = encoder.Value();
	auto fill = [&]( BufferId buffer, std::span<const std::byte> bytes, ResourceUsage usage )
	{
		e.TransitionBuffer( buffer, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		e.WriteBuffer( buffer, 0, bytes );
		e.TransitionBuffer( buffer, ResourceUsage::kCopyDestination, usage );
	};
	fill( vertexBuffer, std::as_bytes( std::span( vertices ) ), ResourceUsage::kVertex );
	fill( indexBuffer, std::as_bytes( std::span( indices ) ), ResourceUsage::kIndex );
	fill( uniform, std::as_bytes( std::span( color ) ), ResourceUsage::kUniform );
	e.TransitionTexture(
	    multisampled.Value(), ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
	e.TransitionTexture(
	    resolved.Value(), ResourceUsage::kUndefined, ResourceUsage::kResolveDestination );
	const ColorAttachment attachment[] = { { multisampled.Value(), LoadOp::kClear,
	    StoreOp::kDiscard, { 0.2f, 0.0f, 0.0f, 1.0f }, resolved.Value() } };
	RenderingDesc rendering;
	rendering.colors = attachment;
	rendering.width = kSize;
	rendering.height = kSize;
	e.BeginRendering( rendering );
	e.SetPipeline( pipeline.Value() );
	e.SetBindGroup( BindGroupRole::kMaterial, group.Value() );
	e.SetVertexBuffer( 0, vertexBuffer );
	e.SetIndexBuffer( indexBuffer, 0, IndexFormat::kUint16 );
	e.DrawIndexed( 12 );
	e.EndRendering();
	e.TransitionTexture(
	    resolved.Value(), ResourceUsage::kResolveDestination, ResourceUsage::kCopySource );
	const std::vector<std::byte> pixels = ReadTexture( suite, device, e, resolved.Value(), kSize );
	bool left = pixels.size() == kSize * kSize * 4;
	bool right = left;
	for ( std::uint32_t y = 0; left && y < kSize; ++y )
	{
		for ( std::uint32_t x = 0; x < kSize; ++x )
		{
			const std::byte *p = pixels.data() + ( y * kSize + x ) * 4;
			if ( x < kSize / 2 )
				left &=
				    Near( p[0], 153 ) && Near( p[1], 51 ) && Near( p[2], 0 ) && Near( p[3], 255 );
			else
				right &=
				    Near( p[0], 51 ) && Near( p[1], 0 ) && Near( p[2], 0 ) && Near( p[3], 255 );
		}
	}
	checks.That( left,
	    "vulkan.multisample the front-facing quad blends additively and resolves on the left" );
	checks.That( right, "vulkan.multisample the back-facing quad is culled (CCW front, Y up)" );
}

void FeatureClauses( testing::Checks &checks, const rendertest::DeviceDriver &driver )
{
	rendertest::detail::Suite suite( checks, driver );
	std::unique_ptr<IRenderDevice2> device = suite.Create();
	if ( !device )
		return;
	ComputeClauses( checks, suite, *device );
	SampledClauses( checks, suite, *device );
	MultisampleClauses( checks, suite, *device );
	(void)device->WaitIdle();
	(void)device->Poll();
}

#endif // !RENDER_DEVICE_VULKAN_SENSITIVITY

} // namespace

#if defined( RENDER_DEVICE_VULKAN_SENSITIVITY )

// render.device.v2.vulkan.sensitivity: the shared suite must fail each bad
// Vulkan adapter on the clause it breaks, and on no other, while the
// undecorated control passes. The adapter's sensitivity knobs
// (VulkanAdapterOptions::Sensitivity) are never set by a product.
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

int main()
{
	testing::Checks checks;
	vulkan::VulkanAdapterOptions options;
	if ( const char *adapter = std::getenv( "RENDER_VK_ADAPTER" ) )
		options.adapterIndex = std::atoi( adapter );
	{
		const std::string failures = FailuresOf( Driver( "under-test", options ) );
		checks.That( failures.empty(), "control-passes" );
		if ( !failures.empty() )
			std::printf( "%s", failures.c_str() );
	}
	struct Case
	{
		const char *name;
		vulkan::VulkanAdapterOptions::Sensitivity defect;
		const char *clause;
	};
	vulkan::VulkanAdapterOptions::Sensitivity flipY;
	flipY.flipY = true;
	vulkan::VulkanAdapterOptions::Sensitivity glDepth;
	glDepth.glDepthRange = true;
	vulkan::VulkanAdapterOptions::Sensitivity asyncCompute;
	asyncCompute.falseClaims.Add( Capability::kAsyncCompute );
	vulkan::VulkanAdapterOptions::Sensitivity aliasing;
	aliasing.falseClaims.Add( Capability::kTransientAliasing );
	vulkan::VulkanAdapterOptions::Sensitivity writeMasks;
	writeMasks.ignoreColorWriteMasks = true;
	vulkan::VulkanAdapterOptions::Sensitivity staleExport;
	staleExport.staleExport = true;
	vulkan::VulkanAdapterOptions::Sensitivity nullExporter;
	nullExporter.nullExternalImages = true;
	const Case cases[] = {
	    { "flipped-y", flipY, "under-test.D13 clip y" },
	    { "gl-depth-range", glDepth, "under-test.D13 clip z" },
	    { "false-async-compute", asyncCompute, "under-test.D15 claimed async compute" },
	    { "false-aliasing", aliasing, "under-test.D15 claims transient-aliasing" },
	    { "ignored-write-masks", writeMasks, "under-test.D17 a red-and-alpha mask" },
	    { "stale-export", staleExport, "under-test.D18 the exported memory" },
	    { "null-exporter", nullExporter, "under-test.D18 the exporter is present" },
	};
	for ( const Case &c : cases )
	{
		vulkan::VulkanAdapterOptions broken = options;
		broken.sensitivity = c.defect;
		const std::string failures = FailuresOf( Driver( "under-test", broken ) );
		const bool detected = failures.find( c.clause ) != std::string::npos;
		checks.That( detected, std::string( "detects " ) + c.name + " on " + c.clause );
		checks.That(
		    OnlyClause( failures, c.clause ), std::string( c.name ) + " fails no other clause" );
		if ( !detected || !OnlyClause( failures, c.clause ) )
			std::printf( "%s: %s", c.name, failures.c_str() );
	}
	return checks.Report();
}

#else

int main()
{
	testing::Checks checks;
	const bool validation = std::getenv( "RENDER_VK_VALIDATION" ) != nullptr;
	const bool layer = vulkan::ValidationLayerAvailable();
	std::atomic<std::uint64_t> messages{ 0 };

	vulkan::VulkanAdapterOptions options;
	options.validation = validation;
	options.validationCounter = &messages;
	if ( const char *adapter = std::getenv( "RENDER_VK_ADAPTER" ) )
		options.adapterIndex = std::atoi( adapter );
	if ( auto probe = vulkan::Create( options ) )
	{
		const std::string_view name = probe.Value()->Facts().adapterName;
		std::printf(
		    "INFO vulkan suite adapter: %.*s\n", static_cast<int>( name.size() ), name.data() );
	}
	rendertest::RunDeviceConformance( checks, Driver( "vulkan", options ) );

	vulkan::VulkanAdapterOptions small = options;
	small.uploadRingBytes = 256 * 1024;
	rendertest::RunDeviceConformance( checks, Driver( "vulkan-small-ring", small ) );
	DescriptorClauses( checks );
	RingClauses( checks, Driver( "vulkan", options ), options );
	FeatureClauses( checks, Driver( "vulkan", options ) );
	HostClauses( checks, false );

	if ( !layer )
	{
		std::printf( "SKIP vulkan.validation layer not installed\n" );
		return checks.Report();
	}
	if ( !validation )
	{
		// The suite once more with the layer, synchronization validation on.
		vulkan::VulkanAdapterOptions validated;
		validated.validation = true;
		validated.validationCounter = &messages;
		validated.adapterIndex = options.adapterIndex;
		rendertest::RunDeviceConformance( checks, Driver( "vulkan-validated", validated ) );
		RingClauses( checks, Driver( "vulkan-validated", validated ), validated );
		FeatureClauses( checks, Driver( "vulkan-validated", validated ) );
	}
	{
		vulkan::VulkanAdapterOptions probe;
		probe.validation = true;
		probe.validationCounter = &messages;
		probe.adapterIndex = options.adapterIndex;
		auto device = vulkan::Create( probe );
		checks.That( device && vulkan::ValidationMessages( *device.Value() ) == 0,
		    "vulkan.validation a validated device reports no message after creation" );
	}
	std::printf( "INFO vulkan.validation messages: %llu\n",
	    static_cast<unsigned long long>( messages.load() ) );
	checks.That( messages.load() == 0,
	    "vulkan.validation the suite runs without a validation or synchronization message" );
	return checks.Report();
}

#endif // RENDER_DEVICE_VULKAN_SENSITIVITY
