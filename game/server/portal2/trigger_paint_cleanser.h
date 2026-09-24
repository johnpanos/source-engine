//========= Portal 2 reconstruction ============================================//
//
// Purpose: Trigger volume that removes paint from entities and paint blobs.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#ifndef TRIGGER_PAINT_CLEANSER_H
#define TRIGGER_PAINT_CLEANSER_H
#ifdef _WIN32
#pragma once
#endif

#include "triggers.h"

class CTriggerPaintCleanser : public CBaseTrigger
{
	DECLARE_CLASS( CTriggerPaintCleanser, CBaseTrigger );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

public:
	CTriggerPaintCleanser();

	virtual void Spawn();
	virtual void Activate();
	virtual void Enable();

	bool IsEnabled() const { return !m_bDisabled; }

	virtual void Touch( CBaseEntity *pOther );

	virtual int UpdateTransmitState();

	virtual void UpdateOnRemove();

	// Reconstruction note: declared in both 2010 builds, but neither server binary
	// contains a definition or a caller.
	static bool PaintCleanserVisibilityCallback( CBaseEntity *pEntity, CBasePlayer *pPlayer );
	static bool PaintCleanserVisibilityFilter( CBaseEntity *pEntity, CBasePlayer *pPlayer );

private:

	// m_bDisabled is networked through CBaseTrigger's CNetworkVarForDerived.
	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_bDisabled );
};

#endif // TRIGGER_PAINT_CLEANSER_H
