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
//			W6 A claimed material the render sequence fails (a texture that
//			   does not import) is a counted, named failure; the pass still
//			   claims it (nothing claimed goes back to legacy).
//			W7 A slot of an earlier world recorded again (a capture across a
//			   level change) fails alone: the next world's queued views stay.
//			W9 A variable the model does not read keeps a material out unless
//			   it holds its shader's neutral value (declared defaults), and a
//			   variable with no neutral value keeps it out.
//			W10 Views of a host frame the backend never recorded are skipped,
//			   not failed; a view whose slot never recorded while another slot
//			   of its frame did is a failure.
//			P1 The resolver's editor preview (ProgramResolver::ResolvePreview)
//			   draws a material strict Resolve refuses, with its blend, and names
//			   every variable it ignores.
//			W8 Objects a frame used outlive it: a level change, or a second
//			   target format, between two slots of one submission leaves the
//			   first slot's objects alive, and they are released at a later
//			   frame's slot.
//			W13 Sparse probe-atlas updates reach a staged world; malformed
//			    rectangles fail the view by name.
//
//=============================================================================//

#include "render/device/null/provider.h"
#include "render/material/program_resolver.h"
#include "render/pass/world/world_pass.h"
#include "../../../../../render/pass/world/group_resources.h"
#include "testing/checks.h"

#include <algorithm>
#include <cstddef>
#include <memory>
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

WorldView View( std::vector<std::uint32_t> surfaces, std::uint64_t hostFrame = 0 )
{
	WorldView view;
	view.surfaces = std::move( surfaces );
	for ( int i = 0; i < 4; ++i )
		view.toClip[i * 5] = 1.0f;
	view.viewport = { 0, 0, 64, 64, 0, 1 };
	view.hostFrame = hostFrame;
	return view;
}

class CostLabels final : public ILabelObserver
{
public:
	void OnBeginLabel( CommandEncoder &, std::string_view name ) override
	{
		names.emplace_back( name );
		++depth;
	}
	void OnEndLabel( CommandEncoder & ) override
	{
		if ( depth == 0 )
			underflow = true;
		--depth;
	}
	bool Has( std::string_view name ) const
	{
		return std::find( names.begin(), names.end(), name ) != names.end();
	}
	std::vector<std::string> names;
	int depth = 0;
	bool underflow = false;
};

// Records slot `tag` into one submission, as the scene stage records a
// section; true when the device accepted it.
bool RecordSlot( IRenderDevice2 &device, WorldPass &pass, std::uint32_t tag,
    const WorldTarget &target, ILabelObserver *observer = nullptr )
{
	auto encoder = device.BeginEncoder( QueueKind::kGraphics );
	if ( !encoder )
		return false;
	encoder.Value().SetLabelObserver( observer );
	pass.Record( tag, encoder.Value(), target );
	return device.Submit( QueueKind::kGraphics, { &encoder.Value(), 1 }, {} ).HasValue();
}

void OpaqueBatching( testing::Checks &checks )
{
	auto created = null::Create( { .completion = null::CompletionMode::kManual } );
	if ( !checks.That( created.HasValue(), "W21.device" ) )
		return;
	auto &device = *created.Value();
	WorldData world = TestWorld();
	auto stage = std::make_shared<WorldStage>();
	stage->lightmap.width = stage->lightmap.height = 1;
	stage->lightmap.flat.resize( 8 );
	world.stage = stage;
	WorldMaterial model;
	model.name = "batch-model";
	model.shader = "VertexLitGeneric";
	model.mesh = true;
	world.materials.push_back( model );
	WorldData::StaticMesh mesh;
	for ( const auto &v : world.vertices )
	{
		material::SurfaceModelVertex vertex;
		std::copy_n( v.position, 3, vertex.position );
		vertex.normal[2] = 1;
		vertex.tangent[0] = vertex.tangent[3] = 1;
		mesh.vertices.push_back( vertex );
	}
	mesh.indices = { 0, 1, 2, 0, 2, 3 };
	mesh.surfaces.push_back( { 4, 0, 0, 6 } );
	world.staticMeshes.push_back( mesh );
	WorldData::StaticInstance instance;
	for ( int i = 0; i < 4; ++i )
		instance.world[i * 5] = 1;
	world.staticInstances.push_back( instance );
	WorldPass pass;
	pass.SetWorld( world );
	WorldView wall = View( { 0 }, 1 ), prop = View( {}, 1 );
	prop.staticInstances = { 0 };
	const auto wallTag = pass.QueueView( wall ), propTag = pass.QueueView( prop );
	const std::uint32_t pair[] = { wallTag, propTag };
	checks.That( pass.OpaqueBatchSize( pair, 1 ) == 2, "W21.world-model-prefix" );
	for ( int boundary = 0; boundary != 11; ++boundary )
	{
		WorldView changed = prop;
		switch ( boundary )
		{
		case 0:
			changed.hostFrame++;
			break;
		case 1:
			changed.toClip[0] = 2;
			break;
		case 2:
			changed.viewport.width = 32;
			break;
		case 3:
			changed.surfaces = { 0 };
			break;
		case 4:
			changed.lights = std::make_shared<StageViewLights>();
			break;
		case 5:
			changed.debug.view = 1;
			break;
		case 6:
			changed.debug.termsOff = 1;
			break;
		case 7:
			changed.stageLighting = std::make_shared<StageLightingInputs>();
			break;
		case 8:
			changed.previousViewValid = true;
			break;
		case 9:
			changed.motionToClip[0] = 2;
			break;
		case 10:
			changed.previousToClip[0] = 2;
			break;
		}
		const std::uint32_t tags[] = { wallTag, pass.QueueView( changed ) };
		checks.That( pass.OpaqueBatchSize( tags, 1 ) == 1,
		    "W21.view-boundary-" + std::to_string( boundary ) );
	}
	for ( const char *shader : { "Refract", "Water", "DepthWrite" } )
	{
		WorldPass other;
		WorldData blocked = world;
		blocked.materials.back().shader = shader;
		other.SetWorld( std::move( blocked ) );
		const std::uint32_t tags[] = { other.QueueView( wall ), other.QueueView( prop ) };
		checks.That( other.OpaqueBatchSize( tags, 1 ) == 1,
		    std::string( "W21.ordered-material-boundary-" ) + shader );
	}
	WorldPass transmission;
	WorldData transmitting = world;
	auto &pbr = transmitting.materials.back();
	pbr.shader = "PBRMetalRough";
	pbr.variables = {
	    { "$basetexture", "base" }, { "$mraotexture", "mrao" }, { "$transmission", "1" } };
	transmission.SetWorld( std::move( transmitting ) );
	const std::uint32_t transmissionTags[] = {
	    transmission.QueueView( wall ), transmission.QueueView( prop ) };
	checks.That(
	    transmission.Draws( 4 ) && transmission.OpaqueBatchSize( transmissionTags, 1 ) == 1,
	    "W21.opaque-blend-transmission-still-needs-scene-capture" );
	WorldPass blended;
	WorldData glass = world;
	glass.materials.back().variables = { { "$translucent", "1" } };
	blended.SetWorld( std::move( glass ) );
	const std::uint32_t glassTags[] = { blended.QueueView( wall ), blended.QueueView( prop ) };
	checks.That( blended.OpaqueBatchSize( glassTags, 1 ) == 1, "W21.blended-boundary" );
	const std::uint32_t duplicate[] = { propTag, propTag };
	checks.That( pass.OpaqueBatchSize( duplicate, 1 ) == 1, "W21.repeated-ticket-boundary" );
	TextureDesc desc;
	desc.format = Format::kRGBA8Srgb;
	desc.width = desc.height = 64;
	desc.usages = { ResourceUsage::kColorAttachment };
	const auto color = device.CreateTexture( desc ).Value();
	desc.format = Format::kD32Float;
	desc.usages = { ResourceUsage::kDepthWrite };
	const auto depth = device.CreateTexture( desc ).Value();
	FakeTextures textures( device );
	WorldTarget target;
	target.device = &device;
	target.color = color;
	target.depth = depth;
	target.colorFormat = Format::kRGBA8Srgb;
	target.depthFormat = Format::kD32Float;
	target.width = target.height = 64;
	target.textures = &textures;
	target.frame = target.streamEpoch = 1;
	target.depthPrepass = true;
	auto encoder = device.BeginEncoder( QueueKind::kGraphics ).Value();
	encoder.TransitionTexture( color, ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
	encoder.TransitionTexture( depth, ResourceUsage::kUndefined, ResourceUsage::kDepthWrite );
	pass.RecordBatch( pair, encoder, target );
	const auto submission = device.Submit( QueueKind::kGraphics, { &encoder, 1 }, {} );
	checks.That( submission && pass.Failures() == 0 && pass.Stats().viewsDrawn == 2 &&
	                 pass.Stats().staticDrawsDrawn == 1,
	    "W21.batch-records-all-tickets" );
	if ( submission )
	{
		target.submitted = submission.Value();
		++target.frame;
		encoder = device.BeginEncoder( QueueKind::kGraphics ).Value();
		pass.RecordBatch( pair, encoder, target );
		const auto replay = device.Submit( QueueKind::kGraphics, { &encoder, 1 }, {} );
		checks.That(
		    replay && !device.IsComplete( submission.Value() ) && pass.Stats().viewsDrawn == 4,
		    "W21.capture-replay-before-completion" );
		null::Control( device )->CompleteAll();
		checks.That(
		    device.IsComplete( submission.Value() ), "W21.batch-resources-outlive-submit" );
	}
	WorldPass invalid;
	invalid.SetWorld( world );
	WorldView laterFrame = prop;
	++laterFrame.hostFrame;
	const std::uint32_t invalidTags[] = {
	    invalid.QueueView( wall ), invalid.QueueView( laterFrame ) };
	encoder = device.BeginEncoder( QueueKind::kGraphics ).Value();
	invalid.RecordBatch( invalidTags, encoder, target );
	checks.That( invalid.Failures() == 1 && invalid.Stats().viewsDrawn == 0,
	    "W21.invalid-batch-refused-before-drawing" );
	for ( auto tag : invalidTags )
		invalid.Record( tag, encoder, target );
	const auto recovered = device.Submit( QueueKind::kGraphics, { &encoder, 1 }, {} );
	checks.That( recovered && invalid.Stats().viewsDrawn == 2 && invalid.Failures() == 1,
	    "W21.refusal-leaves-original-tickets-recordable" );
	null::Control( device )->CompleteAll();
	invalid.ReleaseDevice( device );
	pass.ReleaseDevice( device );
	(void)device.Release( color, {} );
	(void)device.Release( depth, {} );
}

void LitViewLifetime( testing::Checks &checks )
{
	null::NullOptions options;
	options.completion = null::CompletionMode::kManual;
	auto made = null::Create( options );
	if ( !checks.That( made.HasValue(), "W20.deferred-view-device" ) )
		return;
	auto &device = *made.Value();
	auto *control = null::Control( device );
	TextureDesc colorDesc;
	colorDesc.format = Format::kRGBA8Srgb;
	colorDesc.width = colorDesc.height = 64;
	colorDesc.usages = { ResourceUsage::kColorAttachment };
	auto color = device.CreateTexture( colorDesc ).Value();
	TextureDesc depthDesc = colorDesc;
	depthDesc.format = Format::kD32Float;
	depthDesc.usages = { ResourceUsage::kDepthWrite };
	auto depth = device.CreateTexture( depthDesc ).Value();
	auto setup = device.BeginEncoder( QueueKind::kGraphics ).Value();
	setup.TransitionTexture( color, ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
	setup.TransitionTexture( depth, ResourceUsage::kUndefined, ResourceUsage::kDepthWrite );
	(void)device.Submit( QueueKind::kGraphics, { &setup, 1 }, {} );
	WorldData world = TestWorld();
	world.materials[0].variables.clear();
	world.materials[0].textures.clear();
	WorldStage stage;
	stage.lightmap.width = stage.lightmap.height = 4;
	stage.lightmap.flat.resize( 4 * 4 * 8 );
	world.stage = std::make_shared<const WorldStage>( std::move( stage ) );
	WorldPass pass;
	pass.SetWorld( std::move( world ) );
	FakeTextures textures( device );
	WorldTarget target;
	target.device = &device;
	target.color = color;
	target.colorFormat = colorDesc.format;
	target.depth = depth;
	target.depthFormat = depthDesc.format;
	target.width = target.height = 64;
	target.textures = &textures;
	target.lights = std::make_shared<const StageViewLights>();
	target.frame = 1;
	auto encoder = device.BeginEncoder( QueueKind::kGraphics ).Value();
	pass.Record( pass.QueueView( View( { 0 } ) ), encoder, target );
	pass.Record( pass.QueueView( View( { 0 } ) ), encoder, target );
	auto first = device.Submit( QueueKind::kGraphics, { &encoder, 1 }, {} );
	checks.That(
	    first.HasValue() && pass.Failures() == 0, "W20.shared-cohorts-submit-before-completion" );
	if ( first )
	{
		target.submitted = first.Value();
		target.frame = 2;
		encoder = device.BeginEncoder( QueueKind::kGraphics ).Value();
		pass.Record( pass.QueueView( View( { 0 } ) ), encoder, target );
		auto next = device.Submit( QueueKind::kGraphics, { &encoder, 1 }, {} );
		checks.That( next.HasValue() && !device.IsComplete( first.Value() ),
		    "W20.another-frame-retires-bindings-without-waiting-or-overwriting" );
		control->CompleteAll();
		checks.That( device.IsComplete( first.Value() ) && pass.Failures() == 0 &&
		                 !control->Recorded().empty(),
		    "W20.retired-bindings-survive-delayed-execution" );
	}
	pass.ReleaseDevice( device );
	(void)device.Release( color, {} );
	(void)device.Release( depth, {} );
	(void)device.Poll();
	checks.That( device.LiveResourceCount() == 0, "W20.shared-view-teardown-leaks-nothing" );
}

// Exercise the private owner against deferred execution. Native lab images
// independently check the contents consumed by the surface shaders.
void GroupReuse( testing::Checks &checks )
{
	null::NullOptions options;
	options.completion = null::CompletionMode::kManual;
	auto made = null::Create( options );
	if ( !checks.That( made.HasValue(), "W18.reuse-device" ) )
		return;
	auto &device = *made.Value();
	auto *control = null::Control( device );
	GroupResources resources;
	auto first = resources.Acquire( device, 16, ResourceUsage::kUniform ).Value();
	std::vector<std::byte> bytes( 16, std::byte{ 0x31 } );
	auto encoder = device.BeginEncoder( QueueKind::kGraphics ).Value();
	encoder.TransitionBuffer( first.id, first.before, ResourceUsage::kCopyDestination );
	encoder.WriteBuffer( first.id, 0, bytes );
	encoder.TransitionBuffer( first.id, ResourceUsage::kCopyDestination, first.usage );
	first.before = first.usage;
	auto submitted = device.Submit( QueueKind::kGraphics, { &encoder, 1 }, {} );
	if ( !checks.That( submitted.HasValue(), "W18.first-upload-submitted" ) )
		return;
	resources.Retire( device, first, submitted.Value() );
	auto second = resources.Acquire( device, 16, ResourceUsage::kUniform ).Value();
	checks.That( second.id != first.id, "W18.pending-buffer-is-not-reused" );
	control->CompleteAll();
	checks.That( !control->Recorded().empty(), "W18.pending-upload-executes-after-completion" );
	auto reused = resources.Acquire( device, 16, ResourceUsage::kUniform ).Value();
	checks.That( reused.id == first.id && reused.before == ResourceUsage::kUniform,
	    "W18.completed-buffer-reuses-storage-and-preserves-state" );
	bytes.assign( 16, std::byte{ 0x72 } );
	encoder = device.BeginEncoder( QueueKind::kGraphics ).Value();
	encoder.TransitionBuffer( reused.id, reused.before, ResourceUsage::kCopyDestination );
	encoder.WriteBuffer( reused.id, 0, bytes );
	encoder.TransitionBuffer( reused.id, ResourceUsage::kCopyDestination, reused.usage );
	submitted = device.Submit( QueueKind::kGraphics, { &encoder, 1 }, {} );
	control->CompleteAll();
	checks.That( submitted.HasValue(), "W18.reused-buffer-accepts-the-next-view-upload" );
	resources.Retire( device, reused, submitted.Value() );
	auto storage = resources.Acquire( device, 16, ResourceUsage::kStorageRead ).Value();
	auto larger = resources.Acquire( device, 32, ResourceUsage::kUniform ).Value();
	checks.That( storage.id != first.id && larger.id != first.id,
	    "W18.usage-and-size-are-not-interchangeable" );
	checks.That( !resources.Acquire( device, 0, ResourceUsage::kUniform ),
	    "W18.invalid-allocation-still-fails" );
	resources.Retire( device, second, {} );
	resources.Retire( device, storage, {} );
	resources.Retire( device, larger, {} );

	SamplerDesc desc;
	auto sampler = resources.AcquireSampler( device, desc ).Value();
	auto same = resources.AcquireSampler( device, desc ).Value();
	desc.comparison = CompareOp::kLessEqual;
	auto comparison = resources.AcquireSampler( device, desc ).Value();
	checks.That( sampler.id == same.id && !sampler.owned && comparison.id != sampler.id,
	    "W18.identical-samplers-share-but-depth-comparison-stays-distinct" );
	resources.Release( device );
	(void)device.Poll();
	checks.That( device.LiveResourceCount() == 0, "W18.drained-reuse-owner-releases-everything" );

	// Completion of one token must not certify later work or another queue.
	auto early = resources.Acquire( device, 16, ResourceUsage::kUniform ).Value();
	auto late = resources.Acquire( device, 16, ResourceUsage::kUniform ).Value();
	auto otherQueue = resources.Acquire( device, 16, ResourceUsage::kUniform ).Value();
	auto submitEmpty = [&]( QueueKind queue )
	{
		auto empty = device.BeginEncoder( queue ).Value();
		return device.Submit( queue, { &empty, 1 }, {} ).Value();
	};
	const auto earlyToken = submitEmpty( QueueKind::kGraphics );
	const auto lateToken = submitEmpty( QueueKind::kGraphics );
	const auto otherToken = submitEmpty( QueueKind::kCompute );
	resources.Retire( device, early, earlyToken );
	resources.Retire( device, late, lateToken );
	resources.Retire( device, otherQueue, otherToken );
	control->CompleteThrough( QueueKind::kGraphics, earlyToken.value );
	auto completeOnly = resources.Acquire( device, 16, ResourceUsage::kUniform ).Value();
	auto pendingStill = resources.Acquire( device, 16, ResourceUsage::kUniform ).Value();
	checks.That( completeOnly.id == early.id && pendingStill.id != late.id &&
	                 pendingStill.id != otherQueue.id,
	    "W18.completion-does-not-cover-later-submissions-or-other-queues" );
	control->CompleteAll();
	resources.Retire( device, completeOnly, {} );
	resources.Retire( device, pendingStill, {} );
	resources.Release( device );
	(void)device.Poll();

	// Retained cache bounds never become a rendering limit. Entries beyond
	// the cap are destroyed through their original completion token.
	std::vector<GroupResources::Buffer> buffers;
	for ( unsigned i = 0; i < 257; ++i )
		buffers.push_back( resources.Acquire( device, 16, ResourceUsage::kUniform ).Value() );
	encoder = device.BeginEncoder( QueueKind::kGraphics ).Value();
	submitted = device.Submit( QueueKind::kGraphics, { &encoder, 1 }, {} );
	for ( auto buffer : buffers )
		resources.Retire( device, buffer, submitted.Value() );
	(void)device.Poll();
	checks.That( device.LiveResourceCount() == 257, "W18.eviction-keeps-in-flight-resources" );
	control->CompleteAll();
	(void)device.Poll();
	checks.That( device.LiveResourceCount() == 256, "W18.retained-buffer-count-is-bounded" );
	resources.Release( device );
	(void)device.Poll();
	checks.That( device.LiveResourceCount() == 0, "W18.eviction-and-teardown-do-not-leak" );
	// Byte capacity is independent of entry capacity.
	auto big1 = resources.Acquire( device, 33u << 20, ResourceUsage::kStorageRead ).Value();
	auto big2 = resources.Acquire( device, 33u << 20, ResourceUsage::kStorageRead ).Value();
	resources.Retire( device, big1, {} );
	resources.Retire( device, big2, {} );
	(void)device.Poll();
	checks.That( device.LiveResourceCount() == 1, "W18.retained-buffer-bytes-are-bounded" );
	auto oversized = resources.Acquire( device, 65u << 20, ResourceUsage::kStorageRead ).Value();
	resources.Retire( device, oversized, {} );
	(void)device.Poll();
	checks.That(
	    device.LiveResourceCount() == 1, "W18.oversized-buffer-is-served-but-not-retained" );
	resources.Release( device );
	(void)device.Poll();
	bool samplerOwnership = true;
	for ( unsigned i = 0; i < 65; ++i )
	{
		SamplerDesc distinct;
		distinct.minFilter = i & 1 ? Filter::kNearest : Filter::kLinear;
		distinct.magFilter = i & 2 ? Filter::kNearest : Filter::kLinear;
		distinct.mipFilter = i & 4 ? Filter::kNearest : Filter::kLinear;
		distinct.address = AddressMode( ( i / 8 ) % 3 );
		if ( i / 24 != 0 )
			distinct.comparison = CompareOp( i / 24 - 1 );
		auto unique = resources.AcquireSampler( device, distinct ).Value();
		samplerOwnership = samplerOwnership && unique.owned == ( i == 64 );
		if ( unique.owned )
			(void)device.Release( unique.id, {} );
	}
	(void)device.Poll();
	checks.That( samplerOwnership && device.LiveResourceCount() == 64,
	    "W18.sampler-overflow-is-served-with-explicit-group-ownership" );
	resources.Release( device );
	(void)device.Poll();
	checks.That( device.LiveResourceCount() == 0, "W18.bounded-caches-release-all-resources" );
}

} // namespace

int main()
{
	testing::Checks checks;
	checks.That( !IsWorldTag( 0x88000001u ) && IsWorldTag( kWorldTag | kWorldSerialMask ),
	    "world-tags-exclude-temporal-slots" );
	OpaqueBatching( checks );
	GroupReuse( checks );
	LitViewLifetime( checks );
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
	// "program material": the program that draws it (cl_render_debug_claims).
	checks.That( loaded.claimed.size() == 2 && loaded.claimed[0].first == "lightmapped lit" &&
	                 loaded.claimed[1].first == "unlit unlit",
	    "W5.stats-name-the-claimed-materials-and-their-programs" );

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
	CostLabels labels;
	checks.That(
	    RecordSlot( device, pass, tag, target, &labels ), "W2.the-slot-records-and-submits" );
	checks.That( labels.depth == 0 && !labels.underflow &&
	                 labels.Has( "prepare world resources" ) &&
	                 labels.Has( "prepare world materials" ) &&
	                 labels.Has( "world / lightmapped" ) && labels.Has( "world / unlit" ),
	    "cost.world-families-and-preparation-balanced" );
	{
		WorldPass empty;
		CostLabels failed;
		checks.That( RecordSlot( device, empty, tag, target, &failed ) && failed.depth == 0 &&
		                 !failed.underflow && empty.Failures() == 1,
		    "cost.early-failure-closes-preparation" );
	}
	WorldStats stats = pass.Stats();
	checks.That( stats.viewsDrawn == 1 && stats.surfacesDrawn == 2 && stats.viewsFailed == 0,
	    "W2.both-surfaces-draw" );

	checks.That( RecordSlot( device, pass, tag, target ), "W3.the-same-slot-records-again" );
	stats = pass.Stats();
	checks.That( stats.viewsDrawn == 2 && stats.surfacesDrawn == 4 && stats.viewsFailed == 0,
	    "W3.a-re-recorded-slot-draws-the-same-view" );
	{
		WorldPass interleaved;
		interleaved.SetWorld( TestWorld() );
		const std::uint32_t current = interleaved.QueueView( View( { 0 }, 200 ) );
		const std::uint32_t future = interleaved.QueueView( View( { 0 }, 201 ) );
		WorldView streamed = View( {}, 200 );
		WorldView::DynamicDraw geometry;
		const WorldData fixture = TestWorld();
		geometry.material = fixture.materials[1];
		geometry.vertices = fixture.vertices;
		geometry.indices = { 0, 1, 2 };
		streamed.dynamicDraws.push_back( std::move( geometry ) );
		const std::uint32_t dynamic = interleaved.QueueView( std::move( streamed ) );
		WorldTarget streamTarget = target;
		streamTarget.streamEpoch = 1000;
		checks.That( current && future && dynamic &&
		                 RecordSlot( device, interleaved, current, streamTarget ) &&
		                 RecordSlot( device, interleaved, dynamic, streamTarget ) &&
		                 interleaved.Failures() == 0,
		    "W14.render-thread-dynamic-ticket-does-not-discard-main-thread-future-view" );
		streamTarget.streamEpoch = 1001;
		checks.That( RecordSlot( device, interleaved, future, streamTarget ) &&
		                 interleaved.Failures() == 0 && interleaved.Stats().viewsDrawn == 3,
		    "W14.future-frame-still-records-its-promised-view" );
		interleaved.ReleaseDevice( device );
	}
	{
		WorldPass captured;
		captured.SetWorld( TestWorld() );
		std::vector<std::uint32_t> tags;
		auto inputs = std::make_shared<StageLightingInputs>();
		std::weak_ptr<const StageLightingInputs> lifetime = inputs;
		for ( unsigned int i = 0; i < 97; ++i )
		{
			WorldView view = View( { 0 }, 300 );
			view.stageLighting = inputs;
			tags.push_back( captured.QueueView( std::move( view ) ) );
		}
		WorldTarget captureTarget = target;
		captureTarget.streamEpoch = 2000;
		captureTarget.lights = std::make_shared<StageViewLights>();
		bool accepted = true;
		for ( std::uint32_t slot : tags )
			accepted = captured.LightingInputs( slot, captureTarget.streamEpoch ) == inputs &&
			           RecordSlot( device, captured, slot, captureTarget ) && accepted;
		++captureTarget.frame; // readback may use a new GPU submission
		for ( std::uint32_t slot : tags )
			accepted = captured.LightingInputs( slot, captureTarget.streamEpoch ) == inputs &&
			           RecordSlot( device, captured, slot, captureTarget ) && accepted;
		checks.That( accepted && captured.Failures() == 0 && captured.Stats().viewsDrawn == 194,
		    "W15.capture-replays-all-97-slots-with-their-owned-lighting-snapshot" );
		inputs.reset();
		checks.That( !lifetime.expired(), "W15.replay-stream-owns-the-lighting-lifetime" );
		++captureTarget.streamEpoch;
		checks.That( !captured.LightingInputs( tags.front(), captureTarget.streamEpoch ),
		    "W15.another-stream-cannot-borrow-the-retired-lighting-inputs" );
		(void)RecordSlot( device, captured, tags.front(), captureTarget );
		checks.That( captured.Failures() == 1 && lifetime.expired(),
		    "W15.discarded-stream-cannot-be-replayed-under-a-new-stream-identity" );
		captured.ReleaseDevice( device );
	}
	{
		WorldPass lighting;
		lighting.SetWorld( TestWorld() );
		WorldView required = View( { 0 }, 301 );
		required.stageLighting = std::make_shared<StageLightingInputs>();
		const std::uint32_t missing = lighting.QueueView( std::move( required ) );
		const bool attached = lighting.LightingInputs( missing, target.streamEpoch ) != nullptr;
		(void)RecordSlot( device, lighting, missing, target );
		checks.That(
		    attached && lighting.Failures() == 1 &&
		        lighting.Stats().lastFailure.find( "queued lighting inputs" ) != std::string::npos,
		    "W16.a-claimed-stage-view-missing-lighting-fails-by-name" );
		lighting.ReleaseDevice( device );
	}

	// One object-space mesh is shared by its static instances. The same
	// queued world view accepts only instances whose complete mesh is claimed.
	{
		WorldData props = TestWorld();
		WorldMaterial model;
		model.name = "crate";
		model.shader = "VertexLitGeneric";
		model.mesh = true;
		model.variables = { { "$basetexture", "models/crate" } };
		model.textures = { { "$basetexture", 3 } };
		props.materials.push_back( std::move( model ) );
		WorldMaterial alternate = props.materials.back();
		alternate.name = "crate-alternate-skin";
		props.materials.push_back( std::move( alternate ) );
		WorldData::StaticMesh mesh;
		for ( const auto &xy :
		    { std::pair{ 0.0f, 0.0f }, std::pair{ 1.0f, 0.0f }, std::pair{ 0.0f, 1.0f } } )
		{
			material::SurfaceModelVertex vertex;
			vertex.position[0] = xy.first;
			vertex.position[1] = xy.second;
			vertex.normal[2] = 1.0f;
			vertex.tangent[0] = 1.0f;
			vertex.tangent[3] = 1.0f;
			mesh.vertices.push_back( vertex );
		}
		mesh.indices = { 0, 1, 2 };
		mesh.surfaces.push_back( { 4, 0, 0, 3 } );
		mesh.skinMaterials = { { 4 }, { 5 } };
		props.staticMeshes.push_back( std::move( mesh ) );
		WorldData::StaticInstance instance;
		instance.world[0] = instance.world[5] = instance.world[10] = instance.world[15] = 1.0f;
		instance.world[3] = 2.0f;
		props.staticInstances.push_back( instance );
		instance.world[3] = 4.0f;
		instance.skin = 1;
		props.staticInstances.push_back( instance );
		instance.skin = 2;
		props.staticInstances.push_back( instance );
		WorldPass modelPass;
		modelPass.SetWorld( std::move( props ) );
		checks.That( modelPass.DrawsStaticInstance( 0 ) && modelPass.DrawsStaticInstance( 1 ) &&
		                 !modelPass.DrawsStaticInstance( 2 ) && !modelPass.DrawsStaticInstance( 3 ),
		    "W11.claims-shared-geometry-with-valid-skins" );
		WorldView modelView = View( {} );
		modelView.staticInstances = { 0, 1 };
		const std::uint32_t modelTag = modelPass.QueueView( std::move( modelView ) );
		checks.That( modelTag != 0 && modelPass.Stats().staticInstancesQueued == 2,
		    "W11.static-only-view-has-its-own-slot" );
		checks.That( RecordSlot( device, modelPass, modelTag, target ) &&
		                 modelPass.Stats().viewsFailed == 0 &&
		                 modelPass.Stats().staticInstancesQueued == 2 &&
		                 modelPass.Stats().staticDrawsDrawn == 2,
		    "W11.shared-static-mesh-records-through-the-model-program" );
		checks.That( modelPass.DrawsPosedModel( 0, 0 ) && modelPass.DrawsPosedModel( 0, 1 ) &&
		                 !modelPass.DrawsPosedModel( 0, 2 ),
		    "W12.posed-model-claims-only-complete-skins" );
		WorldView poseView = View( {} );
		WorldView::PosedModel pose;
		pose.mesh = 0;
		pose.skin = 1;
		pose.vertices.resize( 3 );
		pose.vertices[0].position[0] = 0.25f;
		pose.vertices[1].position[0] = 1.25f;
		pose.vertices[2].position[1] = 1.0f;
		for ( auto &vertex : pose.vertices )
		{
			vertex.normal[2] = 1.0f;
			vertex.tangent[0] = vertex.tangent[3] = 1.0f;
		}
		poseView.posedModels.push_back( std::move( pose ) );
		const std::uint32_t poseTag = modelPass.QueueView( std::move( poseView ) );
		checks.That( poseTag != 0 && RecordSlot( device, modelPass, poseTag, target ) &&
		                 modelPass.Stats().viewsFailed == 0 &&
		                 modelPass.Stats().posedModelsQueued == 1 &&
		                 modelPass.Stats().posedDrawsDrawn == 1,
		    "W12.posed-model-uses-the-shared-material-and-current-pose" );
		modelPass.ReleaseDevice( device );
	}

	// A frame that straddles a level change (queued mode records its slots
	// after the change): its view was the earlier world's, which is gone. It
	// draws nothing and is skipped, not failed.
	const std::uint32_t stale = pass.QueueView( View( { 0 } ) );
	const std::uint64_t skippedBefore = pass.Stats().viewsSkipped;
	pass.SetWorld( TestWorld() );
	(void)RecordSlot( device, pass, stale, target );
	stats = pass.Stats();
	checks.That(
	    stats.viewsDrawn == 2 && stats.viewsFailed == 0 && stats.viewsSkipped == skippedBefore + 1,
	    "W4.a-view-of-an-earlier-world-draws-nothing-and-is-skipped" );

	const std::uint32_t mixed = pass.QueueView( View( { 0, 2 } ) );
	(void)RecordSlot( device, pass, mixed, target );
	stats = pass.Stats();
	checks.That( stats.viewsDrawn == 3 && stats.surfacesDrawn == 5 && stats.viewsFailed == 1,
	    "W4.a-surface-the-pass-does-not-draw-is-counted" );

	// W9: unread variables against their neutral values.
	{
		WorldData world = TestWorld();
		world.materials[0].variables.push_back( { "$outline", "0" } );
		world.materials[0].defaults = { { "$outline", "0" } };
		world.materials[1].variables.push_back( { "$outline", "1" } );
		world.materials[1].defaults = { { "$outline", "0" } };
		world.materials[2] = world.materials[0];
		world.materials[2].variables.push_back( { "$mystery", "2" } );
		WorldPass unread;
		unread.SetWorld( std::move( world ) );
		bool named = false;
		bool unknown = false;
		for ( const auto &[reason, count] : unread.Stats().gaps )
		{
			named = named || reason.find( "does not read $outline 1" ) != std::string::npos;
			unknown = unknown || reason == "unlit: the model does not read $mystery" ||
			          reason.find( "does not read $mystery" ) != std::string::npos;
		}
		checks.That( unread.Draws( 0 ), "W9.an-unread-variable-at-neutral-is-claimed" );
		checks.That( !unread.Draws( 1 ) && named, "W9.an-unread-variable-set-is-a-named-gap" );
		checks.That(
		    !unread.Draws( 2 ) && unknown, "W9.an-unread-variable-without-neutral-is-a-gap" );
	}

	// P1: the editor preview against strict resolution.
	{
		auto resolver =
		    material::ProgramResolver::Create( device, Format::kRGBA8Srgb, Format::kD32Float, 1 );
		auto mapped = material::MapVariables( "LightmappedGeneric",
		    { { "$basetexture", "metal/plate" }, { "$envmap", "env_cubemap" }, { "$additive", "1" },
		        { "$surfaceprop", "metal" } },
		    {} );
		if ( checks.That( resolver.HasValue() && mapped.HasValue(), "P1.setup" ) )
		{
			material::ProgramResolver &r = *resolver.Value();
			checks.That( !r.Resolve( mapped.Value() ).HasValue(), "P1.strict-resolve-refuses" );
			auto preview = r.ResolvePreview( mapped.Value() );
			checks.That( preview.HasValue() && preview.Value().program.request.pipeline.IsValid() &&
			                 preview.Value().program.blend == BlendMode::kAdditive,
			    "P1.the-preview-draws-with-its-blend" );
			checks.That( preview.HasValue() && preview.Value().ignored.size() == 1 &&
			                 preview.Value().ignored[0] == "$envmap",
			    "P1.the-preview-names-what-it-ignores" );
		}
	}

	// W6: the lit material's texture does not import.
	{
		WorldData broken = TestWorld();
		broken.materials[0].textures = { { "$basetexture", -5 } };
		pass.SetWorld( std::move( broken ) );
		const std::uint64_t before = pass.Failures();
		const std::uint32_t failing = pass.QueueView( View( { 0, 1 } ) );
		target.frame = 10;
		(void)RecordSlot( device, pass, failing, target );
		stats = pass.Stats();
		checks.That( pass.Failures() == before + 1 && stats.viewsFailed == before + 1 &&
		                 stats.lastFailure.find( "lit" ) != std::string::npos &&
		                 stats.lastFailure.find( "did not import" ) != std::string::npos,
		    "W6.a-claimed-material-that-fails-is-a-named-failure" );
		checks.That( pass.Draws( 0 ), "W6.the-failed-material-stays-claimed" );
	}

	// W7: a capture re-records an earlier world's slot after the change.
	{
		pass.SetWorld( TestWorld() );
		const std::uint32_t old = pass.QueueView( View( { 0 } ) );
		target.frame = 11;
		(void)RecordSlot( device, pass, old, target );
		pass.SetWorld( TestWorld() );
		const std::uint32_t next = pass.QueueView( View( { 0, 1 } ) );
		const std::uint64_t before = pass.Failures();
		const std::uint64_t drawn = pass.Stats().viewsDrawn;
		const std::uint64_t skipped = pass.Stats().viewsSkipped;
		target.frame = 12;
		(void)RecordSlot( device, pass, old, target );
		checks.That( pass.Failures() == before && pass.Stats().viewsSkipped == skipped + 1 &&
		                 pass.Stats().viewsDrawn == drawn,
		    "W7.the-earlier-worlds-slot-is-skipped" );
		(void)RecordSlot( device, pass, next, target );
		checks.That( pass.Failures() == before && pass.Stats().viewsDrawn == drawn + 1,
		    "W7.the-next-worlds-view-is-still-queued-and-draws" );
	}

	// W8: two slots of one submission, a level change and a new format
	// between them.
	{
		TextureDesc unormDesc = colorDesc;
		unormDesc.format = Format::kRGBA8Unorm;
		auto unorm = device.CreateTexture( unormDesc );
		auto prepare = device.BeginEncoder( QueueKind::kGraphics );
		if ( unorm && prepare )
		{
			prepare.Value().TransitionTexture(
			    unorm.Value(), ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
			(void)device.Submit( QueueKind::kGraphics, { &prepare.Value(), 1 }, {} );
		}
		WorldTarget unormTarget = target;
		unormTarget.color = unorm ? unorm.Value() : TextureId();
		unormTarget.colorFormat = Format::kRGBA8Unorm;
		unormTarget.encodeOutput = true;

		const std::uint64_t before = pass.Failures();
		const std::uint32_t first = pass.QueueView( View( { 0, 1 } ) );
		auto encoder = device.BeginEncoder( QueueKind::kGraphics );
		bool accepted = false;
		if ( encoder )
		{
			target.frame = unormTarget.frame = 13;
			pass.Record( first, encoder.Value(), target );
			pass.SetWorld( TestWorld() );
			const std::uint32_t second = pass.QueueView( View( { 0, 1 } ) );
			pass.Record( second, encoder.Value(), target );
			const std::uint32_t third = pass.QueueView( View( { 0 } ) );
			pass.Record( third, encoder.Value(), unormTarget );
			accepted =
			    device.Submit( QueueKind::kGraphics, { &encoder.Value(), 1 }, {} ).HasValue();
		}
		checks.That( accepted && pass.Failures() == before,
		    "W8.a-submission-keeps-what-its-earlier-slots-used" );
		// A later frame's slot releases the retired world behind its token.
		const std::uint32_t later = pass.QueueView( View( { 0 } ) );
		target.frame = 14;
		checks.That( RecordSlot( device, pass, later, target ) && pass.Failures() == before,
		    "W8.a-later-frame-releases-the-retired-world" );
		if ( unorm )
			(void)device.Release( unorm.Value(), {} );
	}

	// W10: host frame 100 never records; frame 101 records one of its two.
	{
		const std::uint64_t failures = pass.Failures();
		const std::uint64_t skipped = pass.Stats().viewsSkipped;
		(void)pass.QueueView( View( { 0 }, 100 ) );
		(void)pass.QueueView( View( { 1 }, 100 ) );
		const std::uint32_t lost = pass.QueueView( View( { 0 }, 101 ) );
		const std::uint32_t drawn = pass.QueueView( View( { 1 }, 101 ) );
		(void)lost;
		target.frame = 15;
		(void)RecordSlot( device, pass, drawn, target );
		checks.That( pass.Stats().viewsSkipped == skipped + 2,
		    "W10.views-of-an-unrecorded-frame-are-skipped" );
		checks.That(
		    pass.Failures() == failures + 1 &&
		        pass.Stats().lastFailure.find( "frame that recorded" ) != std::string::npos,
		    "W10.a-lost-slot-of-a-recorded-frame-fails" );
	}

	// W13: sparse GI publication updates the probe atlas the mesh program
	// samples. A rectangle outside the volume fails the view by name.
	{
		WorldData world = TestWorld();
		WorldStage stage;
		stage.lightmap.width = stage.lightmap.height = 4;
		stage.lightmap.flat.resize( 4 * 4 * 8 );
		StageProbeVolume probes;
		probes.atlasWidth = probes.atlasHeight = 4;
		probes.atlas.resize( 4 * 4 * 8 );
		probes.tableTexels = 6;
		probes.rows = 1;
		probes.table.resize( 6 * 4 );
		stage.probes = std::move( probes );
		world.stage = std::make_shared<const WorldStage>( std::move( stage ) );
		WorldPass staged;
		staged.SetWorld( std::move( world ) );
		StageProbeVolume update;
		update.tableTexels = 6;
		update.rows = 1;
		update.table.resize( 6 * 4 );
		target.frame = 16;
		const std::uint32_t initial = staged.QueueView( View( { 0 } ) );
		const bool ready = RecordSlot( device, staged, initial, target ) && staged.Failures() == 0;
		staged.SetStageProbeRegions( { { 1, 1, 1, 1 } }, std::vector<std::byte>( 8 ), {}, update );
		target.frame = 17;
		const std::uint32_t changed = staged.QueueView( View( { 0 } ) );
		checks.That(
		    ready && RecordSlot( device, staged, changed, target ) && staged.Failures() == 0,
		    "W13.sparse-probe-atlas-update-records-with-the-world" );
		staged.SetStageProbeRegions( { { 4, 1, 1, 1 } }, std::vector<std::byte>( 8 ), {}, update );
		target.frame = 18;
		const std::uint32_t malformed = staged.QueueView( View( { 0 } ) );
		(void)RecordSlot( device, staged, malformed, target );
		checks.That( staged.Failures() == 1 && staged.Stats().lastFailure.find(
		                                           "probe atlas update" ) != std::string::npos,
		    "W13.out-of-volume-probe-patch-fails-by-name" );
		staged.ReleaseDevice( device );
	}

	// W19: exercise recycling through its actual lit-world consumer, including
	// two views recorded in one frame and externally owned assignment buffers.
	{
		WorldData world = TestWorld();
		WorldStage stage;
		stage.lightmap.width = stage.lightmap.height = 4;
		stage.lightmap.flat.resize( 4 * 4 * 8 );
		world.stage = std::make_shared<const WorldStage>( std::move( stage ) );
		WorldPass staged;
		staged.SetWorld( std::move( world ) );
		auto lights = std::make_shared<StageViewLights>();
		auto *control = null::Control( device );
		int storageUploads = 0;
		int uploads = 0;
		std::uint64_t viewGroup = 0;
		auto record = [&]( std::uint64_t frame )
		{
			WorldView view = View( { 0 } );
			view.lights = lights;
			target.frame = frame;
			control->ClearRecorded();
			auto encoder = device.BeginEncoder( QueueKind::kGraphics ).Value();
			staged.Record( staged.QueueView( std::move( view ) ), encoder, target );
			auto submitted = device.Submit( QueueKind::kGraphics, { &encoder, 1 }, {} );
			if ( !submitted )
				return -1;
			target.submitted = submitted.Value();
			(void)device.Poll();
			int reused = 0;
			storageUploads = 0;
			uploads = 0;
			std::vector<std::uint64_t> bound;
			for ( const auto &command : control->Recorded() )
			{
				if ( command.op == null::RecordedOp::kTransitionBuffer &&
				     command.before == ResourceUsage::kStorageRead &&
				     command.after == ResourceUsage::kCopyDestination )
					++reused;
				if ( command.op == null::RecordedOp::kTransitionBuffer &&
				     command.after == ResourceUsage::kStorageRead )
					++storageUploads;
				if ( command.op == null::RecordedOp::kSetBindGroup )
					bound.push_back( command.resource );
				if ( command.op == null::RecordedOp::kWriteBuffer )
					++uploads;
			}
			viewGroup = bound.size() >= 2 ? bound[1] : 0;
			return reused;
		};
		checks.That( record( 0 ) == 0 && record( 20 ) == 0,
		    "W19.unknown-frame-resources-wait-for-teardown" );
		const std::uint64_t firstGroup = viewGroup;
		checks.That(
		    record( 20 ) == 0 && storageUploads == 0 && viewGroup == firstGroup && viewGroup != 0,
		    "W20.same-frame-cohorts-share-bindings-with-zero-storage-uploads" );
		lights = std::make_shared<StageViewLights>( *lights );
		lights->view.viewDistance[3] = 4.0f;
		checks.That( record( 20 ) == 0 && storageUploads == 5 && viewGroup != firstGroup,
		    "W20.new-lighting-snapshot-in-the-same-frame-gets-new-storage" );
		const std::uint64_t changedGroup = viewGroup;
		TextureDesc occlusionDesc;
		occlusionDesc.format = Format::kRGBA8Unorm;
		occlusionDesc.width = occlusionDesc.height = 4;
		occlusionDesc.usages = { ResourceUsage::kSampled };
		auto occlusion = device.CreateTexture( occlusionDesc ).Value();
		auto setupOcclusion = device.BeginEncoder( QueueKind::kGraphics ).Value();
		setupOcclusion.TransitionTexture(
		    occlusion, ResourceUsage::kUndefined, ResourceUsage::kSampled );
		(void)device.Submit( QueueKind::kGraphics, { &setupOcclusion, 1 }, {} );
		target.ambientOcclusion = occlusion;
		target.ambientOcclusionDesc = occlusionDesc;
		checks.That( record( 20 ) == 0 && storageUploads == 5 && viewGroup != changedGroup,
		    "W20.changed-screen-input-does-not-borrow-old-bindings" );
		target.ambientOcclusion = {};
		target.ambientOcclusionDesc = {};
		target.clipPlanes[0][0] = 1.0f;
		target.fogColor[0] = 0.25f;
		checks.That(
		    record( 20 ) == 0 && storageUploads == 0 && uploads == 1 && viewGroup == changedGroup,
		    "W20.frame-terms-do-not-reupload-unchanged-view-lighting" );
		target.clipPlanes[0][0] = 0.0f;
		target.fogColor[0] = 0.0f;
		target.streamEpoch = 55;
		checks.That( record( 20 ) == 0 && storageUploads == 5 && viewGroup != changedGroup,
		    "W20.another-recording-stream-does-not-borrow-cached-bindings" );
		target.streamEpoch = 0;
		checks.That( record( 21 ) == 5 && staged.Failures() == 0,
		    "W19.completed-lit-view-reuses-all-five-storage-buffers" );
		BufferDesc borrowedDesc;
		borrowedDesc.size = 16;
		borrowedDesc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kStorageRead };
		auto borrowed = device.CreateBuffer( borrowedDesc ).Value();
		auto encoder = device.BeginEncoder( QueueKind::kGraphics ).Value();
		encoder.TransitionBuffer(
		    borrowed, ResourceUsage::kUndefined, ResourceUsage::kStorageRead );
		(void)device.Submit( QueueKind::kGraphics, { &encoder, 1 }, {} );
		lights = std::make_shared<StageViewLights>( *lights );
		lights->gpuFroxels = lights->gpuIndices = borrowed;
		checks.That( record( 22 ) == 3 && record( 23 ) == 3 && staged.Failures() == 0,
		    "W19.borrowed-assignment-buffers-are-never-overwritten" );
		for ( unsigned int i = 0; i < 257; ++i )
		{
			lights = std::make_shared<StageViewLights>( *lights );
			(void)record( 24 );
		}
		checks.That( record( 24 ) == 0 && storageUploads == 3 && staged.Failures() == 0 &&
		                 staged.Stats().viewsDrawn == staged.Stats().viewsQueued,
		    "W20.binding-cache-overflow-keeps-every-draw-and-uses-transient-storage" );
		staged.ReleaseDevice( device );
		checks.That( device.Release( borrowed, {} ).HasValue(),
		    "W19.borrowed-assignment-buffer-outlives-world-teardown" );
		(void)device.Release( occlusion, {} );
		target.submitted = {};
	}

	// W17: a portal's target has stencil, while its private AO prepass is
	// single-sample D32 without stencil. The two passes have distinct states.
	{
		WorldData world = TestWorld();
		WorldStage stage;
		stage.lightmap.width = stage.lightmap.height = 4;
		stage.lightmap.flat.resize( 4 * 4 * 8 );
		world.stage = std::make_shared<const WorldStage>( std::move( stage ) );
		WorldPass staged;
		staged.SetWorld( std::move( world ) );
		TextureDesc portalDepthDesc = depthDesc;
		portalDepthDesc.format = Format::kD32FloatS8;
		auto portalDepth = device.CreateTexture( portalDepthDesc );
		TextureDesc aoDesc = colorDesc;
		aoDesc.format = Format::kRGBA16Float;
		aoDesc.usages = { ResourceUsage::kSampled };
		auto occlusion = device.CreateTexture( aoDesc );
		bool screened = false;
		if ( portalDepth && occlusion )
		{
			WorldTarget portal = target;
			portal.depth = portalDepth.Value();
			portal.depthFormat = Format::kD32FloatS8;
			portal.ambientOcclusion = occlusion.Value();
			portal.ambientOcclusionDesc = aoDesc;
			portal.drawState.stencil.enabled = true;
			portal.drawState.stencil.compare = CompareOp::kEqual;
			portal.drawState.stencil.reference = 1;
			portal.overrideDepthRange = true;
			portal.minDepth = 0.2f;
			portal.maxDepth = 0.8f;
			portal.screenPasses = [&]( CommandEncoder &, const WorldTarget::Prepass &prepass )
			{
				screened = prepass.depth.IsValid() && prepass.normalRoughness.IsValid();
				return screened;
			};
			const auto queued = staged.QueueView( View( { 0 } ) );
			(void)RecordSlot( device, staged, queued, portal );
		}
		checks.That( screened && staged.Stats().viewsDrawn == 1 && staged.Failures() == 0,
		    "W17.portal-stencil-state-does-not-enter-the-private-AO-prepass" );
		staged.ReleaseDevice( device );
		if ( portalDepth )
			(void)device.Release( portalDepth.Value(), CompletionToken() );
		if ( occlusion )
			(void)device.Release( occlusion.Value(), CompletionToken() );
	}

	pass.ReleaseDevice( device );
	(void)device.WaitIdle();
	return checks.Report();
}
