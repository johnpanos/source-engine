// DWARF declaration skeleton for game/server/logic_playerproxy.h
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/logic_playerproxy.h:15 sizeof=0x4e4 (i386)
struct CLogicPlayerProxy : public CLogicalEntity
{
public:
	COutputEvent m_OnStartSlowingTime; // +0x3c0  // line 22
	COutputEvent m_OnStopSlowingTime; // +0x3d8  // line 23
	COutputEvent m_OnCoopPing; // +0x3f0  // line 24
	COutputEvent m_OnPrimaryPortalPlaced; // +0x408  // line 26
	COutputEvent m_OnSecondaryPortalPlaced; // +0x420  // line 27
	COutputEvent m_PlayerHasAmmo; // +0x438  // line 36
	COutputEvent m_PlayerHasNoAmmo; // +0x450  // line 37
	COutputEvent m_PlayerDied; // +0x468  // line 38
	COutputEvent m_OnDuck; // +0x480  // line 40
	COutputEvent m_OnUnDuck; // +0x498  // line 41
	COutputEvent m_OnJump; // +0x4b0  // line 42
	COutputInt m_RequestedPlayerHealth; // +0x4c8  // line 44
	void InputRequestPlayerHealth( inputdata_t & );  // line 53
	void InputSetPlayerHealth( inputdata_t & );  // line 54
	void InputRequestAmmoState( inputdata_t & );  // line 55
	void InputEnableCappedPhysicsDamage( inputdata_t & );  // line 56
	void InputDisableCappedPhysicsDamage( inputdata_t & );  // line 57
	void InputAddPotatosToPortalgun( inputdata_t & );  // line 60
	void InputRemovePotatosFromPortalgun( inputdata_t & );  // line 61
	virtual void Activate();  // line 64
	virtual bool PassesDamageFilter( const CTakeDamageInfo & );  // line 66
	EHANDLE m_hPlayer; // +0x4e0  // line 68
};
