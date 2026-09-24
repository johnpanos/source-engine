// DWARF declaration skeleton for game/shared/portal2/damage_database.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x15f80 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined DamageDatabase::DamageDatabase() at line 22
}

// game/shared/portal2/damage_database.cpp:13
ConVar max_hitbox_damage_effects_per_entity;

// game/shared/portal2/damage_database.cpp:14
ConVar hitbox_damage_enabled;

// game/shared/portal2/damage_database.cpp:22
DamageDatabase g_DamageDatabase;

// game/shared/portal2/damage_database.cpp:32 (declaration)
void DamageDatabase();

// game/shared/portal2/damage_database.cpp:32 @0x15f20 _ZN14DamageDatabaseC2Ev
DamageDatabase::DamageDatabase()
{
	// inlined CUtlVector<DamageInfo,CUtlMemory<DamageInfo, int> >::CUtlVector() at line 33
}

// game/shared/portal2/damage_database.cpp:32 @0x1b1da0 _ZN14DamageDatabaseC1Ev
DamageDatabase::DamageDatabase()
{
}

// game/shared/portal2/damage_database.cpp:37 @0x1b1d50 _ZN14DamageDatabase18LevelInitPreEntityEv
void DamageDatabase::LevelInitPreEntity()
{
	{
		int i;  // line 39
	}
}

// game/shared/portal2/damage_database.cpp:51 (declaration)
void GetDamageInfoVector( DamagedEntityType entityType );

// game/shared/portal2/damage_database.cpp:51 @0x1b0bb0 _ZNK14DamageDatabase19GetDamageInfoVectorE17DamagedEntityType
const DamageInfoVector &DamageDatabase::GetDamageInfoVector( DamagedEntityType entityType )
{
}

// game/shared/portal2/damage_database.cpp:57 @0x1b1300 _ZNK14DamageDatabase26FindPotentialDamageEffectsEPii17DamagedEntityTypeii
int DamageDatabase::FindPotentialDamageEffects( int *pPotentialDamageEffects, int maxDamageEffects, DamagedEntityType entityType, int damageType, int hitbox )
{
	const DamageInfoVector &damageInfo;  // line 59
	int potentialDamageEffectCount;  // line 61
	{
		int damageIndex;  // line 62
		{
			int hitboxIndex;  // line 72
			// inlined CUtlVector<DamageInfo,CUtlMemory<DamageInfo, int> >::operator[]() at line 72
		}
		// inlined CUtlVector<DamageInfo,CUtlMemory<DamageInfo, int> >::operator[]() at line 67
	}
}

// game/shared/portal2/damage_database.cpp:120 @0x1b13c0 _Z28ReadDamageCutoutDataFromFileR10CUtlVectorI10DamageInfo10CUtlMemoryIS0_iEEP11IFileSystemPKcPKh
bool ReadDamageCutoutDataFromFile( DamageInfoVector &damageInfoArray, IFileSystem *filesystem, const char *szCutoutDataFilename, const unsigned char *pICEKey )
{
	KeyValues *pKV;  // line 126
	const char *modelPath;  // line 130
	{
		KeyValues *pKey;  // line 132
	}
	{
		KeyValues *pMainDamageKeyValues;  // line 137
		{
			DamageInfo newInfo;  // line 141
			KeyValues *pKey;  // line 144
			// inlined CUtlVector<DamageInfo,CUtlMemory<DamageInfo, int> >::AddToTail() at line 227
			{
				int j;  // line 221
			}
			// inlined KeyValues::GetBool() at line 187
		}
	}
	{
		int i;  // line 260
		{
			int j;  // line 262
			// inlined CUtlVector<DamageInfo,CUtlMemory<DamageInfo, int> >::operator[]() at line 275
			{
				int k;  // line 266
				// inlined CUtlVector<DamageInfo,CUtlMemory<DamageInfo, int> >::operator[]() at line 268
			}
			// inlined CUtlVector<DamageInfo,CUtlMemory<DamageInfo, int> >::operator[]() at line 262
		}
	}
}

// game/shared/portal2/damage_database.cpp:287 @0x1b0e40 _Z21ParseStringToIntArrayPKcPii
int ParseStringToIntArray( const char *pString, int *pIntArray, int nMaxArray )
{
	int nInts;  // line 289
	char buffer[1024];  // line 291
	char *pBuffer;  // line 294
	char *pSpace;  // line 295
}

// game/shared/portal2/damage_database.cpp:313 @0x1b1160 _Z27ParseStringToExclusionArrayPKcPA64_ci
int ParseStringToExclusionArray( const char *pString, char (*ppStringArray)[64], int nMaxArray )
{
	int nStrings;  // line 315
	{
		char buffer[1024];  // line 319
		char *pBuffer;  // line 322
		char *pSpace;  // line 323
	}
}

// game/shared/portal2/damage_database.cpp:340 @0x1b0f70 _Z21ParseStringToGibArrayPKcS0_PA64_ci
int ParseStringToGibArray( const char *pString, const char *modelPath, char (*ppStringArray)[64], int nMaxArray )
{
	int nStrings;  // line 342
	{
		char buffer[1024];  // line 346
		char *pBuffer;  // line 349
		char *pSpace;  // line 350
	}
}

// game/shared/portal2/damage_database.cpp:367 @0x1b0cb0 _Z24ParseStringToEffectArrayPKcPA64_cPii
int ParseStringToEffectArray( const char *pString, char (*ppEffectArray)[64], int *pChanceArray, int nMaxArray )
{
	int nStrings;  // line 369
	{
		char buffer[1024];  // line 373
		char *pBuffer;  // line 376
		char *pSpace;  // line 377
	}
}

// game/shared/portal2/damage_database.cpp:439 @0x1b0bd0 _Z19ParseStringToVectorPKcR6Vector
ParseStringToVector( const char *pString, Vector &vector )
{
	char buffer[1024];  // line 441
	char *pBuffer;  // line 444
	char *pSpace;  // line 445
}

// public/tier1/utlmemory.h:707 @0x1b1db0 _ZN10CUtlMemoryI10DamageInfoiE4GrowEi
void CUtlMemory<DamageInfo,int>::Grow( int num )
{
	int nAllocationRequested;  // line 720
	int nNewAllocationCount;  // line 724
	// inlined UtlMemory_CalcNewAllocationCount() at line 724
	// inlined CUtlMemory<DamageInfo,int>::IsExternallyAllocated() at line 711
	// inlined MemAlloc_Alloc() at line 761
}
