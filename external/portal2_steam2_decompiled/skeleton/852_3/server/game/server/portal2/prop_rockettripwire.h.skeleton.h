// DWARF declaration skeleton for game/server/portal2/prop_rockettripwire.h
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/prop_rockettripwire.h:19 sizeof=0x578 (i386)
struct CPropRocketTripwire : public CBaseAnimating
{
public:
	// game/server/portal2/prop_rockettripwire.h:20
	enum TripwireState_t
	{
		DISABLED = 0,
		ACTIVE = 1,
		FIRING = 2,
	};  // line 20
	CPropRocketTripwire();  // line 26
	virtual ~CPropRocketTripwire();  // line 27
	virtual void Precache();  // line 29
	virtual void Spawn();  // line 30
	virtual void UpdateOnRemove();  // line 31
	virtual Vector EyePosition();  // line 32
	virtual Vector EyeDirection();  // line 33
	void WaitingThink();  // line 35
	void ActiveThink();  // line 36
	void PrepareToFireRocket();  // line 37
	void PreFiringThink();  // line 38
	void FireRocketThink();  // line 39
	void EnableTripwire();  // line 40
	void DisableTripwire();  // line 41
	void InputForceFire( inputdata_t & );  // line 43
	void InputEnable( inputdata_t & );  // line 44
	void InputDisable( inputdata_t & );  // line 45
protected:
	void FireRocket();  // line 48
	bool IsAlternativeTripwireTarget( CBaseEntity * );  // line 49
	float m_flRocketSpeed; // +0x514  // line 50
	float m_flRocketLifetime; // +0x518  // line 51
	float m_flTimeLastFired; // +0x51c  // line 52
	COutputEvent m_OnTripped; // +0x520  // line 53
	COutputEvent m_OnRocketExplode; // +0x538  // line 54
	CHandle<CInfoPlacementHelper> m_hPlacementHelper; // +0x550  // line 55
	CHandle<CRocket_Turret_Projectile> m_hCurRocket; // +0x554  // line 56
	bool m_bHasRocketOut; // +0x558  // line 57
	CPropRocketTripwire::TripwireState_t m_State; // +0x55c  // line 58
	float m_flRocketTimeOfDeath; // +0x560  // line 59
	bool m_bDisabled; // +0x564  // line 61
	CNetworkVar( int, m_iMuzzleAttachment ); // +0x568  // line 63
	CNetworkVar( int, m_nBeamEndpointTexture ); // +0x56c  // line 64
	CNetworkVar( bool, m_bDrawBeams ); // +0x570  // line 65
	CNetworkVar( CHandle<CBaseEntity>, m_hRocket ); // +0x574  // line 66
};
