//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of the paint cleanser trigger (erases paint blobs).
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#include "cbase.h"
#include "c_trigger_paint_cleanser.h"
#include "paint_cleanser_manager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_TriggerPaintCleanser, DT_TriggerPaintCleanser, CTriggerPaintCleanser )
RecvPropBool( RECVINFO( m_bDisabled ) ), END_RECV_TABLE()

                                             C_TriggerPaintCleanser::C_TriggerPaintCleanser()
{
}

C_TriggerPaintCleanser::~C_TriggerPaintCleanser()
{
}

void C_TriggerPaintCleanser::Spawn()
{
	BaseClass::Spawn();

	// Let the blobs know about this cleanser
	PaintCleanserManager.AddPaintCleanser( this );
}

void C_TriggerPaintCleanser::UpdateOnRemove()
{
	PaintCleanserManager.RemovePaintCleanser( this );

	BaseClass::UpdateOnRemove();
}
