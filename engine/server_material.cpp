//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Material definitions for the dedicated product (RFC 0001 R12); see
//			server_material.h.
//
//=============================================================================//

#include "server_material.h"

#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "studio.h"
#include "tier1/KeyValues.h"
#include "tier1/strtools.h"
#include "tier1/utldict.h"
#include "tier1/utlvector.h"
#include "tier1/utlbuffer.h"
#include "filesystem.h"
#include "texturecontainer/vtf_container.h"
#include "../materialsystem/vmt_definition.h"

#include <stdlib.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace
{

// A variable of a definition: its text, with the int and float values
// CMaterialVar::SetStringValue derives from it (or the definition's own int or
// float). Vectors and matrices keep their text: no server consumer reads them.
class CServerMaterialVar final : public IMaterialVar
{
public:
	CServerMaterialVar( IMaterial *pOwner, const char *pName, KeyValues *pDefinition )
	    : m_pOwner( pOwner )
	{
		m_Name = pName;
		m_bFakeMaterialVar = false;
		m_nTempIndex = 0;
		m_nNumVectorComps = 4;
		m_pStringVal = NULL;
		m_intVal = 0;
		m_VecVal.Init( 0.0f, 0.0f, 0.0f, 0.0f );
		if ( !pDefinition )
		{
			m_Type = MATERIAL_VAR_TYPE_UNDEFINED;
			m_pStringVal = new char[1];
			m_pStringVal[0] = 0;
			return;
		}
		const char *pText = pDefinition->GetString();
		const int nLen = V_strlen( pText ) + 1;
		m_pStringVal = new char[nLen];
		V_strncpy( m_pStringVal, pText, nLen );
		switch ( pDefinition->GetDataType() )
		{
		case KeyValues::TYPE_INT:
			m_Type = MATERIAL_VAR_TYPE_INT;
			m_intVal = pDefinition->GetInt();
			m_VecVal[0] = m_VecVal[1] = m_VecVal[2] = m_VecVal[3] = (float)m_intVal;
			break;
		case KeyValues::TYPE_FLOAT:
			m_Type = MATERIAL_VAR_TYPE_FLOAT;
			m_VecVal[0] = m_VecVal[1] = m_VecVal[2] = m_VecVal[3] = pDefinition->GetFloat();
			m_intVal = (int)m_VecVal[0];
			break;
		default:
			m_Type = MATERIAL_VAR_TYPE_STRING;
			m_intVal = atoi( m_pStringVal );
			m_VecVal[0] = m_VecVal[1] = m_VecVal[2] = m_VecVal[3] = atof( m_pStringVal );
			break;
		}
	}

	~CServerMaterialVar() { delete[] m_pStringVal; }

	ITexture *GetTextureValue() override { return NULL; }
	char const *GetName() const override { return m_Name.String(); }
	MaterialVarSym_t GetNameAsSymbol() const override
	{
		return (MaterialVarSym_t)(UtlSymId_t)m_Name;
	}
	// A definition is read only: the dedicated server's variables do not change.
	void SetFloatValue( float ) override {}
	void SetIntValue( int ) override {}
	void SetStringValue( char const * ) override {}
	char const *GetStringValue() const override { return m_pStringVal; }
	void SetFourCCValue( FourCC, void * ) override {}
	void GetFourCCValue( FourCC *type, void **ppData ) override
	{
		*type = 0;
		*ppData = NULL;
	}
	void SetVecValue( float const *, int ) override {}
	void SetVecValue( float, float ) override {}
	void SetVecValue( float, float, float ) override {}
	void SetVecValue( float, float, float, float ) override {}
	void GetLinearVecValue( float *val, int numcomps ) const override
	{
		for ( int i = 0; i < numcomps && i < 4; ++i )
			val[i] = m_VecVal[i];
	}
	void SetTextureValue( ITexture * ) override {}
	IMaterial *GetMaterialValue() override { return NULL; }
	void SetMaterialValue( IMaterial * ) override {}
	bool IsDefined() const override { return m_Type != MATERIAL_VAR_TYPE_UNDEFINED; }
	void SetUndefined() override {}
	void SetMatrixValue( VMatrix const & ) override {}
	const VMatrix &GetMatrixValue() override
	{
		static VMatrix s_identity( 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 );
		return s_identity;
	}
	bool MatrixIsIdentity() const override { return true; }
	void CopyFrom( IMaterialVar * ) override {}
	void SetValueAutodetectType( char const * ) override {}
	IMaterial *GetOwningMaterial() override { return m_pOwner; }
	void SetVecComponentValue( float, int ) override {}
	int GetIntValueInternal() const override { return m_intVal; }
	float GetFloatValueInternal() const override { return m_VecVal[0]; }
	float const *GetVecValueInternal() const override { return m_VecVal.Base(); }
	void GetVecValueInternal( float *val, int numcomps ) const override
	{
		GetLinearVecValue( val, numcomps );
	}
	int VectorSizeInternal() const override { return m_nNumVectorComps; }

private:
	IMaterial *m_pOwner;
};

// A material definition: its name, variables and flags, read once.
class CServerMaterial final : public IMaterial
{
public:
	CServerMaterial( const char *pName, KeyValues *pKeyValues, KeyValues *pFallbackBlock )
	    : m_pKeyValues( pKeyValues ), m_pFallbackBlock( pFallbackBlock )
	{
		V_strncpy( m_name, pName, sizeof( m_name ) );
		const VmtProfile profile = VmtDedicatedServerProfile();
		m_flags =
		    pKeyValues ? VmtMaterialVarFlags( *pKeyValues, pFallbackBlock, profile, m_name ) : 0;
		m_bHasProxy = pKeyValues && pKeyValues->FindKey( "Proxies" ) != NULL;
	}

	~CServerMaterial()
	{
		m_vars.PurgeAndDeleteElements();
		if ( m_pKeyValues )
			m_pKeyValues->deleteThis();
	}

	const char *GetName() const override { return m_name; }
	const char *GetTextureGroupName() const override { return ""; }
	PreviewImageRetVal_t GetPreviewImageProperties(
	    int *width, int *height, ImageFormat *imageFormat, bool *isTranslucent ) const override
	{
		*width = *height = 0;
		*imageFormat = IMAGE_FORMAT_UNKNOWN;
		*isTranslucent = false;
		return MATERIAL_NO_PREVIEW_IMAGE;
	}
	PreviewImageRetVal_t GetPreviewImage( unsigned char *, int, int, ImageFormat ) const override
	{
		return MATERIAL_NO_PREVIEW_IMAGE;
	}
	// CMaterial's representative texture: its mapping size and frame count, as
	// CTexture::Precache reads them from the VTF header (sprite models' size
	// and frames on a server).
	int GetMappingWidth() override
	{
		ResolveRepresentativeTexture();
		return m_nMappingWidth;
	}
	int GetMappingHeight() override
	{
		ResolveRepresentativeTexture();
		return m_nMappingHeight;
	}
	int GetNumAnimationFrames() override
	{
		ResolveRepresentativeTexture();
		return m_nFrameCount;
	}
	bool InMaterialPage() override { return false; }
	void GetMaterialOffset( float *pOffset ) override { pOffset[0] = pOffset[1] = 0.0f; }
	void GetMaterialScale( float *pScale ) override { pScale[0] = pScale[1] = 1.0f; }
	IMaterial *GetMaterialPage() override { return NULL; }

	IMaterialVar *FindVar( const char *varName, bool *found, bool complain = true ) override
	{
		for ( CServerMaterialVar *pVar : m_vars )
		{
			if ( !V_stricmp( pVar->GetName(), varName ) )
			{
				if ( found )
					*found = pVar->IsDefined();
				return pVar;
			}
		}
		KeyValues *pDefinition = NULL;
		if ( m_pKeyValues )
		{
			pDefinition = VmtFindUndeclaredVariable(
			    *m_pKeyValues, m_pFallbackBlock, varName, VmtDedicatedServerProfile(), m_name );
		}
		CServerMaterialVar *pVar = new CServerMaterialVar( this, varName, pDefinition );
		m_vars.AddToTail( pVar );
		if ( found )
			*found = pVar->IsDefined();
		return pVar;
	}

	// Definitions live until ServerMaterial_Shutdown; references do not own them.
	void IncrementReferenceCount() override { ++m_nReferences; }
	void DecrementReferenceCount() override { --m_nReferences; }
	int GetEnumerationID() const override { return 0; }
	void GetLowResColorSample( float, float, float *color ) const override
	{
		color[0] = color[1] = color[2] = 0.0f;
	}
	void RecomputeStateSnapshots() override {}
	bool IsTranslucent() override { return ( m_flags & MATERIAL_VAR_TRANSLUCENT ) != 0; }
	bool IsAlphaTested() override { return ( m_flags & MATERIAL_VAR_ALPHATEST ) != 0; }
	bool IsVertexLit() override { return false; }
	VertexFormat_t GetVertexFormat() const override { return 0; }
	bool HasProxy() const override { return m_bHasProxy; }
	bool UsesEnvCubemap() override { return false; }
	bool NeedsTangentSpace() override { return false; }
	bool NeedsPowerOfTwoFrameBufferTexture( bool ) override { return false; }
	bool NeedsFullFrameBufferTexture( bool ) override { return false; }
	bool NeedsSoftwareSkinning() override { return false; }
	void AlphaModulate( float ) override {}
	void ColorModulate( float, float, float ) override {}
	void SetMaterialVarFlag( MaterialVarFlags_t, bool ) override {}
	bool GetMaterialVarFlag( MaterialVarFlags_t flag ) const override
	{
		return ( m_flags & flag ) != 0;
	}
	void GetReflectivity( Vector &reflect ) override { reflect.Init( 0.0f, 0.0f, 0.0f ); }
	bool GetPropertyFlag( MaterialPropertyTypes_t ) override { return false; }
	bool IsTwoSided() override { return ( m_flags & MATERIAL_VAR_NOCULL ) != 0; }
	void SetShader( const char * ) override {}
	int GetNumPasses() override { return 0; }
	int GetTextureMemoryBytes() override { return 0; }
	void Refresh() override {}
	bool NeedsLightmapBlendAlpha() override { return false; }
	bool NeedsSoftwareLighting() override { return false; }
	int ShaderParamCount() const override { return 0; }
	IMaterialVar **GetShaderParams() override { return NULL; }
	bool IsErrorMaterial() const override { return m_pKeyValues == NULL; }
	void SetUseFixedFunctionBakedLighting( bool ) override {}
	float GetAlphaModulation() override { return 1.0f; }
	void GetColorModulation( float *r, float *g, float *b ) override { *r = *g = *b = 1.0f; }
	MorphFormat_t GetMorphFormat() const override { return 0; }
	IMaterialVar *FindVarFast( char const *pVarName, unsigned int * ) override
	{
		return FindVar( pVarName, NULL, false );
	}
	void SetShaderAndParams( KeyValues * ) override {}
	const char *GetShaderName() const override
	{
		return m_pKeyValues ? m_pKeyValues->GetName() : "";
	}
	void DeleteIfUnreferenced() override {}
	bool IsSpriteCard() override { return false; }
	void CallBindProxy( void * ) override {}
	IMaterial *CheckProxyReplacement( void * ) override { return this; }
	void RefreshPreservingMaterialVars() override {}
	bool WasReloadedFromWhitelist() override { return false; }
	bool IsPrecached() const override { return true; }

private:
	// CMaterial::FindRepresentativeTexture's order; the error texture (32x32,
	// one frame) when none is defined or its VTF header does not load.
	void ResolveRepresentativeTexture()
	{
		if ( m_bResolvedTexture )
			return;
		m_bResolvedTexture = true;
		m_nMappingWidth = m_nMappingHeight = 32;
		m_nFrameCount = 1;
		static const char *const s_pTextureVars[] = {
		    "$baseTexture", "$envmapmask", "$bumpmap", "$dudvmap", "$normalmap" };
		for ( const char *pVarName : s_pTextureVars )
		{
			bool bFound = false;
			IMaterialVar *pVar = FindVar( pVarName, &bFound, false );
			const char *pTextureName = bFound ? pVar->GetStringValue() : NULL;
			if ( !pTextureName || !pTextureName[0] )
				continue;
			ReadTextureHeader( pTextureName );
			return;
		}
	}

	// CTextureManager's name rules and CTexture::Precache's header read.
	void ReadTextureHeader( const char *pTextureName )
	{
		char fixedName[MAX_PATH];
		char strippedName[MAX_PATH];
		V_strncpy( fixedName, pTextureName, sizeof( fixedName ) );
		V_strlower( fixedName );
		V_FixSlashes( fixedName, '/' );
		V_StripExtension( fixedName, strippedName, sizeof( strippedName ) );
		char fileName[MAX_PATH + kVmtFileNameExtra];
		VmtTextureFileName( strippedName, fileName, sizeof( fileName ) );
		// The header's fields; the container reader validates its size.
		CUtlBuffer buf;
		if ( !g_pFullFileSystem->ReadFile( fileName, NULL, buf, 4096 ) )
			return;
		const auto header = texturecontainer::vtf::ReadHeader( std::span<const std::byte>(
		    static_cast<const std::byte *>( buf.Base() ), (size_t)buf.TellPut() ) );
		if ( !header )
			return;
		m_nMappingWidth = header.Value().width;
		m_nMappingHeight = header.Value().height;
		m_nFrameCount = header.Value().frames;
	}

	char m_name[MAX_PATH];
	bool m_bResolvedTexture = false;
	int m_nMappingWidth = 32;
	int m_nMappingHeight = 32;
	int m_nFrameCount = 1;
	KeyValues *m_pKeyValues;
	KeyValues *m_pFallbackBlock;
	int m_flags = 0;
	bool m_bHasProxy = false;
	int m_nReferences = 0;
	CUtlVector<CServerMaterialVar *> m_vars;
};

CUtlDict<CServerMaterial *, int> g_ServerMaterials;
CServerMaterial *g_pServerErrorMaterial = NULL;

} // namespace

IMaterial *ServerMaterial_Find( const char *pName )
{
	KeyValues *pKeyValues = NULL;
	KeyValues *pFallbackBlock = NULL;
	char normalizedName[MAX_PATH];
	// The name rules are VmtLoadMaterialDefinition's; normalize once to key the
	// dictionary, then load only names not seen before.
	V_strncpy( normalizedName, pName, sizeof( normalizedName ) );
	V_strlower( normalizedName );
	V_FixSlashes( normalizedName, '/' );
	char key[MAX_PATH];
	V_StripExtension( normalizedName, key, sizeof( key ) );
	const int index = g_ServerMaterials.Find( key );
	if ( index != g_ServerMaterials.InvalidIndex() )
		return g_ServerMaterials[index];

	CServerMaterial *pMaterial;
	if ( VmtLoadMaterialDefinition( pName, VmtDedicatedServerProfile(), &pKeyValues,
	         &pFallbackBlock, normalizedName, sizeof( normalizedName ) ) )
	{
		pMaterial = new CServerMaterial( normalizedName, pKeyValues, pFallbackBlock );
	}
	else
	{
		if ( !g_pServerErrorMaterial )
			g_pServerErrorMaterial = new CServerMaterial( "___error", NULL, NULL );
		pMaterial = g_pServerErrorMaterial;
	}
	g_ServerMaterials.Insert( key, pMaterial );
	return pMaterial;
}

int ServerMaterial_GetStudioMaterialList(
    studiohdr_t *pStudioHdr, int count, IMaterial **ppMaterials )
{
	if ( !pStudioHdr || pStudioHdr->textureindex == 0 )
		return 0;

	int found = 0;
	for ( int i = 0; i < pStudioHdr->numtextures; i++ )
	{
		IMaterial *pMaterial = NULL;
		// iterate quietly through all specified directories until a valid material is found
		for ( int j = 0; j < pStudioHdr->numcdtextures && IsErrorMaterial( pMaterial ); j++ )
		{
			const char *textureName = pStudioHdr->pTexture( i )->pszName();
			if ( textureName[0] == CORRECT_PATH_SEPARATOR ||
			     textureName[0] == INCORRECT_PATH_SEPARATOR )
				++textureName;
			const char *pCdTexture = pStudioHdr->pCdtexture( j );
			if ( pCdTexture[0] == CORRECT_PATH_SEPARATOR ||
			     pCdTexture[0] == INCORRECT_PATH_SEPARATOR )
				++pCdTexture;
			char szPath[MAX_PATH];
			V_ComposeFileName( pCdTexture, textureName, szPath, sizeof( szPath ) );
			pMaterial = ServerMaterial_Find( ( pStudioHdr->flags & STUDIOHDR_FLAGS_OBSOLETE )
			                                     ? "models/obsolete/obsolete"
			                                     : szPath );
		}
		if ( !pMaterial )
			continue;
		if ( found >= count )
			break;
		int k;
		for ( k = 0; k < found; k++ )
		{
			if ( ppMaterials[k] == pMaterial )
				break;
		}
		if ( k >= found )
			ppMaterials[found++] = pMaterial;
	}
	return found;
}

extern IMaterial *g_materialEmpty;

void ServerMaterial_Init()
{
	g_materialEmpty = ServerMaterial_Find( "debug/debugempty" );
}

void ServerMaterial_Shutdown()
{
	g_materialEmpty = NULL;
	for ( int i = g_ServerMaterials.First(); i != g_ServerMaterials.InvalidIndex();
	    i = g_ServerMaterials.Next( i ) )
	{
		if ( g_ServerMaterials[i] != g_pServerErrorMaterial )
			delete g_ServerMaterials[i];
	}
	g_ServerMaterials.RemoveAll();
	delete g_pServerErrorMaterial;
	g_pServerErrorMaterial = NULL;
}
