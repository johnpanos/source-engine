// DWARF declaration skeleton for game/client/portal2/c_weapon_paintgun.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal2/c_weapon_paintgun.h:14 sizeof=0xe94 (i386)
struct C_WeaponPaintGun : public C_BasePortalCombatWeapon
{
public:
	static typedescription_t m_PredDesc[];  // line 18
	C_WeaponPaintGun();  // line 20
	virtual void Precache();  // line 22
	virtual void Spawn();  // line 23
	virtual void WeaponIdle();  // line 24
	virtual void ClientThink();  // line 25
	virtual bool Simulate();  // line 27
	virtual void OnPreDataChanged( DataUpdateType_t );  // line 28
	virtual void OnDataChanged( DataUpdateType_t );  // line 29
	virtual bool Deploy();  // line 30
	virtual bool Holster( C_BaseCombatWeapon * );  // line 31
	virtual void Drop( const Vector & );  // line 32
	virtual void SetSubType( int );  // line 33
	virtual void ItemPostFrame();  // line 34
	void StartHoseEffect();  // line 36
	void StopHoseEffect();  // line 37
	C_BaseViewModel *GetEffectViewModel();  // line 39
	virtual void AddViewmodelBob( C_BaseViewModel *, Vector &, QAngle & );  // line 41
	virtual float CalcViewmodelBob();  // line 42
	void SetPaintPower( PaintPowerType );  // line 44
	void CyclePaintPower( bool );  // line 45
	PaintPowerType GetCurrentPaint();  // line 46
	bool HasPaintPower( PaintPowerType );  // line 47
	bool HasAnyPaintPower();  // line 48
	void ChangeRenderColor( bool );  // line 49
	virtual void PrimaryAttack();  // line 51
	virtual void SecondaryAttack();  // line 52
	int GetPaintAmmo();  // line 54
	bool HasPaintAmmo();  // line 55
	static acttable_t m_acttable[];  // line 57
	virtual acttable_t *ActivityList();  // line 57
	virtual int ActivityListCount();  // line 57
private:
	void SprayPaint( float, int );  // line 60
	CUtlReference<CNewParticleEffect> m_pStreamEffect; // +0xe50  // line 62
	float m_flBobScale; // +0xe5c  // line 63
	float m_flAccumulatedTime; // +0xe60  // line 64
	Vector m_vecOldBlobFirePos; // +0xe64  // line 65
	float m_flLastThinkTime; // +0xe70  // line 67
	int m_nCurrentColor; // +0xe74  // line 70
	bool m_bFiringPaint; // +0xe78  // line 71
	bool m_bFiringErase; // +0xe79  // line 72
	C_WeaponPaintGun::NetworkVar_m_bHasPaint m_bHasPaint; // +0xe7a  // line 74
	C_BaseCombatCharacter *m_pLastOwner; // +0xe80  // line 77
	CUtlReference<CNewParticleEffect> m_pLastEffect; // +0xe84  // line 78
	int m_nBlobRandomSeed; // +0xe90  // line 80
};

// game/client/portal2/c_weapon_paintgun.h:14 (declaration)
~C_WeaponPaintGun();

// game/client/portal2/c_weapon_paintgun.h:14 @0x540780 _ZN16C_WeaponPaintGunD0Ev
C_WeaponPaintGun::~C_WeaponPaintGun()
{
	// inlined CUtlReference<CNewParticleEffect>::~CUtlReference() at line 14
	// inlined CUtlReference<CNewParticleEffect>::~CUtlReference() at line 14
	// inlined C_BasePortalCombatWeapon::~C_BasePortalCombatWeapon() at line 14
}

// game/client/portal2/c_weapon_paintgun.h:14 @0x540990 _ZN16C_WeaponPaintGunD1Ev
C_WeaponPaintGun::~C_WeaponPaintGun()
{
	// inlined CUtlReference<CNewParticleEffect>::~CUtlReference() at line 14
	// inlined CUtlReference<CNewParticleEffect>::~CUtlReference() at line 14
	// inlined C_BasePortalCombatWeapon::~C_BasePortalCombatWeapon() at line 14
}
