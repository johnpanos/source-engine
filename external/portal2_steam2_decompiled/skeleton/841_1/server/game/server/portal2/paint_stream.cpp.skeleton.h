// DWARF declaration skeleton for game/server/portal2/paint_stream.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xaea70 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined ServerClass::ServerClass() at line 17
	// inlined CEntityFactory<CPaintStream>::CEntityFactory() at line 32
}

// game/server/portal2/paint_stream.cpp:14
ConVar draw_paint_server_blobs;

// game/server/portal2/paint_stream.cpp:15
ConVar paintblob_old_data_time_offset;

// game/server/portal2/paint_stream.cpp:17 @0x73a070 _ZN12CPaintStream14GetServerClassEv
ServerClass *CPaintStream::GetServerClass()
{
}

// game/server/portal2/paint_stream.cpp:17 @0x73a080 _ZN12CPaintStream40YouForgotToImplementOrDeclareServerClassEv
int CPaintStream::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/paint_stream.cpp:17 @0xae770 _Z15ServerClassInitIN14DT_PaintStream7ignoredEEiPT_
int ServerClassInit<DT_PaintStream::ignored>( DT_PaintStream::ignored * )
{
	SendTable &sendTable;  // line 17
	char *const g_pSendTableName;  // line 17
	SendProp g_SendProps[4];  // line 17
}

// game/server/portal2/paint_stream.cpp:17
SendTable g_SendTable;

// game/server/portal2/paint_stream.cpp:17
int g_SendTableInit;

// game/server/portal2/paint_stream.cpp:17
static ServerClass g_CPaintStream_ClassReg;

// game/server/portal2/paint_stream.cpp:17
void CPaintStream::m_pClassSendTable;

// game/server/portal2/paint_stream.cpp:25 (declaration)
void GetBaseMap();

// game/server/portal2/paint_stream.cpp:25 @0x73a090 _ZN12CPaintStream14GetDataDescMapEv
datamap_t *CPaintStream::GetDataDescMap()
{
}

// game/server/portal2/paint_stream.cpp:25 @0x73a0a0 _ZN12CPaintStream10GetBaseMapEv
datamap_t *CPaintStream::GetBaseMap()
{
}

// game/server/portal2/paint_stream.cpp:25 @0xae9a0 _Z11DataMapInitI12CPaintStreamEP9datamap_tPT_
datamap_t *DataMapInit<CPaintStream>( CPaintStream * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 25
	CDatadescGeneratedNameHolder nameHolder;  // line 25
	typedescription_t dataDesc[3];  // line 25
}

// game/server/portal2/paint_stream.cpp:25
datamap_t *g_DataMapHolder;

// game/server/portal2/paint_stream.cpp:25
void CPaintStream::m_DataMap;

// game/server/portal2/paint_stream.cpp:32
static CEntityFactory<CPaintStream> paint_stream;

// game/server/portal2/paint_stream.cpp:35 (declaration)
void CPaintStream();

// game/server/portal2/paint_stream.cpp:35 @0x73a630 _ZN12CPaintStreamC2Ev
CPaintStream::CPaintStream()
{
	// inlined CNetworkVarBase<int,CPaintStream::NetworkVar_m_nRenderMode>::operator=<BlobRenderMode_t>() at line 39
	// inlined CUtlVector<CPaintStream*,CUtlMemory<CPaintStream*, int> >::AddToTail() at line 37
	// inlined CSharedUtlVectorBase<BlobDataPerUpdate_t,CPaintStream::SharedVar_m_sharedBlobData>::CSharedUtlVectorBase() at line 35
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::CUtlVector() at line 35
	// inlined CSharedUtlVectorBase<BlobDataPerUpdate_t,CPaintStream::SharedVar_m_sharedBlobData>::~CSharedUtlVectorBase() at line 40
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::~CUtlVector() at line 40
}

// game/server/portal2/paint_stream.cpp:35 @0x73aae0 _ZN12CPaintStreamC1Ev
CPaintStream::CPaintStream()
{
}

// game/server/portal2/paint_stream.cpp:43 (declaration)
~CPaintStream();

// game/server/portal2/paint_stream.cpp:43 @0x73af70 _ZN12CPaintStreamD0Ev
CPaintStream::~CPaintStream()
{
	// inlined CSharedUtlVectorBase<BlobDataPerUpdate_t,CPaintStream::SharedVar_m_sharedBlobData>::GetForModify() at line 47
	// inlined CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> >::Purge() at line 47
	// inlined CSharedUtlVectorBase<BlobDataPerUpdate_t,CPaintStream::SharedVar_m_sharedBlobData>::~CSharedUtlVectorBase() at line 48
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::~CUtlVector() at line 48
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::~CUtlVector() at line 48
	// inlined CSharedUtlVectorBase<BlobDataPerUpdate_t,CPaintStream::SharedVar_m_sharedBlobData>::~CSharedUtlVectorBase() at line 48
}

// game/server/portal2/paint_stream.cpp:43 @0x73b890 _ZN12CPaintStreamD2Ev
CPaintStream::~CPaintStream()
{
	// inlined CSharedUtlVectorBase<BlobDataPerUpdate_t,CPaintStream::SharedVar_m_sharedBlobData>::GetForModify() at line 47
	// inlined CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> >::Purge() at line 47
	// inlined CSharedUtlVectorBase<BlobDataPerUpdate_t,CPaintStream::SharedVar_m_sharedBlobData>::~CSharedUtlVectorBase() at line 48
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::~CUtlVector() at line 48
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::~CUtlVector() at line 48
	// inlined CSharedUtlVectorBase<BlobDataPerUpdate_t,CPaintStream::SharedVar_m_sharedBlobData>::~CSharedUtlVectorBase() at line 48
}

// game/server/portal2/paint_stream.cpp:43 @0x73c1a0 _ZN12CPaintStreamD1Ev
CPaintStream::~CPaintStream()
{
}

// game/server/portal2/paint_stream.cpp:51 @0x73a4c0 _ZN12CPaintStream8PrecacheEv
void CPaintStream::Precache()
{
}

// game/server/portal2/paint_stream.cpp:57 @0x73a470 _ZN12CPaintStream5SpawnEv
void CPaintStream::Spawn()
{
}

// game/server/portal2/paint_stream.cpp:67 @0x73a450 _ZN12CPaintStream19UpdateTransmitStateEv
int CPaintStream::UpdateTransmitState()
{
}

// game/server/portal2/paint_stream.cpp:73 @0x73aaf0 _ZN12CPaintStream12AddPaintBlobEP14CBasePaintBlob
void CPaintStream::AddPaintBlob( CBasePaintBlob *pBlob )
{
	CPaintBlob *pServerBlob;  // line 80
	// inlined CPaintBlob::AssignBlobID() at line 81
	// inlined CBaseEntity::entindex() at line 81
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::AddToTail() at line 78
}

// game/server/portal2/paint_stream.cpp:86 @0x73a1d0 _ZN12CPaintStream37UpdateRenderBoundsAndOriginWorldspaceEv
void CPaintStream::UpdateRenderBoundsAndOriginWorldspace()
{
	{
		Vector vecPos;  // line 98
		Vector vRadius;  // line 112
		CCollisionProperty *prop;  // line 116
		// inlined Vector::operator=() at line 94
		// inlined Vector::operator=() at line 94
		// inlined Vector::operator=() at line 94
		// inlined Vector::Vector() at line 94
		// inlined Vector::operator=() at line 100
		// inlined Vector::operator=() at line 101
		{
			int i;  // line 103
			// inlined Vector::operator=() at line 105
			// inlined VectorMin() at line 106
			// inlined VectorMax() at line 107
		}
		// inlined Vector::operator+() at line 110
		// inlined Vector::operator*() at line 110
		// inlined Vector::operator=() at line 110
		// inlined ReplicateToVector() at line 112
		// inlined Vector::operator VectorByValue&() at line 112
		// inlined Vector::operator-=() at line 113
		// inlined Vector::operator+=() at line 114
	}
}

// game/server/util.h:100 @0x73d340 _ZN14CEntityFactoryI12CPaintStreamE6CreateEPKc
IServerNetworkable *CEntityFactory<CPaintStream>::Create( const char *pClassName )
{
	CPaintStream *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CPaintStream>() at line 102
}

// game/server/util.h:106 @0x73d270 _ZN14CEntityFactoryI12CPaintStreamE7DestroyEP18IServerNetworkable
void CEntityFactory<CPaintStream>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:114 @0x73d2a0 _ZN14CEntityFactoryI12CPaintStreamE13GetEntitySizeEv
size_t CEntityFactory<CPaintStream>::GetEntitySize()
{
}

// game/server/portal2/paint_stream.cpp:122 @0x73d130 _ZN12CPaintStream6UpdateEv
void CPaintStream::Update()
{
	// inlined CPaintStream::DebugDrawBlobs() at line 130
}

// game/server/portal2/paint_stream.cpp:143 @0x73c1b0 _ZN12CPaintStream20UpdateBlobSharedDataEv
void CPaintStream::UpdateBlobSharedData()
{
	BlobDataPerUpdateVector_t &blobData;  // line 145
	int lastIndex;  // line 146
	const int nBlobCount;  // line 150
	BlobDataVector_t &blobDataVector;  // line 152
	int nNumParticles;  // line 154
	const float flShouldRemoveTime;  // line 212
	int nToRemove;  // line 213
	{
		int i;  // line 214
		// inlined CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> >::Count() at line 214
		// inlined CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> >::operator[]() at line 216
	}
	// inlined ConVar::GetFloat() at line 212
	// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::SetCountNonDestructively() at line 208
	// inlined CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> >::RemoveMultipleFromHead() at line 228
	// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::SetCount() at line 153
	// inlined CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> >::operator[]() at line 152
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::Count() at line 150
	// inlined CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> >::AddToTail() at line 147
	// inlined CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> >::Count() at line 146
	// inlined CSharedUtlVectorBase<BlobDataPerUpdate_t,CPaintStream::SharedVar_m_sharedBlobData>::GetForModify() at line 145
	{
		int i;  // line 155
		{
			CPaintBlob *pBlob;  // line 157
			bool bTeleportThisFrame;  // line 163
			{
				BlobData_t &blobData;  // line 167
				// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::operator[]() at line 167
				// inlined Vector::operator=() at line 170
			}
			{
				BlobData_t &blobData;  // line 182
				// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::AddToTail() at line 180
				// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::operator[]() at line 182
				// inlined Vector::operator=() at line 185
				{
					int h;  // line 189
					{
						BlobTeleportationHistory_t &history;  // line 191
						// inlined CUtlVector<BlobTeleportationHistory_t,CUtlMemory<BlobTeleportationHistory_t, int> >::operator[]() at line 191
						// inlined V_swap<Vector>() at line 192
					}
				}
			}
		}
	}
}

// game/server/portal2/paint_stream.cpp:233 (declaration)
void DebugDrawBlobs();

// game/server/portal2/paint_stream.cpp:233 @0x73a100 _ZN12CPaintStream14DebugDrawBlobsEv
void CPaintStream::DebugDrawBlobs()
{
	bool bDebugDraw;  // line 235
	Color debugColor;  // line 236
	// inlined Color::Color() at line 236
	// inlined Color::Color() at line 241
	// inlined Color::operator=() at line 241
	{
		int i;  // line 246
		// inlined Color::b() at line 248
		// inlined Color::g() at line 248
		// inlined Color::r() at line 248
	}
}

// game/server/portal2/paint_stream.cpp:254 @0x73a0b0 _ZNK12CPaintStream18AddPaintToDatabaseEv
void CPaintStream::AddPaintToDatabase()
{
	{
		int i;  // line 256
		{
			CPaintBlob *pBlob;  // line 258
		}
	}
}

// game/server/portal2/paint_stream.cpp:264 @0x73a4d0 _ZN12CPaintStream18SavePaintBlobStateEP5ISave
void CPaintStream::SavePaintBlobState( ISave *pSave )
{
	int count;  // line 266
	{
		EHANDLE hPaintStream;  // line 272
		int power;  // line 274
		// inlined CHandle<CBaseEntity>::CHandle() at line 272
		{
			int i;  // line 276
			{
				CBasePaintBlob *pBlob;  // line 278
				float flMaxStreakTime;  // line 281
				float flStreakSpeedDampenRate;  // line 283
			}
		}
	}
}

// game/server/portal2/paint_stream.cpp:292 @0x73acd0 _ZN12CPaintStream21RestorePaintBlobStateEP8IRestore
void CPaintStream::RestorePaintBlobState( IRestore *pRestore )
{
	ConVarRef blobs_paused;  // line 296
	bool bOldBlobPauseState;  // line 297
	// inlined ConVarRef::SetValue() at line 333
	{
		int count;  // line 304
		{
			EHANDLE hPaintStream;  // line 307
			CPaintStream *pPaintStream;  // line 309
			int power;  // line 311
			{
				int i;  // line 312
				{
					Vector vPosition;  // line 314
					Vector vVelocity;  // line 316
					float flStreakTime;  // line 318
					float flStreakSpeedDampen;  // line 320
					{
						CBasePaintBlob *pBlob;  // line 325
					}
				}
			}
			// inlined CHandle<CBaseEntity>::Get() at line 309
			// inlined CHandle<CBaseEntity>::CHandle() at line 307
		}
	}
	// inlined ConVarRef::SetValue() at line 300
	// inlined ConVarRef::GetBool() at line 297
}

// public/tier1/utlvector.h:655 @0x73d390 _ZN10CUtlVectorI10BlobData_t10CUtlMemoryIS0_iEE10GrowVectorEi
void CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::GrowVector( int num )
{
	// inlined CUtlMemory<BlobData_t,int>::NumAllocated() at line 657
	// inlined CUtlMemory<BlobData_t,int>::Grow() at line 660
	// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::ResetDbgInfo() at line 664
}

// public/tier1/utlvector.h:655 @0x73d4b0 _ZN10CUtlVectorI19BlobDataPerUpdate_t10CUtlMemoryIS0_iEE10GrowVectorEi
void CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> >::GrowVector( int num )
{
	// inlined CUtlMemory<BlobDataPerUpdate_t,int>::NumAllocated() at line 657
	// inlined CUtlMemory<BlobDataPerUpdate_t,int>::Grow() at line 660
	// inlined CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> >::ResetDbgInfo() at line 664
}

// public/tier1/utlvector.h:865 @0x73d5d0 _ZN10CUtlVectorI19BlobDataPerUpdate_t10CUtlMemoryIS0_iEE8SetCountEi
void CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> >::SetCount( int count )
{
	// inlined CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> >::RemoveAll() at line 867
	// inlined CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> >::AddMultipleToTail() at line 868
}
