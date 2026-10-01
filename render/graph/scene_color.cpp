//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Graph-owned scene-color snapshot for transmission consumers.
//
//=============================================================================//

#include "render/graph/scene_color.h"
#include "render/graph/executor.h"

namespace render::graph
{

std::optional<ResourceRef> CaptureSceneColor( GraphBuilder &builder, ResourceRef source )
{
	if ( !source.IsValid() || source.index >= builder.Resources().size() )
		return std::nullopt;
	const ResourceDecl &resource = builder.Resources()[source.index];
	if ( !resource.isTexture )
		return std::nullopt;
	const device::TextureDesc sourceDesc = resource.texture;
	if ( sourceDesc.dimension != device::TextureDimension::k2D ||
	     ( sourceDesc.sampleCount != 1 && sourceDesc.sampleCount != 2 &&
	         sourceDesc.sampleCount != 4 && sourceDesc.sampleCount != 8 ) ||
	     sourceDesc.depthOrLayers != 1 || sourceDesc.mipLevels != 1 || sourceDesc.width == 0 ||
	     sourceDesc.height == 0 || device::IsDepthFormat( sourceDesc.format ) ||
	     device::IsBlockCompressed( sourceDesc.format ) )
		return std::nullopt;
	const std::uint64_t bytes =
	    device::RegionBytes( sourceDesc.format, sourceDesc.width, sourceDesc.height );
	if ( bytes == 0 )
		return std::nullopt;

	ResourceRef captureSource = source;
	if ( sourceDesc.sampleCount > 1 )
	{
		device::TextureDesc resolveDesc = sourceDesc;
		resolveDesc.sampleCount = 1;
		resolveDesc.usages = {};
		resolveDesc.debugName = {};
		captureSource = builder.CreateTexture( "scene-color-resolve", resolveDesc );
		builder.AddPass( "resolve-scene-color", PassKind::kRender )
		    .Write( source, device::ResourceUsage::kColorAttachment )
		    .Write( captureSource, device::ResourceUsage::kResolveDestination )
		    .Execute(
		        [source, captureSource, sourceDesc]( RecordContext &context )
		        {
			        device::ColorAttachment color;
			        color.texture = context.Texture( source );
			        color.resolve = context.Texture( captureSource );
			        color.load = device::LoadOp::kLoad;
			        device::RenderingDesc rendering;
			        rendering.colors = std::span( &color, 1 );
			        rendering.width = sourceDesc.width;
			        rendering.height = sourceDesc.height;
			        context.Encoder().BeginRendering( rendering );
			        context.Encoder().EndRendering();
		        } );
	}
	device::BufferDesc stagingDesc;
	stagingDesc.size = bytes;
	stagingDesc.memory = device::MemoryKind::kDeviceLocal;
	const ResourceRef staging = builder.CreateBuffer( "scene-color-staging", stagingDesc );
	device::TextureDesc snapshotDesc = sourceDesc;
	snapshotDesc.sampleCount = 1;
	snapshotDesc.usages = {};
	snapshotDesc.debugName = {};
	const ResourceRef snapshot = builder.CreateTexture( "scene-color-snapshot", snapshotDesc );
	const device::TextureBufferCopy region{ 0, 0, 0, sourceDesc.width, sourceDesc.height };
	builder.AddPass( "capture-scene-color", PassKind::kCopy )
	    .Read( captureSource, device::ResourceUsage::kCopySource )
	    .Write( staging, device::ResourceUsage::kCopyDestination )
	    .Execute(
	        [captureSource, staging, region]( RecordContext &context )
	        {
		        context.Encoder().CopyTextureToBuffer(
		            context.Texture( captureSource ), context.Buffer( staging ), region );
	        } );
	builder.AddPass( "stage-scene-color", PassKind::kCopy )
	    .Read( staging, device::ResourceUsage::kCopySource )
	    .Write( snapshot, device::ResourceUsage::kCopyDestination )
	    .Execute(
	        [staging, snapshot, region]( RecordContext &context )
	        {
		        context.Encoder().CopyBufferToTexture(
		            context.Buffer( staging ), context.Texture( snapshot ), region );
	        } );
	return snapshot;
}

foundation::Expected<RecordedSceneColor, device::DeviceError> RecordSceneColor(
    device::IRenderDevice2 &device, device::CommandEncoder &encoder, device::TextureId source,
    const device::TextureDesc &sourceDesc )
{
	GraphBuilder builder;
	const ResourceRef imported = builder.ImportTexture( "product scene color", source, sourceDesc,
	    device::ResourceUsage::kColorAttachment, device::ResourceUsage::kColorAttachment );
	const std::optional<ResourceRef> snapshot = CaptureSceneColor( builder, imported );
	if ( !snapshot )
		return foundation::MakeUnexpected( device::DeviceError{
		    device::DeviceStatus::kInvalidDescription, device::DeviceOperation::kCreateTexture } );
	builder.AddPass( "retain scene color for transmission", PassKind::kCopy )
	    .Read( *snapshot, device::ResourceUsage::kSampled )
	    .SideEffect()
	    .Execute( []( RecordContext & ) {} );
	auto compiled = CompileGraph( std::move( builder ) );
	if ( !compiled )
		return foundation::MakeUnexpected( device::DeviceError{
		    device::DeviceStatus::kInvalidDescription, device::DeviceOperation::kCreateTexture } );
	auto recorded = RecordInline( compiled.Value(), device, encoder );
	if ( !recorded )
		return foundation::MakeUnexpected( recorded.Error() );
	RecordedSceneColor result;
	result.texture = recorded.Value().Texture( *snapshot );
	result.desc = sourceDesc;
	result.desc.sampleCount = 1;
	result.desc.usages = { device::ResourceUsage::kSampled };
	result.resources = std::move( recorded ).Value();
	return result;
}

} // namespace render::graph
