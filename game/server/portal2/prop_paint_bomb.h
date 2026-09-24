//========= Portal 2 reconstruction ============================================//
//
// Purpose: Physics prop that splashes paint around itself when it breaks.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef PROP_PAINT_BOMB_H
#define PROP_PAINT_BOMB_H
#ifdef _WIN32
#pragma once
#endif

#include "props.h"
#include "paint_color_manager.h"

class CPropPaintBomb : public CPhysicsProp
{
public:
	DECLARE_CLASS( CPropPaintBomb, CPhysicsProp );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CPropPaintBomb();
	virtual ~CPropPaintBomb();

	virtual void Precache();
	virtual void Spawn();

	virtual void Event_Killed( const CTakeDamageInfo &info );
	virtual void OnFizzled();

	virtual void Break( CBaseEntity *pBreaker, const CTakeDamageInfo &info );

	void SetPaintPowerType( PaintPowerType paintType );
	void CleansePaintPower();

	virtual int UpdateTransmitState();

private:
	void InputDissolve( inputdata_t &in );
	void InputDisablePortalFunnel( inputdata_t &in );
	void InputEnablePortalFunnel( inputdata_t &in );

	COutputEvent m_OnFizzled;
	COutputEvent m_OnExploded;

	CNetworkVar( int, m_nPaintPowerType );
};

#endif // PROP_PAINT_BOMB_H
