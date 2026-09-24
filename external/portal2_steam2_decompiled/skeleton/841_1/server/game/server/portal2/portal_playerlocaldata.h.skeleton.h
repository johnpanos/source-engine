// DWARF declaration skeleton for game/server/portal2/portal_playerlocaldata.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/portal_playerlocaldata.h:20 sizeof=0x1a4 (i386)
struct CPortalPlayerLocalData
{
public:
	int (**_vptr$CPortalPlayerLocalData)(); // +0x0  // line 0
	CPortalPlayerLocalData();  // line 27
	CNetworkVar( float, m_flAirControlSupressionTime ); // +0x4  // line 36
	CPortalPlayerLocalData::NetworkVar_m_nLocatorEntityIndices m_nLocatorEntityIndices; // +0x8  // line 39
	CNetworkVar( bool, m_bPlacingPhoto ); // +0x48  // line 40
	CPortalPlayerLocalData::NetworkVar_m_PaintPowerHudInfoPosition m_PaintPowerHudInfoPosition; // +0x4c  // line 43
	CPortalPlayerLocalData::NetworkVar_m_PaintPowerHudInfoNormal m_PaintPowerHudInfoNormal; // +0x7c  // line 44
	CPortalPlayerLocalData::NetworkVar_m_PaintPowerHudInfoValidity m_PaintPowerHudInfoValidity; // +0xac  // line 45
	CNetworkVector( Vector, m_StickNormal ); // +0xb0  // line 46
	CNetworkVector( Vector, m_OldStickNormal ); // +0xbc  // line 47
	CNetworkVector( Vector, m_vPreUpdateVelocity ); // +0xc8  // line 48
	CNetworkVector( Vector, m_Up ); // +0xd4  // line 49
	CNetworkVector( Vector, m_vStickRotationAxis ); // +0xe0  // line 50
	CNetworkVector( Vector, m_StandHullMin ); // +0xec  // line 51
	CNetworkVector( Vector, m_StandHullMax ); // +0xf8  // line 52
	CNetworkVector( Vector, m_DuckHullMin ); // +0x104  // line 53
	CNetworkVector( Vector, m_DuckHullMax ); // +0x110  // line 54
	CNetworkVector( Vector, m_CachedStandHullMinAttempt ); // +0x11c  // line 55
	CNetworkVector( Vector, m_CachedStandHullMaxAttempt ); // +0x128  // line 56
	CNetworkVector( Vector, m_CachedDuckHullMinAttempt ); // +0x134  // line 57
	CNetworkVector( Vector, m_CachedDuckHullMaxAttempt ); // +0x140  // line 58
	CNetworkVector( Vector, m_vLocalUp ); // +0x14c  // line 59
	CNetworkVector( Vector, m_vEyeOffset ); // +0x158  // line 60
	CNetworkVector( Vector, m_vEyeUpOffset ); // +0x164  // line 61
	CNetworkQAngle( QAngle, m_qQuaternionPunch ); // +0x170  // line 62
	CNetworkVar( PaintPowerType, m_PaintedPowerType ); // +0x17c  // line 63
	CPortalPlayerLocalData::NetworkVar_m_PaintedPowerTimer m_PaintedPowerTimer; // +0x180  // line 64
	static int GetOffset_m_PaintedPowerTimer();  // line 64
	CNetworkVar( float, m_flAirInputScale ); // +0x18c  // line 65
	CNetworkVar( float, m_flCurrentStickTime ); // +0x190  // line 66
	CNetworkVar( StickCameraState, m_nStickCameraState ); // +0x194  // line 67
	CNetworkVar( InAirState, m_InAirState ); // +0x198  // line 68
	CNetworkVar( bool, m_bDoneStickInterp ); // +0x19c  // line 69
	CNetworkVar( bool, m_bDoneCorrectPitch ); // +0x19d  // line 70
	CNetworkVar( bool, m_bAttemptHullResize ); // +0x19e  // line 71
	CNetworkVar( bool, m_bJumpedThisFrame ); // +0x19f  // line 72
	CNetworkVar( bool, m_bDuckedInAir ); // +0x1a0  // line 73
	CNetworkVar( bool, m_bBounced ); // +0x1a1  // line 74
	CNetworkVar( bool, m_bInTractorBeam ); // +0x1a2  // line 75
	CNetworkVar( bool, m_bZoomedIn ); // +0x1a3  // line 76
};

// game/server/portal2/portal_playerlocaldata.h:25 @0x744cb0 _ZN22CPortalPlayerLocalData19NetworkStateChangedEv
void CPortalPlayerLocalData::NetworkStateChanged()
{
}

// game/server/portal2/portal_playerlocaldata.h:25 @0x744cc0 _ZN22CPortalPlayerLocalData19NetworkStateChangedEPv
void CPortalPlayerLocalData::NetworkStateChanged( void *pProp )
{
}

// game/server/portal2/portal_playerlocaldata.h:64 @0x744cd0 _ZN22CPortalPlayerLocalData30NetworkVar_m_PaintedPowerTimer19NetworkStateChangedEv
void CPortalPlayerLocalData::NetworkVar_m_PaintedPowerTimer::NetworkStateChanged()
{
	CPortalPlayerLocalData *pOuter;  // line 64
	// inlined DispatchNetworkStateChanged<CPortalPlayerLocalData>() at line 64
	// inlined CPortalPlayerLocalData::NetworkVar_m_PaintedPowerTimer::GetEmbeddedVarOuterPtr() at line 64
}

// game/server/portal2/portal_playerlocaldata.h:64 @0x744cf0 _ZN22CPortalPlayerLocalData30NetworkVar_m_PaintedPowerTimer19NetworkStateChangedEPv
void CPortalPlayerLocalData::NetworkVar_m_PaintedPowerTimer::NetworkStateChanged( void *pVar )
{
	CPortalPlayerLocalData *pOuter;  // line 64
	// inlined DispatchNetworkStateChanged<CPortalPlayerLocalData>() at line 64
	// inlined CPortalPlayerLocalData::NetworkVar_m_PaintedPowerTimer::GetEmbeddedVarOuterPtr() at line 64
}

// game/server/portal2/portal_playerlocaldata.h:64 (declaration)
void NetworkVar_m_PaintedPowerTimer();
