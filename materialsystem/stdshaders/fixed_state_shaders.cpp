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
	CompressedPos,  // VertexShaderVertexFormat( POSITION | COMPRESSED, 1, NULL, 0 )
	Pos1,           // VertexShaderVertexFormat( POSITION, 1, 0, 0 )
	Pos3EyeGlint,   // VertexShaderVertexFormat( POSITION, 3, { 2, 2, 3 }, 0 )
	FogToFogColor,  // FogToFogColor()
	FogToWhite,     // FogToWhite()
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
};

// When a step runs.
enum class When
{
	Always,
	Flag,    // MATERIAL_VAR_* c is set
	NotFlag, // MATERIAL_VAR_* c is clear
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
	ShaderParamType_t type = SHADER_PARAM_TYPE_FLOAT; // FLOAT or VEC2
	float x = 0.0f;
	float y = 0.0f;
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

constexpr int kMaxParams = 12;
constexpr int kMaxLoads = 3;
constexpr int kMaxSteps = 16;

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
	Default defaults[5] = {};
	Override overrides[2] = {};
	// A SHADER_FALLBACK block's answer, as opposed to a DEFINE_FALLBACK_SHADER
	// alias (fallback above): the shader exists in its own right.
	const char *shaderFallback = nullptr;
};

constexpr int kDistortMapFlags = TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD |
    TEXTUREFLAGS_NODEBUGOVERRIDE | TEXTUREFLAGS_SINGLECOPY | TEXTUREFLAGS_CLAMPS |
    TEXTUREFLAGS_CLAMPT;

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
		return m_Row.fallback ? m_Row.fallback : m_Row.shaderFallback;
	}
	int GetNumParams() const override { return CBaseVSShader::GetNumParams() + m_nParams; }
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
			else
				var->SetFloatValue( value.x );
		}
	}
	void OnInitShaderInstance(
	    IMaterialVar **params, IShaderInit *pShaderInit, const char *pMaterialName ) override
	{
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
			case LoadAs::SkyTexture:
				LoadTexture( param, IsSixteenBitPerChannel( params[param]->GetTextureValue() )
				                        ? 0 : TEXTUREFLAGS_SRGB );
				break;
			case LoadAs::OsxSrgbTexture:
				LoadTexture( param, IsOSX() && g_pHardwareConfig->CanDoSRGBReadFromRTs()
				                        ? TEXTUREFLAGS_SRGB : 0 );
				break;
			}
		}
	}
	void OnDrawElements( IMaterialVar **params, IShaderShadow *pShaderShadow,
	    IShaderDynamicAPI *pShaderAPI, VertexCompressionType_t vertexCompression,
	    CBasePerMaterialContextData **pContextDataPtr ) override
	{
		if ( m_Row.fallback || m_Row.shaderFallback )
			return;
		SHADOW_STATE
		{
			for ( const Step &step : m_Row.steps )
			{
				if ( step.op == Op::End )
					break;
				if ( step.when == When::Flag && !IS_FLAG_SET( step.c ) )
					continue;
				if ( step.when == When::NotFlag && IS_FLAG_SET( step.c ) )
					continue;
				Apply( step, params, pShaderShadow );
			}
		}
		Draw();
	}

private:
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
			pShaderShadow->VertexShaderVertexFormat( step.a, step.b, 0, 0 );
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
