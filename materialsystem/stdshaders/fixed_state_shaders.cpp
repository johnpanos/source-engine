//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shaders whose whole behaviour is fixed state, served from one table
// (RFC 0016 K9, R91: the first rows of the parameter-only provider that
// replaces stdshaders). Each row is a shader the material system knows by
// name: its init-time flags, what its snapshot enables and the vertex format
// it asks for. A fallback row only names the shader it falls back to. A row
// may declare one integer parameter beyond the base set.
//
//===========================================================================//

#include "BaseVSShader.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar gl_amd_occlusion_workaround( "gl_amd_occlusion_workaround", "1" );
static ConVar r_showz_power( "r_showz_power", "1.0f", FCVAR_CHEAT );

namespace
{

enum : unsigned
{
	kColorWrites = 1u << 0,     // leave colour writes on (else off)
	kAlphaWrites = 1u << 1,     // leave alpha writes on (else off)
	kDepthWrites = 1u << 2,     // leave depth writes on (else off)
	kSrgbWrite = 1u << 3,       // EnableSRGBWrite( true )
	kFogToFogColor = 1u << 4,   // FogToFogColor()
	kAmdOcclusionSrgb = 1u << 5, // EnableSRGBWrite( true ) under gl_amd_occlusion_workaround
	kSampler0 = 1u << 6,        // EnableTexture( SHADER_SAMPLER0, true ) first
	kPlainFormat = 1u << 7,     // VERTEX_POSITION, uncompressed (else compressed)
	kAllWrites = kColorWrites | kAlphaWrites | kDepthWrites
};

// One integer parameter beyond the base set, set to its default at init when
// the material leaves it undefined.
struct FixedStateParam
{
	const char *name;
	const char *defaultValue;
	const char *help;
};

struct FixedStateRow
{
	const char *name;
	const char *fallback; // non-null: the row only falls back to this shader
	int shaderFlags;
	int initFlags2;
	unsigned state;
	FixedStateParam param; // name null: none
};

const FixedStateRow kRows[] = {
	{ "WriteZ", "WriteZ_DX9", 0, 0, 0 },
	{ "WriteZ_DX9", nullptr, SHADER_NOT_EDITABLE, 0, kDepthWrites },
	{ "WriteStencil", "WriteStencil_DX9", 0, 0, 0 },
	{ "WriteStencil_DX9", nullptr, SHADER_NOT_EDITABLE, 0, 0 },
	{ "Occlusion", "Occlusion_DX9", 0, 0, 0 },
	{ "Occlusion_DX9", nullptr, SHADER_NOT_EDITABLE, 0, kAmdOcclusionSrgb },
	{ "Black", nullptr, 0, MATERIAL_VAR2_SUPPORTS_HW_SKINNING,
	    kAllWrites | kSrgbWrite | kFogToFogColor },
	{ "HSV", nullptr, SHADER_NOT_EDITABLE, MATERIAL_VAR2_NEEDS_FULL_FRAME_BUFFER_TEXTURE,
	    kAllWrites | kSampler0 | kPlainFormat },
	{ "showz", nullptr, SHADER_NOT_EDITABLE, 0, kAllWrites | kSampler0 | kPlainFormat | kSrgbWrite,
	    { "$ALPHADEPTH", "0", "Depth is stored in alpha channel" } },
};

class CFixedStateShader : public CBaseVSShader
{
public:
	explicit CFixedStateShader( const FixedStateRow &row ) : m_Row( row ) {}

	char const *GetName() const override { return m_Row.name; }
	int GetFlags() const override { return m_Row.shaderFlags; }
	char const *GetFallbackShader( IMaterialVar **params ) const override
	{
		return m_Row.fallback;
	}
	int GetNumParams() const override
	{
		return CBaseVSShader::GetNumParams() + ( m_Row.param.name ? 1 : 0 );
	}
	char const *GetParamName( int param ) const override
	{
		return IsRowParam( param ) ? m_Row.param.name : CBaseVSShader::GetParamName( param );
	}
	char const *GetParamHelp( int param ) const override
	{
		return IsRowParam( param ) ? m_Row.param.help : CBaseVSShader::GetParamHelp( param );
	}
	ShaderParamType_t GetParamType( int param ) const override
	{
		return IsRowParam( param ) ? SHADER_PARAM_TYPE_INTEGER : CBaseVSShader::GetParamType( param );
	}
	char const *GetParamDefault( int param ) const override
	{
		return IsRowParam( param ) ? m_Row.param.defaultValue
		                           : CBaseVSShader::GetParamDefault( param );
	}
	int GetParamFlags( int param ) const override
	{
		return IsRowParam( param ) ? 0 : CBaseVSShader::GetParamFlags( param );
	}

protected:
	void OnInitShaderParams( IMaterialVar **params, const char *pMaterialName ) override
	{
		if ( m_Row.initFlags2 )
			SET_FLAGS2( MaterialVarFlags2_t( m_Row.initFlags2 ) );
		if ( m_Row.param.name && !params[RowParam()]->IsDefined() )
			params[RowParam()]->SetIntValue( atoi( m_Row.param.defaultValue ) );
	}
	void OnInitShaderInstance(
	    IMaterialVar **params, IShaderInit *pShaderInit, const char *pMaterialName ) override
	{
	}
	void OnDrawElements( IMaterialVar **params, IShaderShadow *pShaderShadow,
	    IShaderDynamicAPI *pShaderAPI, VertexCompressionType_t vertexCompression,
	    CBasePerMaterialContextData **pContextDataPtr ) override
	{
		if ( m_Row.fallback )
			return;
		SHADOW_STATE
		{
			const unsigned state = m_Row.state;
			if ( state & kSampler0 )
				pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			if ( !( state & kColorWrites ) )
				pShaderShadow->EnableColorWrites( false );
			if ( !( state & kAlphaWrites ) )
				pShaderShadow->EnableAlphaWrites( false );
			if ( !( state & kDepthWrites ) )
				pShaderShadow->EnableDepthWrites( false );
			if ( ( state & kAmdOcclusionSrgb ) &&
			     g_pHardwareConfig->PlatformRequiresNonNullPixelShaders() &&
			     ( IsLinux() || IsWindows() || IsBSD() ) && gl_amd_occlusion_workaround.GetBool() )
				pShaderShadow->EnableSRGBWrite( true );
			unsigned int flags = VERTEX_POSITION;
			if ( !( state & kPlainFormat ) )
				flags |= VERTEX_FORMAT_COMPRESSED;
			pShaderShadow->VertexShaderVertexFormat( flags, 1, NULL, 0 );
			if ( state & kSrgbWrite )
				pShaderShadow->EnableSRGBWrite( true );
			if ( state & kFogToFogColor )
				FogToFogColor();
		}
		Draw();
	}

private:
	int RowParam() const { return CBaseVSShader::GetNumParams(); }
	bool IsRowParam( int param ) const { return m_Row.param.name && param == RowParam(); }

	const FixedStateRow &m_Row;
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
