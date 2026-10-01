//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's shared support (RFC 0016 K11); see lab_support.h.
//
//=============================================================================//

#include "lab_support.h"

#include "render/device/vulkan/provider.h"
#include "render/shaderlib/debug_view.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <span>
#include <sstream>

namespace render::lab
{

using namespace render::device;
namespace fs = std::filesystem;

std::unique_ptr<LabClusterLists> LabClusterLists::Create( IRenderDevice2 &device,
    const pass::lights::ClusterGrid &grid, std::span<const light_set::RuntimeLight> lights,
    std::span<const area_light::AreaLight> areas )
{
	auto out = std::unique_ptr<LabClusterLists>( new LabClusterLists( device ) );
	auto kernel = pass::lights::ClusterKernel::Create( device );
	if ( !kernel )
		return {};
	out->m_Kernel = std::move( kernel ).Value();
	auto encoded = device.BeginEncoder( QueueKind::kGraphics );
	if ( !encoded )
		return {};
	auto buffers = out->m_Kernel->RecordView(
	    encoded.Value(), pass::lights::PrepareSurfaceClusterDispatch( grid, lights, areas ) );
	if ( !buffers )
		return {};
	out->m_Buffers = buffers.Value();
	auto token = device.Submit( QueueKind::kGraphics, { &encoded.Value(), 1 }, {} );
	if ( !token )
		return {};
	return out;
}
LabClusterLists::~LabClusterLists()
{
	(void)m_Device.WaitIdle();
	if ( m_Kernel )
		m_Kernel->Collect( {} );
}
void LabClusterLists::Bind( material::GroupRequest &request ) const
{
	for ( auto &buffer : request.storage )
	{
		if ( buffer.binding == 1 )
		{
			buffer.external = m_Buffers.froxels;
			buffer.bytes.clear();
		}
		if ( buffer.binding == 2 )
		{
			buffer.external = m_Buffers.indices;
			buffer.bytes.clear();
		}
	}
}

std::optional<std::string> ReadFile( const fs::path &path )
{
	std::ifstream file( path, std::ios::binary );
	if ( !file )
		return std::nullopt;
	std::ostringstream bytes;
	bytes << file.rdbuf();
	return bytes.str();
}

std::optional<fs::path> GameFiles::Resolve( const std::string &relative ) const
{
	fs::path direct = m_Root / relative;
	if ( fs::exists( direct ) )
		return direct;
	std::string lowered = relative;
	std::transform( lowered.begin(), lowered.end(), lowered.begin(),
	    []( unsigned char c )
	    {
		    return char( std::tolower( c ) );
	    } );
	fs::path lower = m_Root / lowered;
	if ( fs::exists( lower ) )
		return lower;
	return std::nullopt;
}

bool GameFiles::Exists( const std::string &path ) const
{
	return Resolve( path ).has_value();
}

bool GameFiles::Read( const std::string &path, std::string &out ) const
{
	const std::optional<fs::path> found = Resolve( path );
	if ( !found )
		return false;
	std::optional<std::string> bytes = ReadFile( *found );
	if ( !bytes )
		return false;
	out = std::move( *bytes );
	return true;
}

std::optional<Format> PortFormat( texturecontainer::PixelFormat format, bool srgb )
{
	using texturecontainer::PixelFormat;
	switch ( format )
	{
	case PixelFormat::Rgba8Unorm:
	case PixelFormat::Rgba8Srgb:
		return srgb ? Format::kRGBA8Srgb : Format::kRGBA8Unorm;
	case PixelFormat::Bgra8Unorm:
	case PixelFormat::Bgra8Srgb:
		return srgb ? Format::kBGRA8Srgb : Format::kBGRA8Unorm;
	case PixelFormat::Rgba16Float:
		return Format::kRGBA16Float;
	case PixelFormat::Bc1Unorm:
	case PixelFormat::Bc1Srgb:
		return srgb ? Format::kBC1Srgb : Format::kBC1Unorm;
	case PixelFormat::Bc2Unorm:
	case PixelFormat::Bc2Srgb:
		return srgb ? Format::kBC2Srgb : Format::kBC2Unorm;
	case PixelFormat::Bc3Unorm:
	case PixelFormat::Bc3Srgb:
		return srgb ? Format::kBC3Srgb : Format::kBC3Unorm;
	case PixelFormat::Bc4Unorm:
		return Format::kBC4Unorm;
	case PixelFormat::Bc5Unorm:
		return Format::kBC5Unorm;
	default:
		return std::nullopt;
	}
}

// Stages a decoded image with its mips; the name is the importer's texture
// reference ("materials/..."), which the programs' groups look up.
std::optional<std::string> StageImage( resources::TextureCache &cache, const std::string &name,
    const texturecontainer::TextureImage &image, bool srgb )
{
	const std::optional<Format> format = PortFormat( image.format, srgb );
	if ( !format || image.levels.empty() )
		return "texture " + name + " has a format the lab does not stage";
	TextureDesc desc;
	desc.format = *format;
	desc.width = image.levels[0].width;
	desc.height = image.levels[0].height;
	desc.mipLevels = std::uint32_t( image.levels.size() );
	desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	std::vector<std::span<const std::byte>> levels;
	for ( const texturecontainer::ImageLevel &level : image.levels )
		levels.emplace_back( level.bytes );
	if ( !cache.StageMips( name, desc, levels ) )
		return "texture " + name + " was refused by the texture cache";
	return std::nullopt;
}

bool WritePfm( const fs::path &path, std::uint32_t width, std::uint32_t height,
    const std::vector<float> &rgba )
{
	std::ofstream file( path, std::ios::binary );
	if ( !file )
		return false;
	file << "PF\n" << width << " " << height << "\n-1.0\n";
	// PFM rows run bottom to top; the target's rows run top to bottom.
	for ( std::uint32_t row = 0; row < height; ++row )
	{
		const std::uint32_t y = height - 1 - row;
		for ( std::uint32_t x = 0; x < width; ++x )
		{
			const float *pixel = &rgba[( std::size_t( y ) * width + x ) * 4];
			file.write( reinterpret_cast<const char *>( pixel ), 3 * sizeof( float ) );
		}
	}
	return bool( file );
}

float HalfToFloat( std::uint16_t half )
{
	const std::uint32_t sign = std::uint32_t( half & 0x8000u ) << 16;
	const std::uint32_t exponent = ( half >> 10 ) & 0x1fu;
	const std::uint32_t mantissa = half & 0x3ffu;
	std::uint32_t bits = 0;
	if ( exponent == 0 )
	{
		if ( mantissa != 0 )
		{
			float value = std::ldexp( float( mantissa ), -24 );
			return sign ? -value : value;
		}
		bits = sign;
	}
	else if ( exponent == 31 )
		bits = sign | 0x7f800000u | ( mantissa << 13 );
	else
		bits = sign | ( ( exponent + 112 ) << 23 ) | ( mantissa << 13 );
	float value;
	std::memcpy( &value, &bits, sizeof( value ) );
	return value;
}

std::uint16_t FloatToHalf( float value )
{
	std::uint32_t bits;
	std::memcpy( &bits, &value, sizeof( bits ) );
	const std::uint32_t sign = ( bits >> 16 ) & 0x8000u;
	const std::uint32_t exponent = ( bits >> 23 ) & 0xffu;
	std::uint32_t mantissa = bits & 0x7fffffu;
	if ( exponent == 0xff )
		return std::uint16_t( sign | 0x7c00u | ( mantissa ? 0x200u : 0u ) );
	const int unbiased = int( exponent ) - 127 + 15;
	if ( unbiased >= 31 )
		return std::uint16_t( sign | 0x7c00u );
	if ( unbiased <= 0 )
	{
		if ( unbiased < -10 )
			return std::uint16_t( sign );
		mantissa |= 0x800000u;
		const int shift = 14 - unbiased;
		std::uint32_t half = mantissa >> shift;
		const std::uint32_t rest = mantissa & ( ( 1u << shift ) - 1u );
		const std::uint32_t halfway = 1u << ( shift - 1 );
		if ( rest > halfway || ( rest == halfway && ( half & 1u ) ) )
			++half;
		return std::uint16_t( sign | half );
	}
	std::uint32_t half = ( std::uint32_t( unbiased ) << 10 ) | ( mantissa >> 13 );
	const std::uint32_t rest = mantissa & 0x1fffu;
	if ( rest > 0x1000u || ( rest == 0x1000u && ( half & 1u ) ) )
		++half; // a carry into the exponent is correct rounding
	return std::uint16_t( sign | half );
}

mapcontainer::WorldLightmapLayer BakedLightmapLayer( bool directOwnedByCore )
{
	return directOwnedByCore ? mapcontainer::WorldLightmapLayer::Indirect
	                         : mapcontainer::WorldLightmapLayer::Total;
}

std::optional<std::string> StageProbeVolume( resources::TextureCache &cache,
    const std::string &name, std::span<const std::byte> lump,
    mapcontainer::ProbeVolumeLayout &layout )
{
	if ( const mapcontainer::ProbeVolumeError error =
	         mapcontainer::ValidateProbeVolume( lump.data(), lump.size(), &layout );
	    error != mapcontainer::ProbeVolumeError::Ok )
		return std::string( "PRBV: " ) + mapcontainer::ProbeVolumeErrorName( error );
	TextureDesc atlas;
	atlas.format = Format::kRGBA16Float;
	atlas.width = layout.atlasWidth;
	atlas.height = layout.atlasHeight;
	atlas.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	std::vector<float> table(
	    std::size_t( layout.gridCount ) * mapcontainer::kProbeGridTableFloats );
	mapcontainer::WriteProbeGridTable( layout, table.data() );
	TextureDesc grids;
	grids.format = Format::kRGBA32Float;
	grids.width = mapcontainer::kProbeGridTableTexels;
	grids.height = layout.gridCount;
	grids.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	if ( !cache.Stage( name + "-atlas", atlas,
	         lump.subspan(
	             std::size_t( layout.atlasOffset ), std::size_t( layout.atlasBytes ) ) ) ||
	     !cache.Stage( name + "-grids", grids, std::as_bytes( std::span( table ) ) ) )
		return std::string( "PRBV: its textures were refused" );
	return std::nullopt;
}

std::optional<std::string> StageReflectionProbes( resources::TextureCache &cache,
    const std::string &name, std::span<const std::byte> lump,
    mapcontainer::ReflectionProbeMode mode, bool relight,
    mapcontainer::ReflectionProbesLayout &layout )
{
	if ( const mapcontainer::ReflectionProbesError error =
	         mapcontainer::ValidateReflectionProbes( lump.data(), lump.size(), &layout );
	    error != mapcontainer::ReflectionProbesError::Ok )
		return std::string( "RPRB: " ) + mapcontainer::ReflectionProbesErrorName( error );
	TextureDesc desc;
	desc.format = Format::kRGBA16Float;
	desc.width = mapcontainer::ReflectionProbeTextureWidth( layout );
	desc.height = mapcontainer::ReflectionProbeTextureRows( layout );
	desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	std::vector<std::uint16_t> texels( std::size_t( desc.width ) * desc.height * 4 );
	mapcontainer::WriteReflectionProbeTexture( lump.data(), layout, mode, texels.data(), relight );
	if ( !cache.Stage( name, desc, std::as_bytes( std::span( texels ) ) ) )
		return std::string( "RPRB: its texture was refused" );
	return std::nullopt;
}

std::optional<std::string> CreateLabDevice(
    bool validate, std::atomic<std::uint64_t> &messages, std::unique_ptr<IRenderDevice2> &out )
{
	// Asked and not installed is a failure: zero messages from no layer
	// proves nothing.
	if ( validate && !vulkan::ValidationLayerAvailable() )
		return std::string( "validation: the Khronos validation layer is not installed" );
	vulkan::VulkanAdapterOptions adapter;
	adapter.validation = validate;
	adapter.validationCounter = &messages;
	if ( const char *index = std::getenv( "RENDER_VK_ADAPTER" ) )
		adapter.adapterIndex = std::atoi( index );
	auto created = vulkan::Create( adapter );
	if ( !created )
		return std::string( "no Vulkan device" );
	out = std::move( created ).Value();
	return std::nullopt;
}

bool ParseDebugOption( const std::string &option, const char *value, frame::DebugControls &debug )
{
	if ( !value )
		return false;
	char *end = nullptr;
	const auto whole = [&]( const char *text )
	{
		return end != text && *end == '\0';
	};
	if ( option == "--debug-view" || option == "--debug-brdf" || option == "--debug-legacy" )
	{
		const unsigned long number = std::strtoul( value, &end, 10 );
		if ( !whole( value ) )
			return false;
		if ( option == "--debug-view" )
			debug.view = std::uint32_t( number );
		else if ( option == "--debug-brdf" )
			debug.brdf = std::uint32_t( number );
		else
			debug.legacy = frame::DebugLegacy( std::uint32_t( number ) );
		return true;
	}
	if ( option == "--debug-scale" || option == "--debug-range" || option == "--debug-threshold" ||
	     option == "--debug-force-roughness" || option == "--debug-force-metalness" )
	{
		const float number = std::strtof( value, &end );
		if ( !whole( value ) )
			return false;
		if ( option == "--debug-scale" )
			debug.viewScale = number;
		else if ( option == "--debug-range" )
			debug.viewRange = number;
		else if ( option == "--debug-threshold" )
			debug.viewThreshold = number;
		else if ( option == "--debug-force-roughness" )
			debug.forceRoughness = number;
		else
			debug.forceMetalness = number;
		return true;
	}
	if ( option == "--debug-program" )
	{
		const std::size_t length = std::strlen( value );
		if ( length >= sizeof( debug.program ) )
			return false;
		std::memcpy( debug.program, value, length + 1 );
		return true;
	}
	if ( option == "--debug-term" )
	{
		std::string names = value;
		std::size_t start = 0;
		while ( start <= names.size() )
		{
			const std::size_t comma = std::min( names.find( ',', start ), names.size() );
			const std::uint32_t bit =
			    shaderlib::DebugTermBit( std::string_view( names ).substr( start, comma - start ) );
			if ( bit == 0 )
				return false;
			debug.termsOff |= bit;
			start = comma + 1;
		}
		return true;
	}
	return false;
}

bool StageConstant( resources::TextureCache &cache, const std::string &name, device::Format format,
    std::span<const std::byte> texel, bool cube )
{
	device::TextureDesc desc;
	desc.format = format;
	desc.width = desc.height = 4;
	if ( cube )
	{
		desc.dimension = device::TextureDimension::kCube;
		desc.depthOrLayers = 6;
	}
	std::vector<std::byte> pixels;
	for ( int i = 0; i < 16 * ( cube ? 6 : 1 ); ++i )
		pixels.insert( pixels.end(), texel.begin(), texel.end() );
	return cache.Stage( name, desc, pixels ).HasValue();
}

std::vector<std::byte> ByteTexel( int r, int g, int b, int a )
{
	return { std::byte( r ), std::byte( g ), std::byte( b ), std::byte( a ) };
}

} // namespace render::lab
