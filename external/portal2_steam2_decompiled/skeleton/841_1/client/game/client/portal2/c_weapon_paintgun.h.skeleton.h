// DWARF declaration skeleton for game/client/portal2/c_weapon_paintgun.h
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal2/c_weapon_paintgun.h:15 sizeof=0xeac (i386)
struct C_WeaponPaintGun : public C_BasePortalCombatWeapon
{
public:
	static typedescription_t m_PredDesc[];  // line 19
	C_WeaponPaintGun();  // line 21
	virtual void Precache();  // line 23
	virtual void Spawn();  // line 24
	virtual void WeaponIdle();  // line 25
	virtual void ClientThink();  // line 26
	virtual bool Simulate();  // line 28
	virtual void OnPreDataChanged( DataUpdateType_t );  // line 29
	virtual void OnDataChanged( DataUpdateType_t );  // line 30
	virtual bool Deploy();  // line 31
	virtual bool Holster( C_BaseCombatWeapon * );  // line 32
	virtual void Drop( const Vector & );  // line 33
	virtual void SetSubType( int );  // line 34
	virtual void ItemPostFrame();  // line 35
	void StartHoseEffect();  // line 37
	void StopHoseEffect();  // line 38
	C_BaseViewModel *GetEffectViewModel();  // line 40
	virtual void AddViewmodelBob( C_BaseViewModel *, Vector &, QAngle & );  // line 42
	virtual float CalcViewmodelBob();  // line 43
	void SetPaintPower( PaintPowerType );  // line 45
	void CyclePaintPower( bool );  // line 46
	PaintPowerType GetCurrentPaint();  // line 47
	bool HasPaintPower( PaintPowerType );  // line 48
	bool HasAnyPaintPower();  // line 49
	void ChangeRenderColor( bool );  // line 50
	virtual void PrimaryAttack();  // line 52
	virtual void SecondaryAttack();  // line 53
	int GetPaintAmmo();  // line 55
	bool HasPaintAmmo();  // line 56
	static acttable_t m_acttable[];  // line 58
	virtual acttable_t *ActivityList();  // line 58
	virtual int ActivityListCount();  // line 58
private:
	void SprayPaint( float, int );  // line 61
	CUtlReference<CNewParticleEffect> m_pStreamEffect; // +0xe54  // line 63
	float m_flBobScale; // +0xe60  // line 64
	float m_flAccumulatedTime; // +0xe64  // line 65
	Vector m_vecOldBlobFirePos; // +0xe68  // line 66
	float m_flLastThinkTime; // +0xe74  // line 68
	int m_nCurrentColor; // +0xe78  // line 71
	bool m_bFiringPaint; // +0xe7c  // line 72
	bool m_bFiringErase; // +0xe7d  // line 73
	C_WeaponPaintGun::NetworkVar_m_bHasPaint m_bHasPaint; // +0xe7e  // line 75
	C_WeaponPaintGun::NetworkVar_m_hPaintStream m_hPaintStream; // +0xe84  // line 76
	C_BaseCombatCharacter *m_pLastOwner; // +0xe98  // line 79
	CUtlReference<CNewParticleEffect> m_pLastEffect; // +0xe9c  // line 80
	int m_nBlobRandomSeed; // +0xea8  // line 82
};

// game/client/portal2/c_weapon_paintgun.h:15 (declaration)
~C_WeaponPaintGun();

// game/client/portal2/c_weapon_paintgun.h:15 @0x542de0 _ZN16C_WeaponPaintGunD0Ev
C_WeaponPaintGun::~C_WeaponPaintGun()
{
	// inlined CUtlReference<CNewParticleEffect>::~CUtlReference() at line 15
	// inlined CUtlReference<CNewParticleEffect>::~CUtlReference() at line 15
	// inlined C_BasePortalCombatWeapon::~C_BasePortalCombatWeapon() at line 15
}

// game/client/portal2/c_weapon_paintgun.h:15 @0x542fe0 _ZN16C_WeaponPaintGunD1Ev
C_WeaponPaintGun::~C_WeaponPaintGun()
{
	// inlined CUtlReference<CNewParticleEffect>::~CUtlReference() at line 15
	// inlined CUtlReference<CNewParticleEffect>::~CUtlReference() at line 15
	// inlined C_BasePortalCombatWeapon::~C_BasePortalCombatWeapon() at line 15
}

// game/client/portal2/c_weapon_paintgun.h:76 (declaration)
void NetworkVar_m_hPaintStream();
