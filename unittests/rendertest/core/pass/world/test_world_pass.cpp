//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.world (RFC 0016 K5) on render.device.null, which
//			validates every transition and binding:
//
//			W1 SetWorld claims each material through the one resolver: a
//			   LightmappedGeneric and an UnlitGeneric (the lightmapped term
//			   with lighting one) are drawn; a material with $envmap stays
//			   legacy with the gap named, and a blended one too.
//			W2 A queued view records its surfaces at its slot, with its lightmap
//			   pages (a surface with no page samples the neutral white).
//			W3 The same slot recorded again (a capture re-records the stream)
//			   draws the same view.
//			W4 A slot whose view was queued against an earlier world draws
//			   nothing and is counted; a view naming a surface the pass does not
//			   draw is counted.
//			W5 Stats name the claimed materials.
//
//=============================================================================//

#include "render/device/null/provider.h"
#include "render/pass/world/world_pass.h"
#include "testing/checks.h"

#include <string>
#include <vector>

namespace
{

using namespace render;
using namespace render::device;
using namespace render::pass::world;

// The backend's textures, stood in by null textures made on import.
class FakeTextures final : public IWorldTextures
{
public:
	explicit FakeTextures( IRenderDevice2 &device ) : m_Device( device ) {}
	TextureId Import( int handle, bool srgb ) override
	{
		++imports;
		if ( handle <= 0 )
			return {};
		TextureDesc desc;
		desc.format = srgb ? Format::kRGBA8Srgb : Format::kRGBA8Unorm;
		desc.width = desc.height = 4;
		desc.usages = { ResourceUsage::kSampled };
		auto texture = m_Device.CreateTexture( desc );
		return texture ? texture.Value() : TextureId();
	}
	SamplerDesc Sampler( int ) override { return {}; }
	int imports = 0;

private:
	IRenderDevice2 &m_Device;
};

WorldData TestWorld()
{
	WorldData world;
	// Four quads, one per surface.
	for ( int s = 0; s < 4; ++s )
	{
		const float x = float( s ) * 2.0f;
		const float corners[4][2] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
		const std::uint32_t first = std::uint32_t( world.vertices.size() );
		for ( const auto &c : corners )
		{
			WorldVertex v;
			v.position[0] = x + c[0];
			v.position[1] = c[1];
			v.uv[0] = c[0];
			v.uv[1] = c[1];
			v.lightmapUv[0] = c[0];
			v.lightmapUv[1] = c[1];
			world.vertices.push_back( v );
		}
		WorldSurface surface;
		surface.material = std::uint32_t( s );
		surface.lightmapPage = s == 1 ? 0 : 7; // the unlit surface has no page
		surface.firstIndex = std::uint32_t( world.indices.size() );
		for ( std::uint32_t i : { 0u, 1u, 2u, 0u, 2u, 3u } )
			world.indices.push_back( first + i );
		surface.indexCount = 6;
		world.surfaces.push_back( surface );
	}
	auto material = []( const char *name, const char *shader,
	                    std::vector<std::pair<std::string, std::string>> variables )
	{
		WorldMaterial m;
		m.name = name;
		m.shader = shader;
		m.variables = std::move( variables );
		m.textures = { { "$basetexture", 3 } };
		return m;
	};
	world.materials.push_back(
	    material( "lit", "LightmappedGeneric", { { "$basetexture", "concrete/wall" } } ) );
	world.materials.push_back(
	    material( "unlit", "UnlitGeneric", { { "$basetexture", "lights/white" } } ) );
	world.materials.push_back( material( "shiny", "LightmappedGeneric",
	    { { "$basetexture", "metal/plate" }, { "$envmap", "env_cubemap" } } ) );
	world.materials.push_back( material( "glass", "LightmappedGeneric",
	    { { "$basetexture", "glass/pane" }, { "$translucent", "1" } } ) );
	return world;
}

WorldView View( std::vector<std::uint32_t> surfaces )
{
	WorldView view;
	view.surfaces = std::move( surfaces );
	for ( int i = 0; i < 4; ++i )
		view.toClip[i * 5] = 1.0f;
	view.viewport = { 0, 0, 64, 64, 0, 1 };
	return view;
}

// Records slot `tag` into one submission, as the scene stage records a
// section; true when the device accepted it.
bool RecordSlot(
    IRenderDevice2 &device, WorldPass &pass, std::uint32_t tag, const WorldTarget &target )
{
	auto encoder = device.BeginEncoder( QueueKind::kGraphics );
	if ( !encoder )
		return false;
	pass.Record( tag, encoder.Value(), target );
	return device.Submit( QueueKind::kGraphics, { &encoder.Value(), 1 }, {} ).HasValue();
}

} // namespace

int main()
{
	testing::Checks checks;
	auto created = null::Create( {} );
	if ( !checks.That( created.HasValue(), "setup.null-device" ) )
		return checks.Report();
	IRenderDevice2 &device = *created.Value();

	WorldPass pass;
	pass.SetWorld( TestWorld() );
	const WorldStats loaded = pass.Stats();
	checks.That( loaded.materials == 4 && loaded.claimedMaterials == 2 && pass.Draws( 0 ) &&
	                 pass.Draws( 1 ) && !pass.Draws( 2 ) && !pass.Draws( 3 ),
	    "W1.lightmapped-and-unlit-claimed-envmap-and-blended-not" );
	bool envmapGap = false;
	bool blendedGap = false;
	for ( const auto &[reason, count] : loaded.gaps )
	{
		envmapGap = envmapGap || reason.find( "$envmap" ) != std::string::npos;
		blendedGap = blendedGap || reason.find( "blended" ) != std::string::npos;
	}
	checks.That( envmapGap && blendedGap, "W1.gaps-are-named" );
	checks.That( loaded.claimed.size() == 2 && loaded.claimed[0].first == "lit" &&
	                 loaded.claimed[1].first == "unlit",
	    "W5.stats-name-the-claimed-materials" );

	// The slot's target: color and depth in their home usages.
	TextureDesc colorDesc;
	colorDesc.format = Format::kRGBA8Srgb;
	colorDesc.width = colorDesc.height = 64;
	colorDesc.usages = { ResourceUsage::kColorAttachment };
	TextureDesc depthDesc = colorDesc;
	depthDesc.format = Format::kD32Float;
	depthDesc.usages = { ResourceUsage::kDepthWrite };
	auto color = device.CreateTexture( colorDesc );
	auto depth = device.CreateTexture( depthDesc );
	auto setup = device.BeginEncoder( QueueKind::kGraphics );
	if ( !color || !depth || !setup )
		return checks.Report();
	setup.Value().TransitionTexture(
	    color.Value(), ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
	setup.Value().TransitionTexture(
	    depth.Value(), ResourceUsage::kUndefined, ResourceUsage::kDepthWrite );
	(void)device.Submit( QueueKind::kGraphics, { &setup.Value(), 1 }, {} );

	FakeTextures textures( device );
	WorldTarget target;
	target.device = &device;
	target.color = color.Value();
	target.colorFormat = Format::kRGBA8Srgb;
	target.depth = depth.Value();
	target.depthFormat = Format::kD32Float;
	target.width = target.height = 64;
	target.textures = &textures;
	target.lightmapScale = 16.0f;
	target.outputScale = 1.3f;

	const std::uint32_t tag = pass.QueueView( View( { 0, 1 } ) );
	checks.That( IsWorldTag( tag ), "W2.a-view-queues-a-world-tag" );
	checks.That( RecordSlot( device, pass, tag, target ), "W2.the-slot-records-and-submits" );
	WorldStats stats = pass.Stats();
	checks.That( stats.viewsDrawn == 1 && stats.surfacesDrawn == 2 && stats.viewsFailed == 0,
	    "W2.both-surfaces-draw" );

	checks.That( RecordSlot( device, pass, tag, target ), "W3.the-same-slot-records-again" );
	stats = pass.Stats();
	checks.That( stats.viewsDrawn == 2 && stats.surfacesDrawn == 4 && stats.viewsFailed == 0,
	    "W3.a-re-recorded-slot-draws-the-same-view" );

	const std::uint32_t stale = pass.QueueView( View( { 0 } ) );
	pass.SetWorld( TestWorld() );
	(void)RecordSlot( device, pass, stale, target );
	stats = pass.Stats();
	checks.That( stats.viewsDrawn == 2 && stats.viewsFailed == 1 &&
	                 stats.lastFailure.find( "no queued view" ) != std::string::npos,
	    "W4.a-view-of-an-earlier-world-draws-nothing" );

	const std::uint32_t mixed = pass.QueueView( View( { 0, 2 } ) );
	(void)RecordSlot( device, pass, mixed, target );
	stats = pass.Stats();
	checks.That( stats.viewsDrawn == 3 && stats.surfacesDrawn == 5 && stats.viewsFailed == 2,
	    "W4.a-surface-the-pass-does-not-draw-is-counted" );

	pass.ReleaseDevice( device );
	(void)device.WaitIdle();
	return checks.Report();
}
