// DWARF declaration skeleton for game/server/portal2/rocketprojectile.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/rocketprojectile.h:18 sizeof=0x8e8 (i386)
struct CRocket_Turret_Projectile : public CBaseCombatCharacter
{
public:
	virtual void Precache();  // line 22
	virtual void Spawn();  // line 23
	virtual void UpdateOnRemove();  // line 24
	virtual void NotifySystemEvent( CBaseEntity *, notify_system_event_t, const notify_system_event_params_t & );  // line 26
	virtual void SetLauncher( EHANDLE );  // line 27
	virtual void CreateSmokeTrail();  // line 28
	virtual void MissileTouch( CBaseEntity * );  // line 30
	void Explode();  // line 31
protected:
	virtual void DoExplosion();  // line 34
	virtual void CreateSounds();  // line 35
	virtual void StopLoopingSounds();  // line 36
	EHANDLE m_hLauncher; // +0x8dc  // line 38
	CSoundPatch *m_pAmbientSound; // +0x8e0  // line 39
	CHandle<RocketTrail> m_hRocketTrail; // +0x8e4  // line 40
};

// game/server/portal2/rocketprojectile.h:18 (declaration)
~CRocket_Turret_Projectile();

// game/server/portal2/rocketprojectile.h:18 @0x774ca0 _ZN25CRocket_Turret_ProjectileD0Ev
CRocket_Turret_Projectile::~CRocket_Turret_Projectile()
{
}

// game/server/portal2/rocketprojectile.h:18 @0x774ce0 _ZN25CRocket_Turret_ProjectileD1Ev
CRocket_Turret_Projectile::~CRocket_Turret_Projectile()
{
}

// game/server/portal2/rocketprojectile.h:18 (declaration)
void CRocket_Turret_Projectile();
