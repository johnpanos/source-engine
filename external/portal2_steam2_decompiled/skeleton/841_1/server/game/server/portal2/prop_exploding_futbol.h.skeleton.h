// DWARF declaration skeleton for game/server/portal2/prop_exploding_futbol.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/prop_exploding_futbol.h:8
enum ExplodingFutbolHolderType_t
{
	EXPLODING_FUTBOL_HELD_BY_NONE = 0,
	EXPLODING_FUTBOL_HELD_BY_PLAYER = 1,
	EXPLODING_FUTBOL_HELD_BY_SPAWNER = 2,
	EXPLODING_FUTBOL_HELD_BY_CATCHER = 3,
	EXPLODING_FUTBOL_HELD_BY_COUNT = 4,
};

// game/server/portal2/prop_exploding_futbol.h:22 sizeof=0x7a4 (i386)
struct CPropExplodingFutbol : public CPhysicsProp
{
public:
	CPropExplodingFutbol();  // line 27
	virtual void Precache();  // line 29
	virtual void Spawn();  // line 30
	virtual void Event_Killed( const CTakeDamageInfo & );  // line 32
	ExplodingFutbolHolderType_t GetHolder();  // line 35
	void SetHolder( ExplodingFutbolHolderType_t );  // line 36
	CPortal_Player *GetLastPlayerToHold();  // line 37
	virtual void SetSpawner( CPropExplodingFutbolSpawner * );  // line 38
	CPropExplodingFutbolSpawner *GetSpawner();  // line 39
	void KillThink();  // line 41
	void AnimThink();  // line 42
	void TimerThink();  // line 43
	void ActivateFutbolTimer( float );  // line 45
	void StopFutbolTimer();  // line 46
	virtual void OnPhysGunPickup( CBasePlayer *, PhysGunPickup_t );  // line 49
	virtual void OnPhysGunDrop( CBasePlayer *, PhysGunDrop_t );  // line 50
	virtual int OnTakeDamage( const CTakeDamageInfo & );  // line 52
	virtual void OnFizzled();  // line 53
	virtual void VPhysicsCollision( int, gamevcollisionevent_t * );  // line 54
	virtual void VPhysicsUpdate( IPhysicsObject * );  // line 55
private:
	void KillFutbol();  // line 58
	void DestroyFutbol( bool );  // line 59
	void ExplodeFutbol();  // line 60
	void DoExplosion( const Vector &, float, CPortal_Base2D * );  // line 61
	void DamageEntity( CBaseEntity *, const Vector &, const Vector & );  // line 62
	void InputExplode( inputdata_t & );  // line 64
	ExplodingFutbolHolderType_t m_Holder; // +0x768  // line 66
	CHandle<CPropExplodingFutbolSpawner> m_hSpawner; // +0x76c  // line 67
	string_t m_strSpawnerName; // +0x770  // line 68
	CHandle<CPortal_Player> m_hLastHeldByPlayer; // +0x774  // line 69
	CUtlVector<CBaseEntity*,CUtlMemory<CBaseEntity*, int> > m_ExplodedEntities; // +0x778  // line 70
	bool m_bShouldRespawn; // +0x78c  // line 72
	bool m_bTimerActive; // +0x78d  // line 73
	bool m_bExplodeOnTouch; // +0x78e  // line 74
	float m_flExplosionTimer; // +0x790  // line 75
	float m_flTotalTimer; // +0x794  // line 76
	float m_flLastTickTime; // +0x798  // line 77
	float m_flLastTimerSoundTime; // +0x79c  // line 78
	float m_flLastFlashTime; // +0x7a0  // line 79
};

// game/server/portal2/prop_exploding_futbol.h:22 (declaration)
~CPropExplodingFutbol();

// game/server/portal2/prop_exploding_futbol.h:22 @0x755b30 _ZN20CPropExplodingFutbolD1Ev
CPropExplodingFutbol::~CPropExplodingFutbol()
{
	// inlined CUtlVector<CBaseEntity*,CUtlMemory<CBaseEntity*, int> >::~CUtlVector() at line 22
}

// game/server/portal2/prop_exploding_futbol.h:22 @0x755dd0 _ZN20CPropExplodingFutbolD0Ev
CPropExplodingFutbol::~CPropExplodingFutbol()
{
	// inlined CUtlVector<CBaseEntity*,CUtlMemory<CBaseEntity*, int> >::~CUtlVector() at line 22
}
