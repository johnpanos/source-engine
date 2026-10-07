//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The `unlit` material family's program (RFC 0016 K4); see
//			unlit_family.h.
//
//=============================================================================//

#include "render/material/unlit_family.h"

#include "family_program.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <span>
#include <string_view>

namespace render::material
{

namespace
{

using namespace render::device;
using detail::ReadFlag;
using detail::ReadParameter;
using detail::SourceGammaToLinear;

// The parameters the family draws, and the ones the caller owns. $selfillum
// is read and has no effect: UnlitGeneric clears MATERIAL_VAR_SELFILLUM at
// init (vertexlitgeneric_dx9_helper.cpp), since an unlit surface already is
// its base color.
// The SpriteCard and Sprite rows of the family's schema: UnlitGeneric
// declares none of them, so an UnlitGeneric material that carries one draws
// as if it did not.
constexpr std::array<std::string_view, 54> kClaimed = { "gammacolorread", "linearwrite",
    "addbasetexture2",
    "addoverblend", "mod2x", "dualsequence", "sequence_blend_mode", "maxlumframeblend1",
    "maxlumframeblend2", "ramptexture", "zoomanimateseq2", "extractgreenalpha", "useinstancing",
    "nosrgb", "orientation", "overbrightfactor",
    "addself", "minsize", "startfadesize", "endfadesize", "maxdistance", "farfadeinterval",
    "spriteorientation", "spriterendermode", "blendframes", "basetexture", "color", "alpha",
    "vertexcolor", "vertexalpha", "alphatest", "alphatestreference", "translucent", "additive",
    "model", "nofog", "nocull", "texture2", "frame2", "texture2transform", "ignorez",
    "hdrcolorscale", "hdrbasetexture", "basetexturetransform", "decal", "frame", "depthblend",
    "depthblendscale", "selfillum", "color2", "splinetype", "maxsize", "spriteorigin" };

} // namespace

UnlitClaim ClaimUnlit( const ParameterBlock &block )
{
	UnlitClaim claim;
	const FamilySchema &family = block.Family();
	if ( family.desc.name != "unlit" )
	{
		claim.reason = "the block is of family " + family.desc.name;
		return claim;
	}
	if ( std::optional<std::string> unclaimed = detail::UnclaimedParameter( block, kClaimed ) )
	{
		claim.reason = "the family does not draw " + *unclaimed;
		return claim;
	}
	const bool translucent = ReadFlag( block, "translucent" );
	const bool additive = ReadFlag( block, "additive" );
	// Gameplay fades (including area-portal covers) modulate $alpha even
	// when the authored material has no $translucent flag.
	const bool alphaBlended =
	    translucent || ReadFlag( block, "vertexalpha" ) || ReadParameter( block, "alpha" ) < 1.0f;
	// $translucent with $additive blends src-alpha/one, as the legacy
	// shaders' additive translucent state does (render.device.v2's
	// kAlphaAdditive).
	claim.blend = additive && translucent ? BlendMode::kAlphaAdditive
	              : additive              ? BlendMode::kAdditive
	              : alphaBlended          ? BlendMode::kAlpha
	                                      : BlendMode::kOpaque;
	claim.alphaWrite = !alphaBlended && !ReadFlag( block, "alphatest" );
	claim.ignoreDepth = ReadFlag( block, "ignorez" );
	claim.depthBlend = ReadFlag( block, "depthblend" );
	// $gammacolorread 1 reads the base without sRGB decoding
	// (vertexlitgeneric_dx9_helper.cpp's EnableSRGBRead).
	claim.baseSrgb = int( ReadParameter( block, "gammacolorread" ) ) != 1;
	// $linearwrite 1 turns off the sRGB write. After a $gammacolorread 1 read
	// with no color modulation the bytes pass through unchanged, which the
	// linear pipeline draws as the decoded base, encoded on output (the menu
	// chapter images). Any modulation in between would happen in the wrong
	// space, so that is refused by name.
	if ( int( ReadParameter( block, "linearwrite" ) ) == 1 )
	{
		bool modulated = ReadFlag( block, "vertexcolor" );
		for ( int c = 0; c < 3; ++c )
			modulated = modulated || ReadParameter( block, "color", c ) != 1.0f ||
			            ReadParameter( block, "color2", c ) != 1.0f;
		if ( claim.baseSrgb || modulated )
		{
			claim.reason = "$linearwrite needs $gammacolorread 1 and no color modulation";
			return claim;
		}
		claim.baseSrgb = true;
	}
	const float depthScale = ReadParameter( block, "depthblendscale" );
	if ( claim.depthBlend && ( !std::isfinite( depthScale ) || depthScale <= 0.0f ) )
	{
		claim.reason = "$depthblend needs a finite positive $depthblendscale";
		return claim;
	}
	SurfaceConstants &constants = claim.constants;
	constants.surfaceControls[0] = ReadFlag( block, "nofog" ) ? 1.0f : 0.0f;
	constants.surfaceControls[1] = ReadParameter( block, "hdrcolorscale" );
	constants.surfaceControls[2] = claim.depthBlend ? 1.0f : 0.0f;
	constants.surfaceControls[3] = depthScale;
	// ComputeModulationColor: $color times $color2 (ApplyColor2Factor), then
	// the shader's gamma-to-linear conversion of the product.
	for ( int c = 0; c < 3; ++c )
	{
		const float tint = ReadParameter( block, "color", c ) * ReadParameter( block, "color2", c );
		if ( !std::isfinite( tint ) || tint < 0.0f )
		{
			claim.reason = "$color times $color2 must be finite and nonnegative";
			return claim;
		}
		constants.tint[c] = SourceGammaToLinear( tint );
	}
	constants.tint[3] = ReadParameter( block, "alpha" );
#if defined( RENDER_MATERIAL_UNLIT_SEEDED_IGNORE_VERTEX_COLOR )
	constants.flags[0] = 0.0f;
#else
	constants.flags[0] = ReadFlag( block, "vertexcolor" ) ? 1.0f : 0.0f;
#endif
	constants.flags[1] = ReadFlag( block, "alphatest" ) ? 1.0f : 0.0f;
	constants.flags[2] = detail::AlphaTestReference( block );
	constants.state[1] = 1.0f; // UnlitGeneric decodes vertex colors per vertex
	constants.state[3] = ReadFlag( block, "vertexalpha" ) ? 1.0f : 0.0f;
	for ( std::size_t i = 0; i < 8; ++i )
		constants.baseTransform[i] = ReadParameter( block, "basetexturetransform", i );
	claim.twoTexture = detail::TextureBound( block, "texture2" );
	constants.meshModes[0] = claim.twoTexture ? 1.0f : 0.0f;
	if ( claim.twoTexture )
	{
		for ( std::size_t i = 0; i < 8; ++i )
			constants.texture2Transform[i] = ReadParameter( block, "texture2transform", i );
	}
	claim.claimed = true;
	return claim;
}

UnlitClaim ClaimSprite( const ParameterBlock &block )
{
	UnlitClaim claim;
	if ( block.Family().desc.name != "unlit" )
	{
		claim.reason = "the sprite block is of family " + block.Family().desc.name;
		return claim;
	}
	// Orientation/origin have already been expanded into the submitted quad.
	// Material flags classify sprites but the shader's render-mode switch owns
	// blending, vertex color and depth. Keep that policy here, not in the bridge.
	constexpr std::string_view keys[] = { "basetexture", "frame", "color", "alpha", "model",
	    "nocull", "nofog", "vertexcolor", "vertexalpha", "translucent", "additive", "spriteorigin",
	    "spriteorientation", "spriterendermode", "ignorevertexcolors", "nosrgb", "hdrcolorscale",
	    "ignorez", "overbrightfactor" }; // Sprite declares no $overbrightfactor (SpriteCard does)
	if ( const auto unread = detail::UnclaimedParameter( block, keys ) )
	{
		claim.reason = "the sprite point does not draw " + *unread;
		return claim;
	}
	const int mode = int( ReadParameter( block, "spriterendermode" ) );
	const bool glow = mode == 3 || mode == 9;
	const bool additive = mode == 5 || glow;
	if ( mode < 0 || ( mode > 5 && mode != 9 ) )
	{
		claim.reason =
		    "$spriterendermode " + std::to_string( mode ) + " needs an unimplemented sprite point";
		return claim;
	}
	claim.blend = additive ? BlendMode::kAlphaAdditive
	              : mode   ? BlendMode::kAlpha
	                       : BlendMode::kOpaque;
	claim.alphaWrite = false;
	// $ignorez: CBaseShader::SetInitialShadowState turns the depth test and
	// writes off before the sprite's render mode sets its state, which only
	// the glow modes change again.
	claim.ignoreDepth = glow || ReadFlag( block, "ignorez" );
	claim.fogToBlack = additive;
	claim.baseSrgb = !ReadFlag( block, "nosrgb" );
	const bool vertexColor = mode != 0 && ( mode != 5 || !ReadFlag( block, "ignorevertexcolors" ) );
	SurfaceConstants &constants = claim.constants;
	constants.flags[0] = vertexColor ? 1.0f : 0.0f;
	constants.state[1] = claim.baseSrgb ? 1.0f : 0.0f;
	constants.state[3] = vertexColor ? 1.0f : 0.0f;
	constants.surfaceControls[0] = ReadFlag( block, "nofog" ) ? 1.0f : 0.0f;
	const float hdrScale = ReadParameter( block, "hdrcolorscale" );
	constants.surfaceControls[1] = claim.baseSrgb ? SourceGammaToLinear( hdrScale ) : hdrScale;
	if ( mode == 5 )
	{
		for ( int c = 0; c < 3; ++c )
		{
			const float tint = ReadParameter( block, "color", c );
			constants.tint[c] = claim.baseSrgb ? SourceGammaToLinear( tint ) : tint;
		}
		constants.tint[3] = ReadParameter( block, "alpha" );
	}
	claim.claimed = true;
	return claim;
}

UnlitClaim ClaimSpriteCard( const ParameterBlock &block )
{
	UnlitClaim claim;
	if ( block.Family().desc.name != "unlit" )
	{
		claim.reason = "the sprite card block is of family " + block.Family().desc.name;
		return claim;
	}
	// spritecard.cpp's parameters. $color and $alpha are not read by
	// spritecard_ps2x (its color is the texture, the overbright factor and the
	// vertex color), and its alpha test is its own (GREATER 0.01), so the
	// standard keys below are inert here, as is the $selfillum flag, which
	// spritecard.cpp never reads (a card's radiance is its texture already).
	// $sequence_blend_mode and
	// $zoomanimateseq2 only act on a second sequence, which is refused.
	constexpr std::string_view keys[] = { "basetexture", "frame", "color", "alpha", "model",
	    "nocull", "nofog", "vertexcolor", "vertexalpha", "translucent", "additive", "ignorez",
	    "alphatest", "alphatestreference", "depthblend", "depthblendscale", "orientation",
	    "splinetype", "overbrightfactor", "addself", "addoverblend", "mod2x", "blendframes",
	    "minsize", "maxsize", "startfadesize", "endfadesize", "maxdistance", "farfadeinterval",
	    "useinstancing", "sequence_blend_mode", "zoomanimateseq2", "dualsequence",
	    "maxlumframeblend1", "maxlumframeblend2", "extractgreenalpha", "addbasetexture2",
	    "ramptexture", "selfillum" };
	if ( const auto unread = detail::UnclaimedParameter( block, keys ) )
	{
		claim.reason = "the sprite card point does not draw " + *unread;
		return claim;
	}
	// The pixel shader's other combos, each a named gap until a claimed
	// material needs it.
	if ( ReadParameter( block, "dualsequence" ) != 0.0f )
	{
		claim.reason = "$dualsequence needs the sprite card point's second sequence";
		return claim;
	}
	if ( ReadParameter( block, "maxlumframeblend1" ) != 0.0f ||
	     ReadParameter( block, "maxlumframeblend2" ) != 0.0f )
	{
		claim.reason = "$maxlumframeblend needs the sprite card point's luminance frame select";
		return claim;
	}
	if ( ReadParameter( block, "extractgreenalpha" ) != 0.0f )
	{
		claim.reason = "$extractgreenalpha needs the sprite card point's green/alpha extract";
		return claim;
	}
	if ( ReadParameter( block, "addbasetexture2" ) != 0.0f )
	{
		claim.reason = "$addbasetexture2 needs the sprite card point's second sequence";
		return claim;
	}
	if ( detail::TextureBound( block, "ramptexture" ) )
	{
		claim.reason = "$ramptexture needs the sprite card point's color ramp";
		return claim;
	}
	const float overbright = ReadParameter( block, "overbrightfactor" );
	const float addSelf = ReadParameter( block, "addself" );
	const float depthScale = ReadParameter( block, "depthblendscale" );
	if ( !std::isfinite( overbright ) || !std::isfinite( addSelf ) )
	{
		claim.reason = "$overbrightfactor and $addself must be finite";
		return claim;
	}
	claim.depthBlend = ReadFlag( block, "depthblend" );
	if ( claim.depthBlend && ( !std::isfinite( depthScale ) || depthScale <= 0.0f ) )
	{
		claim.reason = "$depthblend needs a finite positive $depthblendscale";
		return claim;
	}
	const bool mod2x = ReadFlag( block, "mod2x" );
	const bool premultiplied = addSelf != 0.0f || ReadFlag( block, "addoverblend" );
	// spritecard.cpp's blend state, in its order.
	claim.blend = mod2x                           ? BlendMode::kModulate2x
	              : premultiplied                 ? BlendMode::kPremultiplied
	              : ReadFlag( block, "additive" ) ? BlendMode::kAlphaAdditive
	                                              : BlendMode::kAlpha;
	claim.alphaWrite = false;
	claim.ignoreDepth = ReadFlag( block, "ignorez" );
	// Added radiance fogs toward black, as the additive sprite modes do.
	claim.fogToBlack = claim.blend == BlendMode::kAlphaAdditive;
	SurfaceConstants &constants = claim.constants;
	constants.surfaceControls[0] = ReadFlag( block, "nofog" ) ? 1.0f : 0.0f;
	// The radiance scale: the texture's linear radiance times the overbright
	// factor (spritecard_ps2x applies it in the linear frame buffer's space).
	constants.surfaceControls[1] = overbright;
	constants.surfaceControls[2] = claim.depthBlend ? 1.0f : 0.0f;
	constants.surfaceControls[3] = depthScale;
	// Every card multiplies by its record's color and alpha.
	constants.flags[0] = 1.0f;
	constants.state[1] = 1.0f; // gamma vertex colors, decoded per vertex
	constants.state[3] = 1.0f;
	// SHADER_ALPHAFUNC_GREATER 0.01 on the stored byte, without $addself.
	constants.flags[1] = addSelf == 0.0f ? 1.0f : 0.0f;
	constants.flags[2] = 3.0f / 255.0f;
	// The card terms (surface_program.glsl): .y the second frame blended by
	// the vertex's lightmap offset, .z $addself's weight, .w $mod2x.
	constants.meshModes[1] = 1.0f;
	constants.meshModes[2] = addSelf;
	constants.meshModes[3] = mod2x ? 1.0f : 0.0f;
	claim.claimed = true;
	return claim;
}

sprite_card::Frame SpriteCardTerms( const ParameterBlock &block )
{
	sprite_card::Frame frame;
	frame.sizes.minSize = ReadParameter( block, "minsize" );
	frame.sizes.maxSize = ReadParameter( block, "maxsize" );
	frame.sizes.startFadeSize = ReadParameter( block, "startfadesize" );
	frame.sizes.endFadeSize = ReadParameter( block, "endfadesize" );
	sprite_card::SetFarFade( frame.sizes, ReadParameter( block, "maxdistance" ),
	    ReadParameter( block, "farfadeinterval" ) );
	const bool spline = ReadParameter( block, "splinetype" ) != 0.0f;
	frame.kind = spline ? sprite_card::Kind::kSpline : sprite_card::Kind::kSprite;
	frame.orientation = std::clamp( int( ReadParameter( block, "orientation" ) ), 0, 2 );
	// spritecard.cpp's ANIMBLEND combo is $blendframes; a spline card's
	// frame comes from its sheet range alone.
	frame.animBlend = !spline && ReadFlag( block, "blendframes" );
	return frame;
}

UnlitClaim ClaimDecalModulate( const ParameterBlock &block )
{
	UnlitClaim claim;
	if ( block.Family().desc.name != "decal-modulate" )
	{
		claim.reason = "the block is of family " + block.Family().desc.name;
		return claim;
	}
	// The shader ignores modulation and vertex colors. Projection owns decalscale.
	// $translucent is inert: DecalModulate_dx9.cpp always blends DST_COLOR,
	// SRC_COLOR whatever the material flags say.
	constexpr std::string_view keys[] = { "basetexture", "frame", "decal", "decalscale",
	    "vertexcolor", "vertexalpha", "model", "nocull", "nofog", "translucent" };
	if ( const auto unread = detail::UnclaimedParameter( block, keys ) )
	{
		claim.reason = "the decal point does not draw " + *unread;
		return claim;
	}
	if ( !detail::TextureBound( block, "basetexture" ) )
	{
		claim.reason = "DecalModulate needs its multiplicative texture";
		return claim;
	}
	claim.blend = device::BlendMode::kModulate2x;
	claim.alphaWrite = false;
	claim.decalModulate = true;
	claim.constants.surfaceControls[0] = detail::ReadFlag( block, "nofog" ) ? 1.0f : 0.0f;
	claim.claimed = true;
	return claim;
}

UnlitClaim ClaimModulate( const ParameterBlock &block )
{
	UnlitClaim claim;
	if ( block.Family().desc.name != "modulate" )
	{
		claim.reason = "the block is of family " + block.Family().desc.name;
		return claim;
	}
	// $translucent only selects destination-alpha writes
	// (EvaluateBlendRequirements) and $additive nothing: the blend is fixed.
	// The cloak pass's factor, tint and refraction are read only with
	// $cloakpassenabled.
	constexpr std::string_view keys[] = { "basetexture", "frame", "basetexturetransform", "color",
	    "alpha", "vertexcolor", "vertexalpha", "translucent", "additive", "model", "nocull", "nofog",
	    "writez", "mod2x", "cloakfactor", "cloakcolortint", "refractamount" };
	if ( const auto unread = detail::UnclaimedParameter( block, keys ) )
	{
		claim.reason = "the modulate point does not draw " + *unread;
		return claim;
	}
	if ( detail::ReadFlag( block, "cloakpassenabled" ) )
	{
		claim.reason = "the modulate point does not draw $cloakpassenabled";
		return claim;
	}
	if ( !detail::TextureBound( block, "basetexture" ) )
	{
		claim.reason = "Modulate without $basetexture samples an unbound sampler";
		return claim;
	}
	claim.blend = device::BlendMode::kModulate2x;
	claim.alphaWrite = false;
	claim.decalModulate = true;
	SurfaceConstants &constants = claim.constants;
	// ComputeModulationColor: $color and $alpha unconverted (the vertex
	// shader's modulation, in the texture's gamma space).
	for ( int c = 0; c < 3; ++c )
		constants.tint[c] = ReadParameter( block, "color", c );
	constants.tint[3] = ReadParameter( block, "alpha" );
	constants.flags[0] =
	    ReadFlag( block, "vertexcolor" ) || ReadFlag( block, "vertexalpha" ) ? 1.0f : 0.0f;
	for ( std::size_t i = 0; i < 8; ++i )
		constants.baseTransform[i] = ReadParameter( block, "basetexturetransform", i );
	// baseDecode.y: Modulate's factor; .z: 0.5, the half factor that draws
	// DST_COLOR, ZERO through the 2x blend (1 with $mod2x).
	constants.baseDecode[1] = 1.0f;
	constants.baseDecode[2] = ReadFlag( block, "mod2x" ) ? 1.0f : 0.5f;
	constants.surfaceControls[0] = ReadFlag( block, "nofog" ) ? 1.0f : 0.0f;
	claim.claimed = true;
	return claim;
}

UnlitClaim ClaimSky( const ParameterBlock &block, bool hdr )
{
	UnlitClaim claim;
	if ( block.Family().desc.name != "unlit" )
	{
		claim.reason = "the sky block is of family " + block.Family().desc.name;
		return claim;
	}
	constexpr std::string_view keys[] = { "basetexture", "hdrcompressedtexture",
	    "hdrcompressedtexture0", "hdrcompressedtexture1", "hdrcompressedtexture2", "hdrbasetexture",
	    "basetexturetransform", "color", "frame", "nofog", "ignorez", "model", "nocull" };
	if ( const auto unread = detail::UnclaimedParameter( block, keys ) )
	{
		claim.reason = "the sky point does not draw " + *unread;
		return claim;
	}
	SurfaceConstants &constants = claim.constants;
	// sky_hdr_dx9.cpp: c0 is $color as given; RGBS scales it by 8.
	float scale = 1.0f;
	if ( hdr && detail::TextureBound( block, "hdrcompressedtexture" ) )
	{
		claim.baseParameter = "hdrcompressedtexture";
		claim.baseSrgb = false;
#if defined( RENDER_MATERIAL_SKY_SEEDED_NO_RGBS_DECODE )
		constants.baseDecode[0] = 0.0f;
#else
		constants.baseDecode[0] = 1.0f;
#endif
		scale = 8.0f;
	}
	else if ( hdr && detail::TextureBound( block, "hdrcompressedtexture0" ) )
	{
		claim.reason = "$hdrcompressedtexture0 (three exposures) needs an unimplemented decode";
		return claim;
	}
	else if ( hdr )
	{
		claim.reason = "a lone $hdrbasetexture needs its format's conversion, not yet claimed";
		return claim;
	}
	else
	{
		claim.baseParameter = "basetexture";
		claim.baseSrgb = true;
	}
	if ( !detail::TextureBound( block, claim.baseParameter ) )
	{
		claim.reason = "the sky needs its $" + claim.baseParameter;
		return claim;
	}
	claim.blend = device::BlendMode::kOpaque;
	claim.alphaWrite = true;
	claim.ignoreDepth = true; // SHADER_INIT_PARAMS sets IGNOREZ
	constants.surfaceControls[0] = 1.0f; // and NOFOG
	constants.surfaceControls[1] = scale;
	for ( int c = 0; c < 3; ++c )
		constants.tint[c] = ReadParameter( block, "color", c );
	constants.tint[3] = 1.0f;
	for ( std::size_t i = 0; i < 8; ++i )
		constants.baseTransform[i] = ReadParameter( block, "basetexturetransform", i );
	claim.claimed = true;
	return claim;
}

UnlitClaim ClaimUnlitMesh( const ParameterBlock &block )
{
	// The ordinary unlit claim owns every shared parameter. The authored env
	// map is a model-only extension; the scene's RPRB supplies its radiance.
	ParameterBlock withoutProbe = block;
	const bool nativeProbe = detail::TextureBound( block, "envmap" );
	if ( nativeProbe )
		(void)withoutProbe.SetTexture( "envmap", {} );
	UnlitClaim claim = ClaimUnlit( withoutProbe );
	if ( claim.claimed && claim.depthBlend )
	{
		claim.claimed = false;
		claim.reason = "$depthblend needs the unlit particle point, not the emissive model point";
	}
	claim.nativeProbe = nativeProbe;
	return claim;
}

} // namespace render::material
