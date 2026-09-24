// DWARF declaration skeleton for game/server/portal/npc_portal_turret_floor.h
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal/npc_portal_turret_floor.h:27 sizeof=0x1154 (i386)
struct CNPC_Portal_FloorTurret : public HitBoxDamagedEntity<PlayerPickupPaintPowerUser<CNPC_FloorTurret> >
{
public:
	CNPC_Portal_FloorTurret();  // line 35
	virtual void Precache();  // line 37
	virtual void Spawn();  // line 38
	virtual void Activate();  // line 39
	virtual void UpdateOnRemove();  // line 40
	virtual int OnTakeDamage( const CTakeDamageInfo & );  // line 41
	virtual bool ShouldAttractAutoAim( CBaseEntity * );  // line 43
	virtual float GetAutoAimRadius();  // line 44
	virtual Vector GetAutoAimCenter();  // line 45
	virtual void OnPhysGunPickup( CBasePlayer *, PhysGunPickup_t );  // line 47
	virtual void NotifySystemEvent( CBaseEntity *, notify_system_event_t, const notify_system_event_params_t & );  // line 49
	virtual bool PreThink( turretState_e );  // line 51
	virtual void Shoot( const Vector &, const Vector &, bool );  // line 52
	virtual void SetEyeState( eyeState_t );  // line 53
	void TryEmitSound( const char * );  // line 54
	virtual bool OnSide();  // line 56
	virtual float GetAttackDamageScale( CBaseEntity * );  // line 58
	virtual Vector GetAttackSpread( CBaseCombatWeapon *, CBaseEntity * );  // line 59
	virtual void Retire();  // line 62
	virtual void Deploy();  // line 63
	virtual void ActiveThink();  // line 64
	virtual void SearchThink();  // line 65
	virtual void AutoSearchThink();  // line 66
	virtual void TippedThink();  // line 67
	virtual void HeldThink();  // line 68
	virtual void InactiveThink();  // line 69
	virtual void SuppressThink();  // line 70
	virtual void DisabledThink();  // line 71
	virtual void HackFindEnemy();  // line 72
	virtual void BurnThink();  // line 73
	virtual void StartTouch( CBaseEntity * );  // line 75
	bool IsLaserOn();  // line 77
	void LaserOff();  // line 78
	void LaserOn();  // line 79
	void RopesOn();  // line 80
	void RopesOff();  // line 81
	void FireBullet( const char * );  // line 83
	void InputFireBullet( inputdata_t & );  // line 86
	void InputEnableGagging( inputdata_t & );  // line 87
	void InputDisableGagging( inputdata_t & );  // line 88
	void InputEnablePickup( inputdata_t & );  // line 89
	void InputDisablePickup( inputdata_t & );  // line 90
	void InputSelfDestructImmediately( inputdata_t & );  // line 91
	virtual void Use( CBaseEntity *, CBaseEntity *, $_170, float );  // line 93
protected:
	virtual float GetFireConeZTolerance();  // line 96
private:
	bool IsMovingSuddenly();  // line 99
	bool IsEnemyBehindGlass( CPortal_Base2D *, CBaseEntity *, const Vector &, const Vector &, float );  // line 100
	virtual bool AllowedToIgnite();  // line 103
	CHandle<CRopeKeyframe> m_hRopes[4]; // +0x1108  // line 106
	CNetworkVar( bool, m_bOutOfAmmo ); // +0x1118  // line 108
	CNetworkVar( bool, m_bLaserOn ); // +0x1119  // line 109
	CNetworkVar( int, m_sLaserHaloSprite ); // +0x111c  // line 110
	int m_iBarrelAttachments[4]; // +0x1120  // line 112
	bool m_bShootWithBottomBarrels; // +0x1130  // line 113
	bool m_bDamageForce; // +0x1131  // line 114
	bool m_bPickupEnabled; // +0x1132  // line 115
	float m_fSearchSpeed; // +0x1134  // line 117
	float m_fMovingTargetThreashold; // +0x1138  // line 118
	float m_flDistToEnemy; // +0x113c  // line 119
	float m_flPreviousVelocity; // +0x1140  // line 120
	float m_flBurnExplodeTime; // +0x1144  // line 121
	turretState_e m_iLastState; // +0x1148  // line 123
	float m_fNextTalk; // +0x114c  // line 124
	bool m_bDelayTippedTalk; // +0x1150  // line 125
	bool m_bUsedAsActor; // +0x1151  // line 126
	bool m_bGagged; // +0x1152  // line 127
};
