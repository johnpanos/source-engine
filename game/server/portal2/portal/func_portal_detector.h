//========= Portal 2 reconstruction ============================================//
//
// Purpose: A volume that fires outputs while portals are placed inside it.
//
// Portal 2 version of the Portal 1 detector. It tracks the (up to two) portals
// touching it and fires start and end outputs as portals are placed, moved,
// linked and fizzled. See func_portal_detector.cpp for provenance.
//
//=============================================================================//

#ifndef FUNC_PORTAL_DETECTOR_H
#define FUNC_PORTAL_DETECTOR_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "portal_shareddefs.h"

class CPortal_Base2D;
class CProp_Portal;

class CFuncPortalDetector : public CBaseEntity
{
public:
	DECLARE_CLASS( CFuncPortalDetector, CBaseEntity );
	DECLARE_DATADESC();

	CFuncPortalDetector();
	~CFuncPortalDetector();

	virtual void Spawn( void );
	virtual void NotifyPortalEvent( PortalEvent_t nEventType, CPortal_Base2D *pNotifier );

	// Fires the start (active) or end (inactive) outputs for every portal
	// currently inside the volume.
	virtual void SetActive( bool bActive );

	// Called by CProp_Portal when a portal moves or becomes linked.
	virtual void UpdateOnPortalMoved( CProp_Portal *pPortal );
	virtual void UpdateOnPortalActivated( CProp_Portal *pPortal );

	virtual bool IsActive( void ) { return m_bActive; }
	virtual int GetLinkageGroupID( void ) { return m_iLinkageGroupID; }
	virtual bool IsPortalTouchingDetector( CProp_Portal *pPortal );

	void InputDisable( inputdata_t &inputdata );
	void InputEnable( inputdata_t &inputdata );
	void InputToggle( inputdata_t &inputdata );

	CFuncPortalDetector *m_pNext;	// link in the detector list (GetPortalDetectorList)

protected:
	virtual void PortalPlacedInsideBounds( CProp_Portal *pPortal );
	virtual void PortalRemovedFromInsideBounds( CProp_Portal *pPortal );

	COutputEvent m_OnStartTouchPortal;
	COutputEvent m_OnStartTouchPortal1;
	COutputEvent m_OnStartTouchPortal2;
	COutputEvent m_OnStartTouchLinkedPortal;
	COutputEvent m_OnStartTouchBothLinkedPortals;
	COutputEvent m_OnEndTouchPortal;
	COutputEvent m_OnEndTouchPortal1;
	COutputEvent m_OnEndTouchPortal2;
	COutputEvent m_OnEndTouchLinkedPortal;
	COutputEvent m_OnEndTouchBothLinkedPortals;

private:
	void GetBoxCenterAndExtents( Vector *pCenter, Vector *pExtents );

	bool	m_bActive;					// are we currently detecting portals
	int		m_iLinkageGroupID;			// which portal pair we test for
	bool	m_bCheckAllIDs;				// detect portals of every linkage group
	EHANDLE	m_phTouchingPortals[2];		// indexed by CPortal_Base2D::IsPortal2()
	int		m_iTouchingPortalCount;
};

// Head of the list of every func_portal_detector (and derived) entity.
CFuncPortalDetector *GetPortalDetectorList();

#endif // FUNC_PORTAL_DETECTOR_H
