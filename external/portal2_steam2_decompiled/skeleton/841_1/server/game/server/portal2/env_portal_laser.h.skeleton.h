// DWARF declaration skeleton for game/server/portal2/env_portal_laser.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/env_portal_laser.h:18 sizeof=0x7e4 (i386)
struct CPortalLaser : public CBaseAnimating
{
public:
	CPortalLaser();  // line 23
	virtual ~CPortalLaser();  // line 24
	virtual void Spawn();  // line 25
	virtual void Precache();  // line 26
	virtual void Activate();  // line 27
	virtual void UpdateOnRemove();  // line 28
	virtual bool CreateVPhysics();  // line 29
	virtual int UpdateTransmitState();  // line 30
	void InputTurnOn( inputdata_t & );  // line 32
	void InputTurnOff( inputdata_t & );  // line 33
	void InputToggle( inputdata_t & );  // line 34
	CPortalLaser *m_pChildLaser; // +0x514  // line 36
private:
	void RemoveChildLaser();  // line 41
	void UpdateNextLaser( const Vector &, const Vector &, CBaseEntity * );  // line 42
	void FireLaser( const Vector &, const Vector &, CBaseEntity * );  // line 43
	bool ReflectLaserFromEntity( CBaseEntity * );  // line 44
	bool InPVS();  // line 46
	void CreateHelperEntities();  // line 47
	void CreateSoundProxies();  // line 48
	void DamageEntity( CBaseEntity *, float );  // line 49
	void TurnOn();  // line 50
	void TurnOff();  // line 51
	bool IsOn();  // line 52
	void TurnOnGlow();  // line 54
	void TurnOffGlow();  // line 55
	void TurnOffLaserSound();  // line 57
	void FireAtPoint( trace_t &, bool );  // line 59
	void StrikeThink();  // line 60
	void BeamDamage( trace_t * );  // line 61
	void HideBeam();  // line 63
	void ShowBeam();  // line 64
	void UpdateSoundPosition( const Vector &, const Vector & );  // line 65
	bool StrikeEntitiesAlongLaser( const Vector &, const Vector &, Vector * );  // line 66
	bool ShouldAutoAim( CBaseEntity * );  // line 68
	void SetFromReflectedCube( bool );  // line 70
public:
	// game/server/portal2/env_portal_laser.h:73 sizeof=0x2c (i386)
	struct PortalLaserInfo_t
	{
	public:
		Vector vecStart; // +0x0  // line 74
		Vector vecEnd; // +0xc  // line 75
		CUtlVector<CBaseEntity*,CUtlMemory<CBaseEntity*, int> > sortedEntList; // +0x18  // line 76
	};  // line 73
private:
	CBaseEntity *TraceLaser( bool, const Vector &, const Vector &, float &, trace_t &, CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t, int> > &, Vector * );  // line 80
	CBaseEntity *GetEntitiesAlongLaser( const Vector &, const Vector &, Vector *, CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t, int> > &, bool );  // line 81
	void DamageEntitiesAlongLaser( const CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t, int> > &, bool );  // line 82
	Vector m_vecNearestSoundSource[33]; // +0x518  // line 84
	CBaseEntity *m_pSoundProxy[33]; // +0x6a4  // line 85
	CSoundPatch *m_pAmbientSound[33]; // +0x728  // line 86
	CInfoPlacementHelper *m_pPlacementHelper; // +0x7ac  // line 87
	CSprite *m_pLaserGlow; // +0x7b0  // line 88
	int m_iLaserAttachment; // +0x7b4  // line 89
	string_t m_ModelName; // +0x7b8  // line 90
	bool m_bStartOff; // +0x7bc  // line 91
	bool m_bIsLethal; // +0x7bd  // line 92
	bool m_bFromReflectedCube; // +0x7be  // line 93
	EHANDLE m_hReflector; // +0x7c0  // line 95
	CNetworkVar( Vector, m_vecLaserEndPos ); // +0x7c4  // line 97
	CNetworkVar( Vector, m_vecLaserImpactNormal ); // +0x7d0  // line 98
	CNetworkVar( bool, m_bShouldSpark ); // +0x7dc  // line 99
	CBeam *m_pBeam; // +0x7e0  // line 101
};

// game/server/portal2/env_portal_laser.h:73 (declaration)
~PortalLaserInfo_t();

// game/server/portal2/env_portal_laser.h:73 (declaration)
void PortalLaserInfo_t();
