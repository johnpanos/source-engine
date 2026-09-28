//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.material.programs (RFC 0016 K4, headless): GroupResidency,
//			MaterialPrograms and DrawGroups on render.device.null, which
//			validates every binding and transition and counts live objects.
//
//			- G1 a group exists only after RecordUploads, naming its texture
//			  and its uniform buffer; the constants upload is recorded once;
//			- G2 a group whose texture is absent resolves to nothing until the
//			  texture is staged;
//			- G3 a replaced texture gives a new group naming the new texture,
//			  and the old group is released behind the token;
//			- G4 an evicted texture takes the group away;
//			- G5 Set replaces a group and uploads its constants again;
//			- G6 Remove takes the group away and releases it;
//			- G7 invalid requests are refused;
//			- G8 samplers are shared by description;
//			- G9 destruction releases every object;
//			- G10 a program carries its pipeline, stride, draw-constant size,
//			  draw layout and material group; G11 a draw group its layout.
//
//=============================================================================//

#include "render/device/null/provider.h"
#include "render/material/material_programs.h"
#include "testing/checks.h"

#include <array>
#include <memory>

namespace
{

using namespace render;
using namespace render::material;

device::TextureDesc TexelDesc()
{
	device::TextureDesc desc;
	desc.format = device::Format::kRGBA8Srgb;
	desc.width = desc.height = 1;
	desc.usages = { device::ResourceUsage::kCopyDestination, device::ResourceUsage::kSampled };
	return desc;
}

bool StageTexel( resources::TextureCache &cache, const char *name, std::uint8_t value )
{
	const std::array<std::byte, 4> texel = {
	    std::byte( value ), std::byte( value ), std::byte( value ), std::byte( 255 ) };
	return cache.Stage( name, TexelDesc(), texel ).HasValue();
}

// One submission: the textures' uploads, then the residency's; waits and
// retires both, then polls so released objects are gone.
template <typename Owner>
std::size_t Cycle( device::IRenderDevice2 &device, resources::TextureCache &textures, Owner &owner )
{
	auto encoder = device.BeginEncoder( device::QueueKind::kGraphics );
	if ( !encoder )
		return 0;
	textures.RecordUploads( encoder.Value() );
	const std::size_t recorded = owner.RecordUploads( encoder.Value() );
	device::CommandEncoder encoders[] = { std::move( encoder ).Value() };
	auto token = device.Submit( device::QueueKind::kGraphics, encoders, {} );
	if ( !token )
		return 0;
	while ( !device.IsComplete( token.Value() ) )
		(void)device.Poll();
	textures.Retire( token.Value() );
	owner.Retire( token.Value() );
	(void)device.Poll();
	return recorded;
}

device::BindGroupLayoutId MaterialLayout(
    device::IRenderDevice2 &device, device::BindGroupRole role )
{
	static const device::BindingDesc bindings[] = {
	    { 0, device::BindingKind::kUniformBuffer, 1, { device::ShaderStage::kFragment } },
	    { 1, device::BindingKind::kSampledTexture, 1, { device::ShaderStage::kFragment } },
	    { 2, device::BindingKind::kSampler, 1, { device::ShaderStage::kFragment } } };
	auto layout = device.CreateBindGroupLayout( { role, bindings } );
	return layout ? layout.Value() : device::BindGroupLayoutId();
}

GroupRequest Request( device::BindGroupLayoutId layout, const char *texture, float constant = 1.0f )
{
	GroupRequest request;
	request.layout = layout;
	const float constants[4] = { constant, 0.0f, 0.0f, 1.0f };
	const auto bytes = std::as_bytes( std::span( constants ) );
	request.constants.assign( bytes.begin(), bytes.end() );
	request.textures.push_back( { 1, texture, 2, {} } );
	return request;
}

} // namespace

int main()
{
	testing::Checks checks;
	auto device = device::null::Create( {} ).Value();
	const device::BindGroupLayoutId layout =
	    MaterialLayout( *device, device::BindGroupRole::kMaterial );
	const device::BindGroupLayoutId drawLayout =
	    MaterialLayout( *device, device::BindGroupRole::kDraw );
	if ( !checks.That( layout.IsValid() && drawLayout.IsValid(), "setup.layouts" ) )
		return checks.Report();
	const std::size_t baseline = device->LiveResourceCount();
	{
		resources::TextureCache textures( *device );
		GroupResidency groups( *device, textures );

		// G1.
		checks.That( StageTexel( textures, "white", 255 ) &&
		                 groups.Set( 1, Request( layout, "white" ) ).HasValue(),
		    "G1.a-group-is-set" );
		checks.That( groups.Group( 1 ) == nullptr, "G1.no-group-before-record-uploads" );
		checks.Equal( Cycle( *device, textures, groups ), std::size_t( 1 ),
		    "G1.the-constants-upload-is-recorded" );
		const ResidentGroup *first = groups.Group( 1 );
		const resources::TextureEntry *white = textures.Find( "white" );
		checks.That( first && white && first->textures.size() == 1 &&
		                 first->textures[0].texture == white->texture &&
		                 first->uniforms.size() == 1,
		    "G1.the-group-names-its-texture-and-uniform-buffer" );
		checks.Equal(
		    Cycle( *device, textures, groups ), std::size_t( 0 ), "G1.constants-upload-once" );
		const device::BindGroupId firstGroup = first ? first->group : device::BindGroupId();
		checks.That( groups.Group( 1 ) && groups.Group( 1 )->group == firstGroup,
		    "G1.an-unchanged-group-is-kept" );

		// G2.
		checks.That( groups.Set( 2, Request( layout, "gray" ) ).HasValue(), "G2.set" );
		(void)Cycle( *device, textures, groups );
		checks.That( groups.Group( 2 ) == nullptr && groups.ReadyCount() == 1,
		    "G2.an-absent-texture-means-no-group" );
		(void)StageTexel( textures, "gray", 128 );
		(void)Cycle( *device, textures, groups );
		checks.That( groups.Group( 2 ) != nullptr, "G2.the-group-follows-the-staged-texture" );

		// G3.
		const std::size_t steady = device->LiveResourceCount();
		(void)StageTexel( textures, "white", 250 );
		(void)Cycle( *device, textures, groups );
		const ResidentGroup *replaced = groups.Group( 1 );
		checks.That( replaced && replaced->group != firstGroup &&
		                 replaced->textures[0].texture == textures.Find( "white" )->texture,
		    "G3.a-replaced-texture-gives-a-new-group" );
		(void)Cycle( *device, textures, groups );
		checks.Equal( device->LiveResourceCount(), steady,
		    "G3.the-old-group-and-texture-are-released-behind-the-token" );

		// G4.
		(void)textures.Evict( "gray" );
		(void)Cycle( *device, textures, groups );
		checks.That( groups.Group( 2 ) == nullptr, "G4.an-evicted-texture-takes-the-group-away" );

		// G5.
		checks.That( groups.Set( 1, Request( layout, "white", 0.5f ) ).HasValue(), "G5.replace" );
		checks.Equal( Cycle( *device, textures, groups ), std::size_t( 1 ),
		    "G5.replaced-constants-upload-again" );
		checks.That( groups.Group( 1 ) != nullptr, "G5.the-replaced-group-is-ready" );

		// G6.
		groups.Remove( 1 );
		checks.That( groups.Group( 1 ) == nullptr && groups.Count() == 1, "G6.remove" );

		// G7.
		GroupRequest noLayout = Request( device::BindGroupLayoutId(), "white" );
		checks.That( !groups.Set( 3, noLayout ), "G7.a-group-without-a-layout-is-refused" );

		// G8.
		const std::size_t before = device->LiveResourceCount();
		GroupRequest point = Request( layout, "white" );
		point.textures[0].sampler.magFilter = device::Filter::kNearest;
		(void)groups.Set( 4, point );
		const std::size_t afterOne = device->LiveResourceCount();
		(void)groups.Set( 5, point );
		checks.Equal( device->LiveResourceCount() - afterOne, afterOne - before - 1,
		    "G8.a-second-group-with-the-same-sampler-creates-no-sampler" );
		(void)Cycle( *device, textures, groups );

		// G10, G11.
		MaterialPrograms programs( *device, textures );
		DrawGroups draws( *device, textures );
		ProgramRequest program;
		program.pipeline = device::PipelineId( 7 ); // stored, never used here
		program.vertexStride = 24;
		program.drawConstantBytes = 64;
		program.drawLayout = drawLayout;
		program.material = Request( layout, "white" );
		checks.That( programs.Set( 1, program ).HasValue() &&
		                 draws.Set( 9, Request( drawLayout, "white" ) ).HasValue(),
		    "G10.set" );
		checks.That( !programs.Program( 1 ) && !draws.Group( 9 ), "G10.nothing-before-uploads" );
		(void)Cycle( *device, textures, programs );
		(void)Cycle( *device, textures, draws );
		const DrawProgram *drawn = programs.Program( 1 );
		checks.That( drawn && drawn->pipeline == program.pipeline && drawn->vertexStride == 24 &&
		                 drawn->drawConstantBytes == 64 && drawn->drawLayout == drawLayout &&
		                 drawn->material.group.IsValid() && drawn->material.textures.size() == 1,
		    "G10.a-program-carries-its-pipeline-stride-constants-layout-and-group" );
		const DrawGroup *draw = draws.Group( 9 );
		checks.That( draw && draw->layout == drawLayout && draw->resident.group.IsValid(),
		    "G11.a-draw-group-carries-its-layout-and-group" );
		ProgramRequest bad = program;
		bad.pipeline = device::PipelineId();
		checks.That( !programs.Set( 2, bad ), "G7.a-program-without-a-pipeline-is-refused" );
		bad = program;
		bad.vertexStride = 0;
		checks.That( !programs.Set( 2, bad ), "G7.a-program-with-stride-0-is-refused" );
		bad = program;
		bad.drawConstantBytes = 132;
		checks.That( !programs.Set( 2, bad ), "G7.oversized-draw-constants-are-refused" );
		bad = program;
		bad.drawConstantBytes = 6;
		checks.That(
		    !programs.Set( 2, bad ), "G7.draw-constants-not-a-multiple-of-four-are-refused" );
		checks.That(
		    !programs.Program( 2 ) && programs.Count() == 1, "G7.refusals-change-nothing" );
		programs.Remove( 1 );
		checks.That( !programs.Program( 1 ), "G6.a-removed-program-is-gone" );
		checks.Equal( groups.Failures() + programs.GroupFailures() + draws.GroupFailures(), 0u,
		    "G1.no-group-failures" );
	}
	(void)device->Poll();
	checks.Equal( device->LiveResourceCount(), baseline, "G9.destruction-releases-every-object" );
	return checks.Report();
}
