//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The 3DS shader API's renderer on render.device.pica (see pica_renderer.h).
//
//=============================================================================//

#include "pica_renderer.h"

#include "render/device/device.h"
#include "render/device/encoder.h"
#include "render/device/pica/provider.h"
#include "render/device/pica_codes.h"
#include "render/device/pica_format.h"

#include <cstdio>
#include <cstring>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

// The vertex program, assembled from fullbright.v.pica by the build.
extern "C" const unsigned char fullbright_shbin[];
extern "C" const unsigned fullbright_shbin_size;

// The launch shell's bottom-screen log (launcher_main/n3ds_main.cpp).
extern "C" void N3ds_StartDebugConsole();

namespace pica
{

namespace
{

using namespace render::device;
namespace pf = render::device::pica_format;
namespace pc = render::device::pica;

// The target is a power of two (the presenter samples it); the frame draws
// its top-left 400x240.
constexpr std::uint32_t kTargetWidth = 512;
constexpr std::uint32_t kTargetHeight = 256;
constexpr std::size_t kVertexRingBytes = 3u * 1024u * 1024u;
constexpr std::size_t kIndexRingBytes = 512u * 1024u;
// c0-c3 the column-vector clip rows, c4 the tint (fullbright.v.pica).
constexpr std::uint32_t kDrawConstantBytes = 5 * 16;

// One linear allocation: a device upload buffer, mapped.
struct Allocation
{
	BufferId buffer;
	std::size_t bytes = 0;
	Memory kind = Memory::kVertices;
	bool transient = false;
	std::uint32_t usedInRecording = 0; // the recording that last read it
};

struct Ring
{
	BufferId buffer;
	std::byte *data = nullptr;
	std::size_t bytes = 0;
	std::size_t used = 0;
};

struct State
{
	IRenderDevice2 *device = nullptr; // the render core's, borrowed
	TextureId color;
	TextureId depth;
	BindGroupLayoutId materialLayout;
	SamplerId samplers[2]; // clamped, repeating
	std::vector<std::byte> vertexArtifact;
	std::unordered_map<std::uint64_t, PipelineId> pipelines;
	// Allocations by their first byte's address.
	std::map<std::uintptr_t, Allocation> allocations;
	Ring rings[2]; // Memory::kVertices, kIndices
	std::optional<CommandEncoder> encoder;
	std::uint32_t recording = 1; // increments at every submit
	bool rendering = false;
	ResourceUsage colorUsage = ResourceUsage::kUndefined;
	ResourceUsage depthUsage = ResourceUsage::kUndefined;
	Viewport viewport{ 0, 0, float( kScreenWidth ), float( kScreenHeight ), 0, 1 };
	std::vector<ResourceId> releases; // after the recording that reads them
	bool initialized = false;
	bool inFrame = false;
	Stats stats;
	std::size_t textureBytes = 0;
};

State g_state;
IRenderDevice2 *g_bound = nullptr;

IRenderDevice2 &Device()
{
	return *g_state.device;
}

CompareOp Port( Compare compare )
{
	switch ( compare )
	{
	case Compare::kNever:
		return CompareOp::kNever;
	case Compare::kLess:
		return CompareOp::kLess;
	case Compare::kEqual:
		return CompareOp::kEqual;
	case Compare::kLessEqual:
		return CompareOp::kLessEqual;
	case Compare::kGreater:
		return CompareOp::kGreater;
	case Compare::kNotEqual:
		return CompareOp::kNotEqual;
	case Compare::kGreaterEqual:
		return CompareOp::kGreaterEqual;
	case Compare::kAlways:
		return CompareOp::kAlways;
	}
	return CompareOp::kAlways;
}

// The port blend mode a D3D factor pair draws, or none.
std::optional<BlendMode> PortBlend( const DrawState &state )
{
	if ( !state.blend || ( state.src == Blend::kOne && state.dst == Blend::kZero ) )
		return BlendMode::kOpaque;
	struct Pair
	{
		Blend src, dst;
		BlendMode mode;
	};
	static const Pair pairs[] = { { Blend::kSrcAlpha, Blend::kOneMinusSrcAlpha, BlendMode::kAlpha },
	    { Blend::kOne, Blend::kOneMinusSrcAlpha, BlendMode::kPremultiplied },
	    { Blend::kOne, Blend::kOne, BlendMode::kAdditive },
	    { Blend::kSrcAlpha, Blend::kOne, BlendMode::kAlphaAdditive },
	    { Blend::kDstColor, Blend::kSrcColor, BlendMode::kModulate2x } };
	for ( const Pair &pair : pairs )
		if ( pair.src == state.src && pair.dst == state.dst )
			return pair.mode;
	return std::nullopt;
}

// Everything a pipeline is made of, packed.
std::uint64_t PipelineKey(
    const DrawState &state, BlendMode blend, bool textured, Primitive primitive )
{
	std::uint64_t key = 0;
	int at = 0;
	auto put = [&]( std::uint64_t value, int bits )
	{
		key |= value << at;
		at += bits;
	};
	put( state.depthTest, 1 );
	put( state.depthWrite, 1 );
	put( std::uint64_t( state.depthFunc ), 3 );
	put( std::uint64_t( blend ), 3 );
	put( state.alphaTest, 1 );
	put( std::uint64_t( state.alphaFunc ), 3 );
	put( state.alphaTest ? state.alphaRef : 0, 8 );
	put( state.cull, 1 );
	put( state.colorWrite, 1 );
	put( state.alphaWrite, 1 );
	put( textured, 1 );
	put( std::uint64_t( primitive ), 1 );
	return key;
}

std::uint8_t GpuTest( Compare compare )
{
	switch ( compare )
	{
	case Compare::kNever:
		return pf::test::kNever;
	case Compare::kLess:
		return pf::test::kLess;
	case Compare::kEqual:
		return pf::test::kEqual;
	case Compare::kLessEqual:
		return pf::test::kLessEqual;
	case Compare::kGreater:
		return pf::test::kGreater;
	case Compare::kNotEqual:
		return pf::test::kNotEqual;
	case Compare::kGreaterEqual:
		return pf::test::kGreaterEqual;
	case Compare::kAlways:
		return pf::test::kAlways;
	}
	return pf::test::kAlways;
}

PipelineId PipelineFor(
    const DrawState &state, BlendMode blend, bool textured, Primitive primitive )
{
	const std::uint64_t key = PipelineKey( state, blend, textured, primitive );
	if ( const auto found = g_state.pipelines.find( key ); found != g_state.pipelines.end() )
		return found->second;

	// The texel (when textured) times the vertex colour, which the vertex
	// program has already tinted.
	pf::FragmentProgram fragment;
	pf::CombinerStage stage;
	if ( textured )
	{
		fragment.textures.push_back( { std::uint8_t( BindGroupRole::kMaterial ), 0,
		    std::uint8_t( BindGroupRole::kMaterial ), 1 } );
		stage.rgbSources = {
		    pf::source::kTexture0, pf::source::kPrimaryColor, pf::source::kPrimaryColor };
		stage.rgbCombine = pf::combine::kModulate;
	}
	else
	{
		stage.rgbSources = {
		    pf::source::kPrimaryColor, pf::source::kPrimaryColor, pf::source::kPrimaryColor };
		stage.rgbCombine = pf::combine::kReplace;
	}
	stage.alphaSources = stage.rgbSources;
	stage.alphaCombine = stage.rgbCombine;
	fragment.stages.push_back( stage );
	if ( state.alphaTest )
	{
		fragment.alphaTest = GpuTest( state.alphaFunc );
		fragment.alphaReference = state.alphaRef;
	}
	const std::vector<std::byte> fragmentArtifact = pf::WriteFragmentProgram( fragment );

	static const ReflectedBinding sampled[] = {
	    { std::uint32_t( BindGroupRole::kMaterial ), 0, BindingKind::kSampledTexture },
	    { std::uint32_t( BindGroupRole::kMaterial ), 1, BindingKind::kSampler } };
	const ShaderArtifactView stages[] = {
	    { ShaderStage::kVertex, ArtifactFormat::kPica, g_state.vertexArtifact, "main", {} },
	    { ShaderStage::kFragment, ArtifactFormat::kPica, fragmentArtifact, "main",
	        textured ? std::span<const ReflectedBinding>( sampled )
	                 : std::span<const ReflectedBinding>() } };
	const BindGroupLayoutId layouts[] = {
	    {}, {}, textured ? g_state.materialLayout : BindGroupLayoutId{} };
	static const VertexAttribute attributes[] = { { 0, VertexFormat::kFloat3, 0, 0 },
	    { 1, VertexFormat::kUnorm8x4, 12, 0 }, { 2, VertexFormat::kFloat2, 16, 0 } };
	static const VertexBufferLayout buffers[] = { { sizeof( Vertex ), false } };
	const Format colors[] = { Format::kRGBA8Unorm };
	const BlendMode blends[] = { blend };
	const std::uint8_t masks[] = { std::uint8_t(
	    ( state.colorWrite ? kColorWriteRed | kColorWriteGreen | kColorWriteBlue : 0 ) |
	    ( state.alphaWrite ? kColorWriteAlpha : 0 ) ) };

	PipelineDesc desc;
	desc.stages = stages;
	desc.layouts = layouts;
	desc.drawConstantBytes = kDrawConstantBytes;
	desc.vertex = { attributes, buffers };
	desc.topology = primitive == Primitive::kTriangleStrip ? PrimitiveTopology::kTriangleStrip
	                                                       : PrimitiveTopology::kTriangleList;
	// D3D's front faces are clockwise; it culls the counter-clockwise ones.
	desc.raster.cull = state.cull ? CullMode::kBack : CullMode::kNone;
	desc.raster.frontCounterClockwise = false;
	desc.depthStencil.depthTest = state.depthTest;
	desc.depthStencil.depthWrite = state.depthWrite;
	desc.depthStencil.compare = state.depthTest ? Port( state.depthFunc ) : CompareOp::kAlways;
	desc.colorFormats = colors;
	desc.blends = blends;
	desc.colorWriteMasks = masks;
	desc.depthFormat = Format::kD24UnormS8;
	desc.debugName = "shaderapipica";
	auto pipeline = Device().CreatePipeline( desc );
	const PipelineId id = pipeline ? pipeline.Value() : PipelineId{};
	g_state.pipelines.emplace( key, id );
	return id;
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

// A mapped upload buffer, already in its drawing usage.
std::byte *CreateLinear( Memory kind, std::size_t bytes, BufferId &out )
{
	BufferDesc desc;
	desc.size = bytes;
	desc.memory = MemoryKind::kUpload;
	desc.usages = { UsageOf( kind ) };
	auto buffer = Device().CreateBuffer( desc );
	if ( !buffer )
		return nullptr;
	auto encoder = Device().BeginEncoder( QueueKind::kGraphics );
	if ( !encoder )
		return nullptr;
	encoder.Value().TransitionBuffer( buffer.Value(), ResourceUsage::kUndefined, UsageOf( kind ) );
	CommandEncoder list[] = { std::move( encoder ).Value() };
	(void)Device().Submit( QueueKind::kGraphics, list, {} );
	const std::span<std::byte> mapped = pc::MapUploadBuffer( Device(), buffer.Value() );
	out = buffer.Value();
	return mapped.data();
}

void BeginPass( bool clearColor, bool clearDepth, std::uint32_t rgba )
{
	CommandEncoder &e = *g_state.encoder;
	if ( g_state.rendering )
		e.EndRendering();
	ColorAttachment color{
	    g_state.color, clearColor ? LoadOp::kClear : LoadOp::kLoad, StoreOp::kStore, {}, {} };
	color.clear = { float( ( rgba >> 0 ) & 0xFF ) / 255.0f, float( ( rgba >> 8 ) & 0xFF ) / 255.0f,
	    float( ( rgba >> 16 ) & 0xFF ) / 255.0f, float( ( rgba >> 24 ) & 0xFF ) / 255.0f };
	const ColorAttachment colors[] = { color };
	RenderingDesc rendering;
	rendering.colors = colors;
	rendering.depth = DepthAttachment{
	    g_state.depth, clearDepth ? LoadOp::kClear : LoadOp::kLoad, StoreOp::kStore, 1.0f };
	rendering.width = kScreenWidth;
	rendering.height = kScreenHeight;
	e.BeginRendering( rendering );
	e.SetViewport( g_state.viewport );
	g_state.rendering = true;
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
    : m_texture( 0 ), m_group( 0 ), m_repeat( true ), m_width( 0 ), m_height( 0 ), m_bytes( 0 )
{
}

Texture::~Texture()
{
	Release();
}

void Texture::Release()
{
	if ( m_texture && g_state.initialized )
	{
		// Drawn by the recording, perhaps: released after it is submitted.
		g_state.releases.push_back( TextureId{ m_texture } );
		if ( m_group )
			g_state.releases.push_back( BindGroupId{ m_group } );
		g_state.textureBytes -= m_bytes;
	}
	m_texture = m_group = 0;
	m_bytes = 0;
}

bool Texture::Upload(
    UploadFormat format, int width, int height, int levelCount, const std::uint8_t *const *levels )
{
	Release();
	if ( levelCount < 1 || !g_state.initialized )
		return false;
	TextureDesc desc;
	desc.format = format == UploadFormat::kETC1     ? Format::kETC1Rgb
	              : format == UploadFormat::kETC1A4 ? Format::kETC1A4
	                                                : Format::kRGBA8Unorm;
	desc.width = std::uint32_t( width );
	desc.height = std::uint32_t( height );
	desc.mipLevels = std::uint32_t( levelCount );
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
	std::vector<BufferId> staging;
	std::size_t total = 0;
	for ( int level = 0; level < levelCount; ++level )
	{
		const std::uint32_t w = std::uint32_t( width >> level ),
		                    h = std::uint32_t( height >> level );
		const std::size_t bytes =
		    format == UploadFormat::kRGBA8
		        ? std::size_t( w ) * h * 4
		        : std::size_t( w / 4 ) * ( h / 4 ) * ( format == UploadFormat::kETC1 ? 8 : 16 );
		auto buffer = Device().CreateUploadBuffer(
		    { reinterpret_cast<const std::byte *>( levels[level] ), bytes } );
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
		(void)Device().Release( texture.Value(), {} );
		return false;
	}
	m_texture = texture.Value().value;
	m_width = width;
	m_height = height;
	m_bytes = total;
	g_state.textureBytes += total;
	return true;
}

void Texture::SetWrap( bool wrapS, bool wrapT )
{
	const bool repeat = wrapS && wrapT;
	if ( repeat == m_repeat )
		return;
	m_repeat = repeat;
	if ( m_group )
		g_state.releases.push_back( BindGroupId{ m_group } );
	m_group = 0;
}

std::uint32_t Texture::Group()
{
	if ( !m_texture )
		return 0;
	if ( !m_group )
	{
		const BindGroupEntry entries[] = { { 0, {}, 0, 0, TextureId{ m_texture }, {} },
		    { 1, {}, 0, 0, {}, g_state.samplers[m_repeat ? 1 : 0] } };
		auto group = Device().CreateBindGroup( { g_state.materialLayout, entries } );
		m_group = group ? group.Value().value : 0;
	}
	return m_group;
}

void BindDevice( IRenderDevice2 *device )
{
	g_bound = device;
}

bool Init()
{
	if ( g_state.initialized )
		return true;
	N3ds_StartDebugConsole();
	if ( !g_bound )
	{
		std::printf( "pica: no render core device was handed to the shader API\n" );
		return false;
	}
	g_state.device = g_bound;

	TextureDesc desc;
	desc.format = Format::kRGBA8Unorm;
	desc.width = kTargetWidth;
	desc.height = kTargetHeight;
	desc.usages = {
	    ResourceUsage::kColorAttachment, ResourceUsage::kSampled, ResourceUsage::kCopySource };
	auto color = Device().CreateTexture( desc );
	desc.format = Format::kD24UnormS8;
	desc.usages = { ResourceUsage::kDepthWrite };
	auto depth = Device().CreateTexture( desc );
	static const BindingDesc bindings[] = {
	    { 0, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 1, BindingKind::kSampler, 1, { ShaderStage::kFragment } } };
	auto layout = Device().CreateBindGroupLayout( { BindGroupRole::kMaterial, bindings } );
	SamplerDesc sampler;
	sampler.magFilter = Filter::kLinear;
	sampler.minFilter = Filter::kNearest;
	sampler.mipFilter = Filter::kLinear;
	sampler.address = AddressMode::kClampToEdge;
	auto clamped = Device().CreateSampler( sampler );
	sampler.address = AddressMode::kRepeat;
	auto repeating = Device().CreateSampler( sampler );
	if ( !color || !depth || !layout || !clamped || !repeating )
	{
		std::printf( "pica: the frame's target, layout or samplers were refused\n" );
		g_state = State();
		return false;
	}
	g_state.color = color.Value();
	g_state.depth = depth.Value();
	g_state.materialLayout = layout.Value();
	g_state.samplers[0] = clamped.Value();
	g_state.samplers[1] = repeating.Value();

	pf::VertexProgram program;
	program.drawConstantRegister = 0;
	g_state.vertexArtifact = pf::WriteVertexProgram( program,
	    { reinterpret_cast<const std::byte *>( fullbright_shbin ), fullbright_shbin_size } );

	const std::size_t ringBytes[2] = { kVertexRingBytes, kIndexRingBytes };
	for ( int kind = 0; kind < 2; ++kind )
	{
		Ring &ring = g_state.rings[kind];
		ring.data = CreateLinear( Memory( kind ), ringBytes[kind], ring.buffer );
		ring.bytes = ringBytes[kind];
		if ( !ring.data )
		{
			std::printf( "pica: the transient rings were refused\n" );
			g_state = State();
			return false;
		}
		Allocation allocation;
		allocation.buffer = ring.buffer;
		allocation.bytes = ring.bytes;
		allocation.kind = Memory( kind );
		allocation.transient = true;
		g_state.allocations.emplace( reinterpret_cast<std::uintptr_t>( ring.data ), allocation );
	}
	g_state.initialized = true;
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
	for ( const auto &[key, pipeline] : g_state.pipelines )
		if ( pipeline.IsValid() )
			(void)device.Release( pipeline, {} );
	for ( const auto &[address, allocation] : g_state.allocations )
		(void)device.Release( allocation.buffer, {} );
	for ( ResourceId id : { ResourceId( g_state.color ), ResourceId( g_state.depth ),
			  ResourceId( g_state.materialLayout ), ResourceId( g_state.samplers[0] ),
			  ResourceId( g_state.samplers[1] ) } )
		(void)device.Release( id, {} );
	(void)device.Poll();
	g_state = State();
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
	for ( Ring &ring : g_state.rings )
		ring.used = 0;
	g_state.stats = Stats();
	g_state.viewport = { 0, 0, float( kScreenWidth ), float( kScreenHeight ), 0, 1 };
	OpenRecording();
	BeginPass( false, false, 0 );
}

void EndFrame()
{
	if ( !g_state.inFrame )
		return;
	g_state.stats.textureBytes = g_state.textureBytes;
	SubmitRecording( true );
	(void)pc::PresentTopScreen( Device(), g_state.color, kScreenWidth, kScreenHeight );
	g_state.inFrame = false;
}

void Clear( bool color, bool depth, std::uint32_t rgba )
{
	if ( !g_state.inFrame )
		BeginFrame();
	if ( !color && !depth )
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

void *AllocTransient( Memory kind, std::size_t bytes, std::size_t align )
{
	Ring &ring = g_state.rings[int( kind )];
	std::size_t at = ( ring.used + align - 1 ) & ~( align - 1 );
	if ( !ring.data || at + bytes > ring.bytes )
	{
		++g_state.stats.ringOverflows;
		return nullptr;
	}
	ring.used = at + bytes;
	return ring.data + at;
}

void *AllocLinear( Memory kind, std::size_t bytes )
{
	if ( !g_state.initialized || bytes == 0 )
		return nullptr;
	Allocation allocation;
	allocation.kind = kind;
	allocation.bytes = bytes;
	std::byte *data = CreateLinear( kind, bytes, allocation.buffer );
	if ( !data )
		return nullptr;
	g_state.allocations.emplace( reinterpret_cast<std::uintptr_t>( data ), allocation );
	return data;
}

void FreeLinear( void *ptr )
{
	if ( !ptr )
		return;
	const auto found = g_state.allocations.find( reinterpret_cast<std::uintptr_t>( ptr ) );
	if ( found == g_state.allocations.end() || found->second.transient )
		return;
	g_state.releases.push_back( found->second.buffer );
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
	Allocation *allocation = Find( ptr, nullptr );
	if ( allocation && !allocation->transient && allocation->usedInRecording == g_state.recording &&
	     g_state.encoder )
		SubmitRecording( false );
}

void FlushLinear( const void *ptr, std::size_t bytes )
{
	std::size_t offset = 0;
	if ( Allocation *allocation = Find( ptr, &offset ) )
		pc::FlushUploadBuffer( Device(), allocation->buffer, offset, bytes );
}

void Draw( const float clipFromObject[16], const DrawState &state, Texture *texture,
    const Vertex *vertices, int vertexCount, const std::uint16_t *indices, int indexCount,
    Primitive primitive )
{
	if ( !g_state.inFrame )
		BeginFrame();
	if ( vertexCount <= 0 || !vertices || !g_state.encoder )
		return;
	const std::optional<BlendMode> blend = PortBlend( state );
	if ( !blend )
	{
		++g_state.stats.blendRefusals;
		return;
	}
	std::size_t vertexOffset = 0, indexOffset = 0;
	Allocation *vertexMemory = Find( vertices, &vertexOffset );
	Allocation *indexMemory = indices ? Find( indices, &indexOffset ) : nullptr;
	if ( !vertexMemory || vertexMemory->kind != Memory::kVertices ||
	     ( indices && ( !indexMemory || indexMemory->kind != Memory::kIndices ) ) )
		return;
	const bool textured = texture && texture->Valid();
	const PipelineId pipeline = PipelineFor( state, *blend, textured, primitive );
	if ( !pipeline.IsValid() )
		return;

	// The column-vector rows of D3D's row-vector matrix (the port's clip
	// space is D3D's), then the tint.
	float constants[20];
	for ( int i = 0; i < 4; ++i )
		for ( int j = 0; j < 4; ++j )
			constants[j * 4 + i] = clipFromObject[i * 4 + j];
	std::memcpy( constants + 16, state.tint, sizeof( state.tint ) );

	CommandEncoder &e = *g_state.encoder;
	e.SetPipeline( pipeline );
	if ( textured )
		e.SetBindGroup( BindGroupRole::kMaterial, BindGroupId{ texture->Group() } );
	e.SetDrawConstants( 0, std::as_bytes( std::span( constants ) ) );
	e.SetVertexBuffer( 0, vertexMemory->buffer, vertexOffset );
	vertexMemory->usedInRecording = g_state.recording;
	if ( indices && indexCount > 0 )
	{
		indexMemory->usedInRecording = g_state.recording;
		e.SetIndexBuffer( indexMemory->buffer, indexOffset, IndexFormat::kUint16 );
		e.DrawIndexed( std::uint32_t( indexCount ) );
		g_state.stats.triangles +=
		    std::uint32_t( primitive == Primitive::kTriangles ? indexCount / 3 : indexCount - 2 );
	}
	else
	{
		e.Draw( std::uint32_t( vertexCount ) );
		g_state.stats.triangles +=
		    std::uint32_t( primitive == Primitive::kTriangles ? vertexCount / 3 : vertexCount - 2 );
	}
	++g_state.stats.draws;
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
	e.CopyTextureToBuffer(
	    g_state.color, readback.Value(), { 0, 0, 0, kScreenWidth, kScreenHeight } );
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

} // namespace pica
