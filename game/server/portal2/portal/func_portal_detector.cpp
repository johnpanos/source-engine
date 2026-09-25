//========= Portal 2 reconstruction ============================================//
//
// Purpose: A volume that fires outputs while portals are placed inside it.
//
// Portal 2 version of game/server/portal/func_portal_detector.cpp. Written
// from the retail Linux server.so: the CFuncPortalDetector datadesc (18
// entries), its constructor/destructor list handling, Spawn, SetActive and the
// input, portal-moved, portal-activated, touch-test, placed and removed
// virtuals. The 2010 Steam2 dSYM names GetPortalDetectorList and the
// CEntityClassList. prop_portal.cpp follows the Portal 2 caller shape kept in
// the CS:GO source drop. Not original Valve source; the repository's
// provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "func_portal_detector.h"
#include "prop_portal.h"
#include "prop_portal_shared.h"
#include "portal_util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Spawnflags
#define SF_START_INACTIVE			0x01

CEntityClassList<CFuncPortalDetector> g_FuncPortalDetectorList;
template <> CFuncPortalDetector *CEntityClassList<CFuncPortalDetector>::m_pClassList = NULL;

CFuncPortalDetector *GetPortalDetectorList()
{
	return g_FuncPortalDetectorList.m_pClassList;
}

LINK_ENTITY_TO_CLASS( func_portal_detector, CFuncPortalDetector );

BEGIN_DATADESC( CFuncPortalDetector )

	DEFINE_FIELD( m_bActive, FIELD_BOOLEAN ),
	DEFINE_AUTO_ARRAY( m_phTouchingPortals, FIELD_EHANDLE ),
	DEFINE_FIELD( m_iTouchingPortalCount, FIELD_INTEGER ),
	DEFINE_KEYFIELD( m_iLinkageGroupID, FIELD_INTEGER, "LinkageGroupID" ),
	DEFINE_KEYFIELD( m_bCheckAllIDs, FIELD_BOOLEAN, "CheckAllIDs" ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),

	// Outputs
	DEFINE_OUTPUT( m_OnStartTouchPortal, "OnStartTouchPortal" ),
	DEFINE_OUTPUT( m_OnStartTouchPortal1, "OnStartTouchPortal1" ),
	DEFINE_OUTPUT( m_OnStartTouchPortal2, "OnStartTouchPortal2" ),
	DEFINE_OUTPUT( m_OnStartTouchLinkedPortal, "OnStartTouchLinkedPortal" ),
	DEFINE_OUTPUT( m_OnStartTouchBothLinkedPortals, "OnStartTouchBothLinkedPortals" ),
	DEFINE_OUTPUT( m_OnEndTouchPortal, "OnEndTouchPortal" ),
	DEFINE_OUTPUT( m_OnEndTouchPortal1, "OnEndTouchPortal1" ),
	DEFINE_OUTPUT( m_OnEndTouchPortal2, "OnEndTouchPortal2" ),
	DEFINE_OUTPUT( m_OnEndTouchLinkedPortal, "OnEndTouchLinkedPortal" ),
	DEFINE_OUTPUT( m_OnEndTouchBothLinkedPortals, "OnEndTouchBothLinkedPortals" ),

END_DATADESC()

CFuncPortalDetector::CFuncPortalDetector()
	: m_pNext( NULL ), m_bActive( false ), m_iLinkageGroupID( 0 ), m_bCheckAllIDs( false ),
	  m_iTouchingPortalCount( 0 )
{
	g_FuncPortalDetectorList.Insert( this );
}

CFuncPortalDetector::~CFuncPortalDetector()
{
	g_FuncPortalDetectorList.Remove( this );
}

void CFuncPortalDetector::Spawn()
{
	BaseClass::Spawn();

	m_bActive = !( m_spawnflags & SF_START_INACTIVE );

	// Bind to our model, cause we need the extents for bounds checking
	SetModel( STRING( GetModelName() ) );
	SetRenderMode( kRenderNone );	// Don't draw
	SetSolid( SOLID_VPHYSICS );		// we may want slanted walls, so we'll use OBB
	AddSolidFlags( FSOLID_NOT_SOLID );
}

void CFuncPortalDetector::GetBoxCenterAndExtents( Vector *pCenter, Vector *pExtents )
{
	Vector vMin, vMax;
	CollisionProp()->WorldSpaceAABB( &vMin, &vMax );
	*pCenter = ( vMin + vMax ) * 0.5f;
	*pExtents = ( vMax - vMin ) * 0.5f;
}

void CFuncPortalDetector::SetActive( bool bActive )
{
	m_bActive = bActive;

	Vector vBoxCenter, vBoxExtents;
	GetBoxCenterAndExtents( &vBoxCenter, &vBoxExtents );

	COutputEvent &TouchingPortal = bActive ? m_OnStartTouchPortal : m_OnEndTouchPortal;
	COutputEvent &TouchingPortal1 = bActive ? m_OnStartTouchPortal1 : m_OnEndTouchPortal1;
	COutputEvent &TouchingPortal2 = bActive ? m_OnStartTouchPortal2 : m_OnEndTouchPortal2;
	COutputEvent &TouchingLinked = bActive ? m_OnStartTouchLinkedPortal : m_OnEndTouchLinkedPortal;
	COutputEvent &TouchingBothLinked = bActive ? m_OnStartTouchBothLinkedPortals : m_OnEndTouchBothLinkedPortals;

	bool bTouchedPortal1 = false;
	bool bTouchedPortal2 = false;

	int iPortalCount = CProp_Portal_Shared::AllPortals.Count();
	CProp_Portal **pPortals = CProp_Portal_Shared::AllPortals.Base();
	for ( int i = 0; i != iPortalCount; ++i )
	{
		CProp_Portal *pTempPortal = pPortals[i];
		if ( pTempPortal->GetLinkageGroup() != m_iLinkageGroupID && !m_bCheckAllIDs )
			continue;

		if ( !UTIL_IsBoxIntersectingPortal( vBoxCenter, vBoxExtents, pTempPortal ) )
			continue;

		TouchingPortal.FireOutput( pTempPortal, this );
		if ( pTempPortal->IsPortal2() )
		{
			TouchingPortal2.FireOutput( pTempPortal, this );
			if ( pTempPortal->IsActivedAndLinked() )
			{
				bTouchedPortal2 = true;
				TouchingLinked.FireOutput( pTempPortal, this );
			}
		}
		else
		{
			TouchingPortal1.FireOutput( pTempPortal, this );
			if ( pTempPortal->IsActivedAndLinked() )
			{
				bTouchedPortal1 = true;
				TouchingLinked.FireOutput( pTempPortal, this );
			}
		}
	}

	if ( bTouchedPortal1 && bTouchedPortal2 )
		TouchingBothLinked.FireOutput( this, this );
}

bool CFuncPortalDetector::IsPortalTouchingDetector( CProp_Portal *pPortal )
{
	if ( !pPortal )
		return false;

	return m_phTouchingPortals[0].Get() == pPortal || m_phTouchingPortals[1].Get() == pPortal;
}

void CFuncPortalDetector::UpdateOnPortalMoved( CProp_Portal *pPortal )
{
	if ( !m_bActive )
		return;

	bool bWasTouching = IsPortalTouchingDetector( pPortal );
	bool bIsTouching = false;

	if ( GetLinkageGroupID() == pPortal->GetLinkageGroup() || m_bCheckAllIDs )
	{
		Vector vBoxCenter, vBoxExtents;
		GetBoxCenterAndExtents( &vBoxCenter, &vBoxExtents );
		bIsTouching = UTIL_IsBoxIntersectingPortal( vBoxCenter, vBoxExtents, pPortal );
	}

	if ( !bWasTouching )
	{
		if ( !bIsTouching )
			return;

		m_phTouchingPortals[pPortal->IsPortal2() ? 1 : 0] = pPortal;
		++m_iTouchingPortalCount;
		PortalPlacedInsideBounds( pPortal );
		return;
	}

	// Still inside the volume and still open: nothing changed.
	if ( bIsTouching && pPortal->IsActive() )
		return;

	m_phTouchingPortals[pPortal->IsPortal2() ? 1 : 0] = NULL;
	--m_iTouchingPortalCount;
	PortalRemovedFromInsideBounds( pPortal );
}

void CFuncPortalDetector::UpdateOnPortalActivated( CProp_Portal *pPortal )
{
	if ( !IsPortalTouchingDetector( pPortal ) )
		return;

	m_OnStartTouchLinkedPortal.FireOutput( pPortal, this );
	if ( m_iTouchingPortalCount == 2 )
		m_OnStartTouchBothLinkedPortals.FireOutput( pPortal, this );
}

void CFuncPortalDetector::PortalPlacedInsideBounds( CProp_Portal *pPortal )
{
	m_OnStartTouchPortal.FireOutput( pPortal, this );
	if ( pPortal->IsPortal2() )
		m_OnStartTouchPortal2.FireOutput( pPortal, this );
	else
		m_OnStartTouchPortal1.FireOutput( pPortal, this );

	if ( pPortal->IsActivedAndLinked() )
	{
		m_OnStartTouchLinkedPortal.FireOutput( pPortal, this );
		if ( m_iTouchingPortalCount == 2 )
			m_OnStartTouchBothLinkedPortals.FireOutput( pPortal, this );
	}

	// Hear about the portal fizzling so the end outputs fire.
	pPortal->AddPortalEventListener( this );
}

void CFuncPortalDetector::PortalRemovedFromInsideBounds( CProp_Portal *pPortal )
{
	m_OnEndTouchPortal.FireOutput( pPortal, this );
	if ( pPortal->IsPortal2() )
		m_OnEndTouchPortal2.FireOutput( pPortal, this );
	else
		m_OnEndTouchPortal1.FireOutput( pPortal, this );

	if ( pPortal->IsActivedAndLinked() )
	{
		m_OnEndTouchLinkedPortal.FireOutput( pPortal, this );
		if ( m_iTouchingPortalCount == 0 )
			m_OnEndTouchBothLinkedPortals.FireOutput( pPortal, this );
	}

	pPortal->RemovePortalEventListener( this );
}

void CFuncPortalDetector::NotifyPortalEvent( PortalEvent_t nEventType, CPortal_Base2D *pNotifier )
{
	if ( nEventType != PORTALEVENT_FIZZLE )
		return;

	CProp_Portal *pPortal = dynamic_cast<CProp_Portal *>( pNotifier );
	if ( pPortal )
		UpdateOnPortalMoved( pPortal );
}

void CFuncPortalDetector::InputDisable( inputdata_t &inputdata )
{
	SetActive( false );
}

void CFuncPortalDetector::InputEnable( inputdata_t &inputdata )
{
	SetActive( true );
}

void CFuncPortalDetector::InputToggle( inputdata_t &inputdata )
{
	SetActive( !m_bActive );
}
