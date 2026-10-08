//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The VMT definition rules (RFC 0001 R12); see vmt_definition.h.
//			Moved unchanged from cmaterial.cpp, with the device facts taken
//			from a VmtProfile instead of the material system's hardware config.
//
//=============================================================================//

#include "vmt_definition.h"

#include "filesystem.h"
#include "tier0/dbg.h"
#include "tier1/KeyValues.h"
#include "tier1/strtools.h"
#include "tier1/utlstring.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "render/pbr_material_schema.h"

#include <string.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Make sure these match the bits in imaterial.h (MaterialVarFlags_t)
//-----------------------------------------------------------------------------
static const char *s_pShaderStateString[] = {
    "$debug", "$no_fullbright", "$no_draw", "$use_in_fillrate_mode",

    "$vertexcolor", "$vertexalpha", "$selfillum", "$additive", "$alphatest", "$multipass",
    "$znearer", "$model", "$flat", "$nocull", "$nofog", "$ignorez", "$decal", "$envmapsphere",
    "$noalphamod", "$envmapcameraspace", "$basealphaenvmapmask", "$translucent",
    "$normalmapalphaenvmapmask", "$softwareskin", "$opaquetexture", "$envmapmode", "$nodecal",
    "$halflambert", "$wireframe", "$allowalphatocoverage",

    "" // last one must be null
};

const char *VmtMaterialVarFlagName( int i )
{
	return ( i >= 0 && i < VmtMaterialVarFlagCount() ) ? s_pShaderStateString[i] : "";
}

int VmtMaterialVarFlagCount()
{
	return sizeof( s_pShaderStateString ) / sizeof( char * ) - 1;
}

static inline bool IsWhitespace( char c )
{
	return c == ' ' || c == '\t';
}

int VmtFindMaterialVarFlag( char const *pFlagName )
{
	// Strip preceeding spaces
	while ( pFlagName[0] )
	{
		if ( !IsWhitespace( pFlagName[0] ) )
			break;

		++pFlagName;
	}

	for ( int i = 0; *s_pShaderStateString[i]; ++i )
	{
		const char *pStateString = s_pShaderStateString[i];
		const char *pFound = V_stristr( pFlagName, pStateString );

		// The found string had better start with the first non-whitespace character
		if ( pFound != pFlagName )
			continue;

		// Strip spaces at the end
		int nLen = V_strlen( pStateString );
		pFound += nLen;
		while ( true )
		{
			if ( !pFound[0] )
				return ( 1 << i );

			if ( !IsWhitespace( pFound[0] ) )
				break;

			++pFound;
		}
	}
	return 0;
}

int VmtMaterialVarFlags( KeyValues &vmtKeyValues, KeyValues *pFallbackBlock,
    const VmtProfile &profile, const char *pMaterialName )
{
	int flags = 0;
	int flagMask = 0;
	int overrideMask = 0;
	KeyValues *pBlocks[2] = { pFallbackBlock, &vmtKeyValues };
	for ( int nBlock = 0; nBlock < 2; ++nBlock )
	{
		if ( !pBlocks[nBlock] )
			continue;
		const bool parsingOverrides = ( nBlock == 0 );
		for ( KeyValues *pVar = pBlocks[nBlock]->GetFirstSubKey(); pVar; pVar = pVar->GetNextKey() )
		{
			bool bWasConditional;
			if ( VmtShouldSkipVar( pVar, profile, &bWasConditional, pMaterialName ) )
				continue;
			const int flagbit = VmtFindMaterialVarFlag( VmtVarName( pVar ) );
			if ( !flagbit )
				continue;
			// CMaterial::ParseMaterialFlag: a flag defined twice keeps its
			// first definition, and an override beats the top level.
			const int testMask = parsingOverrides ? overrideMask : flagMask;
			if ( ( testMask & flagbit ) || ( overrideMask & flagbit ) )
				continue;
			if ( parsingOverrides )
				overrideMask |= flagbit;
			else
				flagMask |= flagbit;
			if ( pVar->GetInt() )
				flags |= flagbit;
			else
				flags &= ~flagbit;
		}
	}
	return flags;
}

static bool IsUNCName( const char *pName )
{
	return pName[0] == '/' && pName[1] == '/' && pName[2] != '/';
}

void VmtMaterialFileName( const char *pNormalizedName, char *pOut, int nOut )
{
	if ( IsUNCName( pNormalizedName ) )
	{
		V_strncpy( pOut, pNormalizedName, nOut );
		return;
	}
	V_snprintf( pOut, nOut, "materials/%s", pNormalizedName );
	V_FixDoubleSlashes( pOut );
}

void VmtTextureFileName( const char *pTextureName, char *pOut, int nOut )
{
	V_snprintf(
	    pOut, nOut, IsUNCName( pTextureName ) ? "%s.vtf" : "materials/%s.vtf", pTextureName );
}

const char *VmtMissingShaderName()
{
	return ( IsWindows() && !IsEmulatingGL() ) ? "Wireframe_DX8" : "Wireframe_DX9";
}

VmtProfile VmtDedicatedServerProfile()
{
	VmtProfile profile;
	profile.dxSupportLevel = 90;
	profile.supportsPixelShaders_2_b = true;
	profile.hdrTypeNone = true;
	profile.srgbCorrectBlending = false;
	profile.gpuLevel = 3;
	profile.reduceParticles = false;
	return profile;
}

// Whether a "GPU>=n" or "GPU<n" condition holds; false for any other text.
static bool EvaluateGPULevelCondition( const char *pCond, int nGPULevel, bool *pbHolds )
{
	int nLevel = 0;
	if ( !V_strnicmp( pCond, "GPU>=", 5 ) && V_isdigit( pCond[5] ) && !pCond[6] )
	{
		nLevel = pCond[5] - '0';
		*pbHolds = nGPULevel >= nLevel;
		return true;
	}
	if ( !V_strnicmp( pCond, "GPU<", 4 ) && V_isdigit( pCond[4] ) && !pCond[5] )
	{
		nLevel = pCond[4] - '0';
		*pbHolds = nGPULevel < nLevel;
		return true;
	}
	return false;
}

char const *VmtVarName( KeyValues *pVar )
{
	char const *pVarName = pVar->GetName();
	char const *pQuestion = strchr( pVarName, '?' );
	if ( !pQuestion )
		return pVarName;
	else
		return pQuestion + 1;
}

bool VmtShouldSkipVar(
    KeyValues *pVar, const VmtProfile &profile, bool *pWasConditional, const char *pMaterialName )
{
	char const *pVarName = pVar->GetName();
	char const *pQuestion = strchr( pVarName, '?' );
	if ( ( !pQuestion ) || ( pQuestion == pVarName ) )
	{
		*pWasConditional = false; // unconditional var
		return false;
	}
	else
	{
		bool bShouldSkip = true;
		*pWasConditional = true;
		// parse the conditional part
		char pszConditionName[256];
		V_strncpy( pszConditionName, pVarName, 1 + pQuestion - pVarName );
		char const *pCond = pszConditionName;
		bool bToggle = false;
		if ( pCond[0] == '!' )
		{
			pCond++;
			bToggle = true;
		}

		if ( !stricmp( pCond, "lowfill" ) )
		{
			bShouldSkip = !profile.reduceParticles;
		}
		else if ( !stricmp( pCond, "hdr" ) )
		{
			bShouldSkip = false; //( HardwareConfig()->GetHDRType() == HDR_TYPE_NONE );
		}
		else if ( !stricmp( pCond, "srgb" ) )
		{
			bShouldSkip = ( !profile.srgbCorrectBlending );
		}
		else if ( !stricmp( pCond, "srgb_pc" ) )
		{
			// Portal 2 era: sRGB-correct blending on a PC (not a console).
			bShouldSkip = ( !profile.srgbCorrectBlending );
		}
		else if ( !stricmp( pCond, "sonyps3" ) )
		{
			bShouldSkip = true;
		}
		else if ( !stricmp( pCond, "ldr" ) )
		{
			bShouldSkip = ( !profile.hdrTypeNone );
		}
		else if ( !stricmp( pCond, "360" ) )
		{
			bShouldSkip = !false;
		}
		else if ( !stricmp( pCond, "gameconsole" ) )
		{
			bShouldSkip = !false;
		}
		else if ( bool bHolds = false;
		    EvaluateGPULevelCondition( pCond, profile.gpuLevel, &bHolds ) )
		{
			bShouldSkip = !bHolds;
		}
		else
		{
			Warning( "unrecognized conditional test %s in %s\n", pVarName, pMaterialName );
		}

		return bShouldSkip ^ bToggle;
	}
}

KeyValues *VmtFindUndeclaredVariable( KeyValues &vmtKeyValues, KeyValues *pFallbackBlock,
    const char *pName, const VmtProfile &profile, const char *pMaterialName )
{
	// CMaterial::ParseMaterialVars reads the fallback block, then the top
	// level; a later definition of a variable that is not a shader parameter
	// is ignored, conditional or not.
	KeyValues *pBlocks[2] = { pFallbackBlock, &vmtKeyValues };
	for ( KeyValues *pBlock : pBlocks )
	{
		if ( !pBlock )
			continue;
		for ( KeyValues *pVar = pBlock->GetFirstSubKey(); pVar; pVar = pVar->GetNextKey() )
		{
			bool bWasConditional;
			if ( VmtShouldSkipVar( pVar, profile, &bWasConditional, pMaterialName ) )
				continue;
			const char *pVarName = VmtVarName( pVar );
			if ( VmtFindMaterialVarFlag( pVarName ) )
				continue;
			// CreateMaterialVarFromKeyValue makes no variable from these.
			const KeyValues::types_t type = pVar->GetDataType();
			if ( type == KeyValues::TYPE_STRING )
			{
				const char *pString = pVar->GetString();
				if ( !pString || !pString[0] )
					continue;
			}
			else if ( type != KeyValues::TYPE_INT && type != KeyValues::TYPE_FLOAT )
			{
				continue;
			}
			if ( !V_stricmp( pVarName, pName ) )
				return pVar;
		}
	}
	return NULL;
}

static KeyValues *CheckConditionalFakeShaderName(
    char const *pShaderName, char const *pSuffixName, KeyValues *pKeyValues )
{
	KeyValues *pFallbackSection = pKeyValues->FindKey( pSuffixName );
	if ( pFallbackSection )
		return pFallbackSection;

	char nameBuf[256];
	V_snprintf( nameBuf, sizeof( nameBuf ), "%s_%s", pShaderName, pSuffixName );
	pFallbackSection = pKeyValues->FindKey( nameBuf );

	if ( pFallbackSection )
		return pFallbackSection;

	return NULL;
}

KeyValues *VmtFindBuiltinFallbackBlock(
    char const *pShaderName, KeyValues *pKeyValues, const VmtProfile &profile )
{
	// handle "fake" shader fallbacks which are conditional upon mode. like _hdr_dx9, etc
	// Portal 2's GPU-level blocks first, in CS:GO's order.
	static const char *const s_GPUBlocks[] = { "GPU<1", "GPU<2", "GPU>=1", "GPU>=2" };
	for ( const char *pBlock : s_GPUBlocks )
	{
		bool bHolds = false;
		if ( EvaluateGPULevelCondition( pBlock, profile.gpuLevel, &bHolds ) && bHolds )
		{
			KeyValues *pRet = CheckConditionalFakeShaderName( pShaderName, pBlock, pKeyValues );
			if ( pRet )
				return pRet;
		}
	}
	if ( profile.dxSupportLevel < 90 )
	{
		KeyValues *pRet = CheckConditionalFakeShaderName( pShaderName, "<DX90", pKeyValues );
		if ( pRet )
			return pRet;
	}
	if ( profile.dxSupportLevel < 95 )
	{
		KeyValues *pRet = CheckConditionalFakeShaderName( pShaderName, "<DX95", pKeyValues );
		if ( pRet )
			return pRet;
	}
	if ( profile.dxSupportLevel < 90 || !profile.supportsPixelShaders_2_b )
	{
		KeyValues *pRet = CheckConditionalFakeShaderName( pShaderName, "<DX90_20b", pKeyValues );
		if ( pRet )
			return pRet;
	}
	if ( profile.dxSupportLevel >= 90 && profile.supportsPixelShaders_2_b )
	{
		KeyValues *pRet = CheckConditionalFakeShaderName( pShaderName, ">=DX90_20b", pKeyValues );
		if ( pRet )
			return pRet;
	}
	if ( profile.dxSupportLevel <= 90 )
	{
		KeyValues *pRet = CheckConditionalFakeShaderName( pShaderName, "<=DX90", pKeyValues );
		if ( pRet )
			return pRet;
	}
	if ( profile.dxSupportLevel >= 90 )
	{
		KeyValues *pRet = CheckConditionalFakeShaderName( pShaderName, ">=DX90", pKeyValues );
		if ( pRet )
			return pRet;
	}
	if ( profile.dxSupportLevel > 90 )
	{
		KeyValues *pRet = CheckConditionalFakeShaderName( pShaderName, ">DX90", pKeyValues );
		if ( pRet )
			return pRet;
	}
	//	if ( HardwareConfig()->GetHDRType() != HDR_TYPE_NONE )
	{
		KeyValues *pRet = CheckConditionalFakeShaderName( pShaderName, "hdr_dx9", pKeyValues );
		if ( pRet )
			return pRet;
		pRet = CheckConditionalFakeShaderName( pShaderName, "hdr", pKeyValues );
		if ( pRet )
			return pRet;
	}
	if ( profile.hdrTypeNone )
	{
		KeyValues *pRet = CheckConditionalFakeShaderName( pShaderName, "ldr", pKeyValues );
		if ( pRet )
			return pRet;
	}
	if ( profile.srgbCorrectBlending )
	{
		KeyValues *pRet = CheckConditionalFakeShaderName( pShaderName, "srgb", pKeyValues );
		if ( pRet )
			return pRet;
	}
	if ( profile.dxSupportLevel >= 90 )
	{
		KeyValues *pRet = CheckConditionalFakeShaderName( pShaderName, "dx9", pKeyValues );
		if ( pRet )
			return pRet;
	}
	return NULL;
}

// --- Patch files and loading
void InsertKeyValues( KeyValues &dst, KeyValues &src, bool bCheckForExistence, bool bRecursive )
{
	KeyValues *pSrcVar = src.GetFirstSubKey();
	while ( pSrcVar )
	{
		if ( !bCheckForExistence || dst.FindKey( pSrcVar->GetName() ) )
		{
			switch ( pSrcVar->GetDataType() )
			{
			case KeyValues::TYPE_STRING:
				dst.SetString( pSrcVar->GetName(), pSrcVar->GetString() );
				break;
			case KeyValues::TYPE_INT:
				dst.SetInt( pSrcVar->GetName(), pSrcVar->GetInt() );
				break;
			case KeyValues::TYPE_FLOAT:
				dst.SetFloat( pSrcVar->GetName(), pSrcVar->GetFloat() );
				break;
			case KeyValues::TYPE_PTR:
				dst.SetPtr( pSrcVar->GetName(), pSrcVar->GetPtr() );
				break;
			case KeyValues::TYPE_NONE:
			{
				// Subkey. Recurse.
				KeyValues *pNewDest = dst.FindKey( pSrcVar->GetName(), true );
				Assert( pNewDest );
				InsertKeyValues( *pNewDest, *pSrcVar, bCheckForExistence, true );
			}
			break;
			}
		}
		pSrcVar = pSrcVar->GetNextKey();
	}

	if ( bRecursive && !dst.GetFirstSubKey() )
	{
		// Insert a dummy key to an empty subkey to make sure it doesn't get removed
		dst.SetInt( "__vmtpatchdummy", 1 );
	}

	if ( bCheckForExistence )
	{
		for ( KeyValues *pScan = dst.GetFirstTrueSubKey(); pScan;
		    pScan = pScan->GetNextTrueSubKey() )
		{
			KeyValues *pTmp = src.FindKey( pScan->GetName() );
			if ( !pTmp )
				continue;
			// make sure that this is a subkey.
			if ( pTmp->GetDataType() != KeyValues::TYPE_NONE )
				continue;
			InsertKeyValues( *pScan, *pTmp, bCheckForExistence );
		}
	}
}

void ApplyPatchKeyValues( KeyValues &keyValues, KeyValues &patchKeyValues )
{
	KeyValues *pInsertSection = patchKeyValues.FindKey( "insert" );
	KeyValues *pReplaceSection = patchKeyValues.FindKey( "replace" );

	if ( pInsertSection )
	{
		InsertKeyValues( keyValues, *pInsertSection, false );
	}

	if ( pReplaceSection )
	{
		InsertKeyValues( keyValues, *pReplaceSection, true );
	}

	// Could add other commands here, like "delete", "rename", etc.
}

//-----------------------------------------------------------------------------
// Adds keys from srcKeys to destKeys, overwriting any keys that are already
// there.
//-----------------------------------------------------------------------------
void MergeKeyValues( KeyValues &srcKeys, KeyValues &destKeys )
{
	for ( KeyValues *pKV = srcKeys.GetFirstValue(); pKV; pKV = pKV->GetNextValue() )
	{
		switch ( pKV->GetDataType() )
		{
		case KeyValues::TYPE_STRING:
			destKeys.SetString( pKV->GetName(), pKV->GetString() );
			break;
		case KeyValues::TYPE_INT:
			destKeys.SetInt( pKV->GetName(), pKV->GetInt() );
			break;
		case KeyValues::TYPE_FLOAT:
			destKeys.SetFloat( pKV->GetName(), pKV->GetFloat() );
			break;
		case KeyValues::TYPE_PTR:
			destKeys.SetPtr( pKV->GetName(), pKV->GetPtr() );
			break;
		}
	}
	for ( KeyValues *pKV = srcKeys.GetFirstTrueSubKey(); pKV; pKV = pKV->GetNextTrueSubKey() )
	{
		KeyValues *pDestKV = destKeys.FindKey( pKV->GetName(), true );
		MergeKeyValues( *pKV, *pDestKV );
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void AccumulatePatchKeyValues( KeyValues &srcKeyValues, KeyValues &patchKeyValues )
{
	KeyValues *pDestInsertSection = patchKeyValues.FindKey( "insert" );
	if ( pDestInsertSection == NULL )
	{
		pDestInsertSection = new KeyValues( "insert" );
		patchKeyValues.AddSubKey( pDestInsertSection );
	}

	KeyValues *pDestReplaceSection = patchKeyValues.FindKey( "replace" );
	if ( pDestReplaceSection == NULL )
	{
		pDestReplaceSection = new KeyValues( "replace" );
		patchKeyValues.AddSubKey( pDestReplaceSection );
	}

	KeyValues *pSrcInsertSection = srcKeyValues.FindKey( "insert" );
	if ( pSrcInsertSection )
	{
		MergeKeyValues( *pSrcInsertSection, *pDestInsertSection );
	}

	KeyValues *pSrcReplaceSection = srcKeyValues.FindKey( "replace" );
	if ( pSrcReplaceSection )
	{
		MergeKeyValues( *pSrcReplaceSection, *pDestReplaceSection );
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static bool IsValidPbrPatchInclude( const char *pIncludeFileName )
{
	const char prefix[] = "materials/";
	const char suffix[] = ".vmt";
	const size_t prefixLength = sizeof( prefix ) - 1;
	const size_t suffixLength = sizeof( suffix ) - 1;
	if ( !pIncludeFileName )
		return false;
	const size_t length = strlen( pIncludeFileName );
	if ( length <= prefixLength + suffixLength || length >= MAX_PATH ||
	     V_strnicmp( pIncludeFileName, prefix, prefixLength ) ||
	     V_stricmp( pIncludeFileName + length - suffixLength, suffix ) )
		return false;
	const size_t relativeLength = length - prefixLength - suffixLength;
	char relative[MAX_PATH];
	memcpy( relative, pIncludeFileName + prefixLength, relativeLength );
	relative[relativeLength] = '\0';
	return render::pbr::IsValidFallbackReference( relative );
}

bool AccumulateRecursiveVmtPatches( KeyValues &patchKeyValuesOut, KeyValues **ppBaseKeyValuesOut,
    const KeyValues &keyValues, const char *pPathID, CUtlVector<FileNameHandle_t> *pIncludes,
    bool bValidatePbrIncludes, bool *pInvalidPbrInclude )
{
	if ( pIncludes )
	{
		pIncludes->Purge();
	}

	patchKeyValuesOut.Clear();

	if ( V_stricmp( keyValues.GetName(), "patch" ) != 0 )
	{
		// Not a patch file, nothing to do
		if ( ppBaseKeyValuesOut )
		{
			// flag to the caller that the passed in keyValues are in fact final non-patch values
			*ppBaseKeyValuesOut = NULL;
		}
		return true;
	}

	KeyValues *pCurrentKeyValues = keyValues.MakeCopy();

	// Recurse down through all patch files:
	int nCount = 0;
	while ( ( nCount < 10 ) && ( V_stricmp( pCurrentKeyValues->GetName(), "patch" ) == 0 ) )
	{
		// Accumulate the new patch keys from this file
		AccumulatePatchKeyValues( *pCurrentKeyValues, patchKeyValuesOut );

		// Load the included file
		const char *pIncludeFileName = pCurrentKeyValues->GetString( "include" );
		if ( ( bValidatePbrIncludes || pInvalidPbrInclude ) &&
		     !IsValidPbrPatchInclude( pIncludeFileName ) )
		{
			if ( pInvalidPbrInclude )
				*pInvalidPbrInclude = true;
			if ( bValidatePbrIncludes )
			{
				Warning( "PBR fallback patch has an invalid include path\n" );
				pCurrentKeyValues->deleteThis();
				return false;
			}
		}

		if ( pIncludeFileName == NULL )
		{
			// A patch file without an include key? Not good...
			Warning( "VMT patch file has no include key - invalid!\n" );
			pCurrentKeyValues->deleteThis();
			return false;
		}

		CUtlString includeFileName(
		    pIncludeFileName ); // copy off the string before we clear the keyvalues it lives in
		pCurrentKeyValues->Clear();
		bool bSuccess =
		    pCurrentKeyValues->LoadFromFile( g_pFullFileSystem, includeFileName, pPathID );
		if ( bSuccess )
		{
			if ( pIncludes )
			{
				// Remember that we included this file for the pure server stuff.
				pIncludes->AddToTail( g_pFullFileSystem->FindOrAddFileName( includeFileName ) );
			}
		}
		else
		{
			pCurrentKeyValues->deleteThis();
#ifndef DEDICATED
			Warning( "Failed to load $include VMT file (%s)\n", includeFileName.String() );
#endif
			return false;
		}

		nCount++;
	}
	if ( V_stricmp( pCurrentKeyValues->GetName(), "patch" ) == 0 )
	{
		Warning( "Infinite recursion in patch file?\n" );
		pCurrentKeyValues->deleteThis();
		return false;
	}

	if ( ppBaseKeyValuesOut )
	{
		*ppBaseKeyValuesOut = pCurrentKeyValues;
	}
	else
	{
		pCurrentKeyValues->deleteThis();
	}

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool ExpandPatchFile( KeyValues &keyValues, KeyValues &patchKeyValues, const char *pPathID,
    CUtlVector<FileNameHandle_t> *pIncludes, bool bValidatePbrIncludes, bool *pInvalidPbrInclude )
{
	KeyValues *pNonPatchKeyValues = NULL;
	if ( !patchKeyValues.IsEmpty() )
	{
		pNonPatchKeyValues = keyValues.MakeCopy();
	}
	else
	{
		bool bSuccess = AccumulateRecursiveVmtPatches( patchKeyValues, &pNonPatchKeyValues,
		    keyValues, pPathID, pIncludes, bValidatePbrIncludes, pInvalidPbrInclude );
		if ( !bSuccess )
		{
			return false;
		}
	}

	if ( pNonPatchKeyValues != NULL )
	{
		// We're dealing with a patch file. Apply accumulated patches to final vmt
		ApplyPatchKeyValues( *pNonPatchKeyValues, patchKeyValues );
		keyValues = *pNonPatchKeyValues;
		pNonPatchKeyValues->deleteThis();
	}
	return true;
}

static const char *LookupPbrVmtParameter( const char *pName, void *pContext )
{
	return static_cast<KeyValues *>( pContext )->GetString( pName, "" );
}

bool LoadVMTDefinition( KeyValues &vmtKeyValues, KeyValues &patchKeyValues,
    const char *pMaterialName, bool bAbsolutePath, CUtlVector<FileNameHandle_t> *pIncludes,
    VmtShaderExistsFn pShaderExists )
{
	char pFileName[MAX_PATH];
	const char *pPathID = "GAME";
	if ( !bAbsolutePath )
	{
		Q_snprintf( pFileName, sizeof( pFileName ), "materials/%s.vmt", pMaterialName );
	}
	else
	{
		Q_snprintf( pFileName, sizeof( pFileName ), "%s.vmt", pMaterialName );
		if ( pMaterialName[0] == '/' && pMaterialName[1] == '/' && pMaterialName[2] != '/' )
		{
			// UNC, do full search
			pPathID = NULL;
		}
	}

	if ( !vmtKeyValues.LoadFromFile( g_pFullFileSystem, pFileName, pPathID ) )
	{
		return false;
	}
	bool bInvalidPbrInclude = false;
	if ( !ExpandPatchFile(
	         vmtKeyValues, patchKeyValues, pPathID, pIncludes, false, &bInvalidPbrInclude ) )
		return false;
	const render::pbr::DefinitionResult pbrDefinition = render::pbr::ValidateDefinition(
	    vmtKeyValues.GetName(), LookupPbrVmtParameter, &vmtKeyValues );
	if ( pbrDefinition.status == render::pbr::DefinitionStatus::kValid && bInvalidPbrInclude )
	{
		Warning( "PBR material %s has an invalid primary patch include\n", pMaterialName );
		return false;
	}
	if ( pbrDefinition.status == render::pbr::DefinitionStatus::kMissingRequiredParameter )
	{
		Warning( "PBR material %s is missing required parameter %s\n", pMaterialName,
		    pbrDefinition.parameter );
		return false;
	}
	if ( pbrDefinition.status == render::pbr::DefinitionStatus::kValid )
	{
		const char *pFallback = vmtKeyValues.GetString(
		    render::pbr::Parameter( render::pbr::MaterialParameter::kFallbackMaterial ).name );
		if ( !render::pbr::IsValidFallbackReference( pFallback ) ||
		     strlen( pFallback ) + sizeof( "materials/.vmt" ) >= MAX_PATH )
		{
			Warning( "PBR material %s has an invalid fallback reference\n", pMaterialName );
			return false;
		}
		char pFallbackFileName[MAX_PATH];
		Q_snprintf( pFallbackFileName, sizeof( pFallbackFileName ), "materials/%s.vmt", pFallback );
		if ( !Q_stricmp( pFallbackFileName, pFileName ) ||
		     !g_pFullFileSystem->FileExists( pFallbackFileName, "GAME" ) )
		{
			Warning(
			    "PBR material %s has a missing or self fallback %s\n", pMaterialName, pFallback );
			return false;
		}
		KeyValues *pFallbackKeys = new KeyValues( "pbr_fallback" );
		KeyValues *pFallbackPatches = new KeyValues( "pbr_fallback_patches" );
		bool bValidFallback =
		    pFallbackKeys->LoadFromFile( g_pFullFileSystem, pFallbackFileName, "GAME" );
		if ( bValidFallback )
		{
			bValidFallback =
			    ExpandPatchFile( *pFallbackKeys, *pFallbackPatches, "GAME", NULL, true );
			const char *pFallbackShader = pFallbackKeys->GetName();
			bValidFallback = bValidFallback && V_stricmp( pFallbackShader, "patch" ) != 0 &&
			                 !render::pbr::IsMetalRoughShader( pFallbackShader );
			if ( bValidFallback && pShaderExists )
				bValidFallback = pShaderExists( pFallbackShader );
		}
		pFallbackPatches->deleteThis();
		pFallbackKeys->deleteThis();
		if ( !bValidFallback )
		{
			Warning( "PBR material %s has an invalid fallback VMT %s\n", pMaterialName,
			    pFallbackFileName );
			return false;
		}
	}

	return true;
}

bool VmtLoadMaterialDefinition( const char *pMaterialName, const VmtProfile &profile,
    KeyValues **ppKeyValues, KeyValues **ppFallbackBlock, char *pNormalizedName,
    int nNormalizedSize )
{
	// CMaterialSystem::FindMaterialEx's name rules.
	char fixedName[MAX_PATH];
	char strippedName[MAX_PATH];
	V_strncpy( fixedName, pMaterialName, sizeof( fixedName ) );
	V_strlower( fixedName );
	V_FixSlashes( fixedName, '/' );
	V_StripExtension( fixedName, strippedName, sizeof( strippedName ) );
	V_strncpy( pNormalizedName, strippedName, nNormalizedSize );
	char vmtName[MAX_PATH + kVmtFileNameExtra];
	VmtMaterialFileName( strippedName, vmtName, sizeof( vmtName ) );

	KeyValues *pKeyValues = new KeyValues( "vmt" );
	KeyValues *pPatchKeyValues = new KeyValues( "vmt_patches" );
	const bool bLoaded =
	    LoadVMTDefinition( *pKeyValues, *pPatchKeyValues, vmtName, true, NULL, NULL );
	pPatchKeyValues->deleteThis();
	if ( !bLoaded )
	{
		pKeyValues->deleteThis();
		return false;
	}
	*ppKeyValues = pKeyValues;
	*ppFallbackBlock = VmtFindBuiltinFallbackBlock( VmtMissingShaderName(), pKeyValues, profile );
	return true;
}
