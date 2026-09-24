//========= Portal 2 reconstruction ============================================//
//
// Purpose: Base projector: owns the first projected segment and re-projects
//			it when portals change.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "baseprojector.h"
#include "baseprojectedentity.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sv_projected_entities_use_placement_helper( "sv_projected_entities_use_placement_helper", "1" );

// Source send-table macros require their declaration layout.
// clang-format off
IMPLEMENT_SERVERCLASS_ST( CBaseProjector, DT_BaseProjector )
	SendPropEHandle( SENDINFO( m_hFirstChild ) ),
	SendPropBool( SENDINFO( m_bEnabled ) ),
END_SEND_TABLE()

BEGIN_DATADESC( CBaseProjector )

	DEFINE_FIELD( m_hFirstChild, FIELD_EHANDLE ),

	DEFINE_KEYFIELD( m_bEnabled, FIELD_BOOLEAN, "StartEnabled" ),
	DEFINE_KEYFIELD( m_bDisablePlacementHelper, FIELD_BOOLEAN, "DisableHelper" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),

	DEFINE_THINKFUNC( TestProjectionChangesThink ),

END_DATADESC()

static CUtlVector<CBaseProjector *> s_AllProjectors;
// clang-format on

CBaseProjector::CBaseProjector()
{
	m_bEnabled = true;
	s_AllProjectors.AddToTail( this );
}

CBaseProjector::~CBaseProjector()
{
	Cleanup();
}

void CBaseProjector::UpdateOnRemove( void )
{
	s_AllProjectors.FindAndFastRemove( this );
	Cleanup();

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: Periodically make sure our projection still matches the world
//-----------------------------------------------------------------------------
void CBaseProjector::TestProjectionChangesThink( void )
{
	SetContextThink( &CBaseProjector::TestProjectionChangesThink, gpGlobals->curtime + 0.05f, "TestProjectionChanges" );

	TestAllForProjectionChanges();
}

void CBaseProjector::Spawn( void )
{
	BaseClass::Spawn();

	SetContextThink( &CBaseProjector::TestProjectionChangesThink, gpGlobals->curtime + 0.05f, "TestProjectionChanges" );
}

void CBaseProjector::Activate( void )
{
	BaseClass::Activate();

	if ( m_bEnabled )
	{
		Project();
	}
}

void CBaseProjector::OnRestore( void )
{
	BaseClass::OnRestore();

	if ( m_bEnabled )
	{
		Project();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Create (or reuse) the first segment and project it from the emitter
//-----------------------------------------------------------------------------
void CBaseProjector::Project( void )
{
	bool bCreateNew = ( m_hFirstChild.Get() == NULL );
	if ( bCreateNew )
	{
		m_hFirstChild = CreateNewProjectedEntity();
		if ( m_hFirstChild.Get() == NULL )
			return;
	}

	// Allow up to 16 segments (15 portal redirections)
	m_hFirstChild.Get()->RecursiveProjection( bCreateNew, this, NULL, GetAbsOrigin(), GetAbsAngles(), 16, m_bDisablePlacementHelper );
}

void CBaseProjector::Cleanup( void )
{
	if ( m_hFirstChild.Get() )
	{
		UTIL_Remove( m_hFirstChild.Get() );
		m_hFirstChild = NULL;
	}
}

int CBaseProjector::DrawDebugTextOverlays( void )
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	return text_offset;
}

//-----------------------------------------------------------------------------
// Purpose: Let every projection chain check its portals
//-----------------------------------------------------------------------------
void CBaseProjector::TestAllForProjectionChanges( void )
{
	for ( int i = 0; i < s_AllProjectors.Count(); ++i )
	{
		CBaseProjectedEntity *pFirstChild = s_AllProjectors[i]->m_hFirstChild.Get();
		if ( pFirstChild )
		{
			pFirstChild->TestForProjectionChanges();
		}
	}
}

void CBaseProjector::InputEnable( inputdata_t &data )
{
	EnableProjection( true );
}

void CBaseProjector::InputDisable( inputdata_t &data )
{
	EnableProjection( false );
}

void CBaseProjector::EnableProjection( bool bEnable )
{
	m_bEnabled = bEnable;

	if ( m_bEnabled )
	{
		Project();
	}
	else
	{
		Cleanup();
	}
}
