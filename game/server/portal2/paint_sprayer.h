//========= Portal 2 reconstruction ============================================//
//
// Purpose: info_paint_sprayer: sprays paint blobs into a paint stream.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef PAINT_SPRAYER_H
#define PAINT_SPRAYER_H
#ifdef _WIN32
#pragma once
#endif

#include "baseentity.h"
#include "paint_color_manager.h"
#include "paint_stream.h"

class CSoundPatch;

// Reconstruction note: only PAINT_SPRAYER_SOUND_DRIP is named by the imported
// shared code; the other names follow the FGD "AmbientSound" choices.
enum PaintSprayerAmbientSound_t
{
	PAINT_SPRAYER_SOUND_NONE = 0,
	PAINT_SPRAYER_SOUND_DRIP,
	PAINT_SPRAYER_SOUND_MEDIUM_FLOW,
	PAINT_SPRAYER_SOUND_HEAVY_FLOW
};

class CPaintSprayer : public CBaseEntity
{
	DECLARE_CLASS( CPaintSprayer, CBaseEntity );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

public:
	CPaintSprayer();
	virtual ~CPaintSprayer();

	virtual void Precache();
	virtual void Spawn();
	virtual void Activate();
	virtual int UpdateTransmitState();
	virtual void UpdateOnRemove();

	void StartPaintSpray();
	void StopPaintSpray();

	void InputStart( inputdata_t &inputData );
	void InputStop( inputdata_t &inputData );
	void InputChangePaintType( inputdata_t &inputData );

	void PaintSprayerThink();

	// Set the sprayer data
	void SetPaintSprayerData( bool bSilent, float flBlobsPerSecond, float flMinSpeed, float flMaxSpeed, float flBlobSpreadRadius, float flBlobSpreadAngle, float flBlobStreakPercent, float flMinStreakTime, float flMaxStreakTime, float flMinStreakSpeedDampen, float flMaxStreakSpeedDampen );

	void SetPaintPowerType( PaintPowerType paintType );

private:
	// Shared with the client (paint_sprayer_shared.cpp)
	void SprayPaint( float flDeltaTime );

	bool m_bStartActive;
	float m_flLastThinkTime;
	float m_flAccumulatedTime;
	int m_nBlobSpawned;
	int m_nRenderMode;
	int m_nAmbientSound;

	CNetworkVar( bool, m_bActive );
	CNetworkVar( bool, m_bSilent );
	CNetworkVar( bool, m_bDrawOnly );
	CNetworkVar( int, m_nBlobRandomSeed );

	CNetworkVar( int, m_PaintPowerType );
	CNetworkVar( int, m_nMaxBlobCount );
	CNetworkVar( float, m_flBlobsPerSecond );
	CNetworkVar( float, m_flBlobSpreadRadius );
	CNetworkVar( float, m_flBlobSpreadAngle );
	CNetworkVar( float, m_flMinSpeed );
	CNetworkVar( float, m_flMaxSpeed );

	// how many of the sprayed blobs play an impact sound
	CNetworkVar( float, m_flNoisyBlobPercentage );
	CNetworkVar( float, m_flPercentageSinceLastNoisyBlob );

	//Streak variables
	CNetworkVar( float, m_flStreakPercentage );
	CNetworkVar( float, m_flMinStreakTime );
	CNetworkVar( float, m_flMaxStreakTime );
	CNetworkVar( float, m_flMinStreakSpeedDampen );
	CNetworkVar( float, m_flMaxStreakSpeedDampen );

	CNetworkVar( float, m_flStartTime );

	CNetworkHandle( CPaintStream, m_hPaintStream );

	CSoundPatch *m_pFlowSoundLoop;

	string_t m_strLightPositionName;
	EHANDLE m_hLightPositionEntity;
};

#endif // PAINT_SPRAYER_H
