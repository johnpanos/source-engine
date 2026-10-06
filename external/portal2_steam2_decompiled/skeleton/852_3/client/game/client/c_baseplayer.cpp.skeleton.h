// DWARF declaration skeleton for game/client/c_baseplayer.cpp
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x2f9f0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
	// inlined Vector::Vector() at line 74
	// inlined Vector::Vector() at line 75
	// inlined ClientClass::ClientClass() at line 226
	// inlined PredMapInit<CPlayerState>() at line 286
	// inlined PredMapInit<CPlayerLocalData>() at line 296
	// inlined PredMapInit<C_BasePlayer>() at line 330
}

// game/shared/playernet_vars.h:23 @0x301ae0 _ZN11fogparams_t19NetworkStateChangedEv
void fogparams_t::NetworkStateChanged()
{
}

// game/shared/playernet_vars.h:23 @0x301af0 _ZN11fogparams_t19NetworkStateChangedEPv
void fogparams_t::NetworkStateChanged( void *pProp )
{
}

// public/playerstate.h:25 @0x301b80 _ZN12CPlayerState19NetworkStateChangedEv
void CPlayerState::NetworkStateChanged()
{
}

// public/playerstate.h:25 @0x301b90 _ZN12CPlayerState19NetworkStateChangedEPv
void CPlayerState::NetworkStateChanged( void *pProp )
{
}

// public/playerstate.h:29 @0x301df0 _ZN12CPlayerStateD0Ev
CPlayerState::~CPlayerState()
{
}

// public/playerstate.h:29 @0x301e10 _ZN12CPlayerStateD1Ev
CPlayerState::~CPlayerState()
{
}

// game/client/c_playerlocaldata.h:35 @0x301b60 _ZN16CPlayerLocalData19NetworkStateChangedEv
void CPlayerLocalData::NetworkStateChanged()
{
}

// game/client/c_playerlocaldata.h:35 @0x301b70 _ZN16CPlayerLocalData19NetworkStateChangedEPv
void CPlayerLocalData::NetworkStateChanged( void *pProp )
{
}

// game/shared/playernet_vars.h:62 @0x301b00 _ZN17fogplayerparams_t19NetworkStateChangedEv
void fogplayerparams_t::NetworkStateChanged()
{
}

// game/shared/playernet_vars.h:62 @0x301b10 _ZN17fogplayerparams_t19NetworkStateChangedEPv
void fogplayerparams_t::NetworkStateChanged( void *pProp )
{
}

// game/client/c_baseplayer.cpp:63
int g_nKillCamMode;

// game/client/c_baseplayer.cpp:64
int g_nKillCamTarget1;

// game/client/c_baseplayer.cpp:65
int g_nKillCamTarget2;

// game/client/c_baseplayer.cpp:74
static Vector WALL_MIN;

// game/client/c_baseplayer.cpp:75
static Vector WALL_MAX;

// game/client/c_baseplayer.cpp:82
static C_BasePlayer *s_pLocalPlayer[2];

// game/client/c_baseplayer.h:84 @0x301ba0 _ZN12C_BasePlayer8ClassifyEv
Class_T C_BasePlayer::Classify()
{
}

// game/client/c_baseplayer.cpp:84
static ConVar cl_customsounds;

// game/client/c_baseplayer.cpp:85
static ConVar spec_track;

// game/client/c_baseplayer.cpp:86
static ConVar cl_smooth;

// game/client/c_baseplayer.cpp:87
static ConVar cl_smoothtime;

// game/client/c_baseplayer.cpp:96
ConVar spec_freeze_time;

// game/client/c_baseplayer.cpp:97
ConVar spec_freeze_traveltime;

// game/client/c_baseplayer.cpp:98
ConVar spec_freeze_distance_min;

// game/client/c_baseplayer.cpp:99
ConVar spec_freeze_distance_max;

// game/shared/playernet_vars.h:107 @0x301b20 _ZN13sky3dparams_t19NetworkStateChangedEv
void sky3dparams_t::NetworkStateChanged()
{
}

// game/shared/playernet_vars.h:107 @0x301b30 _ZN13sky3dparams_t19NetworkStateChangedEPv
void sky3dparams_t::NetworkStateChanged( void *pProp )
{
}

// game/client/c_baseplayer.cpp:107 @0x2e640 _Z15ClientClassInitIN14DT_PlayerState7ignoredEEiPT_
int ClientClassInit<DT_PlayerState::ignored>( DT_PlayerState::ignored * )
{
	char *pRecvTableName;  // line 107
	RecvTable &RecvTable;  // line 107
	RecvProp RecvProps[2];  // line 107
}

// game/client/c_baseplayer.cpp:107
RecvTable g_RecvTable;

// game/client/c_baseplayer.cpp:107
int g_RecvTableInit;

// game/client/c_baseplayer.cpp:112 @0x2ee50 _Z15ClientClassInitIN8DT_Local7ignoredEEiPT_
int ClientClassInit<DT_Local::ignored>( DT_Local::ignored * )
{
	char *pRecvTableName;  // line 112
	RecvTable &RecvTable;  // line 112
	RecvProp RecvProps[42];  // line 112
}

// game/client/c_baseplayer.cpp:112
RecvTable g_RecvTable;

// game/client/c_baseplayer.cpp:112
int g_RecvTableInit;

// game/shared/playernet_vars.h:119 @0x301d70 _ZN13sky3dparams_t14NetworkVar_fog19NetworkStateChangedEv
void sky3dparams_t::NetworkVar_fog::NetworkStateChanged()
{
	// inlined DispatchNetworkStateChanged<sky3dparams_t>() at line 119
}

// game/shared/playernet_vars.h:119 @0x301ea0 _ZN13sky3dparams_t14NetworkVar_fog19NetworkStateChangedEPv
void sky3dparams_t::NetworkVar_fog::NetworkStateChanged( void *pVar )
{
	// inlined DispatchNetworkStateChanged<sky3dparams_t>() at line 119
}

// game/shared/playernet_vars.h:125 @0x301b40 _ZN13audioparams_t19NetworkStateChangedEv
void audioparams_t::NetworkStateChanged()
{
}

// game/shared/playernet_vars.h:125 @0x301b50 _ZN13audioparams_t19NetworkStateChangedEPv
void audioparams_t::NetworkStateChanged( void *pProp )
{
}

// game/client/c_baseplayer.h:127 @0x301bb0 _ZN12C_BasePlayer27ActivePlayerCombatCharacterEv
C_BaseCombatCharacter *C_BasePlayer::ActivePlayerCombatCharacter()
{
}

// game/client/c_baseplayer.h:131 @0x301bc0 _ZN12C_BasePlayer18Weapon_DropPrimaryEv
void C_BasePlayer::Weapon_DropPrimary()
{
}

// game/client/c_baseplayer.h:147 @0x301bd0 _ZN12C_BasePlayer29ForceDropOfCarriedPhysObjectsEv
void C_BasePlayer::ForceDropOfCarriedPhysObjects()
{
}

// game/client/c_baseplayer.h:151 @0x301be0 _ZNK12C_BasePlayer8IsPlayerEv
bool C_BasePlayer::IsPlayer()
{
}

// game/client/c_baseplayer.h:152 @0x301bf0 _ZNK12C_BasePlayer9GetHealthEv
int C_BasePlayer::GetHealth()
{
}

// game/client/c_baseplayer.cpp:182 @0x2e760 _Z15ClientClassInitIN23DT_LocalPlayerExclusive7ignoredEEiPT_
int ClientClassInit<DT_LocalPlayerExclusive::ignored>( DT_LocalPlayerExclusive::ignored * )
{
	char *pRecvTableName;  // line 182
	RecvTable &RecvTable;  // line 182
	RecvProp RecvProps[25];  // line 182
}

// game/client/c_baseplayer.cpp:182
RecvTable g_RecvTable;

// game/client/c_baseplayer.cpp:182
int g_RecvTableInit;

// game/client/c_baseplayer.h:199 @0x301c00 _ZNK12C_BasePlayer24GetFlashlightTextureNameEv
const char *C_BasePlayer::GetFlashlightTextureName()
{
}

// game/client/c_baseplayer.h:200 @0x301c10 _ZNK12C_BasePlayer16GetFlashlightFOVEv
float C_BasePlayer::GetFlashlightFOV()
{
}

// game/client/c_baseplayer.h:201 @0x301c20 _ZNK12C_BasePlayer17GetFlashlightFarZEv
float C_BasePlayer::GetFlashlightFarZ()
{
}

// game/client/c_baseplayer.h:202 @0x301c30 _ZNK12C_BasePlayer24GetFlashlightLinearAttenEv
float C_BasePlayer::GetFlashlightLinearAtten()
{
}

// game/client/c_baseplayer.h:203 @0x301c40 _ZNK12C_BasePlayer22CastsFlashlightShadowsEv
bool C_BasePlayer::CastsFlashlightShadows()
{
}

// game/client/c_baseplayer.h:211 @0x301ed0 _ZN12C_BasePlayer24IsAllowedToSwitchWeaponsEv
bool C_BasePlayer::IsAllowedToSwitchWeapons()
{
	// inlined C_BasePlayer::IsObserver() at line 211
}

// game/client/c_baseplayer.h:219 @0x301c50 _ZN12C_BasePlayer21IsOverridingViewmodelEv
bool C_BasePlayer::IsOverridingViewmodel()
{
}

// game/client/c_baseplayer.h:220 @0x301c60 _ZN12C_BasePlayer23DrawOverriddenViewmodelEP15C_BaseViewModeliRK20RenderableInstance_t
int C_BasePlayer::DrawOverriddenViewmodel( C_BaseViewModel *pViewmodel, int flags, const RenderableInstance_t &instance )
{
}

// game/client/c_baseplayer.h:222 @0x301c70 _ZN12C_BasePlayer19GetDefaultAnimSpeedEv
float C_BasePlayer::GetDefaultAnimSpeed()
{
}

// game/client/c_baseplayer.cpp:226 @0x2f6fc0 _ZN12C_BasePlayer40YouForgotToImplementOrDeclareClientClassEv
int C_BasePlayer::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/c_baseplayer.cpp:226 @0x2f6fe0 _ZN12C_BasePlayer14GetClientClassEv
ClientClass *C_BasePlayer::GetClientClass()
{
}

// game/client/c_baseplayer.cpp:226 @0x2dec0 _Z15ClientClassInitIN13DT_BasePlayer7ignoredEEiPT_
int ClientClassInit<DT_BasePlayer::ignored>( DT_BasePlayer::ignored * )
{
	char *pRecvTableName;  // line 226
	RecvTable &RecvTable;  // line 226
	RecvProp RecvProps[32];  // line 226
}

// game/client/c_baseplayer.cpp:226
RecvTable g_RecvTable;

// game/client/c_baseplayer.cpp:226
int g_RecvTableInit;

// game/client/c_baseplayer.cpp:226 @0x300ad0 _ZL26_C_BasePlayer_CreateObjectii
IClientNetworkable *_C_BasePlayer_CreateObject( int entnum, int serialNum )
{
	C_BasePlayer *pRet;  // line 226
	// inlined C_BasePlayer::C_BasePlayer() at line 226
}

// game/client/c_baseplayer.cpp:226
ClientClass __g_C_BasePlayerClientClass;

// game/client/c_baseplayer.cpp:226
void C_BasePlayer::m_pClassRecvTable;

// game/client/c_baseplayer.h:228 @0x301c90 _ZN12C_BasePlayer14ShadowCastTypeEv
ShadowType_t C_BasePlayer::ShadowCastType()
{
}

// game/client/c_baseplayer.h:230 @0x301cb0 _ZN12C_BasePlayer30ShouldReceiveProjectedTexturesEi
bool C_BasePlayer::ShouldReceiveProjectedTextures( int flags )
{
}

// game/client/c_baseplayer.h:280 @0x301cc0 _ZN12C_BasePlayer18IsFollowingPhysicsEv
bool C_BasePlayer::IsFollowingPhysics()
{
}

// game/client/c_baseplayer.cpp:286 @0x2f6ff0 _ZN12CPlayerState14GetPredDescMapEv
datamap_t *CPlayerState::GetPredDescMap()
{
}

// game/client/c_baseplayer.cpp:286 (declaration)
datamap_t *PredMapInit<CPlayerState>( CPlayerState * );

// game/client/c_baseplayer.cpp:286 @0x2f7000 _Z11PredMapInitI12CPlayerStateEP9datamap_tPT_
datamap_t *PredMapInit<CPlayerState>( CPlayerState * )
{
}

// game/client/c_baseplayer.cpp:286
datamap_t *g_PredMapHolder;

// game/client/c_baseplayer.cpp:286
void CPlayerState::m_PredMap;

// game/client/c_baseplayer.h:292 @0x301cd0 _ZNK12C_BasePlayer25PhysicsSolidMaskForEntityEv
unsigned int C_BasePlayer::PhysicsSolidMaskForEntity()
{
}

// game/client/c_baseplayer.cpp:296 @0x2f7030 _ZN16CPlayerLocalData14GetPredDescMapEv
datamap_t *CPlayerLocalData::GetPredDescMap()
{
}

// game/client/c_baseplayer.cpp:296 (declaration)
datamap_t *PredMapInit<CPlayerLocalData>( CPlayerLocalData * );

// game/client/c_baseplayer.cpp:296 @0x2f7040 _Z11PredMapInitI16CPlayerLocalDataEP9datamap_tPT_
datamap_t *PredMapInit<CPlayerLocalData>( CPlayerLocalData * )
{
}

// game/client/c_baseplayer.cpp:296
datamap_t *g_PredMapHolder;

// game/client/c_baseplayer.cpp:296
void CPlayerLocalData::m_PredMap;

// game/client/c_baseplayer.h:307 @0x301ce0 _ZN12C_BasePlayer20Weapon_ShouldSetLastEP18C_BaseCombatWeaponS1_
bool C_BasePlayer::Weapon_ShouldSetLast( C_BaseCombatWeapon *pOldWeapon, C_BaseCombatWeapon *pNewWeapon )
{
}

// game/client/c_baseplayer.h:310 @0x301d90 _ZN12C_BasePlayer13GetLastWeaponEv
C_BaseCombatWeapon *C_BasePlayer::GetLastWeapon()
{
	// inlined CHandle<C_BaseCombatWeapon>::Get() at line 310
}

// game/client/c_baseplayer.h:318 @0x301cf0 _ZN12C_BasePlayer8IsZoomedEv
bool C_BasePlayer::IsZoomed()
{
}

// game/client/c_baseplayer.cpp:330 @0x2f7070 _ZN12C_BasePlayer14GetPredDescMapEv
datamap_t *C_BasePlayer::GetPredDescMap()
{
}

// game/client/c_baseplayer.cpp:330 (declaration)
datamap_t *PredMapInit<C_BasePlayer>( C_BasePlayer * );

// game/client/c_baseplayer.cpp:330 @0x2f7080 _Z11PredMapInitI12C_BasePlayerEP9datamap_tPT_
datamap_t *PredMapInit<C_BasePlayer>( C_BasePlayer * )
{
}

// game/client/c_baseplayer.cpp:330
datamap_t *g_PredMapHolder;

// game/client/c_baseplayer.cpp:330
void C_BasePlayer::m_PredMap;

// game/client/c_baseplayer.cpp:411 (declaration)
void C_BasePlayer();

// game/client/c_baseplayer.cpp:411 @0x2ff9d0 _ZN12C_BasePlayerC2Ev
C_BasePlayer::C_BasePlayer()
{
	// inlined Vector::Init() at line 438
	// inlined CHandle<C_BaseEntity>::operator=() at line 427
	{
		int i;  // line 422
	}
	// inlined CHandle<C_BaseEntity>::operator=() at line 420
	// inlined fogplayerparams_t::fogplayerparams_t() at line 411
	// inlined CNetworkHandleBase<C_ColorCorrection,C_BasePlayer::NetworkVar_m_hColorCorrectionCtrl>::CNetworkHandleBase() at line 411
	// inlined CNetworkHandleBase<C_PostProcessController,C_BasePlayer::NetworkVar_m_hPostProcessCtrl>::CNetworkHandleBase() at line 411
	// inlined C_BasePlayer::StepSoundCache_t::StepSoundCache_t() at line 411
	// inlined CHandle<C_BasePlayer>::CHandle() at line 411
	// inlined CUtlVector<CHandle<C_BasePlayer>,CUtlMemory<CHandle<C_BasePlayer>, int> >::CUtlVector() at line 411
	// inlined CSmartPtr<WaterDebrisEffect,CRefCountAccessor>::CSmartPtr() at line 411
	// inlined TimedEvent::TimedEvent() at line 411
	// inlined C_CommandContext::C_CommandContext() at line 411
	// inlined CUtlVector<CHandle<C_BaseEntity>,CUtlMemory<CHandle<C_BaseEntity>, int> >::CUtlVector() at line 411
	// inlined CHandle<C_BaseEntity>::CHandle() at line 411
	// inlined CInterpolatedVar<Vector>::CInterpolatedVar() at line 411
	// inlined CHandle<C_BaseEntity>::CHandle() at line 411
	// inlined CHandle<C_BaseEntity>::CHandle() at line 411
	// inlined CHandle<C_BaseEntity>::CHandle() at line 411
	// inlined CHandle<C_BaseEntity>::CHandle() at line 411
	// inlined CHandle<C_BaseEntity>::CHandle() at line 411
	// inlined CHandle<C_BaseViewModel>::CHandle() at line 411
	// inlined CHandle<C_BaseCombatWeapon>::CHandle() at line 411
	// inlined CHandle<C_BaseEntity>::CHandle() at line 411
	// inlined CUserCmd::CUserCmd() at line 411
	// inlined CHandle<C_BaseEntity>::CHandle() at line 411
	// inlined CPlayerState::CPlayerState() at line 411
	// inlined CHandle<C_BaseEntity>::CHandle() at line 411
	// inlined C_BasePlayer::NetworkVar_m_Local::NetworkVar_m_Local() at line 411
	// inlined CHandle<C_BaseEntity>::CHandle() at line 411
	// inlined fogparams_t::fogparams_t() at line 411
	// inlined CUserCmd::~CUserCmd() at line 449
	// inlined CPlayerState::~CPlayerState() at line 449
	// inlined C_BasePlayer::NetworkVar_m_Local::~NetworkVar_m_Local() at line 449
	// inlined CUtlVector<CHandle<C_BasePlayer>,CUtlMemory<CHandle<C_BasePlayer>, int> >::~CUtlVector() at line 449
	// inlined CSmartPtr<WaterDebrisEffect,CRefCountAccessor>::~CSmartPtr() at line 449
	// inlined C_CommandContext::~C_CommandContext() at line 449
	// inlined CUtlVector<CHandle<C_BaseEntity>,CUtlMemory<CHandle<C_BaseEntity>, int> >::~CUtlVector() at line 449
	// inlined CInterpolatedVar<Vector>::~CInterpolatedVar() at line 449
}

// game/client/c_baseplayer.cpp:411 @0x300ac0 _ZN12C_BasePlayerC1Ev
C_BasePlayer::C_BasePlayer()
{
}

// game/client/c_baseplayer.h:415 @0x301d00 _ZN12C_BasePlayer10ExitLadderEv
void C_BasePlayer::ExitLadder()
{
}

// game/client/c_baseplayer.h:427 @0x301d10 _ZN12C_BasePlayer5HintsEv
CHintSystem *C_BasePlayer::Hints()
{
}

// game/client/c_baseplayer.h:435 @0x301d20 _ZN12C_BasePlayer12GetFogParamsEv
fogparams_t *C_BasePlayer::GetFogParams()
{
}

// game/client/c_baseplayer.h:450 @0x301d30 _ZN12C_BasePlayer21OnAchievementAchievedEi
void C_BasePlayer::OnAchievementAchieved( int iAchievement )
{
}

// game/client/c_baseplayer.cpp:454 (declaration)
~C_BasePlayer();

// game/client/c_baseplayer.cpp:454 @0x300b30 _ZN12C_BasePlayerD0Ev
C_BasePlayer::~C_BasePlayer()
{
	// inlined CHandle<C_BaseEntity>::Get() at line 456
	{
		int i;  // line 457
		// inlined CFlashlightEffectManager::TurnOffFlashlight() at line 470
	}
	// inlined CUtlVector<CHandle<C_BasePlayer>,CUtlMemory<CHandle<C_BasePlayer>, int> >::~CUtlVector() at line 474
	// inlined CSmartPtr<WaterDebrisEffect,CRefCountAccessor>::~CSmartPtr() at line 474
	// inlined C_CommandContext::~C_CommandContext() at line 474
	// inlined CUtlVector<CHandle<C_BaseEntity>,CUtlMemory<CHandle<C_BaseEntity>, int> >::~CUtlVector() at line 474
	// inlined CInterpolatedVar<Vector>::~CInterpolatedVar() at line 474
	// inlined CUserCmd::~CUserCmd() at line 474
	// inlined CPlayerState::~CPlayerState() at line 474
	// inlined C_BasePlayer::NetworkVar_m_Local::~NetworkVar_m_Local() at line 474
	// inlined CUtlVector<CHandle<C_BasePlayer>,CUtlMemory<CHandle<C_BasePlayer>, int> >::~CUtlVector() at line 474
	// inlined CSmartPtr<WaterDebrisEffect,CRefCountAccessor>::~CSmartPtr() at line 474
	// inlined C_CommandContext::~C_CommandContext() at line 474
	// inlined CUtlVector<CHandle<C_BaseEntity>,CUtlMemory<CHandle<C_BaseEntity>, int> >::~CUtlVector() at line 474
	// inlined CInterpolatedVar<Vector>::~CInterpolatedVar() at line 474
	// inlined CUserCmd::~CUserCmd() at line 474
	// inlined CPlayerState::~CPlayerState() at line 474
	// inlined C_BasePlayer::NetworkVar_m_Local::~NetworkVar_m_Local() at line 474
}

// game/client/c_baseplayer.cpp:454 @0x301300 _ZN12C_BasePlayerD2Ev
C_BasePlayer::~C_BasePlayer()
{
	// inlined CHandle<C_BaseEntity>::Get() at line 456
	{
		int i;  // line 457
		// inlined CFlashlightEffectManager::TurnOffFlashlight() at line 470
	}
	// inlined CUtlVector<CHandle<C_BasePlayer>,CUtlMemory<CHandle<C_BasePlayer>, int> >::~CUtlVector() at line 474
	// inlined CSmartPtr<WaterDebrisEffect,CRefCountAccessor>::~CSmartPtr() at line 474
	// inlined C_CommandContext::~C_CommandContext() at line 474
	// inlined CUtlVector<CHandle<C_BaseEntity>,CUtlMemory<CHandle<C_BaseEntity>, int> >::~CUtlVector() at line 474
	// inlined CInterpolatedVar<Vector>::~CInterpolatedVar() at line 474
	// inlined CUserCmd::~CUserCmd() at line 474
	// inlined CPlayerState::~CPlayerState() at line 474
	// inlined C_BasePlayer::NetworkVar_m_Local::~NetworkVar_m_Local() at line 474
	// inlined CUtlVector<CHandle<C_BasePlayer>,CUtlMemory<CHandle<C_BasePlayer>, int> >::~CUtlVector() at line 474
	// inlined CSmartPtr<WaterDebrisEffect,CRefCountAccessor>::~CSmartPtr() at line 474
	// inlined C_CommandContext::~C_CommandContext() at line 474
	// inlined CUtlVector<CHandle<C_BaseEntity>,CUtlMemory<CHandle<C_BaseEntity>, int> >::~CUtlVector() at line 474
	// inlined CInterpolatedVar<Vector>::~CInterpolatedVar() at line 474
	// inlined CUserCmd::~CUserCmd() at line 474
	// inlined CPlayerState::~CPlayerState() at line 474
	// inlined C_BasePlayer::NetworkVar_m_Local::~NetworkVar_m_Local() at line 474
}

// game/client/c_baseplayer.cpp:454 @0x301ad0 _ZN12C_BasePlayerD1Ev
C_BasePlayer::~C_BasePlayer()
{
}

// game/client/c_baseplayer.cpp:479 @0x2f8470 _ZN12C_BasePlayer5SpawnEv
void C_BasePlayer::Spawn()
{
	int effects;  // line 485
}

// game/client/c_baseplayer.h:480 @0x301de0 _ZN12C_BasePlayer18NetworkVar_m_Local19NetworkStateChangedEv
void C_BasePlayer::NetworkVar_m_Local::NetworkStateChanged()
{
}

// game/client/c_baseplayer.h:480 @0x301ec0 _ZN12C_BasePlayer18NetworkVar_m_Local19NetworkStateChangedEPv
void C_BasePlayer::NetworkVar_m_Local::NetworkStateChanged( void *pVar )
{
}

// game/client/c_baseplayer.cpp:501 @0x2f83c0 _ZN12C_BasePlayer14UpdateOnRemoveEv
void C_BasePlayer::UpdateOnRemove()
{
}

// game/client/c_baseplayer.cpp:529 @0x2f9150 _ZN12C_BasePlayer22AudioStateIsUnderwaterE6Vector
bool C_BasePlayer::AudioStateIsUnderwater( Vector vecMainViewOrigin )
{
	// inlined C_BasePlayer::IsObserver() at line 531
	{
		int cont;  // line 534
	}
}

// game/client/c_baseplayer.cpp:548 @0x2f8320 _ZNK12C_BasePlayer17GetObserverTargetEv
C_BaseEntity *C_BasePlayer::GetObserverTarget()
{
	// inlined C_BasePlayer::IsHLTV() at line 550
	// inlined CHandle<C_BaseEntity>::operator C_BaseEntity*() at line 567
}

// game/client/c_baseplayer.cpp:572 (declaration)
void UpdateViewmodelVisibility( C_BasePlayer *player );

// game/client/c_baseplayer.cpp:585 @0x2fc490 _ZN12C_BasePlayer17SetObserverTargetE7CHandleI12C_BaseEntityE
void C_BasePlayer::SetObserverTarget( EHANDLE &hObserverTarget )
{
	{
		IGameEvent *event;  // line 599
		{
			CSetActiveSplitScreenPlayerGuard g_SSEGuard;  // line 607
		}
		// inlined CBaseHandle::GetEntryIndex() at line 597
		// inlined CBaseHandle::Init() at line 597
		// inlined C_BasePlayer::IsLocalPlayer() at line 605
		// inlined UpdateViewmodelVisibility() at line 610
	}
	// inlined CBaseHandle::GetEntryIndex() at line 592
	// inlined CBaseHandle::GetEntryIndex() at line 592
	// inlined CBaseHandle::GetSerialNumber() at line 592
}

// public/tier1/interpolatedvar.h:587 @0x301e30 _ZN25CInterpolatedVarArrayBaseI6QAngleLb0EED2Ev
CInterpolatedVarArrayBase<QAngle,false>::~CInterpolatedVarArrayBase()
{
	// inlined CInterpolatedVarArrayBase<QAngle,false>::ClearHistory() at line 589
	// inlined CSimpleRingBuffer<CInterpolatedVarEntryBase<QAngle, false> >::~CSimpleRingBuffer() at line 591
}

// game/client/c_baseplayer.cpp:614 @0x2f8a90 _ZNK12C_BasePlayer15GetObserverModeEv
int C_BasePlayer::GetObserverMode()
{
	// inlined C_BasePlayer::IsHLTV() at line 616
}

// game/client/c_baseplayer.cpp:634 @0x2f70b0 _ZN12C_BasePlayer18SetLocalViewAnglesERK6QAngle
void C_BasePlayer::SetLocalViewAngles( const QAngle &viewAngles )
{
	// inlined QAngle::operator=() at line 636
}

// game/client/c_baseplayer.cpp:643 @0x2f82e0 _ZN12C_BasePlayer13SetViewAnglesERK6QAngle
void C_BasePlayer::SetViewAngles( const QAngle &ang )
{
}

// game/client/c_baseplayer.cpp:650 @0x2f7da0 _ZN12C_BasePlayer16GetGroundSurfaceEv
surfacedata_t *C_BasePlayer::GetGroundSurface()
{
	Vector start;  // line 655
	Vector end;  // line 655
	Ray_t ray;  // line 664
	trace_t trace;  // line 667
	// inlined VectorCopy() at line 656
	// inlined Ray_t::Ray_t() at line 664
	// inlined Ray_t::Init() at line 665
	// inlined UTIL_TraceRay() at line 668
}

// game/client/c_baseplayer.cpp:680 (declaration)
void GetPlayerName();

// game/client/c_baseplayer.cpp:680 @0x2f70e0 _ZN12C_BasePlayer13GetPlayerNameEv
const char *C_BasePlayer::GetPlayerName()
{
}

// game/client/c_baseplayer.cpp:688 @0x2f7630 _ZN12C_BasePlayer12IsPlayerDeadEv
bool C_BasePlayer::IsPlayerDead()
{
}

// game/client/c_baseplayer.h:691 @0x301d40 _ZNK12C_BasePlayer8IsDuckedEv
bool C_BasePlayer::IsDucked()
{
}

// game/client/c_baseplayer.h:692 @0x301d50 _ZNK12C_BasePlayer9IsDuckingEv
bool C_BasePlayer::IsDucking()
{
}

// game/client/c_baseplayer.h:693 @0x301d60 _ZN12C_BasePlayer15GetFallVelocityEv
float C_BasePlayer::GetFallVelocity()
{
}

// game/client/c_baseplayer.cpp:696 @0x2f7ad0 _ZN12C_BasePlayer14SetVehicleRoleEi
void C_BasePlayer::SetVehicleRole( int nRole )
{
	char szCmd[64];  // line 705
	// inlined C_BasePlayer::IsInAVehicle() at line 698
}

// game/client/c_baseplayer.cpp:714 @0x2f9960 _ZN12C_BasePlayer16OnPreDataChangedE16DataUpdateType_t
void C_BasePlayer::OnPreDataChanged( DataUpdateType_t updateType )
{
	// inlined CHandle<C_BaseEntity>::operator=() at line 722
	// inlined CNetworkHandleBase<C_FogController,fogplayerparams_t::NetworkVar_m_hCtrl>::operator C_FogController*() at line 722
	{
		int i;  // line 716
	}
}

// game/client/c_baseplayer.cpp:727 @0x2f82b0 _ZN12C_BasePlayer13PreDataUpdateE16DataUpdateType_t
void C_BasePlayer::PreDataUpdate( DataUpdateType_t updateType )
{
}

// game/client/c_baseplayer.cpp:734 @0x2f9f30 _ZN12C_BasePlayer19CheckForLocalPlayerEi
void C_BasePlayer::CheckForLocalPlayer( int nSplitScreenSlot )
{
	int iLocalPlayerIndex;  // line 737
	{
		ConVar *pVar;  // line 770
	}
	// inlined CHandle<C_BasePlayer>::operator=() at line 763
	// inlined CHandle<C_BasePlayer>::operator=() at line 753
}

// game/client/c_baseplayer.cpp:780 @0x2fc6a0 _ZN12C_BasePlayer14PostDataUpdateE16DataUpdateType_t
void C_BasePlayer::PostDataUpdate( DataUpdateType_t updateType )
{
	int nSlot;  // line 786
	CSetActiveSplitScreenPlayerGuard g_SSGuard;  // line 797
	bool bRecheck;  // line 799
	bool bForceEFNoInterp;  // line 809
	// inlined C_BasePlayer::IsLocalPlayer() at line 841
	{
		float flTimeDelta;  // line 820
		// inlined C_BaseEntity::SetSimulatedEveryTick() at line 817
		{
			Vector newVelo;  // line 829
			// inlined Vector::operator-() at line 829
			// inlined Vector::operator/() at line 829
		}
	}
	// inlined C_BasePlayer::IsLocalPlayer() at line 811
	{
		int i;  // line 788
		{
			int nIndex;  // line 790
		}
	}
	// inlined C_BaseEntity::SetSimulatedEveryTick() at line 813
	{
		QAngle angles;  // line 843
		{
			IGameEvent *pEvent;  // line 873
			ConVar *pVar;  // line 881
		}
		{
			IGameEvent *pEvent;  // line 860
			ConVar *pVar;  // line 868
			// inlined Vector::operator=() at line 854
		}
	}
}

// game/client/c_baseplayer.cpp:909 @0x2f7140 _ZN12C_BasePlayer16CanSetSoundMixerEv
bool C_BasePlayer::CanSetSoundMixer()
{
}

// game/client/c_baseplayer.cpp:915 @0x2f8100 _ZN12C_BasePlayer14ReceiveMessageEiR7bf_read
void C_BasePlayer::ReceiveMessage( int classID, bf_read &msg )
{
	int messageType;  // line 924
	// inlined CBitRead::ReadByte() at line 924
}

// game/client/c_baseplayer.cpp:934 @0x2fef20 _ZN12C_BasePlayer9OnRestoreEv
void C_BasePlayer::OnRestore()
{
	int ammoTypes;  // line 949
	{
		int i;  // line 951
	}
	// inlined CAmmoDef::NumAmmoTypes() at line 949
	// inlined C_BasePlayer::IsLocalPlayer() at line 938
	{
		CSetActiveSplitScreenPlayerGuard g_SSEGuard;  // line 940
	}
}

// game/client/c_baseplayer.cpp:960 @0x2fe340 _ZN12C_BasePlayer13OnDataChangedE16DataUpdateType_t
void C_BasePlayer::OnDataChanged( DataUpdateType_t updateType )
{
	{
		Vector vecAbsOrigin;  // line 1023
		Vector vecAbsVelocity;  // line 1024
		solid_t solid;  // line 1026
		// inlined C_BaseEntity::GetAbsVelocity() at line 1024
	}
	// inlined C_BasePlayer::IsLocalPlayer() at line 963
	// inlined C_BasePlayer::IsLocalPlayer() at line 972
	{
		int nSlot;  // line 974
		int ammoTypes;  // line 982
		// inlined C_BasePlayer::FogControllerChanged() at line 1013
		// inlined CHandle<C_BaseEntity>::operator!=() at line 1011
		// inlined CNetworkHandleBase<C_FogController,fogplayerparams_t::NetworkVar_m_hCtrl>::operator C_FogController*() at line 1011
		{
			CSetActiveSplitScreenPlayerGuard g_SSGuard;  // line 1007
		}
		{
			int i;  // line 983
			{
				CSetActiveSplitScreenPlayerGuard g_SSGuard;  // line 988
				const FileWeaponInfo_t *pWeaponData;  // line 991
				{
					CHudHistoryResource *pHudHR;  // line 996
				}
			}
		}
		// inlined CAmmoDef::NumAmmoTypes() at line 982
		// inlined C_BasePlayer::GetSplitScreenPlayerSlot() at line 974
	}
	{
		IPhysicsObject *pObject;  // line 1068
		// inlined C_BaseEntity::VPhysicsGetObject() at line 1068
		// inlined C_BaseEntity::GetAbsVelocity() at line 1076
	}
}

// game/client/c_baseplayer.cpp:1085 @0x2f8a00 _ZN12C_BasePlayer18JustEnteredVehicleEv
bool C_BasePlayer::JustEnteredVehicle()
{
	// inlined C_BasePlayer::IsInAVehicle() at line 1087
	// inlined CHandle<C_BaseEntity>::operator==() at line 1090
}

// game/client/c_baseplayer.cpp:1096 @0x2f8860 _ZNK12C_BasePlayer17IsInVGuiInputModeEv
bool C_BasePlayer::IsInVGuiInputMode()
{
	// inlined CHandle<C_BaseEntity>::Get() at line 1098
}

// game/client/c_baseplayer.cpp:1104 @0x2f87e0 _ZNK12C_BasePlayer26IsInViewModelVGuiInputModeEv
bool C_BasePlayer::IsInViewModelVGuiInputMode()
{
	C_BaseEntity *pScreenEnt;  // line 1106
	C_VGuiScreen *pVguiScreen;  // line 1112
	// inlined CHandle<C_BaseEntity>::Get() at line 1106
}

// game/client/c_baseplayer.cpp:1120 @0x2f9a20 _ZN12C_BasePlayer22DetermineVguiInputModeEP8CUserCmd
void C_BasePlayer::DetermineVguiInputMode( CUserCmd *pCmd )
{
	bool bAttacking;  // line 1155
	C_BaseEntity *pOldScreen;  // line 1185
	// inlined CHandle<C_BaseEntity>::Set() at line 1180
	// inlined CHandle<C_BaseEntity>::Get() at line 1179
	// inlined CHandle<C_BaseEntity>::operator C_BaseEntity*() at line 1189
	// inlined CHandle<C_BaseEntity>::operator=() at line 1187
	// inlined CHandle<C_BaseEntity>::Get() at line 1185
	// inlined vgui::surface() at line 1177
	// inlined C_BasePlayer::IsInAVehicle() at line 1169
	// inlined CHandle<C_BaseEntity>::Get() at line 1132
	// inlined CHandle<C_BaseEntity>::Get() at line 1156
	// inlined CHandle<C_BaseEntity>::Get() at line 1171
	// inlined CHandle<C_BaseEntity>::Set() at line 1172
	// inlined CHandle<C_BaseEntity>::Get() at line 1192
	// inlined CHandle<C_BaseEntity>::Get() at line 1195
}

// game/client/c_baseplayer.cpp:1207 @0x2f9d70 _ZN12C_BasePlayer10CreateMoveEfP8CUserCmd
bool C_BasePlayer::CreateMove( float flInputSampleTime, CUserCmd *pCmd )
{
	// inlined QAngle::operator=() at line 1258
	{
		C_BaseCombatWeapon *pWeapon;  // line 1231
	}
	// inlined C_BasePlayer::IsInAVehicle() at line 1210
	{
		IClientVehicle *pVehicle;  // line 1212
	}
	// inlined QAngle::operator=() at line 1245
}

// game/client/c_baseplayer.cpp:1270 @0x2f7160 _ZN12C_BasePlayer10TeamChangeEi
void C_BasePlayer::TeamChange( int iNewTeam )
{
}

// game/client/c_baseplayer.cpp:1279 @0x2ff0e0 _ZN12C_BasePlayer16UpdateFlashlightEv
void C_BasePlayer::UpdateFlashlight()
{
	int iSsPlayer;  // line 1282
	C_BasePlayer *pFlashlightPlayer;  // line 1285
	{
		Vector vecForward;  // line 1328
		Vector vecRight;  // line 1328
		Vector vecUp;  // line 1328
		Vector vecPos;  // line 1329
		// inlined CFlashlightEffectManager::UpdateFlashlight() at line 1347
		// inlined Vector::operator=() at line 1341
		// inlined Vector::operator+() at line 1341
		// inlined Vector::operator!=() at line 1331
		// inlined Vector::IsValid() at line 1331
		// inlined Vector::operator=() at line 1333
		// inlined Vector::operator=() at line 1334
		// inlined Vector::operator=() at line 1335
		// inlined Vector::operator=() at line 1336
	}
	// inlined CFlashlightEffectManager::SetEntityIndex() at line 1296
	// inlined ToBasePlayer() at line 1290
	// inlined C_BasePlayer::GetViewEntity() at line 1300
	{
		const char *pszTextureName;  // line 1303
		// inlined CFlashlightEffectManager::TurnOnFlashlight() at line 1314
		// inlined CFlashlightEffectManager::TurnOnFlashlight() at line 1310
	}
	// inlined CFlashlightEffectManager::TurnOffFlashlight() at line 1322
}

// game/client/c_baseplayer.cpp:1355 (declaration)
void Flashlight();

// game/client/c_baseplayer.cpp:1355 @0x2ff870 _ZN12C_BasePlayer10FlashlightEv
void C_BasePlayer::Flashlight()
{
}

// game/client/c_baseplayer.cpp:1364 @0x2ff020 _ZN12C_BasePlayer17TurnOffFlashlightEv
void C_BasePlayer::TurnOffFlashlight()
{
	int nSlot;  // line 1367
	// inlined CFlashlightEffectManager::TurnOffFlashlight() at line 1371
}

// game/client/c_baseplayer.cpp:1382 @0x2f8b20 _ZN12C_BasePlayer18CreateWaterEffectsEv
void C_BasePlayer::CreateWaterEffects()
{
	Vector vecVelocity;  // line 1409
	Vector offset;  // line 1412
	SimpleParticle *pParticle;  // line 1416
	float curTime;  // line 1418
	{
		float color;  // line 1442
		// inlined RandomVector() at line 1423
		// inlined Vector::operator*() at line 1423
		// inlined Vector::operator+() at line 1423
		// inlined Vector::operator+() at line 1423
		// inlined Vector::operator=() at line 1423
		// inlined RandomVector() at line 1439
		// inlined Vector::operator=() at line 1439
	}
	// inlined TimedEvent::NextEvent() at line 1421
	// inlined CSmartPtr<WaterDebrisEffect,CRefCountAccessor>::operator=() at line 1405
	// inlined TimedEvent::Init() at line 1395
}

// game/client/c_baseplayer.cpp:1461 @0x2f7170 _ZN12C_BasePlayer12OverrideViewEP10CViewSetup
void C_BasePlayer::OverrideView( CViewSetup *pSetup )
{
}

// game/client/c_baseplayer.cpp:1465 @0x2fe2d0 _ZN12C_BasePlayer17ShouldInterpolateEv
bool C_BasePlayer::ShouldInterpolate()
{
	// inlined C_BasePlayer::IsLocalPlayer() at line 1468
}

// game/client/c_baseplayer.cpp:1485 @0x2f8080 _ZN12C_BasePlayer10ShouldDrawEv
bool C_BasePlayer::ShouldDraw()
{
	// inlined GetSplitScreenViewPlayer() at line 1488
}

// game/client/c_baseplayer.cpp:1491 @0x2f96e0 _ZN12C_BasePlayer9DrawModelEiRK20RenderableInstance_t
int C_BasePlayer::DrawModel( int flags, const RenderableInstance_t &instance )
{
	C_BasePlayer *player;  // line 1494
	// inlined C_BasePlayer::IsObserver() at line 1496
	// inlined C_BasePlayer::GetLocalPlayer() at line 1494
}

// game/client/c_baseplayer.cpp:1510 @0x2f90c0 _ZN12C_BasePlayer19GetPlayerRenderModeEi
PlayerRenderMode_t C_BasePlayer::GetPlayerRenderMode( int nSlot )
{
	C_BasePlayer *pLocalPlayer;  // line 1513
	// inlined C_BasePlayer::IsObserver() at line 1517
}

// game/client/c_baseplayer.cpp:1543 @0x2f9890 _ZN12C_BasePlayer21GetChaseCamViewOffsetEP12C_BaseEntity
Vector C_BasePlayer::GetChaseCamViewOffset( C_BaseEntity *target )
{
	C_BasePlayer *player;  // line 1545
	// inlined ToBasePlayer() at line 1545
}

// game/client/c_baseplayer.cpp:1559 @0x2fd5c0 _ZN12C_BasePlayer16CalcChaseCamViewER6VectorR6QAngleRf
void C_BasePlayer::CalcChaseCamView( Vector &eyeOrigin, QAngle &eyeAngles, float &fov )
{
	C_BaseEntity *target;  // line 1561
	Vector forward;  // line 1582
	Vector viewpoint;  // line 1582
	Vector origin;  // line 1585
	QAngle viewangles;  // line 1589
	float flMaxDistance;  // line 1606
	trace_t trace;  // line 1620
	CTraceFilterNoNPCsOrPlayer filter;  // line 1621
	// inlined VectorCopy() at line 1567
	// inlined VectorCopy() at line 1566
	// inlined Vector::operator VectorByValue&() at line 1585
	// inlined VectorAdd() at line 1587
	// inlined C_BasePlayer::IsLocalPlayer() at line 1595
	// inlined QAngle::operator=() at line 1601
	// inlined QAngle::operator=() at line 1593
	// inlined VectorMA() at line 1618
	// inlined CTraceFilterNoNPCsOrPlayer::CTraceFilterNoNPCsOrPlayer() at line 1621
	// inlined UTIL_TraceHull() at line 1623
	// inlined VectorCopy() at line 1632
	// inlined VectorCopy() at line 1633
	// inlined Vector::operator=() at line 1628
	// inlined Vector::operator-() at line 1629
	// inlined VectorLength() at line 1629
}

// game/client/c_baseplayer.cpp:1638 @0x2f8650 _ZN12C_BasePlayer15CalcRoamingViewER6VectorR6QAngleRf
void C_BasePlayer::CalcRoamingView( Vector &eyeOrigin, QAngle &eyeAngles, float &fov )
{
	C_BaseEntity *target;  // line 1640
	Vector vSmoothOffset;  // line 1668
	// inlined Vector::operator=() at line 1649
	// inlined QAngle::operator=() at line 1650
	// inlined ConVar::GetInt() at line 1652
	{
		C_BaseEntity *target;  // line 1654
		{
			Vector v;  // line 1658
			QAngle a;  // line 1659
			// inlined Vector::operator-() at line 1659
			// inlined Vector::operator VectorByValue&() at line 1659
			// inlined QAngle::operator=() at line 1662
		}
	}
	// inlined Vector::operator+=() at line 1670
}

// game/client/c_baseplayer.cpp:1678 @0x2fb3d0 _ZN12C_BasePlayer17CalcFreezeCamViewER6VectorR6QAngleRf
void C_BasePlayer::CalcFreezeCamView( Vector &eyeOrigin, QAngle &eyeAngles, float &fov )
{
	C_BaseEntity *pTarget;  // line 1680
	float flCurTime;  // line 1688
	float flBlendPerc;  // line 1689
	Vector vecCamDesired;  // line 1692
	Vector vecCamTarget;  // line 1694
	Vector vecEyeOnPlane;  // line 1707
	Vector vecTargetPos;  // line 1709
	Vector vecToTarget;  // line 1710
	float flEyePosZ;  // line 1715
	trace_t trace;  // line 1719
	{
		IGameEvent *pEvent;  // line 1747
	}
	// inlined Vector::operator VectorByValue&() at line 1692
	// inlined VectorAdd() at line 1693
	{
		Vector maxs;  // line 1698
		// inlined GameRules() at line 1698
	}
	// inlined Vector::operator-() at line 1710
	// inlined Vector::operator*() at line 1714
	// inlined Vector::operator-() at line 1714
	// inlined Vector::operator=() at line 1714
	// inlined UTIL_TraceHull() at line 1721
	// inlined Vector::operator-() at line 1739
	// inlined Vector::operator=() at line 1739
	// inlined VectorLerp() at line 1743
	// inlined Vector::operator=() at line 1727
	// inlined UTIL_TraceHull() at line 1733
	// inlined Vector::operator=() at line 1735
}

// game/client/c_baseplayer.cpp:1758 @0x2fdc80 _ZN12C_BasePlayer16CalcInEyeCamViewER6VectorR6QAngleRf
void C_BasePlayer::CalcInEyeCamView( Vector &eyeOrigin, QAngle &eyeAngles, float &fov )
{
	C_BaseEntity *target;  // line 1760
	{
		Vector offset;  // line 1804
		// inlined Vector::operator+=() at line 1805
	}
	// inlined Vector::operator+=() at line 1799
	// inlined Vector::operator+=() at line 1795
	// inlined VectorAdd() at line 1785
	// inlined Vector::operator=() at line 1782
	// inlined QAngle::operator=() at line 1781
	// inlined VectorCopy() at line 1765
	// inlined VectorCopy() at line 1766
}

// game/client/c_baseplayer.cpp:1811 @0x2fbe30 _ZN12C_BasePlayer16CalcDeathCamViewER6VectorR6QAngleRf
void C_BasePlayer::CalcDeathCamView( Vector &eyeOrigin, QAngle &eyeAngles, float &fov )
{
	C_BaseEntity *pKiller;  // line 1813
	float interpolation;  // line 1822
	QAngle aForward;  // line 1828
	Vector origin;  // line 1829
	IRagdoll *pRagdoll;  // line 1831
	Vector vForward;  // line 1845
	trace_t trace;  // line 1851
	// inlined UTIL_TraceHull() at line 1853
	// inlined VectorMA() at line 1849
	// inlined Vector::operator=() at line 1834
	// inlined Vector::operator VectorByValue&() at line 1829
	{
		Vector vKiller;  // line 1840
		QAngle aKiller;  // line 1841
		// inlined Vector::operator-() at line 1840
	}
	// inlined QAngle::operator=() at line 1819
	// inlined Vector::operator=() at line 1858
	// inlined Vector::operator-() at line 1859
	// inlined VectorLength() at line 1859
}

// game/client/c_baseplayer.cpp:1871 @0x2f7180 _ZN12C_BasePlayer27GetActiveWeaponForSelectionEv
C_BaseCombatWeapon *C_BasePlayer::GetActiveWeaponForSelection()
{
}

// game/client/c_baseplayer.cpp:1876 @0x2fd550 _ZN12C_BasePlayer22GetRenderedWeaponModelEv
C_BaseAnimating *C_BasePlayer::GetRenderedWeaponModel()
{
	// inlined C_BasePlayer::IsLocalPlayer() at line 1879
}

// game/client/c_baseplayer.cpp:1893 (declaration)
void GetLocalPlayer( int nSlot );

// game/client/c_baseplayer.cpp:1893 @0x2f71a0 _ZN12C_BasePlayer14GetLocalPlayerEi
C_BasePlayer *C_BasePlayer::GetLocalPlayer( int nSlot )
{
}

// game/client/c_baseplayer.cpp:1903 @0x2f7d10 _ZN12C_BasePlayer45SetRemoteSplitScreenPlayerViewsAreLocalPlayerEb
void C_BasePlayer::SetRemoteSplitScreenPlayerViewsAreLocalPlayer( bool bSet )
{
	{
		int i;  // line 1905
	}
}

// game/client/c_baseplayer.cpp:1914 @0x2f7cb0 _ZN12C_BasePlayer17HasAnyLocalPlayerEv
bool C_BasePlayer::HasAnyLocalPlayer()
{
	{
		int i;  // line 1916
	}
}

// game/client/c_baseplayer.cpp:1924 @0x2f9860 _ZN12C_BasePlayer27GetSplitScreenSlotForPlayerEP12C_BaseEntity
int C_BasePlayer::GetSplitScreenSlotForPlayer( C_BaseEntity *pl )
{
	C_BasePlayer *pPlayer;  // line 1926
	// inlined C_BasePlayer::GetSplitScreenPlayerSlot() at line 1933
	// inlined ToBasePlayer() at line 1926
}

// game/client/c_baseplayer.cpp:1941 @0x2f7a60 _ZN12C_BasePlayer17ThirdPersonSwitchEb
void C_BasePlayer::ThirdPersonSwitch( bool bThirdperson )
{
}

// game/client/c_baseplayer.cpp:1950 @0x2f7b90 _ZN12C_BasePlayer21ShouldDrawLocalPlayerEv
bool C_BasePlayer::ShouldDrawLocalPlayer()
{
	int nSlot;  // line 1952
	CSetActiveSplitScreenPlayerGuard g_SSGuard;  // line 1959
	// inlined C_BasePlayer::GetSplitScreenPlayerSlot() at line 1952
}

// game/client/c_baseplayer.cpp:1966 @0x2fd410 _ZN12C_BasePlayer24GetClientModelRenderableEv
IClientModelRenderable *C_BasePlayer::GetClientModelRenderable()
{
	C_BasePlayer *localPlayer;  // line 1983
	// inlined C_BasePlayer::IsObserver() at line 1984
	// inlined C_BasePlayer::GetLocalPlayer() at line 1983
	// inlined C_BasePlayer::IsLocalPlayer() at line 1973
	{
		bool bThirdPerson;  // line 1975
	}
}

// game/client/c_baseplayer.cpp:2005 (declaration)
void IsLocalPlayer( const C_BaseEntity *pEntity );

// game/client/c_baseplayer.cpp:2005 @0x2f71f0 _ZN12C_BasePlayer13IsLocalPlayerEPK12C_BaseEntity
bool C_BasePlayer::IsLocalPlayer( const C_BaseEntity *pEntity )
{
}

// game/client/c_baseplayer.cpp:2014 @0x2f7230 _ZNK12C_BasePlayer9GetUserIDEv
int C_BasePlayer::GetUserID()
{
	player_info_t pi;  // line 2016
}

// game/client/c_baseplayer.cpp:2026 @0x2f72c0 _ZN12C_BasePlayer12SetAnimationE11PLAYER_ANIM
void C_BasePlayer::SetAnimation( PLAYER_ANIM playerAnim )
{
}

// game/client/c_baseplayer.cpp:2031 @0x2f72d0 _ZN12C_BasePlayer16UpdateClientDataEv
void C_BasePlayer::UpdateClientData()
{
	{
		int i;  // line 2034
	}
}

// game/client/c_baseplayer.cpp:2042 @0x2faf30 _ZN12C_BasePlayer8PreThinkEv
void C_BasePlayer::PreThink()
{
	// inlined C_BaseEntity::GetAbsVelocity() at line 2062
}

// game/client/c_baseplayer.cpp:2067 @0x2f8fd0 _ZN12C_BasePlayer9PostThinkEv
void C_BasePlayer::PostThink()
{
	CMDLCacheCriticalSection cacheCriticalSection;  // line 2070
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 2098
	// inlined CMDLCacheCriticalSection::CMDLCacheCriticalSection() at line 2070
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 2098
}

// game/client/c_baseplayer.cpp:2105 @0x2fcfa0 _ZN12C_BasePlayer21GetToolRecordingStateEP9KeyValues
void C_BasePlayer::GetToolRecordingState( KeyValues *msg )
{
	CVProfScope VProf_;  // line 2110
	float flZNear;  // line 2121
	float flZFar;  // line 2122
	// inlined CVProfScope::~CVProfScope() at line 2149
	{
		Vector cam_ofs;  // line 2132
		QAngle camAngles;  // line 2135
		Vector camForward;  // line 2140
		Vector camRight;  // line 2140
		Vector camUp;  // line 2140
		// inlined QAngle::operator[]() at line 2136
		// inlined VectorMA() at line 2143
		// inlined VectorCopy() at line 2146
	}
	// inlined C_BasePlayer::GetPlayerName() at line 2116
	// inlined KeyValues::SetBool() at line 2115
	// inlined C_BasePlayer::IsLocalPlayer() at line 2115
	// inlined KeyValues::SetBool() at line 2114
	// inlined CVProfScope::CVProfScope() at line 2110
	// inlined CVProfScope::~CVProfScope() at line 2149
	CameraRecordingState_t state;  // line 2118
}

// game/client/c_baseplayer.cpp:2156 @0x2ff880 _ZN12C_BasePlayer8SimulateEv
bool C_BasePlayer::Simulate()
{
	{
		Vector vel;  // line 2172
	}
	// inlined C_BasePlayer::IsLocalPlayer() at line 2159
	{
		CSetActiveSplitScreenPlayerGuard g_SSEGuard;  // line 2161
		// inlined C_BasePlayer::Flashlight() at line 2164
	}
}

// game/client/c_baseplayer.cpp:2191 @0x2f97a0 _ZN12C_BasePlayer12GetViewModelEi
C_BaseViewModel *C_BasePlayer::GetViewModel( int index )
{
	C_BaseViewModel *vm;  // line 2195
	// inlined CHandle<C_BaseViewModel>::operator C_BaseViewModel*() at line 2195
	{
		C_BasePlayer *target;  // line 2199
		// inlined ToBasePlayer() at line 2199
		// inlined C_BasePlayer::IsObserver() at line 2202
	}
}

// game/client/c_baseplayer.cpp:2211 @0x2fcf00 _ZNK12C_BasePlayer15GetActiveWeaponEv
C_BaseCombatWeapon *C_BasePlayer::GetActiveWeapon()
{
	const C_BasePlayer *fromPlayer;  // line 2213
	{
		C_BaseEntity *target;  // line 2218
		// inlined ToBasePlayer() at line 2222
	}
	// inlined C_BasePlayer::IsLocalPlayer() at line 2216
}

// game/client/c_baseplayer.cpp:2233 @0x2f8520 _ZN12C_BasePlayer16GetAutoaimVectorEf
Vector C_BasePlayer::GetAutoaimVector( float flScale )
{
	Vector forward;  // line 2236
	Vector forward;  // line 2236
	// inlined QAngle::operator+() at line 2237
	// inlined QAngle::operator QAngleByValue&() at line 2237
}

// game/client/c_baseplayer.cpp:2241 @0x2fa680 _ZN12C_BasePlayer16PlayPlayerJingleEv
void C_BasePlayer::PlayPlayerJingle()
{
	player_info_t info;  // line 2247
	char soundhex[16];  // line 2254
	char fullsoundname[512];  // line 2258
	CLocalPlayerFilter filter;  // line 2278
	EmitSound_t ep;  // line 2280
	{
		char custname[512];  // line 2263
	}
	// inlined CLocalPlayerFilter::~CLocalPlayerFilter() at line 2286
	// inlined EmitSound_t::~EmitSound_t() at line 2286
	// inlined EmitSound_t::EmitSound_t() at line 2280
	// inlined EmitSound_t::~EmitSound_t() at line 2286
	// inlined CLocalPlayerFilter::~CLocalPlayerFilter() at line 2286
}

// game/client/c_baseplayer.cpp:2290 @0x2f7320 _ZN12C_BasePlayer13SetSuitUpdateEPcii
void C_BasePlayer::SetSuitUpdate( char *name, int fgroup, int iNoRepeat )
{
}

// game/client/c_baseplayer.cpp:2298 @0x2f7330 _ZN12C_BasePlayer12ResetAutoaimEv
void C_BasePlayer::ResetAutoaim()
{
}

// game/client/c_baseplayer.cpp:2310 @0x2fcec0 _ZN12C_BasePlayer13ShouldPredictEv
bool C_BasePlayer::ShouldPredict()
{
	// inlined C_BasePlayer::IsLocalPlayer() at line 2314
}

// game/client/c_baseplayer.cpp:2325 @0x2f7340 _ZN12C_BasePlayer18GetPredictionOwnerEv
C_BasePlayer *C_BasePlayer::GetPredictionOwner()
{
}

// game/client/c_baseplayer.cpp:2334 @0x2fcbd0 _ZN12C_BasePlayer15PhysicsSimulateEv
void C_BasePlayer::PhysicsSimulate()
{
	CVProfScope VProf_;  // line 2337
	C_BaseEntity *pMoveParent;  // line 2339
	C_CommandContext *ctx;  // line 2354
	// inlined CVProfScope::CVProfScope() at line 2337
	// inlined C_BaseEntity::GetMoveParent() at line 2339
	// inlined C_BasePlayer::IsLocalPlayer() at line 2351
	// inlined CVProfScope::~CVProfScope() at line 2384
	// inlined MoveHelper() at line 2376
	// inlined MoveHelper() at line 2384
	// inlined CVProfScope::~CVProfScope() at line 2384
}

// game/client/c_baseplayer.cpp:2388 @0x2f7640 _ZN12C_BasePlayer13GetPunchAngleEv
const QAngle &C_BasePlayer::GetPunchAngle()
{
}

// game/client/c_baseplayer.cpp:2394 @0x2f7650 _ZN12C_BasePlayer13SetPunchAngleERK6QAngle
void C_BasePlayer::SetPunchAngle( const QAngle &angle )
{
	// inlined CNetworkVectorBase<QAngle,CPlayerLocalData::NetworkVar_m_vecPunchAngle>::operator=() at line 2396
}

// game/client/c_baseplayer.cpp:2400 @0x2f7350 _ZNK12C_BasePlayer16GetWaterJumpTimeEv
float C_BasePlayer::GetWaterJumpTime()
{
}

// game/client/c_baseplayer.cpp:2405 @0x2f7360 _ZN12C_BasePlayer16SetWaterJumpTimeEf
void C_BasePlayer::SetWaterJumpTime( float flWaterJumpTime )
{
}

// game/client/c_baseplayer.cpp:2410 @0x2f7380 _ZNK12C_BasePlayer16GetSwimSoundTimeEv
float C_BasePlayer::GetSwimSoundTime()
{
}

// game/client/c_baseplayer.cpp:2415 @0x2f7390 _ZN12C_BasePlayer16SetSwimSoundTimeEf
void C_BasePlayer::SetSwimSoundTime( float flSwimSoundTime )
{
}

// game/client/c_baseplayer.cpp:2424 @0x2f73b0 _ZN12C_BasePlayer15IsUseableEntityEP12C_BaseEntityj
bool C_BasePlayer::IsUseableEntity( C_BaseEntity *pEntity, unsigned int requiredCaps )
{
}

// game/client/c_baseplayer.cpp:2429 @0x2f8ad0 _ZNK12C_BasePlayer12GetUseEntityEv
C_BaseEntity *C_BasePlayer::GetUseEntity()
{
	// inlined CHandle<C_BaseEntity>::operator C_BaseEntity*() at line 2431
}

// game/client/c_baseplayer.cpp:2434 @0x2f73c0 _ZNK12C_BasePlayer21GetPotentialUseEntityEv
C_BaseEntity *C_BasePlayer::GetPotentialUseEntity()
{
}

// game/client/c_baseplayer.cpp:2444 @0x2fdf50 _ZNK12C_BasePlayer6GetFOVEv
float C_BasePlayer::GetFOV()
{
	float flDefaultFOV;  // line 2458
	IClientVehicle *pVehicle;  // line 2459
	float fFOV;  // line 2470
	{
		C_BasePlayer *pTargetPlayer;  // line 2448
		// inlined C_BasePlayer::IsObserver() at line 2451
		// inlined ToBasePlayer() at line 2448
	}
	{
		float deltaTime;  // line 2479
		// inlined C_BasePlayer::GetFinalPredictedTime() at line 2485
		// inlined SimpleSplineRemapValClamped() at line 2498
	}
	// inlined C_BasePlayer::IsLocalPlayer() at line 2477
	// inlined C_BasePlayer::GetVehicle() at line 2459
}

// game/client/c_baseplayer.cpp:2506 @0x2f7a70 _ZN12C_BasePlayer24RecvProxy_LocalVelocityXEPK14CRecvProxyDataPvS3_
void C_BasePlayer::RecvProxy_LocalVelocityX( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_BasePlayer *pPlayer;  // line 2508
	float flNewVel_x;  // line 2512
	Vector vecVelocity;  // line 2514
}

// game/client/c_baseplayer.cpp:2523 @0x2f85f0 _ZN12C_BasePlayer24RecvProxy_LocalVelocityYEPK14CRecvProxyDataPvS3_
void C_BasePlayer::RecvProxy_LocalVelocityY( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_BasePlayer *pPlayer;  // line 2525
	float flNewVel_y;  // line 2529
	Vector vecVelocity;  // line 2531
}

// game/client/c_baseplayer.cpp:2540 @0x2f8590 _ZN12C_BasePlayer24RecvProxy_LocalVelocityZEPK14CRecvProxyDataPvS3_
void C_BasePlayer::RecvProxy_LocalVelocityZ( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_BasePlayer *pPlayer;  // line 2542
	float flNewVel_z;  // line 2546
	Vector vecVelocity;  // line 2548
}

// game/client/c_baseplayer.cpp:2557 @0x2fdec0 _ZN12C_BasePlayer22RecvProxy_ObserverModeEPK14CRecvProxyDataPvS3_
void C_BasePlayer::RecvProxy_ObserverMode( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_BasePlayer *pPlayer;  // line 2561
	// inlined UpdateViewmodelVisibility() at line 2565
	// inlined C_BasePlayer::IsLocalPlayer() at line 2563
}

// game/client/c_baseplayer.cpp:2570 @0x2fc640 _ZN12C_BasePlayer24RecvProxy_ObserverTargetEPK14CRecvProxyDataPvS3_
void C_BasePlayer::RecvProxy_ObserverTarget( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_BasePlayer *pPlayer;  // line 2572
	EHANDLE hTarget;  // line 2576
	// inlined CHandle<C_BaseEntity>::CHandle() at line 2576
	// inlined CHandle<C_BaseEntity>::CHandle() at line 2580
}

// game/client/c_baseplayer.cpp:2583 @0x2f73e0 _ZN12C_BasePlayer23RecvProxy_LocalOriginXYEPK14CRecvProxyDataPvS3_
void C_BasePlayer::RecvProxy_LocalOriginXY( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
}

// game/client/c_baseplayer.cpp:2589 @0x2f7400 _ZN12C_BasePlayer22RecvProxy_LocalOriginZEPK14CRecvProxyDataPvS3_
void C_BasePlayer::RecvProxy_LocalOriginZ( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
}

// game/client/c_baseplayer.cpp:2594 @0x2f7410 _ZN12C_BasePlayer26RecvProxy_NonLocalOriginXYEPK14CRecvProxyDataPvS3_
void C_BasePlayer::RecvProxy_NonLocalOriginXY( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
}

// game/client/c_baseplayer.cpp:2600 @0x2f7430 _ZN12C_BasePlayer25RecvProxy_NonLocalOriginZEPK14CRecvProxyDataPvS3_
void C_BasePlayer::RecvProxy_NonLocalOriginZ( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
}

// game/client/c_baseplayer.cpp:2605 @0x2f88e0 _ZN12C_BasePlayer30RecvProxy_NonLocalCellOriginXYEPK14CRecvProxyDataPvS3_
void C_BasePlayer::RecvProxy_NonLocalCellOriginXY( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_BasePlayer *player;  // line 2607
	const int cellwidth;  // line 2612
}

// game/client/c_baseplayer.cpp:2617 @0x2f88b0 _ZN12C_BasePlayer29RecvProxy_NonLocalCellOriginZEPK14CRecvProxyDataPvS3_
void C_BasePlayer::RecvProxy_NonLocalCellOriginZ( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_BasePlayer *player;  // line 2619
	const int cellwidth;  // line 2623
}

// game/client/c_baseplayer.cpp:2630 @0x2f87d0 _ZN12C_BasePlayer12LeaveVehicleEv
void C_BasePlayer::LeaveVehicle()
{
}

// game/client/c_baseplayer.cpp:2670 @0x2f7440 _ZNK12C_BasePlayer9GetMinFOVEv
float C_BasePlayer::GetMinFOV()
{
}

// game/client/c_baseplayer.cpp:2683 (declaration)
void GetFinalPredictedTime();

// game/client/c_baseplayer.cpp:2683 @0x2f7470 _ZNK12C_BasePlayer21GetFinalPredictedTimeEv
float C_BasePlayer::GetFinalPredictedTime()
{
}

// game/client/c_baseplayer.cpp:2688 @0x2f8950 _ZN12C_BasePlayer19NotePredictionErrorERK6Vector
void C_BasePlayer::NotePredictionError( const Vector &vDelta )
{
	Vector vOldDelta;  // line 2695
	// inlined Vector::operator+() at line 2700
	// inlined Vector::operator=() at line 2700
}

// game/client/c_baseplayer.cpp:2713 @0x2f78a0 _ZN12C_BasePlayer38ForceSetupBonesAtTimeFakeInterpolationEP12matrix3x4a_tf
void C_BasePlayer::ForceSetupBonesAtTimeFakeInterpolation( matrix3x4a_t *pBonesOut, float curtimeOffset )
{
	float cycle;  // line 2716
	Vector origin;  // line 2717
	// inlined C_BaseAnimating::GetPlaybackRate() at line 2725
	// inlined CRangeCheckedVar<float,-0x00000000000000002,2,0>::operator=() at line 2725
	// inlined operator*() at line 2726
	// inlined Vector::operator+() at line 2726
	// inlined Vector::operator VectorByValue&() at line 2726
	// inlined CRangeCheckedVar<float,-0x00000000000000002,2,0>::operator=() at line 2730
}

// game/client/c_baseplayer.cpp:2734 @0x2fe1b0 _ZN12C_BasePlayer24GetRagdollInitBoneArraysEP12matrix3x4a_tS1_S1_f
void C_BasePlayer::GetRagdollInitBoneArrays( matrix3x4a_t *pDeltaBones0, matrix3x4a_t *pDeltaBones1, matrix3x4a_t *pCurrentBones, float boneDt )
{
	float ragdollCreateTime;  // line 2743
	// inlined C_BasePlayer::IsLocalPlayer() at line 2736
}

// game/client/c_baseplayer.cpp:2755 @0x2f74b0 _ZN12C_BasePlayer33GetPredictionErrorSmoothingVectorER6Vector
void C_BasePlayer::GetPredictionErrorSmoothingVector( Vector &vOffset )
{
	float errorAmount;  // line 2764
	// inlined Vector::Init() at line 2768
	// inlined Vector::operator*() at line 2774
	// inlined Vector::operator=() at line 2774
}

// game/client/c_baseplayer.cpp:2781 @0x2f75b0 _ZNK12C_BasePlayer24GetRepresentativeRagdollEv
IRagdoll *C_BasePlayer::GetRepresentativeRagdoll()
{
}

// game/client/c_baseplayer.cpp:2786 @0x2f7880 _ZN12C_BasePlayer20GetHeadLabelMaterialEv
IMaterial *C_BasePlayer::GetHeadLabelMaterial()
{
	// inlined CVoiceStatus::GetHeadLabelMaterial() at line 2791
}

// game/client/c_baseplayer.cpp:2794 @0x2f9680 _Z13IsInFreezeCamv
bool IsInFreezeCam()
{
	C_BasePlayer *pPlayer;  // line 2796
	// inlined C_BasePlayer::GetLocalPlayer() at line 2796
}

// game/client/c_baseplayer.cpp:2807 (declaration)
void FogControllerChanged( bool bSnap );

// game/client/c_baseplayer.cpp:2807 @0x2fafb0 _ZN12C_BasePlayer20FogControllerChangedEb
void C_BasePlayer::FogControllerChanged( bool bSnap )
{
	{
		fogparams_t *pFogParams;  // line 2811
		// inlined fogparams_t::operator=() at line 2838
	}
	// inlined CNetworkHandleBase<C_FogController,fogplayerparams_t::NetworkVar_m_hCtrl>::operator C_FogController*() at line 2809
}

// game/client/c_baseplayer.cpp:2848 @0x2faae0 _ZN12C_BasePlayer19UpdateFogControllerEv
void C_BasePlayer::UpdateFogController()
{
	// inlined CNetworkVarBase<bool,fogparams_t::NetworkVar_enable>::operator=<bool>() at line 2876
	// inlined CNetworkHandleBase<C_FogController,fogplayerparams_t::NetworkVar_m_hCtrl>::operator C_FogController*() at line 2850
	// inlined CNetworkVarBase<float,fogparams_t::NetworkVar_farz>::operator=<int>() at line 2875
	// inlined CHandle<C_BaseEntity>::operator==() at line 2853
	{
		fogparams_t *pFogParams;  // line 2855
		// inlined fogparams_t::operator=() at line 2866
	}
}

// game/client/c_baseplayer.cpp:2887 @0x2fa050 _ZN12C_BasePlayer14UpdateFogBlendEv
void C_BasePlayer::UpdateFogBlend()
{
	{
		float flTimeDelta;  // line 2892
		{
			float flScale;  // line 2895
			float newFarZ;  // line 2905
			float oldFarZ;  // line 2909
			// inlined CNetworkColor32Base<color32_s,fogparams_t::NetworkVar_colorPrimary>::SetR() at line 2896
			// inlined CNetworkColor32Base<color32_s,fogparams_t::NetworkVar_colorPrimary>::SetG() at line 2897
			// inlined CNetworkColor32Base<color32_s,fogparams_t::NetworkVar_colorPrimary>::SetB() at line 2898
			// inlined CNetworkVarBase<float,fogparams_t::NetworkVar_start>::Set() at line 2899
			// inlined CNetworkVarBase<float,fogparams_t::NetworkVar_end>::Set() at line 2900
			// inlined CNetworkVarBase<float,fogparams_t::NetworkVar_maxdensity>::Set() at line 2901
			// inlined CNetworkVarBase<float,fogparams_t::NetworkVar_HDRColorScale>::Set() at line 2902
			// inlined CNetworkVarBase<float,fogparams_t::NetworkVar_farz>::Set() at line 2913
		}
		// inlined CNetworkColor32Base<color32_s,fogparams_t::NetworkVar_colorPrimary>::SetR() at line 2918
		// inlined CNetworkColor32Base<color32_s,fogparams_t::NetworkVar_colorPrimary>::SetG() at line 2919
		// inlined CNetworkColor32Base<color32_s,fogparams_t::NetworkVar_colorPrimary>::SetB() at line 2920
		// inlined CNetworkVarBase<float,fogparams_t::NetworkVar_start>::Set() at line 2921
		// inlined CNetworkVarBase<float,fogparams_t::NetworkVar_end>::Set() at line 2922
		// inlined CNetworkVarBase<float,fogparams_t::NetworkVar_maxdensity>::Set() at line 2923
		// inlined CNetworkVarBase<float,fogparams_t::NetworkVar_HDRColorScale>::Set() at line 2924
		// inlined CNetworkVarBase<float,fogparams_t::NetworkVar_farz>::Set() at line 2925
	}
}

// game/client/c_baseplayer.cpp:2940 @0x2f76e0 _ZNK12C_BasePlayer30GetActivePostProcessControllerEv
C_PostProcessController *C_BasePlayer::GetActivePostProcessController()
{
	// inlined CNetworkHandleBase<C_PostProcessController,C_BasePlayer::NetworkVar_m_hPostProcessCtrl>::Get() at line 2942
}

// game/client/c_baseplayer.cpp:2948 @0x2f7730 _ZNK12C_BasePlayer24GetActiveColorCorrectionEv
C_ColorCorrection *C_BasePlayer::GetActiveColorCorrection()
{
	// inlined CNetworkHandleBase<C_ColorCorrection,C_BasePlayer::NetworkVar_m_hColorCorrectionCtrl>::Get() at line 2950
}

// game/client/c_baseplayer.cpp:2953 @0x2f77f0 _ZN12C_BasePlayer9PreRenderEi
bool C_BasePlayer::PreRender( int nSplitScreenPlayerSlot )
{
	// inlined C_BaseEntity::IsVisible() at line 2955
}

// game/client/c_baseplayer.cpp:2965 @0x2f75c0 _ZN12C_BasePlayer20IsSplitScreenPartnerEPS_
bool C_BasePlayer::IsSplitScreenPartner( C_BasePlayer *pPlayer )
{
	{
		int i;  // line 2970
	}
}

// game/client/c_baseplayer.cpp:2979 (declaration)
void GetSplitScreenPlayerSlot();

// game/client/c_baseplayer.cpp:2979 @0x2f7600 _ZN12C_BasePlayer24GetSplitScreenPlayerSlotEv
int C_BasePlayer::GetSplitScreenPlayerSlot()
{
}

// game/client/c_baseplayer.cpp:2984 @0x2f7610 _ZNK12C_BasePlayer19IsSplitScreenPlayerEv
bool C_BasePlayer::IsSplitScreenPlayer()
{
}

// game/client/c_baseplayer.cpp:2989 @0x2f7780 _ZNK12C_BasePlayer34ShouldRegenerateOriginFromCellBitsEv
bool C_BasePlayer::ShouldRegenerateOriginFromCellBits()
{
	int nSlot;  // line 2993
	int nIndex;  // line 2994
}

// game/client/c_baseplayer.cpp:3002 @0x2f91d0 _Z27CC_DumpClientSoundscapeDataRK8CCommand
CC_DumpClientSoundscapeData( const CCommand &args )
{
	C_BasePlayer *pPlayer;  // line 3004
	bool bFoundOne;  // line 3012
	// inlined C_BasePlayer::GetLocalPlayer() at line 3004
	{
		int i;  // line 3013
		{
			Vector vecPos;  // line 3023
		}
	}
}

// game/client/c_baseplayer.cpp:3030
static ConCommand soundscape_dumpclient;
