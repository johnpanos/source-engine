//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device (the port): the one map from render.device.v2 values
//			to PICA200 register codes (RFC 0026 decision 5), which the kPica
//			artifact forms name and the PICA adapter programs. The codes are
//			the GPU's, as libctru names them in <3ds/gpu/enums.h>; the 3DS
//			build checks each against that header (render/device/pica/codes_check.cpp).
//			Portable: no 3DS SDK.
//
//=============================================================================//

#ifndef RENDER_DEVICE_PICA_CODES_H
#define RENDER_DEVICE_PICA_CODES_H

#include "render/device/pipeline.h"
#include "render/device/resources.h"

#include <cstdint>
#include <optional>

namespace render::device::pica_format
{

// GPU_TESTFUNC
namespace test
{
inline constexpr std::uint8_t kNever = 0, kAlways = 1, kEqual = 2, kNotEqual = 3, kLess = 4,
                              kLessEqual = 5, kGreater = 6, kGreaterEqual = 7;
}
// GPU_BLENDFACTOR
namespace factor
{
inline constexpr std::uint8_t kZero = 0, kOne = 1, kSrcColor = 2, kOneMinusSrcColor = 3,
                              kDstColor = 4, kOneMinusDstColor = 5, kSrcAlpha = 6,
                              kOneMinusSrcAlpha = 7, kDstAlpha = 8, kOneMinusDstAlpha = 9;
}
// GPU_BLENDEQUATION
inline constexpr std::uint8_t kBlendAdd = 0;
// GPU_STENCILOP
namespace stencil
{
inline constexpr std::uint8_t kKeep = 0, kZero = 1, kReplace = 2, kIncrement = 3, kDecrement = 4,
                              kInvert = 5, kIncrementWrap = 6, kDecrementWrap = 7;
}
// GPU_CULLMODE
namespace cull
{
inline constexpr std::uint8_t kNone = 0, kFrontCcw = 1, kBackCcw = 2;
}
// GPU_TEXCOLOR and GPU_COLORBUF / GPU_DEPTHBUF
namespace texel
{
inline constexpr std::uint8_t kRGBA8 = 0, kRGBA4 = 4, kL8 = 7, kETC1 = 0xC, kETC1A4 = 0xD;
}
namespace colorbuffer
{
inline constexpr std::uint8_t kRGBA8 = 0;
}
namespace depthbuffer
{
inline constexpr std::uint8_t kDepth24Stencil8 = 3;
}
// GPU_TEVSRC
namespace source
{
inline constexpr std::uint8_t kPrimaryColor = 0x0, kFragmentPrimary = 0x1, kFragmentSecondary = 0x2,
                              kTexture0 = 0x3, kTexture1 = 0x4, kTexture2 = 0x5, kTexture3 = 0x6,
                              kPreviousBuffer = 0xD, kConstant = 0xE, kPrevious = 0xF;
}
// GPU_COMBINEFUNC, GPU_TEVOP_RGB / GPU_TEVOP_A and GPU_TEVSCALE by name
namespace combine
{
inline constexpr std::uint8_t kReplace = 0x0, kModulate = 0x1, kAdd = 0x2, kInterpolate = 0x4;
}
namespace operand
{
inline constexpr std::uint8_t kRgbColor = 0x0, kRgbAlpha = 0x2, kAlpha = 0x0;
}
namespace scale
{
inline constexpr std::uint8_t k1 = 0x0, k2 = 0x1, k4 = 0x2;
}
inline constexpr std::uint8_t kCombineLast = 0x9;     // GPU_ADD_MULTIPLY
inline constexpr std::uint8_t kCombineDot3Rgb = 0x6;  // GPU_DOT3_RGB
inline constexpr std::uint8_t kCombineDot3Rgba = 0x7; // GPU_DOT3_RGBA
inline constexpr std::uint8_t kScaleLast = 0x2;       // GPU_TEVSCALE_4
inline constexpr std::uint8_t kRgbOperandLast = 0xF;
inline constexpr std::uint8_t kAlphaOperandLast = 0x7;

std::uint8_t TestFunction( CompareOp op );
std::uint8_t StencilOperation( StencilOp op );

struct BlendFactors
{
	std::uint8_t colorSource = factor::kOne;
	std::uint8_t colorDestination = factor::kZero;
	std::uint8_t alphaSource = factor::kOne;
	std::uint8_t alphaDestination = factor::kZero;
	friend constexpr bool operator==( const BlendFactors &, const BlendFactors & ) = default;
};
// Equations are always kBlendAdd; nullopt for a mode the GPU cannot express
// exactly (none of today's modes).
std::optional<BlendFactors> Blend( BlendMode mode );

// The cull register for a port raster state. PVS1 programs keep the port's
// winding (they flip no axis; RFC 0026 decision 3), so the GPU's
// counter-clockwise front matches frontCounterClockwise directly.
std::uint8_t CullMode( const RasterState &raster );

// Texel and buffer codes; nullopt for a format the adapter refuses
// (kUnsupported, RFC 0026 decision 4). SampledFormat is the GPU format the
// texture is stored in (texel_layout.h).
std::optional<std::uint8_t> SampledFormat( Format format );
std::optional<std::uint8_t> ColorBufferFormat( Format format );
std::optional<std::uint8_t> DepthBufferFormat( Format format );

} // namespace render::device::pica_format

#endif // RENDER_DEVICE_PICA_CODES_H
