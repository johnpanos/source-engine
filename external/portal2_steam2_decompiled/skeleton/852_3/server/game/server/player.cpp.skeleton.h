// DWARF declaration skeleton for game/server/player.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xa72e0 _Z41__static_initialization_and_destruction_0ii
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
	// inlined CServerGameTags::CServerGameTags() at line 209
	// inlined ScriptClassDesc_t::ScriptClassDesc_t() at line 440
	// inlined CEntityFactory<CSprayCan>::CEntityFactory() at line 5405
	// inlined CResourcePrecacher::CResourcePrecacher() at line 5406
	// inlined CEntityFactory<CStripWeapons>::CEntityFactory() at line 7400
	// inlined CEntityFactory<CRevertSaved>::CEntityFactory() at line 7466
	// inlined CEntityFactory<CMovementSpeedMod>::CEntityFactory() at line 7571
	// inlined ServerClass::ServerClass() at line 7774
	// inlined CUserMessageThrottleMgr::CUserMessageThrottleMgr() at line 9055
}

// public/game/server/iplayerinfo.h:23 @0x63b490 _ZN7CBotCmdD0Ev
CBotCmd::~CBotCmd()
{
}

// public/game/server/iplayerinfo.h:23 @0x63b4b0 _ZN7CBotCmdD1Ev
CBotCmd::~CBotCmd()
{
}

// public/PlayerState.h:25 @0x63b310 _ZN12CPlayerState19NetworkStateChangedEv
void CPlayerState::NetworkStateChanged()
{
}

// public/PlayerState.h:25 @0x63b320 _ZN12CPlayerState19NetworkStateChangedEPv
void CPlayerState::NetworkStateChanged( void *pProp )
{
}

// public/PlayerState.h:29 @0x63b4d0 _ZN12CPlayerStateD0Ev
CPlayerState::~CPlayerState()
{
}

// public/PlayerState.h:29 @0x63b4f0 _ZN12CPlayerStateD1Ev
CPlayerState::~CPlayerState()
{
}

// game/server/util.h:53 @0x63b840 _Z21_CreateEntityTemplateI9CSprayCanEPT_S2_PKc
CSprayCan *_CreateEntityTemplate<CSprayCan>( CSprayCan *newEnt, const char *className )
{
	// inlined CSprayCan::CSprayCan() at line 55
}

// game/shared/playernet_vars.h:62 @0x63b2f0 _ZN17fogplayerparams_t19NetworkStateChangedEv
void fogplayerparams_t::NetworkStateChanged()
{
}

// game/shared/playernet_vars.h:62 @0x63b300 _ZN17fogplayerparams_t19NetworkStateChangedEPv
void fogplayerparams_t::NetworkStateChanged( void *pProp )
{
}

// game/server/player.cpp:92
ConVar autoaim_max_dist;

// game/server/player.cpp:93
ConVar autoaim_max_deflect;

// game/server/player.cpp:95
ConVar spec_freeze_time;

// game/server/player.cpp:96
ConVar spec_freeze_traveltime;

// game/server/player.cpp:98
ConVar sv_bonus_challenge;

// game/server/util.h:99 @0x63b6f0 _ZN14CEntityFactoryI13CStripWeaponsE6CreateEPKc
IServerNetworkable *CEntityFactory<CStripWeapons>::Create( const char *pClassName )
{
	CStripWeapons *pEnt;  // line 101
	// inlined _CreateEntityTemplate<CStripWeapons>() at line 101
}

// game/server/util.h:99 @0x63b760 _ZN14CEntityFactoryI12CRevertSavedE6CreateEPKc
IServerNetworkable *CEntityFactory<CRevertSaved>::Create( const char *pClassName )
{
	CRevertSaved *pEnt;  // line 101
	// inlined _CreateEntityTemplate<CRevertSaved>() at line 101
}

// game/server/util.h:99 @0x63b7d0 _ZN14CEntityFactoryI17CMovementSpeedModE6CreateEPKc
IServerNetworkable *CEntityFactory<CMovementSpeedMod>::Create( const char *pClassName )
{
	CMovementSpeedMod *pEnt;  // line 101
	// inlined _CreateEntityTemplate<CMovementSpeedMod>() at line 101
}

// game/server/util.h:99 @0x63b8b0 _ZN14CEntityFactoryI9CSprayCanE6CreateEPKc
IServerNetworkable *CEntityFactory<CSprayCan>::Create( const char *pClassName )
{
	CSprayCan *pEnt;  // line 101
}

// game/server/player.cpp:100
ConVar sv_regeneration_wait_time;

// public/tier1/utldict.h:101 @0xa7210 _ZN8CUtlDictI7CBitVecILi64EEiEC2Eiii
void CUtlDict<CBitVec<64>,int>::CUtlDict( int compareType, int growSize, int initSize )
{
	// inlined CUtlMap<const char*,CBitVec<64>,int>::SetLessFunc() at line 113
	// inlined CUtlMap<const char*,CBitVec<64>,int>::CUtlMap() at line 101
	// inlined CUtlMap<const char*,CBitVec<64>,int>::SetLessFunc() at line 105
	// inlined CUtlMap<const char*,CBitVec<64>,int>::SetLessFunc() at line 109
}

// game/server/player.cpp:102
static ConVar old_armor;

// game/server/util.h:105 @0x63b350 _ZN14CEntityFactoryI17CMovementSpeedModE7DestroyEP18IServerNetworkable
void CEntityFactory<CMovementSpeedMod>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:105 @0x63b390 _ZN14CEntityFactoryI12CRevertSavedE7DestroyEP18IServerNetworkable
void CEntityFactory<CRevertSaved>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:105 @0x63b3d0 _ZN14CEntityFactoryI13CStripWeaponsE7DestroyEP18IServerNetworkable
void CEntityFactory<CStripWeapons>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:105 @0x63b410 _ZN14CEntityFactoryI9CSprayCanE7DestroyEP18IServerNetworkable
void CEntityFactory<CSprayCan>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/player.cpp:108
ConVar sv_noclipduringpause;

// game/server/util.h:113 @0x63b380 _ZN14CEntityFactoryI17CMovementSpeedModE13GetEntitySizeEv
size_t CEntityFactory<CMovementSpeedMod>::GetEntitySize()
{
}

// game/server/util.h:113 @0x63b3c0 _ZN14CEntityFactoryI12CRevertSavedE13GetEntitySizeEv
size_t CEntityFactory<CRevertSaved>::GetEntitySize()
{
}

// game/server/util.h:113 @0x63b400 _ZN14CEntityFactoryI13CStripWeaponsE13GetEntitySizeEv
size_t CEntityFactory<CStripWeapons>::GetEntitySize()
{
}

// game/server/util.h:113 @0x63b440 _ZN14CEntityFactoryI9CSprayCanE13GetEntitySizeEv
size_t CEntityFactory<CSprayCan>::GetEntitySize()
{
}

// game/server/player.cpp:141
int gEvilImpulse101;

// game/server/player.cpp:143
bool gInitHUD;

// game/server/player.cpp:162
ConVar sk_player_head;

// game/server/player.cpp:163
ConVar sk_player_chest;

// game/server/player.cpp:164
ConVar sk_player_stomach;

// game/server/player.cpp:165
ConVar sk_player_arm;

// game/server/player.cpp:166
ConVar sk_player_leg;

// game/server/player.cpp:168
ConVar player_debug_print_damage;

// game/server/player.cpp:171 @0x623510 _Z18CC_GiveCurrentAmmov
CC_GiveCurrentAmmo()
{
	CBasePlayer *pPlayer;  // line 173
	{
		CBaseCombatWeapon *pWeapon;  // line 177
		{
			int ammoIndex;  // line 198
			{
				int giveAmount;  // line 202
			}
		}
		{
			int ammoIndex;  // line 183
			{
				int giveAmount;  // line 187
			}
		}
	}
}

// game/server/gameinterface.h:209 @0x6203f0 _ZL48__CreateCServerGameTagsIServerGameTags_interfacev
void *__CreateCServerGameTagsIServerGameTags_interface()
{
}

// game/server/player.cpp:210
static ConCommand givecurrentammo;

// game/server/player.cpp:214 (declaration)
void GetBaseMap();

// game/server/player.cpp:214 @0x620400 _ZN12CPlayerState10GetBaseMapEv
datamap_t *CPlayerState::GetBaseMap()
{
}

// game/server/player.cpp:214 @0xa6be0 _Z11DataMapInitI12CPlayerStateEP9datamap_tPT_
datamap_t *DataMapInit<CPlayerState>( CPlayerState * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 214
	CDatadescGeneratedNameHolder nameHolder;  // line 214
	typedescription_t dataDesc[3];  // line 214
}

// game/server/player.cpp:214
datamap_t *g_DataMapHolder;

// game/server/player.cpp:214
void CPlayerState::m_DataMap;

// game/server/player.cpp:231 (declaration)
void GetBaseMap();

// game/server/player.cpp:231 @0x620410 _ZN11CBasePlayer14GetDataDescMapEv
datamap_t *CBasePlayer::GetDataDescMap()
{
}

// game/server/player.cpp:231 @0x620420 _ZN11CBasePlayer10GetBaseMapEv
datamap_t *CBasePlayer::GetBaseMap()
{
}

// game/server/player.cpp:231 @0xa6d80 _Z11DataMapInitI11CBasePlayerEP9datamap_tPT_
datamap_t *DataMapInit<CBasePlayer>( CBasePlayer * )
{
	// inlined CUtlVectorDataopsInstantiator<13>::GetDataOps<CUtlVector<CHandle<CBaseEntity>, CUtlMemory<CHandle<CBaseEntity>, int> > >() at line 437
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 437
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 231
	CDatadescGeneratedNameHolder nameHolder;  // line 231
	typedescription_t dataDesc[113];  // line 231
}

// game/server/player.cpp:231
datamap_t *g_DataMapHolder;

// game/server/player.cpp:231
void CBasePlayer::m_DataMap;

// public/tier1/utldict.h:240 @0x63e840 _ZN8CUtlDictI7CBitVecILi64EEiE9RemoveAllEv
void CUtlDict<CBitVec<64>,int>::RemoveAll()
{
	int index;  // line 242
	// inlined CUtlMap<const char*,CBitVec<64>,int>::RemoveAll() at line 249
	// inlined CUtlMap<const char*,CBitVec<64>,int>::NextInorder() at line 246
	// inlined CUtlMap<const char*,CBitVec<64>,int>::FirstInorder() at line 242
}

// public/tier1/utllinkedlist.h:246 @0x63b8d0 _ZN14CUtlLinkedListI14CPlayerSimInfotLb0Et10CUtlMemoryI19UtlLinkedListElem_tIS0_tEtEED1Ev
CUtlLinkedList<CPlayerSimInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerSimInfo, short unsigned int>, short unsigned int> >::~CUtlLinkedList()
{
	// inlined CUtlLinkedList<CPlayerSimInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerSimInfo, short unsigned int>, short unsigned int> >::RemoveAll() at line 248
	// inlined CUtlMemory<UtlLinkedListElem_t<CPlayerSimInfo, short unsigned int>,short unsigned int>::~CUtlMemory() at line 248
}

// public/tier1/utllinkedlist.h:246 @0x63b990 _ZN14CUtlLinkedListI14CPlayerCmdInfotLb0Et10CUtlMemoryI19UtlLinkedListElem_tIS0_tEtEED1Ev
CUtlLinkedList<CPlayerCmdInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerCmdInfo, short unsigned int>, short unsigned int> >::~CUtlLinkedList()
{
	// inlined CUtlLinkedList<CPlayerCmdInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerCmdInfo, short unsigned int>, short unsigned int> >::RemoveAll() at line 248
	// inlined CUtlMemory<UtlLinkedListElem_t<CPlayerCmdInfo, short unsigned int>,short unsigned int>::~CUtlMemory() at line 248
}

// public/tier1/utldict.h:297 @0x63cb30 _ZNK8CUtlDictI7CBitVecILi64EEiE4FindEPKc
int CUtlDict<CBitVec<64>,int>::Find( const char *pName )
{
	// inlined CUtlMap<const char*,CBitVec<64>,int>::Find() at line 301
}

// game/server/player.cpp:440 (declaration)
ScriptClassDesc_t *GetScriptDesc<CBasePlayer>( CBasePlayer * );

// game/server/player.cpp:440 @0x620430 _Z13GetScriptDescI11CBasePlayerEP17ScriptClassDesc_tPT_
ScriptClassDesc_t *GetScriptDesc<CBasePlayer>( CBasePlayer * )
{
}

// game/server/player.cpp:440 @0x625010 _ZN11CBasePlayer13GetScriptDescEv
ScriptClassDesc_t *CBasePlayer::GetScriptDesc()
{
}

// game/server/player.cpp:440 @0xa68e0 _Z25InitCBasePlayerScriptDescv
InitCBasePlayerScriptDesc()
{
	ScriptClassDesc_t *pDesc;  // line 440
	ScriptClassDesc_t *pInstanceHelperBase;  // line 440
	{
		ScriptFunctionBinding_t *pBinding;  // line 441
		// inlined ScriptDeduceFunctionSignature<InitCBasePlayerScriptDesc()::_className*, CBasePlayer, bool>() at line 441
		// inlined CUtlVector<ScriptFunctionBinding_t,CUtlMemory<ScriptFunctionBinding_t, int> >::operator[]() at line 441
		// inlined CUtlVector<ScriptFunctionBinding_t,CUtlMemory<ScriptFunctionBinding_t, int> >::AddToTail() at line 441
	}
	bool bInitialized;  // line 440
}

// game/server/player.cpp:440
ScriptClassDesc_t g_CBasePlayer_ScriptDesc;

// game/server/player.cpp:441 sizeof=0x8 (i386)
struct <anonymous>
{
public:
	bool (*__pfn)(CBasePlayer *); // +0x0  // line 441
	int __delta; // +0x4  // line 441
};

// game/server/player.cpp:445
int giPrecacheGrunt;

// game/server/player.cpp:447
void CBasePlayer::s_PlayerEdict;

// public/tier1/utllinkedlist.h:461 @0x63c1e0 _ZN14CUtlLinkedListI14CPlayerCmdInfotLb0Et10CUtlMemoryI19UtlLinkedListElem_tIS0_tEtEE13AllocInternalEb
short unsigned int CUtlLinkedList<CPlayerCmdInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerCmdInfo, short unsigned int>, short unsigned int> >::AllocInternal( bool multilist )
{
	short unsigned int elem;  // line 467
	{
		CUtlMemory<UtlLinkedListElem_t<CPlayerCmdInfo, short unsigned int>,short unsigned int>::Iterator_t it;  // line 472
		// inlined CUtlMemory<UtlLinkedListElem_t<CPlayerCmdInfo, short unsigned int>,short unsigned int>::First() at line 480
		{
		}
		// inlined CUtlMemory<UtlLinkedListElem_t<CPlayerCmdInfo, short unsigned int>,short unsigned int>::IsValidIterator() at line 483
		// inlined CUtlMemory<UtlLinkedListElem_t<CPlayerCmdInfo, short unsigned int>,short unsigned int>::Next() at line 480
		// inlined CUtlMemory<UtlLinkedListElem_t<CPlayerCmdInfo, short unsigned int>,short unsigned int>::IsValidIterator() at line 480
		// inlined CUtlLinkedList<CPlayerCmdInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerCmdInfo, short unsigned int>, short unsigned int> >::ResetDbgInfo() at line 478
		// inlined CUtlMemory<UtlLinkedListElem_t<CPlayerCmdInfo, short unsigned int>,short unsigned int>::First() at line 472
		// inlined CUtlMemory<UtlLinkedListElem_t<CPlayerCmdInfo, short unsigned int>,short unsigned int>::IsValidIterator() at line 472
		// inlined CUtlMemory<UtlLinkedListElem_t<CPlayerCmdInfo, short unsigned int>,short unsigned int>::Next() at line 472
		// inlined CUtlMemory<UtlLinkedListElem_t<CPlayerCmdInfo, short unsigned int>,short unsigned int>::IsValidIterator() at line 474
		{
		}
	}
	// inlined CUtlLinkedList<CPlayerCmdInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerCmdInfo, short unsigned int>, short unsigned int> >::InternalElement() at line 504
	int __executeCount;  // line 485
	int __executeCount;  // line 493
}

// public/tier1/utllinkedlist.h:461 @0x63c450 _ZN14CUtlLinkedListI14CPlayerSimInfotLb0Et10CUtlMemoryI19UtlLinkedListElem_tIS0_tEtEE13AllocInternalEb
short unsigned int CUtlLinkedList<CPlayerSimInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerSimInfo, short unsigned int>, short unsigned int> >::AllocInternal( bool multilist )
{
	short unsigned int elem;  // line 467
	{
		CUtlMemory<UtlLinkedListElem_t<CPlayerSimInfo, short unsigned int>,short unsigned int>::Iterator_t it;  // line 472
		// inlined CUtlMemory<UtlLinkedListElem_t<CPlayerSimInfo, short unsigned int>,short unsigned int>::First() at line 480
		{
		}
		// inlined CUtlMemory<UtlLinkedListElem_t<CPlayerSimInfo, short unsigned int>,short unsigned int>::IsValidIterator() at line 483
		// inlined CUtlMemory<UtlLinkedListElem_t<CPlayerSimInfo, short unsigned int>,short unsigned int>::Next() at line 480
		// inlined CUtlMemory<UtlLinkedListElem_t<CPlayerSimInfo, short unsigned int>,short unsigned int>::IsValidIterator() at line 480
		// inlined CUtlLinkedList<CPlayerSimInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerSimInfo, short unsigned int>, short unsigned int> >::ResetDbgInfo() at line 478
		// inlined CUtlMemory<UtlLinkedListElem_t<CPlayerSimInfo, short unsigned int>,short unsigned int>::First() at line 472
		// inlined CUtlMemory<UtlLinkedListElem_t<CPlayerSimInfo, short unsigned int>,short unsigned int>::IsValidIterator() at line 472
		// inlined CUtlMemory<UtlLinkedListElem_t<CPlayerSimInfo, short unsigned int>,short unsigned int>::Next() at line 472
		// inlined CUtlMemory<UtlLinkedListElem_t<CPlayerSimInfo, short unsigned int>,short unsigned int>::IsValidIterator() at line 474
		{
		}
	}
	// inlined CUtlLinkedList<CPlayerSimInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerSimInfo, short unsigned int>, short unsigned int> >::InternalElement() at line 504
	int __executeCount;  // line 485
	int __executeCount;  // line 493
}

// game/server/player.cpp:466 (declaration)
void GetViewModel( int index );

// game/server/player.cpp:466 @0x620440 _ZN11CBasePlayer12GetViewModelEi
CBaseViewModel *CBasePlayer::GetViewModel( int index )
{
	// inlined CHandle<CBaseViewModel>::Get() at line 469
}

// game/server/player.cpp:475 @0x62ce50 _ZN11CBasePlayer15CreateViewModelEi
void CBasePlayer::CreateViewModel( int index )
{
	CBaseViewModel *vm;  // line 482
	// inlined CBasePlayer::GetViewModel() at line 479
	// inlined CBaseEntity::GetAbsOrigin() at line 485
	// inlined CHandle<CBaseViewModel>::CHandle() at line 490
	// inlined CBasePlayer::NetworkVar_m_hViewModel::Set() at line 490
}

// game/server/player.cpp:497 @0x6367f0 _ZN11CBasePlayer17DestroyViewModelsEv
void CBasePlayer::DestroyViewModels()
{
	int i;  // line 499
	{
		CBaseViewModel *vm;  // line 502
		// inlined CBasePlayer::GetViewModel() at line 502
		// inlined CHandle<CBaseViewModel>::CHandle() at line 507
		// inlined CBasePlayer::NetworkVar_m_hViewModel::Set() at line 507
	}
}

// public/vscript/vscript_templates.h:507 @0x63b690 _ZN21CMemberScriptBinding0IP11CBasePlayerMS0_FbvEbE4CallEPvS5_P15ScriptVariant_tiS7_
bool CMemberScriptBinding0<CBasePlayer*,bool (CBasePlayer::*)(),bool>::Call( ScriptFunctionBindingStorageType_t pFunction, void *pContext, ScriptVariant_t *pArguments, int nArguments, ScriptVariant_t *pReturn )
{
	// inlined ScriptConvertFuncPtrFromVoid<bool (CBasePlayer::*)()>() at line 507
	// inlined ScriptVariant_t::operator=() at line 507
}

// game/server/player.cpp:517 @0x622390 _ZN11CBasePlayer12CreatePlayerEPKcP7edict_t
CBasePlayer *CBasePlayer::CreatePlayer( const char *className, edict_t *ed )
{
	CBasePlayer *player;  // line 519
}

// game/server/player.cpp:530 (declaration)
void CBasePlayer();

// game/server/player.cpp:530 @0x639f30 _ZN11CBasePlayerC2Ev
CBasePlayer::CBasePlayer()
{
	// inlined CBasePlayer::NetworkVar_m_Local::NetworkVar_m_Local() at line 530
	// inlined CBasePlayer::NetworkVar_m_PlayerFog::NetworkVar_m_PlayerFog() at line 530
	// inlined CUtlVector<CHandle<CTonemapTrigger>,CUtlMemory<CHandle<CTonemapTrigger>, int> >::CUtlVector() at line 530
	// inlined CNetworkHandleBase<CPostProcessController,CBasePlayer::NetworkVar_m_hPostProcessCtrl>::CNetworkHandleBase() at line 530
	// inlined CNetworkHandleBase<CColorCorrection,CBasePlayer::NetworkVar_m_hColorCorrectionCtrl>::CNetworkHandleBase() at line 530
	// inlined CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> >::CUtlVector() at line 530
	// inlined CBasePlayer::NetworkVar_pl::NetworkVar_pl() at line 530
	// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hUseEntity>::CNetworkHandleBase() at line 530
	// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hVehicle>::CNetworkHandleBase() at line 530
	// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hObserverTarget>::CNetworkHandleBase() at line 530
	// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hZoomOwner>::CNetworkHandleBase() at line 530
	// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hTonemapController>::CNetworkHandleBase() at line 530
	// inlined CUtlVector<CCommandContext,CUtlMemory<CCommandContext, int> >::CUtlVector() at line 530
	// inlined CSimpleSimTimer::CSimpleSimTimer() at line 530
	// inlined CBasePlayer::NetworkVar_m_hViewModel::NetworkVar_m_hViewModel() at line 530
	// inlined CUserCmd::CUserCmd() at line 530
	// inlined CNetworkVarBase<CHandle<CBaseCombatWeapon>,CBasePlayer::NetworkVar_m_hLastWeapon>::CNetworkVarBase() at line 530
	// inlined CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> >::CUtlVector() at line 530
	// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hViewEntity>::CNetworkHandleBase() at line 530
	// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hConstraintEntity>::CNetworkHandleBase() at line 530
	// inlined CPlayerInfo::CPlayerInfo() at line 530
	// inlined CHandle<CBaseEntity>::CHandle() at line 530
	// inlined CBasePlayer::StepSoundCache_t::StepSoundCache_t() at line 530
	// inlined CUtlLinkedList<CPlayerSimInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerSimInfo, short unsigned int>, short unsigned int> >::CUtlLinkedList() at line 530
	// inlined CUtlLinkedList<CPlayerCmdInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerCmdInfo, short unsigned int>, short unsigned int> >::CUtlLinkedList() at line 530
	// inlined CHandle<CBaseCombatCharacter>::CHandle() at line 530
	// inlined CHandle<CBasePlayer>::CHandle() at line 530
	// inlined CUtlVector<CHandle<CBasePlayer>,CUtlMemory<CHandle<CBasePlayer>, int> >::CUtlVector() at line 530
	// inlined CBaseEntity::AddEFlags() at line 532
	// inlined QAngle::Init() at line 544
	// inlined CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth>::operator=<int>() at line 565
	// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hVehicle>::operator=() at line 570
	// inlined CNetworkVarBase<int,CBasePlayer::NetworkVar_m_iDefaultFOV>::operator=<int>() at line 574
	// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hZoomOwner>::operator=() at line 576
	// inlined CNetworkVarBase<float,CBasePlayer::NetworkVar_m_flLaggedMovementValue>::operator=<float>() at line 582
	// inlined CPlayerInfo::SetParent() at line 586
	// inlined Vector::Vector() at line 607
	// inlined Vector::operator=() at line 607
	// inlined CHandle<CBasePlayer>::operator=() at line 618
	// inlined CNetworkHandleBase<CPostProcessController,CBasePlayer::NetworkVar_m_hPostProcessCtrl>::Set() at line 620
	// inlined CNetworkHandleBase<CColorCorrection,CBasePlayer::NetworkVar_m_hColorCorrectionCtrl>::Set() at line 621
	// inlined CUtlVector<CHandle<CBasePlayer>,CUtlMemory<CHandle<CBasePlayer>, int> >::~CUtlVector() at line 622
	// inlined CPlayerInfo::~CPlayerInfo() at line 622
	// inlined CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> >::~CUtlVector() at line 622
	// inlined CUserCmd::~CUserCmd() at line 622
	// inlined CUtlVector<CCommandContext,CUtlMemory<CCommandContext, int> >::~CUtlVector() at line 622
	// inlined CBasePlayer::NetworkVar_pl::~NetworkVar_pl() at line 622
	// inlined CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> >::~CUtlVector() at line 622
	// inlined CUtlVector<CHandle<CTonemapTrigger>,CUtlMemory<CHandle<CTonemapTrigger>, int> >::~CUtlVector() at line 622
}

// game/server/player.cpp:530 @0x63adc0 _ZN11CBasePlayerC1Ev
CBasePlayer::CBasePlayer()
{
}

// game/server/player.h:588 @0x63b330 _ZN11CBasePlayer8CanSpeakEv
bool CBasePlayer::CanSpeak()
{
}

// public/tier1/utlrbtree.h:606 @0x63b540 _ZNK10CUtlRBTreeIN7CUtlMapIPKc7CBitVecILi64EEiE6Node_tEiNS5_8CKeyLessE10CUtlMemoryI15UtlRBTreeNode_tIS6_iEiEE5LinksEi
const UtlRBTreeLinks_t<int> &CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::Links( int i )
{
}

// game/server/player.cpp:624 (declaration)
~CBasePlayer();

// game/server/player.cpp:624 @0x62eda0 _ZN11CBasePlayerD0Ev
CBasePlayer::~CBasePlayer()
{
	// inlined CUtlVector<CHandle<CBasePlayer>,CUtlMemory<CHandle<CBasePlayer>, int> >::~CUtlVector() at line 627
	// inlined CPlayerInfo::~CPlayerInfo() at line 627
	// inlined CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> >::~CUtlVector() at line 627
	// inlined CUserCmd::~CUserCmd() at line 627
	// inlined CUtlVector<CCommandContext,CUtlMemory<CCommandContext, int> >::~CUtlVector() at line 627
	// inlined CBasePlayer::NetworkVar_pl::~NetworkVar_pl() at line 627
	// inlined CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> >::~CUtlVector() at line 627
	// inlined CUtlVector<CHandle<CTonemapTrigger>,CUtlMemory<CHandle<CTonemapTrigger>, int> >::~CUtlVector() at line 627
	// inlined CPlayerInfo::~CPlayerInfo() at line 627
	// inlined CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> >::~CUtlVector() at line 627
	// inlined CUserCmd::~CUserCmd() at line 627
	// inlined CUtlVector<CCommandContext,CUtlMemory<CCommandContext, int> >::~CUtlVector() at line 627
	// inlined CBasePlayer::NetworkVar_pl::~NetworkVar_pl() at line 627
	// inlined CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> >::~CUtlVector() at line 627
	// inlined CUtlVector<CHandle<CTonemapTrigger>,CUtlMemory<CHandle<CTonemapTrigger>, int> >::~CUtlVector() at line 627
	// inlined CUtlVector<CHandle<CBasePlayer>,CUtlMemory<CHandle<CBasePlayer>, int> >::~CUtlVector() at line 627
}

// game/server/player.cpp:624 @0x631950 _ZN11CBasePlayerD2Ev
CBasePlayer::~CBasePlayer()
{
	// inlined CUtlVector<CHandle<CBasePlayer>,CUtlMemory<CHandle<CBasePlayer>, int> >::~CUtlVector() at line 627
	// inlined CPlayerInfo::~CPlayerInfo() at line 627
	// inlined CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> >::~CUtlVector() at line 627
	// inlined CUserCmd::~CUserCmd() at line 627
	// inlined CUtlVector<CCommandContext,CUtlMemory<CCommandContext, int> >::~CUtlVector() at line 627
	// inlined CBasePlayer::NetworkVar_pl::~NetworkVar_pl() at line 627
	// inlined CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> >::~CUtlVector() at line 627
	// inlined CUtlVector<CHandle<CTonemapTrigger>,CUtlMemory<CHandle<CTonemapTrigger>, int> >::~CUtlVector() at line 627
	// inlined CPlayerInfo::~CPlayerInfo() at line 627
	// inlined CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> >::~CUtlVector() at line 627
	// inlined CUserCmd::~CUserCmd() at line 627
	// inlined CUtlVector<CCommandContext,CUtlMemory<CCommandContext, int> >::~CUtlVector() at line 627
	// inlined CBasePlayer::NetworkVar_pl::~NetworkVar_pl() at line 627
	// inlined CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> >::~CUtlVector() at line 627
	// inlined CUtlVector<CHandle<CTonemapTrigger>,CUtlMemory<CHandle<CTonemapTrigger>, int> >::~CUtlVector() at line 627
	// inlined CUtlVector<CHandle<CBasePlayer>,CUtlMemory<CHandle<CBasePlayer>, int> >::~CUtlVector() at line 627
}

// game/server/player.cpp:624 @0x6323d0 _ZN11CBasePlayerD1Ev
CBasePlayer::~CBasePlayer()
{
}

// game/server/player.cpp:634 @0x623410 _ZN11CBasePlayer14UpdateOnRemoveEv
void CBasePlayer::UpdateOnRemove()
{
	// inlined ScriptVariant_t::ScriptVariant_t() at line 638
	// inlined IScriptVM::SetValue() at line 638
	// inlined CHandle<CBasePlayer>::operator CBasePlayer*() at line 649
}

// public/tier1/utlvector.h:655 @0x63c5e0 _ZN10CUtlVectorI8CUserCmd10CUtlMemoryIS0_iEE10GrowVectorEi
void CUtlVector<CUserCmd,CUtlMemory<CUserCmd, int> >::GrowVector( int num )
{
	// inlined CUtlMemory<CUserCmd,int>::NumAllocated() at line 657
	// inlined CUtlMemory<CUserCmd,int>::Grow() at line 660
	// inlined CUtlVector<CUserCmd,CUtlMemory<CUserCmd, int> >::ResetDbgInfo() at line 664
}

// public/tier1/utlvector.h:655 @0x63c700 _ZN10CUtlVectorI15CCommandContext10CUtlMemoryIS0_iEE10GrowVectorEi
void CUtlVector<CCommandContext,CUtlMemory<CCommandContext, int> >::GrowVector( int num )
{
	// inlined CUtlMemory<CCommandContext,int>::NumAllocated() at line 657
	// inlined CUtlMemory<CCommandContext,int>::Grow() at line 660
	// inlined CUtlVector<CCommandContext,CUtlMemory<CCommandContext, int> >::ResetDbgInfo() at line 664
}

// public/tier1/utlrbtree.h:663 @0x63bed0 _ZN10CUtlRBTreeIN7CUtlMapIPKc7CBitVecILi64EEiE6Node_tEiNS5_8CKeyLessE10CUtlMemoryI15UtlRBTreeNode_tIS6_iEiEE7NewNodeEv
int CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::NewNode()
{
	int elem;  // line 665
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::ResetDbgInfo() at line 702
	// inlined Construct<CUtlMap<const char*, CBitVec<64>, int>::Node_t>() at line 701
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::Element() at line 701
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::Links() at line 692
	{
		CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>,int>::Iterator_t it;  // line 671
		// inlined CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>,int>::First() at line 677
		// inlined CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>,int>::Next() at line 671
		// inlined CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>,int>::IsValidIterator() at line 680
		// inlined CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>,int>::Next() at line 677
		// inlined CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>,int>::IsValidIterator() at line 677
		// inlined CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>,int>::Grow() at line 675
		// inlined CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>,int>::IsValidIterator() at line 672
		// inlined CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>,int>::First() at line 671
		// inlined CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>,int>::IsValidIterator() at line 671
	}
}

// game/server/player.cpp:663
ConVar VisForce;

// game/server/player.cpp:664 @0x627220 _ZN11CBasePlayer15SetupVisibilityEP11CBaseEntityPhi
void CBasePlayer::SetupVisibility( CBaseEntity *pViewEntity, unsigned char *pvs, int pvssize )
{
	Vector org;  // line 666
	{
		int i;  // line 676
		{
			CBasePlayer *pl;  // line 678
			// inlined ToBasePlayer() at line 678
			// inlined GetContainingEntity() at line 678
			// inlined CBaseEntity::entindex() at line 678
			// inlined Vector::operator=() at line 681
		}
	}
	// inlined Vector::operator=() at line 671
}

// game/server/player.cpp:686 @0x6233f0 _ZN11CBasePlayer19UpdateTransmitStateEv
int CBasePlayer::UpdateTransmitState()
{
}

// game/server/player.cpp:692 @0x627370 _ZN11CBasePlayer14ShouldTransmitEPK18CCheckTransmitInfo
int CBasePlayer::ShouldTransmit( const CCheckTransmitInfo *pInfo )
{
	{
		CBaseEntity *pRecipientEntity;  // line 712
		CBasePlayer *pRecipientPlayer;  // line 716
		// inlined CBaseEntity::Instance() at line 712
	}
	// inlined CBasePlayer::IsSplitScreenUserOnEdict() at line 696
	// inlined CHLTVDirector::GetCameraMan() at line 709
	// inlined CBaseEntity::entindex() at line 709
}

// public/tier1/utlmemory.h:707 @0x63c0f0 _ZN10CUtlMemoryI19UtlLinkedListElem_tI14CPlayerCmdInfotEtE4GrowEi
void CUtlMemory<UtlLinkedListElem_t<CPlayerCmdInfo, short unsigned int>,short unsigned int>::Grow( int num )
{
	int nAllocationRequested;  // line 720
	int nNewAllocationCount;  // line 724
	// inlined UtlMemory_CalcNewAllocationCount() at line 724
	// inlined CUtlMemory<UtlLinkedListElem_t<CPlayerCmdInfo, short unsigned int>,short unsigned int>::IsExternallyAllocated() at line 711
	// inlined MemAlloc_Alloc() at line 761
}

// public/tier1/utlmemory.h:707 @0x63c360 _ZN10CUtlMemoryI19UtlLinkedListElem_tI14CPlayerSimInfotEtE4GrowEi
void CUtlMemory<UtlLinkedListElem_t<CPlayerSimInfo, short unsigned int>,short unsigned int>::Grow( int num )
{
	int nAllocationRequested;  // line 720
	int nNewAllocationCount;  // line 724
	// inlined UtlMemory_CalcNewAllocationCount() at line 724
	// inlined CUtlMemory<UtlLinkedListElem_t<CPlayerSimInfo, short unsigned int>,short unsigned int>::IsExternallyAllocated() at line 711
	// inlined MemAlloc_Alloc() at line 761
}

// public/tier1/utlrbtree.h:724 @0x63d0b0 _ZN10CUtlRBTreeIN7CUtlMapIPKc7CBitVecILi64EEiE6Node_tEiNS5_8CKeyLessE10CUtlMemoryI15UtlRBTreeNode_tIS6_iEiEE10RotateLeftEi
void CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::RotateLeft( int elem )
{
	int rightchild;  // line 726
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::SetParent() at line 745
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::SetLeftChild() at line 743
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::SetLeftChild() at line 736
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::Parent() at line 736
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::IsLeftChild() at line 735
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::SetParent() at line 732
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::Parent() at line 732
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::SetParent() at line 729
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::LeftChild() at line 729
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::LeftChild() at line 728
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::SetRightChild() at line 727
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::LeftChild() at line 727
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::RightChild() at line 726
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::Parent() at line 738
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::SetRightChild() at line 738
}

// game/server/player.cpp:743 @0x6288d0 _ZNK11CBasePlayer28WantsLagCompensationOnEntityEPK11CBaseEntityPK8CUserCmdPK7CBitVecILi2048EE
bool CBasePlayer::WantsLagCompensationOnEntity( const CBaseEntity *entity, const CUserCmd *pCmd, const CBitVec<2048> *pEntityTransmitBits )
{
	const Vector &vMyOrigin;  // line 753
	const Vector &vHisOrigin;  // line 754
	float entityMaxSpeed;  // line 758
	float maxDistance;  // line 759
	Vector vForward;  // line 766
	Vector vDiff;  // line 769
	float flCosAngle;  // line 772
	// inlined Vector::operator-() at line 769
	// inlined Vector::DistTo() at line 762
	// inlined ToBasePlayer() at line 758
	// inlined CBaseEntity::GetAbsOrigin() at line 754
	// inlined CBaseEntity::GetAbsOrigin() at line 753
	// inlined CBaseEntity::entindex() at line 750
	// inlined ToBasePlayer() at line 758
}

// public/tier1/utlrbtree.h:754 @0x63d610 _ZN10CUtlRBTreeIN7CUtlMapIPKc7CBitVecILi64EEiE6Node_tEiNS5_8CKeyLessE10CUtlMemoryI15UtlRBTreeNode_tIS6_iEiEE11RotateRightEi
void CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::RotateRight( int elem )
{
	int leftchild;  // line 756
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::SetParent() at line 775
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::SetRightChild() at line 773
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::SetRightChild() at line 766
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::Parent() at line 766
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::IsRightChild() at line 765
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::SetParent() at line 762
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::Parent() at line 762
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::SetParent() at line 759
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::RightChild() at line 759
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::RightChild() at line 758
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::SetLeftChild() at line 757
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::RightChild() at line 757
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::LeftChild() at line 756
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::Parent() at line 768
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::SetLeftChild() at line 768
}

// game/server/player.cpp:779 @0x620490 _ZN11CBasePlayer18PauseBonusProgressEb
void CBasePlayer::PauseBonusProgress( bool bPause )
{
}

// game/server/player.cpp:784 @0x62daa0 _ZN11CBasePlayer16SetBonusProgressEi
void CBasePlayer::SetBonusProgress( int iBonusProgress )
{
	// inlined CNetworkVarBase<int,CBasePlayer::NetworkVar_m_iBonusProgress>::operator=<int>() at line 787
}

// public/tier1/utlrbtree.h:784 @0x63db70 _ZN10CUtlRBTreeIN7CUtlMapIPKc7CBitVecILi64EEiE6Node_tEiNS5_8CKeyLessE10CUtlMemoryI15UtlRBTreeNode_tIS6_iEiEE15InsertRebalanceEi
void CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::InsertRebalance( int elem )
{
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::SetColor() at line 850
	{
		int parent;  // line 788
		int grandparent;  // line 789
		{
			int uncle;  // line 823
			// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::Parent() at line 841
			// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::Parent() at line 840
			// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::SetColor() at line 827
			// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::IsRed() at line 824
			// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::LeftChild() at line 823
			// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::SetColor() at line 828
			// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::SetColor() at line 829
			// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::IsLeftChild() at line 835
			// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::SetColor() at line 844
			// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::SetColor() at line 845
		}
		{
			int uncle;  // line 794
			// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::SetColor() at line 798
			// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::IsRed() at line 795
			// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::RightChild() at line 794
			// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::IsRightChild() at line 806
			// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::SetColor() at line 815
			// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::SetColor() at line 816
			// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::Parent() at line 811
			// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::Parent() at line 812
		}
		// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::IsLeftChild() at line 792
		// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::Parent() at line 789
		// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::Parent() at line 788
	}
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::Color() at line 786
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::Parent() at line 786
}

// game/server/player.cpp:790 @0x62da40 _ZN11CBasePlayer17SetBonusChallengeEi
void CBasePlayer::SetBonusChallenge( int iBonusChallenge )
{
	// inlined CNetworkVarBase<int,CBasePlayer::NetworkVar_m_iBonusChallenge>::operator=<int>() at line 792
}

// game/server/player.cpp:799 (declaration)
void SnapEyeAngles( const QAngle &viewAngles );

// game/server/player.cpp:799 @0x6204b0 _ZN11CBasePlayer13SnapEyeAnglesERK6QAngle
void CBasePlayer::SnapEyeAngles( const QAngle &viewAngles )
{
	// inlined QAngle::operator=() at line 801
}

// game/server/player.h:811 @0x63ba40 _ZN11CBasePlayer18NetworkVar_m_Local19NetworkStateChangedEv
void CBasePlayer::NetworkVar_m_Local::NetworkStateChanged()
{
	// inlined DispatchNetworkStateChanged<CBasePlayer>() at line 811
}

// game/server/player.h:811 @0x63cab0 _ZN11CBasePlayer18NetworkVar_m_Local19NetworkStateChangedEPv
void CBasePlayer::NetworkVar_m_Local::NetworkStateChanged( void *pVar )
{
	// inlined DispatchNetworkStateChanged<CBasePlayer>() at line 811
}

// game/server/player.cpp:812 (declaration)
int TrainSpeed( int iSpeed, int iMax );

// game/server/player.cpp:812 @0x6204e0 _Z10TrainSpeedii
int TrainSpeed( int iSpeed, int iMax )
{
	float fSpeed;  // line 814
	float fMax;  // line 814
	int iRet;  // line 815
}

// game/server/player.h:813 @0x63bac0 _ZN11CBasePlayer22NetworkVar_m_PlayerFog19NetworkStateChangedEv
void CBasePlayer::NetworkVar_m_PlayerFog::NetworkStateChanged()
{
	// inlined DispatchNetworkStateChanged<CBasePlayer>() at line 813
}

// game/server/player.h:813 @0x63ca70 _ZN11CBasePlayer22NetworkVar_m_PlayerFog19NetworkStateChangedEPv
void CBasePlayer::NetworkVar_m_PlayerFog::NetworkStateChanged( void *pVar )
{
	// inlined DispatchNetworkStateChanged<CBasePlayer>() at line 813
}

// game/server/player.h:834 @0x63b450 _ZN11CBasePlayer13NetworkVar_plD0Ev
CBasePlayer::NetworkVar_pl::~NetworkVar_pl()
{
}

// game/server/player.h:834 @0x63b470 _ZN11CBasePlayer13NetworkVar_plD1Ev
CBasePlayer::NetworkVar_pl::~NetworkVar_pl()
{
}

// game/server/player.h:834 @0x63ba80 _ZN11CBasePlayer13NetworkVar_pl19NetworkStateChangedEv
void CBasePlayer::NetworkVar_pl::NetworkStateChanged()
{
	// inlined DispatchNetworkStateChanged<CBasePlayer>() at line 834
}

// game/server/player.h:834 @0x63caf0 _ZN11CBasePlayer13NetworkVar_pl19NetworkStateChangedEPv
void CBasePlayer::NetworkVar_pl::NetworkStateChanged( void *pVar )
{
	// inlined DispatchNetworkStateChanged<CBasePlayer>() at line 834
}

// game/server/player.cpp:836 @0x623af0 _ZN11CBasePlayer10DeathSoundERK15CTakeDamageInfo
void CBasePlayer::DeathSound( const CTakeDamageInfo &info )
{
}

// game/server/player.cpp:861 @0x6236d0 _ZN11CBasePlayer10TakeHealthEfi
int CBasePlayer::TakeHealth( float flHealth, int bitsDamageType )
{
	{
		int bitsDmgTimeBased;  // line 868
	}
}

// public/tier1/utlvector.h:865 @0x63bd80 _ZN10CUtlVectorI7CHandleI11CBaseEntityE10CUtlMemoryIS2_iEE8SetCountEi
void CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> >::SetCount( int count )
{
	// inlined CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> >::RemoveAll() at line 867
	// inlined CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> >::AddMultipleToTail() at line 868
}

// game/server/player.cpp:892 @0x623f30 _ZN11CBasePlayer25DrawDebugGeometryOverlaysEv
void CBasePlayer::DrawDebugGeometryOverlays()
{
	{
		Vector vBodyDir;  // line 899
		Vector eyePos;  // line 900
		Vector vUp;  // line 901
		Vector vSide;  // line 902
		// inlined Vector::operator VectorByValue&() at line 899
		// inlined Vector::operator*() at line 900
		// inlined Vector::operator+() at line 900
		// inlined Vector::Vector() at line 901
		// inlined CrossProduct() at line 903
		// inlined Vector::operator-() at line 904
		// inlined Vector::operator-() at line 904
		// inlined Vector::operator VectorByValue&() at line 904
		// inlined Vector::operator+() at line 904
		// inlined Vector::operator+() at line 904
		// inlined Vector::operator VectorByValue&() at line 904
		// inlined Vector::operator+() at line 905
		// inlined Vector::operator VectorByValue&() at line 905
		// inlined Vector::operator-() at line 905
		// inlined Vector::operator VectorByValue&() at line 905
	}
}

// game/server/player.cpp:913 @0x625020 _ZN11CBasePlayer11TraceAttackERK15CTakeDamageInfoRK6VectorP10CGameTrace
void CBasePlayer::TraceAttack( const CTakeDamageInfo &inputInfo, const Vector &vecDir, trace_t *ptr )
{
	{
		CTakeDamageInfo info;  // line 917
		// inlined CBaseCombatCharacter::SetLastHitGroup() at line 936
		// inlined CTakeDamageInfo::GetAttacker() at line 919
		// inlined CTakeDamageInfo::CTakeDamageInfo() at line 917
		{
			CAI_BaseNPC *pNPC;  // line 924
			// inlined CTakeDamageInfo::GetAttacker() at line 931
			// inlined CTakeDamageInfo::GetAttacker() at line 929
		}
		// inlined CTakeDamageInfo::ScaleDamage() at line 958
		// inlined CTakeDamageInfo::ScaleDamage() at line 954
		// inlined CTakeDamageInfo::ScaleDamage() at line 950
		// inlined CTakeDamageInfo::ScaleDamage() at line 947
		// inlined CTakeDamageInfo::ScaleDamage() at line 944
	}
}

// game/server/player.cpp:983 @0x623180 _ZN11CBasePlayer12DamageEffectEfi
void CBasePlayer::DamageEffect( float flDamage, int fDamageType )
{
	{
		color32 blue;  // line 994
	}
	{
		color32 red;  // line 988
	}
	{
		color32 blue;  // line 1005
		// inlined QAngle::QAngle() at line 1009
		// inlined QAngle::operator QAngleByValue&() at line 1009
	}
}

// game/server/player.cpp:1042 @0x623060 _ZN11CBasePlayer32ShouldTakeDamageInCommentaryModeERK15CTakeDamageInfo
bool CBasePlayer::ShouldTakeDamageInCommentaryMode( const CTakeDamageInfo &inputInfo )
{
	// inlined CTakeDamageInfo::GetInflictor() at line 1049
	// inlined CTakeDamageInfo::GetDamageType() at line 1053
	// inlined CTakeDamageInfo::GetAttacker() at line 1063
	// inlined CTakeDamageInfo::GetAttacker() at line 1049
}

// game/server/player.cpp:1069 @0x62a770 _ZN11CBasePlayer12OnTakeDamageERK15CTakeDamageInfo
int CBasePlayer::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	int bitsDamage;  // line 1072
	int ffound;  // line 1073
	int fmajor;  // line 1074
	int fcritical;  // line 1075
	int fTookDamage;  // line 1076
	int ftrivial;  // line 1077
	float flRatio;  // line 1078
	float flBonus;  // line 1079
	float flHealthPrev;  // line 1080
	CTakeDamageInfo info;  // line 1082
	IServerVehicle *pVehicle;  // line 1084
	float flPunch;  // line 1354
	// inlined CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth>::operator=<int>() at line 1105
	// inlined CTakeDamageInfo::GetDamageType() at line 1072
	// inlined CTakeDamageInfo::CTakeDamageInfo() at line 1082
	// inlined CTakeDamageInfo::GetDamage() at line 1103
	// inlined CTakeDamageInfo::GetAttacker() at line 1137
	{
		char dmgtype[64];  // line 1148
		char outputString[256];  // line 1150
		// inlined CTakeDamageInfo::GetInflictor() at line 1152
		// inlined CTakeDamageInfo::GetDamage() at line 1152
		// inlined CBaseEntity::GetAbsOrigin() at line 1152
		// inlined CBaseEntity::GetAbsOrigin() at line 1152
		// inlined CBaseEntity::GetAbsOrigin() at line 1152
	}
	{
		float flNew;  // line 1172
		float flArmor;  // line 1174
		// inlined CNetworkVarBase<int,CBasePlayer::NetworkVar_m_ArmorValue>::operator-=<float>() at line 1198
		// inlined CNetworkVarBase<int,CBasePlayer::NetworkVar_m_ArmorValue>::operator=<int>() at line 1193
		// inlined CTakeDamageInfo::SetDamage() at line 1201
	}
	// inlined CTakeDamageInfo::GetInflictor() at line 1221
	// inlined CBaseEntity::GetAbsOrigin() at line 1222
	// inlined Vector::operator=() at line 1222
	{
		int i;  // line 1227
		{
			int iDamage;  // line 1231
		}
	}
	// inlined CTakeDamageInfo::GetAttacker() at line 1356
	// inlined CNetworkVectorBase<QAngle,CPlayerLocalData::NetworkVar_m_vecPunchAngle>::SetX() at line 1364
}

// public/tier1/utlrbtree.h:1179 @0x63cca0 _ZNK10CUtlRBTreeIN7CUtlMapIPKc7CBitVecILi64EEiE6Node_tEiNS5_8CKeyLessE10CUtlMemoryI15UtlRBTreeNode_tIS6_iEiEE11NextInorderEi
int CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::NextInorder( int i )
{
	int parent;  // line 1191
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::Parent() at line 1196
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::IsRightChild() at line 1192
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::Parent() at line 1191
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::RightChild() at line 1183
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::RightChild() at line 1185
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::LeftChild() at line 1186
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::LeftChild() at line 1187
}

// game/server/player.h:1379 @0x63bb00 _Z13ForEachPlayerI15DisableAutokickEbRT_
bool ForEachPlayer<DisableAutokick>( DisableAutokick &func )
{
	CVProfScope VProf_;  // line 1381
	// inlined CVProfScope::CVProfScope() at line 1381
	{
		int i;  // line 1382
		{
			CBasePlayer *player;  // line 1384
			// inlined FNullEnt() at line 1389
			// inlined DisableAutokick::operator()() at line 1398
		}
	}
	// inlined CVProfScope::~CVProfScope() at line 1402
	// inlined CVProfScope::~CVProfScope() at line 1402
}

// game/server/player.cpp:1428 @0x62e1a0 _ZN11CBasePlayer20OnDamagedByExplosionERK15CTakeDamageInfo
void CBasePlayer::OnDamagedByExplosion( const CTakeDamageInfo &info )
{
	float lastDamage;  // line 1430
	float distanceFromPlayer;  // line 1432
	CBaseEntity *inflictor;  // line 1434
	bool ear_ringing;  // line 1441
	bool shock;  // line 1442
	int effect;  // line 1447
	CSingleUserRecipientFilter user;  // line 1451
	// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 1452
	// inlined CSingleUserRecipientFilter::CSingleUserRecipientFilter() at line 1451
	// inlined CTakeDamageInfo::GetDamage() at line 1430
	// inlined CTakeDamageInfo::GetInflictor() at line 1434
	{
		Vector delta;  // line 1437
		// inlined Vector::Length() at line 1438
		// inlined CBaseEntity::GetAbsOrigin() at line 1437
		// inlined CBaseEntity::GetAbsOrigin() at line 1437
		// inlined Vector::operator-() at line 1437
	}
	// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 1452
}

// game/server/player.cpp:1462 @0x621cb0 _ZN11CBasePlayer19PackDeadPlayerItemsEv
void CBasePlayer::PackDeadPlayerItems()
{
	int iWeaponRules;  // line 1464
	int iAmmoRules;  // line 1465
	int i;  // line 1466
	CBaseCombatWeapon *rgpPackWeapons[20];  // line 1467
	int iPackAmmo[33];  // line 1468
	int iPW;  // line 1469
	int iPA;  // line 1470
	{
		CBaseCombatWeapon *pPlayerItem;  // line 1490
	}
}

// public/tier1/utlrbtree.h:1483 @0x63e600 _ZN10CUtlRBTreeIN7CUtlMapIPKc7CBitVecILi64EEiE6Node_tEiNS5_8CKeyLessE10CUtlMemoryI15UtlRBTreeNode_tIS6_iEiEE6InsertERKS6_
int CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::Insert( const CUtlMap<const char*,CBitVec<64>,int>::Node_t &insert )
{
	int parent;  // line 1486
	bool leftchild;  // line 1487
	int newNode;  // line 1489
	// inlined CopyConstruct<CUtlMap<const char*, CBitVec<64>, int>::Node_t>() at line 1490
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::Element() at line 1490
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::InsertAt() at line 1489
	// inlined CUtlRBTree<CUtlMap<const char*, CBitVec<64>, int>::Node_t,int,CUtlMap<const char*, CBitVec<64>, int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const char*, CBitVec<64>, int>::Node_t, int>, int> >::FindInsertionPosition() at line 1488
}

// game/server/player.cpp:1553 @0x6382d0 _ZN11CBasePlayer14RemoveAllItemsEb
void CBasePlayer::RemoveAllItems( bool removeSuit )
{
	// inlined CBasePlayer::ResetAutoaim() at line 1557
}

// game/server/player.cpp:1573 (declaration)
void IsDead();

// game/server/player.cpp:1573 @0x620540 _ZNK11CBasePlayer6IsDeadEv
bool CBasePlayer::IsDead()
{
}

// game/server/player.cpp:1578 (declaration)
float DamageForce( const Vector &size, float damage );

// game/server/player.cpp:1591 @0x620560 _ZN11CBasePlayer27GetPhysicsImpactDamageTableEv
const impactdamagetable_t &CBasePlayer::GetPhysicsImpactDamageTable()
{
}

// game/server/player.cpp:1596
static int g_PlayerHurtEvent;

// game/server/player.cpp:1597 @0x62a200 _ZN11CBasePlayer18OnTakeDamage_AliveERK15CTakeDamageInfo
int CBasePlayer::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	CBaseEntity *attacker;  // line 1605
	Vector vecDir;  // line 1610
	IGameEvent *event;  // line 1630
	// inlined CBaseEntity::GetAbsOrigin() at line 1653
	{
		CBasePlayer *player;  // line 1639
		// inlined ToBasePlayer() at line 1639
		// inlined CBasePlayer::GetUserID() at line 1640
	}
	// inlined CBasePlayer::GetUserID() at line 1633
	// inlined CTakeDamageInfo::GetInflictor() at line 1617
	// inlined CTakeDamageInfo::GetInflictor() at line 1611
	// inlined CTakeDamageInfo::GetAttacker() at line 1605
	{
		Vector force;  // line 1620
		// inlined Vector::operator*() at line 1620
		// inlined DamageForce() at line 1620
		// inlined CBaseEntity::WorldAlignSize() at line 1620
		// inlined CTakeDamageInfo::GetBaseDamage() at line 1620
	}
	// inlined Vector::Vector() at line 1613
	// inlined Vector::operator VectorByValue&() at line 1613
	// inlined CTakeDamageInfo::GetInflictor() at line 1613
	// inlined Vector::operator-() at line 1613
	// inlined Vector::operator-() at line 1613
	// inlined Vector::operator=() at line 1613
}

// game/server/player.cpp:1660 @0x62d760 _ZN11CBasePlayer12Event_KilledERK15CTakeDamageInfo
void CBasePlayer::Event_Killed( const CTakeDamageInfo &info )
{
	CSound *pSound;  // line 1662
	IPhysicsObject *pObject;  // line 1709
	// inlined CNetworkVarBase<float,CBasePlayer::NetworkVar_m_flDeathTime>::operator=<float>() at line 1729
	// inlined CBaseEntity::VPhysicsGetObject() at line 1709
	// inlined CBaseEntity::AddSolidFlags() at line 1706
	// inlined CNetworkVarBase<bool,CPlayerState::NetworkVar_deadflag>::operator=<bool>() at line 1705
	// inlined CNetworkVarBase<char,CBaseEntity::NetworkVar_m_lifeState>::operator=<int>() at line 1703
	// inlined CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth>::operator=<int>() at line 1691
}

// game/server/player.cpp:1737 @0x622f70 _ZN11CBasePlayer11Event_DyingEv
void CBasePlayer::Event_Dying()
{
	CTakeDamageInfo info;  // line 1741
	QAngle angles;  // line 1750
}

// game/server/player.cpp:1764 @0x6323e0 _ZN11CBasePlayer12SetAnimationE11PLAYER_ANIM
void CBasePlayer::SetAnimation( PLAYER_ANIM playerAnim )
{
	int animDesired;  // line 1766
	char szAnim[64];  // line 1767
	float speed;  // line 1769
	$_178 idealActivity;  // line 1779
	// inlined CBasePlayer::SetActivity() at line 1858
	// inlined CBaseAnimating::SetCycle() at line 1849
	// inlined CBasePlayer::SetActivity() at line 1877
	// inlined CBaseEntity::GetAbsVelocity() at line 1771
	// inlined Vector::Length2D() at line 1771
	// inlined CBaseEntity::GetFlags() at line 1773
	// inlined CBasePlayer::SetActivity() at line 1889
	// inlined CBaseAnimating::SetCycle() at line 1909
}

// game/server/player.cpp:1932 @0x626f20 _ZN11CBasePlayer9WaterMoveEv
void CBasePlayer::WaterMove()
{
	// inlined CBaseEntity::GetMoveParent() at line 1934
	// inlined INDEXENT() at line 1998
	// inlined GetContainingEntity() at line 1998
	// inlined INDEXENT() at line 1998
	// inlined GetContainingEntity() at line 1998
}

// game/server/player.cpp:2018 @0x624380 _ZN11CBasePlayer10IsOnLadderEv
bool CBasePlayer::IsOnLadder()
{
}

// game/server/player.cpp:2024 @0x620570 _ZNK11CBasePlayer16GetWaterJumpTimeEv
float CBasePlayer::GetWaterJumpTime()
{
}

// game/server/player.cpp:2029 @0x620580 _ZN11CBasePlayer16SetWaterJumpTimeEf
void CBasePlayer::SetWaterJumpTime( float flWaterJumpTime )
{
}

// game/server/player.cpp:2034 @0x6205a0 _ZNK11CBasePlayer16GetSwimSoundTimeEv
float CBasePlayer::GetSwimSoundTime()
{
}

// game/server/player.cpp:2039 @0x6205b0 _ZN11CBasePlayer16SetSwimSoundTimeEf
void CBasePlayer::SetSwimSoundTime( float flSwimSoundTime )
{
}

// game/server/player.cpp:2044 @0x62ec40 _ZN11CBasePlayer17ShowViewPortPanelEPKcbP9KeyValues
void CBasePlayer::ShowViewPortPanel( const char *name, bool bShow, KeyValues *data )
{
	CSingleUserRecipientFilter filter;  // line 2046
	int count;  // line 2049
	KeyValues *subkey;  // line 2050
	// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 2075
	// inlined CSingleUserRecipientFilter::CSingleUserRecipientFilter() at line 2046
	// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 2075
}

// game/server/player.cpp:2079 @0x631280 _ZN11CBasePlayer16PlayerDeathThinkEv
void CBasePlayer::PlayerDeathThink()
{
	float flForward;  // line 2081
	int fAnyButtonDown;  // line 2130
	// inlined CNetworkVarBase<char,CBaseEntity::NetworkVar_m_lifeState>::operator=<int>() at line 2146
	// inlined CBaseEntity::GetAbsVelocity() at line 2087
	// inlined Vector::Length() at line 2087
	{
		Vector vecNewVelocity;  // line 2094
		// inlined Vector::operator*=() at line 2096
		// inlined CBaseEntity::GetAbsVelocity() at line 2094
	}
	// inlined CBasePlayer::HasWeapons() at line 2101
	// inlined CBaseAnimating::StopAnimation() at line 2125
	// inlined CNetworkVarBase<float,CBaseAnimating::NetworkVar_m_flPlaybackRate>::operator=<double>() at line 2128
	// inlined CNetworkVarBase<char,CBaseEntity::NetworkVar_m_lifeState>::operator=<int>() at line 2121
}

// game/server/player.cpp:2230 @0x634b80 _ZN11CBasePlayer16StopObserverModeEv
void CBasePlayer::StopObserverMode()
{
	// inlined CNetworkVarBase<unsigned int,CBasePlayer::NetworkVar_m_afPhysicsFlags>::operator&=<int>() at line 2233
	// inlined CNetworkVarBase<int,CBasePlayer::NetworkVar_m_iObserverMode>::Set() at line 2243
}

// game/server/player.cpp:2250 @0x636270 _ZN11CBasePlayer17StartObserverModeEi
bool CBasePlayer::StartObserverMode( int mode )
{
	// inlined CBasePlayer::IsObserver() at line 2252
	// inlined CNetworkVarBase<unsigned int,CBasePlayer::NetworkVar_m_afPhysicsFlags>::operator|=<PlayerPhysFlag_e>() at line 2261
	// inlined CBaseEntity::AddSolidFlags() at line 2274
	// inlined CNetworkVarBase<int,CPlayerLocalData::NetworkVar_m_iHideHUD>::operator=<int>() at line 2284
	// inlined CNetworkVarBase<char,CBaseEntity::NetworkVar_m_takedamage>::operator=<int>() at line 2285
	// inlined CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth>::operator=<int>() at line 2290
	// inlined CNetworkVarBase<char,CBaseEntity::NetworkVar_m_lifeState>::operator=<int>() at line 2291
	// inlined CNetworkVarBase<bool,CPlayerState::NetworkVar_deadflag>::operator=<bool>() at line 2293
	// inlined CBaseEntity::GetAbsOrigin() at line 2255
	// inlined Vector::operator+() at line 2255
	// inlined Vector::operator VectorByValue&() at line 2255
}

// game/server/player.cpp:2298 @0x6316f0 _ZN11CBasePlayer15SetObserverModeEi
bool CBasePlayer::SetObserverMode( int mode )
{
	// inlined CNetworkVarBase<int,CBasePlayer::NetworkVar_m_iObserverMode>::operator=<int>() at line 2321
	// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hObserverTarget>::operator CBaseEntity*() at line 2336
	// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hObserverTarget>::operator CBaseEntity*() at line 2342
}

// game/server/player.cpp:2354 @0x623640 _ZN11CBasePlayer15GetObserverModeEv
int CBasePlayer::GetObserverMode()
{
}

// game/server/player.cpp:2359 @0x623650 _ZN11CBasePlayer17ForceObserverModeEi
void CBasePlayer::ForceObserverMode( int mode )
{
	int tempMode;  // line 2361
}

// game/server/player.cpp:2383 @0x62b810 _ZN11CBasePlayer21CheckObserverSettingsEv
void CBasePlayer::CheckObserverSettings()
{
	{
		CBaseEntity *target;  // line 2388
		// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hObserverTarget>::operator CBaseEntity*() at line 2388
	}
	{
		CBasePlayer *target;  // line 2425
		// inlined CNetworkHandleBase<CFogController,fogplayerparams_t::NetworkVar_m_hCtrl>::Set() at line 2453
		// inlined CNetworkHandleBase<CFogController,fogplayerparams_t::NetworkVar_m_hCtrl>::Get() at line 2453
		// inlined CNetworkHandleBase<CFogController,fogplayerparams_t::NetworkVar_m_hCtrl>::Get() at line 2451
		// inlined CNetworkHandleBase<CFogController,fogplayerparams_t::NetworkVar_m_hCtrl>::Get() at line 2451
		// inlined ToBasePlayer() at line 2425
		// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hObserverTarget>::Get() at line 2425
		{
			int flagMask;  // line 2431
			int flags;  // line 2433
			// inlined CBaseEntity::GetFlags() at line 2435
			// inlined Vector::operator!=() at line 2442
		}
	}
}

// game/server/player.cpp:2462 @0x637e70 _ZN11CBasePlayer29ValidateCurrentObserverTargetEv
void CBasePlayer::ValidateCurrentObserverTarget()
{
	// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hObserverTarget>::Get() at line 2464
	{
		CBaseEntity *target;  // line 2467
		// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hObserverTarget>::Set() at line 2485
	}
}

// game/server/player.cpp:2494 @0x6205d0 _ZN11CBasePlayer22AttemptToExitFreezeCamEv
void CBasePlayer::AttemptToExitFreezeCam()
{
}

// game/server/player.cpp:2499 @0x623cf0 _ZN11CBasePlayer15StartReplayModeEffi
bool CBasePlayer::StartReplayMode( float fDelay, float fDuration, int iEntity )
{
}

// game/server/player.cpp:2511 @0x6205f0 _ZN11CBasePlayer14StopReplayModeEv
void CBasePlayer::StopReplayMode()
{
}

// game/server/player.cpp:2518 @0x620620 _ZN11CBasePlayer13GetDelayTicksEv
int CBasePlayer::GetDelayTicks()
{
}

// game/server/player.cpp:2533 @0x6206a0 _ZN11CBasePlayer15GetReplayEntityEv
int CBasePlayer::GetReplayEntity()
{
}

// game/server/player.cpp:2538 @0x624950 _ZN11CBasePlayer17GetObserverTargetEv
CBaseEntity *CBasePlayer::GetObserverTarget()
{
	// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hObserverTarget>::Get() at line 2540
}

// game/server/player.cpp:2543 @0x624820 _ZN11CBasePlayer11ObserverUseEb
void CBasePlayer::ObserverUse( bool bIsPressed )
{
	bool bIsHLTV;  // line 2559
	{
		int iCameraManIndex;  // line 2566
		// inlined CBaseEntity::entindex() at line 2571
		// inlined CHLTVDirector::GetCameraMan() at line 2566
		// inlined CBaseEntity::entindex() at line 2573
	}
}

// game/server/player.cpp:2635 @0x624530 _ZN11CBasePlayer14JumptoPositionERK6VectorRK6QAngle
void CBasePlayer::JumptoPosition( const Vector &origin, const QAngle &angles )
{
	// inlined CBasePlayer::SnapEyeAngles() at line 2640
}

// game/server/player.cpp:2643 @0x62d0d0 _ZN11CBasePlayer17SetObserverTargetEP11CBaseEntity
bool CBasePlayer::SetObserverTarget( CBaseEntity *target )
{
	{
		Vector dir;  // line 2656
		Vector end;  // line 2656
		Vector start;  // line 2657
		Ray_t ray;  // line 2663
		trace_t tr;  // line 2666
		CMDLCacheCriticalSection cacheCriticalSection;  // line 2667
		// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 2670
		// inlined UTIL_TraceRay() at line 2668
		// inlined CMDLCacheCriticalSection::CMDLCacheCriticalSection() at line 2667
		// inlined Ray_t::Init() at line 2664
		// inlined Ray_t::Ray_t() at line 2663
		// inlined VectorMA() at line 2661
		// inlined Vector::operator VectorByValue&() at line 2657
		// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 2670
	}
	// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hObserverTarget>::Set() at line 2649
}

// game/server/player.cpp:2676 @0x626700 _ZN11CBasePlayer21IsValidObserverTargetEP11CBaseEntity
bool CBasePlayer::IsValidObserverTarget( CBaseEntity *target )
{
	CBasePlayer *player;  // line 2686
	// inlined ToBasePlayer() at line 2686
}

// game/server/player.cpp:2730 @0x6246b0 _ZN11CBasePlayer31GetNextObserverSearchStartPointEb
int CBasePlayer::GetNextObserverSearchStartPoint( bool bReverse )
{
	int iDir;  // line 2732
	int startIndex;  // line 2734
	// inlined CBaseEntity::entindex() at line 2744
	// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hObserverTarget>::operator CBaseEntity*() at line 2736
	// inlined CBaseEntity::entindex() at line 2739
}

// game/server/player.cpp:2756 @0x6206b0 _ZN11CBasePlayer20PassesObserverFilterEPK11CBaseEntity
bool CBasePlayer::PassesObserverFilter( const CBaseEntity *entity )
{
}

// game/server/player.cpp:2767 @0x621410 _ZN11CBasePlayer22FindNextObserverTargetEb
CBaseEntity *CBasePlayer::FindNextObserverTarget( bool bReverse )
{
	int startIndex;  // line 2780
	int currentIndex;  // line 2782
	int iDir;  // line 2783
	{
		CBaseEntity *nextTarget;  // line 2787
	}
}

// game/server/player.cpp:2810 @0x6206e0 _ZN11CBasePlayer15IsUseableEntityEP11CBaseEntityj
bool CBasePlayer::IsUseableEntity( CBaseEntity *pEntity, unsigned int requiredCaps )
{
	{
		int caps;  // line 2814
	}
}

// game/server/player.cpp:2835 @0x620720 _ZN11CBasePlayer4JumpEv
void CBasePlayer::Jump()
{
}

// game/server/player.cpp:2839 @0x620730 _ZN11CBasePlayer4DuckEv
void CBasePlayer::Duck()
{
}

// game/server/player.cpp:2853 @0x620770 _ZN11CBasePlayer8ClassifyEv
Class_T CBasePlayer::Classify()
{
}

// game/server/player.cpp:2859 (declaration)
void ResetFragCount();

// game/server/player.cpp:2859 @0x620780 _ZN11CBasePlayer14ResetFragCountEv
void CBasePlayer::ResetFragCount()
{
}

// game/server/player.cpp:2865 @0x6207a0 _ZN11CBasePlayer18IncrementFragCountEi
void CBasePlayer::IncrementFragCount( int nCount )
{
}

// game/server/player.cpp:2871 (declaration)
void ResetDeathCount();

// game/server/player.cpp:2871 @0x6207c0 _ZN11CBasePlayer15ResetDeathCountEv
void CBasePlayer::ResetDeathCount()
{
}

// game/server/player.cpp:2877 @0x6207e0 _ZN11CBasePlayer19IncrementDeathCountEi
void CBasePlayer::IncrementDeathCount( int nCount )
{
}

// game/server/player.cpp:2883 @0x620800 _ZN11CBasePlayer9AddPointsEib
void CBasePlayer::AddPoints( int score, bool bAllowNegativeScore )
{
}

// game/server/player.cpp:2904 @0x6227b0 _ZN11CBasePlayer15AddPointsToTeamEib
void CBasePlayer::AddPointsToTeam( int score, bool bAllowNegativeScore )
{
}

// game/server/player.cpp:2916 (declaration)
void GetCommandContextCount();

// game/server/player.cpp:2916 @0x620850 _ZNK11CBasePlayer22GetCommandContextCountEv
int CBasePlayer::GetCommandContextCount()
{
}

// game/server/player.cpp:2926 (declaration)
void GetCommandContext( int index );

// game/server/player.cpp:2926 @0x620860 _ZN11CBasePlayer17GetCommandContextEi
CCommandContext *CBasePlayer::GetCommandContext( int index )
{
	// inlined CUtlVector<CCommandContext,CUtlMemory<CCommandContext, int> >::operator[]() at line 2931
}

// game/server/player.cpp:2937 (declaration)
void AllocCommandContext();

// game/server/player.cpp:2937 @0x627d60 _ZN11CBasePlayer19AllocCommandContextEv
CCommandContext *CBasePlayer::AllocCommandContext()
{
	int idx;  // line 2939
	// inlined CUtlVector<CCommandContext,CUtlMemory<CCommandContext, int> >::AddToTail() at line 2939
}

// game/server/player.cpp:2951 @0x62c600 _ZN11CBasePlayer20RemoveCommandContextEi
void CBasePlayer::RemoveCommandContext( int index )
{
	// inlined CUtlVector<CCommandContext,CUtlMemory<CCommandContext, int> >::Remove() at line 2953
}

// game/server/player.cpp:2959 (declaration)
void RemoveAllCommandContexts();

// game/server/player.cpp:2959 @0x62c4a0 _ZN11CBasePlayer24RemoveAllCommandContextsEv
void CBasePlayer::RemoveAllCommandContexts()
{
	// inlined CUtlVector<CCommandContext,CUtlMemory<CCommandContext, int> >::RemoveAll() at line 2961
}

// game/server/player.cpp:2967 @0x62c220 _ZN11CBasePlayer36RemoveAllCommandContextsExceptNewestEv
CCommandContext *CBasePlayer::RemoveAllCommandContextsExceptNewest()
{
	int count;  // line 2969
	int toRemove;  // line 2970
	{
		CCommandContext *ctx;  // line 2979
		// inlined CBasePlayer::AllocCommandContext() at line 2979
	}
	// inlined CUtlVector<CCommandContext,CUtlMemory<CCommandContext, int> >::RemoveMultiple() at line 2973
	// inlined CUtlVector<CCommandContext,CUtlMemory<CCommandContext, int> >::Count() at line 2969
}

// game/server/player.cpp:2989 @0x627ba0 _ZN11CBasePlayer22ReplaceContextCommandsEP15CCommandContextP8CUserCmdi
void CBasePlayer::ReplaceContextCommands( CCommandContext *ctx, CUserCmd *pCommands, int nCommands )
{
	// inlined CUtlVector<CUserCmd,CUtlMemory<CUserCmd, int> >::RemoveAll() at line 2992
	{
		int i;  // line 2999
		// inlined CUtlVector<CUserCmd,CUtlMemory<CUserCmd, int> >::AddToTail() at line 3001
	}
}

// game/server/player.cpp:3009 (declaration)
void DetermineSimulationTicks();

// game/server/player.cpp:3009 @0x620890 _ZN11CBasePlayer24DetermineSimulationTicksEv
int CBasePlayer::DetermineSimulationTicks()
{
	int command_context_count;  // line 3011
	int context_number;  // line 3013
	int simulation_ticks;  // line 3015
	// inlined CBasePlayer::GetCommandContextCount() at line 3011
	{
		const CCommandContext *ctx;  // line 3020
		// inlined CBasePlayer::GetCommandContext() at line 3020
	}
}

// game/server/player.cpp:3033
static ConVar sv_clockcorrection_msecs;

// game/server/player.cpp:3034
static ConVar sv_playerperfhistorycount;

// game/server/player.cpp:3042 @0x632c50 _ZN11CBasePlayer20AdjustPlayerTimeBaseEi
void CBasePlayer::AdjustPlayerTimeBase( int simulation_ticks )
{
	CPlayerSimInfo *pi;  // line 3048
	{
		float flCorrectionSeconds;  // line 3070
		int nCorrectionTicks;  // line 3071
		int nIdealFinalTick;  // line 3078
		int nEstimatedFinalTick;  // line 3080
		int too_fast_limit;  // line 3083
		int too_slow_limit;  // line 3085
		{
			int nCorrectedTick;  // line 3091
			// inlined CNetworkVarBase<int,CBasePlayer::NetworkVar_m_nTickBase>::operator=<int>() at line 3098
		}
	}
	// inlined CUtlLinkedList<CPlayerSimInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerSimInfo, short unsigned int>, short unsigned int> >::AddToTail() at line 3056
	// inlined CUtlLinkedList<CPlayerSimInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerSimInfo, short unsigned int>, short unsigned int> >::Head() at line 3053
	// inlined CUtlLinkedList<CPlayerSimInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerSimInfo, short unsigned int>, short unsigned int> >::Remove() at line 3053
	// inlined ConVar::GetInt() at line 3049
	// inlined CNetworkVarBase<int,CBasePlayer::NetworkVar_m_nTickBase>::operator=<int>() at line 3066
}

// game/server/player.cpp:3108 @0x62cba0 _ZN11CBasePlayer14RunNullCommandEv
void CBasePlayer::RunNullCommand()
{
	CUserCmd cmd;  // line 3110
	float flOldFrametime;  // line 3113
	float flOldCurtime;  // line 3114
	float flTimeBase;  // line 3119
	// inlined CBasePlayer::SetLastUserCommand() at line 3126
	// inlined CBasePlayer::SetTimeBase() at line 3120
	// inlined QAngle::operator=() at line 3117
	// inlined CUserCmd::CUserCmd() at line 3110
}

// game/server/player.cpp:3138 @0x634f90 _ZN11CBasePlayer15PhysicsSimulateEv
void CBasePlayer::PhysicsSimulate()
{
	CVProfScope VProf_;  // line 3140
	CBaseEntity *pMoveParent;  // line 3143
	int simulation_ticks;  // line 3158
	float savetime;  // line 3181
	float saveframetime;  // line 3182
	int command_context_count;  // line 3184
	CUtlVector<CUserCmd,CUtlMemory<CUserCmd, int> > vecAvailCommands;  // line 3188
	int commandLimit;  // line 3240
	int commandsToRun;  // line 3241
	float vphysicsArrivalTime;  // line 3265
	// inlined CUtlVector<CUserCmd,CUtlMemory<CUserCmd, int> >::~CUtlVector() at line 3313
	{
		int commandsToRollOver;  // line 3244
		{
			CCommandContext *ctx;  // line 3250
		}
		// inlined CBasePlayer::RemoveAllCommandContexts() at line 3256
	}
	// inlined CVProfScope::CVProfScope() at line 3140
	// inlined CBaseEntity::GetMoveParent() at line 3143
	// inlined CBasePlayer::DetermineSimulationTicks() at line 3158
	// inlined CVProfScope::~CVProfScope() at line 3313
	// inlined CBasePlayer::GetCommandContextCount() at line 3184
	// inlined CUtlVector<CUserCmd,CUtlMemory<CUserCmd, int> >::CUtlVector() at line 3188
	{
		int context_number;  // line 3191
		{
			CCommandContext *ctx;  // line 3194
			int numbackup;  // line 3201
			// inlined CUserCmd::operator=() at line 3233
			{
				int i;  // line 3226
				// inlined CUtlVector<CUserCmd,CUtlMemory<CUserCmd, int> >::AddToTail() at line 3228
			}
			// inlined CBasePlayer::GetCommandContext() at line 3194
			{
				int droppedcmds;  // line 3206
				{
					int cmdnum;  // line 3219
					// inlined CUtlVector<CUserCmd,CUtlMemory<CUserCmd, int> >::AddToTail() at line 3220
				}
				// inlined CUtlVector<CUserCmd,CUtlMemory<CUserCmd, int> >::AddToTail() at line 3212
			}
		}
	}
	// inlined CUtlVector<CUserCmd,CUtlMemory<CUserCmd, int> >::Count() at line 3241
	// inlined CBasePlayer::RemoveAllCommandContexts() at line 3262
	{
		CPlayerSimInfo *pi;  // line 3298
		{
			int i;  // line 3278
			{
				CVProfScope VProf_;  // line 3285
				// inlined CVProfScope::~CVProfScope() at line 3288
				// inlined CVProfScope::CVProfScope() at line 3285
				// inlined CVProfScope::~CVProfScope() at line 3288
			}
		}
		// inlined IPredictionSystem::SuppressHostEvents() at line 3275
		// inlined IPredictionSystem::SuppressHostEvents() at line 3293
		// inlined CUtlLinkedList<CPlayerSimInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerSimInfo, short unsigned int>, short unsigned int> >::operator[]() at line 3301
		// inlined CBaseEntity::GetAbsOrigin() at line 3303
		// inlined Vector::operator=() at line 3303
	}
	// inlined CBasePlayer::RemoveAllCommandContexts() at line 3176
	// inlined CVProfScope::~CVProfScope() at line 3313
	// inlined CUtlVector<CUserCmd,CUtlMemory<CUserCmd, int> >::~CUtlVector() at line 3313
	// inlined CVProfScope::~CVProfScope() at line 3313
}

// game/server/player.cpp:3316 @0x6208f0 _ZNK11CBasePlayer25PhysicsSolidMaskForEntityEv
unsigned int CBasePlayer::PhysicsSolidMaskForEntity()
{
}

// game/server/player.cpp:3324 (declaration)
void ForceSimulation();

// game/server/player.cpp:3324 @0x620900 _ZN11CBasePlayer15ForceSimulationEv
void CBasePlayer::ForceSimulation()
{
}

// game/server/player.cpp:3339 @0x627e30 _ZN11CBasePlayer15ProcessUsercmdsEP8CUserCmdiiib
void CBasePlayer::ProcessUsercmds( CUserCmd *cmds, int numcmds, int totalcmds, int dropped_packets, bool paused )
{
	CCommandContext *ctx;  // line 3341
	int i;  // line 3344
	{
		CPlayerCmdInfo pi;  // line 3394
		// inlined CUtlLinkedList<CPlayerCmdInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerCmdInfo, short unsigned int>, short unsigned int> >::AddToTail() at line 3404
		// inlined CUtlLinkedList<CPlayerCmdInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerCmdInfo, short unsigned int>, short unsigned int> >::Head() at line 3401
		// inlined CUtlLinkedList<CPlayerCmdInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerCmdInfo, short unsigned int>, short unsigned int> >::Remove() at line 3401
	}
	// inlined ConVar::GetInt() at line 3392
	// inlined CBasePlayer::ForceSimulation() at line 3387
	{
		bool clear_angles;  // line 3357
		// inlined CUtlVector<CUserCmd,CUtlMemory<CUserCmd, int> >::operator[]() at line 3375
		// inlined VectorCopy() at line 3375
	}
	// inlined CUtlVector<CUserCmd,CUtlMemory<CUserCmd, int> >::AddToTail() at line 3347
	// inlined CBasePlayer::AllocCommandContext() at line 3341
}

// game/server/player.cpp:3408 @0x625850 _ZN11CBasePlayer19DumpPerfToRecipientEPS_i
void CBasePlayer::DumpPerfToRecipient( CBasePlayer *pRecipient, int nMaxRecords )
{
	char buf[256];  // line 3413
	int curpos;  // line 3414
	int nDumped;  // line 3416
	Vector prevo;  // line 3417
	float prevt;  // line 3418
	{
		int i;  // line 3470
		{
			const CPlayerCmdInfo *pi;  // line 3472
			char line[128];  // line 3474
			int len;  // line 3475
			// inlined CUtlLinkedList<CPlayerCmdInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerCmdInfo, short unsigned int>, short unsigned int> >::operator[]() at line 3472
		}
		// inlined CUtlLinkedList<CPlayerCmdInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerCmdInfo, short unsigned int>, short unsigned int> >::Tail() at line 3470
		// inlined CUtlLinkedList<CPlayerCmdInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerCmdInfo, short unsigned int>, short unsigned int> >::Previous() at line 3470
	}
	{
		int i;  // line 3420
		// inlined CUtlLinkedList<CPlayerSimInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerSimInfo, short unsigned int>, short unsigned int> >::Previous() at line 3420
		// inlined CUtlLinkedList<CPlayerSimInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerSimInfo, short unsigned int>, short unsigned int> >::Tail() at line 3420
		{
			const CPlayerSimInfo *pi;  // line 3422
			float vel;  // line 3424
			float dt;  // line 3427
			char line[128];  // line 3434
			int len;  // line 3435
			// inlined Vector::operator=() at line 3458
			// inlined CUtlLinkedList<CPlayerSimInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerSimInfo, short unsigned int>, short unsigned int> >::operator[]() at line 3422
			{
				Vector d;  // line 3430
				// inlined Vector::operator-() at line 3430
				// inlined Vector::Length() at line 3431
			}
		}
	}
}

// game/server/player.cpp:3502
ConVar xc_crouch_debounce;

// game/server/player.cpp:3507 @0x624930 _ZNK11CBasePlayer17HasQueuedUsercmdsEv
bool CBasePlayer::HasQueuedUsercmds()
{
}

// game/server/player.cpp:3517 @0x624b30 _ZN11CBasePlayer16PlayerRunCommandEP8CUserCmdP11IMoveHelper
void CBasePlayer::PlayerRunCommand( CUserCmd *ucmd, IMoveHelper *moveHelper )
{
	// inlined VectorCopy() at line 3523
	// inlined VectorCopy() at line 3536
	// inlined CBasePlayer::ToggleDuck() at line 3546
}

// game/server/player.cpp:3564 (declaration)
void DisableButtons( int nButtons );

// game/server/player.cpp:3564 @0x620920 _ZN11CBasePlayer14DisableButtonsEi
void CBasePlayer::DisableButtons( int nButtons )
{
}

// game/server/player.cpp:3572 (declaration)
void EnableButtons( int nButtons );

// game/server/player.cpp:3572 @0x620940 _ZN11CBasePlayer13EnableButtonsEi
void CBasePlayer::EnableButtons( int nButtons )
{
}

// game/server/player.cpp:3577 @0x634360 _ZN11CBasePlayer15HandleFuncTrainEv
void CBasePlayer::HandleFuncTrain()
{
	CBaseEntity *pTrain;  // line 3594
	float vel;  // line 3595
	// inlined CNetworkVarBase<unsigned int,CBasePlayer::NetworkVar_m_afPhysicsFlags>::operator&=<int>() at line 3645
	{
		trace_t trainTrace;  // line 3625
		// inlined UTIL_TraceLine() at line 3628
		// inlined CBaseEntity::GetAbsOrigin() at line 3628
		// inlined Vector::operator+() at line 3628
		// inlined CBaseEntity::GetAbsOrigin() at line 3628
		// inlined Vector::Vector() at line 3628
	}
	// inlined CFuncTrackTrain::GetMaxSpeed() at line 3665
	// inlined TrainSpeed() at line 3665
}

// game/server/player.cpp:3671 @0x634810 _ZN11CBasePlayer8PreThinkEv
void CBasePlayer::PreThink()
{
	CNavArea *area;  // line 3728
	// inlined CNetworkVarBase<int,CPlayerLocalData::NetworkVar_m_iHideHUD>::operator|=<int>() at line 3689
	// inlined CNetworkVarBase<int,CPlayerLocalData::NetworkVar_m_iHideHUD>::operator&=<int>() at line 3687
	// inlined CBaseEntity::GetFlags() at line 3716
	// inlined CBaseEntity::GetAbsVelocity() at line 3724
	// inlined CNetworkVarBase<float,CPlayerLocalData::NetworkVar_m_flFallVelocity>::operator=<float>() at line 3724
	// inlined CBaseEntity::GetAbsOrigin() at line 3728
	// inlined CNavArea::DecrementPlayerCount() at line 3734
	// inlined CNavArea::IncrementPlayerCount() at line 3737
	// inlined CNavArea::GetPlace() at line 3740
	{
		const char *placeName;  // line 3742
		// inlined CBasePlayer::NetworkVar_m_szLastPlaceName::GetForModify() at line 3745
	}
}

// game/server/player.cpp:3838 @0x620960 _ZN11CBasePlayer20CheckTimeBasedDamageEv
void CBasePlayer::CheckTimeBasedDamage()
{
	int i;  // line 3840
	uint8 bDuration;  // line 3841
	{
		int iDamage;  // line 3859
		{
			int idif;  // line 3890
		}
		{
			int nDif;  // line 3904
		}
	}
	// inlined abs<float>() at line 3850
}

// game/server/player.cpp:4016 @0x62ea90 _ZN11CBasePlayer19UpdateGeigerCounterEv
void CBasePlayer::UpdateGeigerCounter()
{
	uint8 range;  // line 4018
	{
		CSingleUserRecipientFilter user;  // line 4039
		// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 4043
		// inlined CSingleUserRecipientFilter::CSingleUserRecipientFilter() at line 4039
		// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 4043
	}
}

// game/server/player.cpp:4064 @0x622df0 _ZN11CBasePlayer15CheckSuitUpdateEv
void CBasePlayer::CheckSuitUpdate()
{
	int i;  // line 4066
	int isentence;  // line 4067
	int isearch;  // line 4068
	{
		char sentence[512];  // line 4102
	}
}

// game/server/player.cpp:4125 @0x622c10 _ZN11CBasePlayer13SetSuitUpdateEPcii
void CBasePlayer::SetSuitUpdate( char *name, int fgroup, int iNoRepeatTime )
{
	int i;  // line 4127
	int isentence;  // line 4128
	int iempty;  // line 4129
}

// game/server/player.cpp:4222 @0x62a070 _ZN11CBasePlayer17UpdatePlayerSoundEv
void CBasePlayer::UpdatePlayerSound()
{
	int iBodyVolume;  // line 4224
	int iVolume;  // line 4225
	CSound *pSound;  // line 4226
	// inlined CBaseEntity::GetFlags() at line 4236
	// inlined CBaseEntity::GetAbsVelocity() at line 4245
	// inlined Vector::Length() at line 4245
	// inlined CSound::Volume() at line 4273
	// inlined CBaseEntity::GetAbsOrigin() at line 4291
	// inlined CSound::SetSoundOrigin() at line 4291
}

// game/server/player.cpp:4306 @0x6299b0 _Z20FixPlayerCrouchStuckP11CBasePlayer
FixPlayerCrouchStuck( CBasePlayer *pPlayer )
{
	trace_t trace;  // line 4308
	int i;  // line 4311
	Vector org;  // line 4312
	// inlined CBaseEntity::GetAbsOrigin() at line 4316
	// inlined CBaseEntity::GetAbsOrigin() at line 4316
	{
		Vector origin;  // line 4319
		// inlined CBaseEntity::GetAbsOrigin() at line 4319
	}
	// inlined UTIL_TraceHull() at line 4316
	// inlined CBaseEntity::GetAbsOrigin() at line 4312
	// inlined UTIL_TraceHull() at line 4332
	{
		Vector origin;  // line 4335
		// inlined CBaseEntity::GetAbsOrigin() at line 4335
	}
	// inlined CBaseEntity::GetAbsOrigin() at line 4332
	// inlined CBaseEntity::GetAbsOrigin() at line 4332
}

// game/server/player.cpp:4352 @0x623a40 _ZN11CBasePlayer11ForceOriginERK6Vector
void CBasePlayer::ForceOrigin( const Vector &vecOrigin )
{
	// inlined Vector::operator=() at line 4355
}

// game/server/player.cpp:4360 @0x6285a0 _ZN11CBasePlayer26OnTonemapTriggerStartTouchEP15CTonemapTrigger
void CBasePlayer::OnTonemapTriggerStartTouch( CTonemapTrigger *pTonemapTrigger )
{
	// inlined CHandle<CTonemapTrigger>::CHandle() at line 4362
	// inlined CUtlVector<CHandle<CTonemapTrigger>,CUtlMemory<CHandle<CTonemapTrigger>, int> >::FindAndRemove() at line 4362
	// inlined CHandle<CTonemapTrigger>::CHandle() at line 4363
	// inlined CUtlVector<CHandle<CTonemapTrigger>,CUtlMemory<CHandle<CTonemapTrigger>, int> >::AddToTail() at line 4363
}

// game/server/player.cpp:4368 @0x628460 _ZN11CBasePlayer24OnTonemapTriggerEndTouchEP15CTonemapTrigger
void CBasePlayer::OnTonemapTriggerEndTouch( CTonemapTrigger *pTonemapTrigger )
{
	// inlined CHandle<CTonemapTrigger>::CHandle() at line 4370
	// inlined CUtlVector<CHandle<CTonemapTrigger>,CUtlMemory<CHandle<CTonemapTrigger>, int> >::FindAndRemove() at line 4370
}

// game/server/player.cpp:4375 @0x62cfe0 _ZN11CBasePlayer23UpdateTonemapControllerEv
void CBasePlayer::UpdateTonemapController()
{
	// inlined CTonemapSystem::GetMasterTonemapController() at line 4377
	// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hTonemapController>::operator=() at line 4377
}

// game/server/player.cpp:4383 @0x6388a0 _ZN11CBasePlayer9PostThinkEv
void CBasePlayer::PostThink()
{
	CVProfScope VProf_;  // line 4385
	// inlined CVProfScope::CVProfScope() at line 4385
	// inlined CBaseEntity::GetAbsVelocity() at line 4386
	// inlined Vector::operator*() at line 4386
	// inlined Vector::operator*() at line 4386
	// inlined Vector::operator+() at line 4386
	// inlined Vector::operator=() at line 4386
	// inlined CVProfScope::~CVProfScope() at line 4523
	// inlined CBasePlayer::ClearImpulse() at line 4463
	{
		CVProfScope VProf_;  // line 4472
		// inlined CVProfScope::~CVProfScope() at line 4473
		// inlined CVProfScope::CVProfScope() at line 4472
	}
	{
		CVProfScope VProf_;  // line 4476
		// inlined CVProfScope::~CVProfScope() at line 4477
		// inlined CVProfScope::CVProfScope() at line 4476
	}
	// inlined CBaseEntity::SetSimulationTime() at line 4480
	{
		CVProfScope VProf_;  // line 4483
		// inlined CVProfScope::~CVProfScope() at line 4484
		// inlined CVProfScope::CVProfScope() at line 4483
	}
	{
		CVProfScope VProf_;  // line 4487
		// inlined CVProfScope::~CVProfScope() at line 4488
		// inlined CVProfScope::CVProfScope() at line 4487
	}
	{
		CVProfScope VProf_;  // line 4499
		// inlined CVProfScope::~CVProfScope() at line 4500
		// inlined CVProfScope::CVProfScope() at line 4499
		// inlined CVProfScope::~CVProfScope() at line 4500
	}
	// inlined CBaseEntity::GetHealth() at line 4510
	// inlined GlobalEntity_GetState() at line 4510
	{
		color32 hurtScreenOverlay;  // line 4513
	}
	{
		CVProfScope VProf_;  // line 4450
		// inlined CBaseEntity::GetAbsOrigin() at line 4443
		// inlined CBaseEntity::GetAbsVelocity() at line 4456
		{
			CVProfScope VProf_;  // line 4396
			// inlined CVProfScope::CVProfScope() at line 4396
			// inlined CVProfScope::~CVProfScope() at line 4398
		}
		{
			CVProfScope VProf_;  // line 4402
			{
				CPlayerPickupController *pPickup;  // line 4416
				// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hUseEntity>::Get() at line 4416
			}
			// inlined CVProfScope::CVProfScope() at line 4402
			// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hUseEntity>::operator!=() at line 4404
			// inlined CVProfScope::~CVProfScope() at line 4431
			// inlined FClassnameIs() at line 4407
		}
		{
			CVProfScope VProf_;  // line 4435
			// inlined CVProfScope::CVProfScope() at line 4435
			// inlined CVProfScope::~CVProfScope() at line 4436
		}
		// inlined CNetworkVarBase<float,CPlayerLocalData::NetworkVar_m_flFallVelocity>::operator=<int>() at line 4446
		// inlined CVProfScope::CVProfScope() at line 4450
		// inlined CBaseEntity::GetAbsVelocity() at line 4454
		// inlined CBaseEntity::GetAbsVelocity() at line 4454
		// inlined CBaseEntity::GetAbsVelocity() at line 4456
		// inlined CVProfScope::~CVProfScope() at line 4459
		// inlined CVProfScope::~CVProfScope() at line 4459
	}
	// inlined RandomAngle() at line 4495
	// inlined QAngle::operator QAngleByValue&() at line 4495
	// inlined CNetworkVectorBase<QAngle,CPlayerLocalData::NetworkVar_m_vecPunchAngle>::operator=() at line 4495
	// inlined CNetworkVectorBase<QAngle,CPlayerLocalData::NetworkVar_m_vecPunchAngleVel>::Init() at line 4496
	// inlined CVProfScope::~CVProfScope() at line 4523
}

// game/server/player.cpp:4527 @0x6242f0 _ZN11CBasePlayer5TouchEP11CBaseEntity
void CBasePlayer::Touch( CBaseEntity *pOther )
{
	IPhysicsObject *pPhys;  // line 4535
	// inlined CBaseEntity::GetSolid() at line 4532
	// inlined CBaseEntity::GetSolidFlags() at line 4532
	// inlined CBaseEntity::VPhysicsGetObject() at line 4535
	// inlined CBasePlayer::SetTouchedPhysics() at line 4539
}

// game/server/player.cpp:4542 @0x623880 _ZN11CBasePlayer19GetSmoothedVelocityEv
Vector CBasePlayer::GetSmoothedVelocity()
{
}

// game/server/player.cpp:4552
CBaseEntity *g_pLastSpawn;

// game/server/player.cpp:4563 (declaration)
CBaseEntity *FindPlayerStart( const char *pszClassName );

// game/server/player.cpp:4563 @0x6214b0 _Z15FindPlayerStartPKc
CBaseEntity *FindPlayerStart( const char *pszClassName )
{
	CBaseEntity *pStart;  // line 4567
	CBaseEntity *pStartFirst;  // line 4568
}

// game/server/player.cpp:4591 @0x6294e0 _ZN11CBasePlayer19EntSelectSpawnPointEv
CBaseEntity *CBasePlayer::EntSelectSpawnPoint()
{
	CBaseEntity *pSpot;  // line 4593
	edict_t *player;  // line 4594
	// inlined CBaseEntity::edict() at line 4596
	// inlined string_t::operator!() at line 4655
	// inlined FindPlayerStart() at line 4657
	// inlined INDEXENT() at line 4672
	// inlined CBaseEntity::Instance() at line 4672
	// inlined CGlobalEntityList::FindEntityByName() at line 4663
	{
		CBaseEntity *pFirstSpot;  // line 4617
		{
			CBaseEntity *ent;  // line 4643
			{
				CEntitySphereQuery sphere;  // line 4644
				// inlined INDEXENT() at line 4648
				// inlined CEntitySphereQuery::NextEntity() at line 4644
				// inlined GetContainingEntity() at line 4648
				// inlined INDEXENT() at line 4648
				// inlined GetContainingEntity() at line 4648
				// inlined CBaseEntity::GetAbsOrigin() at line 4644
			}
		}
		// inlined Vector::operator==() at line 4626
		{
			int i;  // line 4612
		}
	}
}

// game/server/player.cpp:4682 @0x620b80 _ZN11CBasePlayer12InitialSpawnEv
void CBasePlayer::InitialSpawn()
{
}

// game/server/player.cpp:4693 @0x636990 _ZN11CBasePlayer5SpawnEv
void CBasePlayer::Spawn()
{
	CVProfScope VProf_;  // line 4695
	int effects;  // line 4733
	CSingleUserRecipientFilter user;  // line 4804
	color32 nothing;  // line 4830
	IGameEvent *event;  // line 4841
	// inlined CVProfScope::CVProfScope() at line 4695
	// inlined CBaseEntity::SetSimulatedEveryTick() at line 4708
	// inlined CBaseEntity::SetAnimatedEveryTick() at line 4709
	// inlined CNetworkVarBase<int,CBasePlayer::NetworkVar_m_ArmorValue>::operator=<int>() at line 4711
	// inlined CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iMaxHealth>::operator=<CNetworkVarBase<int, CBaseEntity::NetworkVar_m_iHealth> >() at line 4713
	// inlined CNetworkVarBase<bool,CBaseEntity::NetworkVar_m_bClientSideRagdoll>::operator=<bool>() at line 4736
	// inlined CBasePlayer::InitFogController() at line 4739
	// inlined CBasePlayer::InitPostProcessController() at line 4740
	// inlined CNetworkVarBase<unsigned int,CBasePlayer::NetworkVar_m_afPhysicsFlags>::operator=<int>() at line 4746
	// inlined Vector::operator=() at line 4759
	// inlined Vector::operator=() at line 4760
	// inlined CNetworkVarBase<bool,CPlayerLocalData::NetworkVar_m_bDucked>::operator=<bool>() at line 4767
	// inlined CNetworkVarBase<bool,CPlayerLocalData::NetworkVar_m_bDucking>::operator=<bool>() at line 4768
	{
		CVProfScope VProf_;  // line 4770
		// inlined CVProfScope::~CVProfScope() at line 4774
		// inlined CVProfScope::CVProfScope() at line 4770
		// inlined CVProfScope::~CVProfScope() at line 4774
	}
	// inlined Vector::Vector() at line 4787
	// inlined Vector::operator=() at line 4787
	// inlined ConVar::GetInt() at line 4789
	// inlined CNetworkVarBase<int,CBasePlayer::NetworkVar_m_iBonusChallenge>::operator=<int>() at line 4789
	// inlined CBasePlayer::NetworkVar_m_szLastPlaceName::GetForModify() at line 4802
	// inlined CSingleUserRecipientFilter::CSingleUserRecipientFilter() at line 4804
	// inlined CBasePlayer::LockPlayerInPlace() at line 4815
	{
		CVProfScope VProf_;  // line 4833
		// inlined CVProfScope::~CVProfScope() at line 4834
		// inlined CVProfScope::~CVProfScope() at line 4834
		// inlined CVProfScope::CVProfScope() at line 4833
	}
	// inlined CNetworkVarBase<float,CBasePlayer::NetworkVar_m_flLaggedMovementValue>::operator=<float>() at line 4837
	// inlined Vector::operator=() at line 4838
	// inlined CBaseEntity::GetAbsVelocity() at line 4839
	// inlined CBaseEntity::GetAbsOrigin() at line 4839
	// inlined CBasePlayer::GetUserID() at line 4845
	// inlined Vector::Vector() at line 4854
	// inlined Vector::operator=() at line 4854
	// inlined CBaseCombatCharacter::Spawn() at line 4859
	// inlined ScriptVariant_t::ScriptVariant_t() at line 4866
	// inlined IScriptVM::SetValue() at line 4866
	// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 4867
	// inlined CVProfScope::~CVProfScope() at line 4867
	// inlined CVProfScope::~CVProfScope() at line 4867
	// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 4867
}

// game/server/player.cpp:4870 @0x622bc0 _ZN11CBasePlayer8ActivateEv
void CBasePlayer::Activate()
{
}

// game/server/player.cpp:4883 @0x62bc10 _ZN11CBasePlayer8PrecacheEv
void CBasePlayer::Precache()
{
	CVProfScope VProf_;  // line 4885
	// inlined CVProfScope::CVProfScope() at line 4885
	// inlined CVProfScope::~CVProfScope() at line 4950
	// inlined CVProfScope::~CVProfScope() at line 4950
}

// game/server/player.cpp:4957 @0x622b30 _ZN11CBasePlayer12ForceRespawnEv
void CBasePlayer::ForceRespawn()
{
}

// game/server/player.cpp:4970 @0x6213e0 _ZN11CBasePlayer4SaveER5ISave
int CBasePlayer::Save( ISave &save )
{
}

// game/server/player.cpp:4981 sizeof=0x1 (i386)
struct CPlayerRestoreHelper
{
public:
	const Vector &GetAbsOrigin( CBaseEntity * );  // line 4984
	const Vector &GetAbsVelocity( CBaseEntity * );  // line 4989
};

// game/server/player.cpp:4996 @0x636540 _ZN11CBasePlayer7RestoreER8IRestore
int CBasePlayer::Restore( IRestore &restore )
{
	int status;  // line 4998
	CSaveRestoreData *pSaveData;  // line 5002
	QAngle newViewAngles;  // line 5014
	CPlayerRestoreHelper helper;  // line 5041
	// inlined CNetworkVarBase<bool,CPlayerLocalData::NetworkVar_m_bDucked>::operator=<bool>() at line 5034
	// inlined CNetworkVarBase<unsigned int,CBasePlayer::NetworkVar_m_afPhysicsFlags>::operator&=<int>() at line 5023
	// inlined CBasePlayer::SnapEyeAngles() at line 5017
	{
		CBaseEntity *pSpawnSpot;  // line 5009
		// inlined Vector::Vector() at line 5010
		// inlined Vector::operator+() at line 5010
		// inlined Vector::operator VectorByValue&() at line 5010
	}
	// inlined CNetworkVarBase<bool,CPlayerLocalData::NetworkVar_m_bDucked>::operator=<bool>() at line 5030
}

// game/server/player.cpp:5051 @0x637fc0 _ZN11CBasePlayer9OnRestoreEv
void CBasePlayer::OnRestore()
{
	// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hViewEntity>::operator CBaseEntity*() at line 5055
	// inlined CBasePlayer::SetDefaultFOV() at line 5056
	// inlined CBaseAnimating::LookupPoseParameter() at line 5061
	// inlined ScriptVariant_t::ScriptVariant_t() at line 5068
	// inlined IScriptVM::SetValue() at line 5068
}

// game/server/player.cpp:5077 @0x627760 _ZN11CBasePlayer13SetArmorValueEi
void CBasePlayer::SetArmorValue( int value )
{
	// inlined CNetworkVarBase<int,CBasePlayer::NetworkVar_m_ArmorValue>::operator=<int>() at line 5079
}

// game/server/player.cpp:5082 @0x6275c0 _ZN11CBasePlayer19IncrementArmorValueEii
void CBasePlayer::IncrementArmorValue( int nCount, int nMaxValue )
{
	// inlined CNetworkVarBase<int,CBasePlayer::NetworkVar_m_ArmorValue>::operator+=<int>() at line 5084
	// inlined CNetworkVarBase<int,CBasePlayer::NetworkVar_m_ArmorValue>::operator=<int>() at line 5088
}

// game/server/player.cpp:5092 @0x620bc0 _ZN11CBasePlayer27NotifyNearbyRadiationSourceEf
void CBasePlayer::NotifyNearbyRadiationSource( float flRange )
{
}

// game/server/player.cpp:5102 @0x620bf0 _ZN11CBasePlayer27AllowImmediateDecalPaintingEv
void CBasePlayer::AllowImmediateDecalPainting()
{
}

// game/server/player.cpp:5108 @0x626c60 _ZN11CBasePlayer13CommitSuicideEbb
void CBasePlayer::CommitSuicide( bool bExplode, bool bForce )
{
	CMDLCacheCriticalSection cacheCriticalSection;  // line 5110
	int fDamage;  // line 5122
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 5128
	// inlined CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth>::operator=<int>() at line 5125
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 5128
	// inlined CMDLCacheCriticalSection::CMDLCacheCriticalSection() at line 5110
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 5128
}

// game/server/player.cpp:5132 @0x627960 _ZN11CBasePlayer13CommitSuicideERK6Vectorbb
void CBasePlayer::CommitSuicide( const Vector &vecForce, bool bExplode, bool bForce )
{
	CMDLCacheCriticalSection cacheCriticalSection;  // line 5134
	int nHealth;  // line 5147
	CTakeDamageInfo info;  // line 5150
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 5156
	// inlined CTakeDamageInfo::SetDamagePosition() at line 5155
	// inlined CTakeDamageInfo::SetDamageForce() at line 5154
	// inlined CTakeDamageInfo::SetDamageType() at line 5153
	// inlined CTakeDamageInfo::SetAttacker() at line 5152
	// inlined CTakeDamageInfo::SetDamage() at line 5151
	// inlined CBaseEntity::GetHealth() at line 5147
	// inlined CMDLCacheCriticalSection::CMDLCacheCriticalSection() at line 5134
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 5156
}

// game/server/player.cpp:5162 (declaration)
void HasWeapons();

// game/server/player.cpp:5162 @0x621c70 _ZN11CBasePlayer10HasWeaponsEv
bool CBasePlayer::HasWeapons()
{
	int i;  // line 5164
}

// game/server/player.cpp:5181 @0x622b80 _ZN11CBasePlayer13VelocityPunchERK6Vector
void CBasePlayer::VelocityPunch( const Vector &vecForce )
{
}

// game/server/player.cpp:5197 @0x621820 _ZN11CBasePlayer15CanEnterVehicleEP14IServerVehiclei
bool CBasePlayer::CanEnterVehicle( IServerVehicle *pVehicle, int nRole )
{
	{
		CBaseCombatWeapon *pWeapon;  // line 5207
	}
}

// game/server/player.cpp:5226 @0x6379c0 _ZN11CBasePlayer12GetInVehicleEP14IServerVehiclei
bool CBasePlayer::GetInVehicle( IServerVehicle *pVehicle, int nRole )
{
	CBaseEntity *pEnt;  // line 5235
	Vector vSeatOrigin;  // line 5273
	QAngle qSeatAngles;  // line 5274
	// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hVehicle>::operator=() at line 5302
	// inlined CBasePlayer::ToggleDuck() at line 5299
	// inlined CNetworkVarBase<int,CPlayerLocalData::NetworkVar_m_nJumpTimeMsecs>::operator=<int>() at line 5294
	// inlined CNetworkVarBase<int,CPlayerLocalData::NetworkVar_m_nDuckJumpTimeMsecs>::operator=<int>() at line 5293
	// inlined CNetworkVarBase<int,CPlayerLocalData::NetworkVar_m_nDuckTimeMsecs>::operator=<int>() at line 5292
	// inlined CNetworkVarBase<bool,CPlayerLocalData::NetworkVar_m_bDucking>::operator=<bool>() at line 5291
	// inlined CNetworkVarBase<bool,CPlayerLocalData::NetworkVar_m_bDucked>::operator=<bool>() at line 5290
	{
		CBaseCombatWeapon *pWeapon;  // line 5241
		// inlined CNetworkVarBase<int,CPlayerLocalData::NetworkVar_m_iHideHUD>::operator|=<int>() at line 5248
		// inlined CNetworkVarBase<int,CPlayerLocalData::NetworkVar_m_iHideHUD>::operator|=<int>() at line 5250
	}
}

// game/server/player.cpp:5318 @0x62dd50 _ZN11CBasePlayer12LeaveVehicleERK6VectorRK6QAngle
void CBasePlayer::LeaveVehicle( const Vector &vecExitPoint, const QAngle &vecExitAngles )
{
	IServerVehicle *pVehicle;  // line 5323
	int nRole;  // line 5326
	Vector vNewPos;  // line 5332
	QAngle qAngles;  // line 5333
	// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hVehicle>::operator=() at line 5369
	// inlined CBaseEntity::VPhysicsGetObject() at line 5364
	// inlined CBaseEntity::RemoveEffects() at line 5359
	// inlined CNetworkVarBase<int,CPlayerLocalData::NetworkVar_m_iHideHUD>::operator&=<int>() at line 5357
	// inlined CNetworkVarBase<int,CPlayerLocalData::NetworkVar_m_iHideHUD>::operator&=<int>() at line 5354
	// inlined CBasePlayer::SnapEyeAngles() at line 5351
	// inlined QAngle::operator=() at line 5342
	// inlined Vector::operator=() at line 5341
	// inlined Vector::operator==() at line 5334
	// inlined CBaseEntity::GetAbsAngles() at line 5333
	// inlined CBaseEntity::GetAbsOrigin() at line 5332
	// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hVehicle>::Get() at line 5320
	// inlined CBasePlayer::ShowCrosshair() at line 5378
}

// game/server/player.cpp:5393 sizeof=0x3c0 (i386)
struct CSprayCan : public CPointEntity
{
public:
	void Spawn( CBasePlayer * );  // line 5397
	virtual void Think();  // line 5398
	virtual void Precache();  // line 5400
	virtual int ObjectCaps();  // line 5402
};

// game/server/player.cpp:5393 (declaration)
void CSprayCan();

// game/server/player.cpp:5393 (declaration)
~CSprayCan();

// game/server/player.cpp:5393 @0x63c920 _ZN9CSprayCanD0Ev
CSprayCan::~CSprayCan()
{
	// inlined CPointEntity::~CPointEntity() at line 5393
}

// game/server/player.cpp:5393 @0x63c970 _ZN9CSprayCanD1Ev
CSprayCan::~CSprayCan()
{
	// inlined CPointEntity::~CPointEntity() at line 5393
}

// game/server/player.cpp:5397 @0x622a50 _ZN9CSprayCan5SpawnEP11CBasePlayer
void CSprayCan::Spawn( CBasePlayer *pOwner )
{
	// inlined Vector::Vector() at line 5410
	// inlined Vector::operator+() at line 5410
	// inlined Vector::operator VectorByValue&() at line 5410
}

// game/server/player.cpp:5398 @0x6291e0 _ZN9CSprayCan5ThinkEv
void CSprayCan::Think()
{
	CBasePlayer *pPlayer;  // line 5425
	// inlined CBaseEntity::GetOwnerEntity() at line 5425
	// inlined ToBasePlayer() at line 5425
	{
		int playernum;  // line 5428
		Vector forward;  // line 5430
		trace_t tr;  // line 5431
		// inlined CBaseEntity::entindex() at line 5428
		// inlined CBaseEntity::GetAbsAngles() at line 5433
		// inlined Vector::operator*() at line 5435
		// inlined CBaseEntity::GetAbsOrigin() at line 5435
		// inlined Vector::operator+() at line 5435
		// inlined CBaseEntity::GetAbsOrigin() at line 5435
		// inlined UTIL_TraceLine() at line 5435
	}
}

// game/server/player.cpp:5400 @0x624ff0 _ZN9CSprayCan8PrecacheEv
void CSprayCan::Precache()
{
}

// game/server/player.cpp:5402 @0x63b340 _ZN9CSprayCan10ObjectCapsEv
int CSprayCan::ObjectCaps()
{
}

// game/server/player.cpp:5405
static CEntityFactory<CSprayCan> spraycan;

// game/server/player.cpp:5406
spraycanPrecache::CResourcePrecacher s_ResourcePrecacher;

// game/server/player.cpp:5406 sizeof=0x10 (i386)
struct CResourcePrecacher : public CBaseResourcePrecacher
{
public:
	CResourcePrecacher();  // line 5406
	virtual void Cache( IPrecacheHandler *, bool, ResourceList_t, bool );  // line 5406
};

// game/server/player.cpp:5406 @0x620c10 _ZN16spraycanPrecache18CResourcePrecacher5CacheEP16IPrecacheHandlerbP16ResourceList_t__b
void CResourcePrecacher::Cache( IPrecacheHandler *pPrecacheHandler, bool bPrecache, ResourceList_t hResourceList, bool bIgnoreConditionals )
{
}

// game/server/player.cpp:5447 sizeof=0x3c0 (i386)
struct CBloodSplat : public CPointEntity
{
public:
	void Spawn( CBaseEntity * );  // line 5451
	virtual void Think();  // line 5452
};

// game/server/player.cpp:5447 (declaration)
~CBloodSplat();

// game/server/player.cpp:5447 @0x63c9a0 _ZN11CBloodSplatD0Ev
CBloodSplat::~CBloodSplat()
{
	// inlined CPointEntity::~CPointEntity() at line 5447
}

// game/server/player.cpp:5447 @0x63c9f0 _ZN11CBloodSplatD1Ev
CBloodSplat::~CBloodSplat()
{
	// inlined CPointEntity::~CPointEntity() at line 5447
}

// game/server/player.cpp:5447 (declaration)
void CBloodSplat();

// game/server/player.cpp:5451 @0x6218b0 _ZN11CBloodSplat5SpawnEP11CBaseEntity
void CBloodSplat::Spawn( CBaseEntity *pOwner )
{
	// inlined Vector::Vector() at line 5457
	// inlined Vector::operator+() at line 5457
	// inlined Vector::operator VectorByValue&() at line 5457
}

// game/server/player.cpp:5452 @0x630e00 _ZN11CBloodSplat5ThinkEv
void CBloodSplat::Think()
{
	trace_t tr;  // line 5466
	{
		CBasePlayer *pPlayer;  // line 5470
		Vector forward;  // line 5473
		// inlined UTIL_TraceLine() at line 5476
		// inlined CBaseEntity::GetAbsOrigin() at line 5476
		// inlined Vector::operator+() at line 5476
		// inlined CBaseEntity::GetAbsOrigin() at line 5476
		// inlined Vector::operator*() at line 5476
		// inlined CBaseEntity::GetAbsAngles() at line 5474
		// inlined ToBasePlayer() at line 5471
		// inlined CBaseEntity::GetOwnerEntity() at line 5471
	}
}

// game/server/player.cpp:5488 @0x62db00 _ZN11CBasePlayer13GiveNamedItemEPKcib
CBaseEntity *CBasePlayer::GiveNamedItem( const char *pszName, int iSubType, bool removeIfNotCarried )
{
	EHANDLE pent;  // line 5496
	CBaseCombatWeapon *pWeapon;  // line 5508
	// inlined CHandle<CBaseEntity>::operator=() at line 5498
	// inlined CHandle<CBaseEntity>::operator==() at line 5499
	// inlined CHandle<CBaseEntity>::operator->() at line 5506
	// inlined CBaseEntity::AddSpawnFlags() at line 5506
	// inlined CHandle<CBaseEntity>::operator CBaseEntity*() at line 5508
	// inlined CHandle<CBaseEntity>::operator CBaseEntity*() at line 5514
	// inlined CHandle<CBaseEntity>::operator!=() at line 5516
	// inlined CHandle<CBaseEntity>::operator CBaseEntity*() at line 5521
}

// game/server/player.cpp:5530 @0x62fe70 _ZN11CBasePlayer22FindEntityClassForwardEPc
CBaseEntity *CBasePlayer::FindEntityClassForward( char *classname )
{
	trace_t tr;  // line 5532
	Vector forward;  // line 5534
	{
		CBaseEntity *pHit;  // line 5541
		// inlined FClassnameIs() at line 5542
	}
	// inlined UTIL_TraceLine() at line 5538
	// inlined Vector::operator+() at line 5538
	// inlined Vector::operator*() at line 5538
}

// game/server/player.cpp:5558 @0x62fc20 _ZN11CBasePlayer17FindEntityForwardEb
CBaseEntity *CBasePlayer::FindEntityForward( bool fHull )
{
	trace_t tr;  // line 5560
	Vector forward;  // line 5561
	int mask;  // line 5562
	// inlined UTIL_TraceLine() at line 5576
	// inlined Vector::operator+() at line 5576
	// inlined Vector::operator*() at line 5576
}

// game/server/player.cpp:5592 @0x623990 _ZN11CBasePlayer21FindPickerEntityClassEPc
CBaseEntity *CBasePlayer::FindPickerEntityClass( char *classname )
{
	CBaseEntity *pEntity;  // line 5595
	{
		Vector forward;  // line 5600
		Vector origin;  // line 5601
		// inlined Vector::operator=() at line 5603
	}
}

// game/server/player.cpp:5616 @0x625390 _ZN11CBasePlayer16FindPickerEntityEv
CBaseEntity *CBasePlayer::FindPickerEntity()
{
	CMDLCacheCriticalSection cacheCriticalSection;  // line 5618
	CBaseEntity *pEntity;  // line 5621
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 5632
	// inlined CMDLCacheCriticalSection::CMDLCacheCriticalSection() at line 5618
	{
		Vector forward;  // line 5626
		Vector origin;  // line 5627
		// inlined Vector::operator=() at line 5629
	}
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 5632
}

// game/server/player.cpp:5640 @0x623920 _ZN11CBasePlayer16FindPickerAINodeEi
CAI_Node *CBasePlayer::FindPickerAINode( int nNodeType )
{
	Vector forward;  // line 5642
}

// game/server/player.cpp:5652 @0x6247b0 _ZN11CBasePlayer16FindPickerAILinkEv
CAI_Link *CBasePlayer::FindPickerAILink()
{
	Vector forward;  // line 5654
}

// game/server/player.cpp:5668 @0x622a10 _ZN11CBasePlayer20ForceClientDllUpdateEv
void CBasePlayer::ForceClientDllUpdate()
{
}

// game/server/player.cpp:5690 @0x62f830 _ZN11CBasePlayer15ImpulseCommandsEv
void CBasePlayer::ImpulseCommands()
{
	trace_t tr;  // line 5692
	int iImpulse;  // line 5694
	{
		Vector forward;  // line 5737
		// inlined UTIL_TraceLine() at line 5741
		// inlined Vector::operator+() at line 5741
		// inlined Vector::operator*() at line 5741
	}
	{
		CSprayCan *pCan;  // line 5747
	}
	{
		CBaseCombatWeapon *pWeapon;  // line 5712
	}
}

// game/server/player.cpp:5819 @0x628f80 _ZL10CreateJeepP11CBasePlayer
CreateJeep( CBasePlayer *pPlayer )
{
	Vector vecForward;  // line 5822
	CBaseEntity *pJeep;  // line 5824
	{
		Vector vecOrigin;  // line 5827
		QAngle vecAngles;  // line 5828
		// inlined QAngle::QAngle() at line 5828
		// inlined CBaseEntity::GetAbsAngles() at line 5828
		// inlined Vector::operator+() at line 5827
		// inlined Vector::operator+() at line 5827
		// inlined CBaseEntity::GetAbsOrigin() at line 5827
		// inlined Vector::operator*() at line 5827
		// inlined Vector::Vector() at line 5827
	}
}

// game/server/player.cpp:5842 @0x6291b0 _Z16CC_CH_CreateJeepv
CC_CH_CreateJeep()
{
	CBasePlayer *pPlayer;  // line 5844
}

// game/server/player.cpp:5850
static ConCommand ch_createjeep;

// game/server/player.cpp:5856 @0x628d40 _ZL13CreateAirboatP11CBasePlayer
CreateAirboat( CBasePlayer *pPlayer )
{
	Vector vecForward;  // line 5859
	CBaseEntity *pJeep;  // line 5861
	{
		Vector vecOrigin;  // line 5864
		QAngle vecAngles;  // line 5865
		// inlined QAngle::QAngle() at line 5865
		// inlined CBaseEntity::GetAbsAngles() at line 5865
		// inlined Vector::operator+() at line 5864
		// inlined Vector::operator+() at line 5864
		// inlined CBaseEntity::GetAbsOrigin() at line 5864
		// inlined Vector::operator*() at line 5864
		// inlined Vector::Vector() at line 5864
	}
}

// game/server/player.cpp:5881 @0x628f50 _Z19CC_CH_CreateAirboatv
CC_CH_CreateAirboat()
{
	CBasePlayer *pPlayer;  // line 5883
}

// game/server/player.cpp:5891
static ConCommand ch_createairboat;

// game/server/player.cpp:5896 @0x6302b0 _ZN11CBasePlayer20CheatImpulseCommandsEi
void CBasePlayer::CheatImpulseCommands( int iImpulse )
{
	CBaseEntity *pEntity;  // line 5904
	trace_t tr;  // line 5905
	{
		CAI_BaseNPC *pNPC;  // line 6013
	}
	// inlined CBaseEntity::GetClassname() at line 6024
	// inlined string_t::operator!=() at line 6035
	// inlined string_t::ToCStr() at line 6038
	// inlined string_t::operator!=() at line 6039
	{
		trace_t tr;  // line 6046
		edict_t *pWorld;  // line 6048
		Vector start;  // line 6050
		Vector forward;  // line 6051
		Vector end;  // line 6053
		const char *pTextureName;  // line 6058
		// inlined UTIL_TraceLine() at line 6054
		// inlined Vector::operator+() at line 6053
		// inlined Vector::operator*() at line 6053
		// inlined Vector::operator VectorByValue&() at line 6050
	}
	{
		CAI_BaseNPC *pNPC;  // line 6073
		// inlined CBaseEntity::GetClassname() at line 6076
		// inlined CAI_BaseNPC::SetDebugNPC() at line 6077
	}
	{
		Vector forward;  // line 6100
		// inlined Vector::operator*() at line 6104
		// inlined Vector::operator+() at line 6104
		// inlined UTIL_TraceLine() at line 6104
		{
			CBloodSplat *pBlood;  // line 6108
			// inlined _CreateEntityTemplate<CBloodSplat>() at line 6108
		}
	}
	{
		Vector forward;  // line 5918
		// inlined Vector::operator VectorByValue&() at line 5918
		// inlined Vector::operator*() at line 5919
		// inlined Vector::operator+() at line 5919
		// inlined Vector::operator VectorByValue&() at line 5919
	}
}

// game/server/player.cpp:6127 @0x625e40 _ZN11CBasePlayer13ClientCommandERK8CCommand
bool CBasePlayer::ClientCommand( const CCommand &args )
{
	const char *cmd;  // line 6129
	{
		int nRecip;  // line 6362
		int nRecords;  // line 6367
		CBasePlayer *pl;  // line 6373
		// inlined CBaseEntity::entindex() at line 6362
		// inlined CCommand::Arg() at line 6365
		// inlined CCommand::Arg() at line 6365
		// inlined CCommand::Arg() at line 6370
	}
	// inlined CCommand::operator[]() at line 6129
	{
		ConVarRef mp_allowspectators;  // line 6184
	}
	{
		int nRole;  // line 6163
		IServerVehicle *pVehicle;  // line 6164
		// inlined CCommand::operator[]() at line 6163
	}
	{
		int index;  // line 6317
		CBasePlayer *target;  // line 6319
		// inlined CCommand::operator[]() at line 6323
	}
	{
		int mode;  // line 6212
	}
	{
		CBaseEntity *target;  // line 6299
	}
	{
		CBaseEntity *target;  // line 6281
	}
	{
		Vector origin;  // line 6345
		QAngle angle;  // line 6350
		// inlined CCommand::operator[]() at line 6347
		// inlined CCommand::operator[]() at line 6348
		// inlined CCommand::operator[]() at line 6351
		// inlined CCommand::operator[]() at line 6352
	}
}

// game/server/player.cpp:6391 @0x624e00 _ZN11CBasePlayer10BumpWeaponEP17CBaseCombatWeapon
bool CBasePlayer::BumpWeapon( CBaseCombatWeapon *pWeapon )
{
	CBaseCombatCharacter *pOwner;  // line 6393
	// inlined CBaseEntity::GetClassname() at line 6425
	// inlined CBaseEntity::AddSolidFlags() at line 6448
}

// game/server/player.cpp:6485 @0x638170 _ZN11CBasePlayer16RemovePlayerItemEP17CBaseCombatWeapon
bool CBasePlayer::RemovePlayerItem( CBaseCombatWeapon *pItem )
{
	// inlined CHandle<CBaseCombatWeapon>::operator==() at line 6495
	// inlined CBasePlayer::ResetAutoaim() at line 6489
}

// game/server/player.cpp:6509 @0x6212e0 _ZN11CBasePlayer13ShowViewModelEb
void CBasePlayer::ShowViewModel( bool bShow )
{
	// inlined CNetworkVarBase<bool,CPlayerLocalData::NetworkVar_m_bDrawViewmodel>::operator=<bool>() at line 6511
}

// game/server/player.cpp:6518 (declaration)
void ShowCrosshair( bool bShow );

// game/server/player.cpp:6518 @0x62bf10 _ZN11CBasePlayer13ShowCrosshairEb
void CBasePlayer::ShowCrosshair( bool bShow )
{
	// inlined CNetworkVarBase<int,CPlayerLocalData::NetworkVar_m_iHideHUD>::operator&=<int>() at line 6522
	// inlined CNetworkVarBase<int,CPlayerLocalData::NetworkVar_m_iHideHUD>::operator|=<int>() at line 6526
}

// game/server/player.cpp:6533 @0x6242d0 _ZN11CBasePlayer24ScriptIsPlayerNoclippingEv
bool CBasePlayer::ScriptIsPlayerNoclipping()
{
}

// game/server/player.cpp:6542 @0x620c60 _ZN11CBasePlayer10BodyAnglesEv
QAngle CBasePlayer::BodyAngles()
{
}

// game/server/player.cpp:6556 @0x628ba0 _ZN11CBasePlayer10BodyTargetERK6Vectorb
Vector CBasePlayer::BodyTarget( const Vector &posSrc, bool bNoisy )
{
	// inlined Vector::operator*() at line 6564
	// inlined CBaseEntity::GetAbsOrigin() at line 6564
	// inlined Vector::operator+() at line 6564
}

// game/server/player.cpp:6583 @0x62e680 _ZN11CBasePlayer16UpdateClientDataEv
void CBasePlayer::UpdateClientData()
{
	CSingleUserRecipientFilter user;  // line 6585
	CWorld *world;  // line 6614
	// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 6673
	// inlined CNetworkVarBase<bool,CPlayerLocalData::NetworkVar_m_bPoisoned>::operator=<bool>() at line 6664
	{
		int i;  // line 6655
	}
	{
		variant_t value;  // line 6609
		// inlined variant_t::variant_t() at line 6610
		// inlined variant_t::variant_t() at line 6609
		{
			variant_t value;  // line 6604
			// inlined variant_t::variant_t() at line 6604
			// inlined variant_t::variant_t() at line 6605
		}
	}
	// inlined CSingleUserRecipientFilter::CSingleUserRecipientFilter() at line 6585
	// inlined CNetworkVarBase<int,CPlayerLocalData::NetworkVar_m_iHideHUD>::operator|=<int>() at line 6668
	// inlined CNetworkVarBase<int,CPlayerLocalData::NetworkVar_m_iHideHUD>::operator&=<int>() at line 6670
	// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 6673
}

// game/server/player.cpp:6676 @0x62e590 _ZN11CBasePlayer13UpdateBatteryEv
void CBasePlayer::UpdateBattery()
{
	{
		CSingleUserRecipientFilter user;  // line 6685
		// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 6690
		// inlined CSingleUserRecipientFilter::CSingleUserRecipientFilter() at line 6685
		// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 6690
	}
}

// game/server/player.cpp:6695 @0x62e490 _ZN11CBasePlayer12RumbleEffectEhhh
void CBasePlayer::RumbleEffect( unsigned char index, unsigned char rumbleData, unsigned char rumbleFlags )
{
	CSingleUserRecipientFilter filter;  // line 6700
	// inlined CSingleUserRecipientFilter::CSingleUserRecipientFilter() at line 6700
	// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 6707
	// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 6707
}

// game/server/player.cpp:6710 @0x622360 _ZN11CBasePlayer13EnableControlEb
void CBasePlayer::EnableControl( bool fControl )
{
}

// game/server/player.cpp:6719 @0x62e3b0 _ZN11CBasePlayer16CheckTrainUpdateEv
void CBasePlayer::CheckTrainUpdate()
{
	{
		CSingleUserRecipientFilter user;  // line 6723
		// inlined CSingleUserRecipientFilter::CSingleUserRecipientFilter() at line 6723
		// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 6731
		// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 6731
	}
}

// game/server/player.cpp:6739 @0x620ca0 _ZN11CBasePlayer13ShouldAutoaimEv
bool CBasePlayer::ShouldAutoaim()
{
}

// game/server/player.cpp:6761 @0x623c70 _ZN11CBasePlayer16GetAutoaimVectorEf
Vector CBasePlayer::GetAutoaimVector( float flScale )
{
	autoaim_params_t params;  // line 6763
	// inlined autoaim_params_t::autoaim_params_t() at line 6763
}

// game/server/player.cpp:6775 @0x624640 _ZN11CBasePlayer16GetAutoaimVectorEff
Vector CBasePlayer::GetAutoaimVector( float flScale, float flMaxDist )
{
	autoaim_params_t params;  // line 6777
	// inlined autoaim_params_t::autoaim_params_t() at line 6777
}

// game/server/player.cpp:6789 @0x6245b0 _ZN11CBasePlayer16GetAutoaimVectorEfffP10AimResults
Vector CBasePlayer::GetAutoaimVector( float flScale, float flMaxDist, float flMaxDeflection, AimResults *pAimResults )
{
	autoaim_params_t params;  // line 6791
	// inlined autoaim_params_t::autoaim_params_t() at line 6791
}

// game/server/player.cpp:6814 @0x633f00 _ZN11CBasePlayer16GetAutoaimVectorER16autoaim_params_t
void CBasePlayer::GetAutoaimVector( autoaim_params_t &params )
{
	Vector vecShootPosition;  // line 6830
	QAngle angles;  // line 6831
	Vector forward;  // line 6858
	// inlined Vector::operator=() at line 6871
	// inlined QAngle::operator QAngleByValue&() at line 6869
	// inlined QAngle::operator+() at line 6869
	// inlined QAngle::operator+() at line 6869
	// inlined QAngle::operator=() at line 6868
	// inlined QAngle::operator*() at line 6868
	// inlined QAngle::Init() at line 6857
	// inlined CNetworkVarBase<bool,CBasePlayer::NetworkVar_m_fOnTarget>::operator=<bool>() at line 6836
	// inlined QAngle::operator QAngleByValue&() at line 6831
	// inlined CHandle<CBaseEntity>::Set() at line 6819
	// inlined Vector::operator=() at line 6820
	{
		Vector forward;  // line 6824
		// inlined QAngle::operator+() at line 6825
		// inlined QAngle::operator QAngleByValue&() at line 6825
		// inlined Vector::operator=() at line 6826
	}
	// inlined QAngle::operator=() at line 6862
	// inlined QAngle::operator+() at line 6863
	// inlined QAngle::operator QAngleByValue&() at line 6863
}

// game/server/player.cpp:6880 @0x622870 _ZN11CBasePlayer15GetAutoaimScoreERK6VectorS2_S2_P11CBaseEntityfP17CBaseCombatWeapon
float CBasePlayer::GetAutoaimScore( const Vector &eyePosition, const Vector &viewDir, const Vector &vecTarget, CBaseEntity *pTarget, float fScale, CBaseCombatWeapon *pActiveWeapon )
{
	float targetRadius;  // line 6882
	float targetRadiusSqr;  // line 6893
	Vector vecNearestPoint;  // line 6895
	Vector vecDiff;  // line 6896
	float radiusSqr;  // line 6897
	float score;  // line 6899
	// inlined Vector::LengthSqr() at line 6897
	// inlined Vector::operator-() at line 6896
	// inlined Vector::operator VectorByValue&() at line 6895
	// inlined Vector::operator+() at line 6895
	// inlined Vector::operator*() at line 6895
	// inlined Square<float>() at line 6893
}

// game/server/player.cpp:6924 sizeof=0x14 (i386)
struct CTraceFilterSkipTwoEntitiesAndTeammates : public CTraceFilterSkipTwoEntities
{
public:
	CTraceFilterSkipTwoEntitiesAndTeammates( const IHandleEntity *, const IHandleEntity *, int );  // line 6926
	virtual bool ShouldHitEntity( IHandleEntity *, int );  // line 6931
};

// game/server/player.cpp:6931 @0x63b5e0 _ZN39CTraceFilterSkipTwoEntitiesAndTeammates15ShouldHitEntityEP13IHandleEntityi
bool CTraceFilterSkipTwoEntitiesAndTeammates::ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
{
	CBaseEntity *pEntity;  // line 6933
	const CBaseEntity *pSelf;  // line 6937
	// inlined EntityFromEntityHandle() at line 6933
	// inlined EntityFromEntityHandle() at line 6937
}

// game/server/player.cpp:6950 @0x633170 _ZN11CBasePlayer17AutoaimDeflectionER6VectorR16autoaim_params_t
QAngle CBasePlayer::AutoaimDeflection( Vector &vecSrc, autoaim_params_t &params )
{
	float bestscore;  // line 6952
	float score;  // line 6953
	QAngle eyeAngles;  // line 6954
	Vector bestdir;  // line 6955
	CBaseEntity *bestent;  // line 6956
	trace_t tr;  // line 6957
	Vector v_forward;  // line 6958
	Vector v_right;  // line 6958
	Vector v_up;  // line 6958
	QAngle bestang;  // line 6959
	CBaseEntity *pIgnore;  // line 6972
	CTraceFilterSkipTwoEntitiesAndTeammates traceFilter;  // line 6978
	CBaseEntity *pEntHit;  // line 6982
	int count;  // line 7019
	float maxDeflection;  // line 7141
	{
		bool bAimAtThis;  // line 6990
		{
			int iRelationType;  // line 6993
		}
		// inlined CNetworkVarBase<bool,CBasePlayer::NetworkVar_m_fOnTarget>::operator=<bool>() at line 7003
		// inlined CHandle<CBaseEntity>::Set() at line 7008
		// inlined Vector::operator=() at line 7009
		// inlined Vector::operator=() at line 7010
	}
	{
		CBaseEntity **pList;  // line 7022
		{
			int i;  // line 7025
			{
				Vector center;  // line 7027
				Vector dir;  // line 7028
				CBaseEntity *pEntity;  // line 7029
				float dist;  // line 7066
				float dot;  // line 7073
				// inlined UTIL_TraceLine() at line 7101
				// inlined CBaseEntity::edict() at line 7039
				// inlined Vector::operator=() at line 7062
				// inlined Vector::operator-() at line 7064
				// inlined Vector::operator=() at line 7064
				// inlined Vector::Length2D() at line 7066
				// inlined DotProduct() at line 7073
				// inlined Vector::operator=() at line 7112
			}
		}
		{
			QAngle bestang;  // line 7116
			// inlined QAngle::operator-=() at line 7126
			// inlined QAngle::operator-() at line 7126
			// inlined QAngle::operator-=() at line 7122
			// inlined CNetworkVarBase<bool,CBasePlayer::NetworkVar_m_fOnTarget>::operator=<bool>() at line 7129
			// inlined CHandle<CBaseEntity>::Set() at line 7132
			// inlined Vector::operator=() at line 7133
			// inlined Vector::operator=() at line 7134
		}
	}
	// inlined CNetworkVarBase<bool,CBasePlayer::NetworkVar_m_fOnTarget>::operator=<bool>() at line 6961
	// inlined QAngle::operator=() at line 6963
	// inlined QAngle::operator+() at line 6964
	// inlined QAngle::operator+() at line 6964
	// inlined QAngle::operator QAngleByValue&() at line 6964
	// inlined Vector::operator=() at line 6967
	// inlined QAngle::operator=() at line 6970
	// inlined CTraceFilterSkipTwoEntitiesAndTeammates::CTraceFilterSkipTwoEntitiesAndTeammates() at line 6978
	// inlined Vector::operator*() at line 6979
	// inlined Vector::operator+() at line 6979
	// inlined UTIL_TraceLine() at line 6979
}

// game/server/player.cpp:7154 (declaration)
void ResetAutoaim();

// game/server/player.cpp:7154 @0x6330c0 _ZN11CBasePlayer12ResetAutoaimEv
void CBasePlayer::ResetAutoaim()
{
	// inlined QAngle::Init() at line 7156
	// inlined CNetworkVarBase<bool,CBasePlayer::NetworkVar_m_fOnTarget>::operator=<bool>() at line 7158
}

// game/server/player.cpp:7169 @0x624c60 _ZN11CBasePlayer11Weapon_DropEP17CBaseCombatWeaponPK6VectorS4_
void CBasePlayer::Weapon_Drop( CBaseCombatWeapon *pWeapon, const Vector *pvecTarget, const Vector *pVelocity )
{
	bool bWasActiveWeapon;  // line 7171
	{
		CBaseViewModel *vm;  // line 7191
		// inlined CBasePlayer::GetViewModel() at line 7191
	}
}

// game/server/player.cpp:7204 @0x621c00 _ZN11CBasePlayer15Weapon_DropSlotEi
void CBasePlayer::Weapon_DropSlot( int weaponSlot )
{
	CBaseCombatWeapon *pWeapon;  // line 7206
	{
		int i;  // line 7209
	}
}

// game/server/player.cpp:7227 @0x622800 _ZN11CBasePlayer12Weapon_EquipEP17CBaseCombatWeapon
void CBasePlayer::Weapon_Equip( CBaseCombatWeapon *pWeapon )
{
	bool bShouldSwitch;  // line 7231
}

// game/server/player.cpp:7252 @0x6244a0 _ZN11CBasePlayer18HasNamedPlayerItemEPKc
CBaseEntity *CBasePlayer::HasNamedPlayerItem( const char *pszItemName )
{
	{
		int i;  // line 7254
		// inlined FStrEq() at line 7259
		// inlined CBaseEntity::GetClassname() at line 7259
	}
}

// game/server/player.cpp:7277 @0x622510 _ZN11CBasePlayer10ChangeTeamEibb
void CBasePlayer::ChangeTeam( int iTeamNum, bool bAutoTeam, bool bSilent )
{
	IGameEvent *event;  // line 7294
	// inlined CBasePlayer::GetUserID() at line 7297
}

// game/server/player.cpp:7336 (declaration)
void LockPlayerInPlace();

// game/server/player.cpp:7336 @0x6224a0 _ZN11CBasePlayer17LockPlayerInPlaceEv
void CBasePlayer::LockPlayerInPlace()
{
}

// game/server/player.cpp:7353 @0x622450 _ZN11CBasePlayer12UnlockPlayerEv
void CBasePlayer::UnlockPlayer()
{
}

// game/server/player.cpp:7364 @0x621330 _ZN11CBasePlayer12GetUseEntityEv
CBaseEntity *CBasePlayer::GetUseEntity()
{
	// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hUseEntity>::operator CBaseEntity*() at line 7366
}

// game/server/player.cpp:7369 @0x620d10 _ZN11CBasePlayer21GetPotentialUseEntityEv
CBaseEntity *CBasePlayer::GetPotentialUseEntity()
{
}

// game/server/player.cpp:7377 (declaration)
void HideViewModels();

// game/server/player.cpp:7377 @0x620d30 _ZN11CBasePlayer14HideViewModelsEv
void CBasePlayer::HideViewModels()
{
	{
		int i;  // line 7379
		{
			CBaseViewModel *vm;  // line 7381
			// inlined CBasePlayer::GetViewModel() at line 7381
		}
	}
}

// game/server/player.cpp:7390 sizeof=0x3c0 (i386)
struct CStripWeapons : public CPointEntity
{
public:
	void InputStripWeapons( inputdata_t & );  // line 7393
	void InputStripWeaponsAndSuit( inputdata_t & );  // line 7394
	void StripWeapons( inputdata_t &, bool );  // line 7396
};

// game/server/player.cpp:7390 (declaration)
void CStripWeapons();

// game/server/player.cpp:7390 (declaration)
~CStripWeapons();

// game/server/player.cpp:7390 @0x63c820 _ZN13CStripWeaponsD0Ev
CStripWeapons::~CStripWeapons()
{
	// inlined CPointEntity::~CPointEntity() at line 7390
}

// game/server/player.cpp:7390 @0x63c870 _ZN13CStripWeaponsD1Ev
CStripWeapons::~CStripWeapons()
{
	// inlined CPointEntity::~CPointEntity() at line 7390
}

// game/server/player.cpp:7393 @0x62c100 _ZN13CStripWeapons17InputStripWeaponsER11inputdata_t
void CStripWeapons::InputStripWeapons( inputdata_t &data )
{
	// inlined CStripWeapons::StripWeapons() at line 7410
}

// game/server/player.cpp:7394 @0x62c190 _ZN13CStripWeapons24InputStripWeaponsAndSuitER11inputdata_t
void CStripWeapons::InputStripWeaponsAndSuit( inputdata_t &data )
{
	// inlined CStripWeapons::StripWeapons() at line 7415
}

// game/server/player.cpp:7396 @0x6222c0 _ZN13CStripWeapons12StripWeaponsER11inputdata_tb
void CStripWeapons::StripWeapons( inputdata_t &data, bool stripSuit )
{
	CBasePlayer *pPlayer;  // line 7420
}

// game/server/player.cpp:7397 @0x620de0 _ZN13CStripWeapons14GetDataDescMapEv
datamap_t *CStripWeapons::GetDataDescMap()
{
}

// game/server/player.cpp:7397 @0x620df0 _ZN13CStripWeapons10GetBaseMapEv
datamap_t *CStripWeapons::GetBaseMap()
{
}

// game/server/player.cpp:7400
static CEntityFactory<CStripWeapons> player_weaponstrip;

// game/server/player.cpp:7402 @0xa6b10 _Z11DataMapInitI13CStripWeaponsEP9datamap_tPT_
datamap_t *DataMapInit<CStripWeapons>( CStripWeapons * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 7402
	CDatadescGeneratedNameHolder nameHolder;  // line 7402
	typedescription_t dataDesc[3];  // line 7402
}

// game/server/player.cpp:7402
datamap_t *g_DataMapHolder;

// game/server/player.cpp:7439 sizeof=0x3cc (i386)
struct CRevertSaved : public CPointEntity
{
public:
	virtual void Use( CBaseEntity *, CBaseEntity *, $_170, float );  // line 7442
	void LoadThink();  // line 7443
	float Duration();  // line 7447
	float HoldTime();  // line 7448
	float LoadTime();  // line 7449
	void SetDuration( float );  // line 7451
	void SetHoldTime( float );  // line 7452
	void SetLoadTime( float );  // line 7453
	void InputReload( inputdata_t & );  // line 7456
private:
	float m_loadTime; // +0x3c0  // line 7460
	float m_Duration; // +0x3c4  // line 7461
	float m_HoldTime; // +0x3c8  // line 7462
};

// game/server/player.cpp:7439 (declaration)
void CRevertSaved();

// game/server/player.cpp:7439 (declaration)
~CRevertSaved();

// game/server/player.cpp:7439 @0x63c8a0 _ZN12CRevertSavedD0Ev
CRevertSaved::~CRevertSaved()
{
	// inlined CPointEntity::~CPointEntity() at line 7439
}

// game/server/player.cpp:7439 @0x63c8f0 _ZN12CRevertSavedD1Ev
CRevertSaved::~CRevertSaved()
{
	// inlined CPointEntity::~CPointEntity() at line 7439
}

// game/server/player.cpp:7442 @0x623740 _ZN12CRevertSaved3UseEP11CBaseEntityS1_8USE_TYPEf
void CRevertSaved::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, $_170 useType, float value )
{
	CBasePlayer *pPlayer;  // line 7507
	// inlined CNetworkVarBase<bool,CPlayerState::NetworkVar_deadflag>::operator=<bool>() at line 7512
}

// game/server/player.cpp:7443 @0x620e20 _ZN12CRevertSaved9LoadThinkEv
void CRevertSaved::LoadThink()
{
}

// game/server/player.cpp:7445 @0x620e00 _ZN12CRevertSaved14GetDataDescMapEv
datamap_t *CRevertSaved::GetDataDescMap()
{
}

// game/server/player.cpp:7445 @0x620e10 _ZN12CRevertSaved10GetBaseMapEv
datamap_t *CRevertSaved::GetBaseMap()
{
}

// game/server/player.cpp:7456 @0x625750 _ZN12CRevertSaved11InputReloadER11inputdata_t
void CRevertSaved::InputReload( inputdata_t &inputdata )
{
	CBasePlayer *pPlayer;  // line 7528
	// inlined CNetworkVarBase<bool,CPlayerState::NetworkVar_deadflag>::operator=<bool>() at line 7533
}

// game/server/player.cpp:7466
static CEntityFactory<CRevertSaved> player_loadsaved;

// game/server/player.cpp:7468 @0xa7000 _Z11DataMapInitI12CRevertSavedEP9datamap_tPT_
datamap_t *DataMapInit<CRevertSaved>( CRevertSaved * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 7480
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 7468
	CDatadescGeneratedNameHolder nameHolder;  // line 7468
	typedescription_t dataDesc[6];  // line 7468
}

// game/server/player.cpp:7468
datamap_t *g_DataMapHolder;

// game/server/player.cpp:7482 @0x6223d0 _Z20CreatePlayerLoadSave6Vectorfff
CBaseEntity *CreatePlayerLoadSave( Vector vOrigin, float flDuration, float flHoldTime, float flLoadTime )
{
	CRevertSaved *pRevertSaved;  // line 7484
	// inlined CRevertSaved::SetDuration() at line 7492
	// inlined CRevertSaved::SetHoldTime() at line 7493
	// inlined CRevertSaved::SetLoadTime() at line 7494
}

// game/server/player.cpp:7560 sizeof=0x3c0 (i386)
struct CMovementSpeedMod : public CPointEntity
{
public:
	void InputSpeedMod( inputdata_t & );  // line 7563
private:
	int GetDisabledButtonMask();  // line 7566
};

// game/server/player.cpp:7560 (declaration)
~CMovementSpeedMod();

// game/server/player.cpp:7560 @0x63b510 _ZN17CMovementSpeedModD1Ev
CMovementSpeedMod::~CMovementSpeedMod()
{
	// inlined CPointEntity::~CPointEntity() at line 7560
}

// game/server/player.cpp:7560 (declaration)
void CMovementSpeedMod();

// game/server/player.cpp:7560 @0x63ca20 _ZN17CMovementSpeedModD0Ev
CMovementSpeedMod::~CMovementSpeedMod()
{
	// inlined CPointEntity::~CPointEntity() at line 7560
}

// game/server/player.cpp:7563 @0x632850 _ZN17CMovementSpeedMod13InputSpeedModER11inputdata_t
void CMovementSpeedMod::InputSpeedMod( inputdata_t &data )
{
	CBasePlayer *pPlayer;  // line 7616
	// inlined CBasePlayer::SetLaggedMovementValue() at line 7683
	// inlined variant_t::Float() at line 7683
	// inlined CNetworkVarBase<int,CPlayerLocalData::NetworkVar_m_iHideHUD>::operator&=<int>() at line 7679
	// inlined CNetworkVarBase<int,CPlayerLocalData::NetworkVar_m_iHideHUD>::operator|=<int>() at line 7657
	// inlined CBasePlayer::DisableButtons() at line 7652
	// inlined CMovementSpeedMod::GetDisabledButtonMask() at line 7652
	// inlined variant_t::Float() at line 7629
	// inlined CMovementSpeedMod::GetDisabledButtonMask() at line 7674
	// inlined CBasePlayer::EnableButtons() at line 7674
	// inlined CBaseCombatCharacter::ClearActiveWeapon() at line 7638
	// inlined CBasePlayer::HideViewModels() at line 7641
	// inlined CBasePlayer::Weapon_GetLast() at line 7665
}

// game/server/player.cpp:7566 @0x620e80 _ZN17CMovementSpeedMod21GetDisabledButtonMaskEv
int CMovementSpeedMod::GetDisabledButtonMask()
{
	int nMask;  // line 7579
	// inlined CBaseEntity::HasSpawnFlags() at line 7581
}

// game/server/player.cpp:7568 @0x620e60 _ZN17CMovementSpeedMod14GetDataDescMapEv
datamap_t *CMovementSpeedMod::GetDataDescMap()
{
}

// game/server/player.cpp:7568 @0x620e70 _ZN17CMovementSpeedMod10GetBaseMapEv
datamap_t *CMovementSpeedMod::GetBaseMap()
{
}

// game/server/player.cpp:7571
static CEntityFactory<CMovementSpeedMod> player_speedmod;

// game/server/player.cpp:7573 @0xa6cb0 _Z11DataMapInitI17CMovementSpeedModEP9datamap_tPT_
datamap_t *DataMapInit<CMovementSpeedMod>( CMovementSpeedMod * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 7573
	CDatadescGeneratedNameHolder nameHolder;  // line 7573
	typedescription_t dataDesc[2];  // line 7573
}

// game/server/player.cpp:7573
datamap_t *g_DataMapHolder;

// game/server/player.cpp:7688 @0x620ee0 _Z41SendProxy_CropFlagsToPlayerFlagBitsLengthPK8SendPropPKvS3_P8DVariantii
SendProxy_CropFlagsToPlayerFlagBitsLength( const SendProp *pProp, const void *pStruct, const void *pVarData, DVariant *pOut, int iElement, int objectID )
{
	int mask;  // line 7690
	int data;  // line 7691
}

// game/server/player.cpp:7696 @0x6222a0 _Z28SendProxy_SendLocalDataTablePK8SendPropPKvS3_P20CSendProxyRecipientsi
void *SendProxy_SendLocalDataTable( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID )
{
}

// game/server/player.cpp:7701
static CNonModifiedPointerProxy __proxy_SendProxy_SendLocalDataTable;

// game/server/player.cpp:7703 @0x622280 _Z31SendProxy_SendNonLocalDataTablePK8SendPropPKvS3_P20CSendProxyRecipientsi
void *SendProxy_SendNonLocalDataTable( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID )
{
}

// game/server/player.cpp:7708
static CNonModifiedPointerProxy __proxy_SendProxy_SendNonLocalDataTable;

// game/server/player.cpp:7714 @0xa5cb0 _Z15ServerClassInitIN14DT_PlayerState7ignoredEEiPT_
int ServerClassInit<DT_PlayerState::ignored>( DT_PlayerState::ignored * )
{
	SendTable &sendTable;  // line 7714
	char *const g_pSendTableName;  // line 7714
	SendProp g_SendProps[2];  // line 7714
}

// game/server/player.cpp:7714
SendTable g_SendTable;

// game/server/player.cpp:7714
int g_SendTableInit;

// game/server/player.cpp:7722 @0xa5e50 _Z15ServerClassInitIN23DT_LocalPlayerExclusive7ignoredEEiPT_
int ServerClassInit<DT_LocalPlayerExclusive::ignored>( DT_LocalPlayerExclusive::ignored * )
{
	SendTable &sendTable;  // line 7722
	char *const g_pSendTableName;  // line 7722
	SendProp g_SendProps[25];  // line 7722
}

// game/server/player.cpp:7722
SendTable g_SendTable;

// game/server/player.cpp:7722
int g_SendTableInit;

// game/server/player.cpp:7774 @0x620f00 _ZN11CBasePlayer14GetServerClassEv
ServerClass *CBasePlayer::GetServerClass()
{
}

// game/server/player.cpp:7774 @0x620f10 _ZN11CBasePlayer40YouForgotToImplementOrDeclareServerClassEv
int CBasePlayer::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/player.cpp:7774 @0xa51c0 _Z15ServerClassInitIN13DT_BasePlayer7ignoredEEiPT_
int ServerClassInit<DT_BasePlayer::ignored>( DT_BasePlayer::ignored * )
{
	SendTable &sendTable;  // line 7774
	char *const g_pSendTableName;  // line 7774
	SendProp g_SendProps[32];  // line 7774
}

// game/server/player.cpp:7774
SendTable g_SendTable;

// game/server/player.cpp:7774
int g_SendTableInit;

// game/server/player.cpp:7774
static ServerClass g_CBasePlayer_ClassReg;

// game/server/player.cpp:7774
void CBasePlayer::m_pClassSendTable;

// game/server/player.cpp:7831 @0x622030 _ZN11CBasePlayer19SetupVPhysicsShadowERK6VectorS2_P12CPhysCollidePKcS4_S6_
void CBasePlayer::SetupVPhysicsShadow( const Vector &vecAbsOrigin, const Vector &vecAbsVelocity, CPhysCollide *pStandModel, const char *pStandHullName, CPhysCollide *pCrouchModel, const char *pCrouchHullName )
{
	solid_t solid;  // line 7833
}

// game/server/player.cpp:7876 @0x620f20 _ZN11CBasePlayer17VPhysicsCollisionEiP21gamevcollisionevent_t
void CBasePlayer::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
}

// game/server/player.cpp:7883 @0x621fd0 _ZN11CBasePlayer14VPhysicsUpdateEP14IPhysicsObject
void CBasePlayer::VPhysicsUpdate( IPhysicsObject *pPhysics )
{
	float savedImpact;  // line 7885
}

// game/server/player.cpp:7894 @0x620f30 _ZN11CBasePlayer17ShouldSavePhysicsEv
bool CBasePlayer::ShouldSavePhysics()
{
}

// game/server/player.cpp:7902 @0x623b90 _ZN11CBasePlayer14InitVCollisionERK6VectorS2_
void CBasePlayer::InitVCollision( const Vector &vecAbsOrigin, const Vector &vecAbsVelocity )
{
	CPhysCollide *pModel;  // line 7911
	CPhysCollide *pCrouchModel;  // line 7912
}

// game/server/player.cpp:7918 @0x621f00 _ZN11CBasePlayer21VPhysicsDestroyObjectEv
void CBasePlayer::VPhysicsDestroyObject()
{
}

// game/server/player.cpp:7953 @0x62d540 _ZN11CBasePlayer6GetFOVEv
int CBasePlayer::GetFOV()
{
	int nDefaultFOV;  // line 7955
	int fFOV;  // line 7968
	float deltaTime;  // line 7974
	// inlined CNetworkVarBase<int,CBasePlayer::NetworkVar_m_iFOVStart>::operator=<int>() at line 7979
	// inlined SimpleSplineRemapValClamped() at line 7983
}

// game/server/player.cpp:7994 (declaration)
void GetFOVForNetworking();

// game/server/player.cpp:7994 @0x621e60 _ZN11CBasePlayer19GetFOVForNetworkingEv
int CBasePlayer::GetFOVForNetworking()
{
	int nDefaultFOV;  // line 7996
	int fFOV;  // line 8009
}

// game/server/player.cpp:8023 @0x627a90 _ZN11CBasePlayer39GetFOVDistanceAdjustFactorForNetworkingEv
float CBasePlayer::GetFOVDistanceAdjustFactorForNetworking()
{
	float defaultFOV;  // line 8025
	float localFOV;  // line 8026
	// inlined CBasePlayer::GetFOVForNetworking() at line 8026
}

// game/server/player.cpp:8041 (declaration)
void SetDefaultFOV( int FOV );

// game/server/player.cpp:8041 @0x62d6c0 _ZN11CBasePlayer13SetDefaultFOVEi
void CBasePlayer::SetDefaultFOV( int FOV )
{
	// inlined CNetworkVarBase<int,CBasePlayer::NetworkVar_m_iDefaultFOV>::operator=<int>() at line 8043
}

// game/server/player.cpp:8050 @0x626a10 _ZN11CBasePlayer28ModifyOrAppendPlayerCriteriaERN13ResponseRules11CriteriaSetE
void CBasePlayer::ModifyOrAppendPlayerCriteria( ResponseRules::CriteriaSet &set )
{
	float healthfrac;  // line 8054
	CBaseCombatWeapon *weapon;  // line 8062
	// inlined Vector::Length() at line 8075
	// inlined CBaseEntity::GetAbsVelocity() at line 8075
	// inlined CBaseEntity::GetClassname() at line 8065
}

// game/server/player.cpp:8103 (declaration)
void GetPunchAngle();

// game/server/player.cpp:8103 @0x620f40 _ZN11CBasePlayer13GetPunchAngleEv
const QAngle &CBasePlayer::GetPunchAngle()
{
}

// game/server/player.cpp:8109 @0x6254e0 _ZN11CBasePlayer13SetPunchAngleERK6QAngle
void CBasePlayer::SetPunchAngle( const QAngle &punchAngle )
{
	{
		int index;  // line 8115
		{
			int i;  // line 8117
			{
				CBasePlayer *pPlayer;  // line 8119
			}
		}
		// inlined CBaseEntity::entindex() at line 8115
	}
	// inlined CNetworkVectorBase<QAngle,CPlayerLocalData::NetworkVar_m_vecPunchAngle>::operator=() at line 8111
}

// game/server/player.cpp:8129 @0x625630 _ZN11CBasePlayer13SetPunchAngleEif
void CBasePlayer::SetPunchAngle( int axis, float value )
{
	{
		int index;  // line 8135
		{
			int i;  // line 8137
			{
				CBasePlayer *pPlayer;  // line 8139
			}
		}
		// inlined CBaseEntity::entindex() at line 8135
	}
	// inlined CNetworkVectorBase<QAngle,CPlayerLocalData::NetworkVar_m_vecPunchAngle>::Set() at line 8131
}

// game/server/player.cpp:8152 @0x62c780 _ZN11CBasePlayer26ActivateMovementConstraintEP11CBaseEntityRK6Vectorfffb
void CBasePlayer::ActivateMovementConstraint( CBaseEntity *pEntity, const Vector &vecCenter, float flRadius, float flConstraintWidth, float flSpeedFactor, bool constraintPastRadius )
{
	// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hConstraintEntity>::operator=() at line 8154
	// inlined CNetworkVectorBase<Vector,CBasePlayer::NetworkVar_m_vecConstraintCenter>::operator=() at line 8155
	// inlined CNetworkVarBase<float,CBasePlayer::NetworkVar_m_flConstraintRadius>::operator=<float>() at line 8156
	// inlined CNetworkVarBase<float,CBasePlayer::NetworkVar_m_flConstraintWidth>::operator=<float>() at line 8157
	// inlined CNetworkVarBase<float,CBasePlayer::NetworkVar_m_flConstraintSpeedFactor>::operator=<float>() at line 8158
	// inlined CNetworkVarBase<bool,CBasePlayer::NetworkVar_m_bConstraintPastRadius>::operator=<bool>() at line 8159
}

// game/server/player.cpp:8165 @0x6310f0 _ZN11CBasePlayer28DeactivateMovementConstraintEv
void CBasePlayer::DeactivateMovementConstraint()
{
	// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hConstraintEntity>::operator=() at line 8167
	// inlined CNetworkVarBase<float,CBasePlayer::NetworkVar_m_flConstraintRadius>::operator=<float>() at line 8168
	// inlined CNetworkVectorBase<Vector,CBasePlayer::NetworkVar_m_vecConstraintCenter>::operator=() at line 8169
}

// game/server/player.cpp:8180 @0x6300d0 _ZN11CBasePlayer17DoubleCheckUseNPCEP11CBaseEntityRK6VectorS4_
CBaseEntity *CBasePlayer::DoubleCheckUseNPC( CBaseEntity *pNPC, const Vector &vecSrc, const Vector &vecDir )
{
	trace_t tr;  // line 8182
	// inlined UTIL_TraceLine() at line 8184
	// inlined Vector::operator+() at line 8184
	// inlined Vector::operator*() at line 8184
}

// game/server/player.cpp:8197 @0x623a90 _ZNK11CBasePlayer5IsBotEv
bool CBasePlayer::IsBot()
{
}

// game/server/player.cpp:8202 @0x623a70 _ZNK11CBasePlayer12IsFakeClientEv
bool CBasePlayer::IsFakeClient()
{
}

// game/server/player.cpp:8207 @0x623840 _ZN11CBasePlayer9EquipSuitEb
void CBasePlayer::EquipSuit( bool bPlayEffects )
{
	// inlined CNetworkVarBase<bool,CPlayerLocalData::NetworkVar_m_bWearingSuit>::operator=<bool>() at line 8209
}

// game/server/player.cpp:8212 @0x625350 _ZN11CBasePlayer10RemoveSuitEv
void CBasePlayer::RemoveSuit()
{
	// inlined CNetworkVarBase<bool,CPlayerLocalData::NetworkVar_m_bWearingSuit>::operator=<bool>() at line 8214
}

// game/server/player.cpp:8222 @0x6217b0 _ZN11CBasePlayer14DoImpactEffectER10CGameTracei
void CBasePlayer::DoImpactEffect( trace_t &tr, int nDamageType )
{
}

// game/server/player.cpp:8236 @0x627640 _ZN11CBasePlayer14InputSetHealthER11inputdata_t
void CBasePlayer::InputSetHealth( inputdata_t &inputdata )
{
	int iNewHealth;  // line 8238
	int iDelta;  // line 8239
	{
		int armor;  // line 8247
		// inlined CNetworkVarBase<int,CBasePlayer::NetworkVar_m_ArmorValue>::operator=<int>() at line 8250
		// inlined CNetworkVarBase<int,CBasePlayer::NetworkVar_m_ArmorValue>::operator=<int>() at line 8248
	}
	// inlined CBaseEntity::GetHealth() at line 8239
	// inlined variant_t::Int() at line 8238
}

// game/server/player.cpp:8258 @0x62be90 _ZN11CBasePlayer21InputSetHUDVisibilityER11inputdata_t
void CBasePlayer::InputSetHUDVisibility( inputdata_t &inputdata )
{
	bool bEnable;  // line 8260
	// inlined CNetworkVarBase<int,CPlayerLocalData::NetworkVar_m_iHideHUD>::operator|=<int>() at line 8268
	// inlined variant_t::Bool() at line 8260
	// inlined CNetworkVarBase<int,CPlayerLocalData::NetworkVar_m_iHideHUD>::operator&=<int>() at line 8264
}

// game/server/player.cpp:8276 @0x62bf90 _ZN11CBasePlayer21InputSetFogControllerER11inputdata_t
void CBasePlayer::InputSetFogController( inputdata_t &inputdata )
{
	CFogController *pFogController;  // line 8279
	// inlined CNetworkHandleBase<CFogController,fogplayerparams_t::NetworkVar_m_hCtrl>::Set() at line 8291
	// inlined variant_t::String() at line 8286
	// inlined variant_t::FieldType() at line 8280
	// inlined CHandle<CBaseEntity>::Get() at line 8282
}

// game/server/player.cpp:8298 (declaration)
void InitFogController();

// game/server/player.cpp:8298 @0x62bb30 _ZN11CBasePlayer17InitFogControllerEv
void CBasePlayer::InitFogController()
{
	// inlined CFogSystem::GetMasterFogController() at line 8301
	// inlined CNetworkHandleBase<CFogController,fogplayerparams_t::NetworkVar_m_hCtrl>::operator=() at line 8301
}

// game/server/player.cpp:8307 (declaration)
void InitPostProcessController();

// game/server/player.cpp:8307 @0x639e40 _ZN11CBasePlayer25InitPostProcessControllerEv
void CBasePlayer::InitPostProcessController()
{
	// inlined CPostProcessSystem::GetMasterPostProcessController() at line 8310
	// inlined CNetworkHandleBase<CPostProcessController,CBasePlayer::NetworkVar_m_hPostProcessCtrl>::operator=() at line 8310
}

// game/server/player.cpp:8316 @0x63add0 _ZN11CBasePlayer29InitColorCorrectionControllerEv
void CBasePlayer::InitColorCorrectionController()
{
	// inlined CColorCorrectionSystem::GetMasterColorCorrection() at line 8318
	// inlined CNetworkHandleBase<CColorCorrection,CBasePlayer::NetworkVar_m_hColorCorrectionCtrl>::operator=() at line 8318
}

// game/server/player.cpp:8324 @0x639cc0 _ZN11CBasePlayer29InputSetPostProcessControllerER11inputdata_t
void CBasePlayer::InputSetPostProcessController( inputdata_t &inputdata )
{
	CPostProcessController *pController;  // line 8327
	// inlined CNetworkHandleBase<CPostProcessController,CBasePlayer::NetworkVar_m_hPostProcessCtrl>::Set() at line 8339
	// inlined variant_t::String() at line 8334
	// inlined variant_t::FieldType() at line 8328
	// inlined CHandle<CBaseEntity>::Get() at line 8330
}

// game/server/player.cpp:8346 @0x63aec0 _ZN11CBasePlayer33InputSetColorCorrectionControllerER11inputdata_t
void CBasePlayer::InputSetColorCorrectionController( inputdata_t &inputdata )
{
	CColorCorrection *pController;  // line 8349
	// inlined CNetworkHandleBase<CColorCorrection,CBasePlayer::NetworkVar_m_hColorCorrectionCtrl>::Set() at line 8361
	// inlined variant_t::String() at line 8356
	// inlined variant_t::FieldType() at line 8350
	// inlined CHandle<CBaseEntity>::Get() at line 8352
}

// game/server/player.cpp:8370 @0x62ca50 _ZN11CBasePlayer13SetViewEntityEP11CBaseEntity
void CBasePlayer::SetViewEntity( CBaseEntity *pEntity )
{
	// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hViewEntity>::operator=() at line 8372
	// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hViewEntity>::operator CBaseEntity*() at line 8374
	// inlined CBaseEntity::edict() at line 8380
}

// game/server/player.cpp:8390 @0x621b20 _ZN11CBasePlayer16HasAnyAmmoOfTypeEi
bool CBasePlayer::HasAnyAmmoOfType( int nAmmoIndex )
{
	CBaseCombatWeapon *pWeapon;  // line 8400
	{
		int i;  // line 8403
	}
}

// game/server/player.cpp:8434 (declaration)
void GetNetworkIDString();

// game/server/player.cpp:8434 @0x621ac0 _ZN11CBasePlayer18GetNetworkIDStringEv
const char *CBasePlayer::GetNetworkIDString()
{
	const char *networkIDString;  // line 8436
}

// game/server/player.cpp:8450 @0x621a90 _ZN11CBasePlayer13SetPlayerNameEPKc
void CBasePlayer::SetPlayerName( const char *name )
{
}

// game/server/player.cpp:8464 @0x620f50 _ZN11CBasePlayer20PrepareForFullUpdateEv
void CBasePlayer::PrepareForFullUpdate()
{
}

// game/server/player.cpp:8472 sizeof=0x4 (i386)
struct DisableAutokick
{
public:
	DisableAutokick( int );  // line 8474
	bool operator()( CBasePlayer * );  // line 8479
private:
	int m_userID; // +0x0  // line 8492
};

// game/server/player.cpp:8498 @0x627530 _ZL19mp_disable_autokickRK8CCommand
mp_disable_autokick( const CCommand &args )
{
	int userID;  // line 8512
	DisableAutokick disable;  // line 8513
	// inlined DisableAutokick::DisableAutokick() at line 8513
}

// game/server/player.cpp:8498
static ConCommand mp_disable_autokick_command;

// game/server/player.cpp:8520 (declaration)
void ToggleDuck();

// game/server/player.cpp:8520 @0x620f60 _ZN11CBasePlayer10ToggleDuckEv
void CBasePlayer::ToggleDuck()
{
}

// game/server/player.cpp:8529 @0x621a50 _ZN11CBasePlayer12GetStickDistEv
float CBasePlayer::GetStickDist()
{
	Vector2D controlStick;  // line 8531
	// inlined Vector2D::Length() at line 8536
}

// game/server/player.cpp:8542 @0x621980 _ZN11CBasePlayer15HandleAnimEventEP11animevent_t
void CBasePlayer::HandleAnimEvent( animevent_t *pEvent )
{
	int nEvent;  // line 8544
	// inlined animevent_t::Event() at line 8544
}

// game/server/player.cpp:8567 @0x620f80 _ZN11CPlayerInfo7GetNameEv
const char *CPlayerInfo::GetName()
{
}

// game/server/player.cpp:8573 @0x623ac0 _ZN11CPlayerInfo9GetUserIDEv
int CPlayerInfo::GetUserID()
{
}

// game/server/player.cpp:8579 @0x626c00 _ZN11CPlayerInfo18GetNetworkIDStringEv
const char *CPlayerInfo::GetNetworkIDString()
{
	// inlined CBasePlayer::GetNetworkIDString() at line 8582
}

// game/server/player.cpp:8585 @0x621540 _ZN11CPlayerInfo12GetTeamIndexEv
int CPlayerInfo::GetTeamIndex()
{
}

// game/server/player.cpp:8591 @0x620fb0 _ZN11CPlayerInfo10ChangeTeamEi
void CPlayerInfo::ChangeTeam( int iTeamNum )
{
}

// game/server/player.cpp:8597 @0x620fe0 _ZN11CPlayerInfo12GetFragCountEv
int CPlayerInfo::GetFragCount()
{
}

// game/server/player.cpp:8603 @0x621010 _ZN11CPlayerInfo13GetDeathCountEv
int CPlayerInfo::GetDeathCount()
{
}

// game/server/player.cpp:8609 @0x621040 _ZN11CPlayerInfo11IsConnectedEv
bool CPlayerInfo::IsConnected()
{
}

// game/server/player.cpp:8615 @0x621070 _ZN11CPlayerInfo13GetArmorValueEv
int CPlayerInfo::GetArmorValue()
{
}

// game/server/player.cpp:8621 @0x6243e0 _ZN11CPlayerInfo6IsHLTVEv
bool CPlayerInfo::IsHLTV()
{
}

// game/server/player.cpp:8635 @0x6210a0 _ZN11CPlayerInfo8IsPlayerEv
bool CPlayerInfo::IsPlayer()
{
}

// game/server/player.cpp:8641 @0x6210d0 _ZN11CPlayerInfo12IsFakeClientEv
bool CPlayerInfo::IsFakeClient()
{
}

// game/server/player.cpp:8647 @0x6249b0 _ZN11CPlayerInfo6IsDeadEv
bool CPlayerInfo::IsDead()
{
}

// game/server/player.cpp:8653 @0x621100 _ZN11CPlayerInfo12IsInAVehicleEv
bool CPlayerInfo::IsInAVehicle()
{
}

// game/server/player.cpp:8659 @0x6243b0 _ZN11CPlayerInfo10IsObserverEv
bool CPlayerInfo::IsObserver()
{
}

// game/server/player.cpp:8665 @0x628b50 _ZN11CPlayerInfo12GetAbsOriginEv
const Vector CPlayerInfo::GetAbsOrigin()
{
	// inlined CBaseEntity::GetAbsOrigin() at line 8668
}

// game/server/player.cpp:8671 @0x625490 _ZN11CPlayerInfo12GetAbsAnglesEv
const QAngle CPlayerInfo::GetAbsAngles()
{
	// inlined CBaseEntity::GetAbsAngles() at line 8674
}

// game/server/player.cpp:8677 @0x621130 _ZN11CPlayerInfo13GetPlayerMinsEv
const Vector CPlayerInfo::GetPlayerMins()
{
}

// game/server/player.cpp:8683 @0x624770 _ZN11CPlayerInfo13GetPlayerMaxsEv
const Vector CPlayerInfo::GetPlayerMaxs()
{
}

// game/server/player.cpp:8689 @0x621770 _ZN11CPlayerInfo13GetWeaponNameEv
const char *CPlayerInfo::GetWeaponName()
{
	CBaseCombatWeapon *weap;  // line 8692
}

// game/server/player.cpp:8700 @0x6242a0 _ZN11CPlayerInfo12GetModelNameEv
const char *CPlayerInfo::GetModelName()
{
	// inlined string_t::ToCStr() at line 8703
}

// game/server/player.cpp:8706 @0x623d70 _ZN11CPlayerInfo9GetHealthEv
const int CPlayerInfo::GetHealth()
{
}

// game/server/player.cpp:8712 @0x621180 _ZN11CPlayerInfo12GetMaxHealthEv
const int CPlayerInfo::GetMaxHealth()
{
}

// game/server/player.cpp:8722 @0x621710 _ZN11CPlayerInfo12SetAbsOriginER6Vector
void CPlayerInfo::SetAbsOrigin( Vector &vec )
{
}

// game/server/player.cpp:8731 @0x6216c0 _ZN11CPlayerInfo12SetAbsAnglesER6QAngle
void CPlayerInfo::SetAbsAngles( QAngle &ang )
{
}

// game/server/player.cpp:8740 @0x6211a0 _ZN11CPlayerInfo14RemoveAllItemsEb
void CPlayerInfo::RemoveAllItems( bool removeSuit )
{
}

// game/server/player.cpp:8749 @0x621640 _ZN11CPlayerInfo15SetActiveWeaponEPKc
void CPlayerInfo::SetActiveWeapon( const char *WeaponName )
{
	{
		CBaseCombatWeapon *weap;  // line 8754
	}
}

// game/server/player.cpp:8763 @0x6215f0 _ZN11CPlayerInfo14SetLocalOriginERK6Vector
void CPlayerInfo::SetLocalOrigin( const Vector &origin )
{
}

// game/server/player.cpp:8772 @0x624210 _ZN11CPlayerInfo14GetLocalOriginEv
const Vector CPlayerInfo::GetLocalOrigin()
{
	{
		Vector origin;  // line 8777
	}
	// inlined Vector::Vector() at line 8782
}

// game/server/player.cpp:8786 @0x6215a0 _ZN11CPlayerInfo14SetLocalAnglesERK6QAngle
void CPlayerInfo::SetLocalAngles( const QAngle &angles )
{
}

// game/server/player.cpp:8795 @0x623ec0 _ZN11CPlayerInfo14GetLocalAnglesEv
const QAngle CPlayerInfo::GetLocalAngles()
{
}

// game/server/player.cpp:8808 @0x621560 _ZN11CPlayerInfo22PostClientMessagesSentEv
void CPlayerInfo::PostClientMessagesSent()
{
}

// game/server/player.cpp:8817 @0x6211f0 _ZN11CPlayerInfo10IsEFlagSetEi
bool CPlayerInfo::IsEFlagSet( int nEFlagMask )
{
}

// game/server/player.cpp:8827 @0x634cd0 _ZN11CPlayerInfo13RunPlayerMoveEP7CBotCmd
void CPlayerInfo::RunPlayerMove( CBotCmd *ucmd )
{
	{
		CUserCmd cmd;  // line 8832
		float flOldFrametime;  // line 8849
		float flOldCurtime;  // line 8850
		// inlined CBasePlayer::SetLastUserCommand() at line 8858
		// inlined CBasePlayer::SetTimeBase() at line 8852
		// inlined QAngle::operator=() at line 8844
		// inlined CUserCmd::CUserCmd() at line 8832
	}
}

// game/server/player.cpp:8870 @0x626800 _ZN11CPlayerInfo18SetLastUserCommandERK7CBotCmd
void CPlayerInfo::SetLastUserCommand( const CBotCmd &ucmd )
{
	{
		CUserCmd cmd;  // line 8875
		// inlined CUserCmd::CUserCmd() at line 8875
		// inlined QAngle::operator=() at line 8887
		// inlined CBasePlayer::SetLastUserCommand() at line 8891
	}
}

// game/server/player.cpp:8896 @0x623da0 _ZN11CPlayerInfo18GetLastUserCommandEv
CBotCmd CPlayerInfo::GetLastUserCommand()
{
	CBotCmd &cmd;  // line 8898
	CBotCmd cmd;  // line 8898
	const CUserCmd *ucmd;  // line 8899
	// inlined CBotCmd::CBotCmd() at line 8898
	// inlined QAngle::operator=() at line 8913
}

// game/server/player.cpp:8921 @0x624db0 _ZN11CBasePlayer17Event_KilledOtherEP11CBaseEntityRK15CTakeDamageInfo
void CBasePlayer::Event_KilledOther( CBaseEntity *pVictim, const CTakeDamageInfo &info )
{
}

// game/server/player.cpp:8932 @0x626650 _ZN11CBasePlayer8SetModelEPKc
void CBasePlayer::SetModel( const char *szModelName )
{
	CMDLCacheCriticalSection cacheCriticalSection;  // line 8936
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 8937
	// inlined CBaseAnimating::LookupPoseParameter() at line 8937
	// inlined CMDLCacheCriticalSection::CMDLCacheCriticalSection() at line 8936
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 8937
}

// game/server/player.cpp:8940 @0x6265d0 _ZN11CBasePlayer12SetBodyPitchEf
void CBasePlayer::SetBodyPitch( float flPitch )
{
	// inlined CBaseAnimating::SetPoseParameter() at line 8944
}

// game/server/player.cpp:8948 @0x621230 _ZN11CBasePlayer14AdjustDrownDmgEi
void CBasePlayer::AdjustDrownDmg( int nAmount )
{
}

// game/server/player.cpp:8957 @0x6277a0 _ZN11CBasePlayer20SetSplitScreenPlayerEbPS_
void CBasePlayer::SetSplitScreenPlayer( bool bSplitScreenPlayer, CBasePlayer *pOwner )
{
	// inlined CHandle<CBasePlayer>::operator=() at line 8960
}

// game/server/player.cpp:8967 @0x624400 _ZN11CBasePlayer20IsSplitScreenPartnerEPS_
bool CBasePlayer::IsSplitScreenPartner( CBasePlayer *pPlayer )
{
	// inlined CBasePlayer::GetSplitScreenPlayerOwner() at line 8972
	// inlined CBasePlayer::GetSplitScreenPlayerOwner() at line 8975
}

// game/server/player.cpp:8981 (declaration)
void GetSplitScreenPlayerOwner();

// game/server/player.cpp:8981 @0x621260 _ZN11CBasePlayer25GetSplitScreenPlayerOwnerEv
CBasePlayer *CBasePlayer::GetSplitScreenPlayerOwner()
{
	// inlined CHandle<CBasePlayer>::operator CBasePlayer*() at line 8983
}

// game/server/player.cpp:8986 (declaration)
void IsSplitScreenPlayer();

// game/server/player.cpp:8986 @0x6212b0 _ZNK11CBasePlayer19IsSplitScreenPlayerEv
bool CBasePlayer::IsSplitScreenPlayer()
{
}

// game/server/player.cpp:8991 (declaration)
void IsSplitScreenUserOnEdict( edict_t *edict );

// game/server/player.cpp:8991 @0x626e90 _ZN11CBasePlayer24IsSplitScreenUserOnEdictEP7edict_t
bool CBasePlayer::IsSplitScreenUserOnEdict( edict_t *edict )
{
	CBaseEntity *pCont;  // line 8996
	// inlined CBasePlayer::GetSplitScreenPlayerOwner() at line 8997
	// inlined GetContainingEntity() at line 8996
}

// game/server/player.cpp:9000 @0x626dc0 _ZN11CBasePlayer24GetSplitScreenPlayerSlotEv
int CBasePlayer::GetSplitScreenPlayerSlot()
{
	CBasePlayer *pHost;  // line 9005
	{
		int i;  // line 9009
		{
			edict_t *ed;  // line 9011
			// inlined GetContainingEntity() at line 9012
			// inlined CBaseEntity::entindex() at line 9011
		}
	}
	// inlined CBasePlayer::GetSplitScreenPlayerOwner() at line 9005
}

// game/server/player.cpp:9022 @0x6249d0 _ZN11CBasePlayer21EnsureSplitScreenTeamEv
bool CBasePlayer::EnsureSplitScreenTeam()
{
	// inlined GameRules() at line 9026
	// inlined CBasePlayer::GetSplitScreenPlayerOwner() at line 9026
	// inlined CBasePlayer::GetSplitScreenPlayerOwner() at line 9031
	// inlined CBasePlayer::GetSplitScreenPlayerOwner() at line 9033
	// inlined CBasePlayer::GetSplitScreenPlayerOwner() at line 9034
}

// game/server/player.cpp:9042 sizeof=0x24 (i386)
struct CUserMessageThrottleMgr
{
public:
	CUserMessageThrottleMgr();  // line 9044
	void Start( const char **, int );  // line 9045
	void Finish();  // line 9046
	bool ShouldThrottle( CBasePlayer *, const char * );  // line 9047
private:
	CUtlDict<CBitVec<64>,int> m_SentMessage; // +0x0  // line 9052
};

// game/server/player.cpp:9042 (declaration)
~CUserMessageThrottleMgr();

// game/server/player.cpp:9044 @0x621380 _ZN23CUserMessageThrottleMgrC2Ev
CUserMessageThrottleMgr::CUserMessageThrottleMgr()
{
	// inlined CUtlDict<CBitVec<64>,int>::CUtlDict() at line 9057
}

// game/server/player.cpp:9044 @0x6213b0 _ZN23CUserMessageThrottleMgrC1Ev
CUserMessageThrottleMgr::CUserMessageThrottleMgr()
{
	// inlined CUtlDict<CBitVec<64>,int>::CUtlDict() at line 9057
}

// game/server/player.cpp:9045 @0x63b1c0 _ZN23CUserMessageThrottleMgr5StartEPPKci
void CUserMessageThrottleMgr::Start( const char **pchMessageNames, int nNumMessageNames )
{
	// inlined CUtlDict<CBitVec<64>,int>::Purge() at line 9063
	{
		int i;  // line 9064
		{
			int idx;  // line 9066
			// inlined CBitVecT<CFixedBitVecBase<64> >::ClearAll() at line 9067
			// inlined CUtlDict<CBitVec<64>,int>::Insert() at line 9066
		}
	}
}

// game/server/player.cpp:9046 @0x63b1b0 _ZN23CUserMessageThrottleMgr6FinishEv
void CUserMessageThrottleMgr::Finish()
{
}

// game/server/player.cpp:9047 @0x63b040 _ZN23CUserMessageThrottleMgr14ShouldThrottleEP11CBasePlayerPKc
bool CUserMessageThrottleMgr::ShouldThrottle( CBasePlayer *pPlayer, const char *pchMessageName )
{
	int idx;  // line 9079
	int nPlayerIndex;  // line 9086
	CBitVec<64> &data;  // line 9089
	// inlined CBitVecT<CFixedBitVecBase<64> >::Set() at line 9093
	// inlined CBitVecT<CFixedBitVecBase<64> >::IsBitSet() at line 9090
	// inlined CUtlDict<CBitVec<64>,int>::operator[]() at line 9089
	// inlined CBaseEntity::entindex() at line 9086
}

// game/server/player.cpp:9055
static CUserMessageThrottleMgr g_ThrottleMgr;

// game/server/player.cpp:9097 @0x63b2b0 _ZN11CBasePlayer26StartUserMessageThrottlingEPPKci
void CBasePlayer::StartUserMessageThrottling( const char **pchMessageNames, int nNumMessageNames )
{
}

// game/server/player.cpp:9102 @0x63b2d0 _ZN11CBasePlayer27FinishUserMessageThrottlingEv
void CBasePlayer::FinishUserMessageThrottling()
{
	// inlined CUserMessageThrottleMgr::Finish() at line 9104
}

// game/server/player.cpp:9107 @0x63b0f0 _ZN11CBasePlayer25ShouldThrottleUserMessageEPKc
bool CBasePlayer::ShouldThrottleUserMessage( const char *pchMessageName )
{
	// inlined CUserMessageThrottleMgr::ShouldThrottle() at line 9109
}

// game/server/player.cpp:9115 @0x6212c0 _ZNK11CBasePlayer15PlayerSolidMaskEb
unsigned int CBasePlayer::PlayerSolidMask( bool brushOnly )
{
}

// game/server/player.cpp:9125 @0x627810 _ZN11CBasePlayer14GetPlayerProxyEv
CLogicPlayerProxy *CBasePlayer::GetPlayerProxy()
{
	CLogicPlayerProxy *pProxy;  // line 9127
	// inlined CHandle<CBaseEntity>::operator=() at line 9137
	// inlined CHandle<CBaseEntity>::operator=() at line 9136
	// inlined CHandle<CBaseEntity>::Get() at line 9127
}

// game/server/player.cpp:9143 @0x6278e0 _ZN11CBasePlayer21FirePlayerProxyOutputEPKc9variant_tP11CBaseEntityS4_
void CBasePlayer::FirePlayerProxyOutput( const char *pszOutputName, variant_t &variant, CBaseEntity *pActivator, CBaseEntity *pCaller )
{
	// inlined variant_t::variant_t() at line 9148
}

// game/server/player.cpp:9152 @0x6383f0 _ZN11CBasePlayer14UpdateFXVolumeEv
void CBasePlayer::UpdateFXVolume()
{
	CFogController *pFogController;  // line 9154
	CPostProcessController *pPostProcessController;  // line 9155
	CColorCorrection *pColorCorrectionEnt;  // line 9156
	Vector eyePos;  // line 9158
	CBaseEntity *pViewEntity;  // line 9159
	CFogVolume *pFogVolume;  // line 9169
	// inlined CNetworkHandleBase<CColorCorrection,CBasePlayer::NetworkVar_m_hColorCorrectionCtrl>::Set() at line 9212
	// inlined CNetworkHandleBase<CPostProcessController,CBasePlayer::NetworkVar_m_hPostProcessCtrl>::Set() at line 9207
	// inlined CNetworkHandleBase<CFogController,fogplayerparams_t::NetworkVar_m_hCtrl>::Set() at line 9202
	// inlined CNetworkHandleBase<CFogController,fogplayerparams_t::NetworkVar_m_hCtrl>::Get() at line 9200
	// inlined CFogVolume::GetColorCorrectionController() at line 9174
	// inlined CFogVolume::GetPostProcessController() at line 9173
	// inlined CFogVolume::GetFogController() at line 9172
	// inlined Vector::operator=() at line 9166
	// inlined CBasePlayer::GetViewEntity() at line 9159
	// inlined CBaseEntity::GetAbsOrigin() at line 9162
	// inlined Vector::operator=() at line 9162
	// inlined CFogSystem::GetMasterFogController() at line 9195
	// inlined CPostProcessSystem::GetMasterPostProcessController() at line 9196
	// inlined CColorCorrectionSystem::GetMasterColorCorrection() at line 9197
	// inlined CFogSystem::GetMasterFogController() at line 9178
	// inlined CPostProcessSystem::GetMasterPostProcessController() at line 9183
}
