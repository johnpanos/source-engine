//========= Portal 2 reconstruction ============================================//
//
// Purpose: Portal 2's system levels: cpu_level, gpu_level, mem_level and
//          gpu_mem_level, and the configuration files they select.
//
// Retail Portal 2 (the CS:GO-era client's ConfigureCurrentSystemLevel and
// videocfg's UpdateSystemLevel) reads cfg/<kind>_level_<n>_pc.ekv for each
// level, ICE-encrypted KeyValues, merges them (per convar, the lower or the
// higher value wins) and sets the whitelisted convars they name: mat_picmip
// -1 at high memory levels, detail distances, decal limits and so on. This
// SDK engine has none of it, so textures stayed at mat_picmip 0 and clamped
// to their LOD resource. Not original Valve source; the repository's
// provenance warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "filesystem.h"
#include "mathlib/IceKey.H"
#include "tier0/icommandline.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static void ConfigureCurrentSystemLevel();

static void OnSystemLevelChanged( IConVar *var, const char *pOldValue, float flOldValue )
{
	ConfigureCurrentSystemLevel();
}

// Retail's POSIX (togl) defaults are "3" for cpu, mem and gpu_mem; each level
// is clamped to the highest PC configuration file, so they read as high.
ConVar cpu_level( "cpu_level", "2", FCVAR_ARCHIVE, "CPU detail level: 0 low, 1 medium, 2 high.",
    OnSystemLevelChanged );
ConVar gpu_level( "gpu_level", "3", FCVAR_ARCHIVE,
    "GPU detail level: 0 low, 1 medium, 2 high, 3 very high.", OnSystemLevelChanged );
ConVar mem_level( "mem_level", "2", FCVAR_ARCHIVE, "Memory level: 0 low, 1 medium, 2 high.",
    OnSystemLevelChanged );
ConVar gpu_mem_level( "gpu_mem_level", "2", FCVAR_ARCHIVE,
    "GPU memory level: 0 low, 1 medium, 2 high.", OnSystemLevelChanged );

namespace
{
// The key retail's videocfg uses for "portal2" configuration files.
const unsigned char s_ModKey[] = "UrE66!Ap";

// videocfg's s_pConVarsAllowedInSystemLevel: whether the lower value wins
// when two level files set the convar, and whether a level may set it.
struct SystemLevelConVar_t
{
	const char *m_pName;
	bool m_bChooseLower;
	bool m_bAllowed;
};

const SystemLevelConVar_t s_SystemLevelConVars[] = {
	{ "lower_body", true, true },
	{ "r_shadow_half_update_rate", false, true },
	{ "r_rainparticledensity", true, true },
	{ "cl_particle_fallback_base", false, true },
	{ "cl_particle_fallback_multiplier", false, true },
	{ "r_flashlightdepthtexture", true, true },
	{ "r_shadowrendertotexture", true, true },
	{ "r_shadowfromworldlights", true, true },
	{ "cl_detaildist", true, true },
	{ "cl_detailfade", true, true },
	{ "r_drawmodeldecals", true, false },
	{ "r_decalstaticprops", true, false },
	{ "ragdoll_sleepaftertime", true, true },
	{ "cl_phys_maxticks", true, true },
	{ "r_worldlightmin", true, true },
	{ "props_break_max_pieces", true, true },
	{ "r_worldlights", true, true },
	{ "r_decals", true, false },
	{ "r_decal_overlap_count", true, false },
	{ "mat_bumpmap", true, true },
	{ "mat_detail_tex", true, true },
	{ "mat_specular", true, true },
	{ "mat_phong", true, true },
	{ "mat_grain_enable", true, true },
	{ "mat_local_contrast_enable", true, true },
	{ "mat_motion_blur_enabled", true, true },
	{ "mat_disablehwmorph", false, true },
	{ "r_overlayfademin", true, true },
	{ "r_overlayfademax", true, true },
	{ "cl_ragdoll_maxcount", true, true },
	{ "cl_ragdoll_maxcount_gib", true, true },
	{ "cl_ragdoll_maxcount_generic", true, true },
	{ "cl_ragdoll_maxcount_special", true, true },
	{ "sv_ragdoll_maxcount", true, true },
	{ "sv_ragdoll_maxcount_gib", true, true },
	{ "sv_ragdoll_maxcount_generic", true, true },
	{ "sv_ragdoll_maxcount_special", true, true },
	{ "cl_impacteffects_limit_general", true, true },
	{ "cl_impacteffects_limit_exit", true, true },
	{ "cl_impacteffects_limit_water", true, true },
	{ "cl_ragdoll_self_collision", true, true },
	{ "cl_player_max_decal_count", true, true },
	{ "cl_footstep_fx", true, true },
	{ "mp_usehwmvcds", true, true },
	{ "mp_usehwmmodels", true, true },
	{ "mat_depthfeather_enable", true, true },
	{ "mat_dxlevel", true, true },
	{ "r_rootlod", false, true },
	{ "mat_picmip", false, true },
	{ "mat_force_vertexfog", false, true },
	{ "r_simpleworldmodel_waterreflections_fullscreen", false, true },
	{ "r_simpleworldmodel_drawforrecursionlevel_fullscreen", true, true },
	{ "r_simpleworldmodel_drawbeyonddistance_fullscreen", true, true },
	{ "r_simpleworldmodel_waterreflections_splitscreen", true, true },
	{ "r_simpleworldmodel_drawforrecursionlevel_splitscreen", true, true },
	{ "r_simpleworldmodel_drawbeyonddistance_splitscreen", true, true },
	{ "r_simpleworldmodel_waterreflections_pip", true, true },
	{ "r_simpleworldmodel_drawforrecursionlevel_pip", true, true },
	{ "r_simpleworldmodel_drawbeyonddistance_pip", true, true },
	{ "r_lod_switch_scale", true, true },
	{ "r_lod", false, true },
	{ "r_paintblob_highres_cube", false, true },
	{ "r_paintblob_force_single_pass", true, true },
	{ "r_paintblob_max_number_of_threads", true, true },
};

// The PC configuration files exist for levels 0..2 (gpu_level 0..3).
const int kMaxLevel[4] = { 2, 3, 2, 2 };

// ReadEncryptedKVFile: ICE (level 0) over whole 8-byte blocks; a trailing
// partial block is left as stored.
KeyValues *ReadEncryptedKeyValues( const char *pRelativePath )
{
	CUtlBuffer buffer;
	if ( !g_pFullFileSystem->ReadFile( pRelativePath, "GAME", buffer ) )
		return NULL;
	const int nSize = buffer.TellPut();
	CUtlVector<char> text;
	text.SetCount( nSize + 1 );
	V_memcpy( text.Base(), buffer.Base(), nSize );
	text[nSize] = 0;

	IceKey ice( 0 );
	ice.set( s_ModKey );
	unsigned char block[8];
	for ( int i = 0; i + 8 <= nSize; i += 8 )
	{
		ice.decrypt( reinterpret_cast<unsigned char *>( text.Base() + i ), block );
		V_memcpy( text.Base() + i, block, sizeof( block ) );
	}

	KeyValues *pKV = new KeyValues( "kv" );
	if ( !pKV->LoadFromBuffer( pRelativePath, text.Base() ) )
	{
		pKV->deleteThis();
		return NULL;
	}
	return pKV;
}

// PerformSystemConfiguration: one level file into the merged result.
void MergeSystemLevel( KeyValues *pResult, const char *pKind, int nLevel )
{
	char szPath[MAX_PATH];
	V_snprintf( szPath, sizeof( szPath ), "cfg/%s_%d_pc.ekv", pKind, nLevel );
	KeyValues *pLevel = ReadEncryptedKeyValues( szPath );
	if ( !pLevel )
	{
		DevWarning( "System level configuration: missing %s\n", szPath );
		return;
	}
	for ( KeyValues *pKey = pLevel->GetFirstSubKey(); pKey; pKey = pKey->GetNextKey() )
	{
		const SystemLevelConVar_t *pRule = NULL;
		for ( const SystemLevelConVar_t &rule : s_SystemLevelConVars )
		{
			if ( !V_stricmp( rule.m_pName, pKey->GetName() ) )
			{
				pRule = &rule;
				break;
			}
		}
		if ( !pRule )
		{
			DevWarning( "System level configuration: %s sets unknown convar %s\n", szPath,
			    pKey->GetName() );
			continue;
		}
		if ( !pRule->m_bAllowed )
			continue;
		if ( pResult->FindKey( pKey->GetName() ) )
		{
			const float flOld = pResult->GetFloat( pKey->GetName() );
			const float flNew = pKey->GetFloat();
			if ( pRule->m_bChooseLower ? flNew >= flOld : flNew <= flOld )
				continue;
		}
		pResult->SetString( pKey->GetName(), pKey->GetString() );
	}
	pLevel->deleteThis();
}

bool SetOnCommandLine( const char *pName )
{
	for ( int i = 1; i < CommandLine()->ParmCount(); ++i )
	{
		const char *pParm = CommandLine()->GetParm( i );
		if ( pParm && pParm[0] == '+' && !V_stricmp( pName, pParm + 1 ) )
			return true;
	}
	return false;
}
} // namespace

// UpdateSystemLevel. Retail refuses FCVAR_ARCHIVE convars because its video
// configuration owns them; here mat_picmip and r_rootlod are archived engine
// convars, so the levels set them too (the level is the setting players change).
static void ConfigureCurrentSystemLevel()
{
	if ( !g_pFullFileSystem || !g_pCVar )
		return;
	const int nLevels[4] = { clamp( cpu_level.GetInt(), 0, kMaxLevel[0] ),
		clamp( gpu_level.GetInt(), 0, kMaxLevel[1] ), clamp( mem_level.GetInt(), 0, kMaxLevel[2] ),
		clamp( gpu_mem_level.GetInt(), 0, kMaxLevel[3] ) };
	static const char *const s_Kinds[4] = { "cpu_level", "gpu_level", "mem_level", "gpu_mem_level" };

	KeyValues *pResult = new KeyValues( "kv" );
	for ( int i = 0; i < 4; ++i )
		MergeSystemLevel( pResult, s_Kinds[i], nLevels[i] );

	for ( KeyValues *pKey = pResult->GetFirstSubKey(); pKey; pKey = pKey->GetNextKey() )
	{
		ConVar *pConVar = g_pCVar->FindVar( pKey->GetName() );
		if ( !pConVar || SetOnCommandLine( pKey->GetName() ) )
			continue;
		if ( V_strcmp( pConVar->GetString(), pKey->GetString() ) )
			pConVar->SetValue( pKey->GetString() );
	}
	pResult->deleteThis();
}

class CPortal2SystemLevel : public CAutoGameSystem
{
public:
	CPortal2SystemLevel() : CAutoGameSystem( "CPortal2SystemLevel" ) {}

	virtual void PostInit() { ConfigureCurrentSystemLevel(); }
	// The saved config runs after client init; apply its levels before the
	// map's textures load.
	virtual void LevelInitPreEntity() { ConfigureCurrentSystemLevel(); }
};

static CPortal2SystemLevel s_Portal2SystemLevel;
