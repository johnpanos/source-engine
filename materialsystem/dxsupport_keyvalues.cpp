//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: dxsupport.cfg as KeyValues for the shader APIs (dxsupport_keyvalues.h).
//          Moved from CShaderDeviceMgrBase; the group selection and order
//          belong to render.dxsupport-policy.v1.
//
//===========================================================================//

#include "dxsupport_keyvalues.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#else
#include <strings.h>
#endif

#include "filesystem.h"
#include "tier0/dbg.h"
#include "tier0/icommandline.h"
#include "tier0/platform.h"
#include "tier1/KeyValues.h"
#include "tier1/strtools.h"
#include "tier1/utlbuffer.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace dxsupport
{
namespace
{
int ReadHexValue( KeyValues *pVal, const char *pName )
{
	const char *pString = pVal->GetString( pName, NULL );
	if ( !pString )
		return -1;

	char *pTemp;
	int nVal = strtol( pString, &pTemp, 16 );
	return ( pTemp != pString ) ? nVal : -1;
}

// Copies one value; replaces an existing value of the same name.
void AddKey( KeyValues *pDest, KeyValues *pSrc )
{
	switch ( pSrc->GetDataType() )
	{
	case KeyValues::TYPE_NONE:
		break;
	case KeyValues::TYPE_STRING:
		pDest->SetString( pSrc->GetName(), pSrc->GetString() );
		break;
	case KeyValues::TYPE_INT:
		pDest->SetInt( pSrc->GetName(), pSrc->GetInt() );
		break;
	case KeyValues::TYPE_FLOAT:
		pDest->SetFloat( pSrc->GetName(), pSrc->GetFloat() );
		break;
	case KeyValues::TYPE_PTR:
		pDest->SetPtr( pSrc->GetName(), pSrc->GetPtr() );
		break;
	case KeyValues::TYPE_WSTRING:
		pDest->SetWString( pSrc->GetName(), pSrc->GetWString() );
		break;
	case KeyValues::TYPE_COLOR:
		pDest->SetColor( pSrc->GetName(), pSrc->GetColor() );
		break;
	default:
		Assert( 0 );
		break;
	}
}

void OverrideValues_R( KeyValues *pDest, KeyValues *pSrc )
{
	// Any same-named values get overridden in pDest.
	for ( KeyValues *pSrcValue = pSrc->GetFirstValue(); pSrcValue;
	      pSrcValue = pSrcValue->GetNextValue() )
	{
		// Shouldn't be a container for more keys.
		Assert( pSrcValue->GetDataType() != KeyValues::TYPE_NONE );
		AddKey( pDest, pSrcValue );
	}

	// Recurse.
	for ( KeyValues *pSrcDir = pSrc->GetFirstTrueSubKey(); pSrcDir;
	      pSrcDir = pSrcDir->GetNextTrueSubKey() )
	{
		Assert( pSrcDir->GetDataType() == KeyValues::TYPE_NONE );

		KeyValues *pDestDir = pDest->FindKey( pSrcDir->GetName() );
		if ( pDestDir && pDestDir->GetDataType() == KeyValues::TYPE_NONE )
			OverrideValues_R( pDestDir, pSrcDir );
	}
}

KeyValues *FindMatchingGroup( KeyValues *pSrc, KeyValues *pMatch )
{
	KeyValues *pMatchSubKey = pMatch->FindKey( "name" );
	bool bHasSubKey = ( pMatchSubKey && ( pMatchSubKey->GetDataType() != KeyValues::TYPE_NONE ) );
	const char *name = bHasSubKey ? pMatchSubKey->GetString() : NULL;
	int nMatchVendorID = ReadHexValue( pMatch, "VendorID" );
	int nMatchMinDeviceID = ReadHexValue( pMatch, "MinDeviceID" );
	int nMatchMaxDeviceID = ReadHexValue( pMatch, "MaxDeviceID" );

	KeyValues *pSrcGroup = NULL;
	for ( pSrcGroup = pSrc->GetFirstTrueSubKey(); pSrcGroup;
	      pSrcGroup = pSrcGroup->GetNextTrueSubKey() )
	{
		if ( name )
		{
			KeyValues *pSrcGroupName = pSrcGroup->FindKey( "name" );
			Assert( pSrcGroupName );
			Assert( pSrcGroupName->GetDataType() != KeyValues::TYPE_NONE );
			if ( Q_stricmp( pSrcGroupName->GetString(), name ) )
				continue;
		}

		if ( nMatchVendorID >= 0 )
		{
			int nVendorID = ReadHexValue( pSrcGroup, "VendorID" );
			if ( nMatchVendorID != nVendorID )
				continue;
		}

		if ( nMatchMinDeviceID >= 0 && nMatchMaxDeviceID >= 0 )
		{
			int nMinDeviceID = ReadHexValue( pSrcGroup, "MinDeviceID" );
			int nMaxDeviceID = ReadHexValue( pSrcGroup, "MaxDeviceID" );
			if ( nMinDeviceID < 0 || nMaxDeviceID < 0 )
				continue;

			if ( nMatchMinDeviceID > nMinDeviceID || nMatchMaxDeviceID < nMaxDeviceID )
				continue;
		}

		return pSrcGroup;
	}
	return NULL;
}

void OverrideKeyValues( KeyValues *pDst, KeyValues *pSrc )
{
	for ( KeyValues *pSrcGroup = pSrc->GetFirstTrueSubKey(); pSrcGroup;
	      pSrcGroup = pSrcGroup->GetNextTrueSubKey() )
	{
		// Match each group in pSrc to one in pDst containing the same "name" value:
		KeyValues *pDstGroup = FindMatchingGroup( pDst, pSrcGroup );
		if ( pDstGroup )
			OverrideValues_R( pDstGroup, pSrcGroup );
	}
}

void SpewKeyValues( const char *pLabel, KeyValues *pKeyValues )
{
	if ( !CommandLine()->FindParm( "-debugdxsupport" ) )
		return;
	CUtlBuffer tmpBuf;
	pKeyValues->RecursiveSaveToFile( tmpBuf, 0 );
	Warning( "%s%s\n", pLabel, (const char *)tmpBuf.Base() );
}

// Reads in ConVars + config variables
void LoadConfig( KeyValues *pKeyValues, KeyValues *pConfiguration )
{
	if ( !pKeyValues )
		return;
	SpewKeyValues( "", pKeyValues );
	for ( KeyValues *pGroup = pKeyValues->GetFirstSubKey(); pGroup; pGroup = pGroup->GetNextKey() )
		AddKey( pConfiguration, pGroup );
}

// System RAM in MB as MEMORYSTATUS reports it.
int SystemRamMegabytes()
{
#ifdef _WIN32
	MEMORYSTATUS stat;
	GlobalMemoryStatus( &stat );
	const unsigned long long totalBytes = stat.dwTotalPhys;
#else
	// Installed RAM (/proc/meminfo MemTotal), at least 128 MB, through the
	// 32-bit dwTotalPhys the D3D9 device's GlobalMemoryStatus emulation filled.
	unsigned long totalMB = 2047;
	if ( FILE *pMemInfo = fopen( "/proc/meminfo", "r" ) )
	{
		char line[256];
		const char *pKey = "MemTotal:";
		const size_t keyLength = strlen( pKey );
		while ( fgets( line, sizeof( line ), pMemInfo ) )
		{
			if ( !strncasecmp( pKey, line, keyLength ) )
			{
				totalMB = strtoul( line + keyLength, NULL, 10 ) / 1024;
				break;
			}
		}
		fclose( pMemInfo );
	}
	totalMB = std::max<unsigned long>( totalMB, 128 );
	const unsigned long long totalBytes =
	    std::min<unsigned long long>( 1024ull * 1024ull * totalMB, 0xFFFFFFFFull );
#endif
	char buf[256];
	V_snprintf( buf, sizeof( buf ), "GlobalMemoryStatus: %llu\n", totalBytes );
	Plat_DebugString( buf );
	return static_cast<int>( totalBytes / ( 1024 * 1024 ) );
}
} // namespace

KeyValues *ReadConfig( IFileSystem *pFileSystem, const char *pConfigFile, const char *pOverrideFile )
{
	if ( CommandLine()->CheckParm( "-ignoredxsupportcfg" ) || !pFileSystem )
		return NULL;

	KeyValues *pCfg = new KeyValues( "dxsupport" );
	if ( !pCfg->LoadFromFile( pFileSystem, pConfigFile, "EXECUTABLE_PATH" ) )
	{
		pCfg->deleteThis();
		return NULL;
	}

	char pTempPath[1024];
	if ( pOverrideFile && pFileSystem->GetSearchPath( "GAME", false, pTempPath, sizeof( pTempPath ) ) > 1 )
	{
		// Is there a mod-specific override file?
		KeyValues *pOverride = new KeyValues( "dxsupport_override" );
		if ( pOverride->LoadFromFile( pFileSystem, pOverrideFile, "GAME" ) )
			OverrideKeyValues( pCfg, pOverride );
		pOverride->deleteThis();
	}
	return pCfg;
}

void DescribeGroups( KeyValues *pConfig, std::vector<render::DxSupportGroupFacts> *pFacts,
    std::vector<KeyValues *> *pGroups )
{
	pFacts->clear();
	pGroups->clear();
	for ( KeyValues *pGroup = pConfig->GetFirstSubKey(); pGroup; pGroup = pGroup->GetNextKey() )
	{
		render::DxSupportGroupFacts facts;
		const char *pName = pGroup->GetString( "name", NULL );
		facts.hasName = pName != NULL;
		facts.name = pName ? pName : "";
		facts.nameAsInt = pGroup->GetInt( "name", 0 );
		facts.vendorId = ReadHexValue( pGroup, "VendorID" );
		facts.minDeviceId = ReadHexValue( pGroup, "MinDeviceID" );
		facts.maxDeviceId = ReadHexValue( pGroup, "MaxDeviceID" );
		facts.minMegahertz = pGroup->GetInt( "min megahertz", -1 );
		facts.maxMegahertz = pGroup->GetInt( "max megahertz", -1 );
		facts.minMegabytes = pGroup->GetInt( "min megabytes", -1 );
		facts.maxMegabytes = pGroup->GetInt( "max megabytes", -1 );
		facts.minMegatexels = pGroup->GetInt( "min megatexels", -1 );
		facts.maxMegatexels = pGroup->GetInt( "max megatexels", -1 );
		if ( KeyValues *pPicmip = pGroup->FindKey( "ConVar.mat_picmip", false ) )
			facts.picmip = pPicmip->GetInt();
		pFacts->push_back( facts );
		pGroups->push_back( pGroup );
	}
}

KeyValues *FindCardGroup( KeyValues *pConfig, int nVendorID, int nDeviceID )
{
	std::vector<render::DxSupportGroupFacts> facts;
	std::vector<KeyValues *> groups;
	DescribeGroups( pConfig, &facts, &groups );
	const std::optional<size_t> card = render::FindCardGroup( facts, nVendorID, nDeviceID );
	return card ? groups[*card] : NULL;
}

std::vector<KeyValues *> DeviceGroups( KeyValues *pConfig, int nDxLevel, int nVendorID, int nDeviceID )
{
	std::vector<render::DxSupportGroupFacts> facts;
	std::vector<KeyValues *> groups;
	DescribeGroups( pConfig, &facts, &groups );
	std::vector<KeyValues *> ordered;
	for ( size_t index : render::ResolveDeviceGroupOrder( facts, nDxLevel, nVendorID, nDeviceID ) )
		ordered.push_back( groups[index] );
	return ordered;
}

void FillHostFacts( render::DxSupportQuery *pQuery )
{
	const CPUInformation &pi = *GetCPUInformation();
	pQuery->cpuMegahertz = (int)( pi.m_Speed / 1000000.0f );
	pQuery->cpuIsAmd = Q_stristr( pi.m_szProcessorID, "amd" ) != NULL;

	char buf[256];
	V_snprintf( buf, sizeof( buf ),
	    "GetRecommendedConfigurationInfo: CPU speed: %d MHz, Processor: %s\n", pQuery->cpuMegahertz,
	    pi.m_szProcessorID );
	Plat_DebugString( buf );

	pQuery->systemRamMegabytes = SystemRamMegabytes();
	DevMsg( "%d MB of system RAM\n", pQuery->systemRamMegabytes );
}

void ApplyRecommendedConfig(
    KeyValues *pConfig, const render::DxSupportQuery &query, KeyValues *pConfiguration )
{
	std::vector<render::DxSupportGroupFacts> facts;
	std::vector<KeyValues *> groups;
	DescribeGroups( pConfig, &facts, &groups );
	const render::DxSupportPlan plan = render::ResolveRecommendedConfig( facts, query );
	for ( size_t index : plan.applyOrder )
		LoadConfig( groups[index], pConfiguration );

	if ( plan.picmipFloor )
	{
		KeyValues *pConfigMatPicMip = pConfiguration->FindKey( "ConVar.mat_picmip", false );
		const int oldPicMip = pConfigMatPicMip ? pConfigMatPicMip->GetInt() : 0;
		pConfiguration->SetInt( "ConVar.mat_picmip", std::max( *plan.picmipFloor, oldPicMip ) );
	}

	// Slam the mat_dxlevel ConVar to match the requested dxlevel
	pConfiguration->SetInt( "ConVar.mat_dxlevel", plan.dxLevel );
	SpewKeyValues( "final config:\n", pConfiguration );
}
} // namespace dxsupport
