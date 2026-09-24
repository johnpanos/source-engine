// DWARF declaration skeleton for game/client/portal2/c_paint_stream.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x76530 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined ClientClass::ClientClass() at line 25
	// inlined Cpaint_streamFoo::Cpaint_streamFoo() at line 32
}

// game/client/portal2/c_paint_stream.cpp:17
ConVar draw_paint_isosurface;

// game/client/portal2/c_paint_stream.cpp:18
ConVar draw_paint_client_blobs;

// game/client/portal2/c_paint_stream.cpp:19
ConVar paintblob_draw_distance_from_eye;

// game/client/portal2/c_paint_stream.cpp:20
ConVar paintblob_isosurface_box_width;

// game/client/portal2/c_paint_stream.cpp:23
ConVar paintblob_interpolation_time_offset;

// game/client/portal2/c_paint_stream.cpp:25 @0x526e70 _ZN13C_PaintStream40YouForgotToImplementOrDeclareClientClassEv
int C_PaintStream::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/portal2/c_paint_stream.cpp:25 @0x526e90 _ZN13C_PaintStream14GetClientClassEv
ClientClass *C_PaintStream::GetClientClass()
{
}

// game/client/portal2/c_paint_stream.cpp:25 @0x763a0 _Z15ClientClassInitIN14DT_PaintStream7ignoredEEiPT_
int ClientClassInit<DT_PaintStream::ignored>( DT_PaintStream::ignored * )
{
	char *pRecvTableName;  // line 25
	RecvTable &RecvTable;  // line 25
	RecvProp RecvProps[4];  // line 25
}

// game/client/portal2/c_paint_stream.cpp:25
RecvTable g_RecvTable;

// game/client/portal2/c_paint_stream.cpp:25
int g_RecvTableInit;

// game/client/portal2/c_paint_stream.cpp:25 @0x527660 _ZL27_C_PaintStream_CreateObjectii
IClientNetworkable *_C_PaintStream_CreateObject( int entnum, int serialNum )
{
	C_PaintStream *pRet;  // line 25
	// inlined C_PaintStream::C_PaintStream() at line 25
}

// game/client/portal2/c_paint_stream.cpp:25
ClientClass __g_C_PaintStreamClientClass;

// game/client/portal2/c_paint_stream.cpp:25
void C_PaintStream::m_pClassRecvTable;

// game/client/portal2/c_paint_stream.cpp:32 sizeof=0x1 (i386)
struct Cpaint_streamFoo
{
public:
	Cpaint_streamFoo();  // line 32
};

// game/client/portal2/c_paint_stream.cpp:32 @0x527620 _ZL21CC_PaintStreamFactoryv
C_BaseEntity *CC_PaintStreamFactory()
{
	// inlined C_PaintStream::C_PaintStream() at line 32
}

// game/client/portal2/c_paint_stream.cpp:34 (declaration)
void C_PaintStream();

// game/client/portal2/c_paint_stream.cpp:34 @0x5273b0 _ZN13C_PaintStreamC2Ev
C_PaintStream::C_PaintStream()
{
	// inlined CUtlVector<C_PaintStream*,CUtlMemory<C_PaintStream*, int> >::AddToTail() at line 39
	// inlined CSharedUtlVectorBase<BlobDataPerUpdate_t,C_PaintStream::SharedVar_m_sharedBlobData>::CSharedUtlVectorBase() at line 34
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::CUtlVector() at line 34
	// inlined CSharedUtlVectorBase<BlobDataPerUpdate_t,C_PaintStream::SharedVar_m_sharedBlobData>::~CSharedUtlVectorBase() at line 40
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::~CUtlVector() at line 40
}

// game/client/portal2/c_paint_stream.cpp:34 @0x527610 _ZN13C_PaintStreamC1Ev
C_PaintStream::C_PaintStream()
{
}

// game/client/portal2/c_paint_stream.cpp:43 (declaration)
~C_PaintStream();

// game/client/portal2/c_paint_stream.cpp:43 @0x5276c0 _ZN13C_PaintStreamD2Ev
C_PaintStream::~C_PaintStream()
{
	// inlined CSharedUtlVectorBase<BlobDataPerUpdate_t,C_PaintStream::SharedVar_m_sharedBlobData>::~CSharedUtlVectorBase() at line 46
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::~CUtlVector() at line 46
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::~CUtlVector() at line 46
	// inlined CSharedUtlVectorBase<BlobDataPerUpdate_t,C_PaintStream::SharedVar_m_sharedBlobData>::~CSharedUtlVectorBase() at line 46
}

// game/client/portal2/c_paint_stream.cpp:43 @0x5278e0 _ZN13C_PaintStreamD1Ev
C_PaintStream::~C_PaintStream()
{
}

// game/client/portal2/c_paint_stream.cpp:43 @0x5278f0 _ZN13C_PaintStreamD0Ev
C_PaintStream::~C_PaintStream()
{
	// inlined CSharedUtlVectorBase<BlobDataPerUpdate_t,C_PaintStream::SharedVar_m_sharedBlobData>::~CSharedUtlVectorBase() at line 46
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::~CUtlVector() at line 46
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::~CUtlVector() at line 46
	// inlined CSharedUtlVectorBase<BlobDataPerUpdate_t,C_PaintStream::SharedVar_m_sharedBlobData>::~CSharedUtlVectorBase() at line 46
}

// game/client/portal2/c_paint_stream.cpp:49 @0x526ff0 _ZN13C_PaintStream5SpawnEv
void C_PaintStream::Spawn()
{
}

// game/client/portal2/c_paint_stream.cpp:65 (declaration)
void DebugDrawBlobs();

// game/client/portal2/c_paint_stream.cpp:65 @0x526f20 _ZN13C_PaintStream14DebugDrawBlobsEv
void C_PaintStream::DebugDrawBlobs()
{
	bool bDebugDraw;  // line 67
	Color debugColor;  // line 68
	// inlined Color::Color() at line 68
	// inlined Color::Color() at line 73
	// inlined Color::operator=() at line 73
	{
		int i;  // line 78
		// inlined Color::b() at line 80
		// inlined Color::g() at line 80
		// inlined Color::r() at line 80
	}
}

// game/client/portal2/c_paint_stream.cpp:86 @0x526eb0 _ZN13C_PaintStream15GetRenderAnglesEv
const QAngle &C_PaintStream::GetRenderAngles()
{
}

// game/client/portal2/c_paint_stream.cpp:92 @0x526ed0 _ZN13C_PaintStream15GetRenderOriginEv
const Vector &C_PaintStream::GetRenderOrigin()
{
}

// game/client/portal2/c_paint_stream.cpp:98 @0x527070 _ZN13C_PaintStream25GetRenderBoundsWorldspaceER6VectorS1_
void C_PaintStream::GetRenderBoundsWorldspace( Vector &absMins, Vector &absMaxs )
{
	// inlined Vector::operator=() at line 100
	// inlined Vector::operator=() at line 101
}

// game/client/portal2/c_paint_stream.cpp:105 @0x5270d0 _ZN13C_PaintStream15GetRenderBoundsER6VectorS1_
void C_PaintStream::GetRenderBounds( Vector &theMins, Vector &theMaxs )
{
	// inlined Vector::operator-() at line 107
	// inlined Vector::operator=() at line 107
	// inlined Vector::operator-() at line 108
	// inlined Vector::operator=() at line 108
}

// game/client/portal2/c_paint_stream.cpp:112 @0x526ee0 _ZN13C_PaintStream16WorldSpaceCenterEv
const Vector &C_PaintStream::WorldSpaceCenter()
{
}

// game/client/portal2/c_paint_stream.cpp:118 @0x526f00 _ZN13C_PaintStream31ComputeWorldSpaceSurroundingBoxEP6VectorS1_
void C_PaintStream::ComputeWorldSpaceSurroundingBox( Vector *pMins, Vector *pMaxs )
{
}

// game/client/portal2/c_paint_stream.cpp:124 @0x527160 _ZN13C_PaintStream12AddPaintBlobEP14CBasePaintBlob
void C_PaintStream::AddPaintBlob( CBasePaintBlob *pBlob )
{
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::AddToTail() at line 128
}

// game/client/portal2/c_paint_stream.cpp:133 @0x52a310 _ZN13C_PaintStream37UpdateRenderBoundsAndOriginWorldspaceEv
void C_PaintStream::UpdateRenderBoundsAndOriginWorldspace()
{
	// inlined Vector::operator=() at line 180
	// inlined Vector::operator=() at line 180
	// inlined Vector::operator=() at line 180
	// inlined Vector::Vector() at line 180
	{
		Vector vecPos;  // line 185
		Vector vRadius;  // line 199
		CCollisionProperty *prop;  // line 203
		// inlined Vector::operator=() at line 187
		// inlined Vector::operator=() at line 188
		{
			int i;  // line 190
			// inlined Vector::operator=() at line 192
			// inlined VectorMin() at line 193
			// inlined VectorMax() at line 194
		}
		// inlined Vector::operator+() at line 197
		// inlined Vector::operator*() at line 197
		// inlined Vector::operator=() at line 197
		// inlined ReplicateToVector() at line 199
		// inlined Vector::operator VectorByValue&() at line 199
		// inlined Vector::operator-=() at line 200
		// inlined Vector::operator+=() at line 201
	}
	{
		BlobDataVector_t blobData;  // line 141
		// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::CUtlVector() at line 141
		{
			Vector vecPos;  // line 151
			Vector vRadius;  // line 165
			CCollisionProperty *prop;  // line 169
			// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::operator[]() at line 151
			// inlined Vector::operator=() at line 153
			// inlined Vector::operator=() at line 154
			{
				int i;  // line 156
				// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::operator[]() at line 158
				// inlined Vector::operator=() at line 158
				// inlined VectorMin() at line 159
				// inlined VectorMax() at line 160
			}
			// inlined Vector::operator+() at line 163
			// inlined Vector::operator*() at line 163
			// inlined Vector::operator=() at line 163
			// inlined ReplicateToVector() at line 165
			// inlined Vector::operator VectorByValue&() at line 165
			// inlined Vector::operator-=() at line 166
			// inlined Vector::operator+=() at line 167
		}
		// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::~CUtlVector() at line 174
		// inlined Vector::Vector() at line 146
		// inlined Vector::operator=() at line 146
		// inlined Vector::operator=() at line 146
		// inlined Vector::operator=() at line 146
		// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::~CUtlVector() at line 174
	}
}

// game/client/portal2/c_paint_stream.cpp:215 @0x527200 _Z27BuildPortalMatrixListInAABBRK6VectorS1_R10CUtlVectorI11matrix3x4_t10CUtlMemoryIS3_iEE
BuildPortalMatrixListInAABB( const Vector &vCenter, const Vector &vExtents, PortalMatrixList_t &portalMatrixList )
{
	{
		int i;  // line 217
		{
			C_Portal_Base2D *pPortal;  // line 219
			{
				matrix3x4_t matTransform;  // line 222
				// inlined CUtlVector<matrix3x4_t,CUtlMemory<matrix3x4_t, int> >::AddToTail() at line 224
				// inlined Vector::operator-() at line 223
				// inlined operator*() at line 223
				// inlined Vector::operator VectorByValue&() at line 223
			}
		}
	}
}

// game/client/portal2/c_paint_stream.cpp:230 @0x528c20 _ZN13C_PaintStream9DrawBlobsEP9IMaterial
void C_PaintStream::DrawBlobs( IMaterial *pMaterial )
{
	int nCount;  // line 232
	Vector vCenter;  // line 233
	PortalMatrixList_t portalMatrixList;  // line 235
	// inlined CUtlVector<matrix3x4_t,CUtlMemory<matrix3x4_t, int> >::~CUtlVector() at line 376
	// inlined CUtlVector<matrix3x4_t,CUtlMemory<matrix3x4_t, int> >::~CUtlVector() at line 376
	{
		C_BasePlayer *pPlayer;  // line 328
		Vector vecPlayerPos;  // line 332
		float flDistThreshold;  // line 333
		int iNumParticles;  // line 335
		{
			int i;  // line 338
			{
				CBasePaintBlob *pBlob;  // line 340
				Vector vecWorldCenter;  // line 346
				{
					ImpParticleWithOneInterpolant *imp_particle;  // line 349
					// inlined Point3D::set() at line 352
					// inlined ImpParticle::setFieldScale() at line 351
					// inlined Point3D::operator=() at line 350
				}
				// inlined Vector::DistToSqr() at line 347
				{
					ImpParticleWithOneInterpolant *imp_particle;  // line 359
					// inlined CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants, 16> >::AddToTail() at line 367
					// inlined Point3D::set() at line 362
					// inlined ImpParticle::setFieldScale() at line 361
					// inlined Point3D::operator=() at line 360
				}
			}
		}
		// inlined CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants, 16> >::SetCount() at line 336
		// inlined Vector::operator VectorByValue&() at line 332
		// inlined GetSplitScreenViewPlayer() at line 328
		// inlined Vector::operator/=() at line 316
		{
			int i;  // line 312
			// inlined Vector::operator+=() at line 314
		}
		// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::Count() at line 301
		// inlined CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants, 16> >::SetCountNonDestructively() at line 371
		// inlined ConVar::GetFloat() at line 373
	}
	{
		BlobDataVector_t blobData;  // line 240
		C_BasePlayer *pPlayer;  // line 270
		Vector vecPlayerPos;  // line 274
		float flDistThreshold;  // line 275
		int iNumParticles;  // line 277
		{
			int i;  // line 280
			{
				Vector vecWorldCenter;  // line 282
				{
					ImpParticleWithOneInterpolant *imp_particle;  // line 286
					// inlined Point3D::set() at line 289
					// inlined ImpParticle::setFieldScale() at line 288
					// inlined Point3D::operator=() at line 287
				}
				// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::operator[]() at line 282
				// inlined Vector::DistToSqr() at line 284
			}
		}
		// inlined CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants, 16> >::SetCount() at line 278
		// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::Count() at line 278
		// inlined Vector::operator VectorByValue&() at line 274
		// inlined GetSplitScreenViewPlayer() at line 270
		// inlined Vector::operator/=() at line 258
		{
			int i;  // line 254
			// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::operator[]() at line 256
			// inlined Vector::operator+=() at line 256
		}
		// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::CUtlVector() at line 240
		// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::Count() at line 243
		// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::~CUtlVector() at line 297
		// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::~CUtlVector() at line 297
		// inlined CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants, 16> >::SetCountNonDestructively() at line 295
		// inlined ConVar::GetFloat() at line 297
		// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::~CUtlVector() at line 297
	}
	// inlined Vector::operator VectorByValue&() at line 236
	// inlined operator*() at line 236
	// inlined Vector::operator-() at line 236
	// inlined CUtlVector<matrix3x4_t,CUtlMemory<matrix3x4_t, int> >::CUtlVector() at line 235
	// inlined CUtlVector<matrix3x4_t,CUtlMemory<matrix3x4_t, int> >::Purge() at line 376
	// inlined CUtlVector<matrix3x4_t,CUtlMemory<matrix3x4_t, int> >::~CUtlVector() at line 376
}

// game/client/portal2/c_paint_stream.cpp:380 @0x52a000 _ZN13C_PaintStream9DrawModelEiRK20RenderableInstance_t
int C_PaintStream::DrawModel( int flags, const RenderableInstance_t &instance )
{
	IMaterial *pMaterial;  // line 402
	// inlined UpdateScreenEffectTexture() at line 408
}

// game/client/portal2/c_paint_stream.cpp:418 @0x52acb0 _ZN13C_PaintStream6UpdateEv
void C_PaintStream::Update()
{
	// inlined C_PaintStream::DebugDrawBlobs() at line 426
}

// game/client/portal2/c_paint_stream.cpp:443 @0x527b20 _Z26InterpolateTeleportingBlobfRK10BlobData_tS1_R10CUtlVectorIS_10CUtlMemoryIS_iEE
InterpolateTeleportingBlob( float flPercent, const BlobData_t &blobA, const BlobData_t &blobB, BlobDataVector_t &blobDataVector )
{
	const BlobTeleportationHistoryVector_t &history;  // line 445
	int nHistoryCount;  // line 447
	float *distStack;  // line 450
	const int nLastHistory;  // line 453
	float flTotalDistance;  // line 455
	float flQueryDist;  // line 469
	BlobData_t blobData;  // line 471
	float flPositionPercent;  // line 475
	// inlined Vector::operator=() at line 479
	// inlined Lerp<Vector>() at line 479
	// inlined CUtlVector<BlobTeleportationHistory_t,CUtlMemory<BlobTeleportationHistory_t, int> >::operator[]() at line 479
	// inlined CUtlVector<BlobTeleportationHistory_t,CUtlMemory<BlobTeleportationHistory_t, int> >::Count() at line 447
	// inlined CUtlVector<BlobTeleportationHistory_t,CUtlMemory<BlobTeleportationHistory_t, int> >::operator[]() at line 454
	// inlined Vector::operator-() at line 454
	// inlined Vector::Length() at line 454
	{
		int i;  // line 458
		// inlined CUtlVector<BlobTeleportationHistory_t,CUtlMemory<BlobTeleportationHistory_t, int> >::operator[]() at line 460
		// inlined CUtlVector<BlobTeleportationHistory_t,CUtlMemory<BlobTeleportationHistory_t, int> >::operator[]() at line 460
		// inlined Vector::operator-() at line 460
		// inlined Vector::Length() at line 460
	}
	// inlined CUtlVector<BlobTeleportationHistory_t,CUtlMemory<BlobTeleportationHistory_t, int> >::operator[]() at line 464
	// inlined Vector::operator-() at line 464
	// inlined Vector::Length() at line 464
	// inlined Lerp<float>() at line 473
	// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::AddToTail() at line 480
	{
		int i;  // line 485
		// inlined CUtlVector<BlobTeleportationHistory_t,CUtlMemory<BlobTeleportationHistory_t, int> >::operator[]() at line 490
		// inlined CUtlVector<BlobTeleportationHistory_t,CUtlMemory<BlobTeleportationHistory_t, int> >::operator[]() at line 490
		// inlined Lerp<Vector>() at line 490
		// inlined Vector::operator=() at line 490
		// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::AddToTail() at line 491
	}
	// inlined CUtlVector<BlobTeleportationHistory_t,CUtlMemory<BlobTeleportationHistory_t, int> >::operator[]() at line 497
	// inlined Lerp<Vector>() at line 497
	// inlined Vector::operator=() at line 497
	// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::AddToTail() at line 498
}

// game/client/portal2/c_paint_stream.cpp:502 @0x5282e0 _Z19InterpolateBlobDatafRK10CUtlVectorI10BlobData_t10CUtlMemoryIS0_iEES5_RS3_
InterpolateBlobData( float flPercent, const BlobDataVector_t &dataA, const BlobDataVector_t &dataB, BlobDataVector_t &blobDataVector )
{
	int a;  // line 504
	int b;  // line 505
	{
		const BlobData_t *blobA;  // line 509
		const BlobData_t *blobB;  // line 510
		// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::AddToTail() at line 546
		// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::operator[]() at line 509
		// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::operator[]() at line 510
		// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::AddToTail() at line 535
		{
			BlobData_t blobData;  // line 523
			// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::AddToTail() at line 527
			// inlined Vector::operator=() at line 526
			// inlined Lerp<Vector>() at line 526
			// inlined Lerp<float>() at line 525
		}
	}
}

// game/client/portal2/c_paint_stream.cpp:558 @0x528650 _ZN13C_PaintStream18GetCurrentBlobDataER10CUtlVectorI10BlobData_t10CUtlMemoryIS1_iEE
void C_PaintStream::GetCurrentBlobData( BlobDataVector_t &blobData )
{
	int nTotalFrame;  // line 560
	const float flCurrentTime;  // line 565
	float flOldestDataTime;  // line 568
	int nCurrentFrame;  // line 575
	int nLastFrame;  // line 576
	float flTimeA;  // line 577
	float flTimeB;  // line 577
	{
		const BlobDataVector_t &dataA;  // line 596
		const BlobDataVector_t &dataB;  // line 597
		float flDeltaTime;  // line 599
		float flPercent;  // line 600
	}
	// inlined CSharedUtlVectorBase<BlobDataPerUpdate_t,C_PaintStream::SharedVar_m_sharedBlobData>::Get() at line 568
	// inlined CSharedUtlVectorBase<BlobDataPerUpdate_t,C_PaintStream::SharedVar_m_sharedBlobData>::Count() at line 560
	{
		const BlobDataVector_t &temp;  // line 591
		// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::CopyArray() at line 592
		// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::Base() at line 592
		// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::Count() at line 592
	}
}

// public/tier1/utlvector.h:655 @0x52aeb0 _ZN10CUtlVectorIP14CBasePaintBlob10CUtlMemoryIS1_iEE10GrowVectorEi
void CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::GrowVector( int num )
{
	// inlined CUtlMemory<CBasePaintBlob*,int>::NumAllocated() at line 657
	// inlined CUtlMemory<CBasePaintBlob*,int>::Grow() at line 660
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::ResetDbgInfo() at line 664
}

// public/tier1/utlvector.h:655 @0x52b140 _ZN10CUtlVectorIP13C_PaintStream10CUtlMemoryIS1_iEE10GrowVectorEi
void CUtlVector<C_PaintStream*,CUtlMemory<C_PaintStream*, int> >::GrowVector( int num )
{
	// inlined CUtlMemory<C_PaintStream*,int>::NumAllocated() at line 657
	// inlined CUtlMemory<C_PaintStream*,int>::Grow() at line 660
	// inlined CUtlVector<C_PaintStream*,CUtlMemory<C_PaintStream*, int> >::ResetDbgInfo() at line 664
}

// public/tier1/utlvector.h:655 @0x52b260 _ZN10CUtlVectorI19BlobDataPerUpdate_t10CUtlMemoryIS0_iEE10GrowVectorEi
void CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> >::GrowVector( int num )
{
	// inlined CUtlMemory<BlobDataPerUpdate_t,int>::NumAllocated() at line 657
	// inlined CUtlMemory<BlobDataPerUpdate_t,int>::Grow() at line 660
	// inlined CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> >::ResetDbgInfo() at line 664
}

// public/tier1/utlvector.h:655 @0x52b380 _ZN10CUtlVectorI10BlobData_t10CUtlMemoryIS0_iEE10GrowVectorEi
void CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::GrowVector( int num )
{
	// inlined CUtlMemory<BlobData_t,int>::NumAllocated() at line 657
	// inlined CUtlMemory<BlobData_t,int>::Grow() at line 660
	// inlined CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> >::ResetDbgInfo() at line 664
}

// public/tier1/utlvector.h:655 @0x52b4a0 _ZN10CUtlVectorI11matrix3x4_t10CUtlMemoryIS0_iEE10GrowVectorEi
void CUtlVector<matrix3x4_t,CUtlMemory<matrix3x4_t, int> >::GrowVector( int num )
{
	// inlined CUtlMemory<matrix3x4_t,int>::NumAllocated() at line 657
	// inlined CUtlMemory<matrix3x4_t,int>::Grow() at line 660
	// inlined CUtlVector<matrix3x4_t,CUtlMemory<matrix3x4_t, int> >::ResetDbgInfo() at line 664
}

// public/tier1/utlvector.h:865 @0x52b5c0 _ZN10CUtlVectorI19BlobDataPerUpdate_t10CUtlMemoryIS0_iEE8SetCountEi
void CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> >::SetCount( int count )
{
	// inlined CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> >::RemoveAll() at line 867
	// inlined CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t, int> >::AddMultipleToTail() at line 868
}
