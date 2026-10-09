//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The render core's shader API's renderer (see core_renderer.h): on the 3DS
// on render.device.pica, elsewhere on any render.device.v2 device (RFC 0029:
// the browser's WebGPU adapter).
//
//=============================================================================//

#include "core_renderer.h"

#include "render/device/device.h"
#include "render/device/encoder.h"
#if defined( PLATFORM_3DS )
#include "render/device/pica/provider.h"
#include "render/device/pica_codes.h"
#include "render/device/pica_format.h"
#endif

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#if defined( PLATFORM_3DS )
// The launch shell's bottom-screen log (launcher_main/n3ds_main.cpp).
extern "C" void N3ds_StartDebugConsole();
#endif

namespace corefacade
{

namespace
{

using namespace render::device;
#if defined( PLATFORM_3DS )
namespace pc = render::device::pica;

// The target is a power of two (the presenter samples it); the frame draws
// its top-left 400x240.
std::uint32_t TargetWidth()
{
	return 512;
}
std::uint32_t TargetHeight()
{
	return 256;
}
#else
// The target is the screen.
std::uint32_t TargetWidth()
{
	return std::uint32_t( kScreenWidth );
}
std::uint32_t TargetHeight()
{
	return std::uint32_t( kScreenHeight );
}
#endif

// A mesh's memory (AllocLinear): in place, a mapped buffer of the render
// core's device that is the mesh's one copy (the core reads it where it is,
// CoreMeshStreams); else CPU memory the core never reads directly (the world
// meshes: the core draws the world from the map's own data).
struct Allocation
{
	BufferId buffer;
	// The device buffer's mapped bytes (the 3DS); elsewhere the CPU copy the
	// mesh writes, which FlushLinear copies into the device buffer.
	std::byte *resident = nullptr;
	std::size_t bytes = 0;
	Memory kind = Memory::kVertices;
	bool inPlace = false; // the device buffer is the mesh's one copy
	std::uint32_t usedInRecording = 0; // the recording that last read it
};

// A depth buffer for render targets of one size (D24S8), shared by them.
struct TargetDepth
{
	TextureId id;
	ResourceUsage usage = ResourceUsage::kUndefined;
};

struct State
{
	IRenderDevice2 *device = nullptr; // the render core's, borrowed
	TextureId color;
	TextureId depth;
	// Allocations by their first byte's address.
	std::map<std::uintptr_t, Allocation> allocations;
	std::optional<CommandEncoder> encoder;
	Texture *target = nullptr; // SetTarget's; null: the screen
	// A target that could not be drawn (no depth buffer): its draws, clears
	// and core sections are dropped until the next SetTarget.
	bool dropTargetDraws = false;
	std::map<std::uint64_t, TargetDepth> targetDepths; // by width << 32 | height
	std::uint32_t recording = 1; // increments at every submit
	bool rendering = false;
	ResourceUsage colorUsage = ResourceUsage::kUndefined;
	ResourceUsage depthUsage = ResourceUsage::kUndefined;
	Viewport viewport{ 0, 0, float( kScreenWidth ), float( kScreenHeight ), 0, 1 };
	std::vector<ResourceId> releases; // after the recording that reads them
	BufferId reserve; // ReserveLinear
	CompletionToken submitted; // the last SubmitRecording's
	bool lastSubmitTaken = false;
	// A mode change that arrived inside a frame, applied when it ends.
	int pendingWidth = 0;
	int pendingHeight = 0;
#if !defined( PLATFORM_3DS )
	Presenter presenter = nullptr;
	void *presenterContext = nullptr;
#endif
	bool initialized = false;
	bool inFrame = false;
	Stats stats;
	std::size_t textureBytes = 0;
	std::size_t meshBytes = 0;
	std::size_t residentBytes = 0; // in-place meshes (device memory)
};

State g_state;
IRenderDevice2 *g_bound = nullptr;

} // namespace

// Texture's target fields, for the renderer.
struct TargetAccess
{
	static TextureId Id( const Texture &texture ) { return TextureId{ texture.m_texture }; }
	static ResourceUsage Usage( const Texture &texture )
	{
		return ResourceUsage( texture.m_usage );
	}
	static void SetUsage( Texture &texture, ResourceUsage usage )
	{
		texture.m_usage = std::uint8_t( usage );
	}
};

namespace
{

// What the frame draws into now: the screen's 400x240, or SetTarget's.
struct Target
{
	TextureId color;
	TargetDepth *depth = nullptr; // a render target's; the screen keeps its own
	std::uint32_t width = 0;
	std::uint32_t height = 0;
};

IRenderDevice2 &Device();

Target Current()
{
	Target t;
	if ( !g_state.target )
	{
		t.color = g_state.color;
		t.width = kScreenWidth;
		t.height = kScreenHeight;
		return t;
	}
	const std::uint32_t w = std::uint32_t( g_state.target->Width() );
	const std::uint32_t h = std::uint32_t( g_state.target->Height() );
	TargetDepth &depth = g_state.targetDepths[( std::uint64_t( w ) << 32 ) | h];
	if ( !depth.id.IsValid() )
	{
		TextureDesc desc;
		desc.format = kDepthFormat;
		desc.width = w;
		desc.height = h;
		desc.usages = { ResourceUsage::kDepthWrite, ResourceUsage::kSampled };
		auto made = Device().CreateTexture( desc );
		if ( made )
			depth.id = made.Value();
	}
	t.color = TargetAccess::Id( *g_state.target );
	t.depth = &depth;
	t.width = w;
	t.height = h;
	return t;
}

IRenderDevice2 &Device()
{
	return *g_state.device;
}

// The allocation holding ptr, or null.
Allocation *Find( const void *ptr, std::size_t *offset )
{
	const std::uintptr_t at = reinterpret_cast<std::uintptr_t>( ptr );
	auto found = g_state.allocations.upper_bound( at );
	if ( found == g_state.allocations.begin() )
		return nullptr;
	--found;
	if ( at - found->first >= found->second.bytes )
		return nullptr;
	if ( offset )
		*offset = at - found->first;
	return &found->second;
}

ResourceUsage UsageOf( Memory kind )
{
	return kind == Memory::kVertices ? ResourceUsage::kVertex : ResourceUsage::kIndex;
}

// A mapped upload buffer, already in its drawing usage (elsewhere a device
// buffer and its CPU copy).
std::byte *CreateLinear( Memory kind, std::size_t bytes, BufferId &out )
{
	BufferDesc desc;
	desc.size = bytes;
#if defined( PLATFORM_3DS )
	desc.memory = MemoryKind::kUpload;
	desc.usages = { UsageOf( kind ) };
#else
	// WebGPU sizes copies in whole words.
	desc.size = ( bytes + 3 ) & ~std::size_t( 3 );
	desc.memory = MemoryKind::kDeviceLocal;
	desc.usages = { UsageOf( kind ), ResourceUsage::kCopyDestination };
#endif
	auto buffer = Device().CreateBuffer( desc );
	if ( !buffer )
		return nullptr;
	auto encoder = Device().BeginEncoder( QueueKind::kGraphics );
	if ( !encoder )
		return nullptr;
	encoder.Value().TransitionBuffer( buffer.Value(), ResourceUsage::kUndefined, UsageOf( kind ) );
	CommandEncoder list[] = { std::move( encoder ).Value() };
	(void)Device().Submit( QueueKind::kGraphics, list, {} );
#if defined( PLATFORM_3DS )
	const std::span<std::byte> mapped = pc::MapUploadBuffer( Device(), buffer.Value() );
	out = buffer.Value();
	return mapped.data();
#else
	auto *copy = static_cast<std::byte *>( std::calloc( 1, desc.size ) );
	if ( !copy )
	{
		(void)Device().Release( buffer.Value(), {} );
		return nullptr;
	}
	out = buffer.Value();
	return copy;
#endif
}

void BeginPass( bool clearColor, bool clearDepth, std::uint32_t rgba )
{
	CommandEncoder &e = *g_state.encoder;
	if ( g_state.rendering )
		e.EndRendering();
	g_state.rendering = false;
	Target target = Current();
	if ( g_state.target && ( !target.depth || !target.depth->id.IsValid() ) )
	{
		// No depth buffer for it (out of memory): the screen keeps the frame,
		// and what was meant for the target is dropped.
		g_state.target = nullptr;
		g_state.dropTargetDraws = true;
		target = Current();
		g_state.viewport = { 0, 0, float( target.width ), float( target.height ), 0, 1 };
	}
	if ( g_state.target )
	{
		// A render target's home usages (the screen's are the recording's).
		const ResourceUsage usage = TargetAccess::Usage( *g_state.target );
		if ( usage != ResourceUsage::kColorAttachment )
			e.TransitionTexture( target.color, usage, ResourceUsage::kColorAttachment );
		TargetAccess::SetUsage( *g_state.target, ResourceUsage::kColorAttachment );
		if ( target.depth->usage != ResourceUsage::kDepthWrite )
			e.TransitionTexture(
			    target.depth->id, target.depth->usage, ResourceUsage::kDepthWrite );
		target.depth->usage = ResourceUsage::kDepthWrite;
	}
	ColorAttachment color{
	    target.color, clearColor ? LoadOp::kClear : LoadOp::kLoad, StoreOp::kStore, {}, {} };
	color.clear = { float( ( rgba >> 0 ) & 0xFF ) / 255.0f, float( ( rgba >> 8 ) & 0xFF ) / 255.0f,
	    float( ( rgba >> 16 ) & 0xFF ) / 255.0f, float( ( rgba >> 24 ) & 0xFF ) / 255.0f };
	const ColorAttachment colors[] = { color };
	RenderingDesc rendering;
	rendering.colors = colors;
	rendering.depth = DepthAttachment{ target.depth ? target.depth->id : g_state.depth,
	    clearDepth ? LoadOp::kClear : LoadOp::kLoad, StoreOp::kStore, 1.0f };
	rendering.width = target.width;
	rendering.height = target.height;
	e.BeginRendering( rendering );
	e.SetViewport( g_state.viewport );
	g_state.rendering = true;
}

// Leaves the current render target sampleable (outside a pass).
void LeaveTarget( CommandEncoder &e )
{
	if ( !g_state.target )
		return;
	const ResourceUsage usage = TargetAccess::Usage( *g_state.target );
	if ( usage == ResourceUsage::kColorAttachment )
	{
		e.TransitionTexture( TargetAccess::Id( *g_state.target ), usage, ResourceUsage::kSampled );
		TargetAccess::SetUsage( *g_state.target, ResourceUsage::kSampled );
	}
}

// Opens the frame's recording (the target becomes an attachment).
void OpenRecording()
{
	if ( g_state.encoder )
		return;
	auto encoder = Device().BeginEncoder( QueueKind::kGraphics );
	if ( !encoder )
		return;
	g_state.encoder.emplace( std::move( encoder ).Value() );
	CommandEncoder &e = *g_state.encoder;
	if ( g_state.colorUsage != ResourceUsage::kColorAttachment )
		e.TransitionTexture( g_state.color, g_state.colorUsage, ResourceUsage::kColorAttachment );
	if ( g_state.depthUsage != ResourceUsage::kDepthWrite )
		e.TransitionTexture( g_state.depth, g_state.depthUsage, ResourceUsage::kDepthWrite );
	g_state.colorUsage = ResourceUsage::kColorAttachment;
	g_state.depthUsage = ResourceUsage::kDepthWrite;
}

// Submits what is recorded; the target stays an attachment unless sample.
void SubmitRecording( bool sample )
{
	if ( !g_state.encoder )
		return;
	CommandEncoder &e = *g_state.encoder;
	if ( g_state.rendering )
		e.EndRendering();
	g_state.rendering = false;
	if ( sample )
	{
		e.TransitionTexture(
		    g_state.color, ResourceUsage::kColorAttachment, ResourceUsage::kSampled );
		g_state.colorUsage = ResourceUsage::kSampled;
	}
	CommandEncoder list[] = { std::move( *g_state.encoder ) };
	g_state.encoder.reset();
	auto token = Device().Submit( QueueKind::kGraphics, list, {} );
	g_state.lastSubmitTaken = bool( token );
	if ( token )
		g_state.submitted = token.Value();
	else
	{
		++g_state.stats.submitFailures;
		static unsigned s_reported = 0;
		if ( s_reported++ < 4 )
			std::printf( "pica: the frame's submission was refused: %s (%s)\n",
				DescribeStatus( token.Error().status ), DescribeOperation( token.Error().operation ) );
	}
	++g_state.stats.submits;
	++g_state.recording;
	for ( ResourceId id : g_state.releases )
		(void)Device().Release( id, token ? token.Value() : CompletionToken{} );
	g_state.releases.clear();
	(void)Device().Poll();
	if ( !sample )
	{
		// Continue the frame: a new recording loads what the last one drew.
		OpenRecording();
		BeginPass( false, false, 0 );
	}
}

} // namespace

Texture::Texture()
    : m_texture( 0 ), m_wrap( 3 ), m_width( 0 ), m_height( 0 ), m_bytes( 0 )
{
}

Texture::~Texture()
{
	Release();
}

void Texture::Release()
{
	if ( this == g_state.target )
		SetTarget( nullptr );
	if ( m_texture && g_state.initialized )
	{
		// Drawn by the recording, perhaps: released after it is submitted.
		g_state.releases.push_back( TextureId{ m_texture } );
		g_state.textureBytes -= m_bytes;
	}
	m_texture = 0;
	m_bytes = 0;
	m_target = false;
	m_usage = 0;
}

bool Texture::CreateTarget( int width, int height )
{
	Release();
#if defined( PLATFORM_3DS )
	const auto side = []( int v )
	{
		return v >= 8 && v <= 512 && ( v & ( v - 1 ) ) == 0;
	};
#else
	const auto side = []( int v )
	{
		return v >= 1 && v <= 8192;
	};
#endif
	if ( !g_state.initialized || !side( width ) || !side( height ) )
		return false;
	TextureDesc desc;
	desc.format = Format::kRGBA8Unorm;
	desc.width = std::uint32_t( width );
	desc.height = std::uint32_t( height );
	desc.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kSampled,
	    ResourceUsage::kCopySource, ResourceUsage::kCopyDestination };
	auto texture = Device().CreateTexture( desc );
	if ( !texture )
		return false;
	m_texture = texture.Value().value;
	m_width = width;
	m_height = height;
	m_bytes = std::size_t( width ) * height * 4;
	m_target = true;
	m_usage = std::uint8_t( ResourceUsage::kUndefined );
	g_state.textureBytes += m_bytes;
	return true;
}

bool Texture::Sampleable() const
{
	if ( !m_texture )
		return false;
	if ( !m_target )
		return true;
	return this != g_state.target && ResourceUsage( m_usage ) == ResourceUsage::kSampled;
}

// A non-negative float in [0, 1] as an IEEE half (round to nearest).
static std::uint16_t FloatToHalf( float value )
{
	std::uint32_t bits;
	std::memcpy( &bits, &value, sizeof( bits ) );
	const std::uint32_t sign = ( bits >> 16 ) & 0x8000u;
	const int exponent = int( ( bits >> 23 ) & 0xFF ) - 127 + 15;
	std::uint32_t mantissa = bits & 0x7FFFFFu;
	if ( exponent <= 0 )
	{
		if ( exponent < -10 )
			return std::uint16_t( sign );
		mantissa |= 0x800000u;
		const int shift = 14 - exponent;
		return std::uint16_t( sign | ( ( mantissa + ( 1u << ( shift - 1 ) ) ) >> shift ) );
	}
	if ( exponent >= 31 )
		return std::uint16_t( sign | 0x7C00u );
	const std::uint32_t half = sign | ( std::uint32_t( exponent ) << 10 ) | ( mantissa >> 13 );
	return std::uint16_t( half + ( ( mantissa >> 12 ) & 1u ) );
}

bool Texture::Upload(
    UploadFormat format, int width, int height, int levelCount, const std::uint8_t *const *levels )
{
#if !defined( PLATFORM_3DS )
	// The same shape again (a lightmap page, a font page): refilled in place,
	// so the core's imports and cached groups keep a live id.
	const bool refill = m_texture && !m_target && m_width == width && m_height == height &&
	                    m_levels == levelCount && m_format == std::uint8_t( format );
	if ( !refill )
		Release();
#else
	const bool refill = false;
	Release();
#endif
	if ( levelCount < 1 || !g_state.initialized )
		return false;
	TextureDesc desc;
	desc.format = format == UploadFormat::kETC1     ? Format::kETC1Rgb
	              : format == UploadFormat::kETC1A4 ? Format::kETC1A4
	              : format == UploadFormat::kRGBA4  ? Format::kRGBA4Unorm
	              : format == UploadFormat::kRGBA8Srgb ? Format::kRGBA8Srgb
	              : format == UploadFormat::kRGBA16 || format == UploadFormat::kRGBA16F ? Format::kRGBA16Float
	                                                : Format::kRGBA8Unorm;
	desc.width = std::uint32_t( width );
	desc.height = std::uint32_t( height );
	desc.mipLevels = std::uint32_t( levelCount );
	desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	auto texture =
	    refill ? DeviceResult<TextureId>( TextureId{ m_texture } ) : Device().CreateTexture( desc );
	if ( !texture )
		return false;
	auto encoder = Device().BeginEncoder( QueueKind::kGraphics );
	if ( !encoder )
	{
		if ( !refill )
			(void)Device().Release( texture.Value(), {} );
		return false;
	}
	CommandEncoder &e = encoder.Value();
	e.TransitionTexture( texture.Value(),
	    refill ? ResourceUsage::kSampled : ResourceUsage::kUndefined,
	    ResourceUsage::kCopyDestination );
	std::vector<BufferId> staging;
	std::size_t total = 0;
	for ( int level = 0; level < levelCount; ++level )
	{
		const std::uint32_t w = std::uint32_t( width >> level ),
		                    h = std::uint32_t( height >> level );
		const std::size_t bytes =
		    format == UploadFormat::kRGBA16 || format == UploadFormat::kRGBA16F ? std::size_t( w ) * h * 8
		    : format == UploadFormat::kRGBA8 || format == UploadFormat::kRGBA8Srgb ? std::size_t( w ) * h * 4
		    : format == UploadFormat::kRGBA4 ? std::size_t( w ) * h * 2
		        : std::size_t( w / 4 ) * ( h / 4 ) * ( format == UploadFormat::kETC1 ? 8 : 16 );
		// 16-bit unorm texels become half floats of the same value.
		std::vector<std::uint16_t> halves;
		const std::byte *data = reinterpret_cast<const std::byte *>( levels[level] );
		if ( format == UploadFormat::kRGBA16 )
		{
			const auto *in = reinterpret_cast<const std::uint16_t *>( levels[level] );
			halves.resize( std::size_t( w ) * h * 4 );
			for ( std::size_t i = 0; i < halves.size(); ++i )
				halves[i] = FloatToHalf( float( in[i] ) / 65535.0f );
			data = reinterpret_cast<const std::byte *>( halves.data() );
		}
		auto buffer = Device().CreateUploadBuffer( { data, bytes } );
		if ( !buffer )
			break;
		staging.push_back( buffer.Value() );
		e.CopyBufferToTexture(
		    buffer.Value(), texture.Value(), { 0, std::uint32_t( level ), 0, w, h } );
		total += bytes;
	}
	e.TransitionTexture(
	    texture.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
	CommandEncoder list[] = { std::move( e ) };
	auto token = Device().Submit( QueueKind::kGraphics, list, {} );
	for ( BufferId buffer : staging )
		(void)Device().Release( buffer, token ? token.Value() : CompletionToken{} );
	if ( !token || staging.size() != std::size_t( levelCount ) )
	{
		if ( refill )
			Release();
		else
			(void)Device().Release( texture.Value(), {} );
		return false;
	}
	if ( refill )
		return true; // same id, same bytes
	m_texture = texture.Value().value;
	m_width = width;
	m_height = height;
	m_bytes = total;
	m_format = std::uint8_t( format );
	m_levels = levelCount;
	g_state.textureBytes += total;
	return true;
}

bool Texture::UploadCube( int size, const std::uint8_t *const *faces, bool srgb, bool half )
{
	Release();
	if ( size < 1 || !g_state.initialized )
		return false;
	TextureDesc desc;
	desc.dimension = TextureDimension::kCube;
	desc.format = half ? Format::kRGBA16Float : srgb ? Format::kRGBA8Srgb : Format::kRGBA8Unorm;
	desc.width = desc.height = std::uint32_t( size );
	desc.depthOrLayers = 6;
	desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	auto texture = Device().CreateTexture( desc );
	if ( !texture )
		return false;
	auto encoder = Device().BeginEncoder( QueueKind::kGraphics );
	if ( !encoder )
	{
		(void)Device().Release( texture.Value(), {} );
		return false;
	}
	CommandEncoder &e = encoder.Value();
	e.TransitionTexture(
	    texture.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	const std::size_t bytes = std::size_t( size ) * size * ( half ? 8 : 4 );
	std::vector<BufferId> staging;
	for ( std::uint32_t face = 0; face < 6; ++face )
	{
		auto buffer = Device().CreateUploadBuffer(
		    { reinterpret_cast<const std::byte *>( faces[face] ), bytes } );
		if ( !buffer )
			break;
		staging.push_back( buffer.Value() );
		e.CopyBufferToTexture( buffer.Value(), texture.Value(),
		    { 0, 0, face, std::uint32_t( size ), std::uint32_t( size ) } );
	}
	e.TransitionTexture(
	    texture.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
	CommandEncoder list[] = { std::move( e ) };
	auto token = Device().Submit( QueueKind::kGraphics, list, {} );
	for ( BufferId buffer : staging )
		(void)Device().Release( buffer, token ? token.Value() : CompletionToken{} );
	if ( !token || staging.size() != 6 )
	{
		(void)Device().Release( texture.Value(), {} );
		return false;
	}
	m_texture = texture.Value().value;
	m_width = m_height = size;
	m_bytes = bytes * 6;
	g_state.textureBytes += m_bytes;
	return true;
}

void Texture::SetWrap( bool wrapS, bool wrapT )
{
	const std::uint8_t wrap = ( wrapS ? 1 : 0 ) | ( wrapT ? 2 : 0 );
	if ( wrap == m_wrap )
		return;
	m_wrap = wrap;
}

void BindDevice( IRenderDevice2 *device )
{
	g_bound = device;
}

#if !defined( PLATFORM_3DS )
void SetScreenSize( int width, int height )
{
	if ( width > 0 && height > 0 && !g_state.initialized )
	{
		kScreenWidth = width;
		kScreenHeight = height;
	}
}

void ScreenSize( int &width, int &height )
{
	const bool pending = g_state.pendingWidth > 0 && g_state.pendingHeight > 0;
	width = pending ? g_state.pendingWidth : kScreenWidth;
	height = pending ? g_state.pendingHeight : kScreenHeight;
}

bool ResizeScreen( int width, int height )
{
	if ( width <= 0 || height <= 0 )
		return false;
	if ( !g_state.initialized )
	{
		SetScreenSize( width, height );
		return true;
	}
	if ( g_state.inFrame )
	{
		// Applied when the frame ends (EndFrame), not mid-frame.
		g_state.pendingWidth = width;
		g_state.pendingHeight = height;
		return true;
	}
	g_state.pendingWidth = g_state.pendingHeight = 0;
	if ( width == kScreenWidth && height == kScreenHeight )
		return true;
	TextureDesc desc;
	desc.format = Format::kRGBA8Unorm;
	desc.width = std::uint32_t( width );
	desc.height = std::uint32_t( height );
	desc.usages = {
	    ResourceUsage::kColorAttachment, ResourceUsage::kSampled, ResourceUsage::kCopySource };
	auto color = Device().CreateTexture( desc );
	desc.format = kDepthFormat;
	desc.usages = { ResourceUsage::kDepthWrite, ResourceUsage::kSampled };
	auto depth = Device().CreateTexture( desc );
	if ( !color || !depth )
	{
		if ( color )
			(void)Device().Release( color.Value(), {} );
		if ( depth )
			(void)Device().Release( depth.Value(), {} );
		return false;
	}
	// The old targets go behind the last submission that drew them.
	(void)Device().Release( g_state.color, g_state.submitted );
	(void)Device().Release( g_state.depth, g_state.submitted );
	g_state.color = color.Value();
	g_state.depth = depth.Value();
	g_state.colorUsage = State().colorUsage;
	kScreenWidth = width;
	kScreenHeight = height;
	return true;
}

void BindPresenter( Presenter presenter, void *context )
{
	g_state.presenter = presenter;
	g_state.presenterContext = context;
}
#endif

bool Init()
{
	if ( g_state.initialized )
		return true;
#if defined( PLATFORM_3DS )
	N3ds_StartDebugConsole();
#endif
	if ( !g_bound )
	{
		std::printf( "pica: no render core device was handed to the shader API\n" );
		return false;
	}
	g_state.device = g_bound;

	TextureDesc desc;
	desc.format = Format::kRGBA8Unorm;
	desc.width = TargetWidth();
	desc.height = TargetHeight();
	desc.usages = {
	    ResourceUsage::kColorAttachment, ResourceUsage::kSampled, ResourceUsage::kCopySource };
	auto color = Device().CreateTexture( desc );
	desc.format = kDepthFormat;
	desc.usages = { ResourceUsage::kDepthWrite, ResourceUsage::kSampled };
	auto depth = Device().CreateTexture( desc );
	if ( !color || !depth )
	{
		const DeviceError error = color ? depth.Error() : color.Error();
		std::printf( "pica: the frame's %s target (%ux%u) was refused: %s (%s)\n",
		    color ? "depth" : "colour", TargetWidth(), TargetHeight(),
		    DescribeStatus( error.status ), DescribeOperation( error.operation ) );
#if !defined( PLATFORM_3DS )
		const Presenter presenter = g_state.presenter;
		void *context = g_state.presenterContext;
		g_state = State();
		BindPresenter( presenter, context );
#else
		g_state = State();
#endif
		return false;
	}
	g_state.color = color.Value();
	g_state.depth = depth.Value();

	g_state.initialized = true;
	return true;
}

bool ReserveLinear( std::size_t bytes )
{
	if ( !g_state.initialized || bytes == 0 || g_state.reserve.IsValid() )
		return false;
	BufferDesc desc;
	desc.size = bytes;
	desc.memory = MemoryKind::kUpload;
	desc.usages = { ResourceUsage::kVertex };
	auto buffer = Device().CreateBuffer( desc );
	if ( !buffer )
		return false;
	g_state.reserve = buffer.Value();
	return true;
}

void Shutdown()
{
	if ( !g_state.initialized )
		return;
	if ( g_state.inFrame )
		EndFrame();
	// The device is the render core's and outlives this: everything made
	// here is released on it.
	IRenderDevice2 &device = Device();
	(void)device.WaitIdle();
	for ( ResourceId id : g_state.releases )
		(void)device.Release( id, {} );
	for ( const auto &[address, allocation] : g_state.allocations )
	{
		(void)device.Release( allocation.buffer, {} );
#if !defined( PLATFORM_3DS )
		if ( allocation.inPlace )
			std::free( allocation.resident );
#endif
	}
	if ( g_state.reserve.IsValid() )
		(void)device.Release( g_state.reserve, {} );
	for ( const auto &[size, depth] : g_state.targetDepths )
		if ( depth.id.IsValid() )
			(void)device.Release( depth.id, {} );
	for ( ResourceId id : { ResourceId( g_state.color ), ResourceId( g_state.depth ) } )
		if ( id.value )
			(void)device.Release( id, {} );
	(void)device.Poll();
#if !defined( PLATFORM_3DS )
	const Presenter presenter = g_state.presenter;
	void *context = g_state.presenterContext;
	g_state = State();
	BindPresenter( presenter, context );
#else
	g_state = State();
#endif
}

bool Initialized()
{
	return g_state.initialized;
}

bool InFrame()
{
	return g_state.inFrame;
}

void BeginFrame()
{
	if ( !g_state.initialized || g_state.inFrame )
		return;
	g_state.inFrame = true;
	g_state.stats = Stats();
	const Target target = Current();
	g_state.viewport = { 0, 0, float( target.width ), float( target.height ), 0, 1 };
	OpenRecording();
	BeginPass( false, false, 0 );
}

CompletionToken LastSubmission( bool *submitted )
{
	if ( submitted )
		*submitted = g_state.lastSubmitTaken;
	return g_state.submitted;
}

void EndFrame()
{
	if ( !g_state.inFrame )
		return;
	g_state.stats.textureBytes = g_state.textureBytes;
	g_state.stats.meshBytes = g_state.meshBytes;
	if ( g_state.target && g_state.encoder )
	{
		if ( g_state.rendering )
			g_state.encoder->EndRendering();
		g_state.rendering = false;
		LeaveTarget( *g_state.encoder );
	}
	g_state.target = nullptr;
	g_state.dropTargetDraws = false;
	SubmitRecording( true );
#if defined( PLATFORM_3DS )
	(void)pc::PresentTopScreen( Device(), g_state.color, kScreenWidth, kScreenHeight );
#else
	if ( g_state.presenter )
		(void)g_state.presenter( g_state.presenterContext, Device(), g_state.color.value,
		    std::uint32_t( kScreenWidth ), std::uint32_t( kScreenHeight ) );
#endif
	g_state.inFrame = false;
#if !defined( PLATFORM_3DS )
	if ( g_state.pendingWidth > 0 && g_state.pendingHeight > 0 &&
	     !ResizeScreen( g_state.pendingWidth, g_state.pendingHeight ) )
		std::printf( "pica: the screen's targets were not resized to %dx%d\n", g_state.pendingWidth,
		    g_state.pendingHeight );
	g_state.pendingWidth = g_state.pendingHeight = 0;
#endif
}

void SetTarget( Texture *target )
{
	if ( !g_state.initialized || target == g_state.target )
		return;
	if ( target && ( !target->Valid() || !target->IsTarget() ) )
		return;
	if ( !g_state.inFrame || !g_state.encoder )
	{
		g_state.target = target;
		g_state.dropTargetDraws = false;
		return;
	}
	CommandEncoder &e = *g_state.encoder;
	if ( g_state.rendering )
		e.EndRendering();
	g_state.rendering = false;
	LeaveTarget( e );
	g_state.target = target;
	g_state.dropTargetDraws = false;
	++g_state.stats.targetSwitches;
	const Target next = Current();
	g_state.viewport = { 0, 0, float( next.width ), float( next.height ), 0, 1 };
	BeginPass( false, false, 0 );
}

void Clear( bool color, bool depth, std::uint32_t rgba )
{
	if ( !g_state.inFrame )
		BeginFrame();
	if ( ( !color && !depth ) || g_state.dropTargetDraws )
		return;
	// A new pass that clears; the viewport's region is the whole target, as
	// the legacy renderer's clear was.
	BeginPass( color, depth, rgba );
}

void SetViewport( int x, int y, int width, int height )
{
	g_state.viewport = { float( x ), float( y ), float( width ), float( height ), 0, 1 };
	if ( g_state.rendering )
		g_state.encoder->SetViewport( g_state.viewport );
}

void *AllocLinear( Memory kind, std::size_t bytes, bool inPlace )
{
	if ( !g_state.initialized || bytes == 0 )
		return nullptr;
	Allocation allocation;
	allocation.kind = kind;
	allocation.bytes = bytes;
	if ( inPlace )
	{
		allocation.resident = CreateLinear( kind, bytes, allocation.buffer );
		if ( allocation.resident )
		{
			allocation.inPlace = true;
			g_state.allocations.emplace(
			    reinterpret_cast<std::uintptr_t>( allocation.resident ), allocation );
			g_state.meshBytes += bytes;
			g_state.residentBytes += bytes;
			return allocation.resident;
		}
	}
	auto *data = static_cast<std::byte *>( std::malloc( bytes ) );
	if ( !data )
		return nullptr;
	g_state.allocations.emplace( reinterpret_cast<std::uintptr_t>( data ), allocation );
	g_state.meshBytes += bytes;
	return data;
}

void FreeLinear( void *ptr )
{
	if ( !ptr )
		return;
	const auto found = g_state.allocations.find( reinterpret_cast<std::uintptr_t>( ptr ) );
	if ( found == g_state.allocations.end() )
		return;
	if ( found->second.inPlace )
	{
		g_state.releases.push_back( found->second.buffer );
		g_state.residentBytes -= found->second.bytes;
	}
	g_state.meshBytes -= found->second.bytes;
#if defined( PLATFORM_3DS )
	if ( !found->second.inPlace )
		std::free( ptr );
#else
	std::free( ptr ); // in place, the CPU copy
#endif
	g_state.allocations.erase( found );
	if ( !g_state.encoder )
	{
		for ( ResourceId id : g_state.releases )
			(void)Device().Release( id, {} );
		g_state.releases.clear();
	}
}

void PrepareWrite( const void *ptr )
{
	// In-place memory a recorded draw reads is submitted first; CPU memory
	// no draw reads.
	std::size_t offset = 0;
	const Allocation *allocation = ptr ? Find( ptr, &offset ) : nullptr;
	if ( allocation && allocation->inPlace && g_state.encoder &&
	     allocation->usedInRecording == g_state.recording )
		SubmitRecording( false );
}

bool DeviceBufferOf( const void *ptr, BufferId &buffer, std::uint64_t &offset )
{
	std::size_t at = 0;
	Allocation *allocation = ptr ? Find( ptr, &at ) : nullptr;
	if ( !allocation || !allocation->inPlace )
		return false;
	allocation->usedInRecording = g_state.recording;
	buffer = allocation->buffer;
	offset = at;
	return true;
}

void FlushLinear( const void *ptr, std::size_t bytes )
{
	std::size_t offset = 0;
	Allocation *allocation = Find( ptr, &offset );
	if ( !allocation )
		return;
#if defined( PLATFORM_3DS )
	if ( allocation->inPlace )
		pc::FlushUploadBuffer( Device(), allocation->buffer, offset, bytes );
#else
	if ( !allocation->inPlace || bytes == 0 )
		return;
	// Whole words, inside the buffer; in its own submission, ahead of the
	// recording that draws with it (PrepareWrite submitted any recording that
	// read the old bytes).
	const std::size_t begin = offset & ~std::size_t( 3 );
	const std::size_t end =
	    std::min( ( offset + bytes + 3 ) & ~std::size_t( 3 ), ( allocation->bytes + 3 ) & ~std::size_t( 3 ) );
	auto encoder = Device().BeginEncoder( QueueKind::kGraphics );
	if ( !encoder )
		return;
	const ResourceUsage usage = UsageOf( allocation->kind );
	encoder.Value().TransitionBuffer( allocation->buffer, usage, ResourceUsage::kCopyDestination );
	encoder.Value().WriteBuffer( allocation->buffer, begin,
	    std::span<const std::byte>( allocation->resident + begin, end - begin ) );
	encoder.Value().TransitionBuffer( allocation->buffer, ResourceUsage::kCopyDestination, usage );
	CommandEncoder list[] = { std::move( encoder ).Value() };
	(void)Device().Submit( QueueKind::kGraphics, list, {} );
#endif
}

render::device::CommandEncoder *BeginCoreSection( CoreSectionTarget &target )
{
	if ( !g_state.inFrame )
		BeginFrame();
	if ( !g_state.encoder || g_state.dropTargetDraws )
		return nullptr;
	if ( g_state.rendering )
		g_state.encoder->EndRendering();
	g_state.rendering = false;
	const Target current = Current();
	target.device = g_state.device;
	target.color = current.color.value;
	target.depth = current.depth ? current.depth->id.value : g_state.depth.value;
	target.width = current.width;
	target.height = current.height;
	target.serial = g_state.recording;
	target.submittedEpoch = g_state.submitted.epoch;
	target.submittedValue = g_state.submitted.value;
	return &*g_state.encoder;
}

void FlushRecording()
{
	if ( g_state.encoder && g_state.inFrame )
		SubmitRecording( false );
}

void EndCoreSection()
{
	if ( g_state.encoder && !g_state.rendering )
		BeginPass( false, false, 0 );
}

bool ReadCurrentTarget( int x, int y, int width, int height, std::uint8_t *rgba )
{
	if ( !g_state.initialized || width <= 0 || height <= 0 || !rgba )
		return false;
	const Target current = Current();
	if ( x < 0 || y < 0 || std::uint32_t( x + width ) > current.width ||
	     std::uint32_t( y + height ) > current.height )
		return false;
	BufferDesc out;
	out.size = std::uint64_t( width ) * height * 4;
	out.usages = { ResourceUsage::kCopyDestination };
	out.memory = MemoryKind::kReadback;
	auto readback = Device().CreateBuffer( out );
	if ( !readback )
		return false;
	TextureBufferCopy region;
	region.width = std::uint32_t( width );
	region.height = std::uint32_t( height );
	region.x = std::uint32_t( x );
	region.y = std::uint32_t( y );
	CompletionToken token{};
	bool submitted = false;
	if ( g_state.inFrame && g_state.encoder )
	{
		// In the frame: the target is a colour attachment of the open recording.
		CommandEncoder &e = *g_state.encoder;
		if ( g_state.rendering )
			e.EndRendering();
		g_state.rendering = false;
		e.TransitionTexture( current.color, ResourceUsage::kColorAttachment, ResourceUsage::kCopySource );
		e.TransitionBuffer( readback.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		e.CopyTextureToBuffer( current.color, readback.Value(), region );
		e.TransitionTexture( current.color, ResourceUsage::kCopySource, ResourceUsage::kColorAttachment );
		SubmitRecording( false );
		token = g_state.submitted;
		submitted = g_state.lastSubmitTaken;
	}
	else if ( !g_state.target && g_state.colorUsage == ResourceUsage::kSampled )
	{
		auto encoder = Device().BeginEncoder( QueueKind::kGraphics );
		if ( encoder )
		{
			CommandEncoder &e = encoder.Value();
			e.TransitionTexture( g_state.color, ResourceUsage::kSampled, ResourceUsage::kCopySource );
			e.TransitionBuffer( readback.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
			e.CopyTextureToBuffer( g_state.color, readback.Value(), region );
			e.TransitionTexture( g_state.color, ResourceUsage::kCopySource, ResourceUsage::kSampled );
			CommandEncoder list[] = { std::move( e ) };
			auto result = Device().Submit( QueueKind::kGraphics, list, {} );
			submitted = bool( result );
			if ( result )
				token = result.Value();
		}
	}
	bool read = false;
	if ( submitted && Device().WaitIdle() )
	{
		std::vector<std::byte> pixels( std::size_t( out.size ) );
		read = bool( Device().ReadBuffer( readback.Value(), 0, pixels ) );
		if ( read )
			std::memcpy( rgba, pixels.data(), pixels.size() );
	}
	(void)Device().Release( readback.Value(), token );
	return read;
}

bool CaptureTopScreen( const char *path )
{
	if ( !g_state.initialized || g_state.inFrame || g_state.colorUsage != ResourceUsage::kSampled )
		return false;
	BufferDesc out;
	out.size = std::uint64_t( kScreenWidth ) * kScreenHeight * 4;
	out.usages = { ResourceUsage::kCopyDestination };
	out.memory = MemoryKind::kReadback;
	auto readback = Device().CreateBuffer( out );
	auto encoder = Device().BeginEncoder( QueueKind::kGraphics );
	if ( !readback || !encoder )
		return false;
	CommandEncoder &e = encoder.Value();
	e.TransitionTexture( g_state.color, ResourceUsage::kSampled, ResourceUsage::kCopySource );
	e.TransitionBuffer(
	    readback.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.CopyTextureToBuffer( g_state.color, readback.Value(),
	    { 0, 0, 0, std::uint32_t( kScreenWidth ), std::uint32_t( kScreenHeight ) } );
	e.TransitionTexture( g_state.color, ResourceUsage::kCopySource, ResourceUsage::kSampled );
	CommandEncoder list[] = { std::move( e ) };
	auto token = Device().Submit( QueueKind::kGraphics, list, {} );
	(void)Device().WaitIdle();
	std::vector<std::byte> pixels( std::size_t( out.size ) );
	const bool read = token && Device().ReadBuffer( readback.Value(), 0, pixels );
	(void)Device().Release( readback.Value(), token ? token.Value() : CompletionToken{} );
	FILE *file = read ? std::fopen( path, "wb" ) : nullptr;
	if ( !file )
		return false;
	std::fprintf( file, "P6\n%d %d\n255\n", kScreenWidth, kScreenHeight );
	for ( int i = 0; i < kScreenWidth * kScreenHeight; ++i )
		std::fwrite( &pixels[std::size_t( i ) * 4], 1, 3, file );
	std::fclose( file );
	return true;
}

const Stats &FrameStats()
{
	return g_state.stats;
}

} // namespace corefacade
