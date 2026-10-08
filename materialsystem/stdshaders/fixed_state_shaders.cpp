//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shaders whose whole behaviour is fixed state, served from one table
// (RFC 0016 K9, R91: the first rows of the parameter-only provider that
// replaces stdshaders). Each row is a shader the material system knows by
// name: its init-time flags, what its snapshot enables and the vertex format
// it asks for. A fallback row only names the shader it falls back to. No row
// has parameters beyond the base set.
//
//===========================================================================//

#include "BaseVSShader.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar gl_amd_occlusion_workaround( "gl_amd_occlusion_workaround", "1" );

namespace
{

enum : unsigned
{
	kColorWrites = 1u << 0,     // leave colour writes on (else off)
	kAlphaWrites = 1u << 1,     // leave alpha writes on (else off)
	kDepthWrites = 1u << 2,     // leave depth writes on (else off)
	kSrgbWrite = 1u << 3,       // EnableSRGBWrite( true )
	kFogToFogColor = 1u << 4,   // FogToFogColor()
	kAmdOcclusionSrgb = 1u << 5 // EnableSRGBWrite( true ) under gl_amd_occlusion_workaround
};

struct FixedStateRow
{
	const char *name;
	const char *fallback; // non-null: the row only falls back to this shader
	int shaderFlags;
	int initFlags2;
	unsigned state;
};

const FixedStateRow kRows[] = {
	{ "WriteZ", "WriteZ_DX9", 0, 0, 0 },
	{ "WriteZ_DX9", nullptr, SHADER_NOT_EDITABLE, 0, kDepthWrites },
	{ "WriteStencil", "WriteStencil_DX9", 0, 0, 0 },
	{ "WriteStencil_DX9", nullptr, SHADER_NOT_EDITABLE, 0, 0 },
	{ "Occlusion", "Occlusion_DX9", 0, 0, 0 },
	{ "Occlusion_DX9", nullptr, SHADER_NOT_EDITABLE, 0, kAmdOcclusionSrgb },
	{ "Black", nullptr, 0, MATERIAL_VAR2_SUPPORTS_HW_SKINNING,
	    kColorWrites | kAlphaWrites | kDepthWrites | kSrgbWrite | kFogToFogColor },
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

protected:
	void OnInitShaderParams( IMaterialVar **params, const char *pMaterialName ) override
	{
		if ( m_Row.initFlags2 )
			SET_FLAGS2( MaterialVarFlags2_t( m_Row.initFlags2 ) );
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
			unsigned int flags = VERTEX_POSITION | VERTEX_FORMAT_COMPRESSED;
			pShaderShadow->VertexShaderVertexFormat( flags, 1, NULL, 0 );
			if ( state & kSrgbWrite )
				pShaderShadow->EnableSRGBWrite( true );
			if ( state & kFogToFogColor )
				FogToFogColor();
		}
		Draw();
	}

private:
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
