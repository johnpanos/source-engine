//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: pica_portals: a standalone 3DS program on render.device.pica (RFC
//			0026) that draws one portal scene with several portal techniques
//			and measures each over the same camera path.
//
//			  none                 the portals as flat ellipses: the cost floor
//			  stencil              Portal's own method: mask, depth reset,
//			                       remote scene with an oblique near plane,
//			                       seal; the image reference
//			  stencil-crop         the remote projection cropped to the
//			                       portal's screen rectangle (viewport = that
//			                       rectangle): the clipper rejects what lies
//			                       outside it, culling uses the narrow frustum
//			  screen-rtt           the cropped view into a texture at 1:1,
//			                       sampled by a CPU pre-projected ellipse
//			                       (w = 1, so affine screen coordinates are
//			                       exact without projective texturing)
//			  plane-rtt            Kooima's off-axis projection with the portal
//			                       as image plane: the texture depends on the
//			                       eye's position only and maps onto the
//			                       ellipse with static coordinates
//			  plane-rtt-cached     plane-rtt kept while the eye moves less than
//			                       a texel's worth (turning is free)
//			  plane-rtt-recursive  cached, and a portal seen in a portal shows
//			                       its previous texture (unbounded recursion,
//			                       one render per portal)
//			  seed-mirror          plane-rtt with its texture mirrored: a seeded
//			                       defect the image comparison must catch
//
//			Per frame it records CPU time (recording, and Submit, which
//			replays and drains), draws, indices and the texels the device
//			clears on the CPU; on sampled frames a counting pass draws every
//			draw again with an additive 1/255 into a scratch target to count
//			the fragments the GPU rasterizes (before depth and stencil), by
//			category. Gallery frames are compared with the stencil image and
//			written as PPM.
//
//			Output: sdmc:/portal_lab/report.txt (checks-v1 lines and the
//			summary), frames.csv, <technique>_<frame>.ppm and portal_lab.done.
//
//=============================================================================//

#include <3ds.h>

#include "portal_math.h"
#include "portal_scene.h"

#include "render/device/pica/provider.h"
#include "render/device/pica_codes.h"
#include "render/device/pica_format.h"
#include "testing/checks.h"

#include <sys/stat.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include <vector>

extern "C" u32 __stacksize__;
u32 __stacksize__ = 512 * 1024;

extern "C" const unsigned char portal_scene_shbin[];
extern "C" const unsigned portal_scene_shbin_size;

#ifndef PORTAL_LAB_FRAMES
#define PORTAL_LAB_FRAMES 24 // per segment
#endif

namespace
{

using namespace render::device;
using namespace pica_portals;
namespace pf = render::device::pica_format;

constexpr std::uint32_t kTargetWidth = 512, kTargetHeight = 256;
constexpr std::uint32_t kShownWidth = 400, kShownHeight = 240;
constexpr float kNear = 2, kFar = 2048, kClipOffset = 0.5f;
constexpr float kTicksPerMicrosecond = float( SYSCLOCK_ARM11 ) / 1e6f;
constexpr int kLods = 5; // portal-plane texture sizes
constexpr std::uint32_t kLodWidth[kLods] = { 16, 32, 64, 128, 256 };
constexpr std::uint32_t kLodHeight[kLods] = { 32, 64, 128, 256, 512 };
constexpr float kCacheTexels = 1.0f; // the eye may move this many texels' worth

enum class Technique : std::uint8_t
{
	kStencil,
	kStencilCpuClear,
	kNone,
	kStencilCrop,
	kScreenRtt,
	kPlaneRtt,
	kPlaneRttCached,
	kPlaneRttRecursive,
	kPlaneVisible,
	kPlaneVisibleCached,
	kPlaneVisibleRecursive,
	kStencilCropCone,
	kPlaneBest,
	kPlaneBestHalf,
	kSeedMirror,
	kCount
};

const char *Name( Technique t )
{
	static const char *const names[] = { "stencil", "stencil-cpu-clear", "none", "stencil-crop",
	    "screen-rtt", "plane-rtt", "plane-rtt-cached", "plane-rtt-recursive", "plane-visible",
	    "plane-visible-cached", "plane-visible-recursive", "stencil-crop-cone", "plane-best",
	    "plane-best-half", "seed-mirror" };
	return names[int( t )];
}

enum class Pipe : std::uint8_t
{
	kScene,           // texture x colour, depth, back faces culled
	kSceneStencil,    // the same inside stencil 1
	kTextured,        // texture x colour, depth, no culling (ellipses)
	kTexturedStencil, // the same inside stencil 1
	kFlat,            // colour, depth, no culling (rims)
	kFlatStencil,     // the same inside stencil 1
	kMask,            // stencil := 1 where the ellipse passes depth; no colour
	kDepthReset,      // depth := far inside stencil 1; no colour
	kSeal,            // depth := the ellipse's, stencil := 0, inside stencil 1
	kClear,           // colour, depth and stencil := the pass's clear values
	kCountBack,       // + 1/255 red per fragment, back faces culled, no tests
	kCountNone,       // the same, no culling
	kCount
};

enum class Category : std::uint8_t
{
	kMain,    // the camera's own scene
	kRemote,  // what is seen through portals (in any pass)
	kStencil, // mask, depth reset and seal
	kPortal,  // ellipses and rims drawn in the main view
	kClear,   // GPU clears (a quad per pass)
	kCount
};
const char *const kCategoryNames[] = { "main", "remote", "stencil", "portal", "clear" };

struct DrawCmd
{
	Pipe pipe = Pipe::kScene;
	Category category = Category::kMain;
	M4 clip;
	BufferId vertices, indices;
	std::uint32_t first = 0, count = 0;
	std::int32_t vertexOffset = 0;
	BindGroupId texture;
	Viewport viewport;
};

struct Pass
{
	TextureId color, depth;
	std::uint32_t width = 0, height = 0;
	ClearColor clear;
	// Cleared by a quad drawn first (kClear) instead of the load op, which
	// the PICA adapter runs on the CPU over every texel.
	bool gpuClear = true;
	std::vector<DrawCmd> draws;
};

struct FrameStats
{
	float recordUs = 0, submitUs = 0;
	std::uint32_t draws = 0, indices = 0, passes = 0, rttRenders = 0;
	std::uint64_t clearedTexels = 0;
	bool counted = false;
	std::array<std::uint64_t, std::size_t( Category::kCount )> fragments{};
};

// A portal-plane texture and what it holds.
struct PortalCache
{
	bool valid = false;
	V3 eye;                               // the eye it was rendered for
	int lod = -1;                         // full-portal textures: the size; visible: -1
	int front = 0;                        // of the ping-pong pair
	float u0 = 0, u1 = 1, v0 = 0, v1 = 1; // the portal-plane rectangle it holds (v down)
	float su = 1, sv = 1;                 // the rendered region's share of the texture
	float density = 0;                    // texels per world unit on the portal
	BindGroupId group;
};

std::FILE *g_report = nullptr;

class Lab
{
public:
	testing::Checks checks{ stdout };

	bool Init()
	{
		auto created = pica::Create( {} );
		if ( !created )
		{
			std::printf( "INFO device creation failed: %u\n", unsigned( created.Error().status ) );
			return false;
		}
		m_Device = std::move( created ).Value();
		m_Scene = BuildScene();
		return MakeBuffers() && MakeTextures() && MakePipelines();
	}

	IRenderDevice2 &Device() { return *m_Device; }
	const Scene &GetScene() const { return m_Scene; }

	// -- Resources ------------------------------------------------------------

	BufferId Linear( std::span<const std::byte> bytes, std::size_t size, ResourceUsage usage,
	    std::byte **mapped = nullptr )
	{
		BufferDesc desc;
		desc.size = size;
		desc.memory = MemoryKind::kUpload;
		desc.usages = { usage };
		auto buffer = m_Device->CreateBuffer( desc );
		if ( !buffer )
			return {};
		auto encoder = m_Device->BeginEncoder( QueueKind::kGraphics );
		encoder.Value().TransitionBuffer( buffer.Value(), ResourceUsage::kUndefined, usage );
		CommandEncoder list[] = { std::move( encoder ).Value() };
		(void)m_Device->Submit( QueueKind::kGraphics, list, {} );
		const std::span<std::byte> map = pica::MapUploadBuffer( *m_Device, buffer.Value() );
		if ( map.size() < size )
			return {};
		if ( !bytes.empty() )
		{
			std::memcpy( map.data(), bytes.data(), bytes.size() );
			pica::FlushUploadBuffer( *m_Device, buffer.Value(), 0, bytes.size() );
		}
		if ( mapped )
			*mapped = map.data();
		return buffer.Value();
	}

	bool MakeBuffers()
	{
		const Mesh &mesh = m_Scene.mesh;
		m_SceneVertices = Linear( std::as_bytes( std::span( mesh.vertices ) ),
		    mesh.vertices.size() * sizeof( Vertex ), ResourceUsage::kVertex );
		m_SceneIndices = Linear( std::as_bytes( std::span( mesh.indices ) ),
		    mesh.indices.size() * 2, ResourceUsage::kIndex );
		for ( int i = 0; i < 2; ++i )
		{
			const PortalGeometry &g = m_Scene.geometry[std::size_t( i )];
			m_PortalVertices[i] = Linear( std::as_bytes( std::span( g.vertices ) ),
			    g.vertices.size() * sizeof( Vertex ), ResourceUsage::kVertex );
			m_PortalIndices[i] = Linear( std::as_bytes( std::span( g.indices ) ),
			    g.indices.size() * 2, ResourceUsage::kIndex );
		}
		// The clear quad: identity clip, just short of the far plane.
		const Vertex quad[] = { { -1, -1, 0.99999f, 0, 0, { 26, 26, 31, 255 } },
		    { 1, -1, 0.99999f, 0, 0, { 26, 26, 31, 255 } },
		    { 1, 1, 0.99999f, 0, 0, { 26, 26, 31, 255 } },
		    { -1, 1, 0.99999f, 0, 0, { 26, 26, 31, 255 } } };
		const std::uint16_t quadIndices[] = { 0, 1, 2, 0, 2, 3 };
		m_QuadVertices =
		    Linear( std::as_bytes( std::span( quad ) ), sizeof( quad ), ResourceUsage::kVertex );
		m_QuadIndices = Linear( std::as_bytes( std::span( quadIndices ) ), sizeof( quadIndices ),
		    ResourceUsage::kIndex );
		m_DynamicVertices =
		    Linear( {}, kDynamicVertices * sizeof( Vertex ), ResourceUsage::kVertex, &m_DynamicV );
		m_DynamicIndices = Linear( {}, kDynamicIndices * 2, ResourceUsage::kIndex, &m_DynamicI );
		BufferDesc readback;
		readback.size = std::uint64_t( kTargetWidth ) * kTargetHeight * 4;
		readback.memory = MemoryKind::kReadback;
		readback.usages = { ResourceUsage::kCopyDestination };
		auto made = m_Device->CreateBuffer( readback );
		if ( !made )
			return false;
		m_Readback = made.Value();
		return m_SceneVertices.IsValid() && m_SceneIndices.IsValid() &&
		       m_PortalVertices[1].IsValid() && m_DynamicVertices.IsValid() &&
		       m_DynamicIndices.IsValid();
	}

	TextureId Texture(
	    std::uint32_t w, std::uint32_t h, Format format, UsageSet usages, std::uint32_t mips = 1 )
	{
		TextureDesc desc;
		desc.format = format;
		desc.width = w;
		desc.height = h;
		desc.mipLevels = mips;
		desc.usages = usages;
		auto made = m_Device->CreateTexture( desc );
		return made ? made.Value() : TextureId{};
	}

	void Upload( TextureId texture, std::uint32_t w, std::uint32_t h,
	    const std::vector<std::uint8_t> &rgba, std::uint32_t mip = 0 )
	{
		auto staging = m_Device->CreateUploadBuffer( std::as_bytes( std::span( rgba ) ) );
		auto encoder = m_Device->BeginEncoder( QueueKind::kGraphics );
		CommandEncoder &e = encoder.Value();
		Transition( e, texture, ResourceUsage::kCopyDestination );
		e.CopyBufferToTexture( staging.Value(), texture, { 0, mip, 0, w, h } );
		Transition( e, texture, ResourceUsage::kSampled );
		CommandEncoder list[] = { std::move( e ) };
		auto token = m_Device->Submit( QueueKind::kGraphics, list, {} );
		(void)m_Device->Release( staging.Value(), token ? token.Value() : CompletionToken{} );
	}

	BindGroupId Group( TextureId texture, SamplerId sampler )
	{
		const BindGroupEntry entries[] = {
		    { 0, {}, 0, 0, texture, {} }, { 1, {}, 0, 0, {}, sampler } };
		auto made = m_Device->CreateBindGroup( { m_MaterialLayout, entries } );
		return made ? made.Value() : BindGroupId{};
	}

	bool MakeTextures()
	{
		static const BindingDesc bindings[] = {
		    { 0, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
		    { 1, BindingKind::kSampler, 1, { ShaderStage::kFragment } } };
		m_MaterialLayout =
		    m_Device->CreateBindGroupLayout( { BindGroupRole::kMaterial, bindings } ).Value();
		SamplerDesc repeat;
		m_Repeat = m_Device->CreateSampler( repeat ).Value();
		SamplerDesc clamp;
		clamp.address = AddressMode::kClampToEdge;
		clamp.mipFilter = Filter::kNearest;
		m_ClampLinear = m_Device->CreateSampler( clamp ).Value();
		clamp.minFilter = clamp.magFilter = Filter::kNearest;
		m_ClampNearest = m_Device->CreateSampler( clamp ).Value();

		// The scene's checker (64 x 64, panels of 16 with a seam).
		std::vector<std::uint8_t> checker( 64 * 64 * 4 );
		for ( int y = 0; y < 64; ++y )
			for ( int x = 0; x < 64; ++x )
			{
				const bool seam = ( x % 32 ) == 0 || ( y % 32 ) == 0;
				const bool dark = ( ( x / 16 ) + ( y / 16 ) ) % 2 != 0;
				const std::uint8_t v = seam ? 110 : dark ? 200 : 245;
				std::uint8_t *t = &checker[std::size_t( y * 64 + x ) * 4];
				t[0] = t[1] = t[2] = v;
				t[3] = 255;
			}
		const UsageSet sampled = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
		// Four levels (64 to 8, the PICA's smallest sampled level), box filtered.
		m_Checker = Texture( 64, 64, Format::kRGBA8Unorm, sampled, 4 );
		Upload( m_Checker, 64, 64, checker );
		for ( std::uint32_t mip = 1, size = 32; mip < 4; ++mip, size /= 2 )
		{
			std::vector<std::uint8_t> next( std::size_t( size ) * size * 4 );
			for ( std::uint32_t y = 0; y < size; ++y )
				for ( std::uint32_t x = 0; x < size; ++x )
					for ( std::uint32_t c = 0; c < 4; ++c )
					{
						unsigned sum = 0;
						for ( std::uint32_t k = 0; k < 4; ++k )
							sum += checker[( std::size_t( 2 * y + k / 2 ) * size * 2 + 2 * x +
							                   k % 2 ) *
							                   4 +
							               c];
						next[( std::size_t( y ) * size + x ) * 4 + c] = std::uint8_t( sum / 4 );
					}
			Upload( m_Checker, size, size, next, mip );
			checker = std::move( next );
		}
		m_CheckerGroup = Group( m_Checker, m_Repeat );
		// Flat portal interiors (a portal seen through a portal, no recursion).
		for ( int i = 0; i < 2; ++i )
		{
			const std::uint32_t rgb = i == 0 ? 0x803808 : 0x103870;
			std::vector<std::uint8_t> solid( 8 * 8 * 4 );
			for ( std::size_t t = 0; t < 64; ++t )
			{
				solid[t * 4 + 0] = std::uint8_t( rgb >> 16 );
				solid[t * 4 + 1] = std::uint8_t( rgb >> 8 );
				solid[t * 4 + 2] = std::uint8_t( rgb );
				solid[t * 4 + 3] = 255;
			}
			m_Flat[i] = Texture( 8, 8, Format::kRGBA8Unorm, sampled );
			Upload( m_Flat[i], 8, 8, solid );
			m_FlatGroup[i] = Group( m_Flat[i], m_ClampNearest );
		}

		const UsageSet target = {
		    ResourceUsage::kColorAttachment, ResourceUsage::kSampled, ResourceUsage::kCopySource };
		const UsageSet depth = { ResourceUsage::kDepthWrite };
		m_MainColor = Texture( kTargetWidth, kTargetHeight, Format::kRGBA8Unorm, target );
		m_MainDepth = Texture( kTargetWidth, kTargetHeight, Format::kD24UnormS8, depth );
		m_CountTarget = Texture( kTargetWidth, kTargetHeight, Format::kRGBA8Unorm, target );
		for ( int i = 0; i < 2; ++i )
		{
			m_ScreenColor[i] = Texture( kTargetWidth, kTargetHeight, Format::kRGBA8Unorm, target );
			m_ScreenGroup[i] = Group( m_ScreenColor[i], m_ClampNearest );
		}
		m_ScreenDepth = Texture( kTargetWidth, kTargetHeight, Format::kD24UnormS8, depth );
		for ( int i = 0; i < 2; ++i )
			for ( int k = 0; k < 2; ++k )
			{
				m_VisibleColor[i][k] =
				    Texture( kTargetWidth, kTargetHeight, Format::kRGBA8Unorm, target );
				m_VisibleGroup[i][k] = Group( m_VisibleColor[i][k], m_ClampLinear );
			}
		bool ok = m_MainColor.IsValid() && m_MainDepth.IsValid() && m_CountTarget.IsValid() &&
		          m_ScreenColor[1].IsValid() && m_ScreenDepth.IsValid();
		for ( int lod = 0; lod < kLods; ++lod )
		{
			m_PlaneDepth[lod] =
			    Texture( kLodWidth[lod], kLodHeight[lod], Format::kD24UnormS8, depth );
			ok = ok && m_PlaneDepth[lod].IsValid();
			for ( int i = 0; i < 2; ++i )
				for ( int k = 0; k < 2; ++k )
				{
					TextureId &t = m_PlaneColor[lod][i][k];
					t = Texture( kLodWidth[lod], kLodHeight[lod], Format::kRGBA8Unorm, target );
					m_PlaneGroup[lod][i][k] = Group( t, m_ClampLinear );
					ok = ok && t.IsValid() && m_PlaneGroup[lod][i][k].IsValid();
				}
		}
		return ok && m_CheckerGroup.IsValid() && m_FlatGroup[1].IsValid();
	}

	bool MakePipelines()
	{
		pf::VertexProgram vertex;
		vertex.drawConstantRegister = 0;
		m_VertexArtifact = pf::WriteVertexProgram(
		    vertex, { reinterpret_cast<const std::byte *>( portal_scene_shbin ),
		                portal_scene_shbin_size } );

		auto stage = []( std::uint8_t a, std::uint8_t b, std::uint8_t combine )
		{
			pf::CombinerStage s;
			s.rgbSources = { a, b, b };
			s.alphaSources = s.rgbSources;
			s.rgbCombine = s.alphaCombine = combine;
			return s;
		};
		pf::FragmentProgram textured;
		textured.textures.push_back( { 2, 0, 2, 1 } );
		textured.stages.push_back(
		    stage( pf::source::kTexture0, pf::source::kPrimaryColor, pf::combine::kModulate ) );
		pf::FragmentProgram flat;
		flat.stages.push_back(
		    stage( pf::source::kPrimaryColor, pf::source::kPrimaryColor, pf::combine::kReplace ) );
		pf::FragmentProgram count;
		pf::CombinerStage one =
		    stage( pf::source::kConstant, pf::source::kConstant, pf::combine::kReplace );
		one.constantColor = 0x00000001u; // R = 1/255, the rest 0 (R in the low byte)
		count.stages.push_back( one );
		m_Fragments[0] = pf::WriteFragmentProgram( textured );
		m_Fragments[1] = pf::WriteFragmentProgram( flat );
		m_Fragments[2] = pf::WriteFragmentProgram( count );

		StencilState inside;
		inside.enabled = true;
		inside.compare = CompareOp::kEqual;
		inside.reference = 1;
		inside.writeMask = 0;
		StencilState mark = inside;
		mark.compare = CompareOp::kAlways;
		mark.pass = StencilOp::kReplace;
		mark.writeMask = 255;
		StencilState zero = mark;
		zero.reference = 0;
		StencilState unmark = inside;
		unmark.pass = StencilOp::kZero;
		unmark.writeMask = 255;
		const DepthStencilState normal{ true, true, CompareOp::kLessEqual, {} };
		auto with = []( DepthStencilState ds, StencilState st )
		{
			ds.stencil = st;
			return ds;
		};
		const DepthStencilState overwrite{ true, true, CompareOp::kAlways, {} };
		struct Spec
		{
			Pipe pipe;
			int fragment;
			CullMode cull;
			DepthStencilState ds;
			std::uint8_t mask;
			BlendMode blend;
			bool depth;
		};
		const Spec specs[] = {
		    { Pipe::kScene, 0, CullMode::kBack, normal, kColorWriteAll, BlendMode::kOpaque, true },
		    { Pipe::kSceneStencil, 0, CullMode::kBack, with( normal, inside ), kColorWriteAll,
		        BlendMode::kOpaque, true },
		    { Pipe::kTextured, 0, CullMode::kNone, normal, kColorWriteAll, BlendMode::kOpaque,
		        true },
		    { Pipe::kTexturedStencil, 0, CullMode::kNone, with( normal, inside ), kColorWriteAll,
		        BlendMode::kOpaque, true },
		    { Pipe::kFlat, 1, CullMode::kNone, normal, kColorWriteAll, BlendMode::kOpaque, true },
		    { Pipe::kFlatStencil, 1, CullMode::kNone, with( normal, inside ), kColorWriteAll,
		        BlendMode::kOpaque, true },
		    { Pipe::kMask, 1, CullMode::kNone,
		        with( { true, false, CompareOp::kLessEqual, {} }, mark ), 0, BlendMode::kOpaque,
		        true },
		    { Pipe::kDepthReset, 1, CullMode::kNone, with( overwrite, inside ), 0,
		        BlendMode::kOpaque, true },
		    { Pipe::kSeal, 1, CullMode::kNone, with( overwrite, unmark ), 0, BlendMode::kOpaque,
		        true },
		    { Pipe::kClear, 1, CullMode::kNone, with( overwrite, zero ), kColorWriteAll,
		        BlendMode::kOpaque, true },
		    { Pipe::kCountBack, 2, CullMode::kBack, {}, kColorWriteAll, BlendMode::kAdditive,
		        false },
		    { Pipe::kCountNone, 2, CullMode::kNone, {}, kColorWriteAll, BlendMode::kAdditive,
		        false },
		};
		static const ReflectedBinding sampled[] = {
		    { 2, 0, BindingKind::kSampledTexture }, { 2, 1, BindingKind::kSampler } };
		static const VertexAttribute attributes[] = { { 0, VertexFormat::kFloat3, 0, 0 },
		    { 1, VertexFormat::kFloat2, 12, 0 }, { 2, VertexFormat::kUnorm8x4, 20, 0 } };
		static const VertexBufferLayout buffers[] = { { sizeof( Vertex ), false } };
		const Format colors[] = { Format::kRGBA8Unorm };
		for ( const Spec &s : specs )
		{
			const bool usesTexture = s.fragment == 0;
			const ShaderArtifactView stages[] = {
			    { ShaderStage::kVertex, ArtifactFormat::kPica, m_VertexArtifact, "main", {}, 64 },
			    { ShaderStage::kFragment, ArtifactFormat::kPica, m_Fragments[s.fragment], "main",
			        usesTexture ? std::span<const ReflectedBinding>( sampled )
			                    : std::span<const ReflectedBinding>() } };
			const BindGroupLayoutId layouts[] = {
			    {}, {}, usesTexture ? m_MaterialLayout : BindGroupLayoutId{} };
			const BlendMode blends[] = { s.blend };
			const std::uint8_t masks[] = { s.mask };
			PipelineDesc desc;
			desc.stages = stages;
			desc.layouts = layouts;
			desc.drawConstantBytes = 64;
			desc.vertex = { attributes, buffers };
			desc.raster.cull = s.cull;
			desc.depthStencil = s.ds;
			desc.colorFormats = colors;
			desc.blends = blends;
			desc.colorWriteMasks = masks;
			desc.depthFormat = s.depth ? Format::kD24UnormS8 : Format::kUnknown;
			desc.debugName = "pica_portals";
			auto made = m_Device->CreatePipeline( desc );
			if ( !made )
			{
				std::printf( "INFO pipeline %d refused: %u\n", int( s.pipe ),
				    unsigned( made.Error().status ) );
				return false;
			}
			m_Pipes[std::size_t( s.pipe )] = made.Value();
		}
		return true;
	}

	// -- Usage tracking -------------------------------------------------------

	void Transition( CommandEncoder &e, TextureId texture, ResourceUsage to )
	{
		ResourceUsage &state = m_States[texture.value];
		if ( state == to )
			return;
		e.TransitionTexture( texture, state, to );
		state = to;
	}

	// -- Frame plans ----------------------------------------------------------

	static Viewport Full( std::uint32_t w, std::uint32_t h )
	{
		return { 0, 0, float( w ), float( h ), 0, 1 };
	}

	void AddScene( Pass &pass, const M4 &clip, Pipe pipe, Category category,
	    const Viewport &viewport, const std::array<Plane, 9> *cone = nullptr )
	{
		const auto planes = FrustumPlanes( clip );
		for ( const Chunk &chunk : m_Scene.mesh.chunks )
		{
			if ( !Intersects( planes, chunk.bounds ) )
				continue;
			if ( cone && !IntersectsAll( *cone, chunk.bounds ) )
				continue;
			DrawCmd d;
			d.pipe = pipe;
			d.category = category;
			d.clip = clip;
			d.vertices = m_SceneVertices;
			d.indices = m_SceneIndices;
			d.first = chunk.firstIndex;
			d.count = chunk.indexCount;
			d.vertexOffset = chunk.vertexOffset;
			d.texture = m_CheckerGroup;
			d.viewport = viewport;
			pass.draws.push_back( d );
		}
	}

	DrawCmd PortalDraw( int portal, bool rim, Pipe pipe, Category category, const M4 &clip,
	    BindGroupId texture, const Viewport &viewport ) const
	{
		const PortalGeometry &g = m_Scene.geometry[std::size_t( portal )];
		DrawCmd d;
		d.pipe = pipe;
		d.category = category;
		d.clip = clip;
		d.vertices = m_PortalVertices[portal];
		d.indices = m_PortalIndices[portal];
		d.first = rim ? g.rimFirst : g.fanFirst;
		d.count = rim ? g.rimCount : g.fanCount;
		d.texture = texture;
		d.viewport = viewport;
		return d;
	}

	// Whether portal k can be seen from `eye` through `clip`.
	bool Visible( int k, V3 eye, const M4 &clip ) const
	{
		const Portal &p = m_Scene.portals[std::size_t( k )];
		if ( p.Facing( eye ) <= 0 )
			return false;
		Box box{ { 1e9f, 1e9f, 1e9f }, { -1e9f, -1e9f, -1e9f } };
		for ( V3 c : { p.LowerLeft(), p.LowerRight(), p.UpperLeft(), p.UpperRight() } )
		{
			box.lo = {
			    std::min( box.lo.x, c.x ), std::min( box.lo.y, c.y ), std::min( box.lo.z, c.z ) };
			box.hi = {
			    std::max( box.hi.x, c.x ), std::max( box.hi.y, c.y ), std::max( box.hi.z, c.z ) };
		}
		box.lo = box.lo - V3{ 1, 1, 1 };
		box.hi = box.hi + V3{ 1, 1, 1 };
		return Intersects( FrustumPlanes( clip ), box );
	}

	// The ellipse's boundary in clip space, near-clipped.
	ClipPolygon<kEllipseSegments> Outline( int k, const M4 &clip ) const
	{
		std::array<V4, kEllipseSegments> points;
		for ( int i = 0; i < kEllipseSegments; ++i )
			points[std::size_t( i )] =
			    clip * Point( m_Scene.geometry[std::size_t( k )].boundary[std::size_t( i )] );
		return ClipNear( points, points.size() );
	}

	// Portals seen inside a remote view (never the exit itself), each with its
	// interior: flat, or (recursive) the texture the cache holds.
	void AddInnerPortals( Pass &pass, int exit, V3 virtualEye, const M4 &clip, bool stencil,
	    const Viewport &viewport, bool recursive )
	{
		for ( int k = 0; k < 2; ++k )
		{
			if ( k == exit || !Visible( k, virtualEye, clip ) )
				continue;
			if ( recursive && m_Cache[k].valid )
				AddPlaneOval( pass, k, clip, m_Cache[k], Category::kRemote, viewport );
			else
				pass.draws.push_back(
				    PortalDraw( k, false, stencil ? Pipe::kTexturedStencil : Pipe::kTextured,
				        Category::kRemote, clip, m_FlatGroup[k], viewport ) );
			pass.draws.push_back( PortalDraw( k, true, stencil ? Pipe::kFlatStencil : Pipe::kFlat,
			    Category::kRemote, clip, {}, viewport ) );
		}
	}

	// A portal's ellipse showing a portal-plane texture: the fan's static
	// portal coordinates mapped into the rectangle and region the cache holds,
	// written to the frame's dynamic vertices (the portal's own indices).
	void AddPlaneOval( Pass &pass, int k, const M4 &clip, const PortalCache &cache,
	    Category category, const Viewport &viewport )
	{
		const PortalGeometry &g = m_Scene.geometry[std::size_t( k )];
		const std::uint32_t base = m_DynamicVertexCount;
		if ( base + kEllipseSegments + 1 > kDynamicVertices )
			return;
		for ( int v = 0; v <= kEllipseSegments; ++v )
		{
			Vertex out = g.vertices[std::size_t( v )];
			out.u = ( out.u - cache.u0 ) / ( cache.u1 - cache.u0 ) * cache.su;
			out.v = ( out.v - cache.v0 ) / ( cache.v1 - cache.v0 ) * cache.sv;
			std::memcpy( m_DynamicV + std::size_t( m_DynamicVertexCount++ ) * sizeof( Vertex ),
			    &out, sizeof( out ) );
		}
		DrawCmd d = PortalDraw( k, false, Pipe::kTextured, category, clip, cache.group, viewport );
		d.vertices = m_DynamicVertices;
		d.vertexOffset = std::int32_t( base );
		pass.draws.push_back( d );
	}

	int ChooseLod( const PixelRect &rect ) const
	{
		for ( int lod = 0; lod < kLods; ++lod )
			if ( kLodHeight[lod] >= std::uint32_t( rect.Height() ) &&
			     kLodWidth[lod] >= std::uint32_t( rect.Width() ) * 0.57f )
				return lod;
		return kLods - 1;
	}

	std::vector<Pass> Plan( Technique technique, const Camera &camera, FrameStats &stats )
	{
		const M4 projection =
		    Perspective( 55 * 0.01745329f, float( kShownWidth ) / kShownHeight, kNear, kFar );
		const M4 view = LookAt( camera.eye, camera.forward, { 0, 1, 0 } );
		const M4 main = projection * view;
		const Viewport screen = Full( kShownWidth, kShownHeight );
		m_DynamicVertexCount = m_DynamicIndexCount = 0;

		std::vector<Pass> passes;
		Pass mainPass;
		mainPass.color = m_MainColor;
		mainPass.depth = m_MainDepth;
		mainPass.width = kShownWidth;
		mainPass.height = kShownHeight;
		mainPass.clear = { 0.1f, 0.1f, 0.12f, 1 };
		AddScene( mainPass, main, Pipe::kScene, Category::kMain, screen );

		for ( int i = 0; i < 2; ++i )
		{
			if ( !Visible( i, camera.eye, main ) )
				continue;
			const int j = 1 - i;
			const Portal &entry = m_Scene.portals[std::size_t( i )];
			const Portal &exit = m_Scene.portals[std::size_t( j )];
			const V3 virtualEye = TransformPoint( Transfer( entry, exit ), camera.eye );
			const auto rim = [&]
			{
				mainPass.draws.push_back(
				    PortalDraw( i, true, Pipe::kFlat, Category::kPortal, main, {}, screen ) );
			};
			switch ( technique )
			{
			case Technique::kNone:
				mainPass.draws.push_back( PortalDraw(
				    i, false, Pipe::kTextured, Category::kPortal, main, m_FlatGroup[i], screen ) );
				rim();
				break;
			case Technique::kStencil:
			case Technique::kStencilCpuClear:
			case Technique::kStencilCrop:
			case Technique::kStencilCropCone:
			{
				M4 remote = RemoteClip( projection, view, entry, exit, kClipOffset );
				Viewport viewport = screen;
				const auto cone = PortalCone( camera.eye, entry, exit );
				const bool useCone = technique == Technique::kStencilCropCone;
				if ( technique == Technique::kStencilCrop || useCone )
				{
					const PixelRect r =
					    ScreenRect( Outline( i, main ), int( kShownWidth ), int( kShownHeight ) );
					if ( r.Empty() )
						break;
					remote = Crop( r.X0( kShownWidth ), r.X1( kShownWidth ), r.Y0( kShownHeight ),
					             r.Y1( kShownHeight ) ) *
					         remote;
					viewport = { float( r.left ), float( r.top ), float( r.Width() ),
					    float( r.Height() ), 0, 1 };
				}
				mainPass.draws.push_back(
				    PortalDraw( i, false, Pipe::kMask, Category::kStencil, main, {}, screen ) );
				mainPass.draws.push_back( PortalDraw( i, false, Pipe::kDepthReset,
				    Category::kStencil, AtFarPlane( main ), {}, screen ) );
				AddScene( mainPass, remote, Pipe::kSceneStencil, Category::kRemote, viewport,
				    useCone ? &cone : nullptr );
				AddInnerPortals( mainPass, j, virtualEye, remote, true, viewport, false );
				mainPass.draws.push_back(
				    PortalDraw( i, false, Pipe::kSeal, Category::kStencil, main, {}, screen ) );
				rim();
				break;
			}
			case Technique::kScreenRtt:
			{
				const auto outline = Outline( i, main );
				const PixelRect r = ScreenRect( outline, int( kShownWidth ), int( kShownHeight ) );
				if ( r.Empty() )
					break;
				const M4 remote = Crop( r.X0( kShownWidth ), r.X1( kShownWidth ),
				                      r.Y0( kShownHeight ), r.Y1( kShownHeight ) ) *
				                  RemoteClip( projection, view, entry, exit, kClipOffset );
				Pass rtt;
				rtt.color = m_ScreenColor[i];
				rtt.depth = m_ScreenDepth;
				rtt.width = std::uint32_t( r.Width() );
				rtt.height = std::uint32_t( r.Height() );
				rtt.clear = mainPass.clear;
				const Viewport area = Full( rtt.width, rtt.height );
				AddScene( rtt, remote, Pipe::kScene, Category::kRemote, area );
				AddInnerPortals( rtt, j, virtualEye, remote, false, area, false );
				passes.push_back( std::move( rtt ) );
				++stats.rttRenders;
				// The ellipse pre-projected on the CPU (w = 1), its texture
				// coordinates the pixel it covers within the rectangle.
				const std::uint32_t base = m_DynamicVertexCount;
				for ( std::size_t v = 0; v < outline.count; ++v )
				{
					const V4 &c = outline.v[v];
					const float x = c.x / c.w, y = c.y / c.w;
					const float px = ( x + 1 ) * 0.5f * kShownWidth,
					            py = ( 1 - y ) * 0.5f * kShownHeight;
					Vertex out{ x, y, c.z / c.w, ( px - float( r.left ) ) / kTargetWidth,
					    ( py - float( r.top ) ) / kTargetHeight, { 255, 255, 255, 255 } };
					std::memcpy(
					    m_DynamicV + std::size_t( m_DynamicVertexCount++ ) * sizeof( Vertex ), &out,
					    sizeof( out ) );
				}
				const std::uint32_t firstIndex = m_DynamicIndexCount;
				for ( std::size_t v = 1; v + 1 < outline.count; ++v )
					for ( std::size_t k : { std::size_t( 0 ), v, v + 1 } )
					{
						const std::uint16_t index = std::uint16_t( k );
						std::memcpy(
						    m_DynamicI + std::size_t( m_DynamicIndexCount++ ) * 2, &index, 2 );
					}
				DrawCmd d;
				d.pipe = Pipe::kTextured;
				d.category = Category::kPortal;
				d.clip = M4::Identity();
				d.vertices = m_DynamicVertices;
				d.indices = m_DynamicIndices;
				d.first = firstIndex;
				d.count = m_DynamicIndexCount - firstIndex;
				d.vertexOffset = std::int32_t( base );
				d.texture = m_ScreenGroup[i];
				d.viewport = screen;
				if ( d.count > 0 )
					mainPass.draws.push_back( d );
				rim();
				break;
			}
			case Technique::kPlaneRtt:
			case Technique::kPlaneRttCached:
			case Technique::kPlaneRttRecursive:
			case Technique::kPlaneVisible:
			case Technique::kPlaneVisibleCached:
			case Technique::kPlaneVisibleRecursive:
			case Technique::kPlaneBest:
			case Technique::kPlaneBestHalf:
			case Technique::kSeedMirror:
			{
				const bool best =
				    technique == Technique::kPlaneBest || technique == Technique::kPlaneBestHalf;
				const bool visibleOnly = best || technique == Technique::kPlaneVisible ||
				                         technique == Technique::kPlaneVisibleCached ||
				                         technique == Technique::kPlaneVisibleRecursive;
				const bool reuse = best || technique == Technique::kPlaneRttCached ||
				                   technique == Technique::kPlaneRttRecursive ||
				                   technique == Technique::kPlaneVisibleCached ||
				                   technique == Technique::kPlaneVisibleRecursive;
				const bool recursive = best || technique == Technique::kPlaneRttRecursive ||
				                       technique == Technique::kPlaneVisibleRecursive;
				// Texels per screen pixel of the portal-plane texture.
				const float quality = technique == Technique::kPlaneBestHalf ? 0.5f : 1.0f;
				// What this frame needs: the whole portal at a size (full), or
				// the visible rectangle at about one texel per pixel (visible).
				PortalCache want;
				int lod = -1;
				std::uint32_t regionW = 0, regionH = 0;
				if ( !visibleOnly )
				{
					const PixelRect r =
					    ScreenRect( Outline( i, main ), int( kShownWidth ), int( kShownHeight ) );
					if ( r.Empty() )
						break;
					lod = ChooseLod( r );
					regionW = kLodWidth[lod];
					regionH = kLodHeight[lod];
				}
				else
				{
					const VisibleRect r =
					    VisiblePortalRect( main, entry, int( kShownWidth ), int( kShownHeight ) );
					if ( !r.visible )
						break;
					want.u0 = r.u0;
					want.u1 = r.u1;
					want.v0 = r.v0;
					want.v1 = r.v1;
					regionW = std::uint32_t( std::ceil( r.pixelsWide * quality ) );
					regionH = std::uint32_t( std::ceil( r.pixelsHigh * quality ) );
				}
				regionW = std::max<std::uint32_t>( regionW, 8 );
				regionH = std::max<std::uint32_t>( regionH, 8 );
				const auto density = [&]( const PortalCache &c, float w, float h )
				{
					return std::max( w / ( ( c.u1 - c.u0 ) * entry.width ),
					    h / ( ( c.v1 - c.v0 ) * entry.height ) );
				};
				want.density = density( want, float( regionW ), float( regionH ) );
				PortalCache &cache = m_Cache[i];
				bool fresh = reuse && cache.valid &&
				             Length( camera.eye - cache.eye ) * cache.density <= kCacheTexels;
				if ( fresh && !visibleOnly )
					fresh = cache.lod == lod;
				if ( fresh && visibleOnly )
					fresh = want.u0 >= cache.u0 && want.u1 <= cache.u1 && want.v0 >= cache.v0 &&
					        want.v1 <= cache.v1 && want.density <= cache.density * 1.25f &&
					        want.density >= cache.density * 0.5f;
				if ( !fresh )
				{
					PortalCache next = want;
					if ( visibleOnly && reuse )
					{
						// A guard band of a fifth of the rectangle on each side,
						// at the same density, so that turning keeps it valid.
						const float gu = ( want.u1 - want.u0 ) * 0.2f,
						            gv = ( want.v1 - want.v0 ) * 0.2f;
						next.u0 = std::max( 0.0f, want.u0 - gu );
						next.u1 = std::min( 1.0f, want.u1 + gu );
						next.v0 = std::max( 0.0f, want.v0 - gv );
						next.v1 = std::min( 1.0f, want.v1 + gv );
						regionW = std::uint32_t( std::ceil(
						    float( regionW ) * ( next.u1 - next.u0 ) / ( want.u1 - want.u0 ) ) );
						regionH = std::uint32_t( std::ceil(
						    float( regionH ) * ( next.v1 - next.v0 ) / ( want.v1 - want.v0 ) ) );
					}
					const std::uint32_t textureW = visibleOnly ? kTargetWidth : kLodWidth[lod];
					const std::uint32_t textureH = visibleOnly ? kTargetHeight : kLodHeight[lod];
					regionW = std::min( regionW, textureW );
					regionH = std::min( regionH, textureH );
					M4 clip = PortalPlaneClip( camera.eye, entry, exit, kClipOffset, kFar );
					if ( technique == Technique::kSeedMirror )
						for ( int c = 0; c < 4; ++c )
							clip.m[0][c] = -clip.m[0][c];
					if ( visibleOnly )
						clip = CropToPortalRect( clip, next.u0, next.u1, next.v0, next.v1 );
					// Ping-pong: the previous texture stays readable for recursion.
					const int target = cache.valid && cache.lod == lod ? 1 - cache.front : 0;
					Pass rtt;
					rtt.color =
					    visibleOnly ? m_VisibleColor[i][target] : m_PlaneColor[lod][i][target];
					rtt.depth = visibleOnly ? m_ScreenDepth : m_PlaneDepth[lod];
					rtt.width = regionW;
					rtt.height = regionH;
					rtt.clear = mainPass.clear;
					const Viewport area = Full( rtt.width, rtt.height );
					const auto cone = PortalCone( camera.eye, entry, exit );
					AddScene(
					    rtt, clip, Pipe::kScene, Category::kRemote, area, best ? &cone : nullptr );
					AddInnerPortals( rtt, j, virtualEye, clip, false, area, recursive );
					passes.push_back( std::move( rtt ) );
					++stats.rttRenders;
					next.valid = true;
					next.eye = camera.eye;
					next.lod = lod;
					next.front = target;
					next.su = float( regionW ) / float( textureW );
					next.sv = float( regionH ) / float( textureH );
					next.density = density( next, float( regionW ), float( regionH ) );
					next.group =
					    visibleOnly ? m_VisibleGroup[i][target] : m_PlaneGroup[lod][i][target];
					cache = next;
				}
				AddPlaneOval( mainPass, i, main, cache, Category::kPortal, screen );
				rim();
				break;
			}
			case Technique::kCount:
				break;
			}
		}
		passes.push_back( std::move( mainPass ) );
		const bool gpuClear = technique != Technique::kStencilCpuClear;
		for ( Pass &p : passes )
		{
			p.gpuClear = gpuClear;
			if ( !gpuClear )
			{
				stats.clearedTexels += std::uint64_t( p.width ) * p.height * 2; // colour and depth
				continue;
			}
			DrawCmd clear;
			clear.pipe = Pipe::kClear;
			clear.category = Category::kClear;
			clear.clip = M4::Identity();
			clear.vertices = m_QuadVertices;
			clear.indices = m_QuadIndices;
			clear.count = 6;
			clear.viewport = Full( p.width, p.height );
			p.draws.insert( p.draws.begin(), clear );
		}
		for ( const Pass &p : passes )
		{
			for ( const DrawCmd &d : p.draws )
			{
				++stats.draws;
				stats.indices += d.count;
			}
		}
		stats.passes = std::uint32_t( passes.size() );
		if ( m_DynamicVertexCount )
		{
			pica::FlushUploadBuffer(
			    *m_Device, m_DynamicVertices, 0, m_DynamicVertexCount * sizeof( Vertex ) );
			pica::FlushUploadBuffer( *m_Device, m_DynamicIndices, 0, m_DynamicIndexCount * 2 );
		}
		return passes;
	}

	// The clear alone: an empty main pass cleared by the load op (the
	// adapter's CPU fill) or by the quad, median of 16 submissions each, at
	// three sizes: what grows with the area is the fill, the rest is fixed.
	void ClearBenchmark()
	{
		const std::uint32_t sizes[][2] = { { 8, 8 }, { 128, 128 }, { kShownWidth, kShownHeight } };
		for ( const auto &size : sizes )
			for ( bool gpu : { false, true } )
			{
				std::vector<float> times;
				for ( int k = 0; k < 16; ++k )
				{
					Pass pass;
					pass.color = m_MainColor;
					pass.depth = m_MainDepth;
					pass.width = size[0];
					pass.height = size[1];
					pass.gpuClear = gpu;
					if ( gpu )
					{
						DrawCmd clear;
						clear.pipe = Pipe::kClear;
						clear.category = Category::kClear;
						clear.clip = M4::Identity();
						clear.vertices = m_QuadVertices;
						clear.indices = m_QuadIndices;
						clear.count = 6;
						clear.viewport = Full( size[0], size[1] );
						pass.draws.push_back( clear );
					}
					FrameStats stats;
					const std::vector<Pass> passes = { pass };
					if ( Execute( passes, stats ) )
						times.push_back( stats.submitUs );
				}
				std::sort( times.begin(), times.end() );
				std::printf( "INFO clear benchmark %s %ux%u: median submit %.0f us, min %.0f, max "
				             "%.0f\n",
				    gpu ? "gpu-quad" : "cpu-fill", unsigned( size[0] ), unsigned( size[1] ),
				    times.empty() ? 0.0 : times[times.size() / 2],
				    times.empty() ? 0.0 : times.front(), times.empty() ? 0.0 : times.back() );
			}
	}

	void ResetCaches()
	{
		for ( PortalCache &c : m_Cache )
			c = {};
	}

	// -- Execution ------------------------------------------------------------

	void Record(
	    CommandEncoder &e, const DrawCmd &d, Pipe pipe, PipelineId &bound, BindGroupId &group )
	{
		const PipelineId pipeline = m_Pipes[std::size_t( pipe )];
		if ( pipeline != bound )
		{
			e.SetPipeline( pipeline );
			bound = pipeline;
			group = {};
		}
		e.SetDrawConstants( 0, std::as_bytes( std::span( &d.clip.m[0][0], 16 ) ) );
		const bool textured = pipe == Pipe::kScene || pipe == Pipe::kSceneStencil ||
		                      pipe == Pipe::kTextured || pipe == Pipe::kTexturedStencil;
		if ( textured && d.texture != group )
		{
			e.SetBindGroup( BindGroupRole::kMaterial, d.texture );
			group = d.texture;
		}
		e.SetViewport( d.viewport );
		e.SetVertexBuffer( 0, d.vertices, 0 );
		e.SetIndexBuffer( d.indices, 0, IndexFormat::kUint16 );
		e.DrawIndexed( d.count, 1, d.first, d.vertexOffset );
	}

	bool Execute( const std::vector<Pass> &passes, FrameStats &stats )
	{
		const u64 start = svcGetSystemTick();
		auto encoder = m_Device->BeginEncoder( QueueKind::kGraphics );
		if ( !encoder )
			return false;
		CommandEncoder &e = encoder.Value();
		for ( const Pass &p : passes )
		{
			Transition( e, p.color, ResourceUsage::kColorAttachment );
			Transition( e, p.depth, ResourceUsage::kDepthWrite );
			const LoadOp load = p.gpuClear ? LoadOp::kDiscard : LoadOp::kClear;
			const ColorAttachment color[] = { { p.color, load, StoreOp::kStore, p.clear, {} } };
			RenderingDesc rendering;
			rendering.colors = color;
			rendering.depth = DepthAttachment{ p.depth, load, StoreOp::kStore, 1.0f };
			rendering.width = p.width;
			rendering.height = p.height;
			e.BeginRendering( rendering );
			PipelineId bound;
			BindGroupId group;
			for ( const DrawCmd &d : p.draws )
				Record( e, d, d.pipe, bound, group );
			e.EndRendering();
			Transition( e, p.color, ResourceUsage::kSampled );
		}
		const u64 recorded = svcGetSystemTick();
		CommandEncoder list[] = { std::move( e ) };
		auto token = m_Device->Submit( QueueKind::kGraphics, list, {} );
		const u64 submitted = svcGetSystemTick();
		stats.recordUs += float( recorded - start ) / kTicksPerMicrosecond;
		stats.submitUs = float( submitted - recorded ) / kTicksPerMicrosecond;
		if ( !token )
		{
			std::printf( "INFO submission refused: %u\n", unsigned( token.Error().status ) );
			return false;
		}
		return true;
	}

	// Rasterized fragments per category: every draw again, additive 1/255,
	// no depth or stencil test, into the cleared scratch target.
	void Count( const std::vector<Pass> &passes, FrameStats &stats )
	{
		for ( std::size_t c = 0; c < std::size_t( Category::kCount ); ++c )
		{
			bool any = false;
			for ( const Pass &p : passes )
				for ( const DrawCmd &d : p.draws )
					any = any || std::size_t( d.category ) == c;
			if ( !any )
				continue;
			auto encoder = m_Device->BeginEncoder( QueueKind::kGraphics );
			CommandEncoder &e = encoder.Value();
			Transition( e, m_CountTarget, ResourceUsage::kColorAttachment );
			const ColorAttachment color[] = {
			    { m_CountTarget, LoadOp::kClear, StoreOp::kStore, {}, {} } };
			RenderingDesc rendering;
			rendering.colors = color;
			rendering.width = kTargetWidth;
			rendering.height = kTargetHeight;
			e.BeginRendering( rendering );
			PipelineId bound;
			BindGroupId group;
			for ( const Pass &p : passes )
				for ( const DrawCmd &d : p.draws )
					if ( std::size_t( d.category ) == c )
						Record( e, d,
						    d.pipe == Pipe::kScene || d.pipe == Pipe::kSceneStencil
						        ? Pipe::kCountBack
						        : Pipe::kCountNone,
						    bound, group );
			e.EndRendering();
			Transition( e, m_CountTarget, ResourceUsage::kCopySource );
			e.TransitionBuffer(
			    m_Readback, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
			e.CopyTextureToBuffer(
			    m_CountTarget, m_Readback, { 0, 0, 0, kTargetWidth, kTargetHeight } );
			CommandEncoder list[] = { std::move( e ) };
			if ( !m_Device->Submit( QueueKind::kGraphics, list, {} ) )
				continue;
			(void)m_Device->WaitIdle();
			m_Pixels.resize( std::size_t( kTargetWidth ) * kTargetHeight * 4 );
			if ( !m_Device->ReadBuffer(
			         m_Readback, 0, std::as_writable_bytes( std::span( m_Pixels ) ) ) )
				continue;
			std::uint64_t sum = 0, saturated = 0;
			for ( std::size_t t = 0; t < m_Pixels.size(); t += 4 )
			{
				sum += m_Pixels[t];
				saturated += m_Pixels[t] == 255;
			}
			if ( saturated )
				std::printf(
				    "INFO count saturated on %llu texels\n", (unsigned long long)saturated );
			stats.fragments[c] = sum;
		}
		stats.counted = true;
	}

	// The main target's shown region, RGB, row 0 at the top.
	std::vector<std::uint8_t> ReadMain()
	{
		auto encoder = m_Device->BeginEncoder( QueueKind::kGraphics );
		CommandEncoder &e = encoder.Value();
		Transition( e, m_MainColor, ResourceUsage::kCopySource );
		e.TransitionBuffer(
		    m_Readback, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		e.CopyTextureToBuffer( m_MainColor, m_Readback, { 0, 0, 0, kShownWidth, kShownHeight } );
		Transition( e, m_MainColor, ResourceUsage::kSampled );
		CommandEncoder list[] = { std::move( e ) };
		std::vector<std::uint8_t> rgb;
		if ( !m_Device->Submit( QueueKind::kGraphics, list, {} ) )
			return rgb;
		(void)m_Device->WaitIdle();
		m_Pixels.resize( std::size_t( kShownWidth ) * kShownHeight * 4 );
		if ( !m_Device->ReadBuffer(
		         m_Readback, 0, std::as_writable_bytes( std::span( m_Pixels ) ) ) )
			return rgb;
		rgb.resize( std::size_t( kShownWidth ) * kShownHeight * 3 );
		for ( std::size_t t = 0; t < std::size_t( kShownWidth ) * kShownHeight; ++t )
			for ( int c = 0; c < 3; ++c )
				rgb[t * 3 + std::size_t( c )] = m_Pixels[t * 4 + std::size_t( c )];
		return rgb;
	}

	void Present()
	{
		(void)pica::PresentTopScreen( *m_Device, m_MainColor, kShownWidth, kShownHeight );
	}

private:
	static constexpr std::uint32_t kDynamicVertices = 1024, kDynamicIndices = 2048;

	std::unique_ptr<IRenderDevice2> m_Device;
	Scene m_Scene;
	BufferId m_SceneVertices, m_SceneIndices, m_DynamicVertices, m_DynamicIndices, m_Readback;
	BufferId m_PortalVertices[2], m_PortalIndices[2], m_QuadVertices, m_QuadIndices;
	std::byte *m_DynamicV = nullptr, *m_DynamicI = nullptr;
	std::uint32_t m_DynamicVertexCount = 0, m_DynamicIndexCount = 0;
	BindGroupLayoutId m_MaterialLayout;
	SamplerId m_Repeat, m_ClampLinear, m_ClampNearest;
	TextureId m_Checker, m_Flat[2], m_MainColor, m_MainDepth, m_CountTarget;
	TextureId m_ScreenColor[2], m_ScreenDepth, m_VisibleColor[2][2];
	BindGroupId m_VisibleGroup[2][2];
	TextureId m_PlaneColor[kLods][2][2], m_PlaneDepth[kLods];
	BindGroupId m_CheckerGroup, m_FlatGroup[2], m_ScreenGroup[2], m_PlaneGroup[kLods][2][2];
	std::vector<std::byte> m_VertexArtifact, m_Fragments[3];
	std::array<PipelineId, std::size_t( Pipe::kCount )> m_Pipes{};
	std::map<std::uint64_t, ResourceUsage> m_States;
	std::array<PortalCache, 2> m_Cache;
	std::vector<std::uint8_t> m_Pixels;
};

void WritePpm( const std::string &path, const std::vector<std::uint8_t> &rgb )
{
	if ( std::FILE *f = std::fopen( path.c_str(), "wb" ) )
	{
		std::fprintf( f, "P6\n%u %u\n255\n", unsigned( kShownWidth ), unsigned( kShownHeight ) );
		std::fwrite( rgb.data(), 1, rgb.size(), f );
		std::fclose( f );
	}
}

struct ImageError
{
	double mean = 0;     // mean absolute difference per channel, 0..255
	double badShare = 0; // share of pixels with a channel off by more than 48
	// The mean after a 4 x 4 box filter of both images: what is left once
	// sub-texel resampling (the blur any texture-based portal adds when it
	// samples its texture again) is filtered out; geometric errors remain.
	double lowPass = 0;
};

ImageError Compare( const std::vector<std::uint8_t> &a, const std::vector<std::uint8_t> &b )
{
	ImageError e;
	if ( a.size() != b.size() || a.empty() )
		return { 255, 1 };
	std::uint64_t sum = 0, bad = 0;
	for ( std::size_t p = 0; p < a.size(); p += 3 )
	{
		int worst = 0;
		for ( int c = 0; c < 3; ++c )
		{
			const int d =
			    std::abs( int( a[p + std::size_t( c )] ) - int( b[p + std::size_t( c )] ) );
			sum += std::uint64_t( d );
			worst = std::max( worst, d );
		}
		bad += worst > 48;
	}
	e.mean = double( sum ) / double( a.size() );
	e.badShare = double( bad ) / double( a.size() / 3 );
	std::uint64_t low = 0;
	for ( std::uint32_t by = 0; by < kShownHeight / 4; ++by )
		for ( std::uint32_t bx = 0; bx < kShownWidth / 4; ++bx )
			for ( int c = 0; c < 3; ++c )
			{
				int sa = 0, sb = 0;
				for ( std::uint32_t j = 0; j < 4; ++j )
					for ( std::uint32_t i = 0; i < 4; ++i )
					{
						const std::size_t at =
						    ( std::size_t( by * 4 + j ) * kShownWidth + bx * 4 + i ) * 3 +
						    std::size_t( c );
						sa += a[at];
						sb += b[at];
					}
				low += std::uint64_t( std::abs( sa - sb ) );
			}
	e.lowPass = double( low ) / 16.0 / double( a.size() / 16 );
	return e;
}

struct Summary
{
	int frames = 0, counted = 0;
	double recordUs = 0, submitUs = 0, worstSubmitUs = 0;
	double draws = 0, indices = 0, rttRenders = 0, cleared = 0;
	std::array<double, std::size_t( Category::kCount )> fragments{};
};

} // namespace

int main()
{
	gfxInitDefault();
	mkdir( "sdmc:/portal_lab", 0777 );
	g_report = std::freopen( "sdmc:/portal_lab/report.txt", "w", stdout );
	std::setvbuf( stdout, nullptr, _IONBF, 0 );
	std::FILE *csv = std::fopen( "sdmc:/portal_lab/frames.csv", "w" );
	if ( csv )
		std::fprintf( csv, "technique,segment,frame,record_us,submit_us,passes,draws,indices,"
		                   "rtt_renders,cleared_texels,frags_main,frags_remote,frags_stencil,"
		                   "frags_portal,frags_clear\n" );

	Lab lab;
	if ( !lab.Init() )
	{
		std::printf( "INFO lab initialization failed\n" );
		lab.checks.That( false, "pica_portals.lab the device, scene and pipelines are made" );
	}
	else
	{
		const Scene &scene = lab.GetScene();
		std::printf( "INFO scene %zu vertices, %u triangles, %zu chunks; %d frames per segment\n",
		    scene.mesh.vertices.size(), unsigned( scene.mesh.triangles ), scene.mesh.chunks.size(),
		    PORTAL_LAB_FRAMES );
		lab.ClearBenchmark();
		const int frames = PORTAL_LAB_FRAMES;
		const int gallery = frames / 2; // the middle frame of each segment
		std::map<int, std::vector<std::uint8_t>> reference;
		std::array<std::array<Summary, kSegmentCount>, std::size_t( Technique::kCount )> summary{};
		std::array<std::vector<ImageError>, std::size_t( Technique::kCount )> errors;
		for ( int t = 0; t < int( Technique::kCount ); ++t )
		{
			const Technique technique = Technique( t );
			lab.ResetCaches();
			std::printf( "INFO technique %s\n", Name( technique ) );
			for ( int s = 0; s < kSegmentCount; ++s )
				for ( int f = 0; f < frames; ++f )
				{
					// The seeded defect runs only where it is judged.
					if ( technique == Technique::kSeedMirror && f != gallery )
						continue;
					const Camera camera = PathCamera( scene, s, float( f ) / float( frames ) );
					FrameStats stats;
					const u64 start = svcGetSystemTick();
					const std::vector<Pass> passes = lab.Plan( technique, camera, stats );
					stats.recordUs = float( svcGetSystemTick() - start ) / kTicksPerMicrosecond;
					if ( !lab.Execute( passes, stats ) )
					{
						lab.checks.That( false, std::string( "pica_portals.lab " ) +
						                            Name( technique ) +
						                            " frames are accepted by the device" );
						continue;
					}
					const int frameId = s * frames + f;
					if ( f == gallery )
					{
						const std::vector<std::uint8_t> image = lab.ReadMain();
						char path[96];
						std::snprintf( path, sizeof( path ), "sdmc:/portal_lab/%s_%d.ppm",
						    Name( technique ), frameId );
						WritePpm( path, image );
						if ( technique == Technique::kStencil )
							reference[frameId] = image;
						else
						{
							const ImageError e = Compare( image, reference[frameId] );
							errors[std::size_t( t )].push_back( e );
							std::printf(
							    "INFO image %s %s: mean %.3f, low-pass %.3f, %.3f%% pixels off "
							    "by > 48\n",
							    Name( technique ), kSegments[s].name.data(), e.mean, e.lowPass,
							    e.badShare * 100 );
						}
					}
					if ( f % 4 == 0 || f == gallery )
						lab.Count( passes, stats );
					lab.Present();
					Summary &sum = summary[std::size_t( t )][std::size_t( s )];
					++sum.frames;
					sum.recordUs += stats.recordUs;
					sum.submitUs += stats.submitUs;
					sum.worstSubmitUs = std::max( sum.worstSubmitUs, double( stats.submitUs ) );
					sum.draws += stats.draws;
					sum.indices += stats.indices;
					sum.rttRenders += stats.rttRenders;
					sum.cleared += double( stats.clearedTexels );
					if ( stats.counted )
					{
						++sum.counted;
						for ( std::size_t c = 0; c < stats.fragments.size(); ++c )
							sum.fragments[c] += double( stats.fragments[c] );
					}
					if ( csv )
					{
						std::fprintf( csv, "%s,%s,%d,%.1f,%.1f,%u,%u,%u,%u,%llu", Name( technique ),
						    kSegments[s].name.data(), frameId, stats.recordUs, stats.submitUs,
						    unsigned( stats.passes ), unsigned( stats.draws ),
						    unsigned( stats.indices ), unsigned( stats.rttRenders ),
						    (unsigned long long)stats.clearedTexels );
						for ( std::size_t c = 0; c < stats.fragments.size(); ++c )
							std::fprintf( csv, ",%lld",
							    stats.counted ? (long long)stats.fragments[c] : -1LL );
						std::fprintf( csv, "\n" );
					}
				}
		}

		// The summary: per technique and segment, then the whole path.
		std::printf( "SUMMARY technique segment frames record_us submit_us worst_submit_us draws "
		             "indices rtt_renders cleared_ktexels frags_main frags_remote frags_stencil "
		             "frags_portal frags_clear frags_total\n" );
		for ( int t = 0; t < int( Technique::kCount ); ++t )
		{
			Summary all;
			for ( int s = 0; s <= kSegmentCount; ++s )
			{
				const Summary &x =
				    s < kSegmentCount ? summary[std::size_t( t )][std::size_t( s )] : all;
				if ( s < kSegmentCount )
				{
					all.frames += x.frames;
					all.counted += x.counted;
					all.recordUs += x.recordUs;
					all.submitUs += x.submitUs;
					all.worstSubmitUs = std::max( all.worstSubmitUs, x.worstSubmitUs );
					all.draws += x.draws;
					all.indices += x.indices;
					all.rttRenders += x.rttRenders;
					all.cleared += x.cleared;
					for ( std::size_t c = 0; c < x.fragments.size(); ++c )
						all.fragments[c] += x.fragments[c];
				}
				if ( x.frames == 0 )
					continue;
				const double n = x.frames, k = std::max( 1, x.counted );
				double total = 0;
				for ( double v : x.fragments )
					total += v / k;
				std::printf( "SUMMARY %s %s %d %.0f %.0f %.0f %.1f %.0f %.2f %.1f %.0f %.0f %.0f "
				             "%.0f %.0f %.0f\n",
				    Name( Technique( t ) ), s < kSegmentCount ? kSegments[s].name.data() : "all",
				    x.frames, x.recordUs / n, x.submitUs / n, x.worstSubmitUs, x.draws / n,
				    x.indices / n, x.rttRenders / n, x.cleared / n / 1000, x.fragments[0] / k,
				    x.fragments[1] / k, x.fragments[2] / k, x.fragments[3] / k, x.fragments[4] / k,
				    total );
			}
		}

		// Verdicts. Exact techniques match the stencil image; the portal-plane
		// ones within resampling; the controls must not.
		auto worst = [&]( Technique t )
		{
			ImageError w;
			for ( const ImageError &e : errors[std::size_t( t )] )
			{
				w.mean = std::max( w.mean, e.mean );
				w.badShare = std::max( w.badShare, e.badShare );
				w.lowPass = std::max( w.lowPass, e.lowPass );
			}
			return w;
		};
		const std::size_t gallerySize = std::size_t( kSegmentCount );
		for ( Technique t : { Technique::kStencilCpuClear, Technique::kStencilCrop,
		          Technique::kStencilCropCone, Technique::kScreenRtt } )
		{
			const ImageError w = worst( t );
			lab.checks.That( errors[std::size_t( t )].size() == gallerySize && w.mean < 1.0 &&
			                     w.badShare < 0.005,
			    std::string( "pica_portals.lab " ) + Name( t ) +
			        " matches the stencil image (mean < 1, < 0.5% of pixels off by > 48)" );
		}
		// Texture-based portals resample: judged after the low-pass (the raw
		// mean is printed per image). The thresholds hold the controls out.
		for ( Technique t : { Technique::kPlaneVisible, Technique::kPlaneVisibleCached } )
		{
			const ImageError w = worst( t );
			lab.checks.That( errors[std::size_t( t )].size() == gallerySize && w.lowPass < 1.5 &&
			                     w.badShare < 0.03,
			    std::string( "pica_portals.lab " ) + Name( t ) +
			        " matches the stencil image within resampling (low-pass mean < 1.5, < 3% off "
			        "by > 48)" );
		}
		for ( Technique t : { Technique::kNone, Technique::kSeedMirror } )
		{
			const ImageError w = worst( t );
			lab.checks.That( w.badShare >= 0.01 && w.lowPass >= 3.0,
			    std::string( "pica_portals.lab control " ) + Name( t ) +
			        " differs from the stencil image (>= 1% of pixels off by > 48, low-pass "
			        "mean >= 3)" );
		}
		for ( Technique t :
		    { Technique::kPlaneRttCached, Technique::kPlaneVisibleCached, Technique::kPlaneBest } )
		{
			const Summary &turn = summary[std::size_t( t )][1]; // segment "turn"
			lab.checks.That( turn.frames > 0 && turn.rttRenders <= 2,
			    std::string( "pica_portals.lab " ) + Name( t ) +
			        " renders the portal at most twice while the eye only turns" );
		}
	}

	const int result = lab.checks.Report();
	if ( csv )
		std::fclose( csv );
	std::fflush( stdout );
	if ( g_report )
		std::fclose( g_report );
	if ( std::FILE *done = std::fopen( "sdmc:/portal_lab/portal_lab.done", "w" ) )
	{
		std::fprintf( done, "%d\n", result );
		std::fclose( done );
	}
	gfxExit();
	return result;
}
