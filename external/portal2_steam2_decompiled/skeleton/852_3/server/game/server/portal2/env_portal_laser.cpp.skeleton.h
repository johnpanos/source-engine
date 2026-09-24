// DWARF declaration skeleton for game/server/portal2/env_portal_laser.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xbdd50 _Z41__static_initialization_and_destruction_0ii
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
	// inlined CEntityClassList<CPortalLaser>::CEntityClassList() at line 42
	// inlined CEntityFactory<CPortalLaser>::CEntityFactory() at line 50
}

// game/server/entitylist.h:34 @0x6b5fd0 _ZN16CEntityClassListI12CPortalLaserE23LevelShutdownPostEntityEv
void CEntityClassList<CPortalLaser>::LevelShutdownPostEntity()
{
}

// game/server/portal2/env_portal_laser.cpp:34
ConVar sv_player_collide_with_laser;

// game/server/portal2/env_portal_laser.cpp:37
ConVar new_portal_laser;

// game/server/portal2/env_portal_laser.cpp:38
ConVar laser_delay;

// game/server/portal2/env_portal_laser.cpp:39
ConVar sv_debug_laser;

// game/server/portal2/env_portal_laser.cpp:42
CEntityClassList<CPortalLaser> g_EnvPortalLaserList;

// game/server/portal2/env_portal_laser.cpp:43
void CEntityClassList<CPortalLaser>::m_pClassList;

// game/server/portal2/env_portal_laser.cpp:45 (declaration)
CPortalLaser *GetEnvPortalLaserList();

// game/server/portal2/env_portal_laser.cpp:45 @0x6afe10 _Z21GetEnvPortalLaserListv
CPortalLaser *GetEnvPortalLaserList()
{
}

// game/server/portal2/env_portal_laser.cpp:50
static CEntityFactory<CPortalLaser> env_portal_laser;

// game/server/portal2/env_portal_laser.cpp:52 (declaration)
void GetBaseMap();

// game/server/portal2/env_portal_laser.cpp:52 @0x6afe20 _ZN12CPortalLaser14GetDataDescMapEv
datamap_t *CPortalLaser::GetDataDescMap()
{
}

// game/server/portal2/env_portal_laser.cpp:52 @0x6afe30 _ZN12CPortalLaser10GetBaseMapEv
datamap_t *CPortalLaser::GetBaseMap()
{
}

// game/server/portal2/env_portal_laser.cpp:52 @0xbda60 _Z11DataMapInitI12CPortalLaserEP9datamap_tPT_
datamap_t *DataMapInit<CPortalLaser>( CPortalLaser * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 75
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 52
	CDatadescGeneratedNameHolder nameHolder;  // line 52
	typedescription_t dataDesc[13];  // line 52
}

// game/server/portal2/env_portal_laser.cpp:52
datamap_t *g_DataMapHolder;

// game/server/portal2/env_portal_laser.cpp:52
void CPortalLaser::m_DataMap;

// game/server/portal2/env_portal_laser.cpp:77
static const char LASER_LOOPING_SOUND[15];

// game/server/portal2/env_portal_laser.cpp:78
static const char LASER_BURN_SOUND[19];

// game/server/portal2/env_portal_laser.cpp:85 (declaration)
void CPortalLaser();

// game/server/portal2/env_portal_laser.cpp:85 @0x6b0150 _ZN12CPortalLaserC1Ev
CPortalLaser::CPortalLaser()
{
	// inlined CEntityClassList<CPortalLaser>::Insert() at line 87
}

// game/server/portal2/env_portal_laser.cpp:85 @0x6b01a0 _ZN12CPortalLaserC2Ev
CPortalLaser::CPortalLaser()
{
	// inlined CEntityClassList<CPortalLaser>::Insert() at line 87
}

// game/server/portal2/env_portal_laser.cpp:90 (declaration)
~CPortalLaser();

// game/server/portal2/env_portal_laser.cpp:90 @0x6b00f0 _ZN12CPortalLaserD2Ev
CPortalLaser::~CPortalLaser()
{
	// inlined CEntityClassList<CPortalLaser>::Remove() at line 92
}

// game/server/portal2/env_portal_laser.cpp:90 @0x6b0270 _ZN12CPortalLaserD0Ev
CPortalLaser::~CPortalLaser()
{
	// inlined CEntityClassList<CPortalLaser>::Remove() at line 92
}

// game/server/portal2/env_portal_laser.cpp:90 @0x6b02f0 _ZN12CPortalLaserD1Ev
CPortalLaser::~CPortalLaser()
{
	// inlined CEntityClassList<CPortalLaser>::Remove() at line 92
}

// public/tier1/utlsortvector.h:99 @0x6b5ff0 _ZN14CUtlSortVectorI17LaserVictimInfo_t16CLaserVictimLessE13CompareHelperEPKS0_S4_
int CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::CompareHelper( const $_376 *lhs, const $_376 *rhs )
{
	CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::QSortContext_t *ctx;  // line 101
	// inlined CLaserVictimLess::Less() at line 102
}

// game/server/util.h:99 @0x6b6020 _ZN14CEntityFactoryI12CPortalLaserE6CreateEPKc
IServerNetworkable *CEntityFactory<CPortalLaser>::Create( const char *pClassName )
{
	CPortalLaser *pEnt;  // line 101
	// inlined _CreateEntityTemplate<CPortalLaser>() at line 101
}

// game/server/portal2/env_portal_laser.cpp:100 @0x6b1810 _ZN12CPortalLaser5SpawnEv
void CPortalLaser::Spawn()
{
	// inlined string_t::ToCStr() at line 108
	// inlined string_t::ToCStr() at line 108
	// inlined string_t::ToCStr() at line 110
	// inlined CBaseEntity::SetSolid() at line 117
	// inlined string_t::ToCStr() at line 123
	// inlined string_t::ToCStr() at line 123
	{
		int i;  // line 128
	}
	// inlined CPortalLaser::TurnOn() at line 136
}

// game/server/util.h:105 @0x6b5f90 _ZN14CEntityFactoryI12CPortalLaserE7DestroyEP18IServerNetworkable
void CEntityFactory<CPortalLaser>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:113 @0x6b5fc0 _ZN14CEntityFactoryI12CPortalLaserE13GetEntitySizeEv
size_t CEntityFactory<CPortalLaser>::GetEntitySize()
{
}

// game/server/portal2/env_portal_laser.cpp:143 @0x6b10b0 _ZN12CPortalLaser20CreateHelperEntitiesEv
void CPortalLaser::CreateHelperEntities()
{
	{
		CSoundEnvelopeController &controller;  // line 190
		CPASAttenuationFilter filter;  // line 192
		// inlined CPASAttenuationFilter::~CPASAttenuationFilter() at line 195
		// inlined CBaseEntity::entindex() at line 194
		// inlined CPASAttenuationFilter::CPASAttenuationFilter() at line 192
		// inlined CPASAttenuationFilter::~CPASAttenuationFilter() at line 195
	}
	{
		int i;  // line 146
		// inlined CBeam::SetEndWidth() at line 158
		// inlined CBeam::SetWidth() at line 157
		// inlined CBeam::SetColor() at line 156
		// inlined CBeam::SetWidth() at line 162
		// inlined CBeam::SetEndWidth() at line 163
	}
	// inlined CBaseEntity::GetAbsOrigin() at line 171
	// inlined CBaseEntity::GetAbsAngles() at line 172
	// inlined CBaseEntity::GetAbsOrigin() at line 182
	// inlined CBaseEntity::AddEFlags() at line 183
}

// public/tier1/utlsortvector.h:158 @0x6b60a0 _ZN14CUtlSortVectorI17LaserVictimInfo_t16CLaserVictimLessE12InsertNoSortERKS0_
int CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::InsertNoSort( const $_376 &src )
{
	int lastElement;  // line 161
	// inlined CopyConstruct<LaserVictimInfo_t>() at line 165
	// inlined CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t, int> >::ShiftElementsRight() at line 164
	// inlined CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t, int> >::GrowVector() at line 163
}

// game/server/portal2/env_portal_laser.cpp:202 @0x6b16b0 _ZN12CPortalLaser8ActivateEv
void CPortalLaser::Activate()
{
}

// game/server/portal2/env_portal_laser.cpp:213 @0x6b0810 _ZN12CPortalLaser14UpdateOnRemoveEv
void CPortalLaser::UpdateOnRemove()
{
	{
		int i;  // line 221
	}
}

// game/server/portal2/env_portal_laser.cpp:239 @0x6b01f0 _ZN12CPortalLaser8PrecacheEv
void CPortalLaser::Precache()
{
	// inlined string_t::ToCStr() at line 246
	// inlined string_t::ToCStr() at line 246
	// inlined string_t::ToCStr() at line 248
}

// game/server/portal2/env_portal_laser.cpp:261 @0x6b00d0 _ZN12CPortalLaser14CreateVPhysicsEv
bool CPortalLaser::CreateVPhysics()
{
}

// game/server/portal2/env_portal_laser.cpp:269 @0x6b0350 _ZN12CPortalLaser22FindChildBeamsInSphereERK6VectorfR10CUtlVectorIP5CBeam10CUtlMemoryIS5_iEE
bool CPortalLaser::FindChildBeamsInSphere( const Vector &vecOrigin, float flRadius, CUtlVector<CBeam*,CUtlMemory<CBeam*, int> > &vecBeamsOut )
{
	int iStartCount;  // line 271
	{
		int i;  // line 273
		// inlined Vector::operator-() at line 277
		// inlined CUtlVector<CBeam*,CUtlMemory<CBeam*, int> >::AddToTail() at line 281
		// inlined Vector::operator VectorByValue&() at line 277
	}
	// inlined CUtlVector<CBeam*,CUtlMemory<CBeam*, int> >::Count() at line 271
}

// game/server/portal2/env_portal_laser.cpp:292 (declaration)
void IsOn();

// game/server/portal2/env_portal_laser.cpp:292 @0x6afe40 _ZN12CPortalLaser4IsOnEv
bool CPortalLaser::IsOn()
{
}

// game/server/portal2/env_portal_laser.cpp:301 @0x6b1770 _ZN12CPortalLaser11InputTurnOnER11inputdata_t
void CPortalLaser::InputTurnOn( inputdata_t &inputdata )
{
	// inlined CPortalLaser::TurnOn() at line 305
}

// game/server/portal2/env_portal_laser.cpp:313 @0x6b07f0 _ZN12CPortalLaser12InputTurnOffER11inputdata_t
void CPortalLaser::InputTurnOff( inputdata_t &inputdata )
{
}

// game/server/portal2/env_portal_laser.cpp:325 @0x6b16d0 _ZN12CPortalLaser11InputToggleER11inputdata_t
void CPortalLaser::InputToggle( inputdata_t &inputdata )
{
	// inlined CPortalLaser::TurnOn() at line 333
}

// game/server/portal2/env_portal_laser.cpp:340 @0x6b0640 _ZN12CPortalLaser7TurnOffEv
void CPortalLaser::TurnOff()
{
	// inlined CPortalLaser::HideChildBeams() at line 342
	// inlined CPortalLaser::TurnOffGlow() at line 345
	// inlined CPortalLaser::TurnOffLaserSound() at line 348
}

// game/server/portal2/env_portal_laser.cpp:358 (declaration)
void TurnOn();

// game/server/portal2/env_portal_laser.cpp:358 @0x6b0a90 _ZN12CPortalLaser6TurnOnEv
void CPortalLaser::TurnOn()
{
}

// game/server/portal2/env_portal_laser.cpp:371 @0x6b0cf0 _ZN12CPortalLaser10BeamDamageEP10CGameTrace
void CPortalLaser::BeamDamage( trace_t *ptr )
{
	{
		CBaseEntity *pHit;  // line 375
		{
			Vector dir;  // line 379
			int nDamageType;  // line 381
			CTakeDamageInfo info;  // line 383
			CBaseAnimating *pAnim;  // line 389
			// inlined FClassnameIs() at line 390
			{
				CPhysicsProp *pProp;  // line 412
			}
			// inlined FClassnameIs() at line 390
			// inlined Vector::operator-() at line 379
			// inlined CBaseEntity::GetAbsOrigin() at line 379
			{
				CTakeDamageInfo info;  // line 396
				CNPC_Portal_FloorTurret *pTurret;  // line 399
				// inlined CTakeDamageInfo::SetDamage() at line 397
				// inlined CTakeDamageInfo::SetDamageType() at line 398
			}
		}
	}
}

// game/server/portal2/env_portal_laser.cpp:425 @0x6b0920 _ZN12CPortalLaser10TurnOnGlowEv
void CPortalLaser::TurnOnGlow()
{
	// inlined CSprite::SetAttachment() at line 430
}

// game/server/portal2/env_portal_laser.cpp:444 (declaration)
void TurnOffGlow();

// game/server/portal2/env_portal_laser.cpp:444 @0x6b0000 _ZN12CPortalLaser11TurnOffGlowEv
void CPortalLaser::TurnOffGlow()
{
	// inlined CSprite::FadeAndDie() at line 448
}

// game/server/portal2/env_portal_laser.cpp:458 (declaration)
void TurnOffLaserSound();

// game/server/portal2/env_portal_laser.cpp:458 @0x6affc0 _ZN12CPortalLaser17TurnOffLaserSoundEv
void CPortalLaser::TurnOffLaserSound()
{
	{
		CSoundEnvelopeController &controller;  // line 462
	}
}

// game/server/portal2/env_portal_laser.cpp:474 (declaration)
void FireAtPoint( trace_t &tr, bool bImpact );

// game/server/portal2/env_portal_laser.cpp:474 @0x6b0ff0 _ZN12CPortalLaser11FireAtPointER10CGameTraceb
void CPortalLaser::FireAtPoint( trace_t &tr, bool bImpact )
{
}

// game/server/portal2/env_portal_laser.cpp:488 (declaration)
void HideChildBeams();

// game/server/portal2/env_portal_laser.cpp:488 @0x6aff10 _ZN12CPortalLaser14HideChildBeamsEv
void CPortalLaser::HideChildBeams()
{
	{
		int i;  // line 490
	}
}

// game/server/portal2/env_portal_laser.cpp:503 @0x6b0b30 _ZN12CPortalLaser19UpdateSoundPositionERK6VectorS2_
void CPortalLaser::UpdateSoundPosition( const Vector &vecStart, const Vector &vecEnd )
{
	CBasePlayer *pPlayer;  // line 506
	{
		Vector vecPlayer;  // line 509
		Vector vecNearestPoint;  // line 510
		float flDistToPlayerSqr;  // line 513
		float flNearestToPlayerSqr;  // line 514
		// inlined Vector::operator-() at line 513
		// inlined Vector::operator-() at line 514
		// inlined Vector::operator==() at line 517
		// inlined Vector::operator=() at line 519
	}
	// inlined CBaseEntity::GetAbsOrigin() at line 524
	// inlined Vector::operator=() at line 524
}

// game/server/portal2/env_portal_laser.cpp:531 (declaration)
void DamageEntity( CBaseEntity *pVictim, float flAmount );

// game/server/portal2/env_portal_laser.cpp:531 @0x6afe60 _ZN12CPortalLaser12DamageEntityEP11CBaseEntityf
void CPortalLaser::DamageEntity( CBaseEntity *pVictim, float flAmount )
{
	CTakeDamageInfo info;  // line 533
	// inlined Vector::Vector() at line 534
	// inlined Vector::operator VectorByValue&() at line 534
}

// game/server/portal2/env_portal_laser.cpp:539 sizeof=0x8 (i386)
struct $_376
{
public:
	CBaseEntity *pVictim; // +0x0  // line 540
	float flFraction; // +0x4  // line 541
};

// game/server/portal2/env_portal_laser.cpp:542
typedef $_376 LaserVictimInfo_t;

// game/server/portal2/env_portal_laser.cpp:545 sizeof=0x1 (i386)
struct CLaserVictimLess
{
public:
	bool Less( const $_376 &, const $_376 &, void * );  // line 547
};

// game/server/portal2/env_portal_laser.cpp:558 @0x6b19c0 _ZN12CPortalLaser24StrikeEntitiesAlongLaserERK6VectorS2_PS0_
bool CPortalLaser::StrikeEntitiesAlongLaser( const Vector &vecStart, const Vector &vecEnd, Vector *pVecOut )
{
	Vector vecDirection;  // line 562
	float flLaserWidth;  // line 565
	Vector vecMins;  // line 570
	Vector vecMaxs;  // line 571
	CBaseEntity *list[512];  // line 574
	Ray_t ray;  // line 575
	int nCount;  // line 577
	CBaseEntity *pEntity;  // line 578
	CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess> vsrtVictims;  // line 580
	bool bBestIsTurret;  // line 649
	// inlined CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::~CUtlSortVector() at line 745
	// inlined Vector::operator=() at line 742
	// inlined CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::~CUtlSortVector() at line 745
	// inlined CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::RedoSort() at line 647
	{
		int i;  // line 652
		{
			CPortalLaserTarget *pLaserTarget;  // line 660
			// inlined Vector::operator=() at line 665
			// inlined CPortalLaser::DamageEntity() at line 659
		}
		// inlined FClassnameIs() at line 657
		{
			Vector vecPlayerVelocity;  // line 679
			Vector vecNearestPoint;  // line 683
			Vector vecPlayerPos;  // line 687
			Vector vecLineToLaser;  // line 691
			Vector vecBounce;  // line 695
			Vector vecPushVelocity;  // line 720
			// inlined CBaseEntity::GetAbsVelocity() at line 679
			// inlined CBaseEntity::GetAbsOrigin() at line 684
			// inlined CBaseEntity::GetAbsOrigin() at line 687
			// inlined Vector::operator-() at line 691
			// inlined Vector::LengthSqr() at line 699
			// inlined Vector::Vector() at line 701
			// inlined CrossProduct() at line 701
			// inlined Vector::operator=() at line 701
			// inlined Vector::operator*() at line 720
			// inlined Vector::operator*=() at line 727
			// inlined CPortalLaser::DamageEntity() at line 733
			{
				float flDot;  // line 706
				// inlined DotProduct() at line 706
				// inlined operator*() at line 709
				// inlined Vector::operator*() at line 709
				// inlined Vector::operator+() at line 709
				// inlined Vector::operator=() at line 709
				// inlined DotProduct() at line 715
			}
		}
		// inlined FClassnameIs() at line 672
	}
	{
		int i;  // line 583
		{
			Vector vecNearest;  // line 597
			float flFraction;  // line 598
			float flNearestDistSqr;  // line 601
			// inlined FClassnameIs() at line 589
			{
				bool bIsJumping;  // line 617
				CPortal_Player *pPlayer;  // line 627
				// inlined CBaseEntity::GetAbsOrigin() at line 628
				// inlined ToPortalPlayer() at line 627
				{
					float flZDist;  // line 621
				}
			}
			{
				LaserVictimInfo_t victim;  // line 641
			}
			// inlined FClassnameIs() at line 589
			// inlined CBaseEntity::GetAbsOrigin() at line 599
			// inlined CBaseEntity::GetAbsOrigin() at line 601
			// inlined Vector::operator-() at line 601
			// inlined Vector::Length2DSqr() at line 601
		}
	}
	// inlined CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::CUtlSortVector() at line 580
	// inlined UTIL_EntitiesAlongRay() at line 577
	// inlined Ray_t::Init() at line 576
	// inlined Vector::Length() at line 566
	// inlined Vector::operator-() at line 562
}

// game/server/portal2/env_portal_laser.cpp:752 @0x6b27a0 _ZN12CPortalLaser27NewStrikeEntitiesAlongLaserERK6VectorS2_PS0_b
CBaseEntity *CPortalLaser::NewStrikeEntitiesAlongLaser( const Vector &vecStart, const Vector &vecEnd, Vector *pVecOut, bool bIsFirstTrace )
{
	Vector vecDirection;  // line 756
	float flLaserWidth;  // line 759
	Vector vecMins;  // line 764
	Vector vecMaxs;  // line 765
	CBaseEntity *list[512];  // line 768
	Ray_t ray;  // line 769
	int nCount;  // line 771
	CBaseEntity *pEntity;  // line 772
	CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess> vsrtVictims;  // line 774
	bool bBlockTarget;  // line 849
	// inlined CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::~CUtlSortVector() at line 958
	// inlined CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::~CUtlSortVector() at line 958
	// inlined CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::RedoSort() at line 847
	{
		int i;  // line 852
		{
			Vector vecPlayerVelocity;  // line 892
			Vector vecNearestPoint;  // line 896
			Vector vecPlayerPos;  // line 900
			Vector vecLineToLaser;  // line 904
			Vector vecBounce;  // line 908
			Vector vecPushVelocity;  // line 933
			// inlined CPortalLaser::DamageEntity() at line 946
			// inlined Vector::operator*=() at line 940
			// inlined Vector::operator*() at line 933
			// inlined Vector::operator=() at line 914
			// inlined CrossProduct() at line 914
			// inlined Vector::Vector() at line 914
			// inlined Vector::LengthSqr() at line 912
			// inlined Vector::operator-() at line 904
			// inlined CBaseEntity::GetAbsOrigin() at line 900
			// inlined CBaseEntity::GetAbsOrigin() at line 897
			// inlined CBaseEntity::GetAbsVelocity() at line 892
			{
				CBaseEntity *pNextVictim;  // line 884
				// inlined CPortalLaser::ShouldAutoAim() at line 885
			}
			{
				float flDot;  // line 919
				// inlined DotProduct() at line 919
				// inlined operator*() at line 922
				// inlined Vector::operator*() at line 922
				// inlined Vector::operator+() at line 922
				// inlined Vector::operator=() at line 922
				// inlined DotProduct() at line 928
			}
		}
		// inlined FClassnameIs() at line 875
		// inlined FClassnameIs() at line 857
		{
			CPortalLaserTarget *pLaserTarget;  // line 859
			// inlined CPortalLaser::DamageEntity() at line 872
			// inlined Vector::operator=() at line 864
		}
	}
	{
		int i;  // line 777
		{
			Vector vecNearest;  // line 791
			float flFraction;  // line 792
			float flNearestDistSqr;  // line 795
			// inlined Vector::Length2DSqr() at line 795
			// inlined Vector::operator-() at line 795
			// inlined CBaseEntity::GetAbsOrigin() at line 795
			// inlined CBaseEntity::GetAbsOrigin() at line 793
			// inlined FClassnameIs() at line 783
			{
				LaserVictimInfo_t victim;  // line 841
			}
			{
				bool bIsJumping;  // line 811
				CPortal_Player *pPlayer;  // line 821
				{
					float flZDist;  // line 815
				}
				// inlined ToPortalPlayer() at line 821
				// inlined CBaseEntity::GetAbsOrigin() at line 822
				// inlined CBaseEntity::GetAbsOrigin() at line 828
			}
			// inlined FClassnameIs() at line 783
		}
	}
	// inlined CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::CUtlSortVector() at line 774
	// inlined UTIL_EntitiesAlongRay() at line 771
	// inlined Ray_t::Init() at line 770
	// inlined Vector::Length() at line 760
	// inlined Vector::operator-() at line 756
	// inlined Vector::operator=() at line 955
}

// game/server/portal2/env_portal_laser.cpp:962 (declaration)
void ShouldAutoAim( CBaseEntity *pEntity );

// game/server/portal2/env_portal_laser.cpp:962 @0x6b1060 _ZN12CPortalLaser13ShouldAutoAimEP11CBaseEntity
bool CPortalLaser::ShouldAutoAim( CBaseEntity *pEntity )
{
	// inlined FClassnameIs() at line 967
}

// game/server/portal2/env_portal_laser.cpp:976 @0x6b3650 _ZN12CPortalLaser10TraceLaserEbRK6VectorS2_RfR10CGameTracePS0_
CBaseEntity *CPortalLaser::TraceLaser( bool bIsFirstTrace, const Vector &vecStart, const Vector &vecDirection, float &flTotalBeamLength, trace_t &tr, Vector *pVecAutoAimOffset )
{
	Vector vStart;  // line 980
	Vector vDir;  // line 981
	CTraceFilterSimpleClassnameList traceFilter;  // line 983
	CUtlVector<CPortal_Base2D*,CUtlMemory<CPortal_Base2D*, int> > portalList;  // line 988
	CBaseEntity *pHitEntity;  // line 990
	bool bGoingThroughPortal;  // line 991
	// inlined CUtlVector<CPortal_Base2D*,CUtlMemory<CPortal_Base2D*, int> >::~CUtlVector() at line 1098
	{
		Vector vAutoAimOffset;  // line 1088
		{
			int i;  // line 1089
			{
				CPortal_Base2D *pPortal;  // line 1091
			}
		}
		// inlined Vector::operator=() at line 1095
	}
	// inlined CPortalLaser::ShouldAutoAim() at line 1086
	{
		Vector vPointOnLine;  // line 1077
		// inlined Vector::operator-() at line 1081
		// inlined Vector::operator=() at line 1081
	}
	// inlined CPortalLaser::ShouldAutoAim() at line 1071
	// inlined CTraceFilterSimpleClassnameList::~CTraceFilterSimpleClassnameList() at line 1098
	{
		Ray_t ray;  // line 995
		CPortal_Base2D *pFirstPortal;  // line 1014
		Vector vStrike;  // line 1043
		{
			Vector vPointOnLine;  // line 1047
			// inlined Vector::operator-() at line 1050
			// inlined Vector::Length() at line 1050
			// inlined Vector::operator-() at line 1054
			// inlined Vector::operator=() at line 1054
		}
		{
			Vector vStrike;  // line 1018
			Ray_t rayTransformed;  // line 1036
			// inlined Vector::operator=() at line 1038
			// inlined Ray_t::Ray_t() at line 1036
			{
				Vector vPointOnLine;  // line 1022
				// inlined Vector::operator-() at line 1025
				// inlined Vector::Length() at line 1025
				// inlined Vector::operator-() at line 1029
				// inlined Vector::operator=() at line 1029
			}
		}
		// inlined CPortal_Base2D::GetLinkedPortal() at line 1067
		// inlined Vector::Length() at line 1061
		// inlined Vector::operator-() at line 1061
		// inlined CUtlVector<CPortal_Base2D*,CUtlMemory<CPortal_Base2D*, int> >::AddToHead() at line 1067
		// inlined Vector::operator*() at line 996
		// inlined Vector::operator+() at line 996
		// inlined Ray_t::Init() at line 996
		// inlined UTIL_TraceRay() at line 998
	}
	// inlined CHandle<CBaseEntity>::operator CBaseEntity*() at line 1073
	// inlined CTraceFilterSimpleClassnameList::~CTraceFilterSimpleClassnameList() at line 1098
	// inlined CUtlVector<CPortal_Base2D*,CUtlMemory<CPortal_Base2D*, int> >::~CUtlVector() at line 1098
}

// game/server/portal2/env_portal_laser.cpp:1105 @0x6b42b0 _ZN12CPortalLaser9FireLaserEiRK6VectorS2_P11CBaseEntity
void CPortalLaser::FireLaser( int nNumBeams, const Vector &vecStart, const Vector &vecDirection, CBaseEntity *pParent )
{
	{
		float flTotalBeamLength;  // line 1119
		trace_t tr;  // line 1120
		Vector vAutoAimOffset;  // line 1121
		Vector vDir;  // line 1122
		CBaseEntity *pHitTarget;  // line 1124
		bool bReflected;  // line 1196
		surfacedata_t *pSurfaceData;  // line 1198
		{
			CBaseEntity *pEnt;  // line 1163
			// inlined CHandle<CBaseEntity>::operator CBaseEntity*() at line 1163
			{
				Vector vecForward;  // line 1167
				Vector vecOffset;  // line 1170
				CPropWeightedCube *pCube;  // line 1189
				// inlined CPropWeightedCube::SetLaser() at line 1190
				{
					CPortalSimulator *pSimulator;  // line 1174
					// inlined VMatrix::operator*() at line 1180
					// inlined Vector::operator=() at line 1180
					// inlined VMatrix::ApplyRotation() at line 1181
					// inlined Vector::operator=() at line 1181
				}
				// inlined Vector::operator+() at line 1170
				// inlined Vector::operator*() at line 1170
				// inlined CBaseEntity::GetAbsAngles() at line 1168
			}
		}
		// inlined FClassnameIs() at line 1156
		// inlined Vector::operator VectorByValue&() at line 1153
		// inlined Vector::operator+() at line 1153
		// inlined Vector::operator*() at line 1153
		{
			trace_t tempTrace;  // line 1127
			Vector vNewEndPoint;  // line 1128
			Vector vNewDirection;  // line 1129
			float flNewBeamLength;  // line 1130
			Vector vSecondAutoAimOffset;  // line 1134
			CBaseEntity *pSecondHitTarget;  // line 1135
			// inlined Vector::operator*() at line 1128
			// inlined Vector::operator+() at line 1128
			// inlined Vector::operator+() at line 1128
			// inlined Vector::operator-() at line 1129
			// inlined Vector::Normalized() at line 1129
			// inlined CPortalLaser::DamageEntity() at line 1149
			// inlined CGameTrace::operator=() at line 1144
			// inlined Vector::operator=() at line 1146
		}
		// inlined CPortalLaser::ShouldAutoAim() at line 1125
		// inlined CPortalLaser::FireAtPoint() at line 1224
		{
			Vector vecDir;  // line 1201
			Vector vecBounce;  // line 1205
			// inlined Vector::operator-() at line 1201
			// inlined DotProduct() at line 1205
			// inlined operator*() at line 1205
			// inlined Vector::operator*() at line 1205
			// inlined Vector::operator+() at line 1205
		}
	}
	{
		CTraceFilterSimpleClassnameList traceFilter;  // line 1228
		Ray_t ray;  // line 1233
		trace_t tr;  // line 1236
		CPortal_Base2D *pFirstPortal;  // line 1243
		Vector vecNewTermPoint;  // line 1284
		bool bReflected;  // line 1337
		surfacedata_t *pSurfaceData;  // line 1339
		// inlined Vector::operator*() at line 1234
		// inlined Vector::operator+() at line 1234
		// inlined Ray_t::Init() at line 1234
		// inlined UTIL_TraceRay() at line 1238
		// inlined CBeam::SetEndAttachment() at line 1297
		{
			CBaseEntity *pEnt;  // line 1307
			{
				Vector vecForward;  // line 1311
				Vector vecOffset;  // line 1314
				{
					CPortalSimulator *pSimulator;  // line 1318
					// inlined VMatrix::operator*() at line 1324
					// inlined Vector::operator=() at line 1324
					// inlined VMatrix::ApplyRotation() at line 1325
					// inlined Vector::operator=() at line 1325
				}
				// inlined Vector::operator+() at line 1314
				// inlined Vector::operator*() at line 1314
				// inlined CBaseEntity::GetAbsAngles() at line 1312
			}
			// inlined CHandle<CBaseEntity>::operator CBaseEntity*() at line 1307
		}
		// inlined CTraceFilterSimpleClassnameList::~CTraceFilterSimpleClassnameList() at line 1365
		// inlined CTraceFilterSimpleClassnameList::~CTraceFilterSimpleClassnameList() at line 1365
		// inlined CPortalLaser::FireAtPoint() at line 1365
		{
			Vector vecNewTermPoint;  // line 1250
			Ray_t rayTransformed;  // line 1269
			Vector vecDirection;  // line 1271
			Vector vecStartPos;  // line 1274
			// inlined CPortalLaser::FireAtPoint() at line 1266
			// inlined Ray_t::Ray_t() at line 1269
		}
		{
			Vector vecDir;  // line 1342
			Vector vecBounce;  // line 1346
			// inlined Vector::operator-() at line 1342
			// inlined DotProduct() at line 1346
			// inlined operator*() at line 1346
			// inlined Vector::operator*() at line 1346
			// inlined Vector::operator+() at line 1346
		}
		// inlined CTraceFilterSimpleClassnameList::~CTraceFilterSimpleClassnameList() at line 1365
	}
}

// game/server/portal2/env_portal_laser.cpp:1399 @0x6b5c60 _ZN12CPortalLaser11StrikeThinkEv
void CPortalLaser::StrikeThink()
{
	CVProfScope VProf_;  // line 1401
	Vector vecDir;  // line 1420
	Vector vecOrigin;  // line 1421
	// inlined CVProfScope::CVProfScope() at line 1401
	// inlined CPortalLaser::HideChildBeams() at line 1414
	// inlined Vector::operator=() at line 1417
	// inlined CVProfScope::~CVProfScope() at line 1428
	// inlined CVProfScope::~CVProfScope() at line 1428
}

// game/server/portal2/env_portal_laser.cpp:1432 @0x6b05d0 _Z18FindLasersInSphereRK6VectorfR10CUtlVectorIP5CBeam10CUtlMemoryIS4_iEE
bool FindLasersInSphere( const Vector &vecOrigin, float flRadius, CUtlVector<CBeam*,CUtlMemory<CBeam*, int> > &vecBeamsOut )
{
	int iStartCount;  // line 1434
	// inlined CUtlVector<CBeam*,CUtlMemory<CBeam*, int> >::Count() at line 1434
	{
		CPortalLaser *pLaser;  // line 1435
		// inlined GetEnvPortalLaserList() at line 1435
	}
}
