//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of the paint cleanser trigger (erases paint blobs).
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#ifndef C_TRIGGER_PAINT_CLEANSER_H
#define C_TRIGGER_PAINT_CLEANSER_H

#ifdef _WIN32
#pragma once
#endif

#include "c_triggers.h"

class C_TriggerPaintCleanser : public C_BaseTrigger
{
public:
	DECLARE_CLASS( C_TriggerPaintCleanser, C_BaseTrigger );
	DECLARE_CLIENTCLASS();

	C_TriggerPaintCleanser();
	virtual ~C_TriggerPaintCleanser();

	virtual void Spawn();

	virtual void UpdateOnRemove();

	bool IsEnabled() const { return !m_bDisabled; }

private:
	bool m_bDisabled;
};

typedef C_TriggerPaintCleanser CTriggerPaintCleanser;

#endif // C_TRIGGER_PAINT_CLEANSER_H
