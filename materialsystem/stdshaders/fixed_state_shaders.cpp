//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shaders whose whole behaviour is fixed state, served from one table
// (RFC 0016 K9, R91: the first rows of the parameter-only provider that
// replaces stdshaders). Each row is a shader the material system knows by
// name: its parameters beyond the base set, its init-time flags and defaults,
// the textures it loads and the snapshot calls it makes, in order. A fallback
// row only names the shader it falls back to.
//
// The DX8-era fallbacks some of these shaders named ("Wireframe" below DX9)
// are not rows: every supported device is at DX9 level or above, so those
// branches never returned a shader.
//
//===========================================================================//

#include "BaseVSShader.h"
#include "cloak_blended_pass_helper.h"
#include "convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar gl_amd_occlusion_workaround( "gl_amd_occlusion_workaround", "1" );
static ConVar r_showz_power( "r_showz_power", "1.0f", FCVAR_CHEAT );
// Read by name by the native backend's and the render core's bloom downsample.
static ConVar r_bloomtintr( "r_bloomtintr", "0.3" );
static ConVar r_bloomtintg( "r_bloomtintg", "0.59" );
static ConVar r_bloomtintb( "r_bloomtintb", "0.11" );
static ConVar r_bloomtintexponent( "r_bloomtintexponent", "2.2" );
// Read by name by the native backend's and the render core's motion blur.
ConVar mat_motion_blur_percent_of_screen_max( "mat_motion_blur_percent_of_screen_max", "4.0" );

int GetDefaultDepthFeatheringValue(); // spritecard.cpp

namespace
{

// One snapshot call. Arguments are in a and b.
enum class Op
{
	End,
	ColorWrites,    // EnableColorWrites( a )
	AlphaWrites,    // EnableAlphaWrites( a )
	DepthWrites,    // EnableDepthWrites( a )
	Texture,        // EnableTexture( Sampler_t( a ), true )
	Blending,       // EnableBlending( true ); BlendFunc( a, b )
	Culling,        // EnableCulling( a )
	SrgbWrite,      // EnableSRGBWrite( a )
	SrgbRead,       // EnableSRGBRead( Sampler_t( a ), b )
	HdrSrgb,        // EnableSRGBWrite( true ) when HDR is on
	DepthFunc,      // DepthFunc( ShaderDepthFunc_t( a ) )
	AmdOcclusion,   // EnableSRGBWrite( true ) under gl_amd_occlusion_workaround
	OsxSrgb,        // EnableSRGBRead( SAMPLER0, x ); EnableSRGBWrite( x ), x = OSX sRGB RTs
	OsxSrgb4,       // OsxSrgb over samplers 0 to 3
	CompressedPos,  // VertexShaderVertexFormat( POSITION | COMPRESSED, 1, NULL, 0 )
	Pos1,           // VertexShaderVertexFormat( POSITION, 1, 0, 0 )
	Pos3EyeGlint,   // VertexShaderVertexFormat( POSITION, 3, { 2, 2, 3 }, 0 )
	FogToFogColor,  // FogToFogColor()
	FogToWhite,     // FogToWhite()
	FogToGrey,      // FogToGrey()
	DefaultFog,     // DefaultFog()
	Format,         // VertexShaderVertexFormat( a, b, 0, 0 )
	Initial,        // SetInitialShadowState()
	AdditiveBlend,  // EnableBlending( true ); BlendFunc( ONE, ONE ) if $additive, else ( a, b )
	SkySrgbRead,    // EnableSRGBRead( SAMPLER0, base texture is not 16-bit-per-channel )
	IntroSrgb,      // with $ENABLESRGB (extra a) or on OSX: sRGB read 0 and 1, sRGB write
	OsxSrgbWrite,   // EnableSRGBWrite( OSX sRGB RTs )
	DepthTest,      // EnableDepthTest( a )
	BlendEnable,    // EnableBlending( a )
	BlendFunc,      // BlendFunc( a, b )
	AlphaTest,      // EnableAlphaTest( a )
	AlphaFunc,      // AlphaFunc( ShaderAlphaFunc_t( a ), b )
	AlphaTestFlag,  // EnableAlphaTest( $alphatest )
	SrgbReadDx9,    // EnableSRGBRead( Sampler_t( a ), true ) at DX9 level
	AlphaWritesIfOpaque, // EnableAlphaWrites( base texture neither blends nor alpha tests )
	ShadowBias,     // EnablePolyOffset( SHADOW_BIAS ) unless extra a is set
	ColorWritesIf1, // EnableColorWrites( extra a == 1 )
	PolyOffset,     // EnablePolyOffset( PolygonOffsetMode_t( a ) )
	FogToGreyRaw,   // DisableFogGammaCorrection( true ); FogToGrey()
	DecalFormat,    // DecalModulate's format: POSITION | COMPRESSED, plus NORMAL and texcoords { 2, 0, 3 } with fast vertex textures
	FogToOOOverbright, // FogToOOOverbright()
	AlphaWritesFullyOpaque, // EnableAlphaWrites( opaque base texture, no alpha test, and extra a set (a < 0: always) )
	ModulateFormat,   // Modulate's format: POSITION | COMPRESSED, COLOR with vertex colour or alpha, a texcoord with a base texture or no colour
	TwoTextureBlend,  // UnlitTwoTexture's blend: translucent when alpha-modulating or a texture (base, extra a) is translucent
	TwoTextureFormat, // POSITION | NORMAL | COMPRESSED, COLOR with vertex colour, one texcoord
	LinearReadTexture, // when param b (extra, or -1 the base texture) is defined: sampler a, sRGB read unless 16-bit or extra c is set
	ClearWrites,    // BufferClearObeyStencil: depth, colour and alpha writes from extras 2, 0 and 1
	CullAlphaTested, // EnableCulling( $alphatest && !$nocull )
	DepthWriteAlpha, // DepthWrite's alpha-clip / vertex-texture samplers (extra a: $color_depth)
};

// When a step runs.
enum class When
{
	Always,
	Flag,    // MATERIAL_VAR_* c is set
	NotFlag, // MATERIAL_VAR_* c is clear
	Param,   // extra param c is nonzero
	NotParam, // extra param c is zero
	Texture,  // extra param c is a texture
};

struct Step
{
	Op op = Op::End;
	int a = 0;
	int b = 0;
	When when = When::Always;
	int c = 0;
};

// A parameter beyond the base set, in declaration order.
struct Param
{
	const char *name;
	ShaderParamType_t type;
	const char *defaultValue;
	const char *help;
};

// A texture to load at init: a base parameter, or (extra) the row's own.
enum class LoadAs
{
	Texture,
	CubeMap,
	OsxSrgbTexture, // TEXTUREFLAGS_SRGB where OSX render targets are sRGB
	SkyTexture,     // TEXTUREFLAGS_SRGB unless the texture is 16 bits per channel
	BumpMap,        // LoadBumpMap
	BumpMapWithDefault, // LoadBumpMap, first giving an undefined param its default name
	LinearRead,     // TEXTUREFLAGS_SRGB unless 16 bits per channel or extra textureFlags is set
};

// Row-specific init that is not a plain default.
enum class InitHook
{
	None,
	ParticleSphere, // $DEPTHBLEND from the depth-feathering default; $USINGPIXELSHADER at init
	VertexIdSkinning, // USES_VERTEXID and SUPPORTS_HW_SKINNING with fast vertex textures
	ModelFallback, // MATERIAL_VAR_MODEL defined at init; no base texture falls back to the DX6 model or lightmapped shader
};

bool IsSixteenBitPerChannel( ITexture *texture )
{
	const ImageFormat fmt = texture->GetImageFormat();
	return fmt == IMAGE_FORMAT_RGBA16161616F || fmt == IMAGE_FORMAT_RGBA16161616;
}

// A typed value set at param init on an extra param the material leaves undefined.
struct Default
{
	int extra = -1; // -1 ends the list
	ShaderParamType_t type = SHADER_PARAM_TYPE_FLOAT; // FLOAT, INTEGER, VEC2, COLOR or VEC4
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 0.0f;
};

// A base parameter the row redeclares (SHADER_PARAM_OVERRIDE): its default,
// help and flags. The type is recorded as written but, as with the macro,
// the base parameter keeps its own.
struct Override
{
	int base = -1; // -1: none
	ShaderParamType_t type = SHADER_PARAM_TYPE_TEXTURE;
	const char *defaultValue = nullptr;
	const char *help = nullptr;
	int flags = 0;
};

// An unused slot keeps index -1 (never 0: that is $FLAGS).
struct Load
{
	bool extra = false;
	int index = -1;
	bool ifDefined = false;
	int textureFlags = 0;
	LoadAs as = LoadAs::Texture;
};

constexpr int kMaxParams = 32;
constexpr int kMaxLoads = 4;
constexpr int kMaxSteps = 24;

struct FixedStateRow
{
	const char *name;
	const char *fallback; // non-null: the row only falls back to this shader
	int shaderFlags;
	int initFlags2;
	Param params[kMaxParams];      // name null ends the list
	int intDefault;                // -1: none; else the extra param set to intDefault...
	int intDefaultValue;           // ...when the material leaves it undefined
	Load loads[kMaxLoads];         // index -1 ends the list
	Step steps[kMaxSteps];         // Op::End ends the list
	int initFlags = 0;             // MATERIAL_VAR_* set at init
	Default defaults[10] = {};
	Override overrides[2] = {};
	// A SHADER_FALLBACK block's answer, as opposed to a DEFINE_FALLBACK_SHADER
	// alias (fallback above): the shader exists in its own right.
	const char *shaderFallback = nullptr;
	InitHook hook = InitHook::None;
	// Falls back to unlessFallback when extra unlessDefined is undefined.
	int unlessDefined = -1;
	const char *unlessFallback = nullptr;
	// The cloak's second pass (cloak_blended_pass_helper): the extra index of
	// $CLOAKPASSENABLED, followed by $CLOAKFACTOR, $CLOAKCOLORTINT and
	// $REFRACTAMOUNT; -1 for none.
	int cloak = -1;
	// EvaluateBlendRequirements( BASETEXTURE ) runs each draw (the opaque test
	// AlphaWritesFullyOpaque reads).
	bool evaluateBlend = false;
};

constexpr int kDistortMapFlags = TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD |
    TEXTUREFLAGS_NODEBUGOVERRIDE | TEXTUREFLAGS_SINGLECOPY | TEXTUREFLAGS_CLAMPS |
    TEXTUREFLAGS_CLAMPT;

constexpr int kCableFormat = VERTEX_POSITION | VERTEX_COLOR | VERTEX_TANGENT_S | VERTEX_TANGENT_T;

#define NO_PARAMS {}
#define NO_LOADS { { false, -1, false } }
#define T SHADER_PARAM_TYPE_TEXTURE
#define F SHADER_PARAM_TYPE_FLOAT
#define I SHADER_PARAM_TYPE_INTEGER
#define V4 SHADER_PARAM_TYPE_VEC4
#define S SHADER_PARAM_TYPE_STRING
#define M SHADER_PARAM_TYPE_MATERIAL

const FixedStateRow kRows[] = {
	{ "WriteZ", "WriteZ_DX9", 0, 0, NO_PARAMS, -1, 0, NO_LOADS, {} },
	{ "WriteZ_DX9", nullptr, SHADER_NOT_EDITABLE, 0, NO_PARAMS, -1, 0, NO_LOADS,
	    { { Op::ColorWrites, 0, 0 }, { Op::AlphaWrites, 0, 0 }, { Op::CompressedPos, 0, 0 } } },
	{ "WriteStencil", "WriteStencil_DX9", 0, 0, NO_PARAMS, -1, 0, NO_LOADS, {} },
	{ "WriteStencil_DX9", nullptr, SHADER_NOT_EDITABLE, 0, NO_PARAMS, -1, 0, NO_LOADS,
	    { { Op::ColorWrites, 0, 0 }, { Op::AlphaWrites, 0, 0 }, { Op::DepthWrites, 0, 0 },
	        { Op::CompressedPos, 0, 0 } } },
	{ "Occlusion", "Occlusion_DX9", 0, 0, NO_PARAMS, -1, 0, NO_LOADS, {} },
	{ "Occlusion_DX9", nullptr, SHADER_NOT_EDITABLE, 0, NO_PARAMS, -1, 0, NO_LOADS,
	    { { Op::ColorWrites, 0, 0 }, { Op::AlphaWrites, 0, 0 }, { Op::DepthWrites, 0, 0 },
	        { Op::AmdOcclusion, 0, 0 }, { Op::CompressedPos, 0, 0 } } },
	{ "Black", nullptr, 0, MATERIAL_VAR2_SUPPORTS_HW_SKINNING, NO_PARAMS, -1, 0, NO_LOADS,
	    { { Op::CompressedPos, 0, 0 }, { Op::SrgbWrite, 1, 0 }, { Op::FogToFogColor, 0, 0 } } },
	{ "HSV", nullptr, SHADER_NOT_EDITABLE, MATERIAL_VAR2_NEEDS_FULL_FRAME_BUFFER_TEXTURE,
	    NO_PARAMS, -1, 0, NO_LOADS, { { Op::Texture, SHADER_SAMPLER0, 0 }, { Op::Pos1, 0, 0 } } },
	{ "showz", nullptr, SHADER_NOT_EDITABLE, 0,
	    { { "$ALPHADEPTH", I, "0", "Depth is stored in alpha channel" } }, 0, 0, NO_LOADS,
	    { { Op::Texture, SHADER_SAMPLER0, 0 }, { Op::Pos1, 0, 0 }, { Op::SrgbWrite, 1, 0 } } },
	{ "FilmGrain", "FilmGrain_dx9", 0, 0, NO_PARAMS, -1, 0, NO_LOADS, {} },
	{ "FilmGrain_dx9", nullptr, SHADER_NOT_EDITABLE, MATERIAL_VAR2_NEEDS_FULL_FRAME_BUFFER_TEXTURE,
	    { { "$GRAIN_TEXTURE", T, "0", "Film grain texture" },
	        { "$NOISESCALE", V4, "", "Strength of film grain" } },
	    -1, 0, { { true, 0, false } },
	    { { Op::Blending, SHADER_BLEND_ONE, SHADER_BLEND_SRC_ALPHA },
	        { Op::Texture, SHADER_SAMPLER0, 0 }, { Op::Pos1, 0, 0 } } },
	{ "FilmDust", "FilmDust_dx9", 0, 0, NO_PARAMS, -1, 0, NO_LOADS, {} },
	{ "FilmDust_dx9", nullptr, SHADER_NOT_EDITABLE, MATERIAL_VAR2_NEEDS_FULL_FRAME_BUFFER_TEXTURE,
	    { { "$DUST_TEXTURE", T, "0", "Film dust texture" },
	        { "$CHANNEL_SELECT", V4, "", "Select which color channel to use" } },
	    -1, 0, { { true, 0, false } },
	    { { Op::Culling, 0, 0 }, { Op::Blending, SHADER_BLEND_ZERO, SHADER_BLEND_SRC_COLOR },
	        { Op::Texture, SHADER_SAMPLER0, 0 }, { Op::Pos1, 0, 0 } } },
	{ "Engine_Post", "Engine_Post_dx9", 0, 0, NO_PARAMS, -1, 0, NO_LOADS, {} },
	{ "Engine_Post_dx9", nullptr, SHADER_NOT_EDITABLE, MATERIAL_VAR2_NEEDS_FULL_FRAME_BUFFER_TEXTURE,
	    { { "$FBTEXTURE", T, "_rt_FullFrameFB", "Full framebuffer texture" },
	        { "$AAENABLE", SHADER_PARAM_TYPE_BOOL, "0", "Enable software anti-aliasing" },
	        { "$AAINTERNAL1", V4, "[0 0 0 0]", "Internal anti-aliasing values set via material proxy" },
	        { "$AAINTERNAL2", V4, "[0 0 0 0]", "Internal anti-aliasing values set via material proxy" },
	        { "$AAINTERNAL3", V4, "[0 0 0 0]", "Internal anti-aliasing values set via material proxy" },
	        { "$BLOOMENABLE", SHADER_PARAM_TYPE_BOOL, "1", "Enable bloom" } },
	    1, 0, { { false, BASETEXTURE, true }, { true, 0, true } },
	    // Bloom on sampler 0, the frame on 1 (sRGB only where OSX render
	    // targets are), colour-correction lookups on 2 to 5 read raw.
	    { { Op::BlendEnable, 0, 0 }, { Op::Texture, SHADER_SAMPLER0, 0 },
	        { Op::Texture, SHADER_SAMPLER1, 0 }, { Op::Texture, SHADER_SAMPLER2, 0 },
	        { Op::Texture, SHADER_SAMPLER3, 0 }, { Op::Texture, SHADER_SAMPLER4, 0 },
	        { Op::Texture, SHADER_SAMPLER5, 0 }, { Op::OsxSrgb4, 0, 0 },
	        { Op::SrgbRead, SHADER_SAMPLER2, 0 }, { Op::SrgbRead, SHADER_SAMPLER3, 0 },
	        { Op::SrgbRead, SHADER_SAMPLER4, 0 }, { Op::SrgbRead, SHADER_SAMPLER5, 0 },
	        { Op::Pos1, 0, 0 } },
	    0, { { 2, V4 }, { 3, V4 }, { 4, V4 }, { 5, I, 1 } } },
	{ "floatcombine", nullptr, SHADER_NOT_EDITABLE, 0,
	    { { "$BLOOMTEXTURE", T, "", "" }, { "$SHARPNESS", F, "1", "" }, { "$WOODCUT", F, "0", "" },
	        { "$VIGNETTE_MIN_BRIGHT", F, "1", "" }, { "$VIGNETTE_POWER", F, "4", "" },
	        { "$EDGE_SOFTNESS", F, "0", "" }, { "$BLOOMAMOUNT", F, "1.0", "" },
	        { "$BLOOMEXPONENT", F, "2.0", "" }, { "$ALPHASHARPENFACTOR", F, "0.0", "" } },
	    -1, 0, { { false, BASETEXTURE, true }, { true, 0, true } },
	    { { Op::DepthWrites, 0, 0 }, { Op::Texture, SHADER_SAMPLER0, 0 },
	        { Op::Texture, SHADER_SAMPLER1, 0 }, { Op::Pos1, 0, 0 }, { Op::SrgbWrite, 1, 0 } } },
	{ "Sample4x4", nullptr, SHADER_NOT_EDITABLE, 0,
	    { { "$BASETEXTURE", T, "", "" },
	        { "$PIXSHADER", S, "sample4x4_ps20", "Name of the pixel shader to use" } },
	    // Its own $BASETEXTURE shadows the base one; the source's BASETEXTURE meant it.
	    -1, 0, { { true, 0, false } },
	    { { Op::DepthWrites, 0, 0 }, { Op::AlphaWrites, 1, 0 }, { Op::Texture, SHADER_SAMPLER0, 0 },
	        { Op::Pos1, 0, 0 } } },
	{ "HDRCombineTo16Bit", nullptr, SHADER_NOT_EDITABLE, 0,
	    { { "$SOURCEMRTRENDERTARGET", T, "", "" } }, -1, 0, { { true, 0, false } },
	    { { Op::DepthWrites, 0, 0 }, { Op::AlphaWrites, 0, 0 }, { Op::DepthTest, 0, 0 },
	        { Op::Texture, SHADER_SAMPLER0, 0 }, { Op::Texture, SHADER_SAMPLER1, 0 },
	        { Op::Pos1, 0, 0 } } },
	{ "HDRSelectRange", nullptr, SHADER_NOT_EDITABLE, 0,
	    { { "$SOURCEMRTRENDERTARGET", T, "", "" } }, -1, 0, { { true, 0, false } },
	    { { Op::DepthWrites, 0, 0 }, { Op::AlphaWrites, 0, 0 }, { Op::DepthTest, 0, 0 },
	        { Op::Texture, SHADER_SAMPLER0, 0 }, { Op::Texture, SHADER_SAMPLER1, 0 },
	        { Op::Pos1, 0, 0 } } },
	{ "Downsample", nullptr, SHADER_NOT_EDITABLE, 0, NO_PARAMS, -1, 0,
	    { { false, BASETEXTURE, false } },
	    { { Op::DepthWrites, 0, 0 }, { Op::AlphaWrites, 1, 0 }, { Op::Texture, SHADER_SAMPLER0, 0 },
	        { Op::OsxSrgb, 0, 0 }, { Op::Pos1, 0, 0 } } },
	{ "Bloom", nullptr, SHADER_NOT_EDITABLE, 0,
	    { { "$FBTEXTURE", T, "_rt_FullFrameFB", "" }, { "$BLURTEXTURE", T, "_rt_SmallHDR0", "" } },
	    -1, 0, { { true, 0, true }, { true, 1, true } },
	    { { Op::DepthWrites, 0, 0 }, { Op::Texture, SHADER_SAMPLER0, 0 },
	        { Op::Texture, SHADER_SAMPLER1, 0 }, { Op::Pos1, 0, 0 } } },
	{ "BlurFilterX", nullptr, SHADER_NOT_EDITABLE, 0, NO_PARAMS, -1, 0,
	    { { false, BASETEXTURE, true } },
	    { { Op::DepthWrites, 0, 0 }, { Op::AlphaWrites, 1, 0 }, { Op::Texture, SHADER_SAMPLER0, 0 },
	        { Op::Pos1, 0, 0 }, { Op::OsxSrgb, 0, 0 },
	        { Op::Blending, SHADER_BLEND_ONE, SHADER_BLEND_ONE, When::Flag, MATERIAL_VAR_ADDITIVE } } },
	{ "BlurFilterY", nullptr, SHADER_NOT_EDITABLE, 0,
	    { { "$BLOOMAMOUNT", F, "1.0", "" }, { "$FRAMETEXTURE", T, "_rt_SmallHDR0", "" } }, -1, 0,
	    { { false, BASETEXTURE, true } },
	    { { Op::DepthWrites, 0, 0 }, { Op::AlphaWrites, 1, 0 }, { Op::Texture, SHADER_SAMPLER0, 0 },
	        { Op::Pos1, 0, 0 }, { Op::OsxSrgb, 0, 0 },
	        { Op::Blending, SHADER_BLEND_ONE, SHADER_BLEND_ONE, When::Flag, MATERIAL_VAR_ADDITIVE } },
	    0, { { 0, F, 1.0f } } },
	{ "accumbuff4sample", nullptr, SHADER_NOT_EDITABLE, 0,
	    { { "$TEXTURE0", T, "", "" }, { "$TEXTURE1", T, "", "" }, { "$TEXTURE2", T, "", "" },
	        { "$TEXTURE3", T, "", "" }, { "$WEIGHTS", V4, "", "Weight for Samples" } },
	    -1, 0, { { true, 0, false }, { true, 1, false }, { true, 2, false }, { true, 3, false } },
	    { { Op::DepthWrites, 0, 0 }, { Op::DepthTest, 0, 0 }, { Op::AlphaWrites, 0, 0 },
	        { Op::BlendEnable, 0, 0 }, { Op::Culling, 0, 0 }, { Op::Texture, SHADER_SAMPLER0, 0 },
	        { Op::Texture, SHADER_SAMPLER1, 0 }, { Op::Texture, SHADER_SAMPLER2, 0 },
	        { Op::Texture, SHADER_SAMPLER3, 0 }, { Op::Pos1, 0, 0 }, { Op::OsxSrgb4, 0, 0 } } },
	{ "DebugTextureView", "DebugTextureView_dx9", 0, 0, NO_PARAMS, -1, 0, NO_LOADS, {} },
	{ "DebugTextureView_dx9", nullptr, 0, 0, { { "$SHOWALPHA", SHADER_PARAM_TYPE_BOOL, "0", "" } },
	    -1, 0, { { false, BASETEXTURE, true } },
	    { { Op::DepthWrites, 0, 0 }, { Op::AlphaTest, 1, 0 }, { Op::Texture, SHADER_SAMPLER0, 0 },
	        { Op::CompressedPos, 0, 0 } } },
	{ "BufferClearObeyStencil", "BufferClearObeyStencil_DX9", 0, 0, NO_PARAMS, -1, 0, NO_LOADS, {} },
	{ "BufferClearObeyStencil_DX9", nullptr, SHADER_NOT_EDITABLE, 0,
	    { { "$CLEARCOLOR", I, "1", "activates clearing of color" },
	        { "$CLEARALPHA", I, "-1", "activates clearing of alpha. -1 == copy CLEARCOLOR setting" },
	        { "$CLEARDEPTH", I, "1", "activates clearing of depth" } },
	    // The source's SHADER_INIT set an undefined $CLEARALPHA to -1, but the
	    // corpus shows those materials at 0 after init; the row keeps that.
	    -1, 0, NO_LOADS,
	    { { Op::DepthFunc, SHADER_DEPTHFUNC_ALWAYS, 0 }, { Op::ClearWrites, 0, 0 },
	        { Op::Format, VERTEX_POSITION | VERTEX_COLOR, 1 },
	        { Op::Blending, SHADER_BLEND_ONE, SHADER_BLEND_ZERO }, { Op::AlphaTest, 1, 0 },
	        { Op::AlphaFunc, SHADER_ALPHAFUNC_ALWAYS, 0 } } },
	{ "DecalModulate", "DecalModulate_DX9", 0, 0, NO_PARAMS, -1, 0, NO_LOADS, {} },
	{ "DecalModulate_dx9", nullptr, 0, 0, NO_PARAMS, -1, 0, { { false, BASETEXTURE, false } },
	    { { Op::AlphaTest, 1, 0 }, { Op::AlphaFunc, SHADER_ALPHAFUNC_GREATER, 0 },
	        { Op::DepthWrites, 0, 0 }, { Op::PolyOffset, SHADER_POLYOFFSET_DECAL, 0 },
	        { Op::Texture, SHADER_SAMPLER0, 0 }, { Op::AlphaWrites, 0, 0 },
	        { Op::SrgbRead, SHADER_SAMPLER0, 0 }, { Op::SrgbWrite, 0, 0 },
	        { Op::Blending, SHADER_BLEND_DST_COLOR, SHADER_BLEND_SRC_COLOR },
	        { Op::FogToGreyRaw, 0, 0 }, { Op::DecalFormat, 0, 0 } },
	    MATERIAL_VAR_NO_DEBUG_OVERRIDE, {}, {}, nullptr, InitHook::VertexIdSkinning },
	{ "screenspace_general", "screenspace_general_dx9", 0, 0, NO_PARAMS, -1, 0, NO_LOADS, {} },
	{ "screenspace_general_dx9", nullptr, SHADER_NOT_EDITABLE, 0,
	    { { "$C0_X", F, "0", "" }, { "$C0_Y", F, "0", "" }, { "$C0_Z", F, "0", "" }, { "$C0_W", F, "0", "" }, { "$C1_X", F, "0", "" }, { "$C1_Y", F, "0", "" }, { "$C1_Z", F, "0", "" }, { "$C1_W", F, "0", "" }, { "$C2_X", F, "0", "" }, { "$C2_Y", F, "0", "" }, { "$C2_Z", F, "0", "" }, { "$C2_W", F, "0", "" }, { "$C3_X", F, "0", "" }, { "$C3_Y", F, "0", "" }, { "$C3_Z", F, "0", "" }, { "$C3_W", F, "0", "" }, 
	        { "$PIXSHADER", S, "", "Name of the pixel shader to use" },
	        { "$DISABLE_COLOR_WRITES", I, "0", "" }, { "$ALPHATESTED", F, "0", "" },
	        { "$ALPHA_BLEND_COLOR_OVERLAY", I, "0", "" }, { "$ALPHA_BLEND", I, "0", "" },
	        { "$TEXTURE1", T, "", "" }, { "$TEXTURE2", T, "", "" }, { "$TEXTURE3", T, "", "" },
	        { "$LINEARREAD_BASETEXTURE", I, "0", "" }, { "$LINEARREAD_TEXTURE1", I, "0", "" },
	        { "$LINEARREAD_TEXTURE2", I, "0", "" }, { "$LINEARREAD_TEXTURE3", I, "0", "" },
	        { "$LINEARWRITE", I, "0", "" },
	        { "$X360APPCHOOSER", I, "0", "Needed for movies in 360 launcher" },
	        { "$COPYALPHA", I, "0", "" } },
	    -1, 0,
	    // Each texture reads linear when it is 16 bits per channel or its
	    // $LINEARREAD_* is set (the POSIX branch; every client is POSIX).
	    { { false, BASETEXTURE, true, 24, LoadAs::LinearRead },
	        { true, 21, true, 25, LoadAs::LinearRead }, { true, 22, true, 26, LoadAs::LinearRead },
	        { true, 23, true, 27, LoadAs::LinearRead } },
	    { { Op::DepthWrites, 0, 0 }, { Op::LinearReadTexture, SHADER_SAMPLER0, -1, When::Always, 24 },
	        { Op::LinearReadTexture, SHADER_SAMPLER1, 21, When::Always, 25 },
	        { Op::LinearReadTexture, SHADER_SAMPLER2, 22, When::Always, 26 },
	        { Op::LinearReadTexture, SHADER_SAMPLER3, 23, When::Always, 27 },
	        { Op::Format, VERTEX_POSITION | VERTEX_COLOR, 1, When::Param, 29 },
	        { Op::Blending, SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA, When::Param, 29 },
	        { Op::Format, VERTEX_POSITION, 1, When::NotParam, 29 },
	        { Op::SrgbWrite, 1, 0, When::NotParam, 28 }, { Op::SrgbWrite, 0, 0, When::Param, 28 },
	        { Op::ColorWrites, 0, 0, When::Param, 17 }, { Op::AlphaTest, 1, 0 },
	        { Op::AlphaFunc, SHADER_ALPHAFUNC_GREATER, 0 },
	        { Op::Blending, SHADER_BLEND_ONE, SHADER_BLEND_ONE, When::Flag, MATERIAL_VAR_ADDITIVE },
	        { Op::Blending, SHADER_BLEND_ONE, SHADER_BLEND_ONE_MINUS_SRC_ALPHA, When::Param, 19 },
	        { Op::Blending, SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA, When::Param, 20 },
	        { Op::BlendEnable, 0, 0, When::Param, 30 },
	        { Op::AlphaFunc, SHADER_ALPHAFUNC_ALWAYS, 0, When::Param, 30 } } },
	{ "Modulate", "Modulate_DX9", 0, 0, NO_PARAMS, -1, 0, NO_LOADS, {} },
	{ "Modulate_DX9", nullptr, 0, MATERIAL_VAR2_SUPPORTS_HW_SKINNING,
	    { { "$WRITEZ", SHADER_PARAM_TYPE_BOOL, "0", "Forces z to be written if set" },
	        { "$MOD2X", SHADER_PARAM_TYPE_BOOL, "0", "forces a 2x modulate so that you can brighten and darken things" },
	        { "$CLOAKPASSENABLED", SHADER_PARAM_TYPE_BOOL, "0", "Enables cloak render in a second pass" },
	        { "$CLOAKFACTOR", F, "0.0", "" }, { "$CLOAKCOLORTINT", SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Cloak color tint" },
	        { "$REFRACTAMOUNT", F, "2", "" } },
	    -1, 0, { { false, BASETEXTURE, true } },
	    { { Op::Blending, SHADER_BLEND_DST_COLOR, SHADER_BLEND_SRC_COLOR, When::Param, 1 },
	        { Op::Blending, SHADER_BLEND_DST_COLOR, SHADER_BLEND_ZERO, When::NotParam, 1 },
	        { Op::DepthWrites, 1, 0, When::Param, 0 }, { Op::ModulateFormat },
	        { Op::FogToGrey, 0, 0, When::Param, 1 }, { Op::FogToOOOverbright, 0, 0, When::NotParam, 1 },
	        { Op::AlphaWritesFullyOpaque, 0 } },
	    0, {}, {}, nullptr, InitHook::None, -1, nullptr, 2, true },
	{ "MonitorScreen", "MonitorScreen_DX9", 0, 0, NO_PARAMS, -1, 0, NO_LOADS, {} },
	{ "MonitorScreen_DX9", nullptr, 0, MATERIAL_VAR2_SUPPORTS_HW_SKINNING,
	    { { "$CONTRAST", F, "0.0", "contrast 0 == normal 1 == color*color" },
	        { "$SATURATION", F, "1.0", "saturation 0 == greyscale 1 == normal" },
	        { "$TINT", SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "monitor tint" },
	        { "$TEXTURE2", T, "shadertest/lightmappedtexture", "second texture" },
	        { "$FRAME2", I, "0", "frame number for $texture2" },
	        { "$TEXTURE2TRANSFORM", SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0",
	            "$texture2 texcoord transform" } },
	    -1, 0, { { false, BASETEXTURE, true, TEXTUREFLAGS_SRGB }, { true, 3, true, TEXTUREFLAGS_SRGB } },
	    { { Op::Texture, SHADER_SAMPLER0 }, { Op::SrgbRead, SHADER_SAMPLER0, 1 },
	        { Op::Texture, SHADER_SAMPLER1, 0, When::Texture, 3 },
	        { Op::SrgbRead, SHADER_SAMPLER1, 1, When::Texture, 3 }, { Op::SrgbWrite, 1 },
	        { Op::TwoTextureBlend, 3 },
	        { Op::Format, VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED, 1 },
	        { Op::DefaultFog }, { Op::AlphaWritesFullyOpaque, -1 } },
	    0, { { 0, F, 0.0f }, { 1, F, 1.0f }, { 2, SHADER_PARAM_TYPE_COLOR, 1.0f, 1.0f, 1.0f } }, {}, nullptr,
	    InitHook::ModelFallback, -1, nullptr, -1, true },
	{ "UnlitTwoTexture", "UnlitTwoTexture_DX9", 0, 0, NO_PARAMS, -1, 0, NO_LOADS, {} },
	{ "UnlitTwoTexture_DX9", nullptr, 0, MATERIAL_VAR2_SUPPORTS_HW_SKINNING,
	    { { "$TEXTURE2", T, "shadertest/BaseTexture", "second texture" },
	        { "$FRAME2", I, "0", "frame number for $texture2" },
	        { "$TEXTURE2TRANSFORM", SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0",
	            "$texture2 texcoord transform" },
	        { "$CLOAKPASSENABLED", SHADER_PARAM_TYPE_BOOL, "0", "Enables cloak render in a second pass" },
	        { "$CLOAKFACTOR", F, "0.0", "" }, { "$CLOAKCOLORTINT", SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Cloak color tint" },
	        { "$REFRACTAMOUNT", F, "2", "" } },
	    -1, 0, { { false, BASETEXTURE, true, TEXTUREFLAGS_SRGB }, { true, 0, true, TEXTUREFLAGS_SRGB } },
	    { { Op::Texture, SHADER_SAMPLER0 }, { Op::SrgbRead, SHADER_SAMPLER0, 1 },
	        { Op::Texture, SHADER_SAMPLER1 }, { Op::SrgbRead, SHADER_SAMPLER1, 1 }, { Op::SrgbWrite, 1 },
	        { Op::TwoTextureBlend, 0 }, { Op::TwoTextureFormat }, { Op::DefaultFog },
	        { Op::AlphaWritesFullyOpaque, -1 } },
	    0, {}, {}, nullptr, InitHook::None, -1, nullptr, 3, true },
	{ "Downsample_nohdr", nullptr, SHADER_NOT_EDITABLE, 0,
	    { { "$BLOOMTINTENABLE", I, "1", "" }, { "$CSTRIKE", I, "0", "" } }, 0, 1,
	    { { false, BASETEXTURE, false } },
	    { { Op::DepthWrites, 0, 0 }, { Op::AlphaWrites, 1, 0 }, { Op::Texture, SHADER_SAMPLER0, 0 },
	        { Op::OsxSrgb, 0, 0 }, { Op::Pos1, 0, 0 } } },
	{ "floattoscreen", nullptr, SHADER_NOT_EDITABLE, 0,
	    { { "$FBTEXTURE", T, "", "" },
	        { "$PIXSHADER", S, "floattoscreen_ps20", "Name of the pixel shader to use" } },
	    -1, 0, { { true, 0, true } },
	    { { Op::DepthWrites, 0, 0 }, { Op::Texture, SHADER_SAMPLER0, 0 }, { Op::Pos1, 0, 0 },
	        { Op::SrgbWrite, 1, 0 } } },
	{ "Sample4x4_Blend", nullptr, 0, 0,
	    { { "$BASETEXTURE", T, "", "" },
	        { "$PIXSHADER", S, "sample4x4_ps20", "Name of the pixel shader to use" } },
	    -1, 0, { { true, 0, false } }, // its own $BASETEXTURE, as Sample4x4's
	    { { Op::DepthWrites, 0, 0 }, { Op::AlphaWrites, 1, 0 }, { Op::Texture, SHADER_SAMPLER0, 0 },
	        { Op::Pos1, 0, 0 },
	        { Op::Blending, SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA } } },
	{ "floatcombine_autoexpose", nullptr, 0, 0,
	    { { "$BLOOMTEXTURE", T, "", "" }, { "$SHARPNESS", F, "1", "" }, { "$WOODCUT", F, "0", "" },
	        { "$VIGNETTE_MIN_BRIGHT", F, "1", "" }, { "$VIGNETTE_POWER", F, "4", "" },
	        { "$EDGE_SOFTNESS", F, "0", "" }, { "$BLOOMAMOUNT", F, "1.0", "" },
	        { "$BLOOMEXPONENT", F, "2.0", "" }, { "$ALPHASHARPENFACTOR", F, "0.0", "" },
	        { "$EXPOSURE_TEXTURE", T, "", "" }, { "$AUTOEXPOSE_MIN", F, ".5", "" },
	        { "$AUTOEXPOSE_MAX", F, "2", "" } },
	    -1, 0, { { false, BASETEXTURE, true }, { true, 0, true }, { true, 9, true } },
	    { { Op::DepthWrites, 0, 0 }, { Op::Texture, SHADER_SAMPLER0, 0 },
	        { Op::Texture, SHADER_SAMPLER1, 0 }, { Op::Texture, SHADER_SAMPLER2, 0 },
	        { Op::Pos1, 0, 0 }, { Op::SrgbWrite, 1, 0 } } },
	{ "MotionBlur", "MotionBlur_dx9", 0, 0, NO_PARAMS, -1, 0, NO_LOADS, {} },
	{ "MotionBlur_dx9", nullptr, SHADER_NOT_EDITABLE, 0,
	    { { "$MOTIONBLURINTERNAL", V4, "[0 0 0 0]", "Internal motion blur value set by proxy" } },
	    -1, 0, { { false, BASETEXTURE, true, 0, LoadAs::OsxSrgbTexture } },
	    { { Op::Pos1, 0, 0 }, { Op::Texture, SHADER_SAMPLER0, 0 }, { Op::OsxSrgb, 0, 0 },
	        { Op::DepthWrites, 0, 0 }, { Op::AlphaWrites, 0, 0 } } },
	{ "WindowImposter", "WindowImposter_DX90", 0, 0, NO_PARAMS, -1, 0, NO_LOADS, {} },
	{ "WindowImposter_DX90", nullptr, 0, 0,
	    { { "$ENVMAP", T, "shadertest/shadertest_env", "envmap" } },
	    -1, 0, { { true, 0, false, 0, LoadAs::CubeMap } },
	    { { Op::HdrSrgb, 0, 0 }, { Op::Texture, SHADER_SAMPLER0, 0 }, { Op::Pos1, 0, 0 },
	        { Op::Blending, SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA },
	        { Op::DepthWrites, 0, 0 }, { Op::FogToFogColor, 0, 0 } } },
	{ "ShadowBuild", "ShadowBuild_DX9", 0, 0, NO_PARAMS, -1, 0, NO_LOADS, {} },
	{ "ShadowBuild_DX9", nullptr, SHADER_NOT_EDITABLE, MATERIAL_VAR2_SUPPORTS_HW_SKINNING,
	    { { "$TRANSLUCENT_MATERIAL", M, "", "Points to a material to grab translucency from" } },
	    -1, 0, { { false, BASETEXTURE, true, TEXTUREFLAGS_SRGB } },
	    { { Op::Blending, SHADER_BLEND_ONE, SHADER_BLEND_ONE }, { Op::DepthWrites, 0, 0 },
	        { Op::Texture, SHADER_SAMPLER0, 0 }, { Op::SrgbRead, SHADER_SAMPLER0, 1 },
	        { Op::SrgbWrite, 1, 0 }, { Op::AlphaWrites, 1, 0 }, { Op::DepthWrites, 0, 0 },
	        { Op::DepthFunc, SHADER_DEPTHFUNC_ALWAYS, 0 }, { Op::CompressedPos, 0, 0 } },
	    MATERIAL_VAR_NO_DEBUG_OVERRIDE },
	{ "Bik", nullptr, 0, 0,
	    { { "$YTEXTURE", T, "shadertest/BaseTexture", "Y Bink Texture" },
	        { "$CRTEXTURE", T, "shadertest/BaseTexture", "Cr Bink Texture" },
	        { "$CBTEXTURE", T, "shadertest/BaseTexture", "Cb Bink Texture" } },
	    -1, 0, { { true, 0, true }, { true, 1, true }, { true, 2, true } },
	    { { Op::Texture, SHADER_SAMPLER0, 0 }, { Op::Texture, SHADER_SAMPLER1, 0 },
	        { Op::Texture, SHADER_SAMPLER2, 0 }, { Op::Pos1, 0, 0 }, { Op::SrgbWrite, 0, 0 } } },
	{ "ShadowModel", "ShadowModel_DX9", 0, 0, NO_PARAMS, -1, 0, NO_LOADS, {} },
	{ "ShadowModel_DX9", nullptr, SHADER_NOT_EDITABLE, 0,
	    { { "$BASETEXTUREOFFSET", SHADER_PARAM_TYPE_VEC2, "[0 0]", "$baseTexture texcoord offset" },
	        { "$BASETEXTURESCALE", SHADER_PARAM_TYPE_VEC2, "[1 1]", "$baseTexture texcoord scale" },
	        { "$FALLOFFOFFSET", F, "0", "Distance at which shadow starts to fade" },
	        { "$FALLOFFDISTANCE", F, "100", "Max shadow distance" },
	        { "$FALLOFFAMOUNT", F, "0.9", "Amount to brighten the shadow at max dist" } },
	    -1, 0, { { false, BASETEXTURE, true } },
	    { { Op::Texture, SHADER_SAMPLER0, 0 }, { Op::Blending, SHADER_BLEND_DST_COLOR, SHADER_BLEND_ZERO },
	        { Op::DepthWrites, 0, 0 }, { Op::Format, VERTEX_POSITION | VERTEX_NORMAL, 1 },
	        { Op::FogToWhite, 0, 0 } },
	    0,
	    { { 1, SHADER_PARAM_TYPE_VEC2, 1.0f, 1.0f }, { 3, F, 100.0f }, { 4, F, 0.9f } } },
	// Cloud's source also set $CLOUDSCALE and $MASKSCALE to [1 1] at instance
	// init, but the type defaults ([0 0]) had defined them by then.
	{ "Cloud", "Cloud_dx9", 0, 0, NO_PARAMS, -1, 0, NO_LOADS, {} },
	{ "Cloud_dx9", nullptr, 0, 0,
	    { { "$CLOUDALPHATEXTURE", T, "shadertest/cloudalpha", "cloud alpha texture" },
	        { "$CLOUDSCALE", SHADER_PARAM_TYPE_VEC2, "[1 1]", "cloudscale" },
	        { "$MASKSCALE", SHADER_PARAM_TYPE_VEC2, "[1 1]", "maskscale" } },
	    -1, 0, { { false, BASETEXTURE, false, TEXTUREFLAGS_SRGB }, { true, 0, false, TEXTUREFLAGS_SRGB } },
	    { { Op::DepthWrites, 0, 0 },
	        { Op::AdditiveBlend, SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA },
	        { Op::Texture, SHADER_SAMPLER0, 0 }, { Op::Texture, SHADER_SAMPLER1, 0 },
	        { Op::Format, VERTEX_POSITION, 2 }, { Op::DefaultFog, 0, 0 } },
	    0, {}, { { BASETEXTURE, T, "shadertest/cloud", "cloud texture", 0 } } },
	// A dead shader: it only ever falls back to Wireframe.
	{ "Eyeball", nullptr, 0, 0, NO_PARAMS, -1, 0, NO_LOADS, {}, 0, {},
	    { { BASETEXTURE, T, "models/alyx/pupil_l", "iris texture", 0 },
	        { BASETEXTURETRANSFORM, SHADER_PARAM_TYPE_MATRIX,
	            "center .5 .5 scale 1 1 rotate 0 translate 0 0", "unused",
	            SHADER_PARAM_NOT_EDITABLE } },
	    "Wireframe" },
	{ "Sky_DX9", nullptr, 0, 0, NO_PARAMS, -1, 0,
	    { { false, BASETEXTURE, true, 0, LoadAs::SkyTexture } },
	    { { Op::Initial, 0, 0 }, { Op::Texture, SHADER_SAMPLER0, 0 }, { Op::SkySrgbRead, 0, 0 },
	        { Op::Pos1, 0, 0 }, { Op::SrgbWrite, 1, 0 }, { Op::AlphaWrites, 1, 0 } },
	    MATERIAL_VAR_NOFOG | MATERIAL_VAR_IGNOREZ, {},
	    { { COLOR, SHADER_PARAM_TYPE_VEC3, "[ 1 1 1]", "color multiplier", SHADER_PARAM_NOT_EDITABLE },
	        { ALPHA, F, "1.0", "unused", SHADER_PARAM_NOT_EDITABLE } } },
	{ "IntroScreenSpaceEffect", nullptr, SHADER_NOT_EDITABLE,
	    MATERIAL_VAR2_NEEDS_FULL_FRAME_BUFFER_TEXTURE,
	    { { "$MODE", I, "0", "" }, { "$ENABLESRGB", SHADER_PARAM_TYPE_BOOL, "0", "" } }, 1, 0,
	    NO_LOADS,
	    { { Op::Texture, SHADER_SAMPLER0, 0 }, { Op::Texture, SHADER_SAMPLER1, 0 },
	        { Op::IntroSrgb, 1, 0 }, { Op::Pos1, 0, 0 },
	        { Op::Blending, SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE } } },
	{ "Shadow", nullptr, SHADER_NOT_EDITABLE, 0, NO_PARAMS, -1, 0,
	    { { false, BASETEXTURE, false, TEXTUREFLAGS_SRGB } },
	    { { Op::Texture, SHADER_SAMPLER0, 0 }, { Op::SrgbRead, SHADER_SAMPLER0, 1 },
	        { Op::Blending, SHADER_BLEND_ZERO, SHADER_BLEND_SRC_COLOR }, { Op::DepthWrites, 0, 0 },
	        { Op::Format, VERTEX_POSITION | VERTEX_COLOR, 1 }, { Op::SrgbWrite, 1, 0 },
	        { Op::FogToWhite, 0, 0 } } },
	{ "ColorCorrection", nullptr, SHADER_NOT_EDITABLE, MATERIAL_VAR2_NEEDS_FULL_FRAME_BUFFER_TEXTURE,
	    { { "$WEIGHT_DEFAULT", F, "1", "Volume Texture Default Weight" },
	        { "$WEIGHT0", F, "0", "Volume Texture Weight 0" },
	        { "$WEIGHT1", F, "0", "Volume Texture Weight 1" },
	        { "$WEIGHT2", F, "0", "Volume Texture Weight 2" },
	        { "$WEIGHT3", F, "0", "Volume Texture Weight 3" },
	        { "$NUM_LOOKUPS", I, "0", "Number of lookup maps" },
	        { "$USE_FB_TEXTURE", SHADER_PARAM_TYPE_BOOL, "0", "Use frame buffer texture as input" },
	        { "$INPUT_TEXTURE", T, "0", "Input texture" } },
	    5, 0, NO_LOADS,
	    { { Op::Texture, SHADER_SAMPLER0, 0 }, { Op::Texture, SHADER_SAMPLER1, 0 },
	        { Op::Texture, SHADER_SAMPLER2, 0 }, { Op::Texture, SHADER_SAMPLER3, 0 },
	        { Op::Texture, SHADER_SAMPLER4, 0 }, { Op::Pos1, 0, 0 }, { Op::OsxSrgbWrite, 0, 0 } },
	    0,
	    { { 0, F, 1.0f }, { 1, F, 1.0f }, { 2, F, 1.0f }, { 3, F, 1.0f }, { 4, F, 1.0f } } },
	{ "warp", nullptr, 0, 0, { { "$BASETEXTURE", T, "", "" } }, -1, 0,
	    { { true, 0, false, TEXTUREFLAGS_SRGB } },
	    { { Op::Initial }, { Op::DepthWrites, 0 }, { Op::DepthTest, 0 }, { Op::BlendEnable, 0 },
	        { Op::Texture, SHADER_SAMPLER0 }, { Op::SrgbRead, SHADER_SAMPLER0, 1 },
	        { Op::SrgbWrite, 1 }, { Op::AlphaWrites, 0 }, { Op::AlphaTest, 0 }, { Op::DefaultFog },
	        { Op::Format, VERTEX_POSITION, 2 } } },
	{ "vr_distort_texture", nullptr, 0, 0,
	    { { "$BASETEXTURE", T, "", "" }, { "$DISTORTMAP", T, "vr_distort_map", "" },
	        { "$USERENDERTARGET", I, "0", "" } },
	    -1, 0, { { true, 0, false, TEXTUREFLAGS_SRGB }, { true, 1, false, kDistortMapFlags } },
	    { { Op::Initial }, { Op::DepthWrites, 0 }, { Op::DepthTest, 0 }, { Op::BlendEnable, 0 },
	        { Op::Texture, SHADER_SAMPLER0 }, { Op::SrgbRead, SHADER_SAMPLER0, 1 },
	        { Op::Texture, SHADER_SAMPLER1 }, { Op::SrgbWrite, 1 }, { Op::AlphaWrites, 0 },
	        { Op::AlphaTest, 0 }, { Op::DefaultFog }, { Op::Format, VERTEX_POSITION, 2 } } },
	{ "vr_distort_hud", nullptr, 0, 0,
	    { { "$BASETEXTURE", T, "_rt_gui", "" }, { "$DISTORTMAP", T, "vr_distort_map_left", "" },
	        { "$DISTORTBOUNDS", V4, "[ 0 0 1 1 ]", "" }, { "$HUDTRANSLUCENT", I, "0", "" },
	        { "$HUDUNDISTORT", I, "0", "" } },
	    -1, 0, { { true, 0, false, TEXTUREFLAGS_SRGB }, { true, 1, false, kDistortMapFlags } },
	    { { Op::Initial }, { Op::DepthWrites, 0 }, { Op::DepthTest, 0 },
	        { Op::Texture, SHADER_SAMPLER0 }, { Op::SrgbRead, SHADER_SAMPLER0, 1 },
	        { Op::Texture, SHADER_SAMPLER1 }, { Op::SrgbWrite, 1 }, { Op::AlphaWrites, 0 },
	        { Op::AlphaFunc, SHADER_ALPHAFUNC_GREATER, 0 },
	        { Op::AlphaTest, 1, 0, When::Flag, MATERIAL_VAR_TRANSLUCENT },
	        { Op::Blending, SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA, When::Flag,
	            MATERIAL_VAR_TRANSLUCENT },
	        { Op::AlphaTest, 0, 0, When::NotFlag, MATERIAL_VAR_TRANSLUCENT },
	        { Op::BlendEnable, 0, 0, When::NotFlag, MATERIAL_VAR_TRANSLUCENT },
	        { Op::BlendFunc, SHADER_BLEND_ONE, SHADER_BLEND_ZERO, When::NotFlag,
	            MATERIAL_VAR_TRANSLUCENT },
	        { Op::DefaultFog }, { Op::Format, VERTEX_POSITION, 2 } } },
	{ "Aftershock", "Aftershock_dx9", 0, 0, NO_PARAMS, -1, 0, NO_LOADS, {} },
	{ "Aftershock_dx9", nullptr, 0, MATERIAL_VAR2_SUPPORTS_HW_SKINNING |
	        MATERIAL_VAR2_NEEDS_TANGENT_SPACES | MATERIAL_VAR2_NEEDS_POWER_OF_TWO_FRAME_BUFFER_TEXTURE,
	    { { "$COLORTINT", SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Color tint" },
	        { "$REFRACTAMOUNT", F, "2", "" },
	        { "$NORMALMAP", T, "models/shadertest/shader1_normal", "normal map" },
	        { "$BUMPFRAME", I, "0", "frame number for $bumpmap" },
	        { "$BUMPTRANSFORM", SHADER_PARAM_TYPE_MATRIX,
	            "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$bumpmap texcoord transform" },
	        { "$SILHOUETTETHICKNESS", F, "1", "" },
	        { "$SILHOUETTECOLOR", SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Silhouette color tint" },
	        { "$GROUNDMIN", F, "1", "" }, { "$GROUNDMAX", F, "1", "" },
	        { "$BLURAMOUNT", F, "1", "" }, { "$TIME", F, "0.0", "Needs CurrentTime Proxy" } },
	    -1, 0, { { true, 2, true } },
	    { { Op::Format, VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED, 1 },
	        { Op::Texture, SHADER_SAMPLER0 }, { Op::SrgbRead, SHADER_SAMPLER0, 1 },
	        { Op::Texture, SHADER_SAMPLER1 }, { Op::SrgbRead, SHADER_SAMPLER1, 0 },
	        { Op::SrgbWrite, 1 },
	        { Op::Blending, SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA },
	        { Op::DepthWrites, 0 }, { Op::AlphaWrites, 0 } },
	    MATERIAL_VAR_TRANSLUCENT,
	    { { 1, F, 0.1f }, { 0, SHADER_PARAM_TYPE_VEC4, 1.0f, 1.0f, 1.0f, 1.0f }, { 3, I, 0.0f },
	        { 5, F, 0.2f }, { 6, SHADER_PARAM_TYPE_VEC4, 0.3f, 0.3f, 0.5f, 1.0f }, { 7, F, -0.3f },
	        { 8, F, -0.1f }, { 9, F, 0.01f }, { 10, F, 0.0f } } },
	{ "Cable", "Cable_DX9", 0, 0, NO_PARAMS, -1, 0, NO_LOADS, {} },
	{ "SplineRope", "Cable_DX9", 0, 0, NO_PARAMS, -1, 0, NO_LOADS, {} },
	{ "Cable_DX9", nullptr, 0, 0,
	    { { "$BUMPMAP", T, "cable/cablenormalmap", "bumpmap texture" },
	        { "$MINLIGHT", F, "0.1", "Minimum amount of light (0-1 value)" },
	        { "$MAXLIGHT", F, "0.3", "Maximum amount of light" } },
	    -1, 0,
	    { { true, 0, false, 0, LoadAs::BumpMapWithDefault },
	        { false, BASETEXTURE, false, TEXTUREFLAGS_SRGB } },
	    { { Op::DepthWrites, 0, 0, When::Flag, MATERIAL_VAR_TRANSLUCENT },
	        { Op::Blending, SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA, When::Flag,
	            MATERIAL_VAR_TRANSLUCENT },
	        { Op::AlphaTestFlag }, { Op::Texture, SHADER_SAMPLER0 }, { Op::Texture, SHADER_SAMPLER1 },
	        { Op::SrgbReadDx9, SHADER_SAMPLER1 }, { Op::Format, kCableFormat, -2 },
	        { Op::SrgbWrite, 1 }, { Op::FogToFogColor }, { Op::AlphaWritesIfOpaque } } },
	{ "ParticleSphere", "ParticleSphere_DX9", 0, 0, NO_PARAMS, -1, 0, NO_LOADS, {} },
	{ "ParticleSphere_DX9", nullptr, SHADER_NOT_EDITABLE, 0,
	    { { "$DEPTHBLEND", I, "0", "fade at intersection boundaries" },
	        { "$DEPTHBLENDSCALE", F, "50.0",
	            "Amplify or reduce DEPTHBLEND fading. Lower values make harder edges." },
	        { "$USINGPIXELSHADER", SHADER_PARAM_TYPE_BOOL, "0",
	            "Tells to client code whether the shader is using DX8 vertex/pixel shaders or not" },
	        { "$BUMPMAP", T, "models/shadertest/shader1_normal", "bumpmap" },
	        { "$LIGHTS", SHADER_PARAM_TYPE_FOURCC, "", "array of lights" },
	        { "$LIGHT_POSITION", SHADER_PARAM_TYPE_VEC3, "0 0 0",
	            "This is the directional light position." },
	        { "$LIGHT_COLOR", SHADER_PARAM_TYPE_VEC3, "1 1 1", "This is the directional light color." } },
	    -1, 0, { { true, 3, false, 0, LoadAs::BumpMap } },
	    { { Op::Texture, SHADER_SAMPLER0 }, { Op::Texture, SHADER_SAMPLER1, 0, When::Param, 0 },
	        { Op::Format, VERTEX_POSITION | VERTEX_COLOR, -1 },
	        { Op::Blending, SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA },
	        { Op::DepthWrites, 0 }, { Op::FogToFogColor } },
	    0, { { 1, F, 50.0f } }, {}, nullptr, InitHook::ParticleSphere, 3, "UnlitGeneric_DX6" },
	{ "DepthWrite", nullptr, SHADER_NOT_EDITABLE, MATERIAL_VAR2_SUPPORTS_HW_SKINNING,
	    { { "$ALPHATESTREFERENCE", F, "", "Alpha reference value" },
	        { "$COLOR_DEPTH", SHADER_PARAM_TYPE_BOOL, "0", "Write depth as color" } },
	    -1, 0, NO_LOADS,
	    { { Op::CompressedPos }, { Op::ShadowBias, 1 }, { Op::ColorWritesIf1, 1 },
	        { Op::AlphaWrites, 0 }, { Op::CullAlphaTested }, { Op::DepthWriteAlpha, 1 } } },
	{ "EyeGlint", "EyeGlint_dx9", 0, 0, NO_PARAMS, -1, 0, NO_LOADS, {} },
	{ "EyeGlint_dx9", nullptr, 0, 0, NO_PARAMS, -1, 0, NO_LOADS,
	    { { Op::DepthWrites, 0, 0 }, { Op::Blending, SHADER_BLEND_ONE, SHADER_BLEND_ONE },
	        { Op::Pos3EyeGlint, 0, 0 }, { Op::Culling, 0, 0 }, { Op::SrgbWrite, 0, 0 } } },
};

#undef NO_PARAMS
#undef NO_LOADS
#undef T
#undef F
#undef I
#undef V4
#undef S
#undef M

class CFixedStateShader : public CBaseVSShader
{
public:
	explicit CFixedStateShader( const FixedStateRow &row ) : m_Row( row )
	{
		while ( m_nParams < kMaxParams && m_Row.params[m_nParams].name )
			++m_nParams;
	}

	char const *GetName() const override { return m_Row.name; }
	int GetFlags() const override { return m_Row.shaderFlags; }
	char const *GetFallbackShader( IMaterialVar **params ) const override
	{
		if ( m_Row.fallback )
			return m_Row.fallback;
		if ( m_Row.unlessFallback && params && !params[Extra( m_Row.unlessDefined )]->IsDefined() )
			return m_Row.unlessFallback;
		if ( m_Row.hook == InitHook::ModelFallback && params && !params[BASETEXTURE]->IsDefined() )
			return IS_FLAG_DEFINED( MATERIAL_VAR_MODEL ) ? "VertexLitGeneric_DX6" : "LightmappedGeneric_DX6";
		return m_Row.shaderFallback;
	}
	int GetNumParams() const override { return CBaseVSShader::GetNumParams() + m_nParams; }
	bool NeedsPowerOfTwoFrameBufferTexture( IMaterialVar **params, bool bCheckSpecificToThisFrame ) const override
	{
		if ( m_Row.cloak >= 0 && params[Extra( m_Row.cloak )]->GetIntValue() )
		{
			if ( !bCheckSpecificToThisFrame )
				return true;
			const float factor = params[Extra( m_Row.cloak + 1 )]->GetFloatValue();
			if ( factor > 0.0f && factor < 1.0f )
				return true;
		}
		return CBaseVSShader::NeedsPowerOfTwoFrameBufferTexture( params, bCheckSpecificToThisFrame );
	}
	bool IsTranslucent( IMaterialVar **params ) const override
	{
		if ( m_Row.cloak >= 0 && params[Extra( m_Row.cloak )]->GetIntValue() )
		{
			const float factor = params[Extra( m_Row.cloak + 1 )]->GetFloatValue();
			if ( factor > 0.0f && factor < 1.0f )
				return true;
		}
		return CBaseVSShader::IsTranslucent( params );
	}
	char const *GetParamName( int param ) const override
	{
		const Param *p = RowParam( param );
		return p ? p->name : CBaseVSShader::GetParamName( param );
	}
	char const *GetParamHelp( int param ) const override
	{
		if ( const Override *o = BaseOverride( param ) )
			return o->help;
		const Param *p = RowParam( param );
		return p ? p->help : CBaseVSShader::GetParamHelp( param );
	}
	// An override keeps the base parameter's type, as SHADER_PARAM_OVERRIDE's does.
	ShaderParamType_t GetParamType( int param ) const override
	{
		const Param *p = RowParam( param );
		return p ? p->type : CBaseVSShader::GetParamType( param );
	}
	char const *GetParamDefault( int param ) const override
	{
		if ( const Override *o = BaseOverride( param ) )
			return o->defaultValue;
		const Param *p = RowParam( param );
		return p ? p->defaultValue : CBaseVSShader::GetParamDefault( param );
	}
	int GetParamFlags( int param ) const override
	{
		if ( const Override *o = BaseOverride( param ) )
			return o->flags;
		return RowParam( param ) ? 0 : CBaseVSShader::GetParamFlags( param );
	}

protected:
	void OnInitShaderParams( IMaterialVar **params, const char *pMaterialName ) override
	{
		if ( m_Row.initFlags )
			SET_FLAGS( MaterialVarFlags_t( m_Row.initFlags ) );
		if ( m_Row.initFlags2 )
			SET_FLAGS2( MaterialVarFlags2_t( m_Row.initFlags2 ) );
		if ( m_Row.cloak >= 0 )
		{
			IMaterialVar *enabled = params[Extra( m_Row.cloak )];
			if ( !enabled->IsDefined() )
				enabled->SetIntValue( 0 );
			else if ( enabled->GetIntValue() )
			{
				CloakBlendedPassVars_t cloak;
				SetupCloak( cloak );
				InitParamsCloakBlendedPass( this, params, pMaterialName, cloak );
			}
		}
		if ( m_Row.hook == InitHook::VertexIdSkinning && g_pHardwareConfig->HasFastVertexTextures() )
		{
			SET_FLAGS2( MATERIAL_VAR2_USES_VERTEXID );
			SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING );
		}
		if ( m_Row.hook == InitHook::ModelFallback && !IS_FLAG_DEFINED( MATERIAL_VAR_MODEL ) )
			CLEAR_FLAGS( MATERIAL_VAR_MODEL );
		if ( m_Row.hook == InitHook::ParticleSphere )
		{
			IMaterialVar *depthBlend = params[Extra( 0 )];
			if ( !depthBlend->IsDefined() )
				depthBlend->SetIntValue( GetDefaultDepthFeatheringValue() );
			if ( !g_pHardwareConfig->SupportsPixelShaders_2_b() )
				depthBlend->SetIntValue( 0 );
		}
		if ( m_Row.intDefault >= 0 )
		{
			IMaterialVar *var = params[Extra( m_Row.intDefault )];
			if ( !var->IsDefined() )
				var->SetIntValue( m_Row.intDefaultValue );
		}
		for ( const Default &value : m_Row.defaults )
		{
			if ( value.extra < 0 )
				break;
			IMaterialVar *var = params[Extra( value.extra )];
			if ( var->IsDefined() )
				continue;
			if ( value.type == SHADER_PARAM_TYPE_VEC2 )
				var->SetVecValue( value.x, value.y );
			else if ( value.type == SHADER_PARAM_TYPE_COLOR )
				var->SetVecValue( value.x, value.y, value.z );
			else if ( value.type == SHADER_PARAM_TYPE_VEC4 )
				var->SetVecValue( value.x, value.y, value.z, value.w );
			else if ( value.type == SHADER_PARAM_TYPE_INTEGER )
				var->SetIntValue( int( value.x ) );
			else
				var->SetFloatValue( value.x );
		}
	}
	void OnInitShaderInstance(
	    IMaterialVar **params, IShaderInit *pShaderInit, const char *pMaterialName ) override
	{
		if ( m_Row.hook == InitHook::ParticleSphere )
			params[Extra( 2 )]->SetIntValue( true );
		for ( const Load &load : m_Row.loads )
		{
			if ( load.index < 0 )
				break;
			const int param = load.extra ? Extra( load.index ) : load.index;
			if ( load.ifDefined && !params[param]->IsDefined() )
				continue;
			switch ( load.as )
			{
			case LoadAs::Texture:
				LoadTexture( param, load.textureFlags );
				break;
			case LoadAs::CubeMap:
				LoadCubeMap( param, load.textureFlags );
				break;
			case LoadAs::BumpMapWithDefault:
				if ( !params[param]->IsDefined() )
					params[param]->SetStringValue( GetParamDefault( param ) );
				LoadBumpMap( param );
				break;
			case LoadAs::BumpMap:
				LoadBumpMap( param );
				break;
			case LoadAs::SkyTexture:
				LoadTexture( param, IsSixteenBitPerChannel( params[param]->GetTextureValue() )
				                        ? 0 : TEXTUREFLAGS_SRGB );
				break;
			case LoadAs::LinearRead:
			{
				// textureFlags names the extra param that asks for a linear read.
				const IMaterialVar *linear = params[Extra( load.textureFlags )];
				const bool srgb = !IsSixteenBitPerChannel( params[param]->GetTextureValue() ) &&
				                  !( linear->IsDefined() && linear->GetIntValue() );
				LoadTexture( param, srgb ? TEXTUREFLAGS_SRGB : 0 );
				break;
			}
			case LoadAs::OsxSrgbTexture:
				LoadTexture( param, IsOSX() && g_pHardwareConfig->CanDoSRGBReadFromRTs()
				                        ? TEXTUREFLAGS_SRGB : 0 );
				break;
			}
		}
		// After the row's texture loads, as the shaders ordered it.
		if ( m_Row.cloak >= 0 && params[Extra( m_Row.cloak )]->GetIntValue() )
		{
			CloakBlendedPassVars_t cloak;
			SetupCloak( cloak );
			InitCloakBlendedPass( this, params, cloak );
		}
	}
	void OnDrawElements( IMaterialVar **params, IShaderShadow *pShaderShadow,
	    IShaderDynamicAPI *pShaderAPI, VertexCompressionType_t vertexCompression,
	    CBasePerMaterialContextData **pContextDataPtr ) override
	{
		if ( m_Row.fallback || m_Row.shaderFallback )
			return;
		const bool cloaking = m_Row.cloak >= 0 && params[Extra( m_Row.cloak )]->GetIntValue();
		CloakBlendedPassVars_t cloak;
		if ( cloaking )
			SetupCloak( cloak );
		// A fully opaque cloak replaces the standard pass (not while snapshotting).
		if ( cloaking && !pShaderShadow && CloakBlendedPassIsFullyOpaque( params, cloak ) )
			Draw( false );
		else
		{
			DrawStandard( params, pShaderShadow );
		}
		if ( cloaking )
		{
			const float factor = params[Extra( m_Row.cloak + 1 )]->GetFloatValue();
			if ( pShaderShadow || ( factor > 0.0f && factor < 1.0f ) )
				DrawCloakBlendedPass( this, params, pShaderAPI, pShaderShadow, cloak, vertexCompression );
			else
				Draw( false );
		}
	}

private:
	void SetupCloak( CloakBlendedPassVars_t &cloak ) const
	{
		cloak.m_nCloakFactor = Extra( m_Row.cloak + 1 );
		cloak.m_nCloakColorTint = Extra( m_Row.cloak + 2 );
		cloak.m_nRefractAmount = Extra( m_Row.cloak + 3 );
	}
	void DrawStandard( IMaterialVar **params, IShaderShadow *pShaderShadow )
	{
		if ( m_Row.evaluateBlend )
		{
			const BlendType_t blend = EvaluateBlendRequirements( BASETEXTURE, true );
			m_FullyOpaque = blend != BT_BLENDADD && blend != BT_BLEND && !IS_FLAG_SET( MATERIAL_VAR_ALPHATEST );
		}
		SHADOW_STATE
		{
			for ( const Step &step : m_Row.steps )
			{
				if ( step.op == Op::End )
					break;
				if ( step.when == When::Flag && !IS_FLAG_SET( MaterialVarFlags_t( step.c ) ) )
					continue;
				if ( step.when == When::NotFlag && IS_FLAG_SET( MaterialVarFlags_t( step.c ) ) )
					continue;
				if ( step.when == When::Param && !params[Extra( step.c )]->GetIntValue() )
					continue;
				if ( step.when == When::NotParam && params[Extra( step.c )]->GetIntValue() )
					continue;
				if ( step.when == When::Texture && !params[Extra( step.c )]->IsTexture() )
					continue;
				Apply( step, params, pShaderShadow );
			}
		}
		Draw();
	}

	int Extra( int index ) const { return CBaseVSShader::GetNumParams() + index; }
	const Param *RowParam( int param ) const
	{
		const int index = param - CBaseVSShader::GetNumParams();
		return index >= 0 && index < m_nParams ? &m_Row.params[index] : nullptr;
	}

	const Override *BaseOverride( int param ) const
	{
		for ( const Override &o : m_Row.overrides )
		{
			if ( o.base >= 0 && o.base == param )
				return &o;
		}
		return nullptr;
	}

	void Apply( const Step &step, IMaterialVar **params, IShaderShadow *pShaderShadow )
	{
		switch ( step.op )
		{
		case Op::End:
			break;
		case Op::ColorWrites:
			pShaderShadow->EnableColorWrites( step.a != 0 );
			break;
		case Op::AlphaWrites:
			pShaderShadow->EnableAlphaWrites( step.a != 0 );
			break;
		case Op::DepthWrites:
			pShaderShadow->EnableDepthWrites( step.a != 0 );
			break;
		case Op::Texture:
			pShaderShadow->EnableTexture( Sampler_t( step.a ), true );
			break;
		case Op::Blending:
			pShaderShadow->EnableBlending( true );
			pShaderShadow->BlendFunc( ShaderBlendFactor_t( step.a ), ShaderBlendFactor_t( step.b ) );
			break;
		case Op::Culling:
			pShaderShadow->EnableCulling( step.a != 0 );
			break;
		case Op::SrgbWrite:
			pShaderShadow->EnableSRGBWrite( step.a != 0 );
			break;
		case Op::DepthTest:
			pShaderShadow->EnableDepthTest( step.a != 0 );
			break;
		case Op::BlendEnable:
			pShaderShadow->EnableBlending( step.a != 0 );
			break;
		case Op::BlendFunc:
			pShaderShadow->BlendFunc( ShaderBlendFactor_t( step.a ), ShaderBlendFactor_t( step.b ) );
			break;
		case Op::AlphaTest:
			pShaderShadow->EnableAlphaTest( step.a != 0 );
			break;
		case Op::AlphaFunc:
			pShaderShadow->AlphaFunc( ShaderAlphaFunc_t( step.a ), float( step.b ) );
			break;
		case Op::OsxSrgbWrite:
			pShaderShadow->EnableSRGBWrite( IsOSX() && g_pHardwareConfig->CanDoSRGBReadFromRTs() );
			break;
		case Op::SrgbRead:
			pShaderShadow->EnableSRGBRead( Sampler_t( step.a ), step.b != 0 );
			break;
		case Op::HdrSrgb:
			if ( g_pHardwareConfig->GetHDRType() != HDR_TYPE_NONE )
				pShaderShadow->EnableSRGBWrite( true );
			break;
		case Op::DepthFunc:
			pShaderShadow->DepthFunc( ShaderDepthFunc_t( step.a ) );
			break;
		case Op::AmdOcclusion:
			if ( g_pHardwareConfig->PlatformRequiresNonNullPixelShaders() &&
			     ( IsLinux() || IsWindows() || IsBSD() ) && gl_amd_occlusion_workaround.GetBool() )
				pShaderShadow->EnableSRGBWrite( true );
			break;
		case Op::OsxSrgb:
		{
			// Render targets are pegged as sRGB on OSX, so force these reads and writes.
			const bool force = IsOSX() && g_pHardwareConfig->CanDoSRGBReadFromRTs();
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, force );
			pShaderShadow->EnableSRGBWrite( force );
			break;
		}
		case Op::OsxSrgb4:
		{
			const bool force = IsOSX() && g_pHardwareConfig->CanDoSRGBReadFromRTs();
			for ( int sampler = 0; sampler < 4; ++sampler )
				pShaderShadow->EnableSRGBRead( Sampler_t( SHADER_SAMPLER0 + sampler ), force );
			pShaderShadow->EnableSRGBWrite( force );
			break;
		}
		case Op::CompressedPos:
			pShaderShadow->VertexShaderVertexFormat(
			    VERTEX_POSITION | VERTEX_FORMAT_COMPRESSED, 1, NULL, 0 );
			break;
		case Op::Pos1:
			pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION, 1, 0, 0 );
			break;
		case Op::Pos3EyeGlint:
		{
			int texCoords[3] = { 2, 2, 3 };
			pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION, 3, texCoords, 0 );
			break;
		}
		case Op::FogToFogColor:
			FogToFogColor();
			break;
		case Op::FogToWhite:
			FogToWhite();
			break;
		case Op::DefaultFog:
			DefaultFog();
			break;
		case Op::Format:
			if ( step.b == -1 ) // one 2D texcoord, given explicitly
			{
				int dimensions[] = { 2 };
				pShaderShadow->VertexShaderVertexFormat( step.a, 1, dimensions, 0 );
			}
			else if ( step.b == -2 ) // two 2D texcoords, given explicitly
			{
				int dimensions[] = { 2, 2 };
				pShaderShadow->VertexShaderVertexFormat( step.a, 2, dimensions, 0 );
			}
			else
				pShaderShadow->VertexShaderVertexFormat( step.a, step.b, 0, 0 );
			break;
		case Op::AlphaTestFlag:
			pShaderShadow->EnableAlphaTest( IS_FLAG_SET( MATERIAL_VAR_ALPHATEST ) );
			break;
		case Op::SrgbReadDx9:
			if ( g_pHardwareConfig->GetDXSupportLevel() >= 90 )
				pShaderShadow->EnableSRGBRead( Sampler_t( step.a ), true );
			break;
		case Op::AlphaWritesIfOpaque:
		{
			// Dest alpha is free for special use when the base texture neither blends nor tests.
			const BlendType_t blend = EvaluateBlendRequirements( BASETEXTURE, true );
			pShaderShadow->EnableAlphaWrites( blend != BT_BLENDADD && blend != BT_BLEND &&
			                                  !IS_FLAG_SET( MATERIAL_VAR_ALPHATEST ) );
			break;
		}
		case Op::ShadowBias:
			if ( params[Extra( step.a )]->GetIntValue() == 0 )
				pShaderShadow->EnablePolyOffset( SHADER_POLYOFFSET_SHADOW_BIAS );
			break;
		case Op::ColorWritesIf1:
			pShaderShadow->EnableColorWrites( params[Extra( step.a )]->GetIntValue() == 1 );
			break;
		case Op::PolyOffset:
			pShaderShadow->EnablePolyOffset( PolygonOffsetMode_t( step.a ) );
			break;
		case Op::FogToGreyRaw:
			pShaderShadow->DisableFogGammaCorrection( true );
			FogToGrey();
			break;
		case Op::DecalFormat:
		{
			const bool fast = g_pHardwareConfig->HasFastVertexTextures();
			int texCoordDims[3] = { 2, 0, 3 };
			pShaderShadow->VertexShaderVertexFormat(
			    VERTEX_POSITION | VERTEX_FORMAT_COMPRESSED | ( fast ? VERTEX_NORMAL : 0 ), fast ? 3 : 1,
			    texCoordDims, 0 );
			break;
		}
		case Op::FogToGrey:
			FogToGrey();
			break;
		case Op::FogToOOOverbright:
			FogToOOOverbright();
			break;
		case Op::AlphaWritesFullyOpaque:
			pShaderShadow->EnableAlphaWrites(
			    m_FullyOpaque && ( step.a < 0 || params[Extra( step.a )]->GetIntValue() != 0 ) );
			break;
		case Op::ModulateFormat:
		{
			unsigned int flags = VERTEX_POSITION;
			int texCoords = 0;
			if ( params[BASETEXTURE]->IsTexture() )
			{
				pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
				texCoords = 1;
			}
			if ( IS_FLAG_SET( MATERIAL_VAR_VERTEXCOLOR ) || IS_FLAG_SET( MATERIAL_VAR_VERTEXALPHA ) )
				flags |= VERTEX_COLOR;
			if ( !( flags & VERTEX_COLOR ) && texCoords == 0 )
				texCoords = 1;
			pShaderShadow->VertexShaderVertexFormat( flags | VERTEX_FORMAT_COMPRESSED, texCoords, NULL, 0 );
			break;
		}
		case Op::TwoTextureBlend:
		{
			const bool translucent = IsAlphaModulating() || TextureIsTranslucent( BASETEXTURE, true ) ||
			                         TextureIsTranslucent( Extra( step.a ), true );
			const bool additive = IS_FLAG_SET( MATERIAL_VAR_ADDITIVE );
			if ( translucent )
				EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA,
				    additive ? SHADER_BLEND_ONE : SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
			else if ( additive )
				EnableAlphaBlending( SHADER_BLEND_ONE, SHADER_BLEND_ONE );
			else
				DisableAlphaBlending();
			break;
		}
		case Op::TwoTextureFormat:
			pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED |
			        ( IS_FLAG_SET( MATERIAL_VAR_VERTEXCOLOR ) ? VERTEX_COLOR : 0 ),
			    1, NULL, 0 );
			break;
		case Op::LinearReadTexture:
		{
			IMaterialVar *texture = params[step.b < 0 ? int( BASETEXTURE ) : Extra( step.b )];
			if ( !texture->IsDefined() )
				break;
			pShaderShadow->EnableTexture( Sampler_t( step.a ), true );
			const IMaterialVar *linear = params[Extra( step.c )];
			pShaderShadow->EnableSRGBRead( Sampler_t( step.a ),
			    !IsSixteenBitPerChannel( texture->GetTextureValue() ) &&
			        !( linear->IsDefined() && linear->GetIntValue() ) );
			break;
		}
		case Op::ClearWrites:
		{
			// $CLEARALPHA -1 copies $CLEARCOLOR.
			const bool color = params[Extra( 0 )]->GetIntValue() != 0;
			const int alpha = params[Extra( 1 )]->GetIntValue();
			pShaderShadow->EnableDepthWrites( params[Extra( 2 )]->GetIntValue() != 0 );
			pShaderShadow->EnableColorWrites( color );
			pShaderShadow->EnableAlphaWrites( alpha >= 0 ? alpha != 0 : color );
			break;
		}
		case Op::CullAlphaTested:
			pShaderShadow->EnableCulling(
			    IS_FLAG_SET( MATERIAL_VAR_ALPHATEST ) && !IS_FLAG_SET( MATERIAL_VAR_NOCULL ) );
			break;
		case Op::DepthWriteAlpha:
			if ( !g_pHardwareConfig->HasFastVertexTextures() )
			{
				if ( IS_FLAG_SET( MATERIAL_VAR_ALPHATEST ) )
				{
					pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
					pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );
				}
			}
			else
			{
				SET_FLAGS2( MATERIAL_VAR2_USES_VERTEXID );
				pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
				pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );
			}
			break;
		case Op::Initial:
			SetInitialShadowState();
			break;
		case Op::AdditiveBlend:
			pShaderShadow->EnableBlending( true );
			if ( IS_FLAG_SET( MATERIAL_VAR_ADDITIVE ) )
				pShaderShadow->BlendFunc( SHADER_BLEND_ONE, SHADER_BLEND_ONE );
			else
				pShaderShadow->BlendFunc(
				    ShaderBlendFactor_t( step.a ), ShaderBlendFactor_t( step.b ) );
			break;
		case Op::SkySrgbRead:
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0,
			    !IsSixteenBitPerChannel( params[BASETEXTURE]->GetTextureValue() ) );
			break;
		case Op::IntroSrgb:
			if ( params[Extra( step.a )]->GetIntValue() || IsOSX() )
			{
				pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );
				pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, true );
				pShaderShadow->EnableSRGBWrite( true );
			}
			break;
		}
	}

	const FixedStateRow &m_Row;
	int m_nParams = 0;
	bool m_FullyOpaque = true; // DrawStandard's, for AlphaWritesFullyOpaque
};

// One instance per row; each registers itself with the shader DLL as it is
// constructed (CBaseShader's constructor), as a BEGIN_SHADER block's does.
struct FixedStateShaders
{
	FixedStateShaders()
	{
		for ( const FixedStateRow &row : kRows )
			new CFixedStateShader( row );
	}
};
FixedStateShaders s_FixedStateShaders;

} // namespace
