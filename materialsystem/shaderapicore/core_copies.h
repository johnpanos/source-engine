//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Render-target copies of the core shader API (core_copies.cpp).
//
//===========================================================================//

#ifndef SHADERAPIPICA_CORE_COPIES_H
#define SHADERAPIPICA_CORE_COPIES_H

namespace render::legacy
{
class ICorePassRecorder;
}

namespace corefacade
{

class Texture;

// Texels from the top left.
struct CopyRect
{
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
};

// The depth-alpha half of a frame copy (D3D9's WRITE_DEPTH_TO_DESTALPHA,
// render/legacy/depth_alpha.h): the draw projection's z column (zScale,
// zOffset, wScale, wOffset) and the dest-alpha depth range.
struct CopyDepthAlpha
{
	float projection[4] = {};
	float range = 0.0f;
};

enum class CopyResult
{
	kCopied,  // recorded (or nothing to copy after clipping)
	kRefused, // outside a frame, a destination that is not a render target,
	          // or the target being drawn
};

// Copies `region` of the target being drawn (the screen when none) into the
// same texels of `destination`, a render target, in the frame's order (a
// core section, D37). The region is clipped to both. The destination is
// sampleable afterwards.
// With `depthAlpha` and the frontend's `recorder`, the copy's alpha then
// takes the target's depth (ICorePassRecorder::RecordDepthAlpha), as soft
// particles read it.
CopyResult CopyTargetRegion( Texture &destination, const CopyRect &region,
    const CopyDepthAlpha *depthAlpha = nullptr,
    render::legacy::ICorePassRecorder *recorder = nullptr );

} // namespace corefacade

#endif // SHADERAPIPICA_CORE_COPIES_H
