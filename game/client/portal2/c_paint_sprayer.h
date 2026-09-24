//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of the paint sprayer (info_paint_sprayer).
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#ifndef C_PAINT_SPRAYER_H
#define C_PAINT_SPRAYER_H

#ifdef _WIN32
#pragma once
#endif

#include "c_baseentity.h"
#include "c_paint_stream.h"

class C_PaintSprayer : public C_BaseEntity
{
	DECLARE_CLASS( C_PaintSprayer, C_BaseEntity );
	DECLARE_CLIENTCLASS();
public:
	C_PaintSprayer();
	virtual ~C_PaintSprayer();

	virtual void Precache();
	virtual void Spawn();

	virtual void PreDataUpdate( DataUpdateType_t updateType );
	virtual void PostDataUpdate( DataUpdateType_t updateType );
	virtual void ClientThink();

private:
	// Implemented in paint_sprayer_shared.cpp
	void SprayPaint( float flDeltaTime );

	bool m_bOldActive;
	float m_flLastThinkTime;
	float m_flAccumulatedTime;
	int m_nBlobSpawned;

	// Networked
	bool m_bActive;
	bool m_bSilent;

	int m_PaintPowerType;
	float m_flBlobsPerSecond;
	float m_flBlobSpreadRadius;
	float m_flBlobSpreadAngle;
	float m_flMinSpeed;
	float m_flMaxSpeed;

	float m_flStreakPercentage;
	float m_flMinStreakTime;
	float m_flMaxStreakTime;
	float m_flMinStreakSpeedDampen;
	float m_flMaxStreakSpeedDampen;

	float m_flStartTime;

	CNetworkHandle( C_PaintStream, m_hPaintStream );

	// Reconstruction note: the members below are not in the 2010 DWARF. The
	// imported shared CPaintSprayer::SprayPaint (paint_sprayer_shared.cpp) reads
	// them, so they are provided here; the first three are received from the
	// server entity's keyvalues, the seed and noisy-blob accumulator are local.
	int m_nMaxBlobCount;
	bool m_bDrawOnly;
	float m_flNoisyBlobPercentage;
	int m_nBlobRandomSeed;
	float m_flPercentageSinceLastNoisyBlob;
};

typedef C_PaintSprayer CPaintSprayer;

#endif // C_PAINT_SPRAYER_H
