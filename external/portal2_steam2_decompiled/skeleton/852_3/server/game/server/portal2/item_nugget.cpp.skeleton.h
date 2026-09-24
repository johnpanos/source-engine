// DWARF declaration skeleton for game/server/portal2/item_nugget.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xc0420 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSteamID::CSteamID() at line 650
	// inlined CSteamID::CSteamID() at line 654
	// inlined CSteamID::CSteamID() at line 656
	// inlined CSteamID::CSteamID() at line 659
	// inlined CSteamID::CSteamID() at line 662
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
	// inlined CEntityFactory<CItem_Nugget>::CEntityFactory() at line 55
	// inlined CNuggetDirector::CNuggetDirector() at line 82
}

// game/server/portal2/item_nugget.cpp:19
enum Nugget_Values_t
{
	NUGGET_VALUE_1 = 1,
	NUGGET_VALUE_5 = 2,
	NUGGET_VALUE_10 = 3,
	NUGGET_VALUE_25 = 4,
};

// game/server/portal2/item_nugget.cpp:31 sizeof=0x520 (i386)
struct CItem_Nugget : public CBaseAnimating
{
public:
	virtual void Spawn();  // line 36
	virtual void Precache();  // line 37
	virtual void StartTouch( CBaseEntity * );  // line 38
	virtual CBaseEntity *Respawn();  // line 39
	virtual void UpdateOnRemove();  // line 40
	void MaterializeThink();  // line 41
	float m_fRespawnTime; // +0x514  // line 43
	int m_nValue; // +0x518  // line 44
	string_t m_iGroupName; // +0x51c  // line 45
};

// game/server/portal2/item_nugget.cpp:31 (declaration)
~CItem_Nugget();

// game/server/portal2/item_nugget.cpp:31 @0x6bbd30 _ZN12CItem_NuggetD0Ev
CItem_Nugget::~CItem_Nugget()
{
}

// game/server/portal2/item_nugget.cpp:31 @0x6bbd70 _ZN12CItem_NuggetD1Ev
CItem_Nugget::~CItem_Nugget()
{
}

// game/server/portal2/item_nugget.cpp:31 (declaration)
void CItem_Nugget();

// game/server/portal2/item_nugget.cpp:34 @0x6bb020 _ZN12CItem_Nugget14GetDataDescMapEv
datamap_t *CItem_Nugget::GetDataDescMap()
{
}

// game/server/portal2/item_nugget.cpp:34 @0x6bb030 _ZN12CItem_Nugget10GetBaseMapEv
datamap_t *CItem_Nugget::GetBaseMap()
{
}

// game/server/portal2/item_nugget.cpp:36 @0x6bb920 _ZN12CItem_Nugget5SpawnEv
void CItem_Nugget::Spawn()
{
	float fScale;  // line 92
	float fScaledExtent;  // line 131
	const Vector vModelOffset;  // line 132
	// inlined Vector::operator VectorByValue&() at line 134
	// inlined Vector::operator-() at line 134
	// inlined Vector::operator VectorByValue&() at line 134
	// inlined Vector::operator-() at line 134
	// inlined Vector::operator VectorByValue&() at line 133
	// inlined Vector::operator+() at line 133
	// inlined CBaseEntity::GetAbsOrigin() at line 133
	// inlined Vector::operator VectorByValue&() at line 126
	// inlined Vector::Vector() at line 126
	// inlined CBaseEntity::SetSolidFlags() at line 121
	// inlined CBaseEntity::SetSolid() at line 120
	// inlined CNetworkVarBase<int,CBaseAnimating::NetworkVar_m_nSkin>::operator=<Nugget_Values_t>() at line 98
	// inlined CNetworkVarBase<int,CBaseAnimating::NetworkVar_m_nSkin>::operator=<Nugget_Values_t>() at line 103
	// inlined CNetworkVarBase<int,CBaseAnimating::NetworkVar_m_nSkin>::operator=<Nugget_Values_t>() at line 109
	// inlined CNetworkVarBase<int,CBaseAnimating::NetworkVar_m_nSkin>::operator=<Nugget_Values_t>() at line 115
}

// game/server/portal2/item_nugget.cpp:37 @0x6bb0d0 _ZN12CItem_Nugget8PrecacheEv
void CItem_Nugget::Precache()
{
}

// game/server/portal2/item_nugget.cpp:38 @0x6bb3a0 _ZN12CItem_Nugget10StartTouchEP11CBaseEntity
void CItem_Nugget::StartTouch( CBaseEntity *pOther )
{
	CBasePlayer *pPlayer;  // line 176
	CSingleUserRecipientFilter user;  // line 178
	CPASAttenuationFilter filter;  // line 185
	// inlined CSingleUserRecipientFilter::CSingleUserRecipientFilter() at line 178
	// inlined CBaseEntity::GetClassname() at line 182
	// inlined CPASAttenuationFilter::CPASAttenuationFilter() at line 185
	// inlined CBaseEntity::entindex() at line 186
	// inlined CPASAttenuationFilter::~CPASAttenuationFilter() at line 201
	// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 201
	// inlined CPASAttenuationFilter::~CPASAttenuationFilter() at line 201
	// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 201
}

// game/server/portal2/item_nugget.cpp:39 @0x6bb040 _ZN12CItem_Nugget7RespawnEv
CBaseEntity *CItem_Nugget::Respawn()
{
}

// game/server/portal2/item_nugget.cpp:40 @0x6bb1c0 _ZN12CItem_Nugget14UpdateOnRemoveEv
void CItem_Nugget::UpdateOnRemove()
{
}

// game/server/portal2/item_nugget.cpp:41 @0x6bb2e0 _ZN12CItem_Nugget16MaterializeThinkEv
void CItem_Nugget::MaterializeThink()
{
	// inlined CBaseEntity::RemoveEffects() at line 220
}

// game/server/portal2/item_nugget.cpp:48 @0xc0200 _Z11DataMapInitI12CItem_NuggetEP9datamap_tPT_
datamap_t *DataMapInit<CItem_Nugget>( CItem_Nugget * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 53
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 48
	CDatadescGeneratedNameHolder nameHolder;  // line 48
	typedescription_t dataDesc[5];  // line 48
}

// game/server/portal2/item_nugget.cpp:48
datamap_t *g_DataMapHolder;

// game/server/portal2/item_nugget.cpp:55
static CEntityFactory<CItem_Nugget> item_nugget;

// game/server/portal2/item_nugget.cpp:61 sizeof=0x3c (i386)
struct CNuggetDirector : public CAutoGameSystem
{
public:
	void RegisterNugget( CItem_Nugget * );  // line 65
	void UnregisterNugget( CItem_Nugget * );  // line 66
	void RandomizeAvailableNuggets( float, float );  // line 67
	virtual void LevelInitPostEntity();  // line 68
	// game/server/portal2/item_nugget.cpp:71 sizeof=0x1 (i386)
	struct CNuggetGroupSort
	{
	public:
		bool Less( const CItem_Nugget *, const CItem_Nugget *, void * );  // line 73
	};  // line 71
	CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*, int> > m_UngroupedNuggets; // +0xc  // line 79
	CUtlSortVector<CItem_Nugget*,CNuggetDirector::CNuggetGroupSort> m_GroupedNuggets; // +0x20  // line 80
};

// game/server/portal2/item_nugget.cpp:61 (declaration)
void CNuggetDirector();

// game/server/portal2/item_nugget.cpp:61 (declaration)
~CNuggetDirector();

// game/server/portal2/item_nugget.cpp:61 @0x6bc380 _ZN15CNuggetDirectorD0Ev
CNuggetDirector::~CNuggetDirector()
{
	// inlined CUtlSortVector<CItem_Nugget*,CNuggetDirector::CNuggetGroupSort>::~CUtlSortVector() at line 61
	// inlined CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*, int> >::~CUtlVector() at line 61
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 61
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 61
	// inlined CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*, int> >::~CUtlVector() at line 61
}

// game/server/portal2/item_nugget.cpp:61 @0x6bc590 _ZN15CNuggetDirectorD1Ev
CNuggetDirector::~CNuggetDirector()
{
	// inlined CUtlSortVector<CItem_Nugget*,CNuggetDirector::CNuggetGroupSort>::~CUtlSortVector() at line 61
	// inlined CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*, int> >::~CUtlVector() at line 61
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 61
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 61
	// inlined CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*, int> >::~CUtlVector() at line 61
}

// game/server/portal2/item_nugget.cpp:65 @0x6bb200 _ZN15CNuggetDirector14RegisterNuggetEP12CItem_Nugget
void CNuggetDirector::RegisterNugget( CItem_Nugget *pNugget )
{
	// inlined string_t::operator==() at line 232
	// inlined FStrEq() at line 232
	// inlined CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*, int> >::AddToTail() at line 234
}

// game/server/portal2/item_nugget.cpp:66 @0x6bb110 _ZN15CNuggetDirector16UnregisterNuggetEP12CItem_Nugget
void CNuggetDirector::UnregisterNugget( CItem_Nugget *pNugget )
{
	// inlined string_t::operator==() at line 244
	// inlined FStrEq() at line 244
	// inlined CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*, int> >::FindAndFastRemove() at line 246
}

// game/server/portal2/item_nugget.cpp:67 @0x6bb690 _ZN15CNuggetDirector25RandomizeAvailableNuggetsEff
void CNuggetDirector::RandomizeAvailableNuggets( float fUngroupedAvailabilityChance, float fGroupAvailabilityChance )
{
	int iCount;  // line 256
	string_t iLastGroup;  // line 270
	bool bExists;  // line 271
	{
		int i;  // line 272
		// inlined CBaseEntity::RemoveEffects() at line 282
	}
	// inlined CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*, int> >::Count() at line 269
	{
		int i;  // line 257
		// inlined CBaseEntity::RemoveEffects() at line 261
	}
	// inlined CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*, int> >::Count() at line 256
}

// game/server/portal2/item_nugget.cpp:68 @0x6bb8d0 _ZN15CNuggetDirector19LevelInitPostEntityEv
void CNuggetDirector::LevelInitPostEntity()
{
}

// game/server/portal2/item_nugget.cpp:82
static CNuggetDirector g_NuggetDirector;

// game/server/util.h:99 @0x6bbd90 _ZN14CEntityFactoryI12CItem_NuggetE6CreateEPKc
IServerNetworkable *CEntityFactory<CItem_Nugget>::Create( const char *pClassName )
{
	CItem_Nugget *pEnt;  // line 101
	// inlined _CreateEntityTemplate<CItem_Nugget>() at line 101
}

// game/server/util.h:105 @0x6bbcf0 _ZN14CEntityFactoryI12CItem_NuggetE7DestroyEP18IServerNetworkable
void CEntityFactory<CItem_Nugget>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:113 @0x6bbd20 _ZN14CEntityFactoryI12CItem_NuggetE13GetEntitySizeEv
size_t CEntityFactory<CItem_Nugget>::GetEntitySize()
{
}

// public/tier1/UtlSortVector.h:146 @0x6bc230 _ZN14CUtlSortVectorIP12CItem_NuggetN15CNuggetDirector16CNuggetGroupSortEE6InsertERKS1_
int CUtlSortVector<CItem_Nugget*,CNuggetDirector::CNuggetGroupSort>::Insert( CItem_Nugget *const &src )
{
	int pos;  // line 150
	// inlined CopyConstruct<CItem_Nugget*>() at line 153
	// inlined CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*, int> >::ShiftElementsRight() at line 152
	{
		LoggingResponse_t ret;  // line 148
	}
}

// public/tier1/UtlSortVector.h:211 @0x6bbe00 _ZNK14CUtlSortVectorIP12CItem_NuggetN15CNuggetDirector16CNuggetGroupSortEE4FindERKS1_
int CUtlSortVector<CItem_Nugget*,CNuggetDirector::CNuggetGroupSort>::Find( CItem_Nugget *const &src )
{
	CNuggetDirector::CNuggetGroupSort less;  // line 215
	int start;  // line 217
	int end;  // line 217
	{
		int mid;  // line 220
		// inlined CNuggetDirector::CNuggetGroupSort::Less() at line 221
	}
	{
		LoggingResponse_t ret;  // line 213
	}
}

// public/tier1/UtlSortVector.h:242 @0x6bbf00 _ZNK14CUtlSortVectorIP12CItem_NuggetN15CNuggetDirector16CNuggetGroupSortEE15FindLessOrEqualERKS1_
int CUtlSortVector<CItem_Nugget*,CNuggetDirector::CNuggetGroupSort>::FindLessOrEqual( CItem_Nugget *const &src )
{
	CNuggetDirector::CNuggetGroupSort less;  // line 246
	int start;  // line 247
	int end;  // line 247
	{
		int mid;  // line 250
		// inlined CNuggetDirector::CNuggetGroupSort::Less() at line 251
	}
	{
		LoggingResponse_t ret;  // line 244
	}
}

// game/server/portal2/item_nugget.cpp:291
ConVar sv_randomize_nugget_availability;

// game/server/portal2/item_nugget.cpp:292
ConVar sv_randomize_nugget_availability_ungroupedavailability;

// game/server/portal2/item_nugget.cpp:293
ConVar sv_randomize_nugget_availability_groupavailability;

// public/tier1/UtlSortVector.h:294 @0x6bc000 _ZN14CUtlSortVectorIP12CItem_NuggetN15CNuggetDirector16CNuggetGroupSortEE6RemoveERKS1_
void CUtlSortVector<CItem_Nugget*,CNuggetDirector::CNuggetGroupSort>::Remove( CItem_Nugget *const &search )
{
	int pos;  // line 298
	// inlined CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*, int> >::Remove() at line 301
	{
		LoggingResponse_t ret;  // line 296
	}
}

// public/tier1/utlvector.h:655 @0x6bc110 _ZN10CUtlVectorIP12CItem_Nugget10CUtlMemoryIS1_iEE10GrowVectorEi
void CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*, int> >::GrowVector( int num )
{
	// inlined CUtlMemory<CItem_Nugget*,int>::NumAllocated() at line 657
	// inlined CUtlMemory<CItem_Nugget*,int>::Grow() at line 660
	// inlined CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*, int> >::ResetDbgInfo() at line 664
}
