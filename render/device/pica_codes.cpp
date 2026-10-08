//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.v2 values to PICA200 register codes (codes.h).
//
//=============================================================================//

#include "render/device/pica_codes.h"

namespace render::device::pica_format
{

std::uint8_t TestFunction( CompareOp op )
{
	switch ( op )
	{
	case CompareOp::kNever:
		return test::kNever;
	case CompareOp::kLess:
		return test::kLess;
	case CompareOp::kLessEqual:
		return test::kLessEqual;
	case CompareOp::kEqual:
		return test::kEqual;
	case CompareOp::kGreaterEqual:
		return test::kGreaterEqual;
	case CompareOp::kGreater:
		return test::kGreater;
	case CompareOp::kNotEqual:
		return test::kNotEqual;
	case CompareOp::kAlways:
		break;
	}
	return test::kAlways;
}

std::uint8_t StencilOperation( StencilOp op )
{
	switch ( op )
	{
	case StencilOp::kKeep:
		break;
	case StencilOp::kZero:
		return stencil::kZero;
	case StencilOp::kReplace:
		return stencil::kReplace;
	case StencilOp::kIncrementClamp:
		return stencil::kIncrement;
	case StencilOp::kDecrementClamp:
		return stencil::kDecrement;
	case StencilOp::kInvert:
		return stencil::kInvert;
	case StencilOp::kIncrementWrap:
		return stencil::kIncrementWrap;
	case StencilOp::kDecrementWrap:
		return stencil::kDecrementWrap;
	}
	return stencil::kKeep;
}

std::optional<BlendFactors> Blend( BlendMode mode )
{
	switch ( mode )
	{
	case BlendMode::kOpaque:
		return BlendFactors{};
	case BlendMode::kAlpha:
		return BlendFactors{ factor::kSrcAlpha, factor::kOneMinusSrcAlpha, factor::kSrcAlpha,
		    factor::kOneMinusSrcAlpha };
	case BlendMode::kPremultiplied:
		return BlendFactors{
		    factor::kOne, factor::kOneMinusSrcAlpha, factor::kOne, factor::kOneMinusSrcAlpha };
	case BlendMode::kAdditive:
		return BlendFactors{ factor::kOne, factor::kOne, factor::kOne, factor::kOne };
	case BlendMode::kTransmittance:
		// src + dst * a in colour, the destination's alpha kept.
		return BlendFactors{ factor::kOne, factor::kSrcAlpha, factor::kZero, factor::kOne };
	case BlendMode::kModulate2x:
		// 2 * src * dst = src * dst + dst * src; destination alpha kept.
		return BlendFactors{ factor::kDstColor, factor::kSrcColor, factor::kZero, factor::kOne };
	case BlendMode::kAlphaAdditive:
		return BlendFactors{ factor::kSrcAlpha, factor::kOne, factor::kSrcAlpha, factor::kOne };
	}
	return std::nullopt;
}

std::uint8_t CullMode( const RasterState &raster )
{
	switch ( raster.cull )
	{
	case render::device::CullMode::kNone:
		return cull::kNone;
	case render::device::CullMode::kBack:
		return raster.frontCounterClockwise ? cull::kBackCcw : cull::kFrontCcw;
	case render::device::CullMode::kFront:
		return raster.frontCounterClockwise ? cull::kFrontCcw : cull::kBackCcw;
	}
	return cull::kNone;
}

std::optional<std::uint8_t> SampledFormat( Format format )
{
	switch ( format )
	{
	case Format::kRGBA8Unorm:
	case Format::kBGRA8Unorm: // repacked on upload; the GPU has one RGBA8 order
	case Format::kR8Unorm:    // stored RGBA8 (texel_layout.h): samples (r, 0, 0, 1)
		return texel::kRGBA8;
	case Format::kETC1Rgb:
		return texel::kETC1;
	case Format::kETC1A4:
		return texel::kETC1A4;
	case Format::kRGBA4Unorm:
		return texel::kRGBA4;
	default:
		return std::nullopt;
	}
}

std::optional<std::uint8_t> ColorBufferFormat( Format format )
{
	switch ( format )
	{
	case Format::kRGBA8Unorm:
	case Format::kBGRA8Unorm:
		return colorbuffer::kRGBA8;
	default:
		return std::nullopt;
	}
}

std::optional<std::uint8_t> DepthBufferFormat( Format format )
{
	if ( format == Format::kD24UnormS8 )
		return depthbuffer::kDepth24Stencil8;
	return std::nullopt;
}

} // namespace render::device::pica_format
