// DWARF declaration skeleton for game/server/portal2/env_portal_laser.h
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/env_portal_laser.h:20 sizeof=0x560 (i386)
struct CPortalLaser : public CBaseAnimating
{
public:
	CPortalLaser();  // line 24
	virtual ~CPortalLaser();  // line 25
	virtual void Spawn();  // line 26
	virtual void Precache();  // line 27
	virtual void Activate();  // line 28
	virtual void UpdateOnRemove();  // line 29
	virtual bool CreateVPhysics();  // line 30
	bool FindChildBeamsInSphere( const Vector &, float, CUtlVector<CBeam*,CUtlMemory<CBeam*, int> > & );  // line 32
	bool HasHitCube();  // line 33
	void InputTurnOn( inputdata_t & );  // line 35
	void InputTurnOff( inputdata_t & );  // line 36
	void InputToggle( inputdata_t & );  // line 37
	CPortalLaser *m_pNext; // +0x514  // line 39
private:
	bool InPVS();  // line 45
	void CreateHelperEntities();  // line 46
	void DamageEntity( CBaseEntity *, float );  // line 47
	void TurnOn();  // line 48
	void TurnOff();  // line 49
	bool IsOn();  // line 50
	void TurnOnGlow();  // line 52
	void TurnOffGlow();  // line 53
	void TurnOffLaserSound();  // line 55
	void FireAtPoint( trace_t &, bool );  // line 57
	void StrikeThink();  // line 58
	void BeamDamage( trace_t * );  // line 59
	void HideChildBeams();  // line 61
	void FireLaser( int, const Vector &, const Vector &, CBaseEntity * );  // line 62
	void UpdateSoundPosition( const Vector &, const Vector & );  // line 63
	bool StrikeEntitiesAlongLaser( const Vector &, const Vector &, Vector * );  // line 64
	bool ShouldAutoAim( CBaseEntity * );  // line 66
	CBaseEntity *TraceLaser( bool, const Vector &, const Vector &, float &, trace_t &, Vector * );  // line 67
	CBaseEntity *NewStrikeEntitiesAlongLaser( const Vector &, const Vector &, Vector *, bool );  // line 68
	Vector m_vecNearestSoundSource; // +0x518  // line 70
	CBaseEntity *m_pSoundProxy; // +0x524  // line 71
	CSoundPatch *m_pAmbientSound; // +0x528  // line 72
	CInfoPlacementHelper *m_pPlacementHelper; // +0x52c  // line 73
	CSprite *m_pLaserGlow; // +0x530  // line 74
	int m_iLaserAttachment; // +0x534  // line 75
	string_t m_ModelName; // +0x538  // line 76
	bool m_bStartOff; // +0x53c  // line 77
	bool m_bIsLethal; // +0x53d  // line 78
	bool m_bHasHitCube; // +0x53e  // line 79
	CBeam *m_pChildBeams[8]; // +0x540  // line 82
};
