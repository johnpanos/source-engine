// DWARF declaration skeleton for game/server/portal2/env_portal_laser.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xa8aa0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined ServerClass::ServerClass() at line 46
	// inlined CEntityFactory<CPortalLaser>::CEntityFactory() at line 52
}

// game/server/portal2/env_portal_laser.cpp:32
ConVar sv_player_collide_with_laser;

// game/server/portal2/env_portal_laser.cpp:35
ConVar new_portal_laser;

// game/server/portal2/env_portal_laser.cpp:36
ConVar portal_laser_normal_update;

// game/server/portal2/env_portal_laser.cpp:37
ConVar portal_laser_high_precision_update;

// game/server/portal2/env_portal_laser.cpp:38
ConVar sv_debug_laser;

// game/server/portal2/env_portal_laser.cpp:39
ConVar sv_laser_cube_autoaim;

// game/server/portal2/env_portal_laser.cpp:42
const char *g_pLaserGlowSpriteName;

// game/server/portal2/env_portal_laser.cpp:44
int g_nTotalLaser;

// game/server/portal2/env_portal_laser.cpp:46 @0x709020 _ZN12CPortalLaser14GetServerClassEv
ServerClass *CPortalLaser::GetServerClass()
{
}

// game/server/portal2/env_portal_laser.cpp:46 @0x709030 _ZN12CPortalLaser40YouForgotToImplementOrDeclareServerClassEv
int CPortalLaser::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/env_portal_laser.cpp:46 @0xa8600 _Z15ServerClassInitIN14DT_PortalLaser7ignoredEEiPT_
int ServerClassInit<DT_PortalLaser::ignored>( DT_PortalLaser::ignored * )
{
	SendTable &sendTable;  // line 46
	char *const g_pSendTableName;  // line 46
	SendProp g_SendProps[5];  // line 46
}

// game/server/portal2/env_portal_laser.cpp:46
SendTable g_SendTable;

// game/server/portal2/env_portal_laser.cpp:46
int g_SendTableInit;

// game/server/portal2/env_portal_laser.cpp:46
static ServerClass g_CPortalLaser_ClassReg;

// game/server/portal2/env_portal_laser.cpp:46
void CPortalLaser::m_pClassSendTable;

// game/server/portal2/env_portal_laser.cpp:52
static CEntityFactory<CPortalLaser> env_portal_laser;

// game/server/portal2/env_portal_laser.cpp:54 (declaration)
void GetBaseMap();

// game/server/portal2/env_portal_laser.cpp:54 @0x709040 _ZN12CPortalLaser14GetDataDescMapEv
datamap_t *CPortalLaser::GetDataDescMap()
{
}

// game/server/portal2/env_portal_laser.cpp:54 @0x709050 _ZN12CPortalLaser10GetBaseMapEv
datamap_t *CPortalLaser::GetBaseMap()
{
}

// game/server/portal2/env_portal_laser.cpp:54 @0xa8890 _Z11DataMapInitI12CPortalLaserEP9datamap_tPT_
datamap_t *DataMapInit<CPortalLaser>( CPortalLaser * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 80
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 54
	CDatadescGeneratedNameHolder nameHolder;  // line 54
	typedescription_t dataDesc[16];  // line 54
}

// game/server/portal2/env_portal_laser.cpp:54
datamap_t *g_DataMapHolder;

// game/server/portal2/env_portal_laser.cpp:54
void CPortalLaser::m_DataMap;

// game/server/portal2/env_portal_laser.cpp:82
static const char LASER_LOOPING_SOUND[15];

// game/server/portal2/env_portal_laser.cpp:83
static const char LETHAL_LASER_LOOPING_SOUND[20];

// game/server/portal2/env_portal_laser.cpp:84
static const char LASER_BURN_SOUND[19];

// game/server/portal2/env_portal_laser.cpp:91 (declaration)
void CPortalLaser();

// game/server/portal2/env_portal_laser.cpp:91 @0x7098d0 _ZN12CPortalLaserC2Ev
CPortalLaser::CPortalLaser()
{
	// inlined CNetworkVarBase<bool,CPortalLaser::NetworkVar_m_bShouldSpark>::CNetworkVarBase() at line 98
	// inlined CNetworkVarBase<Vector,CPortalLaser::NetworkVar_m_vecLaserImpactNormal>::CNetworkVarBase() at line 98
	// inlined CNetworkVarBase<Vector,CPortalLaser::NetworkVar_m_vecLaserEndPos>::CNetworkVarBase() at line 98
	// inlined CHandle<CBaseEntity>::CHandle() at line 98
}

// game/server/portal2/env_portal_laser.cpp:91 @0x709d40 _ZN12CPortalLaserC1Ev
CPortalLaser::CPortalLaser()
{
}

// public/tier1/utlsortvector.h:99 @0x711e00 _ZN14CUtlSortVectorI17LaserVictimInfo_t16CLaserVictimLessE13CompareHelperEPKS0_S4_
int CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::CompareHelper( const $_375 *lhs, const $_375 *rhs )
{
	CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::QSortContext_t *ctx;  // line 101
	// inlined CLaserVictimLess::Less() at line 102
}

// game/server/util.h:100 @0x712100 _ZN14CEntityFactoryI12CPortalLaserE6CreateEPKc
IServerNetworkable *CEntityFactory<CPortalLaser>::Create( const char *pClassName )
{
	CPortalLaser *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CPortalLaser>() at line 102
}

// game/server/portal2/env_portal_laser.cpp:103 (declaration)
~CPortalLaser();

// game/server/portal2/env_portal_laser.cpp:103 @0x709320 _ZN12CPortalLaserD0Ev
CPortalLaser::~CPortalLaser()
{
}

// game/server/portal2/env_portal_laser.cpp:103 @0x709360 _ZN12CPortalLaserD1Ev
CPortalLaser::~CPortalLaser()
{
}

// game/server/portal2/env_portal_laser.cpp:103 @0x709390 _ZN12CPortalLaserD2Ev
CPortalLaser::~CPortalLaser()
{
}

// game/server/util.h:106 @0x711dc0 _ZN14CEntityFactoryI12CPortalLaserE7DestroyEP18IServerNetworkable
void CEntityFactory<CPortalLaser>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/portal2/env_portal_laser.cpp:113 @0x70df60 _ZN12CPortalLaser5SpawnEv
void CPortalLaser::Spawn()
{
	// inlined string_t::ToCStr() at line 121
	// inlined string_t::ToCStr() at line 121
	// inlined string_t::ToCStr() at line 123
	// inlined CBaseEntity::SetSolid() at line 129
	// inlined string_t::ToCStr() at line 135
	// inlined string_t::ToCStr() at line 135
	{
		int i;  // line 141
	}
	// inlined CPortalLaser::TurnOn() at line 151
}

// game/server/util.h:114 @0x711df0 _ZN14CEntityFactoryI12CPortalLaserE13GetEntitySizeEv
size_t CEntityFactory<CPortalLaser>::GetEntitySize()
{
}

// game/server/portal2/env_portal_laser.cpp:156 @0x709300 _ZN12CPortalLaser19UpdateTransmitStateEv
int CPortalLaser::UpdateTransmitState()
{
}

// game/server/portal2/env_portal_laser.cpp:165 @0x70d030 _ZN12CPortalLaser20CreateHelperEntitiesEv
void CPortalLaser::CreateHelperEntities()
{
	// inlined CBeam::SetColor() at line 172
	// inlined CBeam::SetWidth() at line 173
	// inlined CBeam::SetEndWidth() at line 174
	// inlined CBaseEntity::GetAbsOrigin() at line 189
	// inlined CBaseEntity::GetAbsAngles() at line 190
	// inlined CBeam::SetWidth() at line 178
	// inlined CBeam::SetEndWidth() at line 179
}

// game/server/portal2/env_portal_laser.cpp:199 @0x709670 _ZN12CPortalLaser18CreateSoundProxiesEv
void CPortalLaser::CreateSoundProxies()
{
	CSoundEnvelopeController &controller;  // line 202
	{
		int i;  // line 204
		{
			CBasePlayer *pPlayer;  // line 206
			// inlined CBaseEntity::AddEFlags() at line 222
			// inlined CBaseEntity::GetAbsOrigin() at line 221
			{
				CSingleUserRecipientFilter filter;  // line 228
				// inlined CBaseEntity::entindex() at line 235
				// inlined CBaseEntity::entindex() at line 231
				// inlined CSingleUserRecipientFilter::CSingleUserRecipientFilter() at line 228
				// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 238
				// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 238
			}
		}
	}
}

// game/server/portal2/env_portal_laser.cpp:246 @0x70df40 _ZN12CPortalLaser8ActivateEv
void CPortalLaser::Activate()
{
}

// game/server/portal2/env_portal_laser.cpp:257 @0x711d70 _ZN12CPortalLaser14UpdateOnRemoveEv
void CPortalLaser::UpdateOnRemove()
{
}

// game/server/portal2/env_portal_laser.cpp:277 @0x7093c0 _ZN12CPortalLaser8PrecacheEv
void CPortalLaser::Precache()
{
	// inlined string_t::ToCStr() at line 294
	// inlined string_t::ToCStr() at line 294
	// inlined string_t::ToCStr() at line 296
}

// game/server/portal2/env_portal_laser.cpp:310 @0x7092e0 _ZN12CPortalLaser14CreateVPhysicsEv
bool CPortalLaser::CreateVPhysics()
{
}

// game/server/portal2/env_portal_laser.cpp:320 (declaration)
void IsOn();

// game/server/portal2/env_portal_laser.cpp:320 @0x709060 _ZN12CPortalLaser4IsOnEv
bool CPortalLaser::IsOn()
{
}

// game/server/portal2/env_portal_laser.cpp:329 @0x70a5e0 _ZN12CPortalLaser11InputTurnOnER11inputdata_t
void CPortalLaser::InputTurnOn( inputdata_t &inputdata )
{
	// inlined CPortalLaser::TurnOn() at line 333
}

// game/server/portal2/env_portal_laser.cpp:341 @0x711d50 _ZN12CPortalLaser12InputTurnOffER11inputdata_t
void CPortalLaser::InputTurnOff( inputdata_t &inputdata )
{
}

// game/server/portal2/env_portal_laser.cpp:353 @0x711ca0 _ZN12CPortalLaser11InputToggleER11inputdata_t
void CPortalLaser::InputToggle( inputdata_t &inputdata )
{
	// inlined CPortalLaser::TurnOn() at line 361
}

// game/server/portal2/env_portal_laser.cpp:368 @0x7119c0 _ZN12CPortalLaser7TurnOffEv
void CPortalLaser::TurnOff()
{
	// inlined CNetworkVarBase<bool,CPortalLaser::NetworkVar_m_bShouldSpark>::operator=<bool>() at line 371
	// inlined CPortalLaser::RemoveChildLaser() at line 373
	// inlined CPortalLaser::HideBeam() at line 375
	// inlined CPortalLaser::TurnOffGlow() at line 378
	// inlined CPortalLaser::TurnOffLaserSound() at line 381
}

// game/server/portal2/env_portal_laser.cpp:391 (declaration)
void TurnOn();

// game/server/portal2/env_portal_laser.cpp:391 @0x70a180 _ZN12CPortalLaser6TurnOnEv
void CPortalLaser::TurnOn()
{
	{
		float flNextUpdate;  // line 396
		// inlined ConVar::GetFloat() at line 396
		// inlined ConVar::GetFloat() at line 396
	}
}

// game/server/portal2/env_portal_laser.cpp:405 @0x70a230 _ZN12CPortalLaser10BeamDamageEP10CGameTrace
void CPortalLaser::BeamDamage( trace_t *ptr )
{
	{
		CBaseEntity *pHit;  // line 409
		{
			Vector dir;  // line 413
			int nDamageType;  // line 415
			CTakeDamageInfo info;  // line 417
			CBaseAnimating *pAnim;  // line 423
			// inlined FClassnameIs() at line 424
			{
				CPhysicsProp *pProp;  // line 446
			}
			// inlined FClassnameIs() at line 424
			// inlined Vector::operator-() at line 413
			// inlined CBaseEntity::GetAbsOrigin() at line 413
			{
				CTakeDamageInfo info;  // line 430
				CNPC_Portal_FloorTurret *pTurret;  // line 433
				// inlined CTakeDamageInfo::SetDamage() at line 431
				// inlined CTakeDamageInfo::SetDamageType() at line 432
			}
		}
	}
}

// game/server/portal2/env_portal_laser.cpp:459 @0x709d50 _ZN12CPortalLaser10TurnOnGlowEv
void CPortalLaser::TurnOnGlow()
{
	// inlined CSprite::SetAttachment() at line 467
}

// game/server/portal2/env_portal_laser.cpp:481 (declaration)
void TurnOffGlow();

// game/server/portal2/env_portal_laser.cpp:481 @0x709210 _ZN12CPortalLaser11TurnOffGlowEv
void CPortalLaser::TurnOffGlow()
{
	// inlined CSprite::FadeAndDie() at line 485
}

// game/server/portal2/env_portal_laser.cpp:495 (declaration)
void TurnOffLaserSound();

// game/server/portal2/env_portal_laser.cpp:495 @0x7091c0 _ZN12CPortalLaser17TurnOffLaserSoundEv
void CPortalLaser::TurnOffLaserSound()
{
	{
		int i;  // line 497
		{
			CSoundEnvelopeController &controller;  // line 501
		}
	}
}

// game/server/portal2/env_portal_laser.cpp:514 @0x70eb30 _ZN12CPortalLaser11FireAtPointER10CGameTraceb
void CPortalLaser::FireAtPoint( trace_t &tr, bool bImpact )
{
	// inlined CNetworkVarBase<bool,CPortalLaser::NetworkVar_m_bShouldSpark>::operator=<bool>() at line 520
	// inlined CNetworkVarBase<Vector,CPortalLaser::NetworkVar_m_vecLaserEndPos>::operator=<Vector>() at line 523
	// inlined Vector::Normalized() at line 524
	// inlined CNetworkVarBase<Vector,CPortalLaser::NetworkVar_m_vecLaserImpactNormal>::operator=<Vector>() at line 524
	// inlined CNetworkVarBase<bool,CPortalLaser::NetworkVar_m_bShouldSpark>::operator=<bool>() at line 528
}

// game/server/portal2/env_portal_laser.cpp:536 (declaration)
void HideBeam();

// game/server/portal2/env_portal_laser.cpp:536 @0x709190 _ZN12CPortalLaser8HideBeamEv
void CPortalLaser::HideBeam()
{
}

// game/server/portal2/env_portal_laser.cpp:543 (declaration)
void ShowBeam();

// game/server/portal2/env_portal_laser.cpp:543 @0x709160 _ZN12CPortalLaser8ShowBeamEv
void CPortalLaser::ShowBeam()
{
}

// public/tier1/utlvector.h:543 @0x711f50 _ZN10CUtlVectorIN12CPortalLaser17PortalLaserInfo_tE10CUtlMemoryIS1_iEED1Ev
CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t, int> >::~CUtlVector()
{
	// inlined CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t, int> >::Purge() at line 545
	// inlined CUtlMemory<CPortalLaser::PortalLaserInfo_t,int>::~CUtlMemory() at line 545
	// inlined CUtlMemory<CPortalLaser::PortalLaserInfo_t,int>::~CUtlMemory() at line 545
}

// game/server/portal2/env_portal_laser.cpp:553 @0x709480 _ZN12CPortalLaser19UpdateSoundPositionERK6VectorS2_
void CPortalLaser::UpdateSoundPosition( const Vector &vecStart, const Vector &vecEnd )
{
	{
		int i;  // line 555
		{
			CBasePlayer *pPlayer;  // line 557
			Vector vecPlayer;  // line 571
			Vector vecNearestPoint;  // line 572
			float flDistToPlayerSqr;  // line 575
			float flNearestToPlayerSqr;  // line 576
			// inlined Vector::operator==() at line 579
			// inlined Vector::operator-() at line 576
			// inlined Vector::operator-() at line 575
			// inlined CBaseEntity::GetAbsOrigin() at line 567
			// inlined Vector::operator=() at line 567
			// inlined Vector::operator=() at line 581
		}
	}
}

// game/server/portal2/env_portal_laser.cpp:589 (declaration)
void DamageEntity( CBaseEntity *pVictim, float flAmount );

// game/server/portal2/env_portal_laser.cpp:589 @0x7090b0 _ZN12CPortalLaser12DamageEntityEP11CBaseEntityf
void CPortalLaser::DamageEntity( CBaseEntity *pVictim, float flAmount )
{
	CTakeDamageInfo info;  // line 591
	// inlined Vector::Vector() at line 592
	// inlined Vector::operator VectorByValue&() at line 592
}

// game/server/portal2/env_portal_laser.cpp:597 sizeof=0x8 (i386)
struct $_375
{
public:
	CBaseEntity *pVictim; // +0x0  // line 598
	float flFraction; // +0x4  // line 599
};

// game/server/portal2/env_portal_laser.cpp:600
typedef $_375 LaserVictimInfo_t;

// game/server/portal2/env_portal_laser.cpp:603 sizeof=0x1 (i386)
struct CLaserVictimLess
{
public:
	bool Less( const $_375 &, const $_375 &, void * );  // line 605
};

// game/server/portal2/env_portal_laser.cpp:616 @0x70c1c0 _ZN12CPortalLaser24StrikeEntitiesAlongLaserERK6VectorS2_PS0_
bool CPortalLaser::StrikeEntitiesAlongLaser( const Vector &vecStart, const Vector &vecEnd, Vector *pVecOut )
{
	Vector vecDirection;  // line 620
	float flLaserWidth;  // line 623
	Vector vecMins;  // line 628
	Vector vecMaxs;  // line 629
	CBaseEntity *list[512];  // line 632
	Ray_t ray;  // line 633
	int nCount;  // line 635
	CBaseEntity *pEntity;  // line 636
	CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess> vsrtVictims;  // line 638
	bool bBestIsTurret;  // line 707
	// inlined CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::~CUtlSortVector() at line 803
	// inlined Vector::operator=() at line 800
	// inlined CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::~CUtlSortVector() at line 803
	// inlined CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::RedoSort() at line 705
	{
		int i;  // line 710
		// inlined FClassnameIs() at line 730
		{
			Vector vecPlayerVelocity;  // line 737
			Vector vecNearestPoint;  // line 741
			Vector vecPlayerPos;  // line 745
			Vector vecLineToLaser;  // line 749
			Vector vecBounce;  // line 753
			Vector vecPushVelocity;  // line 778
			// inlined CBaseEntity::GetAbsVelocity() at line 737
			// inlined CBaseEntity::GetAbsOrigin() at line 742
			// inlined CBaseEntity::GetAbsOrigin() at line 745
			// inlined Vector::operator-() at line 749
			// inlined Vector::LengthSqr() at line 757
			// inlined Vector::Vector() at line 759
			// inlined CrossProduct() at line 759
			// inlined Vector::operator=() at line 759
			// inlined Vector::operator*() at line 778
			// inlined Vector::operator*=() at line 785
			// inlined CPortalLaser::DamageEntity() at line 791
			{
				float flDot;  // line 764
				// inlined DotProduct() at line 764
				// inlined operator*() at line 767
				// inlined Vector::operator*() at line 767
				// inlined Vector::operator+() at line 767
				// inlined Vector::operator=() at line 767
				// inlined DotProduct() at line 773
			}
		}
		// inlined FClassnameIs() at line 715
		{
			CPortalLaserTarget *pLaserTarget;  // line 718
			// inlined Vector::operator=() at line 723
			// inlined CPortalLaser::DamageEntity() at line 717
		}
	}
	{
		int i;  // line 641
		{
			Vector vecNearest;  // line 655
			float flFraction;  // line 656
			float flNearestDistSqr;  // line 659
			{
				LaserVictimInfo_t victim;  // line 699
				// inlined CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::InsertNoSort() at line 700
			}
			// inlined Vector::Length2DSqr() at line 659
			// inlined Vector::operator-() at line 659
			// inlined CBaseEntity::GetAbsOrigin() at line 659
			// inlined CBaseEntity::GetAbsOrigin() at line 657
			// inlined FClassnameIs() at line 647
			{
				bool bIsJumping;  // line 675
				CPortal_Player *pPlayer;  // line 685
				// inlined CBaseEntity::GetAbsOrigin() at line 686
				// inlined ToPortalPlayer() at line 685
				{
					float flZDist;  // line 679
				}
			}
			// inlined FClassnameIs() at line 647
		}
	}
	// inlined CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::CUtlSortVector() at line 638
	// inlined UTIL_EntitiesAlongRay() at line 635
	// inlined Ray_t::Init() at line 634
	// inlined Vector::Length() at line 624
	// inlined Vector::operator-() at line 620
}

// public/tier1/utlvector.h:655 @0x711e30 _ZN10CUtlVectorI17LaserVictimInfo_t10CUtlMemoryIS0_iEE10GrowVectorEi
void CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t, int> >::GrowVector( int num )
{
	// inlined CUtlMemory<LaserVictimInfo_t,int>::NumAllocated() at line 657
	// inlined CUtlMemory<LaserVictimInfo_t,int>::Grow() at line 660
	// inlined CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t, int> >::ResetDbgInfo() at line 664
}

// game/server/portal2/env_portal_laser.cpp:807 @0x70e110 _ZN12CPortalLaser24DamageEntitiesAlongLaserERK10CUtlVectorINS_17PortalLaserInfo_tE10CUtlMemoryIS1_iEEb
void CPortalLaser::DamageEntitiesAlongLaser( const CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t, int> > &infoList, bool bAutoAim )
{
	bool bBlockTarget;  // line 809
	bool bDebug;  // line 810
	{
		int i;  // line 812
		{
			const CUtlVector<CBaseEntity*,CUtlMemory<CBaseEntity*, int> > &victimList;  // line 814
			const Vector &vecStart;  // line 815
			const Vector &vecEnd;  // line 816
			{
				int j;  // line 819
				{
					CBaseEntity *pEntity;  // line 822
					{
						Vector vecPlayerVelocity;  // line 856
						Vector vecNearestPoint;  // line 860
						Vector vecPlayerPos;  // line 864
						Vector vecLineToLaser;  // line 868
						Vector vecBounce;  // line 872
						Vector vecPushVelocity;  // line 899
						{
							float flDot;  // line 885
							// inlined DotProduct() at line 885
							// inlined operator*() at line 888
							// inlined Vector::operator*() at line 888
							// inlined Vector::operator+() at line 888
							// inlined Vector::operator=() at line 888
							// inlined DotProduct() at line 894
						}
						// inlined CBaseEntity::GetAbsVelocity() at line 856
						// inlined CBaseEntity::GetAbsOrigin() at line 861
						// inlined CBaseEntity::GetAbsOrigin() at line 864
						// inlined Vector::operator-() at line 868
						// inlined Vector::LengthSqr() at line 876
						{
							Vector vecDirection;  // line 878
							// inlined Vector::operator-() at line 878
							// inlined Vector::Vector() at line 880
							// inlined CrossProduct() at line 880
							// inlined Vector::operator=() at line 880
						}
						// inlined Vector::operator*() at line 899
						// inlined CPortalLaser::DamageEntity() at line 912
						// inlined Vector::operator*=() at line 906
					}
					// inlined FClassnameIs() at line 849
					{
						ICollideable *pCollision;  // line 826
					}
					// inlined FClassnameIs() at line 833
					{
						CPortalLaserTarget *pTarget;  // line 835
						// inlined CNetworkVarBase<bool,CPortalLaser::NetworkVar_m_bShouldSpark>::operator=<bool>() at line 838
						// inlined CPortalLaser::DamageEntity() at line 841
						// inlined CPortalLaser::DamageEntity() at line 846
					}
				}
			}
			// inlined CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t, int> >::operator[]() at line 814
		}
	}
	// inlined ConVar::GetBool() at line 810
}

// game/server/portal2/env_portal_laser.cpp:926
ConVar sv_laser_tight_box;

// game/server/portal2/env_portal_laser.cpp:927 @0x70a690 _ZN12CPortalLaser21GetEntitiesAlongLaserERK6VectorS2_PS0_R10CUtlVectorINS_17PortalLaserInfo_tE10CUtlMemoryIS5_iEEb
CBaseEntity *CPortalLaser::GetEntitiesAlongLaser( const Vector &vecStart, const Vector &vecEnd, Vector *pVecOut, CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t, int> > &infoList, bool bIsFirstTrace )
{
	CPortalLaser::PortalLaserInfo_t &info;  // line 931
	Vector vecDirection;  // line 936
	float flLaserWidth;  // line 939
	Vector vecMins;  // line 953
	Vector vecMaxs;  // line 954
	CBaseEntity *list[512];  // line 972
	Ray_t ray;  // line 973
	int nCount;  // line 975
	CBaseEntity *pEntity;  // line 976
	CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess> vsrtVictims;  // line 978
	bool bBlockTarget;  // line 1053
	// inlined CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::~CUtlSortVector() at line 1088
	{
		int i;  // line 1056
		// inlined FClassnameIs() at line 1077
		// inlined CUtlVector<CBaseEntity*,CUtlMemory<CBaseEntity*, int> >::AddToTail() at line 1061
		{
			CPortalLaserTarget *pLaserTarget;  // line 1065
			// inlined Vector::operator=() at line 1070
		}
		// inlined FClassnameIs() at line 1063
	}
	// inlined CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::RedoSort() at line 1051
	// inlined CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::~CUtlSortVector() at line 1088
	{
		int i;  // line 981
		{
			Vector vecNearest;  // line 995
			float flFraction;  // line 996
			float flNearestDistSqr;  // line 999
			// inlined FClassnameIs() at line 987
			{
				bool bIsJumping;  // line 1015
				CPortal_Player *pPlayer;  // line 1025
				// inlined ToPortalPlayer() at line 1025
				// inlined CBaseEntity::GetAbsOrigin() at line 1026
				// inlined CBaseEntity::GetAbsOrigin() at line 1032
				{
					float flZDist;  // line 1019
				}
			}
			// inlined FClassnameIs() at line 987
			// inlined CBaseEntity::GetAbsOrigin() at line 997
			// inlined CBaseEntity::GetAbsOrigin() at line 999
			// inlined Vector::operator-() at line 999
			// inlined Vector::Length2DSqr() at line 999
			{
				LaserVictimInfo_t victim;  // line 1045
				// inlined CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::InsertNoSort() at line 1046
			}
		}
	}
	// inlined CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::CUtlSortVector() at line 978
	// inlined UTIL_EntitiesAlongRay() at line 975
	// inlined Ray_t::Init() at line 974
	{
		Vector tempMax;  // line 958
		// inlined Vector::Normalized() at line 962
		// inlined Vector::Normalized() at line 966
	}
	// inlined Vector::Vector() at line 953
	// inlined ConVar::GetFloat() at line 948
	// inlined Vector::operator-() at line 936
	// inlined Vector::operator=() at line 933
	// inlined Vector::operator=() at line 932
	// inlined CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t, int> >::operator[]() at line 931
	// inlined CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t, int> >::AddToTail() at line 930
	// inlined Vector::Length() at line 944
	// inlined Vector::operator=() at line 1085
}

// game/server/portal2/env_portal_laser.cpp:1092 (declaration)
void ShouldAutoAim( CBaseEntity *pEntity );

// game/server/portal2/env_portal_laser.cpp:1092 @0x70a530 _ZN12CPortalLaser13ShouldAutoAimEP11CBaseEntity
bool CPortalLaser::ShouldAutoAim( CBaseEntity *pEntity )
{
	{
		CPortalLaserTarget *pLaserTarget;  // line 1099
		bool bShouldAutoAimTarget;  // line 1100
	}
	// inlined FClassnameIs() at line 1097
}

// game/server/portal2/env_portal_laser.cpp:1115 @0x70b4a0 _ZN12CPortalLaser10TraceLaserEbRK6VectorS2_RfR10CGameTraceR10CUtlVectorINS_17PortalLaserInfo_tE10CUtlMemoryIS7_iEEPS0_
CBaseEntity *CPortalLaser::TraceLaser( bool bIsFirstTrace, const Vector &vecStart, const Vector &vecDirection, float &flTotalBeamLength, trace_t &tr, CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t, int> > &infoList, Vector *pVecAutoAimOffset )
{
	Vector vStart;  // line 1119
	Vector vDir;  // line 1120
	CTraceFilterSimpleClassnameList traceFilter;  // line 1122
	CTraceFilterSimple parentFilter;  // line 1127
	CTraceFilterChain traceChain;  // line 1128
	CUtlVector<CPortal_Base2D*,CUtlMemory<CPortal_Base2D*, int> > portalList;  // line 1130
	CBaseEntity *pHitEntity;  // line 1132
	bool bGoingThroughPortal;  // line 1133
	bool bDebug;  // line 1134
	// inlined CTraceFilterSimpleClassnameList::~CTraceFilterSimpleClassnameList() at line 1243
	// inlined CUtlVector<CPortal_Base2D*,CUtlMemory<CPortal_Base2D*, int> >::~CUtlVector() at line 1243
	// inlined CPortalLaser::ShouldAutoAim() at line 1216
	{
		Ray_t ray;  // line 1138
		CPortal_Base2D *pFirstPortal;  // line 1157
		// inlined CUtlVector<CPortal_Base2D*,CUtlMemory<CPortal_Base2D*, int> >::AddToHead() at line 1212
		// inlined Vector::operator*() at line 1139
		// inlined Vector::operator+() at line 1139
		// inlined Ray_t::Init() at line 1139
		// inlined UTIL_TraceRay() at line 1141
		{
			Vector vStrike;  // line 1187
			// inlined Vector::Length() at line 1205
			// inlined Vector::operator-() at line 1205
			{
				Vector vPointOnLine;  // line 1191
				// inlined Vector::operator-() at line 1194
				// inlined Vector::Length() at line 1194
				// inlined Vector::operator-() at line 1198
				// inlined Vector::operator=() at line 1198
			}
		}
		{
			Vector vStrike;  // line 1161
			Vector vPointOnLine;  // line 1164
			Ray_t rayTransformed;  // line 1179
			// inlined Vector::operator=() at line 1181
			// inlined Ray_t::Ray_t() at line 1179
			// inlined Vector::Length() at line 1166
			// inlined Vector::operator-() at line 1166
			// inlined Vector::operator-() at line 1172
			// inlined Vector::operator=() at line 1172
		}
		// inlined CPortal_Base2D::GetLinkedPortal() at line 1212
	}
	// inlined ConVar::GetBool() at line 1134
	// inlined CBaseEntity::GetParent() at line 1127
	{
		Vector vPointOnLine;  // line 1222
		// inlined Vector::operator-() at line 1226
		// inlined Vector::operator=() at line 1226
	}
	// inlined CPortalLaser::ShouldAutoAim() at line 1231
	{
		Vector vAutoAimOffset;  // line 1233
		{
			int i;  // line 1234
			{
				CPortal_Base2D *pPortal;  // line 1236
			}
		}
		// inlined Vector::operator=() at line 1240
	}
	// inlined CHandle<CBaseEntity>::operator CBaseEntity*() at line 1218
	// inlined CUtlVector<CPortal_Base2D*,CUtlMemory<CPortal_Base2D*, int> >::~CUtlVector() at line 1243
	// inlined CTraceFilterSimpleClassnameList::~CTraceFilterSimpleClassnameList() at line 1243
}

// game/server/portal2/env_portal_laser.cpp:1247 (declaration)
void RemoveChildLaser();

// game/server/portal2/env_portal_laser.cpp:1247 @0x709080 _ZN12CPortalLaser16RemoveChildLaserEv
void CPortalLaser::RemoveChildLaser()
{
}

// game/server/portal2/env_portal_laser.cpp:1257 @0x711450 _ZN12CPortalLaser15UpdateNextLaserERK6VectorS2_P11CBaseEntity
void CPortalLaser::UpdateNextLaser( const Vector &vecStart, const Vector &vecDirection, CBaseEntity *pParent )
{
	// inlined CHandle<CBaseEntity>::Get() at line 1276
	{
		CPropWeightedCube *pOldCube;  // line 1279
		// inlined CHandle<CBaseEntity>::operator=() at line 1282
	}
	// inlined CHandle<CBaseEntity>::operator=() at line 1265
}

// game/server/portal2/env_portal_laser.cpp:1292 @0x711650 _ZN12CPortalLaser22ReflectLaserFromEntityEP11CBaseEntity
bool CPortalLaser::ReflectLaserFromEntity( CBaseEntity *pEntity )
{
	{
		CPropWeightedCube *pCube;  // line 1296
		Vector vecForward;  // line 1301
		Vector vecOffset;  // line 1304
		{
			CPortalSimulator *pSimulator;  // line 1308
			// inlined VMatrix::operator*() at line 1314
			// inlined Vector::operator=() at line 1314
			// inlined VMatrix::ApplyRotation() at line 1315
			// inlined Vector::operator=() at line 1315
		}
		// inlined Vector::operator+() at line 1304
		// inlined Vector::operator*() at line 1304
		// inlined CBaseEntity::GetAbsAngles() at line 1302
		// inlined CPropWeightedCube::HasLaser() at line 1297
	}
}

// game/server/portal2/env_portal_laser.cpp:1338 @0x70f3a0 _ZN12CPortalLaser9FireLaserERK6VectorS2_P11CBaseEntity
void CPortalLaser::FireLaser( const Vector &vecStart, const Vector &vecDirection, CBaseEntity *pParent )
{
	{
		float flTotalBeamLength;  // line 1345
		trace_t tr;  // line 1346
		Vector vAutoAimOffset;  // line 1347
		Vector vDir;  // line 1348
		CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t, int> > firstInfoList;  // line 1351
		CBaseEntity *pHitTarget;  // line 1352
		bool bAutoAimSuccess;  // line 1353
		{
			trace_t tempTrace;  // line 1356
			Vector vNewEndPoint;  // line 1357
			Vector vNewDirection;  // line 1358
			float flNewBeamLength;  // line 1359
			CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t, int> > secondInfoList;  // line 1362
			CBaseEntity *pSecondHitTarget;  // line 1363
			// inlined CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t, int> >::~CUtlVector() at line 1384
			// inlined Vector::operator=() at line 1374
			// inlined CGameTrace::operator=() at line 1372
			// inlined Vector::operator*() at line 1357
			// inlined Vector::operator+() at line 1357
			// inlined Vector::operator+() at line 1357
			// inlined Vector::operator-() at line 1358
			// inlined Vector::Normalized() at line 1358
			// inlined CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t, int> >::CUtlVector() at line 1362
			// inlined CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t, int> >::~CUtlVector() at line 1384
		}
		// inlined CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t, int> >::~CUtlVector() at line 1430
		// inlined CPortalLaser::RemoveChildLaser() at line 1425
		{
			CBaseEntity *pEnt;  // line 1418
			// inlined CHandle<CBaseEntity>::operator CBaseEntity*() at line 1418
		}
		// inlined FClassnameIs() at line 1411
		// inlined CBeam::SetAutoAim() at line 1408
		// inlined Vector::operator VectorByValue&() at line 1403
		// inlined Vector::operator+() at line 1403
		// inlined Vector::operator*() at line 1403
		// inlined CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t, int> >::CUtlVector() at line 1351
		// inlined CPortalLaser::ShouldAutoAim() at line 1354
		{
			CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t, int> > secondInfoList;  // line 1389
			// inlined CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t, int> >::~CUtlVector() at line 1392
			// inlined CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t, int> >::CUtlVector() at line 1389
		}
		// inlined Vector::operator*() at line 1398
		// inlined Vector::operator+() at line 1398
		// inlined Vector::operator VectorByValue&() at line 1398
		// inlined CBeam::SetEndAttachment() at line 1399
	}
	{
		CTraceFilterSimpleClassnameList traceFilter;  // line 1434
		Ray_t ray;  // line 1439
		trace_t tr;  // line 1442
		CPortal_Base2D *pFirstPortal;  // line 1449
		Vector vecNewTermPoint;  // line 1485
		// inlined CTraceFilterSimpleClassnameList::~CTraceFilterSimpleClassnameList() at line 1520
		// inlined CPortalLaser::RemoveChildLaser() at line 1515
		{
			CBaseEntity *pEnt;  // line 1508
			// inlined CHandle<CBaseEntity>::operator CBaseEntity*() at line 1508
		}
		// inlined CBeam::SetEndAttachment() at line 1498
		// inlined Vector::operator*() at line 1440
		// inlined Vector::operator+() at line 1440
		// inlined Ray_t::Init() at line 1440
		// inlined UTIL_TraceRay() at line 1444
		// inlined CTraceFilterSimpleClassnameList::~CTraceFilterSimpleClassnameList() at line 1520
		// inlined CTraceFilterSimpleClassnameList::~CTraceFilterSimpleClassnameList() at line 1520
		{
			Vector vecNewTermPoint;  // line 1453
			Ray_t rayTransformed;  // line 1471
			Vector vecDirection;  // line 1473
			Vector vecStartPos;  // line 1476
			// inlined Ray_t::Ray_t() at line 1471
		}
	}
}

// game/server/portal2/env_portal_laser.cpp:1554 @0x710f10 _ZN12CPortalLaser11StrikeThinkEv
void CPortalLaser::StrikeThink()
{
	CVProfScope VProf_;  // line 1556
	Vector vecDir;  // line 1572
	Vector vecOrigin;  // line 1573
	CBaseEntity *pParent;  // line 1574
	// inlined CVProfScope::CVProfScope() at line 1556
	// inlined CNetworkVarBase<bool,CPortalLaser::NetworkVar_m_bShouldSpark>::operator=<bool>() at line 1563
	{
		int i;  // line 1566
		// inlined Vector::operator=() at line 1568
	}
	// inlined CBaseEntity::GetParent() at line 1574
	{
		int i;  // line 1597
	}
	// inlined CPortalLaser::ShowBeam() at line 1605
	// inlined CVProfScope::~CVProfScope() at line 1610
	// inlined Vector::operator=() at line 1577
	// inlined CBaseEntity::GetAbsAngles() at line 1578
	// inlined CVProfScope::~CVProfScope() at line 1610
}
