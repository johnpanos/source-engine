//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The rules that turn a .vmt file into a material definition (RFC
//			0001 R12): patch expansion, the PBR definition checks, the builtin
//			fallback block a device profile selects (">=DX90", "GPU>=2", "ldr")
//			and conditional variables ("srgb?$x"). One owner, shared by the
//			material system's CMaterial and the dedicated server, which reads
//			material facts such as $surfaceprop without composing a material
//			system.
//
//=============================================================================//

#ifndef VMT_DEFINITION_H
#define VMT_DEFINITION_H

#include "filesystem.h"
#include "tier1/utlvector.h"

class KeyValues;

// The facts about the running device that a definition's fallback blocks and
// conditional variables test.
struct VmtProfile
{
	int dxSupportLevel;
	bool supportsPixelShaders_2_b;
	bool hdrTypeNone; // the device reports HDR_TYPE_NONE
	bool srgbCorrectBlending;
	int gpuLevel;         // Portal 2's gpu_level ("GPU>=n" tests)
	bool reduceParticles; // mat_reduceparticles ("lowfill?" variables)
};

// The profile a dedicated server resolves definitions with: the values the
// empty shader API reported while the dedicated server composed a material
// system (DX level 90 with ps_2_b, no HDR, no sRGB-correct blending, the
// default gpu_level 3, mat_reduceparticles 0). Keeping them keeps every
// server-side material fact unchanged without a material system.
VmtProfile VmtDedicatedServerProfile();

// Whether a shader exists, for the PBR fallback check of LoadVMTDefinition; a
// null callback skips that check (no shaders are loaded, as on a server).
typedef bool ( *VmtShaderExistsFn )( const char *pShaderName );

// Patch files: "patch" { "include" "..." "insert" {} "replace" {} }.
void InsertKeyValues(
    KeyValues &dst, KeyValues &src, bool bCheckForExistence, bool bRecursive = false );
void ApplyPatchKeyValues( KeyValues &keyValues, KeyValues &patchKeyValues );
void MergeKeyValues( KeyValues &srcKeys, KeyValues &destKeys );
void AccumulatePatchKeyValues( KeyValues &srcKeyValues, KeyValues &patchKeyValues );
bool AccumulateRecursiveVmtPatches( KeyValues &patchKeyValuesOut, KeyValues **ppBaseKeyValuesOut,
    const KeyValues &keyValues, const char *pPathID, CUtlVector<FileNameHandle_t> *pIncludes,
    bool bValidatePbrIncludes = false, bool *pInvalidPbrInclude = NULL );
bool ExpandPatchFile( KeyValues &keyValues, KeyValues &patchKeyValues, const char *pPathID,
    CUtlVector<FileNameHandle_t> *pIncludes, bool bValidatePbrIncludes = false,
    bool *pInvalidPbrInclude = NULL );

// Loads materials/<name>.vmt (or <name>.vmt when bAbsolutePath), expands its
// patches and applies the PBR definition rules. False when the material is
// missing or invalid: its users then take the error material.
bool LoadVMTDefinition( KeyValues &vmtKeyValues, KeyValues &patchKeyValues,
    const char *pMaterialName, bool bAbsolutePath, CUtlVector<FileNameHandle_t> *pIncludes,
    VmtShaderExistsFn pShaderExists );

// The builtin fallback block of `pShaderName` the profile selects, or null.
KeyValues *VmtFindBuiltinFallbackBlock(
    char const *pShaderName, KeyValues *pKeyValues, const VmtProfile &profile );

// A variable's name without its condition ("srgb?$x" names "$x").
char const *VmtVarName( KeyValues *pVar );

// Whether a variable is skipped under the profile; *pWasConditional reports
// whether it had a condition. pMaterialName names the material in warnings.
bool VmtShouldSkipVar(
    KeyValues *pVar, const VmtProfile &profile, bool *pWasConditional, const char *pMaterialName );

// The variable CMaterial creates for `pName`, a name that is not a shader
// parameter: the first definition of that name (case-insensitive, condition
// removed) in the fallback block and then the top level, as
// CMaterial::ParseMaterialVars reads them. Definitions skipped under the
// profile, material flags, empty strings and blocks create no variable and
// are passed over. Null when no definition creates one.
KeyValues *VmtFindUndeclaredVariable( KeyValues &vmtKeyValues, KeyValues *pFallbackBlock,
    const char *pName, const VmtProfile &profile, const char *pMaterialName );

// The material flag names ("$translucent", "$nocull", ...): name i is flag
// bit 1 << i of MaterialVarFlags_t. Past the last name the string is empty.
const char *VmtMaterialVarFlagName( int i );
int VmtMaterialVarFlagCount();

// The flag bit a variable name selects (leading and trailing whitespace
// ignored, case-insensitive), or 0 when it names no flag.
int VmtFindMaterialVarFlag( char const *pFlagName );

// The material flags a definition sets, as CMaterial::ParseMaterialVars
// reads them: the fallback block first, then the top level; the first
// definition of a flag in each wins and the fallback block's beats the top
// level's; variables skipped under the profile are ignored.
int VmtMaterialVarFlags( KeyValues &vmtKeyValues, KeyValues *pFallbackBlock,
    const VmtProfile &profile, const char *pMaterialName );

// The file names of materials and textures (one owner of materials/ paths):
// VmtMaterialFileName gives the .vmt file of a normalized material name
// (lowercase, '/' separators, no extension) without its extension, as
// CMaterialSystem::FindMaterial loads it: under materials/, or a UNC name
// (//server/...) as is. VmtTextureFileName gives the .vtf file CTexture reads
// for a texture name. nOut covers the name plus kVmtFileNameExtra.
const int kVmtFileNameExtra = 16;
void VmtMaterialFileName( const char *pNormalizedName, char *pOut, int nOut );
void VmtTextureFileName( const char *pTextureName, char *pOut, int nOut );

// The shader CMaterial uses when a material's shader is not loaded.
const char *VmtMissingShaderName();

// Loads material `pMaterialName` as a material system with no shaders loaded
// does: found as CMaterialSystem::FindMaterial finds it (name lowercased, '/'
// separators, extension removed, under materials/), its fallback block chosen
// under the missing shader's name, as CMaterial substitutes it for the
// unloaded shader. pNormalizedName receives the name without materials/ and
// the extension. On success the caller owns *ppKeyValues (deleteThis) and
// *ppFallbackBlock, possibly null, points into it; false when it does not load
// (its users take the error material).
bool VmtLoadMaterialDefinition( const char *pMaterialName, const VmtProfile &profile,
    KeyValues **ppKeyValues, KeyValues **ppFallbackBlock, char *pNormalizedName,
    int nNormalizedSize );

#endif // VMT_DEFINITION_H
