//========= Portal 2 reconstruction ============================================//
//
// Purpose: Portal detector volume that fires outputs when entities teleport
//			through a portal placed inside it
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef FUNC_PORTALLED_H
#define FUNC_PORTALLED_H
#ifdef _WIN32
#pragma once
#endif

#include "func_portal_detector.h"

class CProp_Portal;

class CFunc_Portalled : public CFuncPortalDetector
{
public:
	DECLARE_CLASS( CFunc_Portalled, CFuncPortalDetector );
	DECLARE_DATADESC();

	COutputEvent m_OnEntityPrePortalled;
	COutputEvent m_OnEntityPostPortalled;

	virtual void OnPrePortalled( CBaseEntity *pOther, bool bDeparting );
	virtual void OnPostPortalled( CBaseEntity *pOther, bool bDeparting );

protected:

	virtual void PortalPlacedInsideBounds( CProp_Portal *pPortal );

	bool m_bFireOnPlayer;
	bool m_bFireOnDeparture;
	bool m_bFireOnArrival;
};

#endif // FUNC_PORTALLED_H
