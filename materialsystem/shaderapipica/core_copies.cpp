//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Render-target copies of the core shader API, recorded in the
// frame's order through a core section (RFC 0016 K9, R91: what
// shaderapivulkan's stream copies did).
//
//===========================================================================//

#include "core_copies.h"

#include "pica_renderer.h"

#include "render/device/device.h"
#include "render/device/encoder.h"
#include "render/legacy/core_passes.h"
#include "render/legacy/depth_alpha.h"

#include <algorithm>

namespace pica
{

using render::device::CommandEncoder;
using render::device::ResourceUsage;
using render::device::TextureCopy;
using render::device::TextureId;

CopyResult CopyTargetRegion( Texture &destination, const CopyRect &region,
    const CopyDepthAlpha *depthAlpha, render::legacy::ICorePassRecorder *recorder )
{
	if ( !InFrame() || !destination.Valid() || !destination.IsTarget() )
		return CopyResult::kRefused;
	CoreSectionTarget target;
	CommandEncoder *encoder = BeginCoreSection( target );
	if ( !encoder )
		return CopyResult::kRefused;
	// The region, clipped to the source (the current target) and the
	// destination, at the same offset in both (D37).
	const int x0 = std::max( region.x, 0 );
	const int y0 = std::max( region.y, 0 );
	const int x1 = std::min( { region.x + region.width, int( target.width ),
	    destination.Width() } );
	const int y1 = std::min( { region.y + region.height, int( target.height ),
	    destination.Height() } );
	if ( destination.Id() == target.color || x0 >= x1 || y0 >= y1 )
	{
		EndCoreSection();
		return destination.Id() == target.color ? CopyResult::kRefused : CopyResult::kCopied;
	}
	const TextureId source{ target.color };
	const TextureId copyTo{ destination.Id() };
	TextureCopy copy;
	copy.x = std::uint32_t( x0 );
	copy.y = std::uint32_t( y0 );
	copy.width = std::uint32_t( x1 - x0 );
	copy.height = std::uint32_t( y1 - y0 );
	encoder->TransitionTexture(
	    source, ResourceUsage::kColorAttachment, ResourceUsage::kCopySource );
	encoder->TransitionTexture(
	    copyTo, ResourceUsage( destination.Usage() ), ResourceUsage::kCopyDestination );
	encoder->CopyTexture( source, copyTo, copy );
	encoder->TransitionTexture(
	    source, ResourceUsage::kCopySource, ResourceUsage::kColorAttachment );
	if ( depthAlpha && recorder && target.depth )
	{
		{
			render::legacy::DepthAlphaCopy request;
			request.device = target.device;
			request.submitted = render::device::CompletionToken{
			    render::device::QueueKind::kGraphics, target.submittedEpoch, target.submittedValue };
			request.recording = target.serial;
			request.target = copyTo;
			request.targetFormat = render::device::Format::kRGBA8Unorm;
			request.targetWidth = std::uint32_t( destination.Width() );
			request.targetHeight = std::uint32_t( destination.Height() );
			request.x = copy.x;
			request.y = copy.y;
			request.width = copy.width;
			request.height = copy.height;
			request.depth = TextureId{ target.depth };
			request.depthUsage = ResourceUsage::kDepthWrite;
			for ( int i = 0; i < 4; ++i )
				request.projection[i] = depthAlpha->projection[i];
			request.range = depthAlpha->range;
			(void)recorder->RecordDepthAlpha( *encoder, request );
		}
	}
	encoder->TransitionTexture( copyTo, ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
	destination.SetUsage( std::uint8_t( ResourceUsage::kSampled ) );
	EndCoreSection();
	return CopyResult::kCopied;
}

} // namespace pica
