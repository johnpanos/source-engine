// DWARF declaration skeleton for game/client/portal2/c_portal_playerlocaldata.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal2/c_portal_playerlocaldata.h:19 sizeof=0x1a4 (i386)
struct C_PortalPlayerLocalData
{
public:
	int (**_vptr$C_PortalPlayerLocalData)(); // +0x0  // line 0
	static typedescription_t m_PredDesc[];  // line 21
	C_PortalPlayerLocalData();  // line 25
	float m_flAirControlSupressionTime; // +0x4  // line 34
	int m_nLocatorEntityIndices[16]; // +0x8  // line 37
	bool m_bPlacingPhoto; // +0x48  // line 38
	Vector m_PaintPowerHudInfoPosition[4]; // +0x4c  // line 42
	Vector m_PaintPowerHudInfoNormal[4]; // +0x7c  // line 43
	bool m_PaintPowerHudInfoValidity[4]; // +0xac  // line 44
	Vector m_StickNormal; // +0xb0  // line 45
	Vector m_OldStickNormal; // +0xbc  // line 46
	Vector m_vPreUpdateVelocity; // +0xc8  // line 47
	Vector m_Up; // +0xd4  // line 48
	Vector m_vStickRotationAxis; // +0xe0  // line 49
	Vector m_StandHullMin; // +0xec  // line 50
	Vector m_StandHullMax; // +0xf8  // line 51
	Vector m_DuckHullMin; // +0x104  // line 52
	Vector m_DuckHullMax; // +0x110  // line 53
	Vector m_CachedStandHullMinAttempt; // +0x11c  // line 54
	Vector m_CachedStandHullMaxAttempt; // +0x128  // line 55
	Vector m_CachedDuckHullMinAttempt; // +0x134  // line 56
	Vector m_CachedDuckHullMaxAttempt; // +0x140  // line 57
	Vector m_vLocalUp; // +0x14c  // line 58
	Vector m_vEyeOffset; // +0x158  // line 59
	Vector m_vEyeUpOffset; // +0x164  // line 60
	QAngle m_qQuaternionPunch; // +0x170  // line 61
	PaintPowerType m_PaintedPowerType; // +0x17c  // line 62
	CountdownTimer m_PaintedPowerTimer; // +0x180  // line 63
	float m_flAirInputScale; // +0x18c  // line 64
	float m_flCurrentStickTime; // +0x190  // line 65
	StickCameraState m_nStickCameraState; // +0x194  // line 66
	InAirState m_InAirState; // +0x198  // line 67
	bool m_bDoneStickInterp; // +0x19c  // line 68
	bool m_bDoneCorrectPitch; // +0x19d  // line 69
	bool m_bAttemptHullResize; // +0x19e  // line 70
	bool m_bJumpedThisFrame; // +0x19f  // line 71
	bool m_bDuckedInAir; // +0x1a0  // line 72
	bool m_bBounced; // +0x1a1  // line 73
	bool m_bInTractorBeam; // +0x1a2  // line 74
};

// game/client/portal2/c_portal_playerlocaldata.h:23 @0x52c870 _ZN23C_PortalPlayerLocalData19NetworkStateChangedEv
void C_PortalPlayerLocalData::NetworkStateChanged()
{
}

// game/client/portal2/c_portal_playerlocaldata.h:23 @0x52c880 _ZN23C_PortalPlayerLocalData19NetworkStateChangedEPv
void C_PortalPlayerLocalData::NetworkStateChanged( void *pProp )
{
}
