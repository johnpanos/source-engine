//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client-side fizzler trigger; predicts portal removal for the local gun
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#ifndef C_TRIGGER_PORTAL_CLEANSER_H
#define C_TRIGGER_PORTAL_CLEANSER_H
#ifdef _WIN32
#pragma once
#endif

#include "c_triggers.h"

// Reconstruction note: the 2010 builds declare this class inside
// c_trigger_portal_cleanser.cpp. The retail-era shared placement and portalgun code
// includes this header and needs the class, so the declaration lives here.
class C_TriggerPortalCleanser : public C_BaseTrigger
{
public:
	DECLARE_CLASS( C_TriggerPortalCleanser, C_BaseTrigger );
	DECLARE_CLIENTCLASS();

	bool IsEnabled( void ) const { return !m_bDisabled; }

private:
	bool m_bDisabled;

public:
	virtual void Touch( C_BaseEntity *pOther );
	virtual void UpdatePartitionListEntry( void );
};

typedef C_TriggerPortalCleanser CTriggerPortalCleanser;

#endif // C_TRIGGER_PORTAL_CLEANSER_H
