//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of the paint bomb, drawn as a ball of paint blobs.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef C_PROP_PAINT_BOMB_H
#define C_PROP_PAINT_BOMB_H
#ifdef _WIN32
#pragma once
#endif

#include "c_physicsprop.h"
#include "paint_blobs_shared.h"
#include "mathlib/vector4d.h"

class C_PropPaintBomb : public C_PhysicsProp
{
public:
	DECLARE_CLASS( C_PropPaintBomb, C_PhysicsProp );
	DECLARE_CLIENTCLASS();

	C_PropPaintBomb();
	virtual ~C_PropPaintBomb();

	virtual void Precache();
	virtual void Spawn();
	virtual void UpdateOnRemove();

	virtual void OnPreDataChanged( DataUpdateType_t updateType );
	virtual void OnDataChanged( DataUpdateType_t updateType );

	virtual void PostDataUpdate( DataUpdateType_t updateType );

	virtual bool ShouldDraw() { return true; }
	virtual int DrawModel( int flags, const RenderableInstance_t &instance );

	virtual void ClientThink();

private:

	void UpdateBlobs( float flDeltaTime );
	void CreatePaintBombPaintBlobs();
	void CleansePaintPower();

	int m_nPaintPowerType;
	int m_nPrevPaintPowerType;

	PaintBlobVector_t m_PaintBlobs;
	Vector4D m_lightingBoxColors[6];

	Vector m_vPrevPosition;
	float m_flAccumulatedTime;
	float m_flLastUpdateTime;
};

#endif // C_PROP_PAINT_BOMB_H
