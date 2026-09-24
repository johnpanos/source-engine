// DWARF declaration skeleton for game/shared/portal2/damage_database.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/shared/portal2/damage_database.h:30
enum DamagedEntityType
{
	DAMAGED_TURRET = 0,
	DAMAGED_ENTITY_TYPE_COUNT = 1,
};

// game/shared/portal2/damage_database.h:39 sizeof=0xba8 (i386)
struct DamageInfo
{
public:
	char damageEffectName[64]; // +0x0  // line 40
	int hitBoxIndices[8]; // +0x40  // line 42
	int hitBoxCount; // +0x60  // line 43
	char exclusionListStrings[16][64]; // +0x64  // line 45
	int exclusionListIndices[16]; // +0x464  // line 46
	int exclusionCount; // +0x4a4  // line 47
	char deathThroesAnimation[64]; // +0x4a8  // line 49
	int damageType; // +0x4e8  // line 51
	char gibModels[16][64]; // +0x4ec  // line 53
	int gibCount; // +0x8ec  // line 54
	float gibSeparationSpeed; // +0x8f0  // line 55
	float gibLifeTime; // +0x8f4  // line 56
	char particleAttachment[64]; // +0x8f8  // line 58
	int particleAttachmentHitbox; // +0x938  // line 59
	char particleEffects[8][64]; // +0x93c  // line 60
	int particleEffectsChance[8]; // +0xb3c  // line 61
	int particleEffectTotalChance; // +0xb5c  // line 62
	int particleEffectCount; // +0xb60  // line 63
	bool isFatalDamage; // +0xb64  // line 65
	char swapModelName[64]; // +0xb65  // line 67
};

// game/shared/portal2/damage_database.h:70
typedef CUtlVector<DamageInfo,CUtlMemory<DamageInfo, int> > DamageInfoVector;

// game/shared/portal2/damage_database.h:73 sizeof=0x20 (i386)
struct DamageDatabase : public CAutoGameSystem
{
public:
	DamageDatabase();  // line 75
	virtual void LevelInitPreEntity();  // line 77
	const DamageInfoVector &GetDamageInfoVector( DamagedEntityType ) const;  // line 79
	int FindPotentialDamageEffects( int *, int, DamagedEntityType, int, int ) const;  // line 80
private:
	DamageInfoVector m_DamageInfo[1]; // +0xc  // line 83
};

// game/shared/portal2/damage_database.h:73 (declaration)
~DamageDatabase();

// game/shared/portal2/damage_database.h:73 @0x1b34e0 _ZN14DamageDatabaseD0Ev
DamageDatabase::~DamageDatabase()
{
	// inlined CUtlVector<DamageInfo,CUtlMemory<DamageInfo, int> >::~CUtlVector() at line 73
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 73
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 73
}

// game/shared/portal2/damage_database.h:73 @0x1b3620 _ZN14DamageDatabaseD1Ev
DamageDatabase::~DamageDatabase()
{
	// inlined CUtlVector<DamageInfo,CUtlMemory<DamageInfo, int> >::~CUtlVector() at line 73
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 73
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 73
}
