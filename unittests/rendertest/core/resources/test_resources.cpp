//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.resources (RFC 0016) on render.device.null: staged texels
//			and mesh bytes land on the device, resources end in their use
//			usage, replacements keep the old resource live until the submission
//			that replaced it completes, and eviction and teardown release
//			everything. A staged mip chain uploads every level it is given
//			(R6).
//
//=============================================================================//

#include "render/device/null/provider.h"
#include "render/resources/mesh_cache.h"
#include "render/resources/texture_cache.h"
#include "testing/checks.h"

#include <algorithm>
#include <span>
#include <vector>

namespace
{

using namespace render;
using device::ResourceUsage;

std::vector<std::byte> Bytes( std::size_t size, std::uint8_t seed )
{
	std::vector<std::byte> bytes( size );
	for ( std::size_t i = 0; i < size; ++i )
		bytes[i] = static_cast<std::byte>( ( i * 13u + seed ) & 0xffu );
	return bytes;
}

// Submits encoder work and completes it.
device::CompletionToken Run( device::IRenderDevice2 &device, device::CommandEncoder &encoder )
{
	auto token = device.Submit( device::QueueKind::kGraphics, { &encoder, 1 }, {} );
	if ( !token )
		return {};
	return token.Value();
}

std::vector<std::byte> ReadTexture( device::IRenderDevice2 &device, device::TextureId texture,
    std::uint32_t width, std::uint32_t height, std::uint32_t mip = 0 )
{
	device::BufferDesc desc;
	desc.size = static_cast<std::uint64_t>( width ) * height * 4;
	desc.usages = { ResourceUsage::kCopyDestination };
	desc.memory = device::MemoryKind::kReadback;
	const device::BufferId buffer = device.CreateBuffer( desc ).Value();
	auto encoder = device.BeginEncoder( device::QueueKind::kGraphics ).Value();
	encoder.TransitionTexture( texture, ResourceUsage::kSampled, ResourceUsage::kCopySource );
	encoder.TransitionBuffer( buffer, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	encoder.CopyTextureToBuffer( texture, buffer, { 0, mip, 0, width, height } );
	encoder.TransitionTexture( texture, ResourceUsage::kCopySource, ResourceUsage::kSampled );
	const device::CompletionToken token = Run( device, encoder );
	if ( !token.NamesSubmission() )
		return {};
	device::null::Control( device )->CompleteThrough( token.queue, token.value );
	std::vector<std::byte> out( desc.size );
	(void)device.ReadBuffer( buffer, 0, out );
	(void)device.Release( buffer, token );
	return out;
}

} // namespace

int main()
{
	testing::Checks checks;
	device::null::NullOptions options;
	options.completion = device::null::CompletionMode::kManual;
	auto device = device::null::Create( options ).Value();
	auto *control = device::null::Control( *device );
	const std::size_t baseline = device->LiveResourceCount();
	{
		resources::TextureCache textures( *device );
		device::TextureDesc desc;
		desc.format = device::Format::kRGBA8Unorm;
		desc.width = 4;
		desc.height = 2;
		// The cache adds what it needs (copy destination, sampled); the test
		// reads texels back, so it asks for copy source too.
		desc.usages = { ResourceUsage::kCopySource };
		const std::vector<std::byte> pixels = Bytes( 32, 1 );
		auto staged = textures.Stage( "wall", desc, pixels );
		checks.That( staged.HasValue() && staged.Value().revision == 1, "R1.a-texture-stages" );
		checks.That( !textures.Stage( "short", desc, Bytes( 31, 0 ) ),
		    "R1.bytes-that-do-not-match-the-description-fail" );
		auto encoder = device->BeginEncoder( device::QueueKind::kGraphics ).Value();
		checks.Equal(
		    textures.RecordUploads( encoder ), std::size_t( 1 ), "R1.one-upload-recorded" );
		const device::CompletionToken first = Run( *device, encoder );
		checks.That( first.NamesSubmission(), "R1.the-upload-submits" );
		textures.Retire( first );
		control->CompleteThrough( first.queue, first.value );
		(void)device->Poll();
		const std::vector<std::byte> texels = ReadTexture( *device, staged.Value().texture, 4, 2 );
		checks.That( texels == pixels, "R1.staged-texels-land-and-the-texture-ends-sampled" );

		auto replaced = textures.Stage( "wall", desc, Bytes( 32, 2 ) );
		checks.That( replaced && replaced.Value().revision == 2 &&
		                 replaced.Value().texture != staged.Value().texture,
		    "R2.replacing-makes-a-new-revision" );
		auto second = device->BeginEncoder( device::QueueKind::kGraphics ).Value();
		(void)textures.RecordUploads( second );
		const device::CompletionToken token = Run( *device, second );
		textures.Retire( token );
		(void)device->Poll();
		const std::size_t live = device->LiveResourceCount();
		(void)device->Poll();
		checks.Equal( device->LiveResourceCount(), live,
		    "R2.the-old-texture-lives-until-its-replacement-completes" );
		control->CompleteThrough( token.queue, token.value );
		(void)device->Poll();
		checks.That( device->LiveResourceCount() < live, "R2.then-it-is-released" );
		checks.That( textures.Evict( "wall" ).HasValue() && !textures.Find( "wall" ) &&
		                 !textures.Evict( "wall" ),
		    "R3.evict-removes-a-name-once" );
		textures.Retire( token );

		// R6: a mip chain, every level uploaded in one submission.
		device::TextureDesc chain = desc;
		chain.width = 8;
		chain.height = 4;
		chain.mipLevels = 4; // 8x4, 4x2, 2x1, 1x1
		std::vector<std::vector<std::byte>> levels;
		for ( std::uint32_t m = 0; m < chain.mipLevels; ++m )
			levels.push_back(
			    Bytes( std::size_t( std::max( 8u >> m, 1u ) ) * std::max( 4u >> m, 1u ) * 4,
			        std::uint8_t( 10 + m ) ) );
		std::vector<std::span<const std::byte>> views( levels.begin(), levels.end() );
		auto mips = textures.StageMips( "chain", chain, views );
		checks.That( mips.HasValue() && mips.Value().desc.mipLevels == 4, "R6.a-mip-chain-stages" );
		std::vector<std::span<const std::byte>> wrong = views;
		const std::vector<std::byte> shortLevel = Bytes( 7, 0 );
		wrong[2] = shortLevel;
		checks.That( !textures.StageMips( "wrong", chain, wrong ) && !textures.Find( "wrong" ),
		    "R6.a-level-of-the-wrong-size-fails-and-stages-nothing" );
		device::TextureDesc fewer = chain;
		fewer.mipLevels = 2;
		checks.That( !textures.StageMips( "fewer", fewer, views ) &&
		                 !textures.StageMips( "none", chain, {} ),
		    "R6.more-levels-than-the-texture-has-or-none-fail" );
		auto third = device->BeginEncoder( device::QueueKind::kGraphics ).Value();
		checks.Equal( textures.RecordUploads( third ), std::size_t( 1 ),
		    "R6.one-upload-for-the-whole-chain" );
		control->ClearRecorded();
		const device::CompletionToken chainToken = Run( *device, third );
		control->CompleteThrough( chainToken.queue, chainToken.value );
		std::size_t copies = 0;
		for ( const auto &command : control->Recorded() )
			copies += command.op == device::null::RecordedOp::kCopyBufferToTexture ? 1 : 0;
		checks.Equal( copies, std::size_t( 4 ), "R6.one-copy-per-level" );
		textures.Retire( chainToken );
		bool landed = mips.HasValue();
		for ( std::uint32_t m = 0; landed && m < chain.mipLevels; ++m )
			landed = ReadTexture( *device, mips.Value().texture, std::max( 8u >> m, 1u ),
			             std::max( 4u >> m, 1u ), m ) == levels[m];
		checks.That( landed, "R6.every-level-lands-in-its-mip" );
		checks.That( textures.Evict( "chain" ).HasValue(), "R6.the-chain-evicts" );
		textures.Retire( chainToken );

		resources::MeshCache meshes( *device );
		const std::vector<std::byte> vertices = Bytes( 96, 3 );
		const std::vector<std::byte> indices = Bytes( 12, 4 );
		auto mesh =
		    meshes.Stage( "crate", { vertices, 32, indices, device::IndexFormat::kUint16 } );
		checks.That( mesh && mesh.Value().vertexCount == 3 && mesh.Value().indexCount == 6,
		    "R4.a-mesh-stages-with-its-counts" );
		checks.That(
		    !meshes.Stage( "bad", { Bytes( 95, 0 ), 32, {}, device::IndexFormat::kUint16 } ),
		    "R4.a-partial-vertex-fails" );
		auto upload = device->BeginEncoder( device::QueueKind::kGraphics ).Value();
		(void)meshes.RecordUploads( upload );
		control->ClearRecorded();
		const device::CompletionToken meshToken = Run( *device, upload );
		checks.That( meshToken.NamesSubmission(), "R4.the-mesh-upload-submits" );
		control->CompleteThrough( meshToken.queue, meshToken.value );
		// The buffers were created without copy-source usage, so the
		// recorded stream is the evidence: each written once, then left in
		// its use usage.
		bool vertexWritten = false;
		bool vertexReady = false;
		bool indexReady = false;
		for ( const auto &command : control->Recorded() )
		{
			if ( command.op == device::null::RecordedOp::kWriteBuffer &&
			     command.resource == mesh.Value().vertices.value )
				vertexWritten = command.count == vertices.size();
			if ( command.op == device::null::RecordedOp::kTransitionBuffer &&
			     command.resource == mesh.Value().vertices.value )
				vertexReady = command.after == ResourceUsage::kVertex;
			if ( command.op == device::null::RecordedOp::kTransitionBuffer &&
			     command.resource == mesh.Value().indices.value )
				indexReady = command.after == ResourceUsage::kIndex;
		}
		checks.That( vertexWritten, "R4.vertex-bytes-are-written" );
		checks.That( vertexReady && indexReady, "R4.buffers-end-in-kVertex-and-kIndex" );
		meshes.Retire( meshToken );
	}
	control->CompleteAll();
	(void)device->Poll();
	checks.Equal( device->LiveResourceCount(), baseline, "R5.teardown-releases-everything" );
	return checks.Report();
}
