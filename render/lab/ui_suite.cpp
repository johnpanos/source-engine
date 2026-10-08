//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab suite ui (RFC 0016 K8 UI cohort; render.ui-draw-list.v1
//			drawn by render.pass.world's dynamic draws, UiListView): the
//			screen UI's draw list drawn by the core, judged by oracles that
//			share no code with the pass or its shaders. Each restates the
//			legacy 2D path (vertexlitgeneric_dx9_helper.cpp's UnlitGeneric:
//			the base read through sRGB, the vertex color GammaToLinear, pow
//			2.2, written to a linear target):
//			- placement: a command's pixels are exactly those whose centers
//			  lie inside its corners after the list's scale and offset (no
//			  corner sits on a pixel center), inside the list's viewport;
//			- composition: opaque, $vertexalpha (alpha), $additive, and
//			  $additive with $translucent (alpha-additive) against the
//			  background in linear light, and a texture's texels (point
//			  sampled) modulated by the vertex color;
//			- order: overlapping commands draw in list order;
//			- the list's materials: a command draws its own material;
//			- a malformed list is refused whole, by name.
//			Each judgment also runs against a bad producer (the list
//			reordered, its offset, its viewport or its materials ignored),
//			which the suite requires to fail.
//
//=============================================================================//

#include "lab_canvas.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "render/pass/world/world_pass.h"
#include "render/ui_draw_list.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace render::lab
{

namespace
{

using namespace render::device;
using namespace render::pass::world;
using ui_draw_list::Command;
using ui_draw_list::Vertex;

constexpr std::uint32_t kSize = 64;
constexpr float kBackground[4] = { 0.2f, 0.4f, 0.6f, 0.5f };
constexpr float kTolerance = 0.004f; // RGBA16F against exact arithmetic

// The textures: 1 is white; 2 the 2 x 2 checker (top-left, top-right,
// bottom-left, bottom-right), point sampled.
constexpr std::uint8_t kTexels[4][4] = {
    { 255, 0, 0, 64 }, { 0, 255, 0, 255 }, { 0, 0, 255, 128 }, { 255, 255, 255, 0 } };

class UiTextures final : public IWorldTextures
{
public:
	TextureId white[2], checker[2]; // [decoded]
	TextureId Import( int handle, bool decoded ) override
	{
		return handle == 1 ? white[decoded] : handle == 2 ? checker[decoded] : TextureId();
	}
	SamplerDesc Sampler( int ) override
	{
		SamplerDesc desc;
		desc.minFilter = desc.magFilter = desc.mipFilter = Filter::kNearest;
		desc.address = AddressMode::kClampToEdge;
		return desc;
	}
};

struct Rgba
{
	double c[4];
};

double SrgbDecode( double v )
{
	return v <= 0.04045 ? v / 12.92 : std::pow( ( v + 0.055 ) / 1.055, 2.4 );
}

// The legacy source color: the base (sRGB decoded) times the vertex color
// (pow 2.2) when $vertexcolor; alpha the base's times the vertex alpha when
// $vertexalpha.
Rgba Source( const std::uint8_t texel[4], const std::uint8_t color[4], bool vertexAlpha )
{
	Rgba out;
	for ( int c = 0; c < 3; ++c )
		out.c[c] = SrgbDecode( texel[c] / 255.0 ) * std::pow( color[c] / 255.0, 2.2 );
	out.c[3] = texel[3] / 255.0 * ( vertexAlpha ? color[3] / 255.0 : 1.0 );
	return out;
}

enum class Blend
{
	kOpaque,
	kAlpha,
	kAdditive,
	kAlphaAdditive
};

Rgba Over( const Rgba &source, Blend blend )
{
	Rgba out = source;
	for ( int c = 0; c < 3; ++c )
	{
		const double b = kBackground[c], s = source.c[c], a = source.c[3];
		out.c[c] = blend == Blend::kOpaque     ? s
		           : blend == Blend::kAlpha    ? s * a + b * ( 1.0 - a )
		           : blend == Blend::kAdditive ? b + s
		                                       : b + s * a;
	}
	return out;
}

Rgba Background()
{
	return { { kBackground[0], kBackground[1], kBackground[2], kBackground[3] } };
}

bool Near( const float *pixel, const Rgba &expected )
{
	for ( int c = 0; c < 3; ++c )
	{
		if ( !( std::abs( pixel[c] - expected.c[c] ) <= kTolerance ) )
			return false;
	}
	return true;
}

// Whether exactly the pixels whose centers lie in [x0, x1) x [y0, y1) show
// `inside`, and every other pixel the background.
bool Covers( const CanvasImage &image, float x0, float y0, float x1, float y1, const Rgba &inside )
{
	for ( std::uint32_t y = 0; y < image.height; ++y )
	{
		for ( std::uint32_t x = 0; x < image.width; ++x )
		{
			const float cx = x + 0.5f, cy = y + 0.5f;
			const bool in = cx >= x0 && cx < x1 && cy >= y0 && cy < y1;
			if ( !Near( image.At( x, y ), in ? inside : Background() ) )
				return false;
		}
	}
	return true;
}

WorldMaterial Material(
    std::string name, int texture, std::vector<std::pair<std::string, std::string>> extra )
{
	WorldMaterial material;
	material.name = std::move( name );
	material.shader = "UnlitGeneric";
	material.variables = { { "$basetexture", texture == 2 ? "ui/checker" : "ui/white" },
	    { "$vertexcolor", "1" }, { "$ignorez", "1" } };
	for ( auto &pair : extra )
		material.variables.push_back( std::move( pair ) );
	material.textures = { { "$basetexture", texture } };
	return material;
}

// A list being built: its vertices, commands and materials.
struct UiList
{
	float scale = 1.0f;
	float offset[2] = { 0.5f, 0.5f }; // the surface's default pixel offset (0.5)
	int viewport[4] = { 0, 0, int( kSize ), int( kSize ) };
	std::vector<Vertex> vertices;
	std::vector<Command> commands;
	std::vector<WorldMaterial> materials;

	void Quad( float x0, float y0, float x1, float y1, const std::uint8_t color[4],
	    std::uint32_t material )
	{
		Command command = { std::uint32_t( vertices.size() ), 6, material };
		const float xs[4] = { x0, x1, x1, x0 };
		const float ys[4] = { y0, y0, y1, y1 };
		const float ss[4] = { 0, 1, 1, 0 };
		const float ts[4] = { 0, 0, 1, 1 };
		for ( int index : { 0, 1, 2, 0, 2, 3 } )
		{
			Vertex v = { xs[index], ys[index], ss[index], ts[index], {} };
			std::copy_n( color, 4, v.color );
			vertices.push_back( v );
		}
		commands.push_back( command );
	}

	ui_draw_list::ListView View() const
	{
		return { scale, { offset[0], offset[1] },
		    { viewport[0], viewport[1], viewport[2], viewport[3] }, vertices.data(),
		    std::uint32_t( vertices.size() ), commands.data(), std::uint32_t( commands.size() ),
		    std::uint32_t( materials.size() ) };
	}
};

std::optional<std::string> RunChecks(
    bool validate, std::span<const std::uint32_t>, Results &results, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( auto why = CreateLabDevice( validate, counter, device ) )
		return why;
	resources::TextureCache textures( *device );
	material::GroupResidency groups( *device, textures );
	std::unique_ptr<Canvas> canvas;
	if ( auto why = Canvas::Create( *device, kSize, kSize, canvas ) )
		return why;
	UiTextures imports;
	for ( bool decoded : { false, true } )
	{
		TextureDesc desc;
		desc.format = decoded ? Format::kRGBA8Srgb : Format::kRGBA8Unorm;
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
		desc.width = desc.height = 1;
		const std::byte white[4] = {
		    std::byte( 255 ), std::byte( 255 ), std::byte( 255 ), std::byte( 255 ) };
		auto w = textures.Stage( decoded ? "ui/white/srgb" : "ui/white/unorm", desc, white );
		desc.width = desc.height = 2;
		std::byte checker[16];
		for ( int i = 0; i < 16; ++i )
			checker[i] = std::byte( kTexels[i / 4][i % 4] );
		auto c = textures.Stage( decoded ? "ui/checker/srgb" : "ui/checker/unorm", desc, checker );
		if ( !w || !c )
			return "the UI fixture textures did not stage";
		imports.white[decoded] = w.Value().texture;
		imports.checker[decoded] = c.Value().texture;
	}

	WorldPass pass;
	pass.SetWorld( WorldData() );
	std::uint64_t frame = 0;
	// Converts and draws one list over a cleared canvas; the refusal text
	// when the list or the pass refuses it.
	auto draw = [&]( const UiList &list, CanvasImage &image ) -> std::optional<std::string>
	{
		WorldView view;
		if ( auto why = UiListView( list.View(), list.materials, view ) )
			return "refused: " + *why;
		const std::uint32_t tag = pass.QueueView( std::move( view ) );
		if ( !tag )
			return "refused: " + pass.Stats().lastRefusal;
		CanvasPost post = [&]( CommandEncoder &encoder, TextureId color,
		                      TextureId depth ) -> std::optional<std::string>
		{
			WorldTarget target;
			target.device = device.get();
			target.color = color;
			target.depth = depth;
			target.colorFormat = kCanvasColor;
			target.depthFormat = kCanvasDepth;
			target.width = target.height = kSize;
			target.textures = &imports;
			target.frame = ++frame;
			pass.Record( tag, encoder, target );
			return std::nullopt;
		};
		return canvas->Render( textures, groups, {},
		    { kBackground[0], kBackground[1], kBackground[2], kBackground[3] }, &image, post );
	};
	auto mustDraw = [&]( const UiList &list, CanvasImage &image ) -> std::optional<std::string>
	{
		if ( auto why = draw( list, image ) )
			return "a well-formed UI list did not draw: " + *why;
		return std::nullopt;
	};

	const std::uint8_t kWhite[4] = { 255, 255, 255, 255 };
	const std::uint8_t kColor[4] = { 200, 100, 50, 128 };
	const Rgba white = { { 1.0, 1.0, 1.0, 1.0 } };

	// Placement: scale 1.5, the surface's pixel offset and a further quarter
	// pixel; the same list with its offset ignored (a bad producer) misses.
	{
		UiList list;
		list.materials.push_back( Material( "ui/opaque", 1, {} ) );
		list.scale = 1.5f;
		list.offset[0] = 0.75f;
		list.offset[1] = 0.25f;
		list.Quad( 4, 6, 20, 18, kWhite, 0 );
		const float x0 = 4 * 1.5f + 0.75f, x1 = 20 * 1.5f + 0.75f;
		const float y0 = 6 * 1.5f + 0.25f, y1 = 18 * 1.5f + 0.25f;
		UiList ignored = list;
		ignored.scale = 1.0f;
		CanvasImage image, bad;
		if ( auto why = mustDraw( list, image ) )
			return why;
		if ( auto why = mustDraw( ignored, bad ) )
			return why;
		results.That( Covers( image, x0, y0, x1, y1, white ), "ui.placement.scale-offset" );
		results.That(
		    !Covers( bad, x0, y0, x1, y1, white ), "ui.placement.control-ignored-scale-fails" );
	}

	// Viewport: the list's viewport places its pixels.
	{
		UiList list;
		list.materials.push_back( Material( "ui/opaque", 1, {} ) );
		list.viewport[0] = 16;
		list.viewport[1] = 8;
		list.viewport[2] = 32;
		list.viewport[3] = 32;
		list.Quad( 2, 3, 10, 9, kWhite, 0 );
		UiList ignored = list;
		ignored.viewport[0] = ignored.viewport[1] = 0;
		ignored.viewport[2] = ignored.viewport[3] = int( kSize );
		CanvasImage image, bad;
		if ( auto why = mustDraw( list, image ) )
			return why;
		if ( auto why = mustDraw( ignored, bad ) )
			return why;
		results.That( Covers( image, 18.5f, 11.5f, 26.5f, 17.5f, white ), "ui.placement.viewport" );
		results.That( !Covers( bad, 18.5f, 11.5f, 26.5f, 17.5f, white ),
		    "ui.placement.control-ignored-viewport-fails" );
	}

	// Composition: the white base times the vertex color, by each blend.
	struct BlendCase
	{
		const char *name;
		Blend blend;
		bool vertexAlpha;
		std::vector<std::pair<std::string, std::string>> extra;
	};
	const BlendCase kBlends[] = { { "opaque", Blend::kOpaque, false, {} },
	    { "alpha", Blend::kAlpha, true, { { "$vertexalpha", "1" } } },
	    { "additive", Blend::kAdditive, false, { { "$additive", "1" } } },
	    { "alpha-additive", Blend::kAlphaAdditive, true,
	        { { "$additive", "1" }, { "$translucent", "1" }, { "$vertexalpha", "1" } } } };
	for ( const BlendCase &blend : kBlends )
	{
		UiList list;
		list.materials.push_back( Material( std::string( "ui/" ) + blend.name, 1, blend.extra ) );
		list.Quad( 8, 8, 40, 40, kColor, 0 );
		CanvasImage image;
		if ( auto why = mustDraw( list, image ) )
			return why;
		const Rgba expected = Over( Source( kWhite, kColor, blend.vertexAlpha ), blend.blend );
		results.That( Covers( image, 8.5f, 8.5f, 40.5f, 40.5f, expected ),
		    std::string( "ui.blend." ) + blend.name );
	}

	// Texture: the checker's texels times the vertex color, alpha blended
	// ($translucent with $vertexalpha: the texel's alpha times the vertex's).
	{
		const std::uint8_t tint[4] = { 255, 128, 255, 192 };
		UiList list;
		list.materials.push_back(
		    Material( "ui/checker", 2, { { "$vertexalpha", "1" }, { "$translucent", "1" } } ) );
		list.Quad( 16, 16, 48, 48, tint, 0 );
		CanvasImage image;
		if ( auto why = mustDraw( list, image ) )
			return why;
		bool ok = true;
		for ( int q = 0; q < 4; ++q )
		{
			const Rgba expected = Over( Source( kTexels[q], tint, true ), Blend::kAlpha );
			const std::uint32_t x0 = 16 + ( q % 2 ) * 16, y0 = 16 + ( q / 2 ) * 16;
			for ( std::uint32_t y = y0 + 1; y + 1 < y0 + 16; ++y )
				for ( std::uint32_t x = x0 + 1; x + 1 < x0 + 16; ++x )
					ok = ok && Near( image.At( x, y ), expected );
		}
		results.That( ok, "ui.texture.texels" );
	}

	// Order and materials: blue drawn after red wins where they overlap, each
	// with its own material; the reordered list and the list with one
	// material for both (bad producers) do not.
	{
		const std::uint8_t red[4] = { 255, 0, 0, 255 }, blue[4] = { 0, 0, 255, 255 };
		UiList list;
		list.materials.push_back( Material( "ui/opaque", 1, {} ) );
		list.materials.push_back( Material( "ui/additive", 1, { { "$additive", "1" } } ) );
		list.Quad( 8, 8, 32, 32, red, 0 );
		list.Quad( 20, 20, 44, 44, blue, 0 );
		list.Quad( 48, 48, 60, 60, blue, 1 ); // additive over the background
		UiList reordered = list;
		std::swap( reordered.commands[0], reordered.commands[1] );
		UiList oneMaterial = list;
		oneMaterial.commands[2].material = 0;
		CanvasImage image, bad, badMaterial;
		if ( auto why = mustDraw( list, image ) )
			return why;
		if ( auto why = mustDraw( reordered, bad ) )
			return why;
		if ( auto why = mustDraw( oneMaterial, badMaterial ) )
			return why;
		const Rgba blueOut = Over( Source( kWhite, blue, false ), Blend::kOpaque );
		const Rgba redOut = Over( Source( kWhite, red, false ), Blend::kOpaque );
		const Rgba added = Over( Source( kWhite, blue, false ), Blend::kAdditive );
		results.That( Near( image.At( 25, 25 ), blueOut ) && Near( image.At( 10, 10 ), redOut ) &&
		                  Near( image.At( 40, 40 ), blueOut ),
		    "ui.order.list-order" );
		results.That( Near( image.At( 54, 54 ), added ), "ui.materials.per-command" );
		results.That( !Near( bad.At( 25, 25 ), blueOut ), "ui.order.control-reordered-fails" );
		results.That(
		    !Near( badMaterial.At( 54, 54 ), added ), "ui.materials.control-one-material-fails" );
	}

	// Malformed lists are refused whole, by name.
	{
		struct Bad
		{
			const char *name;
			const char *why;
			void ( *mutate )( UiList & );
		};
		static const Bad kBad[] = {
		    { "vertices-outside", "outside the list",
		        []( UiList &l )
		        {
			        l.commands[0].vertexCount = 12;
		        } },
		    { "material-outside", "material outside",
		        []( UiList &l )
		        {
			        l.commands[0].material = 3;
		        } },
		    { "partial-triangle", "whole triangles",
		        []( UiList &l )
		        {
			        l.commands[0].vertexCount = 4;
		        } },
		    { "empty-viewport", "viewport is empty",
		        []( UiList &l )
		        {
			        l.viewport[2] = 0;
		        } },
		    { "zero-scale", "out of range",
		        []( UiList &l )
		        {
			        l.scale = 0.0f;
		        } },
		};
		for ( const Bad &bad : kBad )
		{
			UiList list;
			list.materials.push_back( Material( "ui/opaque", 1, {} ) );
			list.Quad( 8, 8, 16, 16, kColor, 0 );
			bad.mutate( list );
			WorldView view;
			const std::optional<std::string> why = UiListView( list.View(), list.materials, view );
			results.That( why && why->find( bad.why ) != std::string::npos,
			    std::string( "ui.refuse." ) + bad.name, why.value_or( "accepted" ) );
		}
	}

	results.That(
	    pass.Stats().viewsFailed == 0, "ui.claimed-views-complete", pass.Stats().lastFailure );
	(void)device->WaitIdle();
	pass.ReleaseDevice( *device );
	messages = counter.load();
	results.That( messages == 0, "ui.validation-silent" );
	return std::nullopt;
}

} // namespace

int RunUiSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "ui", {}, RunChecks );
}

} // namespace render::lab
