//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Render-target copies of the core shader API (core_copies.cpp).
//
//===========================================================================//

#ifndef SHADERAPIPICA_CORE_COPIES_H
#define SHADERAPIPICA_CORE_COPIES_H

namespace pica
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
CopyResult CopyTargetRegion( Texture &destination, const CopyRect &region );

} // namespace pica

#endif // SHADERAPIPICA_CORE_COPIES_H
