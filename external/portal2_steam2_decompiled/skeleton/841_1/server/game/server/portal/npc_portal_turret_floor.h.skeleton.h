// DWARF declaration skeleton for game/server/portal/npc_portal_turret_floor.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal/npc_portal_turret_floor.h:27 sizeof=0x115c (i386)
struct CNPC_Portal_FloorTurret : public HitBoxDamagedEntity<PlayerPickupPaintPowerUser<CNPC_FloorTurret> >
{
public:
	CNPC_Portal_FloorTurret();  // line 35
	virtual void Precache();  // line 37
	virtual void Spawn();  // line 38
	virtual void Activate();  // line 39
	virtual void OnRestore();  // line 40
	virtual void UpdateOnRemove();  // line 41
	virtual int OnTakeDamage( const CTakeDamageInfo & );  // line 42
	virtual bool ShouldAttractAutoAim( CBaseEntity * );  // line 44
	virtual float GetAutoAimRadius();  // line 45
	virtual Vector GetAutoAimCenter();  // line 46
	virtual void OnPhysGunPickup( CBasePlayer *, PhysGunPickup_t );  // line 48
	virtual void NotifySystemEvent( CBaseEntity *, notify_system_event_t, const notify_system_event_params_t & );  // line 50
	virtual bool PreThink( turretState_e );  // line 52
	virtual void Shoot( const Vector &, const Vector &, bool );  // line 53
	virtual void SetEyeState( eyeState_t );  // line 54
	void TryEmitSound( const char * );  // line 55
	virtual bool OnSide();  // line 57
	void EnableTipController( bool );  // line 58
	virtual float GetAttackDamageScale( CBaseEntity * );  // line 60
	virtual Vector GetAttackSpread( CBaseCombatWeapon *, CBaseEntity * );  // line 61
	virtual bool HasPreferredCarryAnglesForPlayer( CBasePlayer * );  // line 63
	virtual QAngle PreferredCarryAngles();  // line 64
	virtual void Retire();  // line 67
	virtual void Deploy();  // line 68
	virtual void ActiveThink();  // line 69
	virtual void SearchThink();  // line 70
	virtual void AutoSearchThink();  // line 71
	virtual void TippedThink();  // line 72
	virtual void HeldThink();  // line 73
	virtual void InactiveThink();  // line 74
	virtual void DieThink();  // line 75
	virtual void SuppressThink();  // line 76
	virtual void DisabledThink();  // line 77
	virtual void HackFindEnemy();  // line 78
	virtual void BurnThink();  // line 79
	void OnEnteredTractorBeam();  // line 81
	void OnExitedTractorBeam();  // line 82
	void TractorBeamThink();  // line 83
	virtual void StartTouch( CBaseEntity * );  // line 85
	bool IsLaserOn();  // line 87
	void LaserOff();  // line 88
	void LaserOn();  // line 89
	void RopesOn();  // line 90
	void RopesOff();  // line 91
	void FireBullet( const char * );  // line 93
	void InputFireBullet( inputdata_t & );  // line 96
	void InputEnableGagging( inputdata_t & );  // line 97
	void InputDisableGagging( inputdata_t & );  // line 98
	void InputEnablePickup( inputdata_t & );  // line 99
	void InputDisablePickup( inputdata_t & );  // line 100
	void InputSelfDestructImmediately( inputdata_t & );  // line 101
	void InputSetAsBouncePainted( inputdata_t & );  // line 102
	virtual void Use( CBaseEntity *, CBaseEntity *, $_170, float );  // line 104
protected:
	virtual float GetFireConeZTolerance();  // line 107
private:
	bool IsMovingSuddenly();  // line 110
	bool IsEnemyBehindGlass( CPortal_Base2D *, CBaseEntity *, const Vector &, const Vector &, float );  // line 111
	virtual bool AllowedToIgnite();  // line 114
	CHandle<CRopeKeyframe> m_hRopes[4]; // +0x1108  // line 117
	CNetworkVar( bool, m_bOutOfAmmo ); // +0x1118  // line 119
	CNetworkVar( bool, m_bLaserOn ); // +0x1119  // line 120
	CNetworkVar( int, m_sLaserHaloSprite ); // +0x111c  // line 121
	bool m_bIsDead; // +0x1120  // line 123
	int m_iBarrelAttachments[4]; // +0x1124  // line 125
	bool m_bShootWithBottomBarrels; // +0x1134  // line 126
	bool m_bDamageForce; // +0x1135  // line 127
	bool m_bPickupEnabled; // +0x1136  // line 128
	float m_fSearchSpeed; // +0x1138  // line 130
	float m_fMovingTargetThreashold; // +0x113c  // line 131
	bool m_bShootAtMovingObjects; // +0x1140  // line 132
	float m_flDistToEnemy; // +0x1144  // line 133
	float m_flPreviousVelocity; // +0x1148  // line 134
	float m_flBurnExplodeTime; // +0x114c  // line 135
	turretState_e m_iLastState; // +0x1150  // line 137
	float m_fNextTalk; // +0x1154  // line 138
	bool m_bDelayTippedTalk; // +0x1158  // line 139
	bool m_bUsedAsActor; // +0x1159  // line 140
	bool m_bGagged; // +0x115a  // line 141
	bool m_bSeeEnemyThroughPortal; // +0x115b  // line 142
};
