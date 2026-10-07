//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The 3DS build checks every code in codes.h against libctru's
//			<3ds/gpu/enums.h> at compile time; elsewhere this file is empty.
//
//=============================================================================//

#include "render/device/pica_codes.h"

#if defined( __3DS__ )
#include <3ds/gpu/enums.h>

namespace render::device::pica_format
{

static_assert( test::kNever == GPU_NEVER && test::kAlways == GPU_ALWAYS &&
               test::kEqual == GPU_EQUAL && test::kNotEqual == GPU_NOTEQUAL &&
               test::kLess == GPU_LESS && test::kLessEqual == GPU_LEQUAL &&
               test::kGreater == GPU_GREATER && test::kGreaterEqual == GPU_GEQUAL );
static_assert(
    factor::kZero == GPU_ZERO && factor::kOne == GPU_ONE && factor::kSrcColor == GPU_SRC_COLOR &&
    factor::kOneMinusSrcColor == GPU_ONE_MINUS_SRC_COLOR && factor::kDstColor == GPU_DST_COLOR &&
    factor::kOneMinusDstColor == GPU_ONE_MINUS_DST_COLOR && factor::kSrcAlpha == GPU_SRC_ALPHA &&
    factor::kOneMinusSrcAlpha == GPU_ONE_MINUS_SRC_ALPHA && factor::kDstAlpha == GPU_DST_ALPHA &&
    factor::kOneMinusDstAlpha == GPU_ONE_MINUS_DST_ALPHA );
static_assert( kBlendAdd == GPU_BLEND_ADD );
static_assert( stencil::kKeep == GPU_STENCIL_KEEP && stencil::kZero == GPU_STENCIL_ZERO &&
               stencil::kReplace == GPU_STENCIL_REPLACE &&
               stencil::kIncrement == GPU_STENCIL_INCR && stencil::kDecrement == GPU_STENCIL_DECR &&
               stencil::kInvert == GPU_STENCIL_INVERT &&
               stencil::kIncrementWrap == GPU_STENCIL_INCR_WRAP &&
               stencil::kDecrementWrap == GPU_STENCIL_DECR_WRAP );
static_assert( cull::kNone == GPU_CULL_NONE && cull::kFrontCcw == GPU_CULL_FRONT_CCW &&
               cull::kBackCcw == GPU_CULL_BACK_CCW );
static_assert( texel::kRGBA8 == GPU_RGBA8 && texel::kL8 == GPU_L8 && texel::kETC1 == GPU_ETC1 &&
               texel::kETC1A4 == GPU_ETC1A4 );
static_assert( colorbuffer::kRGBA8 == GPU_RB_RGBA8 );
static_assert( depthbuffer::kDepth24Stencil8 == GPU_RB_DEPTH24_STENCIL8 );
static_assert( source::kPrimaryColor == GPU_PRIMARY_COLOR &&
               source::kFragmentPrimary == GPU_FRAGMENT_PRIMARY_COLOR &&
               source::kFragmentSecondary == GPU_FRAGMENT_SECONDARY_COLOR &&
               source::kTexture0 == GPU_TEXTURE0 && source::kTexture1 == GPU_TEXTURE1 &&
               source::kTexture2 == GPU_TEXTURE2 && source::kTexture3 == GPU_TEXTURE3 &&
               source::kPreviousBuffer == GPU_PREVIOUS_BUFFER &&
               source::kConstant == GPU_CONSTANT && source::kPrevious == GPU_PREVIOUS );
static_assert( combine::kReplace == GPU_REPLACE && combine::kModulate == GPU_MODULATE &&
               combine::kAdd == GPU_ADD && combine::kInterpolate == GPU_INTERPOLATE &&
               operand::kRgbColor == GPU_TEVOP_RGB_SRC_COLOR &&
               operand::kRgbAlpha == GPU_TEVOP_RGB_SRC_ALPHA &&
               operand::kAlpha == GPU_TEVOP_A_SRC_ALPHA && scale::k1 == GPU_TEVSCALE_1 &&
               scale::k2 == GPU_TEVSCALE_2 && scale::k4 == GPU_TEVSCALE_4 );
static_assert( kCombineLast == GPU_ADD_MULTIPLY && kCombineDot3Rgb == GPU_DOT3_RGB &&
               kCombineDot3Rgba == GPU_DOT3_RGBA && kScaleLast == GPU_TEVSCALE_4 );
static_assert(
    kRgbOperandLast == GPU_TEVOP_RGB_0x0F && kAlphaOperandLast == GPU_TEVOP_A_ONE_MINUS_SRC_B );

} // namespace render::device::pica_format
#endif
