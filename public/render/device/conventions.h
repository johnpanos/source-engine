//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The fixed conventions of render.device.v2 (RFC 0016 decision 6).
//			Every adapter presents these; one whose API differs corrects it
//			privately (OpenGL uses glClipControl). Shaders never branch on the
//			backend.
//
//=============================================================================//

#ifndef RENDER_DEVICE_CONVENTIONS_H
#define RENDER_DEVICE_CONVENTIONS_H

namespace render::device::conventions
{

// Clip-space depth runs from 0 (near) to 1 (far).
inline constexpr float kClipDepthNear = 0.0f;
inline constexpr float kClipDepthFar = 1.0f;
// Clip-space +Y points up the framebuffer.
inline constexpr bool kClipYUp = true;
// Framebuffer and texture row 0 is the top row.
inline constexpr bool kOriginTopLeft = true;
// Texel centers lie at half-integer coordinates.
inline constexpr float kTexelCenter = 0.5f;

} // namespace render::device::conventions

#endif // RENDER_DEVICE_CONVENTIONS_H
