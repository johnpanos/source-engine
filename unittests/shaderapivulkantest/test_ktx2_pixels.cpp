//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RFC 0008 F3 native BC7 package upload and pixel fixture.
//
//===========================================================================//

#include "../../materialsystem/shaderapivulkan/sdl3/sdl3_vulkan_surface_host.h"
#include "../../materialsystem/shaderapivulkan/vulkan_device.h"
#include "testing/conformance_result.h"
#ifdef RFC0008_KTX_READER
#include "texturecontainer/texture_image.h"
#include "../../materialsystem/shaderapivulkan/vulkan_texture_image.h"
#endif

#include <SDL3/SDL.h>

#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#ifdef RFC0008_KTX_READER
#include <fstream>
#include <iterator>
#include <span>
#endif
#include <memory>
#include <string>
#include <vector>

namespace
{

// Four 4x4 blocks fill an 8x8 image. These were produced from solid RGBA8
// sRGB sources by the pinned KTX-Software UASTC LDR -> BC7 toolchain.
constexpr std::array<std::uint8_t, 16> kRedBlock = { 0x20, 0xff, 0x3f, 0x00, 0x00, 0x00, 0xfc, 0xff,
    0xaf, 0xaa, 0xaa, 0xaa, 0x00, 0x00, 0x00, 0x00 };
constexpr std::array<std::uint8_t, 16> kBlueBlock = { 0x20, 0x00, 0x00, 0x00, 0xf0, 0xff, 0xff,
    0xff, 0xaf, 0xaa, 0xaa, 0xaa, 0x00, 0x00, 0x00, 0x00 };
constexpr std::array<std::uint8_t, 8> kBc4MaskBlock = {
    0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
constexpr std::array<std::uint8_t, 16> kBc5NormalBlock = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
constexpr std::array<std::uint8_t, 16> kBc6HdrBlock = { 0xaf, 0xf3, 0xad, 0x33, 0x06, 0x07, 0x2c,
    0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#ifdef RFC0008_KTX_READER
constexpr std::array<std::array<std::uint8_t, 3>, 3> kMipColors = {
    { { 0, 255, 0 }, { 255, 255, 0 }, { 0, 0, 255 } } };
#endif

constexpr float kQuad[6][8] = {
    { -0.5f, -0.5f, 0.5f, 1, 1, 1, 0, 0 },
    { 0.5f, 0.5f, 0.5f, 1, 1, 1, 1, 1 },
    { 0.5f, -0.5f, 0.5f, 1, 1, 1, 1, 0 },
    { -0.5f, -0.5f, 0.5f, 1, 1, 1, 0, 0 },
    { -0.5f, 0.5f, 0.5f, 1, 1, 1, 0, 1 },
    { 0.5f, 0.5f, 0.5f, 1, 1, 1, 1, 1 },
};

template <size_t N> std::vector<std::uint8_t> Blocks( const std::array<std::uint8_t, N> &block )
{
	std::vector<std::uint8_t> bytes;
	for ( int i = 0; i < 4; ++i )
		bytes.insert( bytes.end(), block.begin(), block.end() );
	return bytes;
}

bool DrawAndCapture( render_vulkan::CVulkanContext &context, int texture,
    std::array<std::uint8_t, 4> &center, std::array<std::uint8_t, 4> &corner, std::string &error,
    float textureScale = 1.0f, bool srgbRead = true )
{
	context.ClearDynamicQueue();
	context.SetClearColor( 0, 0, 0, 1 );
	context.SelectDynamicShader( render_vulkan::CVulkanContext::kDynShaderTextured );
	context.SelectDynamicColorSpace(
	    srgbRead ? render_vulkan::CVulkanContext::kColorSrgbReadBase : 0 );
	context.BindManagedTexture( texture );
	const float white[4] = { 1, 1, 1, 1 };
	context.SetDynamicModulation( white );
	std::array<float, 6 * 8> vertices = {};
	for ( std::size_t vertex = 0; vertex < 6; ++vertex )
	{
		for ( std::size_t component = 0; component < 8; ++component )
			vertices[vertex * 8 + component] = kQuad[vertex][component];
		vertices[vertex * 8 + 6] *= textureScale;
		vertices[vertex * 8 + 7] *= textureScale;
	}
	context.QueueDynamicTriangles( vertices.data(), 6 );
	context.RequestCapture();
	bool skip = false;
	if ( !context.BeginFrame( &skip, &error ) || skip || !context.EndFrame( &error ) )
		return false;
	int width = 0;
	int height = 0;
	const std::vector<std::uint8_t> &pixels = context.GetCapturedPixels( &width, &height );
	if ( width < 4 || height < 4 || pixels.size() < static_cast<size_t>( width * height * 4 ) )
	{
		error = "BC7 frame capture is empty or incomplete";
		return false;
	}
	const size_t middle = ( static_cast<size_t>( height / 2 ) * width + width / 2 ) * 4;
	for ( int channel = 0; channel < 4; ++channel )
	{
		center[channel] = pixels[middle + channel];
		corner[channel] = pixels[channel];
	}
	return true;
}

} // namespace

int main()
{
	if ( !SDL_Init( SDL_INIT_VIDEO ) )
	{
		std::fprintf( stderr, "SKIP: SDL video unavailable: %s\n", SDL_GetError() );
		return 77;
	}
	SDL_Window *window = SDL_CreateWindow( "ktx2-native-pixels", 128, 128, SDL_WINDOW_VULKAN );
	if ( !window )
	{
		std::fprintf( stderr, "SKIP: Vulkan window unavailable: %s\n", SDL_GetError() );
		SDL_Quit();
		return 77;
	}
	std::string error;
	std::unique_ptr<render_vulkan::IVulkanSurfaceHost> host =
	    render_vulkan::MakeSdl3LegacySurfaceHost( window, &error );
	render_vulkan::CVulkanContext context;
	render_vulkan::VulkanContextConfig config;
	config.appName = "ktx2-native-pixels";
	config.enableValidation = true;
	if ( !host || !context.Init( *host, config, &error ) )
	{
		std::fprintf( stderr, "SKIP: native Vulkan context unavailable: %s\n", error.c_str() );
		SDL_DestroyWindow( window );
		SDL_Quit();
		return 77;
	}
	std::printf( "KTX2_DEVICE vendorID=%u deviceID=%u name=%s\n", context.VendorId(),
	    context.DeviceId(), context.DeviceName() );
	int checks = 0;
	int failures = 0;
	const auto check = [&]( bool condition, const char *message )
	{
		++checks;
		if ( !condition )
		{
			++failures;
			std::fprintf( stderr, "FAIL: %s\n", message );
		}
	};
	const std::uint32_t maxDimension = context.MaxSampledTextureDimension();
	check( maxDimension >= 4096, "selected device reports a usable 2D texture limit" );
	check( context.MaxAnisotropicLevel() >= 1 && context.MaxAnisotropicLevel() <= 16,
	    "selected device reports a bounded anisotropic filter level" );
	if ( maxDimension < static_cast<std::uint32_t>( INT32_MAX ) )
	{
		const int oversized = context.CreateManagedTexture(
		    static_cast<int>( maxDimension + 1 ), 1, VK_FORMAT_R8G8B8A8_UNORM, &error );
		check( oversized < 0 && error.find( "2D image limit" ) != std::string::npos,
		    "oversized texture fails before image allocation" );
	}
	if ( maxDimension >= 16384 )
	{
		const int large =
		    context.CreateManagedTexture( 16384, 16384, VK_FORMAT_BC7_SRGB_BLOCK, &error, 0, 15 );
		check( large >= 0 && context.ManagedTextureMipLevels( large ) == 15,
		    "device allocates a 16K BC7 texture with its full mip chain" );
		if ( large >= 0 )
			context.DestroyManagedTexture( large );
	}
	const bool meshReady = context.InitDynamicMesh( &error );
	check( meshReady, "native textured pipeline initializes" );
	const int red = context.CreateManagedTexture( 8, 8, VK_FORMAT_BC7_SRGB_BLOCK, &error );
	const int blue = context.CreateManagedTexture( 8, 8, VK_FORMAT_BC7_SRGB_BLOCK, &error );
	const int mask = context.CreateManagedTexture( 8, 8, VK_FORMAT_BC4_UNORM_BLOCK, &error );
	const int normal = context.CreateManagedTexture( 8, 8, VK_FORMAT_BC5_UNORM_BLOCK, &error );
	const int hdr = context.CreateManagedTexture( 8, 8, VK_FORMAT_BC6H_UFLOAT_BLOCK, &error );
	const int hdrReference =
	    context.CreateManagedTexture( 8, 8, VK_FORMAT_R16G16B16A16_SFLOAT, &error );
	check( red >= 0 && blue >= 0, "device creates BC7 sRGB images" );
	check( mask >= 0 && normal >= 0 && hdr >= 0 && hdrReference >= 0,
	    "device creates BC4, BC5, BC6H and float reference images" );
	if ( meshReady && red >= 0 && blue >= 0 && mask >= 0 && normal >= 0 && hdr >= 0 &&
	     hdrReference >= 0 )
	{
		std::vector<std::uint8_t> redBlocks = Blocks( kRedBlock );
#ifdef RFC0008_KTX_READER
		int packagedHandle = -1;
		int packagedMipsHandle = -1;
		int packagedMraoHandle = -1;
		std::ifstream package( "quality/fixtures/ktx2/red-8x8-bc7.ktx2", std::ios::binary );
		std::vector<char> packageBytes(
		    std::istreambuf_iterator<char>{ package }, std::istreambuf_iterator<char>{} );
		const auto readerImage =
		    texturecontainer::ReadKtx2Image( std::as_bytes( std::span( packageBytes ) ) );
		check( readerImage.HasValue(), "packaged KTX2 reads before native upload" );
		if ( readerImage && readerImage.Value().format == texturecontainer::PixelFormat::Bc7Srgb &&
		     readerImage.Value().levels.size() == 1 )
		{
			const auto &level = readerImage.Value().levels[0];
			redBlocks.resize( level.bytes.size() );
			for ( std::size_t index = 0; index < level.bytes.size(); ++index )
				redBlocks[index] = static_cast<std::uint8_t>( level.bytes[index] );
			check( level.width == 8 && level.height == 8 && redBlocks == Blocks( kRedBlock ),
			    "KTX2 reader returns exact BC7 upload blocks" );
			auto shortImage = readerImage.Value();
			shortImage.levels[0].bytes.pop_back();
			const auto shortUpload =
			    render_vulkan::CreateManagedTextureImage( context, shortImage );
			check( !shortUpload && shortUpload.Error().code ==
			                           render_vulkan::TextureImageUploadError::InvalidImage,
			    "image bridge rejects short BC7 bytes before GPU creation" );
			auto unknownImage = readerImage.Value();
			unknownImage.format = static_cast<texturecontainer::PixelFormat>( 999 );
			const auto unknownUpload =
			    render_vulkan::CreateManagedTextureImage( context, unknownImage );
			check( !unknownUpload && unknownUpload.Error().code ==
			                             render_vulkan::TextureImageUploadError::UnsupportedFormat,
			    "image bridge rejects an unknown required format" );
			const auto packagedUpload =
			    render_vulkan::CreateManagedTextureImage( context, readerImage.Value() );
			check( packagedUpload.HasValue(), "image bridge creates and uploads packaged BC7" );
			if ( packagedUpload )
				packagedHandle = packagedUpload.Value();
		}
		else
		{
			check( false, "KTX2 package format and topology match the native image" );
		}
		std::ifstream maskPackage( "quality/fixtures/ktx2/mrao-8x8-bc7.ktx2", std::ios::binary );
		std::vector<char> maskPackageBytes(
		    std::istreambuf_iterator<char>{ maskPackage }, std::istreambuf_iterator<char>{} );
		const auto maskImage =
		    texturecontainer::ReadKtx2Image( std::as_bytes( std::span( maskPackageBytes ) ) );
		check( maskImage && maskImage.Value().format == texturecontainer::PixelFormat::Bc7Unorm,
		    "linear MRAO KTX2 reads as BC7 UNORM" );
		if ( maskImage )
		{
			const auto maskUpload =
			    render_vulkan::CreateManagedTextureImage( context, maskImage.Value() );
			check(
			    maskUpload.HasValue(), "linear BC7 MRAO uploads through the shared image bridge" );
			if ( maskUpload )
				packagedMraoHandle = maskUpload.Value();
		}
		std::ifstream mipPackage(
		    "quality/fixtures/ktx2/red-8x8-rgba8-mips.ktx2", std::ios::binary );
		std::vector<char> mipPackageBytes(
		    std::istreambuf_iterator<char>{ mipPackage }, std::istreambuf_iterator<char>{} );
		const auto mipImage =
		    texturecontainer::ReadKtx2Image( std::as_bytes( std::span( mipPackageBytes ) ) );
		check( mipImage.HasValue(), "four-mip KTX2 package reads" );
		if ( mipImage )
		{
			// Each lower level has a distinct color. Sampling by derivative
			// below proves the uploaded bytes, not merely the GPU mip count.
			auto markedMips = mipImage.Value();
			for ( std::size_t level = 1; level < markedMips.levels.size(); ++level )
			{
				auto &bytes = markedMips.levels[level].bytes;
				for ( std::size_t pixel = 0; pixel < bytes.size(); pixel += 4 )
				{
					bytes[pixel] = std::byte{ kMipColors[level - 1][0] };
					bytes[pixel + 1] = std::byte{ kMipColors[level - 1][1] };
					bytes[pixel + 2] = std::byte{ kMipColors[level - 1][2] };
				}
			}
			const auto mipUpload = render_vulkan::CreateManagedTextureImage( context, markedMips );
			check( mipUpload.HasValue(), "image bridge uploads every RGBA8 mip" );
			if ( mipUpload )
			{
				packagedMipsHandle = mipUpload.Value();
				check( context.ManagedTextureMipLevels( packagedMipsHandle ) == 4,
				    "GPU image keeps the authored four-level chain" );
				context.SetManagedTextureSamplerState(
				    packagedMipsHandle, render_vulkan::CVulkanContext::kSamplerMipPoint );
			}
		}
#endif
		const std::vector<std::uint8_t> blueBlocks = Blocks( kBlueBlock );
		const std::vector<std::uint8_t> maskBlocks = Blocks( kBc4MaskBlock );
		const std::vector<std::uint8_t> normalBlocks = Blocks( kBc5NormalBlock );
		const std::vector<std::uint8_t> hdrBlocks = Blocks( kBc6HdrBlock );
		std::vector<std::uint16_t> hdrReferencePixels;
		for ( int i = 0; i < 64; ++i )
		{
			// IEEE 754 half: 0.5, 0.25, 0.125, 1.0.
			hdrReferencePixels.insert(
			    hdrReferencePixels.end(), { 0x3800, 0x3400, 0x3000, 0x3c00 } );
		}
		check( !context.UploadManagedTexture( red, redBlocks.data(), redBlocks.size() - 1, &error ),
		    "short BC7 payload is rejected" );
		check( !context.UploadManagedTextureRegion( red, 1, 0, 4, 4, redBlocks.data(), 16, &error ),
		    "misaligned BC7 region is rejected" );
		check( !context.UploadManagedTexture(
		           mask, maskBlocks.data(), maskBlocks.size() - 1, &error ) &&
		           !context.UploadManagedTexture(
		               normal, normalBlocks.data(), normalBlocks.size() - 1, &error ) &&
		           !context.UploadManagedTexture(
		               hdr, hdrBlocks.data(), hdrBlocks.size() - 1, &error ),
		    "short BC4, BC5 and BC6H payloads are rejected" );
		check(
		    context.UploadManagedTexture( red, redBlocks.data(), redBlocks.size(), &error ) &&
		        context.UploadManagedTexture(
		            blue, blueBlocks.data(), blueBlocks.size(), &error ) &&
		        context.UploadManagedTexture(
		            mask, maskBlocks.data(), maskBlocks.size(), &error ) &&
		        context.UploadManagedTexture(
		            normal, normalBlocks.data(), normalBlocks.size(), &error ) &&
		        context.UploadManagedTexture( hdr, hdrBlocks.data(), hdrBlocks.size(), &error ) &&
		        context.UploadManagedTexture( hdrReference,
		            reinterpret_cast<const std::uint8_t *>( hdrReferencePixels.data() ),
		            hdrReferencePixels.size() * sizeof( std::uint16_t ), &error ),
		    "all selected BC fixtures upload" );
		std::array<std::uint8_t, 4> center = {}, corner = {};
		check( DrawAndCapture( context, red, center, corner, error ),
		    "red BC7 fixture renders and captures" );
		check(
		    center[0] >= 250 && center[1] <= 5 && center[2] <= 5, "red BC7 fixture samples red" );
		check( corner[0] <= 5 && corner[1] <= 5 && corner[2] <= 5, "frame corner remains black" );
		check( DrawAndCapture( context, blue, center, corner, error ),
		    "blue BC7 fixture renders and captures" );
		check(
		    center[0] <= 5 && center[1] <= 5 && center[2] >= 250, "blue BC7 fixture samples blue" );
		check( DrawAndCapture( context, mask, center, corner, error ),
		    "BC4 mask fixture renders and captures" );
		check( center[0] >= 250 && center[1] <= 5 && center[2] <= 5,
		    "BC4 mask samples the red channel" );
		check( DrawAndCapture( context, normal, center, corner, error ),
		    "BC5 normal fixture renders and captures" );
		check( center[0] <= 5 && center[1] >= 250 && center[2] <= 5,
		    "BC5 normal samples the green channel" );
		check( DrawAndCapture( context, hdr, center, corner, error ),
		    "BC6H HDR fixture renders and captures" );
		const std::array<std::uint8_t, 4> hdrCaptured = center;
		check( DrawAndCapture( context, hdrReference, center, corner, error ),
		    "float HDR reference renders and captures" );
		std::fprintf( stderr, "BC6H RGB=(%u,%u,%u), float reference=(%u,%u,%u)\n", hdrCaptured[0],
		    hdrCaptured[1], hdrCaptured[2], center[0], center[1], center[2] );
		check( std::abs( int( hdrCaptured[0] ) - int( center[0] ) ) <= 8 &&
		           std::abs( int( hdrCaptured[1] ) - int( center[1] ) ) <= 8 &&
		           std::abs( int( hdrCaptured[2] ) - int( center[2] ) ) <= 8,
		    "BC6H pixel matches the independent float upload" );
#ifdef RFC0008_KTX_READER
		if ( packagedHandle >= 0 )
		{
			check( DrawAndCapture( context, packagedHandle, center, corner, error ),
			    "reader-to-bridge BC7 image renders" );
			check( center[0] >= 250 && center[1] <= 5 && center[2] <= 5,
			    "reader-to-bridge BC7 pixels sample red" );
			context.DestroyManagedTexture( packagedHandle );
		}
		if ( packagedMraoHandle >= 0 )
		{
			check(
			    DrawAndCapture( context, packagedMraoHandle, center, corner, error, 1.0f, false ),
			    "linear BC7 MRAO image renders" );
			std::fprintf(
			    stderr, "Linear BC7 MRAO RGB=(%u,%u,%u)\n", center[0], center[1], center[2] );
			check( center[0] >= 245 && center[1] >= 110 && center[1] <= 145 && center[2] >= 245,
			    "linear BC7 keeps metal, roughness and AO channels separate" );
			context.DestroyManagedTexture( packagedMraoHandle );
		}
		if ( packagedMipsHandle >= 0 )
		{
			check( DrawAndCapture( context, packagedMipsHandle, center, corner, error ),
			    "reader-to-bridge four-mip image renders" );
			check( center[0] >= 250 && center[1] <= 5 && center[2] <= 5,
			    "reader-to-bridge RGBA8 pixels sample red" );
			for ( std::size_t level = 1; level <= 3; ++level )
			{
				const float scale = static_cast<float>( 8U << level );
				check( DrawAndCapture( context, packagedMipsHandle, center, corner, error, scale ),
				    "reader-to-bridge lower mip renders" );
				check( std::abs( int( center[0] ) - int( kMipColors[level - 1][0] ) ) <= 5 &&
				           std::abs( int( center[1] ) - int( kMipColors[level - 1][1] ) ) <= 5 &&
				           std::abs( int( center[2] ) - int( kMipColors[level - 1][2] ) ) <= 5,
				    "reader-to-bridge lower mip samples its authored pixels" );
			}
			if ( context.MaxAnisotropicLevel() > 1 )
			{
				context.SetManagedTextureSamplerState(
				    packagedMipsHandle, render_vulkan::CVulkanContext::kSamplerLinear |
				                            render_vulkan::CVulkanContext::kSamplerMipLinear |
				                            render_vulkan::CVulkanContext::kSamplerAnisotropic );
				context.SetAnisotropicLevel( context.MaxAnisotropicLevel() );
				check( DrawAndCapture( context, packagedMipsHandle, center, corner, error ),
				    "mipmapped KTX2 image samples with a device-supported anisotropic sampler" );
				context.SetAnisotropicLevel( 1 );
				check( DrawAndCapture( context, packagedMipsHandle, center, corner, error ),
				    "sampler level can change after a submitted frame" );
			}
			context.DestroyManagedTexture( packagedMipsHandle );
		}
#endif
	}
	context.Shutdown();
	SDL_DestroyWindow( window );
	SDL_Quit();
	return testing::ReportConformance( checks, failures );
}
