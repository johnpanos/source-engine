// DWARF declaration skeleton for game/client/c_baseentity.cpp
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x2c2d0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
	// inlined CUtlLinkedList<C_BaseEntity*,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<C_BaseEntity*, short unsigned int>, short unsigned int> >::CUtlLinkedList() at line 103
	// inlined CPredictableList::CPredictableList() at line 109
	// inlined CRecordingList::CRecordingList() at line 196
	// inlined ClientClass::ClientClass() at line 483
	// inlined ScriptClassDesc_t::ScriptClassDesc_t() at line 524
	// inlined PredMapInit<C_BaseEntity>() at line 612
	// inlined CUtlVector<C_BaseEntity*,CUtlMemory<C_BaseEntity*, int> >::CUtlVector() at line 2663
}

// public/ihandleentity.h:21 @0x2cce70 _ZN13IHandleEntityD0Ev
IHandleEntity::~IHandleEntity()
{
}

// public/ihandleentity.h:21 @0x2cce90 _ZN13IHandleEntityD1Ev
IHandleEntity::~IHandleEntity()
{
}

// public/iclientunknown.h:32 @0x2ccde0 _ZN14IClientUnknownD0Ev
IClientUnknown::~IClientUnknown()
{
}

// public/iclientunknown.h:32 @0x2cce00 _ZN14IClientUnknownD1Ev
IClientUnknown::~IClientUnknown()
{
}

// public/IClientEntity.h:32 @0x2cce20 _ZN13IClientEntityD0Ev
IClientEntity::~IClientEntity()
{
	// inlined IClientUnknown::~IClientUnknown() at line 32
}

// public/IClientEntity.h:32 @0x2ccfc0 _ZN13IClientEntityD1Ev
IClientEntity::~IClientEntity()
{
	// inlined IClientUnknown::~IClientUnknown() at line 32
}

// game/client/c_baseentity.cpp:60
static bool g_bWasSkipping;

// game/client/c_baseentity.cpp:61
static bool g_bWasThreaded;

// game/client/c_baseentity.cpp:62
static int g_nThreadModeTicks;

// game/client/c_baseentity.cpp:63
static ConVar cl_interp_threadmodeticks;

// game/client/c_baseentity.cpp:66 @0x2c07a0 _Z24cc_cl_interp_all_changedP7IConVarPKcf
cc_cl_interp_all_changed( IConVar *pConVar, const char *pOldString, float flOldValue )
{
	ConVarRef var;  // line 68
	{
		C_BaseEntityIterator iterator;  // line 71
		C_BaseEntity *pEnt;  // line 72
		// inlined C_BaseEntity::AddToEntityList() at line 77
	}
}

// game/client/c_baseentity.cpp:83
static ConVar report_cliententitysim;

// game/client/c_baseentity.cpp:84
static ConVar cl_extrapolate;

// game/client/c_baseentity.cpp:85
static ConVar cl_interp_npcs;

// game/client/c_baseentity.cpp:86
static ConVar cl_interp_all;

// game/client/c_baseentity.cpp:87
ConVar r_drawmodeldecals;

// game/client/c_baseentity.cpp:89
void C_BaseEntity::m_nPredictionRandomSeed;

// game/client/c_baseentity.cpp:90
void C_BaseEntity::m_pPredictionPlayer;

// game/client/c_baseentity.cpp:91
void C_BaseEntity::s_bAbsQueriesValid;

// game/client/c_baseentity.cpp:92
void C_BaseEntity::s_bAbsRecomputationEnabled;

// game/client/c_baseentity.cpp:93
void C_BaseEntity::s_bInterpolate;

// game/client/c_baseentity.cpp:95
void C_BaseEntity::sm_bDisableTouchFuncs;

// game/client/c_baseentity.cpp:97
static ConVar r_drawrenderboxes;

// game/client/c_baseentity.cpp:99
static bool g_bAbsRecomputationStack[8];

// game/client/c_baseentity.cpp:100
static short unsigned int g_iAbsRecomputationStackPos;

// game/client/c_baseentity.cpp:103
static CUtlLinkedList<C_BaseEntity*,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<C_BaseEntity*, short unsigned int>, short unsigned int> > g_EntityLists[5];

// game/client/c_baseentity.cpp:104
static bool s_bImmediateRemovesAllowed;

// game/client/c_baseentity.cpp:109
static CPredictableList g_Predictables[2];

// game/client/c_baseentity.cpp:110 (declaration)
CPredictableList *GetPredictables( int nSlot );

// game/client/c_baseentity.cpp:110 @0x2ba330 _Z15GetPredictablesi
CPredictableList *GetPredictables( int nSlot )
{
}

// public/tier1/utlstack.h:114 @0x2cd120 _ZN9CUtlStackIPK17typedescription_t10CUtlMemoryIS2_iEED1Ev
CUtlStack<const typedescription_t*,CUtlMemory<const typedescription_t*, int> >::~CUtlStack()
{
	// inlined CUtlStack<const typedescription_t*,CUtlMemory<const typedescription_t*, int> >::Purge() at line 116
	// inlined CUtlMemory<const typedescription_t*,int>::~CUtlMemory() at line 116
	// inlined CUtlMemory<const typedescription_t*,int>::~CUtlMemory() at line 116
}

// game/client/c_baseentity.cpp:121 (declaration)
void AddToPredictableList( C_BaseEntity *add );

// game/client/c_baseentity.cpp:121 @0x2c18f0 _ZN16CPredictableList20AddToPredictableListEP12C_BaseEntity
void CPredictableList::AddToPredictableList( C_BaseEntity *add )
{
}

// game/client/c_baseentity.cpp:137 (declaration)
void RemoveFromPredictablesList( C_BaseEntity *remove );

// game/client/c_baseentity.cpp:137 @0x2bc6d0 _ZN16CPredictableList26RemoveFromPredictablesListEP12C_BaseEntity
void CPredictableList::RemoveFromPredictablesList( C_BaseEntity *remove )
{
	// inlined CUtlVector<C_BaseEntity*,CUtlMemory<C_BaseEntity*, int> >::FindAndRemove() at line 139
}

// public/tier1/UtlSortVector.h:146 @0x2cd780 _ZN14CUtlSortVectorIP12C_BaseEntity17CEntIndexLessFuncE6InsertERKS1_
int CUtlSortVector<C_BaseEntity*,CEntIndexLessFunc>::Insert( C_BaseEntity *const &src )
{
	int pos;  // line 150
	// inlined CopyConstruct<C_BaseEntity*>() at line 153
	// inlined CUtlVector<C_BaseEntity*,CUtlMemory<C_BaseEntity*, int> >::ShiftElementsRight() at line 152
	{
		LoggingResponse_t ret;  // line 148
	}
}

// game/client/c_baseentity.cpp:174 sizeof=0x4 (i386)
struct IRecordingList
{
public:
	int (**_vptr$IRecordingList)(); // +0x0  // line 0
	virtual ~IRecordingList();  // line 176
	virtual void AddToList( ClientEntityHandle_t );  // line 177
	virtual void RemoveFromList( ClientEntityHandle_t );  // line 178
	virtual int Count();  // line 180
	virtual IClientRenderable *Get( int );  // line 181
};

// game/client/c_baseentity.cpp:176 @0x2ccda0 _ZN14IRecordingListD0Ev
IRecordingList::~IRecordingList()
{
}

// game/client/c_baseentity.cpp:176 @0x2ccdc0 _ZN14IRecordingListD1Ev
IRecordingList::~IRecordingList()
{
}

// game/client/c_baseentity.cpp:185 sizeof=0x18 (i386)
struct CRecordingList : public IRecordingList
{
public:
	virtual void AddToList( ClientEntityHandle_t );  // line 187
	virtual void RemoveFromList( ClientEntityHandle_t );  // line 188
	virtual int Count();  // line 190
	virtual IClientRenderable *Get( int );  // line 191
private:
	CUtlVector<CBaseHandle,CUtlMemory<CBaseHandle, int> > m_Recording; // +0x4  // line 193
};

// game/client/c_baseentity.cpp:185 (declaration)
~CRecordingList();

// game/client/c_baseentity.cpp:185 (declaration)
void CRecordingList();

// game/client/c_baseentity.cpp:185 @0x2cd430 _ZN14CRecordingListD0Ev
CRecordingList::~CRecordingList()
{
	// inlined CUtlVector<CBaseHandle,CUtlMemory<CBaseHandle, int> >::~CUtlVector() at line 185
}

// game/client/c_baseentity.cpp:185 @0x2cd510 _ZN14CRecordingListD1Ev
CRecordingList::~CRecordingList()
{
	// inlined CUtlVector<CBaseHandle,CUtlMemory<CBaseHandle, int> >::~CUtlVector() at line 185
}

// game/client/c_baseentity.cpp:187 @0x2c0ea0 _ZN14CRecordingList9AddToListE11CBaseHandle
void CRecordingList::AddToList( ClientEntityHandle_t &add )
{
	// inlined CUtlVector<CBaseHandle,CUtlMemory<CBaseHandle, int> >::Find() at line 207
	// inlined CUtlVector<CBaseHandle,CUtlMemory<CBaseHandle, int> >::AddToTail() at line 213
}

// game/client/c_baseentity.cpp:188 @0x2bc650 _ZN14CRecordingList14RemoveFromListE11CBaseHandle
void CRecordingList::RemoveFromList( ClientEntityHandle_t &remove )
{
	// inlined CUtlVector<CBaseHandle,CUtlMemory<CBaseHandle, int> >::FindAndRemove() at line 222
}

// game/client/c_baseentity.cpp:190 @0x2bc380 _ZN14CRecordingList5CountEv
int CRecordingList::Count()
{
}

// game/client/c_baseentity.cpp:191 @0x2bd4e0 _ZN14CRecordingList3GetEi
IClientRenderable *CRecordingList::Get( int index )
{
	// inlined CBaseHandle::CBaseHandle() at line 232
}

// game/client/c_baseentity.cpp:196
static CRecordingList g_RecordingList;

// game/client/c_baseentity.cpp:197
IRecordingList *recordinglist;

// public/tier1/UtlSortVector.h:211 @0x2cd8d0 _ZNK14CUtlSortVectorIP12C_BaseEntity17CEntIndexLessFuncE4FindERKS1_
int CUtlSortVector<C_BaseEntity*,CEntIndexLessFunc>::Find( C_BaseEntity *const &src )
{
	CEntIndexLessFunc less;  // line 215
	int start;  // line 217
	int end;  // line 217
	{
		int mid;  // line 220
		// inlined CUtlVector<C_BaseEntity*,CUtlMemory<C_BaseEntity*, int> >::Element() at line 221
		// inlined CEntIndexLessFunc::Less() at line 221
		// inlined CUtlVector<C_BaseEntity*,CUtlMemory<C_BaseEntity*, int> >::Element() at line 225
		// inlined CEntIndexLessFunc::Less() at line 225
	}
	{
		LoggingResponse_t ret;  // line 213
	}
}

// public/tier1/UtlSortVector.h:242 @0x2cd5f0 _ZNK14CUtlSortVectorIP12C_BaseEntity17CEntIndexLessFuncE15FindLessOrEqualERKS1_
int CUtlSortVector<C_BaseEntity*,CEntIndexLessFunc>::FindLessOrEqual( C_BaseEntity *const &src )
{
	CEntIndexLessFunc less;  // line 246
	int start;  // line 247
	int end;  // line 247
	{
		int mid;  // line 250
		// inlined CUtlVector<C_BaseEntity*,CUtlMemory<C_BaseEntity*, int> >::Element() at line 251
		// inlined CEntIndexLessFunc::Less() at line 251
		// inlined CUtlVector<C_BaseEntity*,CUtlMemory<C_BaseEntity*, int> >::Element() at line 255
		// inlined CEntIndexLessFunc::Less() at line 255
	}
	{
		LoggingResponse_t ret;  // line 244
	}
}

// game/client/c_baseentity.cpp:245 (declaration)
void CCurTimeScopeGuard( float flNewCurTime, bool bOptionalCondition );

// game/client/c_baseentity.cpp:245 @0x2ba350 _ZN18CCurTimeScopeGuardC2Efb
CCurTimeScopeGuard::CCurTimeScopeGuard( float flNewCurTime, bool bOptionalCondition )
{
}

// game/client/c_baseentity.cpp:245 @0x2ba3a0 _ZN18CCurTimeScopeGuardC1Efb
CCurTimeScopeGuard::CCurTimeScopeGuard( float flNewCurTime, bool bOptionalCondition )
{
}

// game/client/c_baseentity.cpp:259 (declaration)
~CCurTimeScopeGuard();

// game/client/c_baseentity.cpp:259 @0x2ba3f0 _ZN18CCurTimeScopeGuardD2Ev
CCurTimeScopeGuard::~CCurTimeScopeGuard()
{
}

// game/client/c_baseentity.cpp:259 @0x2ba420 _ZN18CCurTimeScopeGuardD1Ev
CCurTimeScopeGuard::~CCurTimeScopeGuard()
{
}

// game/client/c_baseentity.cpp:280 @0x2ba450 _Z18RecvProxy_AnimTimePK14CRecvProxyDataPvS2_
RecvProxy_AnimTime( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_BaseEntity *pEntity;  // line 282
	int t;  // line 285
	int tickbase;  // line 286
	int addt;  // line 287
}

// game/client/c_baseentity.cpp:308 @0x2be9a0 _Z24RecvProxy_SimulationTimePK14CRecvProxyDataPvS2_
RecvProxy_SimulationTime( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_BaseEntity *pEntity;  // line 310
	int t;  // line 313
	int tickbase;  // line 314
	int addt;  // line 315
}

// game/client/c_baseentity.cpp:336 @0x2bea30 _ZN12C_BaseEntity18RecvProxy_CellBitsEPK14CRecvProxyDataPvS3_
void C_BaseEntity::RecvProxy_CellBits( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_BaseEntity *pEnt;  // line 338
	// inlined C_BaseEntity::SetCellBits() at line 340
}

// game/client/c_baseentity.cpp:351 @0x2bda10 _ZN12C_BaseEntity15RecvProxy_CellXEPK14CRecvProxyDataPvS3_
void C_BaseEntity::RecvProxy_CellX( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_BaseEntity *pEnt;  // line 353
	int *cellX;  // line 355
}

// game/client/c_baseentity.cpp:367 @0x2bd9c0 _ZN12C_BaseEntity15RecvProxy_CellYEPK14CRecvProxyDataPvS3_
void C_BaseEntity::RecvProxy_CellY( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_BaseEntity *pEnt;  // line 369
	int *cellY;  // line 371
}

// game/client/c_baseentity.cpp:383 @0x2bd970 _ZN12C_BaseEntity15RecvProxy_CellZEPK14CRecvProxyDataPvS3_
void C_BaseEntity::RecvProxy_CellZ( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_BaseEntity *pEnt;  // line 385
	int *cellZ;  // line 387
}

// game/client/c_baseentity.cpp:399 @0x2bd8d0 _ZN12C_BaseEntity20RecvProxy_CellOriginEPK14CRecvProxyDataPvS3_
void C_BaseEntity::RecvProxy_CellOrigin( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_BaseEntity *pEnt;  // line 401
	Vector *vecNetworkOrigin;  // line 403
	{
		const int cellwidth;  // line 413
	}
}

// game/client/c_baseentity.cpp:420 @0x2bd850 _ZN12C_BaseEntity22RecvProxy_CellOriginXYEPK14CRecvProxyDataPvS3_
void C_BaseEntity::RecvProxy_CellOriginXY( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_BaseEntity *pEnt;  // line 422
	Vector *vecNetworkOrigin;  // line 424
	const int cellwidth;  // line 431
}

// game/client/c_baseentity.cpp:440 @0x2bd7f0 _ZN12C_BaseEntity21RecvProxy_CellOriginZEPK14CRecvProxyDataPvS3_
void C_BaseEntity::RecvProxy_CellOriginZ( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_BaseEntity *pEnt;  // line 442
	float *vecNetworkOriginZ;  // line 444
	const int cellwidth;  // line 450
}

// game/client/c_baseentity.cpp:458 @0x2c04b0 _Z23RecvProxy_LocalVelocityPK14CRecvProxyDataPvS2_
RecvProxy_LocalVelocity( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_BaseEntity *pEnt;  // line 460
	Vector vecVelocity;  // line 462
	// inlined C_BaseEntity::SetLocalVelocity() at line 469
}

// game/client/c_baseentity.cpp:471 @0x2c1d10 _Z23RecvProxy_ToolRecordingPK14CRecvProxyDataPvS2_
RecvProxy_ToolRecording( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_BaseEntity *pEnt;  // line 476
	// inlined C_BaseEntity::SetToolRecording() at line 477
}

// game/client/c_baseentity.cpp:483 @0x2ba4e0 _ZN12C_BaseEntity40YouForgotToImplementOrDeclareClientClassEv
int C_BaseEntity::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/c_baseentity.cpp:483 @0x2ba500 _ZN12C_BaseEntity14GetClientClassEv
ClientClass *C_BaseEntity::GetClientClass()
{
}

// game/client/c_baseentity.cpp:483 @0x2cb6a0 _ZL26_C_BaseEntity_CreateObjectii
IClientNetworkable *_C_BaseEntity_CreateObject( int entnum, int serialNum )
{
	C_BaseEntity *pRet;  // line 483
	// inlined C_BaseEntity::C_BaseEntity() at line 483
	// inlined C_BaseEntity::operator new() at line 483
	// inlined C_BaseEntity::operator delete() at line 483
}

// game/client/c_baseentity.cpp:483
ClientClass __g_C_BaseEntityClientClass;

// game/client/c_baseentity.cpp:483
void C_BaseEntity::m_pClassRecvTable;

// game/client/c_baseentity.cpp:485 @0x2be630 _ZL18RecvProxy_MoveTypePK14CRecvProxyDataPvS2_
RecvProxy_MoveType( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	// inlined C_BaseEntity::SetMoveType() at line 487
}

// game/client/c_baseentity.cpp:490 @0x2be980 _ZL21RecvProxy_MoveCollidePK14CRecvProxyDataPvS2_
RecvProxy_MoveCollide( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	// inlined C_BaseEntity::SetMoveCollide() at line 492
}

// game/client/c_baseentity.cpp:505 @0x2bcf30 _Z21RecvProxy_EffectFlagsPK14CRecvProxyDataPvS2_
RecvProxy_EffectFlags( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
}

// public/vscript/vscript_templates.h:507 @0x2cceb0 _ZN21CMemberScriptBinding0IP12C_BaseEntityMS0_FivEiE4CallEPvS5_P15ScriptVariant_tiS7_
bool CMemberScriptBinding0<C_BaseEntity*,int (C_BaseEntity::*)(),int>::Call( ScriptFunctionBindingStorageType_t pFunction, void *pContext, ScriptVariant_t *pArguments, int nArguments, ScriptVariant_t *pReturn )
{
	// inlined ScriptConvertFuncPtrFromVoid<int (C_BaseEntity::*)()>() at line 507
	// inlined ScriptVariant_t::operator=() at line 507
}

// public/vscript/vscript_templates.h:507 @0x2ccf10 _ZN21CMemberScriptBinding0IP12C_BaseEntityMS0_FRK6VectorvES4_E4CallEPvS8_P15ScriptVariant_tiSA_
bool CMemberScriptBinding0<C_BaseEntity*,const Vector& (C_BaseEntity::*)(),const Vector&>::Call( ScriptFunctionBindingStorageType_t pFunction, void *pContext, ScriptVariant_t *pArguments, int nArguments, ScriptVariant_t *pReturn )
{
	// inlined ScriptConvertFuncPtrFromVoid<const Vector& (C_BaseEntity::*)()>() at line 507
	// inlined ScriptVariant_t::operator=() at line 507
}

// game/client/c_baseentity.cpp:510 @0x2c0140 _Z19RecvProxy_ClrRenderPK14CRecvProxyDataPvS2_
RecvProxy_ClrRender( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_BaseEntity *pEnt;  // line 513
	uint32 color;  // line 514
	color32 c;  // line 515
	// inlined C_BaseEntity::SetRenderColor() at line 516
	// inlined C_BaseEntity::SetRenderAlpha() at line 517
}

// game/client/c_baseentity.cpp:520 @0x2bc00 _Z15ClientClassInitIN22DT_AnimTimeMustBeFirst7ignoredEEiPT_
int ClientClassInit<DT_AnimTimeMustBeFirst::ignored>( DT_AnimTimeMustBeFirst::ignored * )
{
	char *pRecvTableName;  // line 520
	RecvTable &RecvTable;  // line 520
	RecvProp RecvProps[2];  // line 520
}

// game/client/c_baseentity.cpp:520
RecvTable g_RecvTable;

// game/client/c_baseentity.cpp:520
int g_RecvTableInit;

// game/client/c_baseentity.cpp:524 (declaration)
ScriptClassDesc_t *GetScriptDesc<C_BaseEntity>( C_BaseEntity * );

// game/client/c_baseentity.cpp:524 @0x2b240 _Z13GetScriptDescI12C_BaseEntityEP17ScriptClassDesc_tPT_
ScriptClassDesc_t *GetScriptDesc<C_BaseEntity>( C_BaseEntity * )
{
}

// game/client/c_baseentity.cpp:524 @0x2be890 _ZN12C_BaseEntity13GetScriptDescEv
ScriptClassDesc_t *C_BaseEntity::GetScriptDesc()
{
}

// game/client/c_baseentity.cpp:524 @0x2bdf0 _Z26InitC_BaseEntityScriptDescv
InitC_BaseEntityScriptDesc()
{
	ScriptClassDesc_t *pDesc;  // line 524
	ScriptClassDesc_t *pInstanceHelperBase;  // line 524
	{
		ScriptFunctionBinding_t *pBinding;  // line 529
		// inlined ScriptDeduceFunctionSignature<InitC_BaseEntityScriptDesc()::_className*, C_BaseEntity, int>() at line 529
		// inlined CUtlVector<ScriptFunctionBinding_t,CUtlMemory<ScriptFunctionBinding_t, int> >::operator[]() at line 529
		// inlined CUtlVector<ScriptFunctionBinding_t,CUtlMemory<ScriptFunctionBinding_t, int> >::AddToTail() at line 529
	}
	{
		ScriptFunctionBinding_t *pBinding;  // line 528
		// inlined ScriptDeduceFunctionSignature<InitC_BaseEntityScriptDesc()::_className*, C_BaseEntity, const Vector&>() at line 528
		// inlined CUtlVector<ScriptFunctionBinding_t,CUtlMemory<ScriptFunctionBinding_t, int> >::operator[]() at line 528
		// inlined CUtlVector<ScriptFunctionBinding_t,CUtlMemory<ScriptFunctionBinding_t, int> >::AddToTail() at line 528
	}
	{
		ScriptFunctionBinding_t *pBinding;  // line 527
		// inlined ScriptDeduceFunctionSignature<InitC_BaseEntityScriptDesc()::_className*, C_BaseEntity, const Vector&>() at line 527
		// inlined CUtlVector<ScriptFunctionBinding_t,CUtlMemory<ScriptFunctionBinding_t, int> >::operator[]() at line 527
		// inlined CUtlVector<ScriptFunctionBinding_t,CUtlMemory<ScriptFunctionBinding_t, int> >::AddToTail() at line 527
	}
	{
		ScriptFunctionBinding_t *pBinding;  // line 526
		// inlined ScriptDeduceFunctionSignature<InitC_BaseEntityScriptDesc()::_className*, C_BaseEntity, const Vector&>() at line 526
		// inlined CUtlVector<ScriptFunctionBinding_t,CUtlMemory<ScriptFunctionBinding_t, int> >::operator[]() at line 526
		// inlined CUtlVector<ScriptFunctionBinding_t,CUtlMemory<ScriptFunctionBinding_t, int> >::AddToTail() at line 526
	}
	{
		ScriptFunctionBinding_t *pBinding;  // line 525
		// inlined ScriptDeduceFunctionSignature<InitC_BaseEntityScriptDesc()::_className*, C_BaseEntity, const Vector&>() at line 525
		// inlined CUtlVector<ScriptFunctionBinding_t,CUtlMemory<ScriptFunctionBinding_t, int> >::operator[]() at line 525
		// inlined CUtlVector<ScriptFunctionBinding_t,CUtlMemory<ScriptFunctionBinding_t, int> >::AddToTail() at line 525
	}
	bool bInitialized;  // line 524
}

// game/client/c_baseentity.cpp:524
ScriptClassDesc_t g_C_BaseEntity_ScriptDesc;

// game/client/c_baseentity.cpp:525 sizeof=0x8 (i386)
struct <anonymous>
{
public:
	const Vector &(*__pfn)(const C_BaseEntity *); // +0x0  // line 525
	int __delta; // +0x4  // line 525
};

// game/client/c_baseentity.cpp:526 sizeof=0x8 (i386)
struct <anonymous>
{
public:
	const Vector &(*__pfn)(C_BaseEntity *); // +0x0  // line 526
	int __delta; // +0x4  // line 526
};

// game/client/c_baseentity.cpp:529 sizeof=0x8 (i386)
struct <anonymous>
{
public:
	int (*__pfn)(const C_BaseEntity *); // +0x0  // line 529
	int __delta; // +0x4  // line 529
};

// game/client/c_baseentity.cpp:540 @0x2b250 _Z15ClientClassInitIN13DT_BaseEntity7ignoredEEiPT_
int ClientClassInit<DT_BaseEntity::ignored>( DT_BaseEntity::ignored * )
{
	char *pRecvTableName;  // line 540
	RecvTable &RecvTable;  // line 540
	RecvProp RecvProps[40];  // line 540
}

// game/client/c_baseentity.cpp:540
RecvTable g_RecvTable;

// game/client/c_baseentity.cpp:540
int g_RecvTableInit;

// game/client/c_baseentity.cpp:612 @0x2ba510 _ZN12C_BaseEntity14GetPredDescMapEv
datamap_t *C_BaseEntity::GetPredDescMap()
{
}

// game/client/c_baseentity.cpp:612 (declaration)
datamap_t *PredMapInit<C_BaseEntity>( C_BaseEntity * );

// game/client/c_baseentity.cpp:612 @0x2ba520 _Z11PredMapInitI12C_BaseEntityEP9datamap_tPT_
datamap_t *PredMapInit<C_BaseEntity>( C_BaseEntity * )
{
}

// game/client/c_baseentity.cpp:612
datamap_t *g_PredMapHolder;

// game/client/c_baseentity.cpp:612
void C_BaseEntity::m_PredMap;

// public/tier1/utlvector.h:655 @0x2cd000 _ZN10CUtlVectorI13VarMapEntry_t10CUtlMemoryIS0_iEE10GrowVectorEi
void CUtlVector<VarMapEntry_t,CUtlMemory<VarMapEntry_t, int> >::GrowVector( int num )
{
	// inlined CUtlMemory<VarMapEntry_t,int>::NumAllocated() at line 657
	// inlined CUtlMemory<VarMapEntry_t,int>::Grow() at line 660
	// inlined CUtlVector<VarMapEntry_t,CUtlMemory<VarMapEntry_t, int> >::ResetDbgInfo() at line 664
}

// public/tier1/utlvector.h:655 @0x2cd1f0 _ZN10CUtlVectorI11CBaseHandle10CUtlMemoryIS0_iEE10GrowVectorEi
void CUtlVector<CBaseHandle,CUtlMemory<CBaseHandle, int> >::GrowVector( int num )
{
	// inlined CUtlMemory<CBaseHandle,int>::NumAllocated() at line 657
	// inlined CUtlMemory<CBaseHandle,int>::Grow() at line 660
	// inlined CUtlVector<CBaseHandle,CUtlMemory<CBaseHandle, int> >::ResetDbgInfo() at line 664
}

// public/tier1/utlvector.h:655 @0x2cd310 _ZN10CUtlVectorIP12C_BaseEntity10CUtlMemoryIS1_iEE10GrowVectorEi
void CUtlVector<C_BaseEntity*,CUtlMemory<C_BaseEntity*, int> >::GrowVector( int num )
{
	// inlined CUtlMemory<C_BaseEntity*,int>::NumAllocated() at line 657
	// inlined CUtlMemory<C_BaseEntity*,int>::Grow() at line 660
	// inlined CUtlVector<C_BaseEntity*,CUtlMemory<C_BaseEntity*, int> >::ResetDbgInfo() at line 664
}

// game/client/c_baseentity.cpp:710 @0x2befc0 _Z19SpewInterpolatedVarP16CInterpolatedVarI6VectorE
SpewInterpolatedVar( CInterpolatedVar<Vector> *pVar )
{
	int i;  // line 713
	CApparentVelocity<Vector,CDefaultCalcDistance<Vector> > apparent;  // line 714
	float prevtime;  // line 715
	{
		float changetime;  // line 718
		Vector *pVal;  // line 719
		float vel;  // line 723
		// inlined CInterpolatedVarArrayBase<Vector,false>::GetNext() at line 725
		// inlined CApparentVelocity<Vector,CDefaultCalcDistance<Vector> >::AddSample() at line 723
		// inlined CInterpolatedVarArrayBase<Vector,false>::GetHistoryValue() at line 719
	}
}

// game/client/c_baseentity.cpp:731 @0x2c6c90 _Z19SpewInterpolatedVarP16CInterpolatedVarI6VectorEffb
SpewInterpolatedVar( CInterpolatedVar<Vector> *pVar, float flNow, float flInterpAmount, bool bSpewAllEntries )
{
	float target;  // line 733
	int i;  // line 736
	CApparentVelocity<Vector,CDefaultCalcDistance<Vector> > apparent;  // line 737
	float newtime;  // line 738
	Vector newVec;  // line 739
	bool bSpew;  // line 740
	{
		float changetime;  // line 744
		Vector *pVal;  // line 745
		float vel;  // line 790
		// inlined CInterpolatedVarArrayBase<Vector,false>::GetHistoryValue() at line 745
		{
			Vector o;  // line 751
			bool bInterp;  // line 753
			float frac;  // line 754
			char desc[32];  // line 755
			// inlined CInterpolatedVarArrayBase<Vector,false>::DebugInterpolate() at line 752
			{
				int savei;  // line 765
				float oldtertime;  // line 767
				// inlined CInterpolatedVarArrayBase<Vector,false>::GetNext() at line 766
				// inlined CInterpolatedVarArrayBase<Vector,false>::GetHistoryValue() at line 768
			}
		}
		// inlined CApparentVelocity<Vector,CDefaultCalcDistance<Vector> >::AddSample() at line 790
		// inlined CInterpolatedVarArrayBase<Vector,false>::GetNext() at line 795
	}
}

// game/client/c_baseentity.cpp:801 @0x2bc530 _Z19SpewInterpolatedVarP16CInterpolatedVarIfE
SpewInterpolatedVar( CInterpolatedVar<float> *pVar )
{
	int i;  // line 804
	CApparentVelocity<float,CDefaultCalcDistance<float> > apparent;  // line 805
	{
		float changetime;  // line 808
		float *pVal;  // line 809
		float vel;  // line 813
		// inlined CInterpolatedVarArrayBase<float,false>::GetNext() at line 815
		// inlined CApparentVelocity<float,CDefaultCalcDistance<float> >::AddSample() at line 813
		// inlined CInterpolatedVarArrayBase<float,false>::GetHistoryValue() at line 809
	}
}

// game/client/c_baseentity.cpp:844 @0x2bd4b0 _ZN12C_BaseEntity18SetAbsQueriesValidEb
void C_BaseEntity::SetAbsQueriesValid( bool bValid )
{
}

// game/client/c_baseentity.cpp:860 @0x2bd480 _ZN12C_BaseEntity17IsAbsQueriesValidEv
bool C_BaseEntity::IsAbsQueriesValid()
{
}

// game/client/c_baseentity.cpp:867 @0x2bd420 _ZN12C_BaseEntity27PushEnableAbsRecomputationsEb
void C_BaseEntity::PushEnableAbsRecomputations( bool bEnable )
{
}

// game/client/c_baseentity.cpp:883 @0x2bd3e0 _ZN12C_BaseEntity26PopEnableAbsRecomputationsEv
void C_BaseEntity::PopEnableAbsRecomputations()
{
}

// game/client/c_baseentity.cpp:898 @0x2bd3a0 _ZN12C_BaseEntity23EnableAbsRecomputationsEb
void C_BaseEntity::EnableAbsRecomputations( bool bEnable )
{
}

// game/client/c_baseentity.cpp:909 @0x2bd370 _ZN12C_BaseEntity26IsAbsRecomputationsEnabledEv
bool C_BaseEntity::IsAbsRecomputationsEnabled()
{
}

// game/client/c_baseentity.cpp:916 @0x2ba550 _ZN12C_BaseEntity20GetTextureFrameIndexEv
int C_BaseEntity::GetTextureFrameIndex()
{
}

// game/client/c_baseentity.cpp:921 @0x2ba560 _ZN12C_BaseEntity20SetTextureFrameIndexEi
void C_BaseEntity::SetTextureFrameIndex( int iIndex )
{
}

// game/client/c_baseentity.cpp:930 (declaration)
void Interp_SetupMappings( VarMapping_t *map );

// game/client/c_baseentity.cpp:930 @0x2bfd20 _ZN12C_BaseEntity20Interp_SetupMappingsEP12VarMapping_t
void C_BaseEntity::Interp_SetupMappings( VarMapping_t *map )
{
	int c;  // line 935
	// inlined CUtlVector<VarMapEntry_t,CUtlMemory<VarMapEntry_t, int> >::Count() at line 935
	{
		int i;  // line 936
		{
			VarMapEntry_t *e;  // line 938
			IInterpolatedVar *watcher;  // line 939
			void *data;  // line 940
			int type;  // line 941
			// inlined CUtlVector<VarMapEntry_t,CUtlMemory<VarMapEntry_t, int> >::operator[]() at line 938
		}
	}
}

// game/client/c_baseentity.cpp:948 (declaration)
void Interp_RestoreToLastNetworked( VarMapping_t *map );

// game/client/c_baseentity.cpp:948 @0x2c14d0 _ZN12C_BaseEntity29Interp_RestoreToLastNetworkedEP12VarMapping_t
void C_BaseEntity::Interp_RestoreToLastNetworked( VarMapping_t *map )
{
	Vector oldOrigin;  // line 952
	QAngle oldAngles;  // line 953
	int c;  // line 955
	// inlined CUtlVector<VarMapEntry_t,CUtlMemory<VarMapEntry_t, int> >::Count() at line 955
	{
		int i;  // line 956
		{
			VarMapEntry_t *e;  // line 958
			IInterpolatedVar *watcher;  // line 959
		}
	}
	// inlined C_BaseEntity::BaseInterpolatePart2() at line 963
}

// game/client/c_baseentity.cpp:966 (declaration)
void Interp_UpdateInterpolationAmounts( VarMapping_t *map );

// game/client/c_baseentity.cpp:966 @0x2bfca0 _ZN12C_BaseEntity33Interp_UpdateInterpolationAmountsEP12VarMapping_t
void C_BaseEntity::Interp_UpdateInterpolationAmounts( VarMapping_t *map )
{
	int c;  // line 971
	// inlined CUtlVector<VarMapEntry_t,CUtlMemory<VarMapEntry_t, int> >::Count() at line 971
	{
		int i;  // line 972
		{
			VarMapEntry_t *e;  // line 974
			IInterpolatedVar *watcher;  // line 975
		}
	}
}

// game/client/c_baseentity.cpp:980 (declaration)
void Interp_HierarchyUpdateInterpolationAmounts();

// game/client/c_baseentity.cpp:980 @0x2c8a60 _ZN12C_BaseEntity42Interp_HierarchyUpdateInterpolationAmountsEv
void C_BaseEntity::Interp_HierarchyUpdateInterpolationAmounts()
{
	{
		C_BaseEntity *pChild;  // line 984
		// inlined C_BaseEntity::NextMovePeer() at line 984
		// inlined C_BaseEntity::FirstMoveChild() at line 984
		// inlined C_BaseEntity::Interp_HierarchyUpdateInterpolationAmounts() at line 986
	}
	// inlined C_BaseEntity::Interp_UpdateInterpolationAmounts() at line 982
}

// game/client/c_baseentity.cpp:990 (declaration)
void Interp_Interpolate( VarMapping_t *map, float currentTime );

// game/client/c_baseentity.cpp:1027 (declaration)
void C_BaseEntity();

// game/client/c_baseentity.cpp:1027 @0x2caa00 _ZN12C_BaseEntityC2Ev
C_BaseEntity::C_BaseEntity()
{
	// inlined C_BaseEntity::AddToEntityList() at line 1086
	{
		int i;  // line 1082
	}
	// inlined CNetworkVarBase<bool,C_BaseEntity::NetworkVar_m_bAnimatedEveryTick>::operator=<bool>() at line 1052
	// inlined CNetworkVarBase<bool,C_BaseEntity::NetworkVar_m_bSimulatedEveryTick>::operator=<bool>() at line 1051
	// inlined C_BaseEntity::SetPredictionEligible() at line 1048
	// inlined CThreadFastMutex::CThreadFastMutex() at line 1029
	// inlined CThreadFastMutex::CThreadFastMutex() at line 1029
	// inlined CHandle<C_BaseEntity>::CHandle() at line 1029
	// inlined CHandle<C_BasePlayer>::CHandle() at line 1029
	// inlined CInterpolatedVar<QAngle>::CInterpolatedVar() at line 1029
	// inlined CInterpolatedVar<Vector>::CInterpolatedVar() at line 1029
	// inlined CHandle<C_BaseEntity>::CHandle() at line 1029
	// inlined C_BaseEntity::NetworkVar_m_Particles::NetworkVar_m_Particles() at line 1029
	// inlined C_BaseEntity::NetworkVar_m_Collision::NetworkVar_m_Collision() at line 1029
	// inlined CHandle<C_BaseEntity>::CHandle() at line 1029
	// inlined CHandle<C_BaseEntity>::CHandle() at line 1029
	// inlined CHandle<C_BaseEntity>::CHandle() at line 1029
	// inlined CHandle<C_BaseEntity>::CHandle() at line 1029
	// inlined CHandle<C_BaseEntity>::CHandle() at line 1029
	// inlined CBitVec<2>::CBitVec() at line 1029
	// inlined CUtlVector<thinkfunc_t,CUtlMemory<thinkfunc_t, int> >::CUtlVector() at line 1029
	// inlined CBaseHandle::CBaseHandle() at line 1029
	// inlined CBitVec<2>::CBitVec() at line 1029
	// inlined CHandle<C_BaseEntity>::CHandle() at line 1029
	// inlined CHandle<C_BaseEntity>::CHandle() at line 1029
	// inlined CHandle<C_BaseEntity>::CHandle() at line 1029
	// inlined VarMapping_t::VarMapping_t() at line 1029
	// inlined IClientEntity::IClientEntity() at line 1029
	// inlined C_BaseEntity::NetworkVar_m_Particles::~NetworkVar_m_Particles() at line 1104
	// inlined C_BaseEntity::NetworkVar_m_Collision::~NetworkVar_m_Collision() at line 1104
	// inlined CUtlVector<thinkfunc_t,CUtlMemory<thinkfunc_t, int> >::~CUtlVector() at line 1104
	// inlined VarMapping_t::~VarMapping_t() at line 1104
	// inlined IClientEntity::~IClientEntity() at line 1104
	// inlined CInterpolatedVar<Vector>::~CInterpolatedVar() at line 1104
	// inlined CInterpolatedVar<QAngle>::~CInterpolatedVar() at line 1104
}

// game/client/c_baseentity.cpp:1027 @0x2cb690 _ZN12C_BaseEntityC1Ev
C_BaseEntity::C_BaseEntity()
{
}

// public/tier1/interpolatedvar.h:1060 @0x2cdc10 _ZN25CInterpolatedVarArrayBaseI6VectorLb0EE28GetDerivative_SmoothVelocityEPS0_f
void CInterpolatedVarArrayBase<Vector,false>::GetDerivative_SmoothVelocity( Vector *pOut, float currentTime )
{
	CInterpolatedVarArrayBase<Vector,false>::CInterpolationInfo info;  // line 1062
	CSimpleRingBuffer<CInterpolatedVarEntryBase<Vector, false> > &history;  // line 1066
	bool bExtrapolate;  // line 1067
	int realOlder;  // line 1068
	// inlined CInterpolatedVarArrayBase<Vector,false>::_Derivative_Linear() at line 1123
	// inlined CSimpleRingBuffer<CInterpolatedVarEntryBase<Vector, false> >::operator[]() at line 1123
	// inlined CSimpleRingBuffer<CInterpolatedVarEntryBase<Vector, false> >::operator[]() at line 1123
	// inlined CInterpolatedVarArrayBase<Vector,false>::GetInterpolationInfo() at line 1063
	// inlined CSimpleRingBuffer<CInterpolatedVarEntryBase<Vector, false> >::operator[]() at line 1072
	// inlined CSimpleRingBuffer<CInterpolatedVarEntryBase<Vector, false> >::operator[]() at line 1072
	// inlined CSimpleRingBuffer<CInterpolatedVarEntryBase<Vector, false> >::operator[]() at line 1072
	// inlined CInterpolatedVarArrayBase<Vector,false>::_Derivative_Hermite_SmoothVelocity() at line 1072
	// inlined CInterpolatedVarArrayBase<Vector,false>::IsValidIndex() at line 1080
	// inlined CSimpleRingBuffer<CInterpolatedVarEntryBase<Vector, false> >::operator[]() at line 1080
	{
		float flDestTime;  // line 1109
		float diff;  // line 1110
		{
			float scale;  // line 1114
			{
				int i;  // line 1115
				// inlined Vector::operator*=() at line 1117
			}
		}
		// inlined ConVar::GetFloat() at line 1111
		// inlined CSimpleRingBuffer<CInterpolatedVarEntryBase<Vector, false> >::operator[]() at line 1110
		// inlined CInterpolatedVarArrayBase<Vector,false>::_Derivative_Linear() at line 1106
		// inlined CSimpleRingBuffer<CInterpolatedVarEntryBase<Vector, false> >::operator[]() at line 1106
		// inlined CSimpleRingBuffer<CInterpolatedVarEntryBase<Vector, false> >::operator[]() at line 1106
	}
}

// game/client/c_baseentity.cpp:1110 (declaration)
void CleanUpAlphaProperty();

// game/client/c_baseentity.cpp:1110 @0x2ba580 _ZN12C_BaseEntity20CleanUpAlphaPropertyEv
void C_BaseEntity::CleanUpAlphaProperty()
{
}

// game/client/c_baseentity.cpp:1125 (declaration)
~C_BaseEntity();

// game/client/c_baseentity.cpp:1125 @0x2cb740 _ZN12C_BaseEntityD0Ev
C_BaseEntity::~C_BaseEntity()
{
	// inlined C_BaseEntity::CleanUpAlphaProperty() at line 1128
	{
		int i;  // line 1133
		// inlined C_BaseEntity::RemoveFromEntityList() at line 1135
	}
	// inlined CInterpolatedVar<QAngle>::~CInterpolatedVar() at line 1137
	// inlined CInterpolatedVar<Vector>::~CInterpolatedVar() at line 1137
	// inlined C_BaseEntity::NetworkVar_m_Particles::~NetworkVar_m_Particles() at line 1137
	// inlined C_BaseEntity::NetworkVar_m_Collision::~NetworkVar_m_Collision() at line 1137
	// inlined CUtlVector<thinkfunc_t,CUtlMemory<thinkfunc_t, int> >::~CUtlVector() at line 1137
	// inlined VarMapping_t::~VarMapping_t() at line 1137
	// inlined IClientEntity::~IClientEntity() at line 1137
	// inlined C_BaseEntity::operator delete() at line 1137
	// inlined CInterpolatedVar<QAngle>::~CInterpolatedVar() at line 1137
	// inlined CInterpolatedVar<Vector>::~CInterpolatedVar() at line 1137
	// inlined C_BaseEntity::NetworkVar_m_Particles::~NetworkVar_m_Particles() at line 1137
	// inlined C_BaseEntity::NetworkVar_m_Collision::~NetworkVar_m_Collision() at line 1137
	// inlined CUtlVector<thinkfunc_t,CUtlMemory<thinkfunc_t, int> >::~CUtlVector() at line 1137
	// inlined VarMapping_t::~VarMapping_t() at line 1137
	// inlined IClientEntity::~IClientEntity() at line 1137
}

// game/client/c_baseentity.cpp:1125 @0x2cc260 _ZN12C_BaseEntityD2Ev
C_BaseEntity::~C_BaseEntity()
{
	// inlined C_BaseEntity::CleanUpAlphaProperty() at line 1128
	{
		int i;  // line 1133
		// inlined C_BaseEntity::RemoveFromEntityList() at line 1135
	}
	// inlined CInterpolatedVar<QAngle>::~CInterpolatedVar() at line 1137
	// inlined CInterpolatedVar<Vector>::~CInterpolatedVar() at line 1137
	// inlined C_BaseEntity::NetworkVar_m_Particles::~NetworkVar_m_Particles() at line 1137
	// inlined C_BaseEntity::NetworkVar_m_Collision::~NetworkVar_m_Collision() at line 1137
	// inlined CUtlVector<thinkfunc_t,CUtlMemory<thinkfunc_t, int> >::~CUtlVector() at line 1137
	// inlined VarMapping_t::~VarMapping_t() at line 1137
	// inlined IClientEntity::~IClientEntity() at line 1137
	// inlined C_BaseEntity::NetworkVar_m_Collision::~NetworkVar_m_Collision() at line 1137
	// inlined CUtlVector<thinkfunc_t,CUtlMemory<thinkfunc_t, int> >::~CUtlVector() at line 1137
	// inlined VarMapping_t::~VarMapping_t() at line 1137
	// inlined IClientEntity::~IClientEntity() at line 1137
	// inlined CInterpolatedVar<QAngle>::~CInterpolatedVar() at line 1137
	// inlined CInterpolatedVar<Vector>::~CInterpolatedVar() at line 1137
	// inlined C_BaseEntity::NetworkVar_m_Particles::~NetworkVar_m_Particles() at line 1137
}

// game/client/c_baseentity.cpp:1125 @0x2ccd80 _ZN12C_BaseEntityD1Ev
C_BaseEntity::~C_BaseEntity()
{
}

// game/client/c_baseentity.cpp:1139 @0x2c2280 _ZN12C_BaseEntity5ClearEv
void C_BaseEntity::Clear()
{
	// inlined CBaseHandle::Term() at line 1144
	// inlined C_BaseEntity::CleanUpAlphaProperty() at line 1153
	// inlined C_BaseEntity::SetLocalOrigin() at line 1155
	// inlined C_BaseEntity::SetLocalAngles() at line 1156
	// inlined Vector::Init() at line 1158
	// inlined QAngle::Init() at line 1159
	// inlined Vector::Init() at line 1160
	// inlined Vector::Init() at line 1162
	// inlined Vector::Init() at line 1163
	// inlined C_BaseEntity::SetSolid() at line 1167
	// inlined C_BaseEntity::SetSolidFlags() at line 1168
	// inlined C_BaseEntity::SetMoveType() at line 1170
	// inlined C_BaseEntity::ClearEffects() at line 1172
	// inlined C_BaseEntity::SetRenderColor() at line 1176
	// inlined C_BaseEntity::SetRenderAlpha() at line 1177
	// inlined C_BaseEntity::SetRenderFX() at line 1178
	// inlined CHandle<C_BaseEntity>::operator=() at line 1182
	// inlined Vector::Init() at line 1187
	// inlined QAngle::Init() at line 1188
}

// game/client/c_baseentity.cpp:1206 @0x2ba5d0 _ZN12C_BaseEntity22GetClientAlphaPropertyEv
IClientAlphaProperty *C_BaseEntity::GetClientAlphaProperty()
{
}

// game/client/c_baseentity.cpp:1215 @0x2ba5e0 _ZN12C_BaseEntity5SpawnEv
void C_BaseEntity::Spawn()
{
}

// game/client/c_baseentity.h:1217 @0x2cdb90 _ZN12C_BaseEntity16ScriptGetForwardEv
const Vector &C_BaseEntity::ScriptGetForward()
{
	// inlined C_BaseEntity::GetVectors() at line 1217
	Vector vecForward;  // line 1217
}

// game/client/c_baseentity.h:1218 @0x2cdae0 _ZN12C_BaseEntity13ScriptGetLeftEv
const Vector &C_BaseEntity::ScriptGetLeft()
{
	// inlined C_BaseEntity::GetVectors() at line 1218
	Vector vecLeft;  // line 1218
}

// game/client/c_baseentity.h:1219 @0x2cda60 _ZN12C_BaseEntity11ScriptGetUpEv
const Vector &C_BaseEntity::ScriptGetUp()
{
	// inlined C_BaseEntity::GetVectors() at line 1219
	Vector vecUp;  // line 1219
}

// game/client/c_baseentity.cpp:1222 @0x2ba5f0 _ZN12C_BaseEntity8ActivateEv
void C_BaseEntity::Activate()
{
}

// game/client/c_baseentity.cpp:1229 @0x2ba600 _ZN12C_BaseEntity17SpawnClientEntityEv
void C_BaseEntity::SpawnClientEntity()
{
}

// game/client/c_baseentity.cpp:1236 @0x2ba610 _ZN12C_BaseEntity8PrecacheEv
void C_BaseEntity::Precache()
{
}

// game/client/c_baseentity.cpp:1245 @0x2c6220 _ZN12C_BaseEntity4InitEii
bool C_BaseEntity::Init( int entnum, int iSerialNum )
{
	// inlined C_BaseEntity::Interp_SetupMappings() at line 1258
}

// game/client/c_baseentity.cpp:1270 @0x2c6130 _ZN12C_BaseEntity24InitializeAsClientEntityEPKcb
bool C_BaseEntity::InitializeAsClientEntity( const char *pszModelName, bool bRenderWithViewModels )
{
	int nModelIndex;  // line 1272
	// inlined C_BaseEntity::Interp_SetupMappings() at line 1290
}

// game/client/c_baseentity.cpp:1298 @0x2c2720 _ZN12C_BaseEntity31InitializeAsClientEntityByIndexEib
bool C_BaseEntity::InitializeAsClientEntityByIndex( int iIndex, bool bRenderWithViewModels )
{
	// inlined C_BaseEntity::RenderWithViewModels() at line 1301
	// inlined C_BaseEntity::SetModelByIndex() at line 1304
}

// game/client/c_baseentity.cpp:1322 @0x2c31c0 _ZN12C_BaseEntity4TermEv
void C_BaseEntity::Term()
{
	// inlined C_BaseEntity::RemoveFromAimEntsList() at line 1373
	// inlined C_BaseEntity::RemoveFromLeafSystem() at line 1371
	// inlined C_BaseEntity::DestroyModelInstance() at line 1368
	{
		int i;  // line 1332
		// inlined CPredictableList::RemoveFromPredictablesList() at line 1334
	}
	// inlined C_BaseEntity::GetClientHandle() at line 1349
	// inlined C_BaseEntity::GetClientHandle() at line 1353
}

// game/client/c_baseentity.cpp:1383 @0x2ba620 _ZN12C_BaseEntity13SetRefEHandleERK11CBaseHandle
void C_BaseEntity::SetRefEHandle( const CBaseHandle &handle )
{
}

// game/client/c_baseentity.cpp:1389 @0x2ba640 _ZNK12C_BaseEntity13GetRefEHandleEv
const CBaseHandle &C_BaseEntity::GetRefEHandle()
{
}

// game/client/c_baseentity.cpp:1397 @0x2c9780 _ZN12C_BaseEntity7ReleaseEv
void C_BaseEntity::Release()
{
	// inlined C_BaseEntity::DestroyIntermediateData() at line 1408
	{
		C_BaseAnimating::AutoAllowBoneAccess boneaccess;  // line 1400
	}
}

// game/client/c_baseentity.cpp:1421 @0x2ba660 _ZN12C_BaseEntity19CreateModelInstanceEv
void C_BaseEntity::CreateModelInstance()
{
}

// game/client/c_baseentity.cpp:1432 (declaration)
void DestroyModelInstance();

// game/client/c_baseentity.cpp:1432 @0x2ba6c0 _ZN12C_BaseEntity20DestroyModelInstanceEv
void C_BaseEntity::DestroyModelInstance()
{
}

// game/client/c_baseentity.cpp:1441 @0x2ba710 _ZN12C_BaseEntity14SetRemovalFlagEb
void C_BaseEntity::SetRemovalFlag( bool bRemove )
{
}

// game/client/c_baseentity.cpp:1453 (declaration)
void SetRenderAlpha( uint8 a );

// game/client/c_baseentity.cpp:1453 @0x2ba740 _ZN12C_BaseEntity14SetRenderAlphaEh
void C_BaseEntity::SetRenderAlpha( uint8 a )
{
	// inlined CNetworkColor32Base<color32_s,C_BaseEntity::NetworkVar_m_clrRender>::GetA() at line 1455
	// inlined CNetworkColor32Base<color32_s,C_BaseEntity::NetworkVar_m_clrRender>::SetA() at line 1457
}

// game/client/c_baseentity.cpp:1462 (declaration)
void GetRenderAlpha();

// game/client/c_baseentity.cpp:1462 @0x2ba7a0 _ZNK12C_BaseEntity14GetRenderAlphaEv
uint8 C_BaseEntity::GetRenderAlpha()
{
}

// game/client/c_baseentity.cpp:1471 @0x2ba7c0 _ZNK12C_BaseEntity14GetMinFadeDistEv
float C_BaseEntity::GetMinFadeDist()
{
}

// game/client/c_baseentity.cpp:1476 @0x2ba7d0 _ZNK12C_BaseEntity14GetMaxFadeDistEv
float C_BaseEntity::GetMaxFadeDist()
{
}

// game/client/c_baseentity.cpp:1481 @0x2ba7e0 _ZN12C_BaseEntity15SetDistanceFadeEff
void C_BaseEntity::SetDistanceFade( float flMinDist, float flMaxDist )
{
	// inlined C_BaseEntity::AlphaProp() at line 1493
}

// game/client/c_baseentity.cpp:1496 @0x2bead0 _ZN12C_BaseEntity18SetGlobalFadeScaleEf
void C_BaseEntity::SetGlobalFadeScale( float flFadeScale )
{
	int modelType;  // line 1499
	// inlined C_BaseEntity::AlphaProp() at line 1514
	{
		CMDLCacheCriticalSection cacheCriticalSection;  // line 1502
		MDLHandle_t hStudioHdr;  // line 1503
		// inlined CMDLCacheCriticalSection::CMDLCacheCriticalSection() at line 1502
		{
			const studiohdr_t *pStudioHdr;  // line 1506
		}
		// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 1512
		// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 1512
	}
}

// game/client/c_baseentity.cpp:1517 (declaration)
void GetGlobalFadeScale();

// game/client/c_baseentity.cpp:1517 @0x2ba830 _ZNK12C_BaseEntity18GetGlobalFadeScaleEv
float C_BaseEntity::GetGlobalFadeScale()
{
}

// game/client/c_baseentity.cpp:1526 @0x2bdfe0 _ZN12C_BaseEntity21VPhysicsGetObjectListEPP14IPhysicsObjecti
int C_BaseEntity::VPhysicsGetObjectList( IPhysicsObject **pList, int listMax )
{
	IPhysicsObject *pPhys;  // line 1528
	// inlined C_BaseEntity::VPhysicsGetObject() at line 1528
}

// game/client/c_baseentity.cpp:1542 @0x2ba840 _ZN12C_BaseEntity15VPhysicsIsFleshEv
bool C_BaseEntity::VPhysicsIsFlesh()
{
	IPhysicsObject *pList[1024];  // line 1544
	int count;  // line 1545
	{
		int i;  // line 1546
		{
			int material;  // line 1548
			const surfacedata_t *pSurfaceData;  // line 1549
		}
	}
}

// game/client/c_baseentity.cpp:1557 @0x2bdde0 _ZN12C_BaseEntity37VPhysicsCompensateForPredictionErrorsEPKh
void C_BaseEntity::VPhysicsCompensateForPredictionErrors( const uint8 *predicted_state_data )
{
	IPhysicsObject *pPhysicsObject;  // line 1579
	IPredictedPhysicsObject *pPredictedObject;  // line 1580
	// inlined C_BaseEntity::VPhysicsGetObject() at line 1579
	{
		Vector vPredictedOrigin;  // line 1583
		Vector vOriginDelta;  // line 1597
		Vector vPredictedVelocity;  // line 1601
		Vector vVelocityDelta;  // line 1608
		{
			const typedescription_t *tdOrigin;  // line 1586
		}
		// inlined Vector::operator-() at line 1597
		{
			const typedescription_t *tdVelocity;  // line 1603
		}
		// inlined Vector::operator-() at line 1608
	}
}

// game/client/c_baseentity.cpp:1637 @0x2ba900 _ZNK12C_BaseEntity14HealthFractionEv
float C_BaseEntity::HealthFraction()
{
	float flFraction;  // line 1642
}

// game/client/c_baseentity.cpp:1654 (declaration)
void GetVectors( Vector *pForward, Vector *pRight, Vector *pUp );

// game/client/c_baseentity.cpp:1654 @0x2c41f0 _ZNK12C_BaseEntity10GetVectorsEP6VectorS1_S1_
void C_BaseEntity::GetVectors( Vector *pForward, Vector *pRight, Vector *pUp )
{
	const matrix3x4_t &entityToWorld;  // line 1657
	// inlined Vector::operator*=() at line 1667
	// inlined C_BaseEntity::EntityToWorldTransform() at line 1657
}

// game/client/c_baseentity.cpp:1677 (declaration)
void UpdateVisibilityAllEntities();

// game/client/c_baseentity.cpp:1677 @0x2c2240 _ZN12C_BaseEntity27UpdateVisibilityAllEntitiesEv
void C_BaseEntity::UpdateVisibilityAllEntities()
{
	C_BaseEntityIterator iterator;  // line 1679
	C_BaseEntity *pEnt;  // line 1680
}

// game/client/c_baseentity.cpp:1688 @0x2c2640 _ZL19cl_updatevisibilityRK8CCommand
cl_updatevisibility( const CCommand &args )
{
	// inlined C_BaseEntity::UpdateVisibilityAllEntities() at line 1690
}

// game/client/c_baseentity.cpp:1688
static ConCommand cl_updatevisibility_command;

// game/client/c_baseentity.cpp:1693 (declaration)
void RenderWithViewModels( bool bEnable );

// game/client/c_baseentity.cpp:1693 @0x2ba9a0 _ZN12C_BaseEntity20RenderWithViewModelsEb
void C_BaseEntity::RenderWithViewModels( bool bEnable )
{
}

// game/client/c_baseentity.cpp:1699 (declaration)
void IsRenderingWithViewModels();

// game/client/c_baseentity.cpp:1699 @0x2baa00 _ZNK12C_BaseEntity25IsRenderingWithViewModelsEv
bool C_BaseEntity::IsRenderingWithViewModels()
{
}

// game/client/c_baseentity.cpp:1706 @0x2baa10 _ZN12C_BaseEntity25DisableCachedRenderBoundsEb
void C_BaseEntity::DisableCachedRenderBounds( bool bDisabled )
{
}

// game/client/c_baseentity.cpp:1712 @0x2baa70 _ZNK12C_BaseEntity28IsCachedRenderBoundsDisabledEv
bool C_BaseEntity::IsCachedRenderBoundsDisabled()
{
}

// game/client/c_baseentity.cpp:1717 @0x2c1db0 _ZN12C_BaseEntity16UpdateVisibilityEv
void C_BaseEntity::UpdateVisibility()
{
	uint32 nPreviousValue;  // line 1720
	int nBitsSet;  // line 1724
	// inlined C_BaseEntity::RemoveFromLeafSystem() at line 1756
	// inlined CBitVecT<CFixedBitVecBase<2> >::GetDWord() at line 1720
	// inlined CBitVecT<CFixedBitVecBase<2> >::ClearAll() at line 1723
	{
		int hh;  // line 1728
		{
			CSetActiveSplitScreenPlayerGuard g_SSGuard;  // line 1730
			bool bDraw;  // line 1731
			// inlined CBitVecT<CFixedBitVecBase<2> >::Set() at line 1734
		}
	}
	// inlined C_BaseEntity::OnSplitscreenRenderingChanged() at line 1744
	// inlined C_BaseEntity::AddToLeafSystem() at line 1751
}

// game/client/c_baseentity.h:1752 @0x2ccd90 _ZN12C_BaseEntity22NetworkVar_m_Collision19NetworkStateChangedEv
void C_BaseEntity::NetworkVar_m_Collision::NetworkStateChanged()
{
}

// game/client/c_baseentity.h:1752 @0x2ccfa0 _ZN12C_BaseEntity22NetworkVar_m_Collision19NetworkStateChangedEPv
void C_BaseEntity::NetworkVar_m_Collision::NetworkStateChanged( void *pVar )
{
}

// game/client/c_baseentity.h:1753 @0x2ccf90 _ZN12C_BaseEntity22NetworkVar_m_Particles19NetworkStateChangedEv
void C_BaseEntity::NetworkVar_m_Particles::NetworkStateChanged()
{
}

// game/client/c_baseentity.h:1753 @0x2ccfb0 _ZN12C_BaseEntity22NetworkVar_m_Particles19NetworkStateChangedEPv
void C_BaseEntity::NetworkVar_m_Particles::NetworkStateChanged( void *pVar )
{
}

// game/client/c_baseentity.cpp:1760 @0x2bc350 _ZN12C_BaseEntity28ShouldDrawForSplitScreenUserEi
bool C_BaseEntity::ShouldDrawForSplitScreenUser( int nSlot )
{
}

// game/client/c_baseentity.cpp:1768 @0x2bd2b0 _ZN12C_BaseEntity10ShouldDrawEv
bool C_BaseEntity::ShouldDraw()
{
	{
		CPULevel_t nCPULevel;  // line 1783
		bool bNoDraw;  // line 1784
		GPULevel_t nGPULevel;  // line 1789
	}
}

// game/client/c_baseentity.cpp:1799 @0x2baa80 _ZN12C_BaseEntity13TestCollisionERK5Ray_tjR10CGameTrace
bool C_BaseEntity::TestCollision( const Ray_t &ray, unsigned int mask, trace_t &trace )
{
}

// game/client/c_baseentity.cpp:1804 @0x2baa90 _ZN12C_BaseEntity12TestHitboxesERK5Ray_tjR10CGameTrace
bool C_BaseEntity::TestHitboxes( const Ray_t &ray, unsigned int fContentsMask, trace_t &tr )
{
}

// game/client/c_baseentity.cpp:1812 @0x2baaa0 _ZN12C_BaseEntity31ComputeWorldSpaceSurroundingBoxEP6VectorS1_
void C_BaseEntity::ComputeWorldSpaceSurroundingBox( Vector *pVecWorldMins, Vector *pVecWorldMaxs )
{
}

// game/client/c_baseentity.cpp:1825 @0x2c46d0 _ZN12C_BaseEntity14ReceiveMessageEiR7bf_read
void C_BaseEntity::ReceiveMessage( int classID, bf_read &msg )
{
	int messageType;  // line 1830
	// inlined CBitRead::ReadByte() at line 1830
	// inlined C_BaseEntity::RemoveAllDecals() at line 1833
}

// game/client/c_baseentity.cpp:1839 @0x2baac0 _ZN12C_BaseEntity19GetDataTableBasePtrEv
void *C_BaseEntity::GetDataTableBasePtr()
{
}

// game/client/c_baseentity.cpp:1848 @0x2bd7a0 _ZN12C_BaseEntity14ShadowCastTypeEv
ShadowType_t C_BaseEntity::ShadowCastType()
{
	int modelType;  // line 1853
}

// game/client/c_baseentity.cpp:1857 @0x2bdbd0 _ZN12C_BaseEntity29ShouldRenderInFastReflectionsEv
bool C_BaseEntity::ShouldRenderInFastReflections()
{
	// inlined C_BaseEntity::GetMoveParent() at line 1859
}

// game/client/c_baseentity.cpp:1866 @0x2baae0 _ZNK12C_BaseEntity21GetShadowCastDistanceEPf12ShadowType_t
bool C_BaseEntity::GetShadowCastDistance( float *pDistance, ShadowType_t shadowType )
{
}

// game/client/c_baseentity.cpp:1879 @0x2bc2f0 _ZNK12C_BaseEntity23GetShadowUseOtherEntityEv
C_BaseEntity *C_BaseEntity::GetShadowUseOtherEntity()
{
	// inlined CHandle<C_BaseEntity>::operator C_BaseEntity*() at line 1881
}

// game/client/c_baseentity.cpp:1887 @0x2be7b0 _ZN12C_BaseEntity23SetShadowUseOtherEntityEPS_
void C_BaseEntity::SetShadowUseOtherEntity( C_BaseEntity *pEntity )
{
	// inlined CHandle<C_BaseEntity>::operator=() at line 1889
}

// game/client/c_baseentity.cpp:1892 @0x2bab10 _ZN12C_BaseEntity23GetRotationInterpolatorEv
CInterpolatedVar<QAngle> &C_BaseEntity::GetRotationInterpolator()
{
}

// game/client/c_baseentity.cpp:1897 @0x2bab20 _ZN12C_BaseEntity21GetOriginInterpolatorEv
CInterpolatedVar<Vector> &C_BaseEntity::GetOriginInterpolator()
{
}

// game/client/c_baseentity.cpp:1905 @0x2be6e0 _ZNK12C_BaseEntity22GetShadowCastDirectionEP6Vector12ShadowType_t
bool C_BaseEntity::GetShadowCastDirection( Vector *pDirection, ShadowType_t shadowType )
{
	// inlined CHandle<C_BaseEntity>::operator C_BaseEntity*() at line 1907
}

// game/client/c_baseentity.cpp:1917 @0x2be800 _ZN12C_BaseEntity30ShouldReceiveProjectedTexturesEi
bool C_BaseEntity::ShouldReceiveProjectedTextures( int flags )
{
	// inlined C_BaseEntity::IsEffectActive() at line 1921
}

// game/client/c_baseentity.cpp:1947 @0x2bab40 _ZN12C_BaseEntity13IsShadowDirtyEv
bool C_BaseEntity::IsShadowDirty()
{
}

// game/client/c_baseentity.cpp:1952 @0x2bab70 _ZN12C_BaseEntity15MarkShadowDirtyEb
void C_BaseEntity::MarkShadowDirty( bool bDirty )
{
	// inlined C_BaseEntity::AddEFlags() at line 1956
	// inlined C_BaseEntity::RemoveEFlags() at line 1960
}

// game/client/c_baseentity.cpp:1964 @0x2bdb60 _ZN12C_BaseEntity15GetShadowParentEv
IClientRenderable *C_BaseEntity::GetShadowParent()
{
	C_BaseEntity *pParent;  // line 1966
	// inlined C_BaseEntity::GetMoveParent() at line 1966
}

// game/client/c_baseentity.cpp:1970 @0x2babb0 _ZN12C_BaseEntity16FirstShadowChildEv
IClientRenderable *C_BaseEntity::FirstShadowChild()
{
	C_BaseEntity *pChild;  // line 1972
	// inlined C_BaseEntity::FirstMoveChild() at line 1972
}

// game/client/c_baseentity.cpp:1976 @0x2bac20 _ZN12C_BaseEntity14NextShadowPeerEv
IClientRenderable *C_BaseEntity::NextShadowPeer()
{
	C_BaseEntity *pPeer;  // line 1978
	// inlined C_BaseEntity::NextMovePeer() at line 1978
}

// game/client/c_baseentity.cpp:1987 @0x2bac90 _ZNK12C_BaseEntity8entindexEv
int C_BaseEntity::entindex()
{
}

// game/client/c_baseentity.cpp:1992 @0x2baca0 _ZNK12C_BaseEntity19GetSoundSourceIndexEv
int C_BaseEntity::GetSoundSourceIndex()
{
	// inlined CBaseHandle::GetEntryIndex() at line 2000
}

// game/client/c_baseentity.cpp:2006 @0x2bace0 _ZN12C_BaseEntity15GetRenderOriginEv
const Vector &C_BaseEntity::GetRenderOrigin()
{
}

// game/client/c_baseentity.cpp:2011 @0x2bad00 _ZN12C_BaseEntity15GetRenderAnglesEv
const QAngle &C_BaseEntity::GetRenderAngles()
{
}

// game/client/c_baseentity.cpp:2016 @0x2c42b0 _ZN12C_BaseEntity26RenderableToWorldTransformEv
const matrix3x4_t &C_BaseEntity::RenderableToWorldTransform()
{
	// inlined C_BaseEntity::EntityToWorldTransform() at line 2018
}

// game/client/c_baseentity.cpp:2021 @0x2bad20 _ZN12C_BaseEntity21GetPVSNotifyInterfaceEv
IPVSNotify *C_BaseEntity::GetPVSNotifyInterface()
{
}

// game/client/c_baseentity.cpp:2031 @0x2c4470 _ZN12C_BaseEntity15GetRenderBoundsER6VectorS1_
void C_BaseEntity::GetRenderBounds( Vector &theMins, Vector &theMaxs )
{
	int nModelType;  // line 2033
	// inlined Vector::operator=() at line 2054
	// inlined Vector::operator=() at line 2054
	// inlined QAngle::operator==() at line 2042
	// inlined Vector::operator=() at line 2044
	// inlined Vector::operator=() at line 2045
	// inlined C_BaseEntity::EntityToWorldTransform() at line 2061
}

// game/client/c_baseentity.cpp:2067 @0x2bc510 _ZN12C_BaseEntity25GetRenderBoundsWorldspaceER6VectorS1_
void C_BaseEntity::GetRenderBoundsWorldspace( Vector &mins, Vector &maxs )
{
}

// game/client/c_baseentity.cpp:2073 @0x2bad40 _ZN12C_BaseEntity21GetShadowRenderBoundsER6VectorS1_12ShadowType_t
void C_BaseEntity::GetShadowRenderBounds( Vector &mins, Vector &maxs, ShadowType_t shadowType )
{
}

// game/client/c_baseentity.cpp:2085 @0x2c41d0 _ZNK12C_BaseEntity12GetAbsOriginEv
const Vector &C_BaseEntity::GetAbsOrigin()
{
}

// game/client/c_baseentity.cpp:2097 @0x2c41b0 _ZNK12C_BaseEntity12GetAbsAnglesEv
const QAngle &C_BaseEntity::GetAbsAngles()
{
}

// game/client/c_baseentity.cpp:2108 (declaration)
void SetNetworkOrigin( const Vector &org );

// game/client/c_baseentity.cpp:2108 @0x2bad80 _ZN12C_BaseEntity16SetNetworkOriginERK6Vector
void C_BaseEntity::SetNetworkOrigin( const Vector &org )
{
	// inlined Vector::operator=() at line 2110
}

// game/client/c_baseentity.cpp:2117 (declaration)
void SetNetworkAngles( const QAngle &ang );

// game/client/c_baseentity.cpp:2117 @0x2badb0 _ZN12C_BaseEntity16SetNetworkAnglesERK6QAngle
void C_BaseEntity::SetNetworkAngles( const QAngle &ang )
{
	// inlined QAngle::operator=() at line 2119
}

// game/client/c_baseentity.cpp:2126 (declaration)
void GetNetworkOrigin();

// game/client/c_baseentity.cpp:2126 @0x2bade0 _ZNK12C_BaseEntity16GetNetworkOriginEv
const Vector &C_BaseEntity::GetNetworkOrigin()
{
}

// game/client/c_baseentity.cpp:2136 (declaration)
void GetNetworkAngles();

// game/client/c_baseentity.cpp:2136 @0x2badf0 _ZNK12C_BaseEntity16GetNetworkAnglesEv
const QAngle &C_BaseEntity::GetNetworkAngles()
{
}

// game/client/c_baseentity.cpp:2146 @0x2bae10 _ZNK12C_BaseEntity8GetModelEv
const model_t *C_BaseEntity::GetModel()
{
}

// game/client/c_baseentity.cpp:2157 (declaration)
void GetModelIndex();

// game/client/c_baseentity.cpp:2157 @0x2bae20 _ZNK12C_BaseEntity13GetModelIndexEv
int C_BaseEntity::GetModelIndex()
{
}

// game/client/c_baseentity.cpp:2166 (declaration)
void SetModelIndex( int index );

// game/client/c_baseentity.cpp:2166 @0x2c2680 _ZN12C_BaseEntity13SetModelIndexEi
void C_BaseEntity::SetModelIndex( int index )
{
	const model_t *pModel;  // line 2169
	// inlined C_BaseEntity::SetModelPointer() at line 2170
}

// game/client/c_baseentity.cpp:2173 (declaration)
void SetModelPointer( const model_t *pModel );

// game/client/c_baseentity.cpp:2173 @0x2c21c0 _ZN12C_BaseEntity15SetModelPointerEPK7model_t
void C_BaseEntity::SetModelPointer( const model_t *pModel )
{
	// inlined C_BaseEntity::DestroyModelInstance() at line 2177
}

// game/client/c_baseentity.cpp:2189 (declaration)
void SetMoveType( MoveType_t val, MoveCollide_t moveCollide );

// game/client/c_baseentity.cpp:2189 @0x2bae30 _ZN12C_BaseEntity11SetMoveTypeE10MoveType_t13MoveCollide_t
void C_BaseEntity::SetMoveType( MoveType_t val, MoveCollide_t moveCollide )
{
	// inlined C_BaseEntity::SetMoveCollide() at line 2200
}

// game/client/c_baseentity.cpp:2203 (declaration)
void SetMoveCollide( MoveCollide_t val );

// game/client/c_baseentity.cpp:2203 @0x2bae50 _ZN12C_BaseEntity14SetMoveCollideE13MoveCollide_t
void C_BaseEntity::SetMoveCollide( MoveCollide_t val )
{
}

// game/client/c_baseentity.cpp:2212 @0x2bae70 _ZN12C_BaseEntity23ComputeTranslucencyTypeEv
RenderableTranslucencyType_t C_BaseEntity::ComputeTranslucencyType()
{
}

// game/client/c_baseentity.cpp:2223 (declaration)
void OnTranslucencyTypeChanged();

// game/client/c_baseentity.cpp:2223 @0x2baef0 _ZN12C_BaseEntity25OnTranslucencyTypeChangedEv
void C_BaseEntity::OnTranslucencyTypeChanged()
{
}

// game/client/c_baseentity.cpp:2235 (declaration)
void OnSplitscreenRenderingChanged();

// game/client/c_baseentity.cpp:2235 @0x2baf50 _ZN12C_BaseEntity29OnSplitscreenRenderingChangedEv
void C_BaseEntity::OnSplitscreenRenderingChanged()
{
	// inlined ComputeSplitscreenRenderingFlags() at line 2239
}

// game/client/c_baseentity.cpp:2244 @0x2bb000 _ZN12C_BaseEntity14GetRenderFlagsEv
int C_BaseEntity::GetRenderFlags()
{
}

// game/client/c_baseentity.cpp:2254 @0x2bb010 _ZN12C_BaseEntity8GetMouthEv
CMouthInfo *C_BaseEntity::GetMouth()
{
}

// game/client/c_baseentity.cpp:2265 @0x2bd620 _ZN12C_BaseEntity22GetSoundSpatializationER20SpatializationInfo_t
bool C_BaseEntity::GetSoundSpatialization( SpatializationInfo_t &info )
{
	const model_t *pModel;  // line 2280
	// inlined VectorCopy() at line 2306
	// inlined Vector::operator=() at line 2289
	{
		Vector mins;  // line 2294
		Vector maxs;  // line 2294
		Vector center;  // line 2294
		// inlined VectorAdd() at line 2297
		// inlined Vector::operator+=() at line 2300
	}
}

// game/client/c_baseentity.cpp:2317 @0x2bd5c0 _ZN12C_BaseEntity13GetAttachmentEiR6VectorR6QAngle
bool C_BaseEntity::GetAttachment( int number, Vector &origin, QAngle &angles )
{
	// inlined Vector::operator=() at line 2319
	// inlined QAngle::operator=() at line 2320
}

// game/client/c_baseentity.cpp:2324 @0x2bd580 _ZN12C_BaseEntity13GetAttachmentEiR6Vector
bool C_BaseEntity::GetAttachment( int number, Vector &origin )
{
	// inlined Vector::operator=() at line 2326
}

// game/client/c_baseentity.cpp:2330 @0x2c4380 _ZN12C_BaseEntity13GetAttachmentEiR11matrix3x4_t
bool C_BaseEntity::GetAttachment( int number, matrix3x4_t &matrix )
{
	// inlined C_BaseEntity::EntityToWorldTransform() at line 2332
}

// game/client/c_baseentity.cpp:2336 @0x2c3b70 _ZN12C_BaseEntity21GetAttachmentVelocityEiR6VectorR10Quaternion
bool C_BaseEntity::GetAttachmentVelocity( int number, Vector &originVel, Quaternion &angleVel )
{
	// inlined C_BaseEntity::GetAbsVelocity() at line 2338
	// inlined Vector::operator=() at line 2338
	// inlined Quaternion::Init() at line 2339
}

// game/client/c_baseentity.cpp:2348 @0x2bb030 _ZN12C_BaseEntity18GetRenderClipPlaneEv
float *C_BaseEntity::GetRenderClipPlane()
{
}

// game/client/c_baseentity.cpp:2360 @0x2ca380 _ZN12C_BaseEntity14DrawBrushModelEbbb
int C_BaseEntity::DrawBrushModel( bool bDrawingTranslucency, bool bShadowDepth, bool bTwoPass )
{
	CVProfScope VProf_;  // line 2362
	// inlined CVProfScope::CVProfScope() at line 2362
	{
		DrawBrushModelMode_t mode;  // line 2372
	}
	// inlined CVProfScope::~CVProfScope() at line 2380
	// inlined CVProfScope::~CVProfScope() at line 2380
}

// game/client/c_baseentity.cpp:2387 @0x2bc810 _ZN12C_BaseEntity9DrawModelEiRK20RenderableInstance_t
int C_BaseEntity::DrawModel( int flags, const RenderableInstance_t &instance )
{
	int drawn;  // line 2392
	int modelType;  // line 2398
}

// game/client/c_baseentity.cpp:2427 @0x2bb060 _ZN12C_BaseEntity10SetupBonesEP12matrix3x4a_tiif
bool C_BaseEntity::SetupBones( matrix3x4a_t *pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime )
{
}

// game/client/c_baseentity.cpp:2435 @0x2bb080 _ZN12C_BaseEntity12SetupWeightsEPK11matrix3x4_tiPfS3_
void C_BaseEntity::SetupWeights( const matrix3x4_t *pBoneToWorld, int nFlexWeightCount, float *pFlexWeights, float *pFlexDelayedWeights )
{
}

// game/client/c_baseentity.cpp:2443 @0x2bb0a0 _ZN12C_BaseEntity17DoAnimationEventsEv
void C_BaseEntity::DoAnimationEvents()
{
}

// game/client/c_baseentity.cpp:2448 @0x2bda60 _ZN12C_BaseEntity24UpdatePartitionListEntryEv
void C_BaseEntity::UpdatePartitionListEntry()
{
	CollideType_t shouldCollide;  // line 2451
	int list;  // line 2454
}

// game/client/c_baseentity.cpp:2470 @0x2c96b0 _ZN12C_BaseEntity20NotifyShouldTransmitE21ShouldTransmitState_t
void C_BaseEntity::NotifyShouldTransmit( ShouldTransmitState_t state )
{
}

// game/client/c_baseentity.cpp:2536 (declaration)
void MarkMessageReceived();

// game/client/c_baseentity.cpp:2536 @0x2bb0b0 _ZN12C_BaseEntity19MarkMessageReceivedEv
void C_BaseEntity::MarkMessageReceived()
{
}

// game/client/c_baseentity.cpp:2546 @0x2c2d20 _ZN12C_BaseEntity13PreDataUpdateE16DataUpdateType_t
void C_BaseEntity::PreDataUpdate( DataUpdateType_t updateType )
{
	bool bnewentity;  // line 2557
	// inlined ClientLeafSystem() at line 2590
	// inlined QAngle::operator=() at line 2581
	// inlined Vector::operator=() at line 2580
	// inlined C_BaseEntity::Interp_RestoreToLastNetworked() at line 2561
	{
		CMDLCacheCriticalSection cacheCriticalSection;  // line 2567
		// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 2568
		// inlined CMDLCacheCriticalSection::CMDLCacheCriticalSection() at line 2567
		// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 2568
	}
}

// game/client/c_baseentity.cpp:2594 @0x2bb0e0 _ZN12C_BaseEntity12GetOldOriginEv
const Vector &C_BaseEntity::GetOldOrigin()
{
}

// game/client/c_baseentity.cpp:2600 @0x2c92d0 _ZN12C_BaseEntity11UnlinkChildEPS_S0_
void C_BaseEntity::UnlinkChild( C_BaseEntity *pParent, C_BaseEntity *pChild )
{
	// inlined CHandle<C_BaseEntity>::operator==() at line 2610
	// inlined CHandle<C_BaseEntity>::operator C_BaseEntity*() at line 2617
	// inlined CHandle<C_BaseEntity>::operator C_BaseEntity*() at line 2621
	// inlined CHandle<C_BaseEntity>::operator=() at line 2626
	// inlined CHandle<C_BaseEntity>::operator=() at line 2627
	// inlined CHandle<C_BaseEntity>::operator=() at line 2628
	// inlined C_BaseEntity::RemoveFromAimEntsList() at line 2629
	// inlined C_BaseEntity::Interp_HierarchyUpdateInterpolationAmounts() at line 2631
}

// game/client/c_baseentity.cpp:2634 @0x2c90e0 _ZN12C_BaseEntity9LinkChildEPS_S0_
void C_BaseEntity::LinkChild( C_BaseEntity *pParent, C_BaseEntity *pChild )
{
	// inlined CHandle<C_BaseEntity>::operator=() at line 2650
	// inlined CHandle<C_BaseEntity>::operator C_BaseEntity*() at line 2652
	// inlined CHandle<C_BaseEntity>::operator=() at line 2656
	// inlined CHandle<C_BaseEntity>::operator=() at line 2657
	// inlined C_BaseEntity::AddToAimEntsList() at line 2658
	// inlined C_BaseEntity::Interp_HierarchyUpdateInterpolationAmounts() at line 2660
	// inlined CHandle<C_BaseEntity>::operator=() at line 2654
}

// game/client/c_baseentity.cpp:2663
CUtlVector<C_BaseEntity*,CUtlMemory<C_BaseEntity*, int> > g_AimEntsList;

// game/client/c_baseentity.cpp:2669 @0x2be180 _ZN12C_BaseEntity16MarkAimEntsDirtyEv
void C_BaseEntity::MarkAimEntsDirty()
{
	int i;  // line 2687
	int c;  // line 2688
	// inlined CUtlVector<C_BaseEntity*,CUtlMemory<C_BaseEntity*, int> >::Count() at line 2688
	{
		C_BaseEntity *pEnt;  // line 2691
		// inlined C_BaseEntity::AddEFlags() at line 2695
	}
}

// game/client/c_baseentity.cpp:2701 @0x2c49a0 _ZN12C_BaseEntity19CalcAimEntPositionsEv
void C_BaseEntity::CalcAimEntPositions()
{
	CVProfScope VProf_;  // line 2703
	int i;  // line 2704
	int c;  // line 2705
	// inlined CVProfScope::CVProfScope() at line 2703
	// inlined CUtlVector<C_BaseEntity*,CUtlMemory<C_BaseEntity*, int> >::Count() at line 2705
	{
		C_BaseEntity *pEnt;  // line 2708
	}
	// inlined CVProfScope::~CVProfScope() at line 2715
	// inlined CVProfScope::~CVProfScope() at line 2715
}

// game/client/c_baseentity.cpp:2719 (declaration)
void AddToAimEntsList();

// game/client/c_baseentity.cpp:2719 @0x2c0f40 _ZN12C_BaseEntity16AddToAimEntsListEv
void C_BaseEntity::AddToAimEntsList()
{
	// inlined CUtlVector<C_BaseEntity*,CUtlMemory<C_BaseEntity*, int> >::AddToTail() at line 2725
}

// game/client/c_baseentity.cpp:2728 (declaration)
void RemoveFromAimEntsList();

// game/client/c_baseentity.cpp:2728 @0x2bf3e0 _ZN12C_BaseEntity21RemoveFromAimEntsListEv
void C_BaseEntity::RemoveFromAimEntsList()
{
	unsigned int c;  // line 2736
	unsigned int last;  // line 2740
	{
		C_BaseEntity *lastEntity;  // line 2749
		// inlined CUtlVector<C_BaseEntity*,CUtlMemory<C_BaseEntity*, int> >::operator[]() at line 2749
		// inlined CUtlVector<C_BaseEntity*,CUtlMemory<C_BaseEntity*, int> >::FastRemove() at line 2751
	}
	// inlined CUtlVector<C_BaseEntity*,CUtlMemory<C_BaseEntity*, int> >::Count() at line 2736
	// inlined CUtlVector<C_BaseEntity*,CUtlMemory<C_BaseEntity*, int> >::FastRemove() at line 2745
}

// game/client/c_baseentity.cpp:2765 @0x2ca310 _ZN12C_BaseEntity25HierarchyUpdateMoveParentEv
void C_BaseEntity::HierarchyUpdateMoveParent()
{
	// inlined CBaseHandle::ToInt() at line 2767
	// inlined CHandle<C_BaseEntity>::operator C_BaseEntity*() at line 2770
}

// game/client/c_baseentity.cpp:2777 @0x2c9a40 _ZN12C_BaseEntity18HierarchySetParentEPS_
void C_BaseEntity::HierarchySetParent( C_BaseEntity *pNewParent )
{
	EHANDLE newParentHandle;  // line 2781
	C_BaseEntity *list[1024];  // line 2798
	int listReadIndex;  // line 2799
	int listWriteIndex;  // line 2800
	{
		C_BaseEntity *pParent;  // line 2805
		{
			C_BaseEntity *pChild;  // line 2808
			// inlined C_BaseEntity::FirstMoveChild() at line 2808
			// inlined C_BaseEntity::NextMovePeer() at line 2808
		}
	}
	// inlined CBaseHandle::ToInt() at line 2783
	// inlined CHandle<C_BaseEntity>::Set() at line 2782
	// inlined CHandle<C_BaseEntity>::operator C_BaseEntity*() at line 2788
}

// game/client/c_baseentity.cpp:2820 @0x2c9890 _ZN12C_BaseEntity9SetParentEPS_i
void C_BaseEntity::SetParent( C_BaseEntity *pParentEntity, int iParentAttachment )
{
	EHANDLE newParentHandle;  // line 2824
	Vector vecAbsOrigin;  // line 2830
	QAngle angAbsRotation;  // line 2831
	Vector vecAbsVelocity;  // line 2832
	// inlined Vector::Init() at line 2849
	// inlined QAngle::Init() at line 2848
	// inlined Vector::Init() at line 2847
	// inlined CHandle<C_BaseEntity>::operator C_BaseEntity*() at line 2837
	// inlined CBaseHandle::IsValid() at line 2835
	// inlined C_BaseEntity::GetAbsVelocity() at line 2832
	// inlined CHandle<C_BaseEntity>::Set() at line 2825
}

// game/client/c_baseentity.cpp:2861 @0x2c9530 _ZN12C_BaseEntity19UnlinkFromHierarchyEv
void C_BaseEntity::UnlinkFromHierarchy()
{
	C_BaseEntity *pChild;  // line 2875
	// inlined C_BaseEntity::FirstMoveChild() at line 2875
	// inlined CHandle<C_BaseEntity>::operator C_BaseEntity*() at line 2866
	// inlined CBaseHandle::IsValid() at line 2864
	// inlined CHandle<C_BaseEntity>::operator!=() at line 2878
	// inlined C_BaseEntity::FirstMoveChild() at line 2887
}

// game/client/c_baseentity.cpp:2895 @0x2c6090 _ZN12C_BaseEntity18ValidateModelIndexEv
void C_BaseEntity::ValidateModelIndex()
{
	// inlined C_BaseEntity::SetModelByIndex() at line 2897
}

// game/client/c_baseentity.cpp:2900 @0x2bdc40 _ZN12C_BaseEntity16IsParentChangingEv
bool C_BaseEntity::IsParentChanging()
{
}

// game/client/c_baseentity.cpp:2909 @0x2c9ba0 _ZN12C_BaseEntity14PostDataUpdateE16DataUpdateType_t
void C_BaseEntity::PostDataUpdate( DataUpdateType_t updateType )
{
	bool animTimeChanged;  // line 2939
	bool originChanged;  // line 2940
	bool anglesChanged;  // line 2941
	bool simTimeChanged;  // line 2942
	bool simulationChanged;  // line 2945
	bool bPredictable;  // line 2947
	// inlined C_BaseEntity::SetSolid() at line 2927
	// inlined C_BaseEntity::MoveToLastReceivedPosition() at line 2920
	// inlined Vector::operator!=() at line 2940
	// inlined QAngle::operator!=() at line 2941
	// inlined CHandle<C_BaseEntity>::operator C_BaseEntity*() at line 2973
	// inlined C_BaseEntity::MarkMessageReceived() at line 2975
	// inlined CHandle<C_BasePlayer>::operator C_BasePlayer*() at line 2996
	// inlined CHandle<C_BasePlayer>::operator C_BasePlayer*() at line 2999
	// inlined ToBasePlayer() at line 2999
	// inlined C_BaseEntity::Teleported() at line 3008
	// inlined C_BaseEntity::AddToEntityList() at line 3009
	// inlined CHandle<C_BaseEntity>::operator C_BaseEntity*() at line 3013
	// inlined CHandle<C_BaseEntity>::operator!=() at line 3013
}

// game/client/c_baseentity.cpp:3023 @0x2bd200 _ZN12C_BaseEntity20CheckInitPredictableEPKc
void C_BaseEntity::CheckInitPredictable( const char *context )
{
	C_BasePlayer *pOwner;  // line 3068
}

// game/client/c_baseentity.cpp:3080 @0x2bf570 _ZN12C_BaseEntity18GetPredictionOwnerEv
C_BasePlayer *C_BaseEntity::GetPredictionOwner()
{
	C_BasePlayer *pOwner;  // line 3082
	{
		C_BaseViewModel *vm;  // line 3088
		// inlined ToBasePlayer() at line 3091
		// inlined ToBaseViewModel() at line 3088
	}
	// inlined C_BaseEntity::GetOwnerEntity() at line 3085
	// inlined ToBasePlayer() at line 3082
	// inlined ToBasePlayer() at line 3085
}

// game/client/c_baseentity.cpp:3098 @0x2bb0f0 _ZN12C_BaseEntity15IsSelfAnimatingEv
bool C_BaseEntity::IsSelfAnimating()
{
}

// game/client/c_baseentity.cpp:3107 @0x2bb100 _ZNK12C_BaseEntity9GetEFlagsEv
int C_BaseEntity::GetEFlags()
{
}

// game/client/c_baseentity.cpp:3112 @0x2bb110 _ZN12C_BaseEntity9SetEFlagsEi
void C_BaseEntity::SetEFlags( int iEFlags )
{
}

// game/client/c_baseentity.cpp:3121 (declaration)
void SetModelByIndex( int nModelIndex );

// game/client/c_baseentity.cpp:3121 @0x2c2850 _ZN12C_BaseEntity15SetModelByIndexEi
void C_BaseEntity::SetModelByIndex( int nModelIndex )
{
	// inlined C_BaseEntity::SetModelIndex() at line 3123
}

// game/client/c_baseentity.cpp:3130 @0x2c5b10 _ZN12C_BaseEntity8SetModelEPKc
bool C_BaseEntity::SetModel( const char *pModelName )
{
	{
		int nModelIndex;  // line 3134
		// inlined C_BaseEntity::SetModelByIndex() at line 3135
	}
	// inlined C_BaseEntity::SetModelByIndex() at line 3140
}

// game/client/c_baseentity.cpp:3145 @0x2c5860 _ZN12C_BaseEntity25OnStoreLastNetworkedValueEv
void C_BaseEntity::OnStoreLastNetworkedValue()
{
	bool bRestore;  // line 3147
	Vector savePos;  // line 3148
	QAngle saveAng;  // line 3149
	int c;  // line 3162
	// inlined C_BaseEntity::SetLocalAngles() at line 3179
	// inlined C_BaseEntity::SetLocalOrigin() at line 3178
	{
		int i;  // line 3163
		{
			VarMapEntry_t *e;  // line 3165
			IInterpolatedVar *watcher;  // line 3166
			int type;  // line 3168
		}
	}
	// inlined CUtlVector<VarMapEntry_t,CUtlMemory<VarMapEntry_t, int> >::Count() at line 3162
	// inlined C_BaseEntity::MoveToLastReceivedPosition() at line 3159
	// inlined Vector::operator=() at line 3156
	// inlined QAngle::operator=() at line 3157
}

// game/client/c_baseentity.cpp:3189 @0x2c0bd0 _ZN12C_BaseEntity28OnLatchInterpolatedVariablesEi
void C_BaseEntity::OnLatchInterpolatedVariables( int flags )
{
	float changetime;  // line 3191
	bool bUpdateLastNetworkedValue;  // line 3193
	int c;  // line 3197
	// inlined C_BaseEntity::GetLastChangeTime() at line 3191
	// inlined CUtlVector<VarMapEntry_t,CUtlMemory<VarMapEntry_t, int> >::Count() at line 3197
	{
		int i;  // line 3198
		{
			VarMapEntry_t *e;  // line 3200
			IInterpolatedVar *watcher;  // line 3201
			int type;  // line 3203
			// inlined CUtlVector<VarMapEntry_t,CUtlMemory<VarMapEntry_t, int> >::operator[]() at line 3200
		}
	}
	// inlined C_BaseEntity::AddToEntityList() at line 3217
}

// game/client/c_baseentity.cpp:3221 @0x2c54b0 _ZN12C_BaseEntity20BaseInterpolatePart1ERfR6VectorR6QAngleRi
int C_BaseEntity::BaseInterpolatePart1( float &currentTime, Vector &oldOrigin, QAngle &oldAngles, int &bNoMoreChanges )
{
	// inlined C_BaseEntity::Interp_Interpolate() at line 3252
	// inlined QAngle::operator=() at line 3250
	// inlined Vector::operator=() at line 3249
	// inlined C_BaseEntity::IsFollowingEntity() at line 3228
	// inlined C_BaseEntity::MoveToLastReceivedPosition() at line 3231
	{
		int slot;  // line 3238
		C_BasePlayer *localplayer;  // line 3240
	}
}

// game/client/c_baseentity.cpp:3259 (declaration)
void BaseInterpolatePart2( Vector &oldOrigin, QAngle &oldAngles, int nChangeFlags );

// game/client/c_baseentity.cpp:3259 @0x2bcb40 _ZN12C_BaseEntity20BaseInterpolatePart2ER6VectorR6QAnglei
void C_BaseEntity::BaseInterpolatePart2( Vector &oldOrigin, QAngle &oldAngles, int nChangeFlags )
{
	// inlined Vector::operator!=() at line 3261
	// inlined QAngle::operator!=() at line 3266
}

// game/client/c_baseentity.cpp:3282 @0x2ca630 _ZN12C_BaseEntity11InterpolateEf
bool C_BaseEntity::Interpolate( float currentTime )
{
	CVProfScope VProf_;  // line 3284
	Vector oldOrigin;  // line 3286
	QAngle oldAngles;  // line 3287
	int bNoMoreChanges;  // line 3289
	int retVal;  // line 3290
	int nChangeFlags;  // line 3300
	// inlined CVProfScope::CVProfScope() at line 3284
	// inlined C_BaseEntity::RemoveFromEntityList() at line 3295
	// inlined C_BaseEntity::BaseInterpolatePart2() at line 3301
	// inlined CVProfScope::~CVProfScope() at line 3303
	// inlined CVProfScope::~CVProfScope() at line 3303
}

// game/client/c_baseentity.cpp:3306 @0x2c2c60 _ZN12C_BaseEntity10OnNewModelEv
CStudioHdr *C_BaseEntity::OnNewModel()
{
	// inlined C_BaseEntity::OnTranslucencyTypeChanged() at line 3308
}

// game/client/c_baseentity.cpp:3315 @0x2bb130 _ZN12C_BaseEntity19OnNewParticleEffectEPKcP18CNewParticleEffect
void C_BaseEntity::OnNewParticleEffect( const char *pszParticleName, CNewParticleEffect *pNewParticleEffect )
{
}

// game/client/c_baseentity.cpp:3320 @0x2bb140 _ZN12C_BaseEntity23OnParticleEffectDeletedEP18CNewParticleEffect
void C_BaseEntity::OnParticleEffectDeleted( CNewParticleEffect *pParticleEffect )
{
}

// game/client/c_baseentity.cpp:3334 (declaration)
void Teleported();

// game/client/c_baseentity.cpp:3334 @0x2bb150 _ZN12C_BaseEntity10TeleportedEv
bool C_BaseEntity::Teleported()
{
	// inlined CHandle<C_BaseEntity>::operator C_BaseEntity*() at line 3337
	// inlined CHandle<C_BaseEntity>::operator!=() at line 3337
}

// game/client/c_baseentity.cpp:3350 @0x2bb200 _ZN12C_BaseEntity10IsSubModelEv
bool C_BaseEntity::IsSubModel()
{
}

// game/client/c_baseentity.cpp:3365 @0x2c1740 _ZN12C_BaseEntity18CreateLightEffectsEv
bool C_BaseEntity::CreateLightEffects()
{
	dlight_t *dl;  // line 3367
	bool bHasLightEffects;  // line 3369
	// inlined Vector::operator=() at line 3387
	// inlined C_BaseEntity::IsViewEntity() at line 3371
	// inlined C_BaseEntity::IsEffectActive() at line 3373
	// inlined Vector::operator=() at line 3377
}

// game/client/c_baseentity.cpp:3396 (declaration)
void MoveToLastReceivedPosition( bool force );

// game/client/c_baseentity.cpp:3396 @0x2c1be0 _ZN12C_BaseEntity26MoveToLastReceivedPositionEb
void C_BaseEntity::MoveToLastReceivedPosition( bool force )
{
	// inlined C_BaseEntity::SetLocalOrigin() at line 3400
	// inlined C_BaseEntity::SetLocalAngles() at line 3401
}

// game/client/c_baseentity.cpp:3405 @0x2c1620 _ZN12C_BaseEntity17ShouldInterpolateEv
bool C_BaseEntity::ShouldInterpolate()
{
	C_BaseEntity *pChild;  // line 3421
	// inlined C_BaseEntity::NextMovePeer() at line 3427
	// inlined C_BaseEntity::IsViewEntity() at line 3407
	// inlined CBitVecT<CFixedBitVecBase<2> >::IsAllClear() at line 3414
	// inlined C_BaseEntity::FirstMoveChild() at line 3421
}

// game/client/c_baseentity.cpp:3435 @0x2c5c50 _ZN12C_BaseEntity19ProcessTeleportListEv
void C_BaseEntity::ProcessTeleportList()
{
	int iNext;  // line 3437
	{
		int iCur;  // line 3438
		{
			C_BaseEntity *pCur;  // line 3441
			bool teleport;  // line 3443
			bool ef_nointerp;  // line 3444
			// inlined CUtlLinkedList<C_BaseEntity*,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<C_BaseEntity*, short unsigned int>, short unsigned int> >::Next() at line 3440
			// inlined C_BaseEntity::Teleported() at line 3443
			// inlined C_BaseEntity::MoveToLastReceivedPosition() at line 3452
			// inlined C_BaseEntity::RemoveFromEntityList() at line 3458
		}
		// inlined CUtlLinkedList<C_BaseEntity*,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<C_BaseEntity*, short unsigned int>, short unsigned int> >::Head() at line 3438
	}
}

// game/client/c_baseentity.cpp:3463 (declaration)
void CheckInterpolatedVarParanoidMeasurement();

// game/client/c_baseentity.cpp:3463 @0x2bb270 _ZN12C_BaseEntity39CheckInterpolatedVarParanoidMeasurementEv
void C_BaseEntity::CheckInterpolatedVarParanoidMeasurement()
{
}

// game/client/c_baseentity.cpp:3501 (declaration)
void ProcessInterpolatedList();

// game/client/c_baseentity.cpp:3501 @0x2bb280 _ZN12C_BaseEntity23ProcessInterpolatedListEv
void C_BaseEntity::ProcessInterpolatedList()
{
	int iNext;  // line 3506
	{
		int iCur;  // line 3507
		// inlined CUtlLinkedList<C_BaseEntity*,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<C_BaseEntity*, short unsigned int>, short unsigned int> >::Head() at line 3507
		{
			C_BaseEntity *pCur;  // line 3510
			// inlined CUtlLinkedList<C_BaseEntity*,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<C_BaseEntity*, short unsigned int>, short unsigned int> >::Next() at line 3509
		}
	}
}

// game/client/c_baseentity.cpp:3520 @0x2bdaf0 _ZN12C_BaseEntity15GetAimEntOriginEP13IClientEntityP6VectorP6QAngle
void C_BaseEntity::GetAimEntOrigin( IClientEntity *pAttachedTo, Vector *pOrigin, QAngle *pAngles )
{
	// inlined Vector::operator=() at line 3525
	// inlined QAngle::operator=() at line 3526
}

// game/client/c_baseentity.cpp:3530 @0x2c99e0 _ZN12C_BaseEntity19StopFollowingEntityEv
void C_BaseEntity::StopFollowingEntity()
{
	// inlined C_BaseEntity::RemoveEffects() at line 3535
	// inlined C_BaseEntity::RemoveSolidFlags() at line 3536
	// inlined C_BaseEntity::SetMoveType() at line 3537
}

// game/client/c_baseentity.cpp:3540 (declaration)
void IsFollowingEntity();

// game/client/c_baseentity.cpp:3540 @0x2bb2f0 _ZN12C_BaseEntity17IsFollowingEntityEv
bool C_BaseEntity::IsFollowingEntity()
{
	// inlined C_BaseEntity::GetMoveParent() at line 3542
}

// game/client/c_baseentity.cpp:3545 @0x2bf650 _ZN12C_BaseEntity17GetFollowedEntityEv
C_BaseEntity *C_BaseEntity::GetFollowedEntity()
{
	// inlined C_BaseEntity::IsFollowingEntity() at line 3547
}

// game/client/c_baseentity.cpp:3556 @0x2bb360 _ZN12C_BaseEntity28GetTextureAnimationStartTimeEv
float C_BaseEntity::GetTextureAnimationStartTime()
{
}

// game/client/c_baseentity.cpp:3565 @0x2bb370 _ZN12C_BaseEntity23TextureAnimationWrappedEv
void C_BaseEntity::TextureAnimationWrapped()
{
}

// game/client/c_baseentity.cpp:3570 @0x2bb390 _ZN12C_BaseEntity11ClientThinkEv
void C_BaseEntity::ClientThink()
{
}

// game/client/c_baseentity.cpp:3575
static ConVar cl_interpolate;

// game/client/c_baseentity.cpp:3577 @0x2c8590 _ZN12C_BaseEntity25InterpolateServerEntitiesEv
void C_BaseEntity::InterpolateServerEntities()
{
	CVProfScope VProf_;  // line 3579
	bool bPrevInterpolate;  // line 3581
	INetChannelInfo *nci;  // line 3592
	CInterpolationContext context;  // line 3615
	// inlined CVProfScope::CVProfScope() at line 3579
	{
		C_BaseEntityIterator iterator;  // line 3606
		C_BaseEntity *pEnt;  // line 3607
		// inlined C_BaseEntity::Interp_UpdateInterpolationAmounts() at line 3610
	}
	// inlined CInterpolationContext::CInterpolationContext() at line 3615
	// inlined CInterpolationContext::SetLastTimeStamp() at line 3616
	// inlined C_BaseEntity::ProcessInterpolatedList() at line 3635
	// inlined CInterpolationContext::~CInterpolationContext() at line 3635
	// inlined CVProfScope::~CVProfScope() at line 3635
	// inlined CInterpolationContext::EnableExtrapolation() at line 3619
	{
		C_BaseEntityIterator iterator;  // line 3625
		C_BaseEntity *pEnt;  // line 3626
	}
	// inlined CVProfScope::~CVProfScope() at line 3635
	// inlined CInterpolationContext::~CInterpolationContext() at line 3635
}

// game/client/c_baseentity.cpp:3640 @0x2bb3a0 _ZN12C_BaseEntity18AddVisibleEntitiesEv
void C_BaseEntity::AddVisibleEntities()
{
}

// game/client/c_baseentity.cpp:3677 @0x2bb3c0 _ZN12C_BaseEntity16OnPreDataChangedE16DataUpdateType_t
void C_BaseEntity::OnPreDataChanged( DataUpdateType_t type )
{
}

// game/client/c_baseentity.cpp:3683 @0x2c2100 _ZN12C_BaseEntity13OnDataChangedE16DataUpdateType_t
void C_BaseEntity::OnDataChanged( DataUpdateType_t type )
{
	// inlined C_BaseEntity::AlphaProp() at line 3697
	// inlined C_BaseEntity::AlphaProp() at line 3698
	// inlined C_BaseEntity::AlphaProp() at line 3701
}

// game/client/c_baseentity.cpp:3704 @0x2bb400 _ZN12C_BaseEntity14GetThinkHandleEv
ClientThinkHandle_t C_BaseEntity::GetThinkHandle()
{
}

// game/client/c_baseentity.cpp:3710 @0x2bb420 _ZN12C_BaseEntity14SetThinkHandleEP21CClientThinkHandlePtr
void C_BaseEntity::SetThinkHandle( ClientThinkHandle_t hThink )
{
}

// game/client/c_baseentity.cpp:3719 @0x2bd530 _ZN12C_BaseEntity18GetColorModulationEPf
void C_BaseEntity::GetColorModulation( float *color )
{
}

// game/client/c_baseentity.cpp:3730 @0x2be0d0 _ZN12C_BaseEntity14GetCollideTypeEv
CollideType_t C_BaseEntity::GetCollideType()
{
	// inlined C_BaseEntity::IsSolid() at line 3735
}

// game/client/c_baseentity.cpp:3756 @0x2bb440 _ZNK12C_BaseEntity12IsBrushModelEv
bool C_BaseEntity::IsBrushModel()
{
	int modelType;  // line 3758
}

// game/client/c_baseentity.cpp:3767 @0x2bebf0 _ZN12C_BaseEntity14AddStudioDecalERK5Ray_tiibR10CGameTracei
void C_BaseEntity::AddStudioDecal( const Ray_t &ray, int hitbox, int decalIndex, bool doTrace, trace_t &tr, int maxLODToDecal )
{
	Vector up;  // line 3789
	// inlined Vector::Vector() at line 3789
	{
		Vector temp;  // line 3795
		Ray_t betterRay;  // line 3797
		// inlined VectorSubtract() at line 3796
		// inlined Ray_t::Init() at line 3798
	}
}

// game/client/c_baseentity.cpp:3812 @0x2bd0e0 _ZN12C_BaseEntity18AddBrushModelDecalERK5Ray_tRK6VectoribR10CGameTrace
void C_BaseEntity::AddBrushModelDecal( const Ray_t &ray, const Vector &decalCenter, int decalIndex, bool doTrace, trace_t &tr )
{
	Vector vecNormal;  // line 3814
	// inlined Vector::operator=() at line 3820
	// inlined Vector::operator=() at line 3824
	// inlined Vector::operator*=() at line 3826
}

// game/client/c_baseentity.cpp:3838 @0x2bee40 _ZN12C_BaseEntity8AddDecalERK6VectorS2_S2_iibR10CGameTracei
void C_BaseEntity::AddDecal( const Vector &rayStart, const Vector &rayEnd, const Vector &decalCenter, int hitbox, int decalIndex, bool doTrace, trace_t &tr, int maxLODToDecal )
{
	Ray_t ray;  // line 3840
	int modelType;  // line 3847
	// inlined Ray_t::Init() at line 3841
	// inlined Vector::operator*=() at line 3845
}

// game/client/c_baseentity.cpp:3868 (declaration)
void RemoveAllDecals();

// game/client/c_baseentity.cpp:3868 @0x2bb480 _ZN12C_BaseEntity15RemoveAllDecalsEv
void C_BaseEntity::RemoveAllDecals()
{
}

// game/client/c_baseentity.cpp:3878 @0x2c0050 _ZN12C_BaseEntity19SnatchModelInstanceEPS_
bool C_BaseEntity::SnatchModelInstance( C_BaseEntity *pToEntity )
{
	// inlined C_BaseEntity::DestroyModelInstance() at line 3885
	// inlined C_BaseEntity::SetModelInstance() at line 3888
	// inlined C_BaseEntity::SetModelInstance() at line 3891
}

// game/client/c_baseentity.cpp:3902 (declaration)
void operator new( size_t stAllocateBlock );

// game/client/c_baseentity.cpp:3902 @0x2bd000 _ZN12C_BaseEntitynwEm
void *C_BaseEntity::operator new( size_t stAllocateBlock )
{
	void *pMem;  // line 3906
}

// game/client/c_baseentity.cpp:3911 @0x2bcfb0 _ZN12C_BaseEntitynaEm
void *C_BaseEntity::operator new []( size_t stAllocateBlock )
{
	void *pMem;  // line 3915
}

// game/client/c_baseentity.cpp:3920 @0x2bcf50 _ZN12C_BaseEntitynwEmiPKci
void *C_BaseEntity::operator new( size_t stAllocateBlock, int nBlockUse, const char *pFileName, int nLine )
{
	void *pMem;  // line 3923
	// inlined MemAlloc_Alloc() at line 3923
}

// game/client/c_baseentity.cpp:3928 @0x2be750 _ZN12C_BaseEntitynaEmiPKci
void *C_BaseEntity::operator new []( size_t stAllocateBlock, int nBlockUse, const char *pFileName, int nLine )
{
	void *pMem;  // line 3931
	// inlined MemAlloc_Alloc() at line 3931
}

// game/client/c_baseentity.cpp:3941 (declaration)
void operator delete( void *pMem );

// game/client/c_baseentity.cpp:3941 @0x2bb4f0 _ZN12C_BaseEntitydlEPv
void C_BaseEntity::operator delete( void *pMem )
{
}

// game/client/c_baseentity.cpp:3958 @0x2bd0c0 _ZN12C_BaseEntity7GetTeamEv
C_Team *C_BaseEntity::GetTeam()
{
}

// game/client/c_baseentity.cpp:3967 @0x2bb520 _ZNK12C_BaseEntity13GetTeamNumberEv
int C_BaseEntity::GetTeamNumber()
{
}

// game/client/c_baseentity.cpp:3975 @0x2bb530 _ZN12C_BaseEntity19GetRenderTeamNumberEv
int C_BaseEntity::GetRenderTeamNumber()
{
}

// game/client/c_baseentity.cpp:3983 @0x2bb550 _ZN12C_BaseEntity10InSameTeamEPS_
bool C_BaseEntity::InSameTeam( C_BaseEntity *pEntity )
{
}

// game/client/c_baseentity.cpp:3994 @0x2bd090 _ZN12C_BaseEntity11InLocalTeamEv
bool C_BaseEntity::InLocalTeam()
{
}

// game/client/c_baseentity.cpp:4000 @0x2bd050 _ZN12C_BaseEntity18SetNextClientThinkEf
void C_BaseEntity::SetNextClientThink( float nextThinkTime )
{
	// inlined C_BaseEntity::GetClientHandle() at line 4003
}

// game/client/c_baseentity.cpp:4006 (declaration)
void AddToLeafSystem();

// game/client/c_baseentity.cpp:4006 @0x2bb720 _ZN12C_BaseEntity15AddToLeafSystemEv
void C_BaseEntity::AddToLeafSystem()
{
}

// game/client/c_baseentity.cpp:4011 @0x2bb5a0 _ZN12C_BaseEntity15AddToLeafSystemEb
void C_BaseEntity::AddToLeafSystem( bool bRenderWithViewModels )
{
	// inlined ClientLeafSystem() at line 4024
	// inlined ClientLeafSystem() at line 4027
	// inlined ClientLeafSystem() at line 4028
	// inlined ClientLeafSystem() at line 4017
	// inlined ComputeSplitscreenRenderingFlags() at line 4017
	// inlined ClientLeafSystem() at line 4018
	// inlined ClientLeafSystem() at line 4019
}

// game/client/c_baseentity.cpp:4036 @0x2bf720 _ZN12C_BaseEntity12CreateShadowEv
void C_BaseEntity::CreateShadow()
{
	CBitVec<2> bvPrevBits;  // line 4038
	ShadowType_t typeSeen;  // line 4042
	ShadowType_t shadowType[2];  // line 4043
	{
		int flags;  // line 4073
		// inlined C_BaseEntity::GetClientHandle() at line 4080
	}
	// inlined CBitVecT<CFixedBitVecBase<2> >::Copy() at line 4039
	// inlined CBitVecT<CFixedBitVecBase<2> >::ClearAll() at line 4040
	{
		int hh;  // line 4044
		{
			CSetActiveSplitScreenPlayerGuard g_SSGuard;  // line 4046
			// inlined CBitVecT<CFixedBitVecBase<2> >::Set() at line 4050
		}
	}
	// inlined CBitVecT<CFixedBitVecBase<2> >::IsAllClear() at line 4062
	// inlined C_BaseEntity::DestroyShadow() at line 4065
	// inlined CBitVecT<CFixedBitVecBase<2> >::IsAllClear() at line 4068
	// inlined CBitVecT<CFixedBitVecBase<2> >::Compare() at line 4062
}

// game/client/c_baseentity.cpp:4092 (declaration)
void DestroyShadow();

// game/client/c_baseentity.cpp:4092 @0x2bb740 _ZN12C_BaseEntity13DestroyShadowEv
void C_BaseEntity::DestroyShadow()
{
}

// game/client/c_baseentity.cpp:4107 (declaration)
void RemoveFromLeafSystem();

// game/client/c_baseentity.cpp:4107 @0x2bf960 _ZN12C_BaseEntity20RemoveFromLeafSystemEv
void C_BaseEntity::RemoveFromLeafSystem()
{
	// inlined ClientLeafSystem() at line 4112
	// inlined C_BaseEntity::DestroyShadow() at line 4115
}

// game/client/c_baseentity.cpp:4125 @0x2c2070 _ZN12C_BaseEntity10SetDormantEb
void C_BaseEntity::SetDormant( bool bDormant )
{
}

// game/client/c_baseentity.cpp:4144 @0x2bb7a0 _ZN12C_BaseEntity9IsDormantEv
bool C_BaseEntity::IsDormant()
{
}

// game/client/c_baseentity.cpp:4158 @0x2be030 _ZN12C_BaseEntity30SetDestroyedOnRecreateEntitiesEv
void C_BaseEntity::SetDestroyedOnRecreateEntities()
{
}

// game/client/c_baseentity.cpp:4169 @0x2c3fd0 _ZN12C_BaseEntity12SetAbsOriginERK6Vector
void C_BaseEntity::SetAbsOrigin( const Vector &absOrigin )
{
	C_BaseEntity *pMoveParent;  // line 4184
	// inlined Vector::operator=() at line 4188
	// inlined C_BaseEntity::GetMoveParent() at line 4184
	// inlined Vector::operator=() at line 4181
	// inlined C_BaseEntity::RemoveEFlags() at line 4179
	// inlined Vector::operator==() at line 4174
	// inlined C_BaseEntity::EntityToWorldTransform() at line 4193
	// inlined VectorITransform() at line 4193
}

// game/client/c_baseentity.cpp:4196 @0x2c3df0 _ZN12C_BaseEntity12SetAbsAnglesERK6QAngle
void C_BaseEntity::SetAbsAngles( const QAngle &absAngles )
{
	C_BaseEntity *pMoveParent;  // line 4215
	// inlined QAngle::operator==() at line 4205
	// inlined C_BaseEntity::RemoveEFlags() at line 4209
	// inlined QAngle::operator=() at line 4211
	// inlined C_BaseEntity::GetMoveParent() at line 4215
	// inlined QAngle::operator=() at line 4219
	// inlined QAngle::operator==() at line 4224
	// inlined QAngle::Init() at line 4226
	{
		matrix3x4_t worldToParent;  // line 4231
		matrix3x4_t localMatrix;  // line 4231
		// inlined C_BaseEntity::EntityToWorldTransform() at line 4232
		// inlined MatrixAngles() at line 4234
	}
}

// game/client/c_baseentity.cpp:4238 @0x2c3ca0 _ZN12C_BaseEntity14SetAbsVelocityERK6Vector
void C_BaseEntity::SetAbsVelocity( const Vector &vecAbsVelocity )
{
	C_BaseEntity *pMoveParent;  // line 4249
	Vector relVelocity;  // line 4259
	// inlined Vector::operator=() at line 4253
	// inlined C_BaseEntity::GetMoveParent() at line 4249
	// inlined Vector::operator=() at line 4247
	// inlined Vector::operator==() at line 4240
	// inlined C_BaseEntity::GetAbsVelocity() at line 4260
	// inlined VectorSubtract() at line 4260
	// inlined C_BaseEntity::EntityToWorldTransform() at line 4263
	// inlined VectorIRotate() at line 4263
}

// game/client/c_baseentity.cpp:4300 (declaration)
void GetLocalOrigin();

// game/client/c_baseentity.cpp:4300 @0x2bb7c0 _ZNK12C_BaseEntity14GetLocalOriginEv
const Vector &C_BaseEntity::GetLocalOrigin()
{
}

// game/client/c_baseentity.cpp:4305 @0x2bb7d0 _ZNK12C_BaseEntity17GetLocalOriginDimEi
vec_t C_BaseEntity::GetLocalOriginDim( int iDim )
{
}

// game/client/c_baseentity.cpp:4311 (declaration)
void SetLocalOrigin( const Vector &origin );

// game/client/c_baseentity.cpp:4311 @0x2bcab0 _ZN12C_BaseEntity14SetLocalOriginERK6Vector
void C_BaseEntity::SetLocalOrigin( const Vector &origin )
{
	// inlined Vector::operator!=() at line 4313
	// inlined Vector::operator=() at line 4316
}

// game/client/c_baseentity.cpp:4320 @0x2bdf50 _ZN12C_BaseEntity17SetLocalOriginDimEif
void C_BaseEntity::SetLocalOriginDim( int iDim, vec_t flValue )
{
	// inlined Vector::operator[]() at line 4322
}

// game/client/c_baseentity.cpp:4331 (declaration)
void GetLocalAngles();

// game/client/c_baseentity.cpp:4331 @0x2bb7f0 _ZNK12C_BaseEntity14GetLocalAnglesEv
const QAngle &C_BaseEntity::GetLocalAngles()
{
}

// game/client/c_baseentity.cpp:4336 @0x2bb800 _ZNK12C_BaseEntity17GetLocalAnglesDimEi
vec_t C_BaseEntity::GetLocalAnglesDim( int iDim )
{
}

// game/client/c_baseentity.cpp:4342 (declaration)
void SetLocalAngles( const QAngle &angles );

// game/client/c_baseentity.cpp:4342 @0x2bca20 _ZN12C_BaseEntity14SetLocalAnglesERK6QAngle
void C_BaseEntity::SetLocalAngles( const QAngle &angles )
{
	// inlined QAngle::operator!=() at line 4352
	// inlined QAngle::operator=() at line 4356
}

// game/client/c_baseentity.cpp:4360 @0x2be050 _ZN12C_BaseEntity17SetLocalAnglesDimEif
void C_BaseEntity::SetLocalAnglesDim( int iDim, vec_t flValue )
{
	// inlined QAngle::operator[]() at line 4363
}

// game/client/c_baseentity.cpp:4371 (declaration)
void SetLocalVelocity( const Vector &vecVelocity );

// game/client/c_baseentity.cpp:4371 @0x2bc990 _ZN12C_BaseEntity16SetLocalVelocityERK6Vector
void C_BaseEntity::SetLocalVelocity( const Vector &vecVelocity )
{
	// inlined Vector::operator!=() at line 4373
	// inlined Vector::operator=() at line 4376
}

// game/client/c_baseentity.cpp:4380 @0x2be650 _ZN12C_BaseEntity23SetLocalAngularVelocityERK6QAngle
void C_BaseEntity::SetLocalAngularVelocity( const QAngle &vecAngVelocity )
{
	// inlined QAngle::operator!=() at line 4382
	// inlined QAngle::operator=() at line 4385
}

// game/client/c_baseentity.cpp:4390 @0x2c03c0 _ZN12C_BaseEntity8TeleportEPK6VectorPK6QAngleS2_
void C_BaseEntity::Teleport( const Vector *newPosition, const QAngle *newAngles, const Vector *newVelocity )
{
	int iEffects;  // line 4394
	// inlined C_BaseEntity::SetLocalVelocity() at line 4407
	// inlined C_BaseEntity::SetNetworkAngles() at line 4402
	// inlined C_BaseEntity::SetNetworkOrigin() at line 4397
	// inlined C_BaseEntity::GetEffects() at line 4394
}

// game/client/c_baseentity.cpp:4417 @0x2c1ab0 _ZN12C_BaseEntity17SetLocalTransformERK11matrix3x4_t
void C_BaseEntity::SetLocalTransform( const matrix3x4_t &localTransform )
{
	Vector vecLocalOrigin;  // line 4419
	QAngle vecLocalAngles;  // line 4420
	// inlined C_BaseEntity::SetLocalAngles() at line 4424
	// inlined C_BaseEntity::SetLocalOrigin() at line 4423
	// inlined MatrixAngles() at line 4422
}

// game/client/c_baseentity.cpp:4431 (declaration)
void MoveToAimEnt();

// game/client/c_baseentity.cpp:4431 @0x2c4110 _ZN12C_BaseEntity12MoveToAimEntEv
void C_BaseEntity::MoveToAimEnt()
{
	Vector vecAimEntOrigin;  // line 4433
	QAngle vecAimEntAngles;  // line 4434
	// inlined C_BaseEntity::GetMoveParent() at line 4435
}

// game/client/c_baseentity.cpp:4441 @0x2bf1a0 _ZNK12C_BaseEntity22BoneMergeFastCullBloatER6VectorS1_RKS0_S3_
void C_BaseEntity::BoneMergeFastCullBloat( Vector &localMins, Vector &localMaxs, const Vector &thisEntityMins, const Vector &thisEntityMaxs )
{
	float flExpand;  // line 4445
	// inlined Vector::operator-() at line 4445
	// inlined Vector::Length() at line 4445
}

// game/client/c_baseentity.cpp:4457 (declaration)
void GetParentToWorldTransform( matrix3x4_t &tempMatrix );

// game/client/c_baseentity.cpp:4457 @0x2c3bd0 _ZN12C_BaseEntity25GetParentToWorldTransformER11matrix3x4_t
matrix3x4_t &C_BaseEntity::GetParentToWorldTransform( matrix3x4_t &tempMatrix )
{
	C_BaseEntity *pMoveParent;  // line 4459
	// inlined C_BaseEntity::GetMoveParent() at line 4459
	{
		Vector vOrigin;  // line 4469
		QAngle vAngles;  // line 4470
	}
	// inlined C_BaseEntity::EntityToWorldTransform() at line 4479
}

// game/client/c_baseentity.cpp:4487 @0x2c34c0 _ZN12C_BaseEntity20CalcAbsolutePositionEv
void C_BaseEntity::CalcAbsolutePosition()
{
	CAutoLockT<CThreadFastMutex> generated_id_4504;  // line 4504
	matrix3x4_t matEntityToParent;  // line 4532
	matrix3x4_t scratchMatrix;  // line 4537
	// inlined CAutoLockT<CThreadFastMutex>::CAutoLockT() at line 4504
	// inlined C_BaseEntity::RemoveEFlags() at line 4511
	// inlined CHandle<C_BaseEntity>::operator!() at line 4513
	// inlined Vector::operator=() at line 4518
	// inlined QAngle::operator=() at line 4519
	// inlined CAutoLockT<CThreadFastMutex>::~CAutoLockT() at line 4563
	// inlined C_BaseEntity::MoveToAimEnt() at line 4526
	// inlined C_BaseEntity::GetParentToWorldTransform() at line 4538
	// inlined QAngle::operator==() at line 4544
	// inlined CHandle<C_BaseEntity>::operator->() at line 4547
	// inlined VectorCopy() at line 4547
	// inlined CHandle<C_BaseEntity>::operator->() at line 4560
	// inlined CAutoLockT<CThreadFastMutex>::~CAutoLockT() at line 4563
	// inlined MatrixAngles() at line 4551
	// inlined CAutoLockT<CThreadFastMutex>::~CAutoLockT() at line 4563
}

// game/client/c_baseentity.cpp:4566 @0x2c3960 _ZN12C_BaseEntity20CalcAbsoluteVelocityEv
void C_BaseEntity::CalcAbsoluteVelocity()
{
	CAutoLockT<CThreadFastMutex> generated_id_4571;  // line 4571
	C_BaseEntity *pMoveParent;  // line 4580
	// inlined CAutoLockT<CThreadFastMutex>::CAutoLockT() at line 4571
	// inlined C_BaseEntity::GetMoveParent() at line 4580
	// inlined Vector::operator=() at line 4583
	// inlined CAutoLockT<CThreadFastMutex>::~CAutoLockT() at line 4603
	// inlined C_BaseEntity::EntityToWorldTransform() at line 4587
	// inlined VectorRotate() at line 4587
	{
		Vector vOriginVel;  // line 4593
		Quaternion vAngleVel;  // line 4594
		// inlined Vector::operator+=() at line 4597
	}
	// inlined C_BaseEntity::GetAbsVelocity() at line 4603
	// inlined Vector::operator+=() at line 4603
	// inlined CAutoLockT<CThreadFastMutex>::~CAutoLockT() at line 4603
}

// game/client/c_baseentity.cpp:4635 @0x2c43c0 _ZN12C_BaseEntity18ComputeAbsPositionERK6VectorPS0_
void C_BaseEntity::ComputeAbsPosition( const Vector &vecLocalPosition, Vector *pAbsPosition )
{
	C_BaseEntity *pMoveParent;  // line 4637
	// inlined VectorTransform() at line 4644
	// inlined C_BaseEntity::EntityToWorldTransform() at line 4644
	// inlined C_BaseEntity::GetMoveParent() at line 4637
	// inlined Vector::operator=() at line 4640
}

// game/client/c_baseentity.cpp:4652 @0x2c42d0 _ZN12C_BaseEntity19ComputeAbsDirectionERK6VectorPS0_
void C_BaseEntity::ComputeAbsDirection( const Vector &vecLocalDirection, Vector *pAbsDirection )
{
	C_BaseEntity *pMoveParent;  // line 4654
	// inlined VectorRotate() at line 4661
	// inlined C_BaseEntity::EntityToWorldTransform() at line 4661
	// inlined C_BaseEntity::GetMoveParent() at line 4654
	// inlined Vector::operator=() at line 4657
}

// game/client/c_baseentity.cpp:4670 @0x2be2d0 _ZN12C_BaseEntity21MarkRenderHandleDirtyEv
void C_BaseEntity::MarkRenderHandleDirty()
{
	ClientRenderHandle_t handle;  // line 4673
	// inlined C_BaseEntity::GetRenderHandle() at line 4673
	// inlined ClientLeafSystem() at line 4676
}

// game/client/c_baseentity.cpp:4684 @0x2c5f20 _ZN12C_BaseEntity19ShutdownPredictableEv
void C_BaseEntity::ShutdownPredictable()
{
	// inlined C_BaseEntity::DestroyIntermediateData() at line 4693
	{
		int i;  // line 4689
		// inlined CPredictableList::RemoveFromPredictablesList() at line 4691
	}
}

// game/client/c_baseentity.cpp:4701 @0x2c5380 _ZN12C_BaseEntity15InitPredictableEP12C_BasePlayer
void C_BaseEntity::InitPredictable( C_BasePlayer *pOwner )
{
	int slot;  // line 4710
	CSetActiveSplitScreenPlayerGuard g_SSGuard;  // line 4711
	{
		int i;  // line 4725
	}
	// inlined CPredictableList::AddToPredictableList() at line 4716
}

// game/client/c_baseentity.cpp:4736 @0x2c6340 _ZN12C_BaseEntity14SetPredictableEb
void C_BaseEntity::SetPredictable( bool state )
{
	// inlined C_BaseEntity::Interp_UpdateInterpolationAmounts() at line 4741
}

// game/client/c_baseentity.cpp:4748 (declaration)
void GetPredictable();

// game/client/c_baseentity.cpp:4748 @0x2bb820 _ZNK12C_BaseEntity14GetPredictableEv
bool C_BaseEntity::GetPredictable()
{
}

// game/client/c_baseentity.cpp:4758 @0x2c4e70 _ZN12C_BaseEntity23PreEntityPacketReceivedEi
void C_BaseEntity::PreEntityPacketReceived( int commands_acknowledged )
{
	bool copyintermediate;  // line 4762
}

// game/client/c_baseentity.cpp:4793 @0x2c5340 _ZN12C_BaseEntity24PostEntityPacketReceivedEv
void C_BaseEntity::PostEntityPacketReceived()
{
}

// game/client/c_baseentity.cpp:4809 @0x2c51c0 _ZN12C_BaseEntity23PostNetworkDataReceivedEi
bool C_BaseEntity::PostNetworkDataReceived( int commands_acknowledged )
{
	bool haderrors;  // line 4811
	bool errorcheck;  // line 4815
	bool showthis;  // line 4821
	// inlined ConVar::GetInt() at line 4821
	{
		uint8 *predicted_state_data;  // line 4837
		const uint8 *original_state_data;  // line 4839
		CPredictionCopy errorCheckHelper;  // line 4842
		// inlined CPredictionCopy::~CPredictionCopy() at line 4848
		// inlined C_BaseEntity::GetPredictedFrame() at line 4837
		// inlined CPredictionCopy::~CPredictionCopy() at line 4848
	}
}

// game/client/c_baseentity.cpp:4855 @0x2bcf10 _ZN12C_BaseEntity7SetSizeERK6VectorS2_
void C_BaseEntity::SetSize( const Vector &vecMin, const Vector &vecMax )
{
	// inlined C_BaseEntity::SetCollisionBounds() at line 4857
}

// game/client/c_baseentity.cpp:4865 @0x2bb830 _ZN12C_BaseEntity13PrecacheModelEPKc
int C_BaseEntity::PrecacheModel( const char *name )
{
}

// game/client/c_baseentity.cpp:4874 @0x2c0970 _ZN12C_BaseEntity6RemoveEv
void C_BaseEntity::Remove()
{
	// inlined C_BaseEntity::IsMarkedForDeletion() at line 4876
	// inlined C_BaseEntity::AddEFlags() at line 4878
	// inlined C_BaseEntity::AddSolidFlags() at line 4884
	// inlined C_BaseEntity::SetMoveType() at line 4885
	// inlined C_BaseEntity::AddToEntityList() at line 4890
}

// game/client/c_baseentity.cpp:4902 (declaration)
void GetPredictionEligible();

// game/client/c_baseentity.cpp:4902 @0x2bb860 _ZNK12C_BaseEntity21GetPredictionEligibleEv
bool C_BaseEntity::GetPredictionEligible()
{
}

// game/client/c_baseentity.cpp:4912 @0x2bdfa0 _ZN12C_BaseEntity8InstanceE11CBaseHandle
C_BaseEntity *C_BaseEntity::Instance( CBaseHandle &hEnt )
{
	// inlined CBaseHandle::CBaseHandle() at line 4914
}

// game/client/c_baseentity.cpp:4923 @0x2bddb0 _ZN12C_BaseEntity8InstanceEi
C_BaseEntity *C_BaseEntity::Instance( int iEnt )
{
}

// game/client/c_baseentity.cpp:4938 @0x2bcd40 _ZN12C_BaseEntity12GetClassnameEv
const char *C_BaseEntity::GetClassname()
{
	bool gotname;  // line 4942
	{
		const char *mapname;  // line 4946
	}
	char outstr[256];  // line 4940
}

// game/client/c_baseentity.cpp:4963 (declaration)
void GetDebugName();

// game/client/c_baseentity.cpp:4963 @0x2bcf00 _ZN12C_BaseEntity12GetDebugNameEv
const char *C_BaseEntity::GetDebugName()
{
}

// game/client/c_baseentity.cpp:4973 @0x2bcce0 _Z18CreateEntityByNamePKc
C_BaseEntity *CreateEntityByName( const char *className )
{
	C_BaseEntity *ent;  // line 4975
}

// game/client/c_baseentity.cpp:5000 @0x2be310 _ZL12dlight_debugRK8CCommand
dlight_debug( const CCommand &args )
{
	dlight_t *el;  // line 5002
	C_BasePlayer *player;  // line 5003
	Vector start;  // line 5006
	Vector forward;  // line 5007
	Vector end;  // line 5009
	trace_t tr;  // line 5010
	// inlined Vector::operator=() at line 5012
	// inlined Vector::operator-() at line 5012
	// inlined Vector::operator*() at line 5012
	// inlined UTIL_TraceLine() at line 5011
	// inlined Vector::operator+() at line 5009
	// inlined Vector::operator*() at line 5009
	// inlined Vector::operator VectorByValue&() at line 5006
}

// game/client/c_baseentity.cpp:5000
static ConCommand dlight_debug_command;

// game/client/c_baseentity.cpp:5026 @0x2bb870 _ZNK12C_BaseEntity15IsClientCreatedEv
bool C_BaseEntity::IsClientCreated()
{
}

// game/client/c_baseentity.cpp:5046 @0x2bb880 _ZN12C_BaseEntity27CreatePredictedEntityByNameEPKcS1_ib
C_BaseEntity *C_BaseEntity::CreatePredictedEntityByName( const char *classname, const char *module, int line, bool persist )
{
}

// game/client/c_baseentity.cpp:5143 @0x2bb890 _ZN12C_BaseEntity23OnPredictedEntityRemoveEbPS_
bool C_BaseEntity::OnPredictedEntityRemove( bool isbeingremoved, C_BaseEntity *predicted )
{
}

// game/client/c_baseentity.cpp:5188 @0x2c0380 _ZN12C_BaseEntity14SetOwnerEntityEPS_
void C_BaseEntity::SetOwnerEntity( C_BaseEntity *pOwner )
{
	// inlined CHandle<C_BaseEntity>::operator=() at line 5190
}

// game/client/c_baseentity.cpp:5196 @0x2bb8a0 _ZN12C_BaseEntity10ChangeTeamEi
void C_BaseEntity::ChangeTeam( int iTeamNum )
{
}

// game/client/c_baseentity.cpp:5205 @0x2bb8c0 _ZN12C_BaseEntity12SetModelNameEPKc
void C_BaseEntity::SetModelName( string_t name )
{
}

// game/client/c_baseentity.cpp:5214 @0x2bb8e0 _ZNK12C_BaseEntity12GetModelNameEv
string_t C_BaseEntity::GetModelName()
{
}

// game/client/c_baseentity.cpp:5222 @0x2c9660 _ZN12C_BaseEntity14UpdateOnRemoveEv
void C_BaseEntity::UpdateOnRemove()
{
}

// game/client/c_baseentity.cpp:5235 (declaration)
void SetPredictionEligible( bool canpredict );

// game/client/c_baseentity.cpp:5235 @0x2bb8f0 _ZN12C_BaseEntity21SetPredictionEligibleEb
void C_BaseEntity::SetPredictionEligible( bool canpredict )
{
}

// game/client/c_baseentity.cpp:5246 @0x2bb910 _ZN12C_BaseEntity20GetAttackDamageScaleEv
float C_BaseEntity::GetAttackDamageScale()
{
}

// game/client/c_baseentity.cpp:5267 @0x2bb920 _ZNK12C_BaseEntity20IsDormantPredictableEv
bool C_BaseEntity::IsDormantPredictable()
{
}

// game/client/c_baseentity.cpp:5276 @0x2bb930 _ZN12C_BaseEntity21SetDormantPredictableEb
void C_BaseEntity::SetDormantPredictable( bool dormant )
{
}

// game/client/c_baseentity.cpp:5299 @0x2bb970 _ZNK12C_BaseEntity23BecameDormantThisPacketEv
bool C_BaseEntity::BecameDormantThisPacket()
{
}

// game/client/c_baseentity.cpp:5317 (declaration)
void IsIntermediateDataAllocated();

// game/client/c_baseentity.cpp:5317 @0x2bb9b0 _ZNK12C_BaseEntity27IsIntermediateDataAllocatedEv
bool C_BaseEntity::IsIntermediateDataAllocated()
{
}

// game/client/c_baseentity.cpp:5329 @0x2c1940 _ZN12C_BaseEntity24AllocateIntermediateDataEv
void C_BaseEntity::AllocateIntermediateData()
{
	size_t allocsize;  // line 5334
	{
		int i;  // line 5347
	}
	{
		int i;  // line 5339
	}
	// inlined C_BaseEntity::GetIntermediateDataSize() at line 5334
}

// game/client/c_baseentity.cpp:5363 (declaration)
void DestroyIntermediateData();

// game/client/c_baseentity.cpp:5363 @0x2bcc20 _ZN12C_BaseEntity23DestroyIntermediateDataEv
void C_BaseEntity::DestroyIntermediateData()
{
	{
		int i;  // line 5368
	}
	{
		int i;  // line 5376
	}
}

// game/client/c_baseentity.cpp:5395 @0x2bb9d0 _ZN12C_BaseEntity28ShiftIntermediateDataForwardEii
void C_BaseEntity::ShiftIntermediateDataForward( int slots_to_remove, int number_of_commands_run )
{
	uint8 *saved[150];  // line 5405
	int i;  // line 5408
	{
		int slot;  // line 5423
	}
}

// game/client/c_baseentity.cpp:5435 @0x2bba70 _ZN12C_BaseEntity42ShiftFirstPredictedIntermediateDataForwardEi
void C_BaseEntity::ShiftFirstPredictedIntermediateDataForward( int slots_to_remove )
{
	uint8 *saved_FirstPredicted[151];  // line 5448
	int i;  // line 5451
	int iEndBase;  // line 5463
}

// game/client/c_baseentity.cpp:5482 (declaration)
void GetPredictedFrame( int framenumber );

// game/client/c_baseentity.cpp:5482 @0x2bbb70 _ZN12C_BaseEntity17GetPredictedFrameEi
void *C_BaseEntity::GetPredictedFrame( int framenumber )
{
}

// game/client/c_baseentity.cpp:5498 @0x2bbbb0 _ZN12C_BaseEntity22GetFirstPredictedFrameEi
void *C_BaseEntity::GetFirstPredictedFrame( int framenumber )
{
}

// game/client/c_baseentity.cpp:5513 (declaration)
void GetOriginalNetworkDataObject();

// game/client/c_baseentity.cpp:5513 @0x2bbbe0 _ZN12C_BaseEntity28GetOriginalNetworkDataObjectEv
void *C_BaseEntity::GetOriginalNetworkDataObject()
{
}

// game/client/c_baseentity.cpp:5530 (declaration)
void ComputePackedOffsets();

// game/client/c_baseentity.cpp:5530 @0x2bcbf0 _ZN12C_BaseEntity20ComputePackedOffsetsEv
void C_BaseEntity::ComputePackedOffsets()
{
	datamap_t *map;  // line 5533
}

// game/client/c_baseentity.cpp:5544 (declaration)
void GetIntermediateDataSize();

// game/client/c_baseentity.cpp:5544 @0x2c0550 _ZN12C_BaseEntity23GetIntermediateDataSizeEv
int C_BaseEntity::GetIntermediateDataSize()
{
	const datamap_t *map;  // line 5549
	int size;  // line 5553
	// inlined C_BaseEntity::ComputePackedOffsets() at line 5547
}

// game/client/c_baseentity.cpp:5565 @0x2c0b80 _ZN12C_BaseEntity10SUB_RemoveEv
void C_BaseEntity::SUB_Remove()
{
}

// game/client/c_baseentity.cpp:5577 @0x2c28f0 _Z30FindEntityInFrontOfLocalPlayerv
C_BaseEntity *FindEntityInFrontOfLocalPlayer()
{
	C_BasePlayer *pPlayer;  // line 5584
	{
		trace_t tr;  // line 5588
		Vector forward;  // line 5589
		// inlined Vector::operator*() at line 5591
		// inlined Vector::operator+() at line 5591
		// inlined UTIL_TraceLine() at line 5591
	}
}

// game/client/c_baseentity.cpp:5603 @0x2c4640 _ZL14RemoveDecals_fv
RemoveDecals_f()
{
	C_BaseEntity *pHit;  // line 5605
	// inlined C_BaseEntity::RemoveAllDecals() at line 5608
}

// game/client/c_baseentity.cpp:5612
static ConCommand cl_removedecals;

// game/client/c_baseentity.cpp:5618 @0x2bbbf0 _ZN12C_BaseEntity22ClearBBoxVisualizationEv
void C_BaseEntity::ClearBBoxVisualization()
{
}

// game/client/c_baseentity.cpp:5626 (declaration)
void ToggleBBoxVisualization( int fVisFlags );

// game/client/c_baseentity.cpp:5626 @0x2bbc00 _ZN12C_BaseEntity23ToggleBBoxVisualizationEi
void C_BaseEntity::ToggleBBoxVisualization( int fVisFlags )
{
}

// game/client/c_baseentity.cpp:5641 (declaration)
void ToggleBBoxVisualization( int fVisFlags, const CCommand &args );

// game/client/c_baseentity.cpp:5669 @0x2c2b50 _ZL11cl_ent_bboxRK8CCommand
cl_ent_bbox( const CCommand &args )
{
	// inlined ToggleBBoxVisualization() at line 5671
}

// game/client/c_baseentity.cpp:5669
static ConCommand cl_ent_bbox_command;

// game/client/c_baseentity.cpp:5678 @0x2c4910 _ZL13cl_ent_absboxRK8CCommand
cl_ent_absbox( const CCommand &args )
{
	// inlined ToggleBBoxVisualization() at line 5680
}

// game/client/c_baseentity.cpp:5678
static ConCommand cl_ent_absbox_command;

// game/client/c_baseentity.cpp:5687 @0x2c4880 _ZL11cl_ent_rboxRK8CCommand
cl_ent_rbox( const CCommand &args )
{
	// inlined ToggleBBoxVisualization() at line 5689
}

// game/client/c_baseentity.cpp:5687
static ConCommand cl_ent_rbox_command;

// game/client/c_baseentity.cpp:5695 @0x2bbc40 _ZN12C_BaseEntity22DrawBBoxVisualizationsEv
void C_BaseEntity::DrawBBoxVisualizations()
{
	{
		Vector vecRenderMins;  // line 5713
		Vector vecRenderMaxs;  // line 5713
	}
	{
		Vector vecSurroundMins;  // line 5705
		Vector vecSurroundMaxs;  // line 5705
	}
}

// game/client/c_baseentity.cpp:5724 (declaration)
void SetRenderMode( RenderMode_t nRenderMode, bool bForceUpdate );

// game/client/c_baseentity.cpp:5724 @0x2bbe70 _ZN12C_BaseEntity13SetRenderModeE12RenderMode_tb
void C_BaseEntity::SetRenderMode( RenderMode_t nRenderMode, bool bForceUpdate )
{
}

// game/client/c_baseentity.cpp:5733 (declaration)
void SetRenderFX( RenderFx_t nRenderFX, float flStartTime, float flDuration );

// game/client/c_baseentity.cpp:5733 @0x2bbed0 _ZN12C_BaseEntity11SetRenderFXE10RenderFx_tff
void C_BaseEntity::SetRenderFX( RenderFx_t nRenderFX, float flStartTime, float flDuration )
{
	bool bStartTimeUnspecified;  // line 5735
}

// game/client/c_baseentity.cpp:5759 @0x2c4f10 _ZN12C_BaseEntity8SaveDataEPKcii
void C_BaseEntity::SaveData( const char *context, int slot, int type )
{
	CVProfScope VProf_;  // line 5762
	void *dest;  // line 5764
	CPredictionCopy copyHelper;  // line 5772
	// inlined C_BaseEntity::GetOriginalNetworkDataObject() at line 5764
	// inlined CVProfScope::CVProfScope() at line 5762
	// inlined C_BaseEntity::GetPredictedFrame() at line 5764
	// inlined CPredictionCopy::~CPredictionCopy() at line 5773
	// inlined CVProfScope::~CVProfScope() at line 5773
	// inlined CVProfScope::~CVProfScope() at line 5773
	// inlined CPredictionCopy::~CPredictionCopy() at line 5773
}

// game/client/c_baseentity.cpp:5788 @0x2c4b90 _ZN12C_BaseEntity11RestoreDataEPKcii
void C_BaseEntity::RestoreData( const char *context, int slot, int type )
{
	CVProfScope VProf_;  // line 5791
	const void *src;  // line 5793
	const int savedEFlagsMask;  // line 5797
	int savedEFlags;  // line 5798
	CPredictionCopy copyHelper;  // line 5800
	// inlined C_BaseEntity::GetOriginalNetworkDataObject() at line 5793
	// inlined CVProfScope::CVProfScope() at line 5791
	// inlined C_BaseEntity::GetPredictedFrame() at line 5793
	// inlined C_BaseEntity::AddEFlags() at line 5805
	// inlined CPredictionCopy::~CPredictionCopy() at line 5805
	// inlined CVProfScope::~CVProfScope() at line 5805
	// inlined CVProfScope::~CVProfScope() at line 5805
	// inlined CPredictionCopy::~CPredictionCopy() at line 5805
}

// game/client/c_baseentity.cpp:5810 @0x2c2ff0 _ZN12C_BaseEntity17OnPostRestoreDataEv
void C_BaseEntity::OnPostRestoreData()
{
	// inlined C_BaseEntity::GetMoveParent() at line 5815
	{
		CMDLCacheCriticalSection cacheCriticalSection;  // line 5823
		// inlined CMDLCacheCriticalSection::CMDLCacheCriticalSection() at line 5823
		// inlined C_BaseEntity::GetModelIndex() at line 5824
		// inlined C_BaseEntity::SetModelByIndex() at line 5824
		// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 5824
		// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 5824
	}
	// inlined C_BaseEntity::AddToAimEntsList() at line 5817
}

// game/client/c_baseentity.cpp:5833 @0x2c6bc0 _ZN12C_BaseEntity19EstimateAbsVelocityER6Vector
void C_BaseEntity::EstimateAbsVelocity( Vector &vel )
{
	CInterpolationContext context;  // line 5841
	// inlined Vector::operator=() at line 5837
	// inlined C_BaseEntity::GetAbsVelocity() at line 5837
	// inlined CInterpolationContext::CInterpolationContext() at line 5841
	// inlined CInterpolationContext::EnableExtrapolation() at line 5842
	// inlined CInterpolationContext::~CInterpolationContext() at line 5843
}

// game/client/c_baseentity.cpp:5846 (declaration)
void Interp_Reset( VarMapping_t *map );

// game/client/c_baseentity.cpp:5846 @0x2bbf70 _ZN12C_BaseEntity12Interp_ResetEP12VarMapping_t
void C_BaseEntity::Interp_Reset( VarMapping_t *map )
{
	int c;  // line 5849
	// inlined CUtlVector<VarMapEntry_t,CUtlMemory<VarMapEntry_t, int> >::Count() at line 5849
	{
		int i;  // line 5850
		{
			VarMapEntry_t *e;  // line 5852
			IInterpolatedVar *watcher;  // line 5853
		}
	}
}

// game/client/c_baseentity.cpp:5859 @0x2bf6a0 _ZN12C_BaseEntity12ResetLatchedEv
void C_BaseEntity::ResetLatched()
{
	// inlined C_BaseEntity::Interp_Reset() at line 5864
}

// game/client/c_baseentity.cpp:5873 (declaration)
float AdjustInterpolationAmount( C_BaseEntity *pEntity, float baseInterpolation );

// game/client/c_baseentity.cpp:5896 @0x2bf9e0 _ZN12C_BaseEntity22GetInterpolationAmountEi
float C_BaseEntity::GetInterpolationAmount( int flags )
{
	int serverTickMultiple;  // line 5899
	int expandedServerTickMultiple;  // line 5917
	// inlined AdjustInterpolationAmount() at line 5914
	// inlined C_BaseEntity::IsAnimatedEveryTick() at line 5923
	// inlined AdjustInterpolationAmount() at line 5937
}

// game/client/c_baseentity.cpp:5941 (declaration)
void GetLastChangeTime( int flags );

// game/client/c_baseentity.cpp:5941 @0x2bbfd0 _ZN12C_BaseEntity17GetLastChangeTimeEi
float C_BaseEntity::GetLastChangeTime( int flags )
{
	// inlined C_BaseEntity::GetAnimTime() at line 5953
	{
		float st;  // line 5958
		// inlined C_BaseEntity::GetSimulationTime() at line 5958
	}
}

// game/client/c_baseentity.cpp:5971 @0x2bc390 _ZNK12C_BaseEntity18GetPrevLocalOriginEv
const Vector &C_BaseEntity::GetPrevLocalOrigin()
{
	// inlined CInterpolatedVarArrayBase<Vector,false>::GetPrev() at line 5973
}

// game/client/c_baseentity.cpp:5976 @0x2bc3e0 _ZNK12C_BaseEntity18GetPrevLocalAnglesEv
const QAngle &C_BaseEntity::GetPrevLocalAngles()
{
	// inlined CInterpolatedVarArrayBase<QAngle,false>::GetPrev() at line 5978
}

// game/client/c_baseentity.cpp:5984 @0x2bc060 _ZN12C_BaseEntity10IsFloatingEv
bool C_BaseEntity::IsFloating()
{
}

// game/client/c_baseentity.cpp:5992 (declaration)
void GetBaseMap();

// game/client/c_baseentity.cpp:5992 @0x2bc070 _ZN12C_BaseEntity14GetDataDescMapEv
datamap_t *C_BaseEntity::GetDataDescMap()
{
}

// game/client/c_baseentity.cpp:5992 @0x2bc080 _ZN12C_BaseEntity10GetBaseMapEv
datamap_t *C_BaseEntity::GetBaseMap()
{
}

// game/client/c_baseentity.cpp:5992 @0x2bd20 _Z11DataMapInitI12C_BaseEntityEP9datamap_tPT_
datamap_t *DataMapInit<C_BaseEntity>( C_BaseEntity * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 5992
	CDatadescGeneratedNameHolder nameHolder;  // line 5992
	typedescription_t dataDesc[6];  // line 5992
}

// game/client/c_baseentity.cpp:5992
datamap_t *g_DataMapHolder;

// game/client/c_baseentity.cpp:5992
void C_BaseEntity::m_DataMap;

// game/client/c_baseentity.cpp:6004 @0x2bc090 _ZN12C_BaseEntity17ShouldSavePhysicsEv
bool C_BaseEntity::ShouldSavePhysics()
{
}

// game/client/c_baseentity.cpp:6012 @0x2c3b50 _ZN12C_BaseEntity6OnSaveEv
void C_BaseEntity::OnSave()
{
}

// game/client/c_baseentity.cpp:6024 @0x2c2020 _ZN12C_BaseEntity9OnRestoreEv
void C_BaseEntity::OnRestore()
{
}

// game/client/c_baseentity.cpp:6040 @0x2bf480 _ZN12C_BaseEntity4SaveER5ISave
int C_BaseEntity::Save( ISave &save )
{
	int status;  // line 6043
	// inlined C_BaseEntity::SaveDataDescBlock() at line 6043
}

// game/client/c_baseentity.cpp:6052 (declaration)
void SaveDataDescBlock( ISave &save, datamap_t *dmap );

// game/client/c_baseentity.cpp:6052 @0x2bc0a0 _ZN12C_BaseEntity17SaveDataDescBlockER5ISaveP9datamap_t
int C_BaseEntity::SaveDataDescBlock( ISave &save, datamap_t *dmap )
{
	int nResult;  // line 6054
}

// game/client/c_baseentity.cpp:6058 @0x2bc0c0 _ZN12C_BaseEntity12SetClassnameEPKc
void C_BaseEntity::SetClassname( const char *className )
{
}

// game/client/c_baseentity.cpp:6070 @0x2bf4c0 _ZN12C_BaseEntity7RestoreER8IRestore
int C_BaseEntity::Restore( IRestore &restore )
{
	int status;  // line 6073
	// inlined CHandle<C_BaseEntity>::operator!=() at line 6080
	// inlined C_BaseEntity::RestoreDataDescBlock() at line 6073
}

// game/client/c_baseentity.cpp:6092 (declaration)
void RestoreDataDescBlock( IRestore &restore, datamap_t *dmap );

// game/client/c_baseentity.cpp:6092 @0x2bc0d0 _ZN12C_BaseEntity20RestoreDataDescBlockER8IRestoreP9datamap_t
int C_BaseEntity::RestoreDataDescBlock( IRestore &restore, datamap_t *dmap )
{
}

// game/client/c_baseentity.cpp:6100 @0x2bc0f0 _ZN12C_BaseEntity10ObjectCapsEv
int C_BaseEntity::ObjectCaps()
{
}

// game/client/c_baseentity.cpp:6109 @0x2bc100 _ZN12C_BaseEntity12MyNPCPointerEv
C_AI_BaseNPC *C_BaseEntity::MyNPCPointer()
{
}

// game/client/c_baseentity.cpp:6125 @0x2bdc60 _ZN12C_BaseEntity36RemoveRecipientsIfNotCloseCaptioningER17C_RecipientFilter
void C_BaseEntity::RemoveRecipientsIfNotCloseCaptioning( C_RecipientFilter &filter )
{
	ConVar closecaption;  // line 6127
}

// game/client/c_baseentity.cpp:6139 @0x2c2000 _ZN12C_BaseEntity16EnableInToolViewEb
void C_BaseEntity::EnableInToolView( bool bEnable )
{
}

// game/client/c_baseentity.cpp:6147 (declaration)
void SetToolRecording( bool recording );

// game/client/c_baseentity.cpp:6147 @0x2bc130 _ZN12C_BaseEntity16SetToolRecordingEb
void C_BaseEntity::SetToolRecording( bool recording )
{
	// inlined C_BaseEntity::GetClientHandle() at line 6153
	// inlined C_BaseEntity::GetClientHandle() at line 6158
}

// game/client/c_baseentity.cpp:6163 (declaration)
void HasRecordedThisFrame();

// game/client/c_baseentity.cpp:6163 @0x2bc1c0 _ZNK12C_BaseEntity20HasRecordedThisFrameEv
bool C_BaseEntity::HasRecordedThisFrame()
{
}

// game/client/c_baseentity.cpp:6173 @0x2c6520 _ZN12C_BaseEntity21GetToolRecordingStateEP9KeyValues
void C_BaseEntity::GetToolRecordingState( KeyValues *msg )
{
	CVProfScope VProf_;  // line 6179
	C_BaseEntity *pOwner;  // line 6181
	C_BaseEntity *pParent;  // line 6201
	// inlined CVProfScope::~CVProfScope() at line 6213
	// inlined C_BaseEntity::GetMoveParent() at line 6210
	// inlined C_BaseEntity::GetMoveParent() at line 6201
	// inlined QAngle::operator=() at line 6192
	// inlined Vector::operator=() at line 6191
	// inlined CHandle<C_BaseEntity>::operator C_BaseEntity*() at line 6181
	// inlined CVProfScope::CVProfScope() at line 6179
	// inlined BaseEntityRecordingState_t::BaseEntityRecordingState_t() at line 6183
	// inlined CVProfScope::~CVProfScope() at line 6213
	BaseEntityRecordingState_t state;  // line 6183
}

// game/client/c_baseentity.cpp:6216 @0x2bc1f0 _ZN12C_BaseEntity25CleanupToolRecordingStateEP9KeyValues
void C_BaseEntity::CleanupToolRecordingState( KeyValues *msg )
{
}

// game/client/c_baseentity.cpp:6220 @0x2be8b0 _ZN12C_BaseEntity17RecordToolMessageEv
void C_BaseEntity::RecordToolMessage()
{
	KeyValues *msg;  // line 6229
}

// game/client/c_baseentity.cpp:6243 @0x2c6960 _ZN12C_BaseEntity18ToolRecordEntitiesEv
void C_BaseEntity::ToolRecordEntities()
{
	CVProfScope VProf_;  // line 6245
	CSetActiveSplitScreenPlayerGuard g_SSGuard;  // line 6250
	int c;  // line 6253
	// inlined CVProfScope::CVProfScope() at line 6245
	// inlined CVProfScope::~CVProfScope() at line 6261
	{
		int i;  // line 6254
		{
			IClientRenderable *pRenderable;  // line 6256
		}
	}
	// inlined CVProfScope::~CVProfScope() at line 6261
}

// game/client/c_baseentity.cpp:6268 (declaration)
void AddToEntityList( entity_list_ids_t listId );

// game/client/c_baseentity.cpp:6268 @0x2c05a0 _ZN12C_BaseEntity15AddToEntityListE17entity_list_ids_t
void C_BaseEntity::AddToEntityList( entity_list_ids_t listId )
{
	// inlined CUtlLinkedList<C_BaseEntity*,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<C_BaseEntity*, short unsigned int>, short unsigned int> >::AddToTail() at line 6273
}

// game/client/c_baseentity.cpp:6277 (declaration)
void RemoveFromEntityList( entity_list_ids_t listId );

// game/client/c_baseentity.cpp:6277 @0x2bf250 _ZN12C_BaseEntity20RemoveFromEntityListE17entity_list_ids_t
void C_BaseEntity::RemoveFromEntityList( entity_list_ids_t listId )
{
	// inlined CUtlLinkedList<C_BaseEntity*,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<C_BaseEntity*, short unsigned int>, short unsigned int> >::Remove() at line 6283
}

// game/client/c_baseentity.cpp:6289 @0x2bfdb0 _ZN12C_BaseEntity6AddVarEPvP16IInterpolatedVarib
void C_BaseEntity::AddVar( void *data, IInterpolatedVar *watcher, int type, bool bSetup )
{
	bool bAddIt;  // line 6292
	{
		int i;  // line 6293
		// inlined CUtlVector<VarMapEntry_t,CUtlMemory<VarMapEntry_t, int> >::operator[]() at line 6300
		// inlined CUtlVector<VarMapEntry_t,CUtlMemory<VarMapEntry_t, int> >::Count() at line 6293
		// inlined CUtlVector<VarMapEntry_t,CUtlMemory<VarMapEntry_t, int> >::operator[]() at line 6295
		// inlined C_BaseEntity::RemoveVar() at line 6300
	}
	{
		VarMapEntry_t map;  // line 6317
		// inlined CUtlVector<VarMapEntry_t,CUtlMemory<VarMapEntry_t, int> >::AddToTail() at line 6324
		// inlined CUtlVector<VarMapEntry_t,CUtlMemory<VarMapEntry_t, int> >::AddToHead() at line 6328
	}
}

// game/client/c_baseentity.cpp:6341 (declaration)
void RemoveVar( void *data, bool bAssert );

// game/client/c_baseentity.cpp:6341 @0x2bc750 _ZN12C_BaseEntity9RemoveVarEPvb
void C_BaseEntity::RemoveVar( void *data, bool bAssert )
{
	{
		int i;  // line 6343
		// inlined CUtlVector<VarMapEntry_t,CUtlMemory<VarMapEntry_t, int> >::Count() at line 6343
		// inlined CUtlVector<VarMapEntry_t,CUtlMemory<VarMapEntry_t, int> >::operator[]() at line 6345
		// inlined CUtlVector<VarMapEntry_t,CUtlMemory<VarMapEntry_t, int> >::Remove() at line 6350
	}
}

// game/client/c_baseentity.cpp:6360 @0x2c63c0 _ZN12C_BaseEntity20CheckCLInterpChangedEv
void C_BaseEntity::CheckCLInterpChanged()
{
	float flCurValue_Interp;  // line 6362
	float flCurValue_InterpNPCs;  // line 6365
	{
		C_BaseEntityIterator iterator;  // line 6375
		C_BaseEntity *pEnt;  // line 6376
		// inlined C_BaseEntity::Interp_UpdateInterpolationAmounts() at line 6379
	}
	// inlined ConVar::GetFloat() at line 6365
	float flLastValue_Interp;  // line 6363
	float flLastValue_InterpNPCs;  // line 6366
}

// game/client/c_baseentity.cpp:6384 @0x2bc200 _ZN12C_BaseEntity17DontRecordInToolsEv
void C_BaseEntity::DontRecordInTools()
{
}

// game/client/c_baseentity.cpp:6391 @0x2bc210 _ZNK12C_BaseEntity15GetCreationTickEv
int C_BaseEntity::GetCreationTick()
{
}

// game/client/c_baseentity.cpp:6397 @0x2c0ff0 _ZN12C_BaseEntity16SimulateEntitiesEv
void C_BaseEntity::SimulateEntities()
{
	// inlined C_BaseEntity::PurgeRemovedEntities() at line 6457
	{
		int iNext;  // line 6403
		{
			int iCur;  // line 6404
			{
				C_BaseEntity *pCur;  // line 6407
				bool bRemove;  // line 6414
				// inlined C_BaseEntity::RemoveFromEntityList() at line 6420
				// inlined CUtlLinkedList<C_BaseEntity*,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<C_BaseEntity*, short unsigned int>, short unsigned int> >::Next() at line 6406
			}
			// inlined CUtlLinkedList<C_BaseEntity*,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<C_BaseEntity*, short unsigned int>, short unsigned int> >::Head() at line 6404
		}
	}
	{
		CFastTimer fastTimer;  // line 6426
		int iNext;  // line 6428
		{
			int iCur;  // line 6429
			{
				C_BaseEntity *pCur;  // line 6432
				bool bRemove;  // line 6440
				// inlined C_BaseEntity::GetDebugName() at line 6449
				// inlined CCycleCount::GetMillisecondsF() at line 6449
				// inlined C_BaseEntity::RemoveFromEntityList() at line 6446
				// inlined CUtlLinkedList<C_BaseEntity*,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<C_BaseEntity*, short unsigned int>, short unsigned int> >::Next() at line 6431
				// inlined CFastTimer::Start() at line 6436
				// inlined CFastTimer::End() at line 6448
			}
			// inlined CUtlLinkedList<C_BaseEntity*,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<C_BaseEntity*, short unsigned int>, short unsigned int> >::Head() at line 6429
		}
		// inlined CFastTimer::CFastTimer() at line 6426
	}
}

// game/client/c_baseentity.cpp:6461 (declaration)
void PurgeRemovedEntities();

// game/client/c_baseentity.cpp:6461 @0x2bc430 _ZN12C_BaseEntity20PurgeRemovedEntitiesEv
void C_BaseEntity::PurgeRemovedEntities()
{
	int iNext;  // line 6463
	{
		int iCur;  // line 6464
		// inlined CUtlLinkedList<C_BaseEntity*,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<C_BaseEntity*, short unsigned int>, short unsigned int> >::Head() at line 6464
		{
			C_BaseEntity *pCur;  // line 6467
			// inlined CUtlLinkedList<C_BaseEntity*,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<C_BaseEntity*, short unsigned int>, short unsigned int> >::Next() at line 6466
		}
	}
	// inlined CUtlLinkedList<C_BaseEntity*,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<C_BaseEntity*, short unsigned int>, short unsigned int> >::RemoveAll() at line 6470
}

// game/client/c_baseentity.cpp:6475 @0x2c01d0 _ZN12C_BaseEntity17PreRenderEntitiesEi
void C_BaseEntity::PreRenderEntities( int nSplitScreenPlayerSlot )
{
	CMDLCacheCriticalSection cacheCriticalSection;  // line 6477
	int iNext;  // line 6478
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 6495
	{
		int iCur;  // line 6479
		{
			C_BaseEntity *pCur;  // line 6482
			bool bRemove;  // line 6487
			// inlined CUtlLinkedList<C_BaseEntity*,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<C_BaseEntity*, short unsigned int>, short unsigned int> >::Next() at line 6481
			// inlined C_BaseEntity::RemoveFromEntityList() at line 6493
		}
		// inlined CUtlLinkedList<C_BaseEntity*,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<C_BaseEntity*, short unsigned int>, short unsigned int> >::Head() at line 6479
	}
	// inlined CMDLCacheCriticalSection::CMDLCacheCriticalSection() at line 6477
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 6495
}

// game/client/c_baseentity.cpp:6500 @0x2bc220 _ZN12C_BaseEntity9PreRenderEi
bool C_BaseEntity::PreRender( int nSplitScreenPlayerSlot )
{
	bool bNeedsPrerender;  // line 6502
}

// game/client/c_baseentity.cpp:6512 (declaration)
void IsViewEntity();

// game/client/c_baseentity.cpp:6512 @0x2bc240 _ZNK12C_BaseEntity12IsViewEntityEv
bool C_BaseEntity::IsViewEntity()
{
}

// game/client/c_baseentity.cpp:6517 @0x2bc630 _ZNK12C_BaseEntity22IsAbleToHaveFireEffectEv
bool C_BaseEntity::IsAbleToHaveFireEffect()
{
}

// game/client/c_baseentity.cpp:6523 @0x2c2be0 _ZN12C_BaseEntity12SetBlurStateEb
void C_BaseEntity::SetBlurState( bool bShouldBlur )
{
	// inlined C_BaseEntity::OnTranslucencyTypeChanged() at line 6528
}

// game/client/c_baseentity.cpp:6532 @0x2bc290 _ZN12C_BaseEntity9IsBlurredEv
bool C_BaseEntity::IsBlurred()
{
}

// game/client/c_baseentity.cpp:6537 @0x2bc2a0 _ZN12C_BaseEntity22OnParseMapDataFinishedEv
void C_BaseEntity::OnParseMapDataFinished()
{
}

// game/client/c_baseentity.cpp:6544 (declaration)
void SetCellBits( int cellbits );

// game/client/c_baseentity.cpp:6544 @0x2bc2b0 _ZN12C_BaseEntity11SetCellBitsEi
bool C_BaseEntity::SetCellBits( int cellbits )
{
}

// game/client/c_baseentity.cpp:6555 @0x2bc2e0 _ZNK12C_BaseEntity34ShouldRegenerateOriginFromCellBitsEv
bool C_BaseEntity::ShouldRegenerateOriginFromCellBits()
{
}

// game/client/c_baseentity.cpp:6565 @0x2bce10 _ZN12C_BaseEntity17GetScriptInstanceEv
HSCRIPT C_BaseEntity::GetScriptInstance()
{
	{
		char *szName;  // line 6571
	}
}

// game/client/c_baseentity.cpp:6582 @0x2bdc90 _Z14CC_CL_Find_EntRK8CCommand
CC_CL_Find_Ent( const CCommand &args )
{
	int iCount;  // line 6590
	const char *pszSubString;  // line 6591
	C_BaseEntity *ent;  // line 6594
	// inlined CCommand::operator[]() at line 6591
	{
		const char *pszClassname;  // line 6597
		bool bMatches;  // line 6599
	}
}

// game/client/c_baseentity.cpp:6617
static ConCommand cl_find_ent;

// game/client/c_baseentity.cpp:6620 @0x2be1d0 _Z20CC_CL_Find_Ent_IndexRK8CCommand
CC_CL_Find_Ent_Index( const CCommand &args )
{
	int iIndex;  // line 6628
	C_BaseEntity *ent;  // line 6629
	{
		const char *pszClassname;  // line 6632
	}
}

// game/client/c_baseentity.cpp:6640
static ConCommand cl_find_ent_index;
