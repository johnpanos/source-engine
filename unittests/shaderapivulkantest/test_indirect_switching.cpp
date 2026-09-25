//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.indirect-switching on native Vulkan (RFC 0011 G3.3): the
//          producer switcher (render/indirect_switcher.h) driving the native
//          probe volume that PBRMetalRough models sample per pixel, with real
//          frames and the provider's completion serials. Baked -> scripted
//          fake and back, a failed Begin, and a simulated device loss
//          mid-fade (the world's GPU resources dropped and recreated). Every
//          frame's mean over the probe-lit quad stays within tolerance of the
//          seed's; the fade's peak residency stays within budget; with the
//          validation layer present, no retired image is used after release.
//
//===========================================================================//

#include "../../materialsystem/shaderapivulkan/sdl3/sdl3_vulkan_surface_host.h"
#include "../../materialsystem/shaderapivulkan/vulkan_device.h"
#include "../../materialsystem/shaderapivulkan/vulkan_world_lightmap.h"
#include "render/indirect_switcher.h"
#include "testing/conformance_result.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace
{
using render_vulkan::CVulkanContext;
using namespace indirect_light;

int g_checks = 0;
int g_failures = 0;

void Check( bool condition, const std::string &message )
{
	++g_checks;
	if ( !condition )
	{
		++g_failures;
		std::fprintf( stderr, "FAIL: %s\n", message.c_str() );
	}
}

std::shared_ptr<const Volume> LoadSeed()
{
	std::vector<unsigned char> bytes;
	if ( std::FILE *file = std::fopen( "quality/fixtures/gi/prbv/gpu.prbv", "rb" ) )
	{
		unsigned char block[4096];
		size_t read;
		while ( ( read = std::fread( block, 1, sizeof( block ), file ) ) > 0 )
			bytes.insert( bytes.end(), block, block + read );
		std::fclose( file );
	}
	return Volume::FromBytes( std::move( bytes ) );
}

// The provider's own serials: a producer's resources and the switcher's
// retirement follow them.
class ContextTracker final : public IResourceTracker
{
public:
	explicit ContextTracker( CVulkanContext &context ) : m_context( context ) {}
	uint64_t Acquire( size_t ) override { return ++m_next; }
	void Release( uint64_t, uint64_t ) override {}
	uint64_t SubmittedSerial() const override { return m_context.SubmittedFrameSerial(); }
	uint64_t CompletedSerial() const override
	{
		// The newest serial the device reports complete (a zero-timeout query).
		uint64_t completed = m_completed;
		while ( completed < m_context.SubmittedFrameSerial() &&
		        m_context.WaitForSubmittedFrame( completed + 1, 0 ) )
			++completed;
		m_completed = completed;
		return completed;
	}

private:
	CVulkanContext &m_context;
	uint64_t m_next = 0;
	mutable uint64_t m_completed = 0;
};

class Catalog final : public IProducerCatalog
{
public:
	bool Offers( ProducerKind kind, const IndirectScene & ) const override
	{
		return kind == ProducerKind::Baked || kind == ProducerKind::ScriptedFake;
	}
	std::unique_ptr<IProducer> Create( ProducerKind kind ) override
	{
		if ( kind == ProducerKind::Baked )
			return std::make_unique<BakedProducer>();
		if ( kind != ProducerKind::ScriptedFake )
			return nullptr;
		if ( failNext )
		{
			failNext = false;
			// A fake whose scene lacks its data: Begin must fail cleanly.
			struct Failing final : IProducer
			{
				ProducerCaps Caps() const override { return ScriptedFakeProducer().Caps(); }
				foundation::Expected<void, IndirectError> Begin(
				    const IndirectScene &, const PublishedVolume &, IResourceTracker & ) override
				{
					return foundation::MakeUnexpected( IndirectError::MissingSceneData );
				}
				void Schedule( FrameWork &, const light_set::Snapshot & ) override {}
				std::optional<PublishedVolume> Published() const override { return std::nullopt; }
				RetireTicket End() override { return {}; }
			};
			return std::make_unique<Failing>();
		}
		// A visible switch: the indirect light 1.5 times the seed's.
		return std::make_unique<ScriptedFakeProducer>( 1.5f, 3 );
	}
	bool failNext = false;
};

bool Upload( CVulkanContext &context, const Volume &volume, std::string *error )
{
	std::vector<float> table( volume.layout.gridCount * mapcontainer::kProbeGridTableFloats );
	mapcontainer::WriteProbeGridTable( volume.layout, table.data() );
	world_mesh_gpu::ProbeVolumeUploadRequest request;
	request.atlasWidth = volume.layout.atlasWidth;
	request.atlasHeight = volume.layout.atlasHeight;
	request.atlas = volume.bytes.data() + volume.layout.atlasOffset;
	request.gridCount = volume.layout.gridCount;
	request.tableFloats = mapcontainer::kProbeGridTableFloats;
	request.gridTable = table.data();
	return render_vulkan::UploadWorldProbeVolume( context, request, error );
}

struct Images
{
	int base, mrao, normal, emission, env;
};

// Draws the probe-lit quad in the indirect view (the volume's indirect
// layer per pixel, no albedo) and returns the mean red level over its
// covered part of the grid (x <= 0.2).
bool DrawFrame( CVulkanContext &context, const Images &images, float *mean, std::string *error )
{
	context.ClearDynamicQueue();
	context.SetClearColor( 0, 0, 0, 1 );
	CVulkanContext::SkinConstants constants;
	std::memset( constants.ps, 0, sizeof( constants.ps ) );
	const float identity[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	std::memcpy( constants.viewProj, identity, sizeof( identity ) );
	const float row0[4] = { 1, 0, 0, 0 };
	const float row1[4] = { 0, 1, 0, 0 };
	std::memcpy( constants.texXform0, row0, sizeof( row0 ) );
	std::memcpy( constants.texXform1, row1, sizeof( row1 ) );
	constants.eyePos[2] = 0.5f - 1000.0f;
	context.SetDynamicSkinConstants( constants );
	context.SelectDynamicShader( CVulkanContext::kDynShaderPbrModel );
	context.SelectDynamicColorSpace(
	    CVulkanContext::kColorSrgbReadBase | CVulkanContext::kColorSrgbWrite );
	context.BindManagedTexture( images.base );
	context.BindManagedSampler( 10, images.mrao );
	context.BindManagedSampler( 1, images.normal );
	context.BindManagedSampler( 2, images.emission );
	context.BindManagedSampler( 3, images.env );
	const float quad[6][8] = {
	    { -0.5f, -0.5f, 0.5f, 1, 0, 0, 0, 0 },
	    { 0.5f, 0.5f, 0.5f, 1, 0, 0, 1, 1 },
	    { 0.5f, -0.5f, 0.5f, 1, 0, 0, 1, 0 },
	    { -0.5f, -0.5f, 0.5f, 1, 0, 0, 0, 0 },
	    { -0.5f, 0.5f, 0.5f, 1, 0, 0, 0, 1 },
	    { 0.5f, 0.5f, 0.5f, 1, 0, 0, 1, 1 },
	};
	float normalTangent[6][7];
	float alpha[6] = {};
	for ( auto &frame : normalTangent )
	{
		const float values[7] = { 0, 0, -1, 1, 0, 0, 1 };
		std::memcpy( frame, values, sizeof( values ) );
	}
	context.QueueDynamicTriangles( &quad[0][0], 6, nullptr, &normalTangent[0][0], alpha );
	context.RequestCapture();
	bool skip = false;
	if ( !context.BeginFrame( &skip, error ) || skip || !context.EndFrame( error ) )
		return false;
	int width = 0, height = 0;
	const std::vector<uint8_t> &pixels = context.GetCapturedPixels( &width, &height );
	double sum = 0.0;
	int count = 0;
	for ( int py = 0; py < height; ++py )
	{
		for ( int px = 0; px < width; ++px )
		{
			// D3D9 clip space: world x = px / w * 2 - 1, y = 1 - py / h * 2.
			const float x = float( px ) / width * 2.0f - 1.0f;
			const float y = 1.0f - float( py ) / height * 2.0f;
			if ( x < -0.45f || x > 0.15f || y < -0.45f || y > 0.45f )
				continue;
			sum += pixels[( size_t( py ) * width + px ) * 4];
			++count;
		}
	}
	*mean = count ? float( sum / count ) : 0.0f;
	return count > 0;
}

} // namespace

int main()
{
	if ( !SDL_Init( SDL_INIT_VIDEO ) )
	{
		std::fprintf( stderr, "SKIP: SDL video unavailable: %s\n", SDL_GetError() );
		return 77;
	}
	SDL_Window *window = SDL_CreateWindow( "indirect-switching", 128, 128, SDL_WINDOW_VULKAN );
	if ( !window )
	{
		std::fprintf( stderr, "SKIP: Vulkan window unavailable: %s\n", SDL_GetError() );
		SDL_Quit();
		return 77;
	}
	std::string error;
	std::unique_ptr<render_vulkan::IVulkanSurfaceHost> host =
	    render_vulkan::MakeSdl3LegacySurfaceHost( window, &error );
	CVulkanContext context;
	render_vulkan::VulkanContextConfig config;
	config.appName = "indirect-switching";
	config.enableValidation = true;
#ifdef VULKAN_TEST_DESCRIPTOR_SET_LIMIT
	// As a device that binds only this many descriptor sets (the Vulkan
	// minimum is four): every PBR and GI variant must still be built.
	config.descriptorSetLimit = VULKAN_TEST_DESCRIPTOR_SET_LIMIT;
#endif
	if ( !host || !context.Init( *host, config, &error ) )
	{
		std::fprintf( stderr, "SKIP: native Vulkan context unavailable: %s\n", error.c_str() );
		SDL_DestroyWindow( window );
		SDL_Quit();
		return 77;
	}
	Check( context.InitDynamicMesh( &error ) && context.PbrModelPipelineSupported() &&
	           context.ProbeVolumeSamplingSupported(),
	    "model PBR samples the probe volume per pixel on this device" );
	const std::shared_ptr<const Volume> seed = LoadSeed();
	Check( seed != nullptr, "the switching seed (quality/fixtures/gi/prbv/gpu.prbv) loads" );
	Images images{};
	images.base = context.CreateManagedTexture(
	    1, 1, VK_FORMAT_R8G8B8A8_UNORM, &error, 0, 1, VK_FORMAT_R8G8B8A8_SRGB );
	images.mrao = context.CreateManagedTexture( 1, 1, VK_FORMAT_R8G8B8A8_UNORM, &error );
	images.normal = context.CreateManagedTexture( 1, 1, VK_FORMAT_R8G8B8A8_UNORM, &error );
	images.emission = context.CreateManagedTexture( 1, 1, VK_FORMAT_R8G8B8A8_UNORM, &error );
	images.env = context.CreateManagedTexture(
	    1, 1, VK_FORMAT_R16G16B16A16_SFLOAT, &error, 0, 1, VK_FORMAT_UNDEFINED, true );
	const std::array<uint8_t, 4> grey = { 190, 190, 190, 255 };
	const std::array<uint8_t, 4> mrao = { 0, 230, 255, 255 };
	const std::array<uint8_t, 4> flat = { 128, 128, 255, 255 };
	const std::array<uint8_t, 4> black = { 0, 0, 0, 255 };
	const std::array<uint16_t, 4> envTexel = { 0, 0, 0, 0x3c00 };
	bool ready = seed && images.base >= 0 && images.env >= 0 &&
	             context.UploadManagedTexture( images.base, grey.data(), 4, &error ) &&
	             context.UploadManagedTexture( images.mrao, mrao.data(), 4, &error ) &&
	             context.UploadManagedTexture( images.normal, flat.data(), 4, &error ) &&
	             context.UploadManagedTexture( images.emission, black.data(), 4, &error );
	for ( uint32_t face = 0; ready && face < 6; ++face )
		ready = context.UploadManagedTexture(
		    images.env, reinterpret_cast<const uint8_t *>( envTexel.data() ), 8, &error, 0, face );
	const std::array<uint8_t, 3 * 40> vertices = {};
	const std::array<uint32_t, 3> indices = { 0, 1, 2 };
	ready = ready && context.UploadWorldMesh( vertices.data(), vertices.size(), indices.data(),
	                     sizeof( indices ), &error );
	Check( ready, "images and the map-scoped world mesh are resident" );
	if ( !ready || g_failures )
	{
		context.Shutdown();
		SDL_DestroyWindow( window );
		SDL_Quit();
		return testing::ReportConformance( g_checks, g_failures );
	}
	context.SetIndirectLightView( 1, 2.0f );

	ContextTracker tracker( context );
	Catalog catalog;
	Switcher switcher( catalog, tracker, 8 );
	IndirectScene scene;
	scene.baked = seed;
	Check( bool( switcher.BeginMap( scene, ProducerKind::Baked ) ), "the map begins Baked" );
	uint64_t uploaded = 0;
	float seedMean = -1.0f;
	float lowest = 1e9f;
	int fadeFrames = 0;
	int uploads = 0;
	const auto frame = [&]( const char *what ) -> float
	{
		FrameWork work;
		work.frameSerial = context.SubmittedFrameSerial() + 1;
		work.resources = &tracker;
		const light_set::Snapshot lights;
		const FrameVolume published = switcher.Frame( work, lights );
		for ( auto &job : work.jobs )
			job();
		if ( published.volume && published.generation != uploaded )
		{
			uploaded = published.generation;
			++uploads;
			Check( Upload( context, *published.volume, &error ),
			    std::string( what ) + ": the published volume uploads" );
		}
		fadeFrames += published.fading;
		float mean = 0.0f;
		Check(
		    DrawFrame( context, images, &mean, &error ), std::string( what ) + ": frame renders" );
		if ( seedMean < 0.0f )
			seedMean = mean;
		lowest = std::min( lowest, mean / seedMean );
		return mean;
	};
	for ( int i = 0; i < 3; ++i )
		frame( "baked" );
	const float bakedMean = seedMean;
	Check( bakedMean > 20.0f, "the probe-lit quad is lit by the baked volume" );

	// A failed Begin keeps Baked and changes nothing.
	catalog.failNext = true;
	const auto failed = switcher.Select( ProducerKind::ScriptedFake );
	const uint64_t before = uploaded;
	const float kept = frame( "after a failed Begin" );
	Check( !failed && switcher.Active() == ProducerKind::Baked && uploaded == before &&
	           std::abs( kept - bakedMean ) < 0.5f,
	    "a failed Begin keeps Baked and its frame unchanged" );

	// Baked -> fake: a visible, gradual switch with no black frame.
	Check( bool( switcher.Select( ProducerKind::ScriptedFake ) ), "the fake begins" );
	for ( int i = 0; i < 20; ++i )
		frame( "baked -> fake" );
	const float fakeMean = frame( "fake" );
	Check( switcher.Active() == ProducerKind::ScriptedFake && fadeFrames >= 7,
	    "the fake becomes active over the declared fade" );
	std::fprintf( stderr,
	    "switch means: baked %.1f fake %.1f lowest %.3f of the seed, %d uploads\n", bakedMean,
	    fakeMean, lowest, uploads );
	Check( fakeMean > bakedMean + 5.0f, "the fake's brighter indirect light reaches the pixels" );
	Check( switcher.PeakResidentBytes() <= 3 * seed->bytes.size(),
	    "the fade holds at most the old, the new and the blended volume" );

	// Device loss mid-fade back to Baked (simulated: the world's GPU
	// resources are dropped and recreated, as after a lost device).
	Check( bool( switcher.Select( ProducerKind::Baked ) ), "selecting Baked again" );
	bool lostMidFade = false;
	for ( int i = 0; i < 30 && !lostMidFade; ++i )
	{
		FrameWork work;
		work.frameSerial = context.SubmittedFrameSerial() + 1;
		work.resources = &tracker;
		const light_set::Snapshot lights;
		const FrameVolume peek = switcher.Frame( work, lights );
		if ( peek.fading && peek.weight > 0.3f )
		{
			lostMidFade = true;
			switcher.DeviceLost();
			context.ReleaseWorldMesh();
			Check( !context.ProbeVolumeResident(), "the lost device's probe volume is gone" );
			Check( context.UploadWorldMesh( vertices.data(), vertices.size(), indices.data(),
			           sizeof( indices ), &error ),
			    "the world mesh returns on the recovered device" );
			Check( bool( switcher.DeviceRecovered() ), "recovery restores the selection" );
			uploaded = 0; // the recovered device needs the volume again
		}
		else
		{
			if ( peek.volume && peek.generation != uploaded )
			{
				uploaded = peek.generation;
				Check( Upload( context, *peek.volume, &error ), "fade frame uploads" );
			}
			float mean = 0.0f;
			Check( DrawFrame( context, images, &mean, &error ), "fade frame renders" );
			lowest = std::min( lowest, mean / seedMean );
		}
	}
	Check( lostMidFade, "device loss arrives mid-fade" );
	for ( int i = 0; i < 20; ++i )
		frame( "after recovery" );
	const float recovered = frame( "recovered" );
	Check( switcher.Active() == ProducerKind::Baked && std::abs( recovered - bakedMean ) <= 1.0f,
	    "after recovery the selection (Baked) is back, as it was" );
	std::fprintf( stderr, "lowest frame mean %.3f of the seed's\n", lowest );
	Check( lowest >= 0.95f, "no black frame: every frame's mean stays within 5% of the seed's" );
	switcher.EndMap();
	for ( int i = 0; i < 4; ++i )
		frame( "after the map" );
	Check( switcher.RetiredCount() == 0, "ended producers retire behind the completion serial" );
	context.SetIndirectLightView( 0, 1.0f );
	context.ReleaseWorldMesh();
	std::fprintf( stderr, "validation layer %s, %u message(s)\n",
	    context.ValidationEnabled() ? "on" : "unavailable", context.ValidationErrorCount() );
	if ( context.ValidationEnabled() )
		Check( context.ValidationErrorCount() == 0,
		    "no validation message: no retired image was used after release" );
	for ( int handle : { images.base, images.mrao, images.normal, images.emission, images.env } )
		if ( handle >= 0 )
			context.DestroyManagedTexture( handle );
	context.Shutdown();
	SDL_DestroyWindow( window );
	SDL_Quit();
	return testing::ReportConformance( g_checks, g_failures );
}
