// DWARF declaration skeleton for game/server/gameinterface.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x77db0 _Z41__static_initialization_and_destruction_0ii
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
	// inlined CUtlLinkedList<CMapEntityRef,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CMapEntityRef, short unsigned int>, short unsigned int> >::CUtlLinkedList() at line 164
	// inlined CStringTableSaveRestoreOps::CStringTableSaveRestoreOps() at line 258
	// inlined CServerGameDLL::CServerGameDLL() at line 642
	// inlined CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> >::CUtlVector() at line 1004
	// inlined CServerGameEnts::CServerGameEnts() at line 2585
	// inlined CServerGameClients::CServerGameClients() at line 2861
	// inlined CServerDLLSharedAppSystems::CServerDLLSharedAppSystems() at line 3747
}

// game/shared/saverestore_stringtable.h:31 @0x4abbb0 _ZN26CStringTableSaveRestoreOps4SaveERK22SaveRestoreFieldInfo_tP5ISave
void CStringTableSaveRestoreOps::Save( const SaveRestoreFieldInfo_t &fieldInfo, ISave *pSave )
{
	int *pStringIndex;  // line 33
	const char *pString;  // line 34
	int nLen;  // line 35
}

// game/shared/saverestore_stringtable.h:40 @0x4abb00 _ZN26CStringTableSaveRestoreOps7RestoreERK22SaveRestoreFieldInfo_tP8IRestore
void CStringTableSaveRestoreOps::Restore( const SaveRestoreFieldInfo_t &fieldInfo, IRestore *pRestore )
{
	int *pStringIndex;  // line 42
	int nLen;  // line 43
	char *pTemp;  // line 44
}

// game/shared/saverestore_stringtable.h:49 @0x4ab9a0 _ZN26CStringTableSaveRestoreOps9MakeEmptyERK22SaveRestoreFieldInfo_t
void CStringTableSaveRestoreOps::MakeEmpty( const SaveRestoreFieldInfo_t &fieldInfo )
{
	int *pStringIndex;  // line 51
}

// game/shared/saverestore_stringtable.h:55 @0x4ab9b0 _ZN26CStringTableSaveRestoreOps7IsEmptyERK22SaveRestoreFieldInfo_t
bool CStringTableSaveRestoreOps::IsEmpty( const SaveRestoreFieldInfo_t &fieldInfo )
{
}

// game/server/gameinterface.cpp:144
static CSteamAPIContext s_SteamAPIContext;

// game/server/gameinterface.cpp:145
CSteamAPIContext *steamapicontext;

// game/server/gameinterface.cpp:149
static CSteamGameServerAPIContext s_SteamGameServerAPIContext;

// game/server/gameinterface.cpp:150
CSteamGameServerAPIContext *steamgameserverapicontext;

// game/server/gameinterface.cpp:154
IUploadGameStats *gamestatsuploader;

// game/server/gameinterface.cpp:159
CTimedEventMgr g_NetworkPropertyEventMgr;

// game/server/gameinterface.cpp:164
CUtlLinkedList<CMapEntityRef,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CMapEntityRef, short unsigned int>, short unsigned int> > g_MapEntityRefs;

// game/server/gameinterface.cpp:167
IVEngineServer *engine;

// game/server/gameinterface.cpp:168
IVoiceServer *g_pVoiceServer;

// game/server/gameinterface.cpp:170
IFileSystem *filesystem;

// game/server/gameinterface.cpp:174
INetworkStringTableContainer *networkstringtable;

// game/server/gameinterface.h:175 @0x4ab990 _ZN20CMapLoadEntityFilter18ShouldCreateEntityEPKc
bool CMapLoadEntityFilter::ShouldCreateEntity( const char *pClassname )
{
}

// game/server/gameinterface.cpp:175
IStaticPropMgrServer *staticpropmgr;

// game/server/gameinterface.cpp:176
IUniformRandomStream *random_valve;

// game/server/gameinterface.cpp:177
IEngineSound *enginesound;

// game/server/gameinterface.cpp:178
ISpatialPartition *partition;

// game/server/gameinterface.cpp:179
IVModelInfo *modelinfo;

// game/server/gameinterface.cpp:180
IEngineTrace *enginetrace;

// game/server/gameinterface.h:181 @0x4ac030 _ZN20CMapLoadEntityFilter16CreateNextEntityEPKc
CBaseEntity *CMapLoadEntityFilter::CreateNextEntity( const char *pClassname )
{
	CBaseEntity *pRet;  // line 183
	CMapEntityRef ref;  // line 185
	// inlined CUtlLinkedList<CMapEntityRef,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CMapEntityRef, short unsigned int>, short unsigned int> >::AddToTail() at line 196
	// inlined CBaseEntity::entindex() at line 191
}

// game/server/gameinterface.cpp:181
IFileLoggingListener *filelogginglistener;

// game/server/gameinterface.cpp:182
IGameEventManager2 *gameeventmanager;

// game/server/gameinterface.cpp:183
IDataCache *datacache;

// game/server/gameinterface.cpp:184
IVDebugOverlay *debugoverlay;

// game/server/gameinterface.cpp:185
ISoundEmitterSystemBase *soundemitterbase;

// game/server/gameinterface.cpp:186
IServerPluginHelpers *serverpluginhelpers;

// game/server/gameinterface.cpp:190
IServerEngineTools *serverenginetools;

// game/server/gameinterface.cpp:191
IServerFoundry *serverfoundry;

// game/server/gameinterface.cpp:192
ISceneFileCache *scenefilecache;

// game/server/gameinterface.cpp:196
IXboxSystem *xboxsystem;

// game/server/gameinterface.cpp:197
IScriptManager *scriptmanager;

// game/server/gameinterface.cpp:198
IBlackBox *blackboxrecorder;

// game/server/gameinterface.h:209 @0x4a32c0 _ZL48__CreateCServerGameTagsIServerGameTags_interfacev
void *__CreateCServerGameTagsIServerGameTags_interface()
{
}

// game/server/gameinterface.cpp:221
ConVar sv_massreport;

// game/server/gameinterface.cpp:222
ConVar sv_force_transmit_ents;

// game/server/gameinterface.cpp:224
ConVar sv_autosave;

// game/server/gameinterface.cpp:225
ConVar *sv_maxreplay;

// game/server/gameinterface.cpp:227
static ConVar *g_pcv_commentary;

// game/server/gameinterface.cpp:228
static ConVar *g_pcv_ThreadMode;

// game/server/gameinterface.cpp:234
static CSteam3Server s_Steam3Server;

// game/server/gameinterface.cpp:235 (declaration)
CSteam3Server &Steam3Server();

// game/server/gameinterface.cpp:235 @0x4a32d0 _Z12Steam3Serverv
CSteam3Server &Steam3Server()
{
}

// game/server/gameinterface.cpp:243 (declaration)
void CSteam3Server();

// game/server/gameinterface.cpp:243 @0x4a32e0 _ZN13CSteam3ServerC2Ev
CSteam3Server::CSteam3Server()
{
	// inlined CSteamGameServerAPIContext::CSteamGameServerAPIContext() at line 243
}

// game/server/gameinterface.cpp:243 @0x4a4d90 _ZN13CSteam3ServerC1Ev
CSteam3Server::CSteam3Server()
{
	// inlined CSteamGameServerAPIContext::CSteamGameServerAPIContext() at line 243
}

// game/server/gameinterface.cpp:250
INetworkStringTable *g_pStringTableParticleEffectNames;

// game/server/gameinterface.cpp:251
INetworkStringTable *g_pStringTableEffectDispatch;

// game/server/gameinterface.cpp:252
INetworkStringTable *g_pStringTableVguiScreen;

// game/server/gameinterface.cpp:253
INetworkStringTable *g_pStringTableMaterials;

// game/server/gameinterface.cpp:254
INetworkStringTable *g_pStringTableInfoPanel;

// game/server/gameinterface.cpp:255
INetworkStringTable *g_pStringTableClientSideChoreoScenes;

// game/server/gameinterface.cpp:256
INetworkStringTable *g_pStringTableExtraParticleFiles;

// game/server/gameinterface.cpp:258
CStringTableSaveRestoreOps g_VguiScreenStringOps;

// game/server/gameinterface.cpp:261
CGlobalVars *gpGlobals;

// game/server/gameinterface.cpp:262
static int g_nCommandClientIndex;

// game/server/gameinterface.cpp:265
static int g_nCurrentChapterIndex;

// game/server/gameinterface.cpp:267
static ConVar sv_showhitboxes;

// game/server/gameinterface.cpp:269
static ClientPutInServerOverrideFn g_pClientPutInServerOverride;

// game/server/gameinterface.cpp:272
CSharedEdictChangeInfo *g_pSharedChangeInfo;

// game/server/gameinterface.cpp:274 (declaration)
void GetChangeAccessor();

// game/server/gameinterface.cpp:274 @0x4a3310 _ZN10CBaseEdict17GetChangeAccessorEv
IChangeInfoAccessor *CBaseEdict::GetChangeAccessor()
{
}

// game/server/gameinterface.cpp:279 @0x4a3340 _ZNK10CBaseEdict17GetChangeAccessorEv
const IChangeInfoAccessor *CBaseEdict::GetChangeAccessor()
{
}

// game/server/gameinterface.cpp:286 @0x4a3370 _Z25ClientPutInServerOverridePFP11CBasePlayerP7edict_tPKcE
ClientPutInServerOverride( ClientPutInServerOverrideFn fn )
{
}

// game/server/gameinterface.cpp:291
ConVar ai_post_frame_navigation;

// public/tier1/refcount.h:294 @0x4abd00 _ZN12CRefCounted1I11IRefCounted20CRefCountServiceBaseILb1E6CRefMTEED0Ev
CRefCounted1<IRefCounted,CRefCountServiceBase<true, CRefMT> >::~CRefCounted1()
{
	// inlined CRefCountServiceBase<true,CRefMT>::~CRefCountServiceBase() at line 294
}

// public/tier1/refcount.h:294 @0x4abd40 _ZN12CRefCounted1I11IRefCounted20CRefCountServiceBaseILb1E6CRefMTEED1Ev
CRefCounted1<IRefCounted,CRefCountServiceBase<true, CRefMT> >::~CRefCounted1()
{
	// inlined CRefCountServiceBase<true,CRefMT>::~CRefCountServiceBase() at line 294
}

// public/tier1/refcount.h:295 @0x4abc90 _ZN12CRefCounted1I11IRefCounted20CRefCountServiceBaseILb1E6CRefMTEE6AddRefEv
int CRefCounted1<IRefCounted,CRefCountServiceBase<true, CRefMT> >::AddRef()
{
	// inlined CRefCountServiceBase<true,CRefMT>::DoAddRef() at line 295
}

// game/server/gameinterface.cpp:295
static bool g_bHeadTrackingEnabled;

// public/tier1/refcount.h:296 @0x4abc10 _ZN12CRefCounted1I11IRefCounted20CRefCountServiceBaseILb1E6CRefMTEE7ReleaseEv
int CRefCounted1<IRefCounted,CRefCountServiceBase<true, CRefMT> >::Release()
{
	// inlined CRefCountServiceBase<true,CRefMT>::DoRelease() at line 296
}

// game/server/gameinterface.cpp:297 @0x4a3390 _Z21IsHeadTrackingEnabledv
bool IsHeadTrackingEnabled()
{
}

// game/server/gameinterface.cpp:310 (declaration)
int UTIL_GetCommandClientIndex();

// game/server/gameinterface.cpp:310 @0x4a33a0 _Z26UTIL_GetCommandClientIndexv
int UTIL_GetCommandClientIndex()
{
}

// game/server/gameinterface.cpp:323 @0x4a5820 _Z21UTIL_GetCommandClientv
CBasePlayer *UTIL_GetCommandClient()
{
	int idx;  // line 325
	// inlined UTIL_GetCommandClientIndex() at line 325
}

// game/server/gameinterface.cpp:415 @0x4a5a70 _Z20DrawAllDebugOverlaysv
DrawAllDebugOverlays()
{
	{
		CBasePlayer *pPlayer;  // line 422
		{
			CBaseEntity *pEntity;  // line 427
		}
	}
	{
		CBasePlayer *pPlayer;  // line 448
		{
			CAI_Node *pAINode;  // line 480
			{
				Vector vecPos;  // line 492
				{
					CBaseEntity *pEnt;  // line 497
					// inlined Vector::Vector() at line 500
					// inlined Vector::operator+() at line 500
					// inlined Vector::operator VectorByValue&() at line 500
				}
				// inlined CAI_Node::GetHint() at line 495
				// inlined Vector::operator VectorByValue&() at line 493
				// inlined Vector::Vector() at line 493
				// inlined Vector::operator VectorByValue&() at line 493
				// inlined Vector::Vector() at line 493
			}
		}
		{
			CAI_Link *pAILink;  // line 453
			{
				Vector startPos;  // line 457
				Vector endPos;  // line 458
				Vector linkDir;  // line 459
				float linkLen;  // line 460
				// inlined Vector::operator VectorByValue&() at line 470
				// inlined Vector::Vector() at line 470
				// inlined Vector::operator VectorByValue&() at line 470
				// inlined Vector::Vector() at line 470
				// inlined Vector::operator-() at line 459
				// inlined CAI_Network::GetNode() at line 458
				// inlined CAI_Network::GetNode() at line 457
				// inlined Vector::Vector() at line 474
				// inlined Vector::operator VectorByValue&() at line 474
				// inlined Vector::Vector() at line 474
				// inlined Vector::operator VectorByValue&() at line 474
				// inlined Vector::Vector() at line 465
				// inlined Vector::operator VectorByValue&() at line 465
				// inlined Vector::Vector() at line 465
				// inlined Vector::operator VectorByValue&() at line 465
			}
		}
	}
	{
		const CEntInfo *pInfo;  // line 530
		{
			CBaseEntity *ent;  // line 534
			{
				CMDLCacheCriticalSection cacheCriticalSection;  // line 538
				// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 539
				// inlined CMDLCacheCriticalSection::CMDLCacheCriticalSection() at line 538
				// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 539
			}
		}
		// inlined CBaseEntityList::FirstEntInfo() at line 530
	}
	{
		const CEntInfo *pInfo;  // line 547
		// inlined CBaseEntityList::FirstEntInfo() at line 547
		{
			CBaseEntity *ent;  // line 551
			char tempstr[512];  // line 555
			// inlined string_t::ToCStr() at line 559
			// inlined CBaseEntity::VPhysicsGetObject() at line 552
			// inlined CBaseEntity::VPhysicsGetObject() at line 559
			// inlined CBaseEntity::VPhysicsGetObject() at line 559
		}
	}
}

// public/vstdlib/jobthread.h:449 @0x4ac240 _ZN4CJobD1Ev
CJob::~CJob()
{
	// inlined CThreadEvent::~CThreadEvent() at line 449
	// inlined CRefCounted1<IRefCounted,CRefCountServiceBase<true, CRefMT> >::~CRefCounted1() at line 449
	// inlined CRefCounted1<IRefCounted,CRefCountServiceBase<true, CRefMT> >::~CRefCounted1() at line 449
}

// public/vstdlib/jobthread.h:449 @0x4ac2d0 _ZN4CJobD0Ev
CJob::~CJob()
{
	// inlined CThreadEvent::~CThreadEvent() at line 449
	// inlined CRefCounted1<IRefCounted,CRefCountServiceBase<true, CRefMT> >::~CRefCounted1() at line 449
	// inlined CRefCounted1<IRefCounted,CRefCountServiceBase<true, CRefMT> >::~CRefCounted1() at line 449
}

// public/tier1/utllinkedlist.h:461 @0x4abea0 _ZN14CUtlLinkedListI13CMapEntityReftLb0Et10CUtlMemoryI19UtlLinkedListElem_tIS0_tEtEE13AllocInternalEb
short unsigned int CUtlLinkedList<CMapEntityRef,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CMapEntityRef, short unsigned int>, short unsigned int> >::AllocInternal( bool multilist )
{
	short unsigned int elem;  // line 467
	{
		CUtlMemory<UtlLinkedListElem_t<CMapEntityRef, short unsigned int>,short unsigned int>::Iterator_t it;  // line 472
		// inlined CUtlMemory<UtlLinkedListElem_t<CMapEntityRef, short unsigned int>,short unsigned int>::First() at line 480
		{
		}
		// inlined CUtlMemory<UtlLinkedListElem_t<CMapEntityRef, short unsigned int>,short unsigned int>::IsValidIterator() at line 483
		// inlined CUtlMemory<UtlLinkedListElem_t<CMapEntityRef, short unsigned int>,short unsigned int>::Next() at line 480
		// inlined CUtlMemory<UtlLinkedListElem_t<CMapEntityRef, short unsigned int>,short unsigned int>::IsValidIterator() at line 480
		// inlined CUtlLinkedList<CMapEntityRef,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CMapEntityRef, short unsigned int>, short unsigned int> >::ResetDbgInfo() at line 478
		// inlined CUtlMemory<UtlLinkedListElem_t<CMapEntityRef, short unsigned int>,short unsigned int>::First() at line 472
		// inlined CUtlMemory<UtlLinkedListElem_t<CMapEntityRef, short unsigned int>,short unsigned int>::IsValidIterator() at line 472
		// inlined CUtlMemory<UtlLinkedListElem_t<CMapEntityRef, short unsigned int>,short unsigned int>::Next() at line 472
		// inlined CUtlMemory<UtlLinkedListElem_t<CMapEntityRef, short unsigned int>,short unsigned int>::IsValidIterator() at line 474
		{
		}
	}
	// inlined CUtlLinkedList<CMapEntityRef,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CMapEntityRef, short unsigned int>, short unsigned int> >::InternalElement() at line 504
	int __executeCount;  // line 485
	int __executeCount;  // line 493
}

// public/vstdlib/jobthread.h:514 @0x4ab9d0 _ZN4CJob8DescribeEv
const char *CJob::Describe()
{
}

// public/vstdlib/jobthread.h:536 @0x4ab9e0 _ZN4CJob7DoAbortEb
JobStatus_t CJob::DoAbort( bool bDiscard )
{
}

// public/vstdlib/jobthread.h:537 @0x4ab9f0 _ZN4CJob9DoCleanupEv
void CJob::DoCleanup()
{
}

// public/vstdlib/jobthread.h:543 @0x4ac360 _ZN11CFunctorJobD1Ev
CFunctorJob::~CFunctorJob()
{
	// inlined CRefPtr<CFunctor>::~CRefPtr() at line 543
	// inlined CJob::~CJob() at line 543
	// inlined CJob::~CJob() at line 543
}

// public/vstdlib/jobthread.h:543 @0x4ac440 _ZN11CFunctorJobD0Ev
CFunctorJob::~CFunctorJob()
{
	// inlined CRefPtr<CFunctor>::~CRefPtr() at line 543
	// inlined CJob::~CJob() at line 543
	// inlined CJob::~CJob() at line 543
}

// public/tier1/functors.h:544 @0x4aba60 _ZN9CFunctor1IPFvbEb12CRefCounted1I8CFunctor20CRefCountServiceBaseILb1E6CRefMTEEEclEv
void CFunctor1<void (*)(bool),bool,CRefCounted1<CFunctor, CRefCountServiceBase<true, CRefMT> > >::operator()()
{
}

// public/tier1/functors.h:544 @0x4abcc0 _ZN9CFunctor1IPFvbEb12CRefCounted1I8CFunctor20CRefCountServiceBaseILb1E6CRefMTEEED1Ev
CFunctor1<void (*)(bool),bool,CRefCounted1<CFunctor, CRefCountServiceBase<true, CRefMT> > >::~CFunctor1()
{
	// inlined CRefCounted1<CFunctor,CRefCountServiceBase<true, CRefMT> >::~CRefCounted1() at line 544
}

// public/tier1/functors.h:544 @0x4abd80 _ZN9CFunctor1IPFvbEb12CRefCounted1I8CFunctor20CRefCountServiceBaseILb1E6CRefMTEEED0Ev
CFunctor1<void (*)(bool),bool,CRefCounted1<CFunctor, CRefCountServiceBase<true, CRefMT> > >::~CFunctor1()
{
	// inlined CRefCounted1<CFunctor,CRefCountServiceBase<true, CRefMT> >::~CRefCounted1() at line 544
}

// public/vstdlib/jobthread.h:558 @0x4aba10 _ZN11CFunctorJob9DoExecuteEv
JobStatus_t CFunctorJob::DoExecute()
{
	// inlined CBaseAutoPtr<CFunctor>::operator*() at line 560
}

// public/vstdlib/jobthread.h:564 @0x4aba00 _ZN11CFunctorJob8DescribeEv
const char *CFunctorJob::Describe()
{
}

// game/server/gameinterface.cpp:569
static ConVar sv_threaded_init;

// game/server/gameinterface.cpp:571 @0x4a4ac0 _ZL15InitGameSystemsPFPvPKcPiE
bool InitGameSystems( CreateInterfaceFn appSystemFactory )
{
}

// public/eiface.h:596 @0x4abac0 _ZN15IServerGameEntsD0Ev
IServerGameEnts::~IServerGameEnts()
{
}

// public/eiface.h:596 @0x4abae0 _ZN15IServerGameEntsD1Ev
IServerGameEnts::~IServerGameEnts()
{
}

// game/server/gameinterface.cpp:642
CServerGameDLL g_ServerGameDLL;

// game/server/gameinterface.cpp:643 @0x4a33c0 _ZL46__CreateCServerGameDLLIServerGameDLL_interfacev
void *__CreateCServerGameDLLIServerGameDLL_interface()
{
}

// game/server/gameinterface.cpp:643
static InterfaceReg __g_CreateCServerGameDLLIServerGameDLL_reg;

// game/server/gameinterface.cpp:647 @0x4a9fa0 _ZN14CServerGameDLL7DLLInitEPFPvPKcPiES5_S5_P11CGlobalVars
bool CServerGameDLL::DLLInit( CreateInterfaceFn appSystemFactory, CreateInterfaceFn physicsFactory, CreateInterfaceFn fileSystemFactory, CGlobalVars *pGlobals )
{
	factorylist_t factories;  // line 772
	bool bPrecacheParticles;  // line 788
	bool bInitSuccess;  // line 819
	{
		CFunctorJob *pGameJob;  // line 822
		float flLastUpdateTime;  // line 828
		{
			float flTime;  // line 832
		}
		// inlined CreateFunctor<void, bool, bool>() at line 822
		// inlined CFunctorJob::CFunctorJob() at line 822
		// inlined CJob::IsFinished() at line 830
	}
	{
		IMatchExtensions *pIMatchExtensions;  // line 747
	}
	// inlined CSteamGameServerAPIContext::Init() at line 669
	// inlined CSteamAPIContext::Init() at line 666
}

// public/tier1/utlvector.h:655 @0x77c90 _ZN10CUtlVectorI15AppSystemInfo_t10CUtlMemoryIS0_iEE10GrowVectorEi
void CUtlVector<AppSystemInfo_t,CUtlMemory<AppSystemInfo_t, int> >::GrowVector( int num )
{
	// inlined CUtlMemory<AppSystemInfo_t,int>::NumAllocated() at line 657
	// inlined CUtlMemory<AppSystemInfo_t,int>::Grow() at line 660
	// inlined CUtlVector<AppSystemInfo_t,CUtlMemory<AppSystemInfo_t, int> >::ResetDbgInfo() at line 664
}

// public/tier1/utlmemory.h:707 @0x4abdb0 _ZN10CUtlMemoryI19UtlLinkedListElem_tI13CMapEntityReftEtE4GrowEi
void CUtlMemory<UtlLinkedListElem_t<CMapEntityRef, short unsigned int>,short unsigned int>::Grow( int num )
{
	int nAllocationRequested;  // line 720
	int nNewAllocationCount;  // line 724
	// inlined UtlMemory_CalcNewAllocationCount() at line 724
	// inlined CUtlMemory<UtlLinkedListElem_t<CMapEntityRef, short unsigned int>,short unsigned int>::IsExternallyAllocated() at line 711
	// inlined MemAlloc_Alloc() at line 761
}

// game/server/gameinterface.cpp:862 @0x4a4ab0 _ZN14CServerGameDLL8PostInitEv
void CServerGameDLL::PostInit()
{
}

// game/server/gameinterface.cpp:877 @0x4a33d0 _ZN14CServerGameDLL13PostToolsInitEv
void CServerGameDLL::PostToolsInit()
{
}

// game/server/gameinterface.cpp:885 @0x4a48b0 _ZN14CServerGameDLL11DLLShutdownEv
void CServerGameDLL::DLLShutdown()
{
	char *pFilename;  // line 906
	// inlined CSteamGameServerAPIContext::Clear() at line 936
	// inlined CSteamAPIContext::Clear() at line 935
}

// game/server/gameinterface.cpp:951 @0x4a44f0 _ZNK14CServerGameDLL15GetTickIntervalEv
float CServerGameDLL::GetTickInterval()
{
	float tickinterval;  // line 953
	{
		float tickrate;  // line 965
	}
}

// game/server/gameinterface.cpp:975 @0x4a4810 _ZN14CServerGameDLL8GameInitEv
bool CServerGameDLL::GameInit()
{
	IGameEvent *event;  // line 982
}

// game/server/gameinterface.cpp:993 @0x4a4800 _ZN14CServerGameDLL12GameShutdownEv
void CServerGameDLL::GameShutdown()
{
}

// game/server/gameinterface.cpp:998
static bool g_OneWayTransition;

// game/server/gameinterface.cpp:999 @0x4a3410 _Z24Game_SetOneWayTransitionv
Game_SetOneWayTransition()
{
}

// game/server/gameinterface.cpp:1004
static CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> > g_RestoredEntities;

// game/server/gameinterface.cpp:1006
static bool g_InRestore;

// game/server/gameinterface.cpp:1008 @0x4a69f0 _Z17AddRestoredEntityP11CBaseEntity
AddRestoredEntity( CBaseEntity *pEntity )
{
	// inlined CHandle<CBaseEntity>::CHandle() at line 1014
	// inlined CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> >::AddToTail() at line 1014
}

// game/server/gameinterface.cpp:1017 @0x4a6610 _Z18EndRestoreEntitiesv
EndRestoreEntities()
{
	// inlined CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> >::Purge() at line 1036
	{
		int i;  // line 1026
		{
			CBaseEntity *pEntity;  // line 1028
			// inlined CHandle<CBaseEntity>::Get() at line 1028
			{
				CMDLCacheCriticalSection cacheCriticalSection;  // line 1031
				// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 1032
				// inlined CMDLCacheCriticalSection::CMDLCacheCriticalSection() at line 1031
				// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 1032
			}
		}
		// inlined CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> >::Count() at line 1026
	}
}

// game/server/gameinterface.cpp:1048 (declaration)
void BeginRestoreEntities();

// game/server/gameinterface.cpp:1048 @0x4a6560 _Z20BeginRestoreEntitiesv
BeginRestoreEntities()
{
	// inlined CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> >::Purge() at line 1055
}

// game/server/gameinterface.cpp:1069 @0x4a3430 _ZN14CServerGameDLL11IsRestoringEv
bool CServerGameDLL::IsRestoring()
{
}

// game/server/gameinterface.cpp:1074 @0x4a3450 _ZN14CServerGameDLL19SupportsSaveRestoreEv
bool CServerGameDLL::SupportsSaveRestore()
{
}

// game/server/gameinterface.cpp:1084 @0x4a6ff0 _ZN14CServerGameDLL9LevelInitEPKcS1_S1_S1_bb
bool CServerGameDLL::LevelInit( const char *pMapName, const char *pMapEntities, const char *pOldLevel, const char *pLandmarkName, bool loadGame, bool background )
{
	CVProfScope VProf_;  // line 1086
	// inlined CVProfScope::CVProfScope() at line 1086
	// inlined UpdateChapterRestrictions() at line 1088
	// inlined BeginRestoreEntities() at line 1103
	{
		CBaseEntity *pAutosave;  // line 1134
	}
	{
		CMapLoadEntityFilter filter;  // line 1155
		// inlined CUtlLinkedList<CMapEntityRef,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CMapEntityRef, short unsigned int>, short unsigned int> >::Purge() at line 1154
		// inlined CMapLoadEntityFilter::CMapLoadEntityFilter() at line 1155
	}
	// inlined CServerGameDLL::LoadMessageOfTheDay() at line 1172
	// inlined CAI_TimedSemaphore::Release() at line 1178
	// inlined CAI_TimedSemaphore::Release() at line 1179
	// inlined GameRules() at line 1187
	// inlined CVProfScope::~CVProfScope() at line 1189
	// inlined CVProfScope::~CVProfScope() at line 1189
}

// game/server/gameinterface.cpp:1216 @0x4a5970 _ZN14CServerGameDLL14ServerActivateEP7edict_tii
void CServerGameDLL::ServerActivate( edict_t *pEdictList, int edictCount, int clientMax )
{
	{
		CBaseEntity *pClass;  // line 1227
		{
			CMDLCacheCriticalSection cacheCriticalSection;  // line 1231
			// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 1237
			// inlined CMDLCacheCriticalSection::CMDLCacheCriticalSection() at line 1231
			// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 1237
		}
		// inlined CGlobalEntityList::FirstEnt() at line 1227
	}
}

// game/server/gameinterface.cpp:1264 @0x4a9e70 _ZN14CServerGameDLL27GameServerSteamAPIActivatedEv
void CServerGameDLL::GameServerSteamAPIActivated()
{
	// inlined CSteamGameServerAPIContext::Init() at line 1267
}

// game/server/gameinterface.cpp:1275
ConVar trace_report;

// game/server/gameinterface.cpp:1277 @0x4a4f30 _ZN14CServerGameDLL9GameFrameEb
void CServerGameDLL::GameFrame( bool simulating )
{
	CVProfScope VProf_;  // line 1279
	float oldframetime;  // line 1307
	// inlined CVProfScope::CVProfScope() at line 1279
	// inlined CSteam3Server::CheckInitialized() at line 1288
	// inlined GameRules() at line 1290
	{
		CVProfScope VProf_;  // line 1330
		// inlined CVProfScope::CVProfScope() at line 1330
		// inlined CVProfScope::~CVProfScope() at line 1331
	}
	{
		CVProfScope VProf_;  // line 1334
		// inlined CVProfScope::CVProfScope() at line 1334
		// inlined CVProfScope::~CVProfScope() at line 1335
	}
	{
		CVProfScope VProf_;  // line 1339
		// inlined CVProfScope::CVProfScope() at line 1339
		// inlined CVProfScope::~CVProfScope() at line 1340
	}
	{
		CVProfScope VProf_;  // line 1360
		// inlined CVProfScope::~CVProfScope() at line 1361
		// inlined CVProfScope::CVProfScope() at line 1360
		// inlined CVProfScope::~CVProfScope() at line 1361
	}
	{
		int total;  // line 1366
		int totals[3];  // line 1366
		{
			int i;  // line 1367
		}
	}
	// inlined CVProfScope::~CVProfScope() at line 1385
	// inlined CVProfScope::~CVProfScope() at line 1385
	// inlined CVProfScope::~CVProfScope() at line 1385
}

// game/server/gameinterface.cpp:1392 @0x4a8580 _ZN14CServerGameDLL15PreClientUpdateEb
void CServerGameDLL::PreClientUpdate( bool simulating )
{
	CBaseAnimating *anim;  // line 1434
	// inlined CBaseEntity::Instance() at line 1434
	// inlined INDEXENT() at line 1434
	// inlined ConVar::GetInt() at line 1410
	{
		CBaseEntity *pEntity;  // line 1416
		{
			CBaseAnimating *anim;  // line 1424
			// inlined ConVar::GetString() at line 1420
		}
	}
}

// game/server/gameinterface.cpp:1441 @0x4a4cc0 _ZN14CServerGameDLL5ThinkEb
void CServerGameDLL::Think( bool finalTick )
{
	{
		CBasePlayer *pPlayer;  // line 1446
		// inlined CBasePlayer::GetDeathTime() at line 1448
	}
}

// game/server/gameinterface.cpp:1464 @0x4a3460 _ZN14CServerGameDLL24OnQueryCvarValueFinishedEiP7edict_t21EQueryCvarValueStatusPKcS4_
void CServerGameDLL::OnQueryCvarValueFinished( QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, $_155 eStatus, const char *pCvarName, const char *pCvarValue )
{
}

// game/server/gameinterface.cpp:1469 @0x4a58d0 _ZN14CServerGameDLL13LevelShutdownEv
void CServerGameDLL::LevelShutdown()
{
	CMDLCacheCriticalSection cacheCriticalSection;  // line 1471
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 1493
	// inlined CMDLCacheCriticalSection::CMDLCacheCriticalSection() at line 1471
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 1493
}

// game/server/gameinterface.cpp:1501 @0x4a3470 _ZN14CServerGameDLL19GetAllServerClassesEv
ServerClass *CServerGameDLL::GetAllServerClasses()
{
}

// game/server/gameinterface.cpp:1507 @0x4a47f0 _ZN14CServerGameDLL18GetGameDescriptionEv
const char *CServerGameDLL::GetGameDescription()
{
}

// game/server/gameinterface.cpp:1512 @0x4a67a0 _ZN14CServerGameDLL25CreateNetworkStringTablesEv
void CServerGameDLL::CreateNetworkStringTables()
{
	// inlined PrecacheMaterial() at line 1533
	// inlined PrecacheParticleSystem() at line 1536
	// inlined PrecacheEffect() at line 1539
	// inlined CStringTableSaveRestoreOps::Init() at line 1545
}

// game/server/gameinterface.cpp:1548 @0x4a47e0 _ZN14CServerGameDLL8SaveInitEi
CSaveRestoreData *CServerGameDLL::SaveInit( int size )
{
}

// game/server/gameinterface.cpp:1561 @0x4ab310 _ZN14CServerGameDLL15SaveWriteFieldsEP16CSaveRestoreDataPKcPvP9datamap_tP17typedescription_ti
void CServerGameDLL::SaveWriteFields( CSaveRestoreData *pSaveData, const char *pname, void *pBaseData, datamap_t *pMap, typedescription_t *pFields, int fieldCount )
{
	CSave saveHelper;  // line 1563
	// inlined CSave::~CSave() at line 1564
	// inlined CSave::~CSave() at line 1564
}

// game/server/gameinterface.cpp:1579 @0x4ab4f0 _ZN14CServerGameDLL14SaveReadFieldsEP16CSaveRestoreDataPKcPvP9datamap_tP17typedescription_ti
void CServerGameDLL::SaveReadFields( CSaveRestoreData *pSaveData, const char *pname, void *pBaseData, datamap_t *pMap, typedescription_t *pFields, int fieldCount )
{
	CRestore restoreHelper;  // line 1581
	// inlined CRestore::~CRestore() at line 1582
	// inlined CRestore::~CRestore() at line 1582
}

// game/server/gameinterface.cpp:1587 @0x4a47d0 _ZN14CServerGameDLL15SaveGlobalStateEP16CSaveRestoreData
void CServerGameDLL::SaveGlobalState( CSaveRestoreData *s )
{
}

// game/server/gameinterface.cpp:1592 @0x4a47c0 _ZN14CServerGameDLL18RestoreGlobalStateEP16CSaveRestoreData
void CServerGameDLL::RestoreGlobalState( CSaveRestoreData *s )
{
}

// game/server/gameinterface.cpp:1597 @0x4ab410 _ZN14CServerGameDLL4SaveEP16CSaveRestoreData
void CServerGameDLL::Save( CSaveRestoreData *s )
{
	CSave saveHelper;  // line 1599
	// inlined CSave::~CSave() at line 1600
	// inlined CSave::~CSave() at line 1600
}

// game/server/gameinterface.cpp:1603 @0x4ab840 _ZN14CServerGameDLL7RestoreEP16CSaveRestoreDatab
void CServerGameDLL::Restore( CSaveRestoreData *s, bool b )
{
	CRestore restore;  // line 1608
	// inlined CRestore::~CRestore() at line 1613
	// inlined CRestore::~CRestore() at line 1613
}

// game/server/gameinterface.cpp:1624 @0x4a4740 _ZN14CServerGameDLL18GetUserMessageInfoEiPciRi
bool CServerGameDLL::GetUserMessageInfo( int msg_type, char *name, int maxnamelength, int &size )
{
}

// game/server/gameinterface.cpp:1634 @0x4a3490 _ZN14CServerGameDLL22GetStandardSendProxiesEv
CStandardSendProxies *CServerGameDLL::GetStandardSendProxies()
{
}

// game/server/gameinterface.cpp:1639 @0x4ab5f0 _ZN14CServerGameDLL26CreateEntityTransitionListEP16CSaveRestoreDatai
int CServerGameDLL::CreateEntityTransitionList( CSaveRestoreData *s, int a )
{
	CRestore restoreHelper;  // line 1641
	int base;  // line 1643
	int movedCount;  // line 1645
	// inlined CRestore::~CRestore() at line 1655
	// inlined CRestore::~CRestore() at line 1655
}

// game/server/gameinterface.cpp:1658 @0x4a34a0 _ZN14CServerGameDLL7PreSaveEP16CSaveRestoreData
void CServerGameDLL::PreSave( CSaveRestoreData *s )
{
}

// game/server/gameinterface.cpp:1667 sizeof=0x8 (i386)
struct $_442
{
public:
	char *pBSPName; // +0x0  // line 1668
	char *pTitleName; // +0x4  // line 1669
};

// game/server/gameinterface.cpp:1670
typedef $_442 TITLECOMMENT;

// game/server/gameinterface.cpp:1673
static const TITLECOMMENT gTitleComments[23];

// game/server/gameinterface.cpp:1781 @0x4a4580 _ZN14CServerGameDLL14GetSaveCommentEPciffb
void CServerGameDLL::GetSaveComment( char *text, int maxlength, float flMinutes, float flSeconds, bool bNoTime )
{
	char comment[64];  // line 1783
	const char *pName;  // line 1784
	int i;  // line 1785
	const char *mapname;  // line 1787
	{
		int minutes;  // line 1827
		int seconds;  // line 1828
		int minutesAdd;  // line 1837
	}
	// inlined string_t::ToCStr() at line 1787
	{
		int j;  // line 1797
	}
}

// game/server/gameinterface.cpp:1845 @0x4ab220 _ZN14CServerGameDLL16WriteSaveHeadersEP16CSaveRestoreData
void CServerGameDLL::WriteSaveHeaders( CSaveRestoreData *s )
{
	CSave saveHelper;  // line 1847
	// inlined CSave::~CSave() at line 1849
	// inlined CSave::~CSave() at line 1849
}

// game/server/gameinterface.cpp:1852 @0x4ab750 _ZN14CServerGameDLL18ReadRestoreHeadersEP16CSaveRestoreData
void CServerGameDLL::ReadRestoreHeaders( CSaveRestoreData *s )
{
	CRestore restoreHelper;  // line 1854
	// inlined CRestore::~CRestore() at line 1856
	// inlined CRestore::~CRestore() at line 1856
}

// game/server/gameinterface.cpp:1859 @0x4a34c0 _ZN14CServerGameDLL17PreSaveGameLoadedEPKcb
void CServerGameDLL::PreSaveGameLoaded( const char *pSaveName, bool bInGame )
{
}

// game/server/gameinterface.cpp:1870 @0x4a4bd0 _ZN14CServerGameDLL16ShouldHideServerEv
bool CServerGameDLL::ShouldHideServer()
{
}

// game/server/gameinterface.cpp:1884 @0x4a9a70 _ZN14CServerGameDLL18InvalidateMdlCacheEv
void CServerGameDLL::InvalidateMdlCache()
{
	CBaseAnimating *pAnimating;  // line 1886
	{
		CBaseEntity *pEntity;  // line 1887
		// inlined CBaseAnimating::InvalidateMdlCache() at line 1892
		// inlined CGlobalEntityList::FirstEnt() at line 1887
	}
}

// game/server/gameinterface.cpp:1898 (declaration)
KeyValues *FindLaunchOptionByValue( KeyValues *pLaunchOptions, const char *szLaunchOption );

// game/server/gameinterface.cpp:1916 @0x4aaae0 _ZN14CServerGameDLL17ApplyGameSettingsEP9KeyValues
void CServerGameDLL::ApplyGameSettings( KeyValues *pKV )
{
	bool bAlreadyLoadingMap;  // line 1931
	const char *szBspName;  // line 1932
	{
		const char *szMapCommand;  // line 2225
	}
	{
		int numSlots;  // line 2181
		{
			ConVar sv_portal_players;  // line 2183
			// inlined ConVarRef::SetValue() at line 2191
			// inlined ConVarRef::SetValue() at line 2195
		}
	}
	{
		const char *szNewMap;  // line 1938
		KeyValues *pLaunchOptions;  // line 1942
		{
			int numSlots;  // line 2053
			// inlined FindLaunchOptionByValue() at line 2054
			// inlined FindLaunchOptionByValue() at line 2056
		}
		// inlined FindLaunchOptionByValue() at line 1975
		{
			int numSlots;  // line 1979
			ConVar sv_portal_players;  // line 1987
		}
		{
			ConVar sv_portal_players;  // line 1959
			// inlined ConVarRef::SetValue() at line 1965
		}
		// inlined FindLaunchOptionByValue() at line 1950
		// inlined FindLaunchOptionByValue() at line 1946
		// inlined FindLaunchOptionByValue() at line 1946
	}
	ConVarRef coop_ref;  // line 1962
	ConVarRef coop_ref;  // line 2186
}

// game/server/gameinterface.cpp:2233 @0x4a34f0 _ZN14CServerGameDLL21ShouldPreferSteamAuthEv
bool CServerGameDLL::ShouldPreferSteamAuth()
{
}

// game/server/gameinterface.cpp:2238 @0x4a3500 _ZN14CServerGameDLL18GetMatchmakingTagsEPcm
void CServerGameDLL::GetMatchmakingTags( char *buf, size_t bufSize )
{
}

// game/server/gameinterface.cpp:2244 @0x4a3510 _ZN14CServerGameDLL23ServerHibernationUpdateEb
void CServerGameDLL::ServerHibernationUpdate( bool bHibernating )
{
}

// game/server/gameinterface.cpp:2252 @0x4a44b0 _ZN14CServerGameDLL20BuildAdjacentMapListEv
void CServerGameDLL::BuildAdjacentMapList()
{
	CSaveRestoreData *pSaveData;  // line 2255
}

// game/server/gameinterface.cpp:2265 (declaration)
bool IsValidPath( const char *pszFilename );

// game/server/gameinterface.cpp:2282 @0x4a3820 _ZL20ValidateMOTDFilenameP7IConVarPKcf
ValidateMOTDFilename( IConVar *pConVar, const char *oldValue, float flOldValue )
{
	ConVarRef var;  // line 2284
	// inlined ConVarRef::SetValue() at line 2287
	// inlined IsValidPath() at line 2285
	// inlined ConVarRef::GetString() at line 2285
}

// game/server/gameinterface.cpp:2291
static ConVar motdfile;

// game/server/gameinterface.cpp:2292
static ConVar hostfile;

// game/server/gameinterface.cpp:2293 @0x4a42a0 _Z12LoadMOTDFilePKcP6ConVar
LoadMOTDFile( const char *stringname, ConVar *pConvarFilename )
{
	char data[2048];  // line 2295
	int length;  // line 2297
	FileHandle_t hFile;  // line 2304
	// inlined ConVar::GetString() at line 2300
	// inlined ConVar::GetString() at line 2297
	// inlined ConVar::GetString() at line 2304
}

// game/server/gameinterface.cpp:2316 (declaration)
void LoadMessageOfTheDay();

// game/server/gameinterface.cpp:2316 @0x4a4460 _ZN14CServerGameDLL19LoadMessageOfTheDayEv
void CServerGameDLL::LoadMessageOfTheDay()
{
}

// game/server/gameinterface.cpp:2323
ConVar sv_unlockedchapters;

// game/server/gameinterface.cpp:2328 (declaration)
void UpdateChapterRestrictions( const char *mapname );

// game/server/gameinterface.cpp:2420 (declaration)
void PrecacheMaterial( const char *pMaterialName );

// game/server/gameinterface.cpp:2420 @0x4a3520 _Z16PrecacheMaterialPKc
PrecacheMaterial( const char *pMaterialName )
{
}

// game/server/gameinterface.cpp:2430 @0x4a4240 _Z16GetMaterialIndexPKc
int GetMaterialIndex( const char *pMaterialName )
{
	{
		int nIndex;  // line 2434
	}
}

// game/server/gameinterface.cpp:2454 @0x4a3560 _Z24GetMaterialNameFromIndexi
const char *GetMaterialNameFromIndex( int nMaterialIndex )
{
}

// game/server/gameinterface.cpp:2463 (declaration)
int PrecacheParticleSystem( const char *pParticleSystemName );

// game/server/gameinterface.cpp:2463 @0x4a3590 _Z22PrecacheParticleSystemPKc
int PrecacheParticleSystem( const char *pParticleSystemName )
{
}

// game/server/gameinterface.cpp:2469 @0x4a6d90 _Z30PrecacheParticleFileAndSystemsPKc
PrecacheParticleFileAndSystems( const char *pParticleSystemFile )
{
	CUtlVector<CUtlString,CUtlMemory<CUtlString, int> > systems;  // line 2478
	int nCount;  // line 2481
	// inlined CUtlVector<CUtlString,CUtlMemory<CUtlString, int> >::~CUtlVector() at line 2485
	// inlined CUtlVector<CUtlString,CUtlMemory<CUtlString, int> >::~CUtlVector() at line 2485
	{
		int i;  // line 2482
		// inlined PrecacheParticleSystem() at line 2484
	}
	// inlined CUtlVector<CUtlString,CUtlMemory<CUtlString, int> >::Count() at line 2481
	// inlined CUtlVector<CUtlString,CUtlMemory<CUtlString, int> >::CUtlVector() at line 2478
}

// game/server/gameinterface.cpp:2488 @0x4a4200 _Z22PrecacheGameSoundsFilePKc
PrecacheGameSoundsFile( const char *pSoundFile )
{
}

// game/server/gameinterface.cpp:2497 @0x4a41a0 _Z22GetParticleSystemIndexPKc
int GetParticleSystemIndex( const char *pParticleSystemName )
{
	{
		int nIndex;  // line 2501
	}
}

// game/server/gameinterface.cpp:2515 @0x4a35d0 _Z30GetParticleSystemNameFromIndexi
const char *GetParticleSystemNameFromIndex( int nMaterialIndex )
{
}

// game/server/gameinterface.cpp:2526 (declaration)
void PrecacheEffect( const char *pEffectName );

// game/server/gameinterface.cpp:2526 @0x4a3620 _Z14PrecacheEffectPKc
PrecacheEffect( const char *pEffectName )
{
}

// game/server/gameinterface.cpp:2536 @0x4a4140 _Z14GetEffectIndexPKc
int GetEffectIndex( const char *pEffectName )
{
	{
		int nIndex;  // line 2540
	}
}

// game/server/gameinterface.cpp:2554 @0x4a3660 _Z22GetEffectNameFromIndexi
const char *GetEffectNameFromIndex( int nEffectIndex )
{
}

// game/server/gameinterface.cpp:2566 @0x4a4ba0 _Z16IsEngineThreadedv
bool IsEngineThreaded()
{
}

// game/server/gameinterface.cpp:2576 sizeof=0x4 (i386)
struct CServerGameEnts : public IServerGameEnts
{
public:
	virtual void MarkEntitiesAsTouching( edict_t *, edict_t * );  // line 2578
	virtual void FreeContainingEntity( edict_t * );  // line 2579
	virtual edict_t *BaseEntityToEdict( CBaseEntity * );  // line 2580
	virtual CBaseEntity *EdictToBaseEntity( edict_t * );  // line 2581
	virtual void CheckTransmit( CCheckTransmitInfo *, const short unsigned int *, int );  // line 2582
	virtual void PrepareForFullUpdate( edict_t * );  // line 2583
};

// game/server/gameinterface.cpp:2576 (declaration)
~CServerGameEnts();

// game/server/gameinterface.cpp:2576 @0x4aba80 _ZN15CServerGameEntsD0Ev
CServerGameEnts::~CServerGameEnts()
{
}

// game/server/gameinterface.cpp:2576 @0x4abaa0 _ZN15CServerGameEntsD1Ev
CServerGameEnts::~CServerGameEnts()
{
}

// game/server/gameinterface.cpp:2576 (declaration)
void CServerGameEnts();

// game/server/gameinterface.cpp:2578 @0x4a6c40 _ZN15CServerGameEnts22MarkEntitiesAsTouchingEP7edict_tS1_
void CServerGameEnts::MarkEntitiesAsTouching( edict_t *e1, edict_t *e2 )
{
	CBaseEntity *entity;  // line 2594
	CBaseEntity *entityTouched;  // line 2595
	{
		trace_t tr;  // line 2599
		// inlined Vector::operator=() at line 2601
		// inlined Vector::operator*() at line 2601
		// inlined Vector::operator+() at line 2601
		// inlined CBaseEntity::GetAbsOrigin() at line 2601
		// inlined CBaseEntity::GetAbsOrigin() at line 2601
	}
	// inlined GetContainingEntity() at line 2595
	// inlined GetContainingEntity() at line 2594
}

// game/server/gameinterface.cpp:2579 @0x4a4130 _ZN15CServerGameEnts20FreeContainingEntityEP7edict_t
void CServerGameEnts::FreeContainingEntity( edict_t *e )
{
}

// game/server/gameinterface.cpp:2580 @0x4a4c10 _ZN15CServerGameEnts17BaseEntityToEdictEP11CBaseEntity
edict_t *CServerGameEnts::BaseEntityToEdict( CBaseEntity *pEnt )
{
	// inlined CBaseEntity::edict() at line 2614
}

// game/server/gameinterface.cpp:2581 @0x4a8c80 _ZN15CServerGameEnts17EdictToBaseEntityEP7edict_t
CBaseEntity *CServerGameEnts::EdictToBaseEntity( edict_t *pEdict )
{
	// inlined CBaseEntity::Instance() at line 2622
}

// game/server/gameinterface.cpp:2582 @0x4a8860 _ZN15CServerGameEnts13CheckTransmitEP18CCheckTransmitInfoPKti
void CServerGameEnts::CheckTransmit( CCheckTransmitInfo *pInfo, const short unsigned int *pEdictIndices, int nEdicts )
{
	edict_t *pBaseEdict;  // line 2650
	CBaseEntity *pRecipientEntity;  // line 2652
	CMDLCacheCriticalSection cacheCriticalSection;  // line 2657
	CBasePlayer *pRecipientPlayer;  // line 2658
	const int skyBoxArea;  // line 2659
	const bool bIsHLTV;  // line 2661
	const bool bIsReplay;  // line 2665
	// inlined CBaseEntity::Instance() at line 2652
	// inlined CMDLCacheCriticalSection::CMDLCacheCriticalSection() at line 2657
	// inlined CBasePlayer::IsHLTV() at line 2661
	{
		int i;  // line 2673
		{
			int iEdict;  // line 2675
			edict_t *pEdict;  // line 2683
			int nFlags;  // line 2684
			CBaseEntity *pEnt;  // line 2725
			CServerNetworkProperty *netProp;  // line 2746
			bool bSameAreaAsSky;  // line 2766
			bool bInPVS;  // line 2773
			CBaseEntity *orig;  // line 2783
			CServerNetworkProperty *check;  // line 2784
			{
				CServerNetworkProperty *pEnt;  // line 2710
				CServerNetworkProperty *pParent;  // line 2714
				// inlined CBitVecT<CFixedBitVecBase<2048> >::Set() at line 2702
				// inlined CBaseEdict::GetNetworkable() at line 2710
				// inlined CServerNetworkProperty::edict() at line 2718
				// inlined CServerNetworkProperty::entindex() at line 2719
				// inlined CBitVecT<CFixedBitVecBase<2048> >::Set() at line 2707
			}
			{
				int checkIndex;  // line 2790
				edict_t *checkEdict;  // line 2799
				int checkFlags;  // line 2800
				{
					bool bMoveParentInPVS;  // line 2828
				}
				// inlined CServerNetworkProperty::entindex() at line 2790
				{
					CBaseEntity *pCheckEntity;  // line 2813
				}
			}
		}
	}
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 2839
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 2839
}

// game/server/gameinterface.cpp:2583 @0x4a87f0 _ZN15CServerGameEnts20PrepareForFullUpdateEP7edict_t
void CServerGameEnts::PrepareForFullUpdate( edict_t *pEdict )
{
	CBaseEntity *pEntity;  // line 2849
	CBasePlayer *pPlayer;  // line 2856
	// inlined CBaseEntity::Instance() at line 2849
}

// game/server/gameinterface.cpp:2585 @0x4a36b0 _ZL48__CreateCServerGameEntsIServerGameEnts_interfacev
void *__CreateCServerGameEntsIServerGameEnts_interface()
{
}

// game/server/gameinterface.cpp:2585
static CServerGameEnts __g_CServerGameEnts_singleton;

// game/server/gameinterface.cpp:2585
static InterfaceReg __g_CreateCServerGameEntsIServerGameEnts_reg;

// game/server/gameinterface.cpp:2861
CServerGameClients g_ServerGameClients;

// game/server/gameinterface.cpp:2862 @0x4a36c0 _ZL54__CreateCServerGameClientsIServerGameClients_interfacev
void *__CreateCServerGameClientsIServerGameClients_interface()
{
}

// game/server/gameinterface.cpp:2862
static InterfaceReg __g_CreateCServerGameClientsIServerGameClients_reg;

// game/server/gameinterface.cpp:2875 @0x4a36d0 _ZN18CServerGameClients13ClientConnectEP7edict_tPKcS3_Pci
bool CServerGameClients::ClientConnect( edict_t *pEdict, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen )
{
}

// game/server/gameinterface.cpp:2884 @0x4a86f0 _ZN18CServerGameClients12ClientActiveEP7edict_tb
void CServerGameClients::ClientActive( edict_t *pEdict, bool bLoadGame )
{
	CMDLCacheCriticalSection cacheCriticalSection;  // line 2886
	CBasePlayer *pPlayer;  // line 2903
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 2905
	// inlined CBaseEntity::Instance() at line 2903
	{
		CBaseEntity *pEntity;  // line 2896
		// inlined CGlobalEntityList::FirstEnt() at line 2896
	}
	// inlined CMDLCacheCriticalSection::CMDLCacheCriticalSection() at line 2886
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 2905
}

// game/server/gameinterface.cpp:2913 @0x4a4120 _ZN18CServerGameClients18ClientFullyConnectEP7edict_t
void CServerGameClients::ClientFullyConnect( edict_t *pEdict )
{
}

// game/server/gameinterface.cpp:2923 @0x4a82f0 _ZN18CServerGameClients16ClientDisconnectEP7edict_t
void CServerGameClients::ClientDisconnect( edict_t *pEdict )
{
	bool g_fGameOver;  // line 2925
	CBasePlayer *player;  // line 2927
	// inlined CBaseEntity::Instance() at line 2927
	{
		CSound *pSound;  // line 2934
		// inlined CBaseEntity::AddSolidFlags() at line 2948
		// inlined CBasePlayer::SetMaxSpeed() at line 2932
	}
}

// game/server/gameinterface.cpp:2970 @0x4a6b00 _ZN18CServerGameClients17ClientPutInServerEP7edict_tPKc
void CServerGameClients::ClientPutInServer( edict_t *pEntity, const char *playername )
{
	CBasePlayer *pPlayer;  // line 2977
	// inlined GetContainingEntity() at line 2977
	// inlined ToBasePlayer() at line 2977
	{
		bool bIsSplitScreenPlayer;  // line 2980
		CBasePlayer *pAttachedTo;  // line 2981
		// inlined CBaseEntity::entindex() at line 2980
		// inlined CBaseEntity::entindex() at line 2984
		// inlined GetContainingEntity() at line 2984
	}
}

// game/server/gameinterface.cpp:2991 @0x4a6aa0 _ZN18CServerGameClients13ClientCommandEP7edict_tRK8CCommand
void CServerGameClients::ClientCommand( edict_t *pEntity, const CCommand &args )
{
	CBasePlayer *pPlayer;  // line 2993
	// inlined ToBasePlayer() at line 2993
	// inlined GetContainingEntity() at line 2993
}

// game/server/gameinterface.cpp:3003 @0x4a7db0 _ZN18CServerGameClients21ClientSettingsChangedEP7edict_t
void CServerGameClients::ClientSettingsChanged( edict_t *pEdict )
{
	CBasePlayer *player;  // line 3009
	bool useInterpolation;  // line 3029
	bool usePrediction;  // line 3057
	// inlined CBaseEdict::GetUnknown() at line 3006
	// inlined CBaseEntity::Instance() at line 3009
	// inlined CBaseEntity::entindex() at line 3023
	// inlined ConVar::GetFloat() at line 3027
	// inlined ConVar::GetFloat() at line 3027
	// inlined CBaseEntity::entindex() at line 3029
	// inlined CBaseEntity::entindex() at line 3057
	// inlined CBaseEntity::entindex() at line 3061
	// inlined CBaseEntity::entindex() at line 3062
	{
		float flLerpRatio;  // line 3032
		float flLerpAmount;  // line 3035
		// inlined ConVar::GetFloat() at line 3039
		// inlined CBaseEntity::entindex() at line 3035
		// inlined CBaseEntity::entindex() at line 3032
		// inlined ConVar::GetFloat() at line 3041
	}
	const ConVar *pMinUpdateRate;  // line 3024
	const ConVar *pMaxUpdateRate;  // line 3025
	const ConVar *pMin;  // line 3037
	const ConVar *pMax;  // line 3038
}

// game/server/gameinterface.cpp:3084 @0x4a8cb0 _Z38TestAreaPortalVisibilityThroughPortalsP19CFuncAreaPortalBaseP7edict_tPhi
int TestAreaPortalVisibilityThroughPortals( CFuncAreaPortalBase *pAreaPortal, edict_t *pViewEntity, unsigned char *pvs, int pvssize )
{
	int iPortalCount;  // line 3086
	CPortal_Base2D **pPortals;  // line 3090
	{
		int i;  // line 3092
		{
			CPortal_Base2D *pLocalPortal;  // line 3094
			{
				CPortal_Base2D *pRemotePortal;  // line 3097
				// inlined CNetworkHandleBase<CPortal_Base2D,CPortal_Base2D::NetworkVar_m_hLinkedPortal>::Get() at line 3097
				{
					bool bIsOpenOnClient;  // line 3103
					float fovDistanceAdjustFactor;  // line 3104
					Vector portalOrg;  // line 3105
					CUtlVector<Vector,CUtlMemory<Vector, int> > orgs;  // line 3106
					int iPortalNeedsThisPortalOpen;  // line 3108
					// inlined CBaseEntity::GetAbsOrigin() at line 3105
					// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::CUtlVector() at line 3106
					// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::AddToTail() at line 3107
					// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::~CUtlVector() at line 3114
					// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::~CUtlVector() at line 3114
					// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::~CUtlVector() at line 3114
				}
			}
		}
	}
	// inlined CUtlVector<CPortal_Base2D*,CUtlMemory<CPortal_Base2D*, int> >::Base() at line 3090
	// inlined CUtlVector<CPortal_Base2D*,CUtlMemory<CPortal_Base2D*, int> >::Count() at line 3086
}

// game/server/gameinterface.cpp:3135 @0x4a8fe0 _ZN18CServerGameClients21ClientSetupVisibilityEP7edict_tS1_Phi
void CServerGameClients::ClientSetupVisibility( edict_t *pViewEntity, edict_t *pClient, unsigned char *pvs, int pvssize )
{
	Vector org;  // line 3137
	CBaseEntity *pVE;  // line 3145
	float fovDistanceAdjustFactor;  // line 3157
	CUtlVector<Vector,CUtlMemory<Vector, int> > areaPortalOrigins;  // line 3159
	CBasePlayer *pPlayer;  // line 3161
	unsigned char portalBits[24];  // line 3190
	int portalNums[512];  // line 3193
	int isOpen[512];  // line 3194
	int iOutPortal;  // line 3195
	CPortal_Player *pPortalPlayer;  // line 3250
	// inlined Vector::operator=() at line 3166
	// inlined GetContainingEntity() at line 3148
	// inlined Vector::operator=() at line 3152
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::CUtlVector() at line 3159
	// inlined GetContainingEntity() at line 3161
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::AddToTail() at line 3172
	{
		int i;  // line 3175
		{
			CBasePlayer *pl;  // line 3177
			// inlined CBaseEntity::entindex() at line 3177
			// inlined GetContainingEntity() at line 3177
			// inlined ToBasePlayer() at line 3177
			// inlined Vector::operator=() at line 3180
			// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::AddToTail() at line 3181
		}
	}
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::AddToTail() at line 3187
	{
		short unsigned int i;  // line 3197
		// inlined CUtlLinkedList<CFuncAreaPortalBase*,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CFuncAreaPortalBase*, short unsigned int>, short unsigned int> >::Next() at line 3197
		{
			CFuncAreaPortalBase *pCur;  // line 3199
			bool bIsOpenOnClient;  // line 3201
			// inlined CUtlLinkedList<CFuncAreaPortalBase*,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CFuncAreaPortalBase*, short unsigned int>, short unsigned int> >::operator[]() at line 3199
		}
		// inlined CUtlLinkedList<CFuncAreaPortalBase*,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CFuncAreaPortalBase*, short unsigned int>, short unsigned int> >::Head() at line 3197
	}
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::~CUtlVector() at line 3254
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::~CUtlVector() at line 3254
}

// game/server/gameinterface.cpp:3275 @0x4a7920 _ZN18CServerGameClients15ProcessUsercmdsEP7edict_tP7bf_readiiibb
float CServerGameClients::ProcessUsercmds( edict_t *player, bf_read *buf, int numcmds, int totalcmds, int dropped_packets, bool ignore, bool paused )
{
	int i;  // line 3277
	CUserCmd *from;  // line 3278
	CUserCmd *to;  // line 3278
	CUserCmd cmds[64];  // line 3282
	CUserCmd cmdNull;  // line 3284
	CBasePlayer *pPlayer;  // line 3289
	CBaseEntity *pEnt;  // line 3290
	CMDLCacheCriticalSection cacheCriticalSection;  // line 3327
	{
		const char *name;  // line 3298
		// inlined CBitBuffer::SetOverflowFlag() at line 3307
	}
	// inlined CBaseEntity::Instance() at line 3290
	// inlined CUserCmd::CUserCmd() at line 3284
	// inlined CUserCmd::CUserCmd() at line 3282
	// inlined CUserCmd::Reset() at line 3312
	// inlined CMDLCacheCriticalSection::CMDLCacheCriticalSection() at line 3327
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 3330
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 3330
}

// game/server/gameinterface.cpp:3334 @0x4a4dc0 _ZN18CServerGameClients22PostClientMessagesSentEv
void CServerGameClients::PostClientMessagesSent()
{
	CVProfScope VProf_;  // line 3336
	// inlined CVProfScope::~CVProfScope() at line 3337
	// inlined CVProfScope::CVProfScope() at line 3336
	// inlined CVProfScope::~CVProfScope() at line 3337
}

// game/server/gameinterface.cpp:3341 @0x4a3720 _ZN18CServerGameClients16SetCommandClientEi
void CServerGameClients::SetCommandClient( int index )
{
}

// game/server/gameinterface.cpp:3346 @0x4a7890 _ZN18CServerGameClients14GetReplayDelayEP7edict_tRi
int CServerGameClients::GetReplayDelay( edict_t *pEdict, int &entity )
{
	CBasePlayer *pPlayer;  // line 3348
	// inlined CBaseEntity::Instance() at line 3348
}

// game/server/gameinterface.cpp:3362 @0x4a84e0 _ZN18CServerGameClients17ClientEarPositionEP7edict_tP6Vector
void CServerGameClients::ClientEarPosition( edict_t *pEdict, Vector *pEarOrigin )
{
	CBasePlayer *pPlayer;  // line 3364
	// inlined Vector::operator=() at line 3367
	// inlined CBaseEntity::Instance() at line 3364
	// inlined Vector::operator=() at line 3373
}

// game/server/gameinterface.cpp:3382 @0x4a84a0 _ZN18CServerGameClients14GetPlayerStateEP7edict_t
CPlayerState *CServerGameClients::GetPlayerState( edict_t *player )
{
	CBasePlayer *pBasePlayer;  // line 3388
	// inlined CBaseEdict::GetUnknown() at line 3385
	// inlined CBaseEntity::Instance() at line 3388
}

// game/server/gameinterface.cpp:3401 @0x4a6230 _ZN18CServerGameClients16GetBugReportInfoEPci
void CServerGameClients::GetBugReportInfo( char *buf, int buflen )
{
	recentNPCSpeech_t speech[5];  // line 3403
	int num;  // line 3404
	int i;  // line 3405
	int iPortalCount;  // line 3435
	// inlined CUtlVector<CProp_Portal*,CUtlMemory<CProp_Portal*, int> >::Count() at line 3435
	{
		CProp_Portal **pPortals;  // line 3438
		// inlined CUtlVector<CProp_Portal*,CUtlMemory<CProp_Portal*, int> >::Base() at line 3438
		{
			int i;  // line 3440
			{
				CProp_Portal *pTempPortal;  // line 3442
				// inlined CBaseEntity::GetAbsOrigin() at line 3443
				// inlined CBaseEntity::GetAbsOrigin() at line 3443
				// inlined CBaseEntity::GetAbsOrigin() at line 3443
			}
		}
	}
	{
		CBaseEntity *ent;  // line 3411
		// inlined string_t::ToCStr() at line 3418
		// inlined string_t::ToCStr() at line 3418
		// inlined CBaseEntity::GetClassname() at line 3418
		// inlined CBaseEntity::entindex() at line 3418
	}
}

// game/server/gameinterface.cpp:3452 @0x4a40a0 _ZN18CServerGameClients11ClientVoiceEP7edict_t
void CServerGameClients::ClientVoice( edict_t *pEdict )
{
	CBasePlayer *pPlayer;  // line 3454
	// inlined CBaseEntity::Instance() at line 3454
}

// game/server/gameinterface.cpp:3467 @0x4a3740 _ZN18CServerGameClients18NetworkIDValidatedEPKcS1_
void CServerGameClients::NetworkIDValidated( const char *pszUserName, const char *pszNetworkID )
{
}

// game/server/gameinterface.cpp:3471 @0x4a3750 _ZN18CServerGameClients24GetMaxSplitscreenPlayersEv
int CServerGameClients::GetMaxSplitscreenPlayers()
{
}

// game/server/gameinterface.cpp:3476 @0x4a3760 _ZN18CServerGameClients18GetMaxHumanPlayersEv
int CServerGameClients::GetMaxHumanPlayers()
{
}

// game/server/gameinterface.cpp:3486 @0x4a4020 _ZN18CServerGameClients22ClientCommandKeyValuesEP7edict_tP9KeyValues
void CServerGameClients::ClientCommandKeyValues( edict_t *pEntity, KeyValues *pKeyValues )
{
	const char *szCommand;  // line 3491
	// inlined FStrEq() at line 3493
}

// game/server/gameinterface.cpp:3505
static bf_write *g_pMsgBuffer;

// game/server/gameinterface.cpp:3507 @0x4a4c30 _Z18EntityMessageBeginP11CBaseEntityb
EntityMessageBegin( CBaseEntity *entity, bool reliable )
{
	// inlined CBaseEntity::entindex() at line 3513
}

// game/server/gameinterface.cpp:3516 @0x4a3f90 _Z16UserMessageBeginR16IRecipientFilterPKc
UserMessageBegin( IRecipientFilter &filter, const char *messagename )
{
	int msg_type;  // line 3522
}

// game/server/gameinterface.cpp:3532 @0x4a37a0 _Z10MessageEndv
MessageEnd()
{
}

// game/server/gameinterface.cpp:3541 @0x4a3f40 _Z16MessageWriteBytei
MessageWriteByte( int iValue )
{
}

// game/server/gameinterface.cpp:3549 @0x4a3ef0 _Z16MessageWriteChari
MessageWriteChar( int iValue )
{
}

// game/server/gameinterface.cpp:3557 @0x4a3b90 _Z17MessageWriteShorti
MessageWriteShort( int iValue )
{
}

// game/server/gameinterface.cpp:3565 @0x4a3ea0 _Z16MessageWriteWordi
MessageWriteWord( int iValue )
{
}

// game/server/gameinterface.cpp:3573 @0x4a3af0 _Z16MessageWriteLongi
MessageWriteLong( int iValue )
{
}

// game/server/gameinterface.cpp:3581 @0x4a3e50 _Z17MessageWriteFloatf
MessageWriteFloat( float flValue )
{
}

// game/server/gameinterface.cpp:3589 @0x4a3e00 _Z17MessageWriteAnglef
MessageWriteAngle( float flValue )
{
}

// game/server/gameinterface.cpp:3597 @0x4a3db0 _Z17MessageWriteCoordf
MessageWriteCoord( float flValue )
{
}

// game/server/gameinterface.cpp:3605 @0x4a3d60 _Z21MessageWriteVec3CoordRK6Vector
MessageWriteVec3Coord( const Vector &rgflValue )
{
}

// game/server/gameinterface.cpp:3613 @0x4a3d10 _Z22MessageWriteVec3NormalRK6Vector
MessageWriteVec3Normal( const Vector &rgflValue )
{
}

// game/server/gameinterface.cpp:3621 @0x4a3c80 _Z26MessageWriteBitVecIntegralRK6Vector
MessageWriteBitVecIntegral( const Vector &vecValue )
{
	{
		int i;  // line 3626
	}
}

// game/server/gameinterface.cpp:3632 @0x4a3c30 _Z18MessageWriteAnglesRK6QAngle
MessageWriteAngles( const QAngle &rgflValue )
{
}

// game/server/gameinterface.cpp:3640 @0x4a3be0 _Z18MessageWriteStringPKc
MessageWriteString( const char *sz )
{
}

// game/server/gameinterface.cpp:3648 @0x4a3b40 _Z18MessageWriteEntityi
MessageWriteEntity( int iValue )
{
}

// game/server/gameinterface.cpp:3656 @0x4a5850 _Z19MessageWriteEHandleP11CBaseEntity
MessageWriteEHandle( CBaseEntity *pEntity )
{
	long int iEncodedEHandle;  // line 3661
	{
		EHANDLE hEnt;  // line 3665
		int iSerialNum;  // line 3667
		// inlined CHandle<CBaseEntity>::CHandle() at line 3665
		// inlined CBaseHandle::GetEntryIndex() at line 3668
	}
}

// game/server/gameinterface.cpp:3679 @0x4a39a0 _Z16MessageWriteBoolb
MessageWriteBool( bool bValue )
{
	// inlined bf_write::WriteOneBit() at line 3684
}

// game/server/gameinterface.cpp:3687 @0x4a38a0 _Z20MessageWriteUBitLongji
MessageWriteUBitLong( unsigned int data, int numbits )
{
	// inlined bf_write::WriteUBitLong() at line 3692
}

// game/server/gameinterface.cpp:3695 @0x4a3aa0 _Z20MessageWriteSBitLongii
MessageWriteSBitLong( int data, int numbits )
{
}

// game/server/gameinterface.cpp:3703 @0x4a3a50 _Z16MessageWriteBitsPKvi
MessageWriteBits( const void *pIn, int nBits )
{
}

// game/server/gameinterface.cpp:3712 sizeof=0x18 (i386)
struct CServerDLLSharedAppSystems : public IServerDLLSharedAppSystems
{
public:
	CServerDLLSharedAppSystems();  // line 3714
	virtual int Count();  // line 3723
	virtual const char *GetDllName( int );  // line 3727
	virtual const char *GetInterfaceName( int );  // line 3731
private:
	void AddAppSystem( const char *, const char * );  // line 3736
	CUtlVector<AppSystemInfo_t,CUtlMemory<AppSystemInfo_t, int> > m_Systems; // +0x4  // line 3744
};

// game/server/gameinterface.cpp:3712 (declaration)
~CServerDLLSharedAppSystems();

// game/server/gameinterface.cpp:3723 @0x4aba30 _ZN26CServerDLLSharedAppSystems5CountEv
int CServerDLLSharedAppSystems::Count()
{
}

// game/server/gameinterface.cpp:3727 @0x4abc70 _ZN26CServerDLLSharedAppSystems10GetDllNameEi
const char *CServerDLLSharedAppSystems::GetDllName( int idx )
{
}

// game/server/gameinterface.cpp:3731 @0x4aba40 _ZN26CServerDLLSharedAppSystems16GetInterfaceNameEi
const char *CServerDLLSharedAppSystems::GetInterfaceName( int idx )
{
}

// game/server/gameinterface.cpp:3747 @0x4a37d0 _ZL70__CreateCServerDLLSharedAppSystemsIServerDLLSharedAppSystems_interfacev
void *__CreateCServerDLLSharedAppSystemsIServerDLLSharedAppSystems_interface()
{
}

// game/server/gameinterface.cpp:3747
static CServerDLLSharedAppSystems __g_CServerDLLSharedAppSystems_singleton;

// game/server/gameinterface.cpp:3747
static InterfaceReg __g_CreateCServerDLLSharedAppSystemsIServerDLLSharedAppSystems_reg;

// game/server/gameinterface.cpp:3753 @0x4a37e0 _ZN15CServerGameTags19GetTaggedConVarListEP9KeyValues
void CServerGameTags::GetTaggedConVarList( KeyValues *pCvarTagList )
{
}
