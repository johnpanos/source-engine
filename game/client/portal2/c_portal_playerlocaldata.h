//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client copy of the Portal 2 player data sent only to the owning
//			client.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef C_PORTAL_PLAYERLOCALDATA_H
#define C_PORTAL_PLAYERLOCALDATA_H
#ifdef _WIN32
#pragma once
#endif

#include "basetypes.h"
#include "mathlib/vector.h"
#include "ehandle.h"
#include "util_shared.h"
#include "paint_enum.h"
#include "portal_player_shared.h"

class C_Trigger_TractorBeam;

// Number of HUD locator targets the view finder can show.
// Reconstruction note: DWARF gives the array size (16); the original name of
// this constant is unknown.
#define MAX_PORTAL_LOCATOR_ENTITIES	16

//-----------------------------------------------------------------------------
// Purpose: Player specific data for Portal 2 ( sent only to local player, too )
//-----------------------------------------------------------------------------
class C_PortalPlayerLocalData
{
public:
	DECLARE_PREDICTABLE();
	DECLARE_CLASS_NOBASE( C_PortalPlayerLocalData );
	DECLARE_EMBEDDED_NETWORKVAR();

	C_PortalPlayerLocalData();

	// Air control is suppressed while this is non-zero
	float m_flAirControlSupressionTime;

	// Entities the HUD view finder draws locators for
	int m_nLocatorEntityIndices[MAX_PORTAL_LOCATOR_ENTITIES];
	bool m_bPlacingPhoto;

	// Paint power HUD info
	Vector m_PaintPowerHudInfoPosition[PAINT_POWER_TYPE_COUNT];
	Vector m_PaintPowerHudInfoNormal[PAINT_POWER_TYPE_COUNT];
	bool m_PaintPowerHudInfoValidity[PAINT_POWER_TYPE_COUNT];
	Vector m_StickNormal;
	Vector m_OldStickNormal;
	Vector m_vPreUpdateVelocity;
	Vector m_Up;
	Vector m_vStickRotationAxis;
	Vector m_StandHullMin;
	Vector m_StandHullMax;
	Vector m_DuckHullMin;
	Vector m_DuckHullMax;
	Vector m_CachedStandHullMinAttempt;
	Vector m_CachedStandHullMaxAttempt;
	Vector m_CachedDuckHullMinAttempt;
	Vector m_CachedDuckHullMaxAttempt;
	Vector m_vLocalUp;
	Vector m_vEyeOffset;
	Vector m_vEyeUpOffset;
	QAngle m_qQuaternionPunch;
	PaintPowerType m_PaintedPowerType;
	CountdownTimer m_PaintedPowerTimer;
	float m_flAirInputScale;
	float m_flCurrentStickTime;
	StickCameraState m_nStickCameraState;
	InAirState m_InAirState;
	bool m_bDoneStickInterp;
	bool m_bDoneCorrectPitch;
	bool m_bAttemptHullResize;
	bool m_bJumpedThisFrame;
	bool m_bDuckedInAir;
	bool m_bBounced;
	bool m_bInTractorBeam;
	bool m_bZoomedIn;

	// Reconstruction note: the members below are absent from both 2010 builds.
	// The imported retail player, movement and animation-state code uses them,
	// so they mirror the server's CPortalPlayerLocalData additions.
	bool m_bShowingViewFinder;
	CHandle< C_Trigger_TractorBeam > m_hTractorBeam;
	int m_nTractorBeamCount;
	bool m_bSlowingTime;
	float m_flSlowTimeRemaining;
	float m_flSlowTimeMaximum;
	bool m_bPreventedCrouchJumpThisFrame;
	bool m_bBouncedThisFrame;
	float m_fBouncedTime;

	// Predicted on both sides from the same inputs; never networked.
	CachedPaintPowerChoiceResult m_CachedPaintPowerChoiceResults[PAINT_POWER_TYPE_COUNT];
};

EXTERN_RECV_TABLE( DT_PortalLocal );

#endif // C_PORTAL_PLAYERLOCALDATA_H
