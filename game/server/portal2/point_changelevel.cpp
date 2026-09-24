//========= Portal 2 reconstruction ============================================//
//
// Purpose: Point entity that changes the level, and the landmarks that carry
//			players between the origin and destination maps
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static char st_szOriginMap[32];
static char st_szDestinationMap[32];

//-----------------------------------------------------------------------------
// Purpose: Entry and exit landmarks all share one well-known name so the
//			level transition can match them across maps
//-----------------------------------------------------------------------------
class CInfoLandmark : public CPointEntity
{
public:
	DECLARE_CLASS( CInfoLandmark, CPointEntity );

	virtual void Activate( void )
	{
		BaseClass::Activate();
		SetName( AllocPooledString( "__p2_landmark" ) );
	}
};

LINK_ENTITY_TO_CLASS( info_landmark_entry, CInfoLandmark );
LINK_ENTITY_TO_CLASS( info_landmark_exit, CInfoLandmark );

//-----------------------------------------------------------------------------
// Purpose: Changes to the map named by the input parameter
//-----------------------------------------------------------------------------
class CPointChangelevel : public CPointEntity
{
public:
	DECLARE_CLASS( CPointChangelevel, CPointEntity );
	DECLARE_DATADESC();

private:
	void InputChangeLevel( inputdata_t &inputdata );

	COutputEvent m_OnChangeLevel;
};

BEGIN_DATADESC( CPointChangelevel )

	DEFINE_INPUTFUNC( FIELD_STRING, "ChangeLevel", InputChangeLevel ),

	DEFINE_OUTPUT( m_OnChangeLevel, "OnChangeLevel" ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( point_changelevel, CPointChangelevel );

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPointChangelevel::InputChangeLevel( inputdata_t &inputdata )
{
	m_OnChangeLevel.FireOutput( inputdata.pActivator, this );

	// Remember where we came from and where we are going
	V_strncpy( st_szOriginMap, STRING( gpGlobals->mapname ), sizeof( st_szOriginMap ) );
	V_strncpy( st_szDestinationMap, inputdata.value.String(), sizeof( st_szDestinationMap ) );

	engine->ChangeLevel( st_szDestinationMap, "__p2_landmark" );
}

//-----------------------------------------------------------------------------
// Purpose: Accessors for the transition that was last started
//-----------------------------------------------------------------------------
const char *ChangeLevel_DestinationMapName( void )
{
	return st_szDestinationMap;
}

const char *ChangeLevel_OriginMapName( void )
{
	return st_szOriginMap;
}

const char *ChangeLevel_GetLandmarkName( void )
{
	return "__p2_landmark";
}
