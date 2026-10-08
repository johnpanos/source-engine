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
	AmdOcclusion,   // EnableSRGBWrite( true ) under gl_amd_occlusion_workaround
	OsxSrgb,        // EnableSRGBRead( SAMPLER0, x ); EnableSRGBWrite( x ), x = OSX sRGB RTs
	CompressedPos,  // VertexShaderVertexFormat( POSITION | COMPRESSED, 1, NULL, 0 )
	Pos1,           // VertexShaderVertexFormat( POSITION, 1, 0, 0 )
	Pos3EyeGlint,   // VertexShaderVertexFormat( POSITION, 3, { 2, 2, 3 }, 0 )
	FogToFogColor,  // FogToFogColor()
};

struct Step
{
	Op op;
	int a;
	int b;
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
// An unused slot keeps index -1 (never 0: that is $FLAGS).
struct Load
{
	bool extra = false;
	int index = -1;
	bool ifDefined = false;
};

constexpr int kMaxParams = 9;
constexpr int kMaxLoads = 2;
constexpr int kMaxSteps = 8;

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
};

#define NO_PARAMS {}
#define NO_LOADS { { false, -1, false } }
#define T SHADER_PARAM_TYPE_TEXTURE
#define F SHADER_PARAM_TYPE_FLOAT
#define I SHADER_PARAM_TYPE_INTEGER
#define V4 SHADER_PARAM_TYPE_VEC4
#define S SHADER_PARAM_TYPE_STRING

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
		return m_Row.fallback;
	}
	int GetNumParams() const override { return CBaseVSShader::GetNumParams() + m_nParams; }
	char const *GetParamName( int param ) const override
	{
		const Param *p = RowParam( param );
		return p ? p->name : CBaseVSShader::GetParamName( param );
	}
	char const *GetParamHelp( int param ) const override
	{
		const Param *p = RowParam( param );
		return p ? p->help : CBaseVSShader::GetParamHelp( param );
	}
	ShaderParamType_t GetParamType( int param ) const override
	{
		const Param *p = RowParam( param );
		return p ? p->type : CBaseVSShader::GetParamType( param );
	}
	char const *GetParamDefault( int param ) const override
	{
		const Param *p = RowParam( param );
		return p ? p->defaultValue : CBaseVSShader::GetParamDefault( param );
	}
	int GetParamFlags( int param ) const override
	{
		return RowParam( param ) ? 0 : CBaseVSShader::GetParamFlags( param );
	}

protected:
	void OnInitShaderParams( IMaterialVar **params, const char *pMaterialName ) override
	{
		if ( m_Row.initFlags2 )
			SET_FLAGS2( MaterialVarFlags2_t( m_Row.initFlags2 ) );
		if ( m_Row.intDefault >= 0 )
		{
			IMaterialVar *var = params[Extra( m_Row.intDefault )];
			if ( !var->IsDefined() )
				var->SetIntValue( m_Row.intDefaultValue );
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
			if ( !load.ifDefined || params[param]->IsDefined() )
				LoadTexture( param );
		}
	}
	void OnDrawElements( IMaterialVar **params, IShaderShadow *pShaderShadow,
	    IShaderDynamicAPI *pShaderAPI, VertexCompressionType_t vertexCompression,
	    CBasePerMaterialContextData **pContextDataPtr ) override
	{
		if ( m_Row.fallback )
			return;
		SHADOW_STATE
		{
			for ( const Step &step : m_Row.steps )
			{
				if ( step.op == Op::End )
					break;
				Apply( step, pShaderShadow );
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

	void Apply( const Step &step, IShaderShadow *pShaderShadow )
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
