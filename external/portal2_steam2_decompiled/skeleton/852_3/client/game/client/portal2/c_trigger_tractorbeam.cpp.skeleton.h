// DWARF declaration skeleton for game/client/portal2/c_trigger_tractorbeam.cpp
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x79950 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
	// inlined PredMapInit<C_ProjectedTractorBeamEntity>() at line 22
	// inlined ClientClass::ClientClass() at line 25
	// inlined Cprojected_tractor_beam_entityFoo::Cprojected_tractor_beam_entityFoo() at line 29
	// inlined ClientClass::ClientClass() at line 65
	// inlined PredMapInit<C_Trigger_TractorBeam>() at line 91
	// inlined Ctrigger_tractorbeamFoo::Ctrigger_tractorbeamFoo() at line 96
	// inlined C_Trigger_TractorBeam_NonConformantDataHandler::C_Trigger_TractorBeam_NonConformantDataHandler() at line 641
	// inlined CTractorBeamFoo::CTractorBeamFoo() at line 687
}

// game/client/portal2/c_trigger_tractorbeam.cpp:22 @0x53ac10 _ZN28C_ProjectedTractorBeamEntity14GetPredDescMapEv
datamap_t *C_ProjectedTractorBeamEntity::GetPredDescMap()
{
}

// game/client/portal2/c_trigger_tractorbeam.cpp:22 (declaration)
datamap_t *PredMapInit<C_ProjectedTractorBeamEntity>( C_ProjectedTractorBeamEntity * );

// game/client/portal2/c_trigger_tractorbeam.cpp:22 @0x53ac20 _Z11PredMapInitI28C_ProjectedTractorBeamEntityEP9datamap_tPT_
datamap_t *PredMapInit<C_ProjectedTractorBeamEntity>( C_ProjectedTractorBeamEntity * )
{
}

// game/client/portal2/c_trigger_tractorbeam.cpp:22
datamap_t *g_PredMapHolder;

// game/client/portal2/c_trigger_tractorbeam.cpp:22
void C_ProjectedTractorBeamEntity::m_PredMap;

// game/client/portal2/c_trigger_tractorbeam.cpp:25 @0x53ac50 _ZN28C_ProjectedTractorBeamEntity40YouForgotToImplementOrDeclareClientClassEv
int C_ProjectedTractorBeamEntity::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/portal2/c_trigger_tractorbeam.cpp:25 @0x53ac70 _ZN28C_ProjectedTractorBeamEntity14GetClientClassEv
ClientClass *C_ProjectedTractorBeamEntity::GetClientClass()
{
}

// game/client/portal2/c_trigger_tractorbeam.cpp:25 @0x53ae80 _ZL42_C_ProjectedTractorBeamEntity_CreateObjectii
IClientNetworkable *_C_ProjectedTractorBeamEntity_CreateObject( int entnum, int serialNum )
{
	C_ProjectedTractorBeamEntity *pRet;  // line 25
	// inlined C_ProjectedTractorBeamEntity::C_ProjectedTractorBeamEntity() at line 25
}

// game/client/portal2/c_trigger_tractorbeam.cpp:25 @0x79800 _Z15ClientClassInitIN29DT_ProjectedTractorBeamEntity7ignoredEEiPT_
int ClientClassInit<DT_ProjectedTractorBeamEntity::ignored>( DT_ProjectedTractorBeamEntity::ignored * )
{
	char *pRecvTableName;  // line 25
	RecvTable &RecvTable;  // line 25
	RecvProp RecvProps[3];  // line 25
}

// game/client/portal2/c_trigger_tractorbeam.cpp:25
RecvTable g_RecvTable;

// game/client/portal2/c_trigger_tractorbeam.cpp:25
int g_RecvTableInit;

// game/client/portal2/c_trigger_tractorbeam.cpp:25
ClientClass __g_C_ProjectedTractorBeamEntityClientClass;

// game/client/portal2/c_trigger_tractorbeam.cpp:25
void C_ProjectedTractorBeamEntity::m_pClassRecvTable;

// game/client/portal2/c_trigger_tractorbeam.cpp:29 sizeof=0x1 (i386)
struct Cprojected_tractor_beam_entityFoo
{
public:
	Cprojected_tractor_beam_entityFoo();  // line 29
};

// game/client/portal2/c_trigger_tractorbeam.cpp:29 @0x53bfc0 _ZL36CC_ProjectedTractorBeamEntityFactoryv
C_BaseEntity *CC_ProjectedTractorBeamEntityFactory()
{
	// inlined C_ProjectedTractorBeamEntity::C_ProjectedTractorBeamEntity() at line 29
}

// game/client/portal2/c_trigger_tractorbeam.cpp:35 @0x53bef0 _ZN21C_Trigger_TractorBeam15RecvProxy_StartEPK14CRecvProxyDataPvS3_
void C_Trigger_TractorBeam::RecvProxy_Start( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_Trigger_TractorBeam *pBeam;  // line 37
	Vector vStart;  // line 40
	// inlined Vector::operator!=() at line 42
	// inlined Vector::operator=() at line 44
}

// game/client/portal2/c_trigger_tractorbeam.cpp:50 @0x53bb00 _ZN21C_Trigger_TractorBeam13RecvProxy_EndEPK14CRecvProxyDataPvS3_
void C_Trigger_TractorBeam::RecvProxy_End( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_Trigger_TractorBeam *pBeam;  // line 52
	Vector vEnd;  // line 55
	// inlined Vector::operator!=() at line 57
	// inlined Vector::operator=() at line 59
}

// game/client/portal2/c_trigger_tractorbeam.cpp:65 @0x53ac80 _ZN21C_Trigger_TractorBeam40YouForgotToImplementOrDeclareClientClassEv
int C_Trigger_TractorBeam::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/portal2/c_trigger_tractorbeam.cpp:65 @0x53aca0 _ZN21C_Trigger_TractorBeam14GetClientClassEv
ClientClass *C_Trigger_TractorBeam::GetClientClass()
{
}

// game/client/portal2/c_trigger_tractorbeam.cpp:65 @0x79290 _Z15ClientClassInitIN22DT_Trigger_TractorBeam7ignoredEEiPT_
int ClientClassInit<DT_Trigger_TractorBeam::ignored>( DT_Trigger_TractorBeam::ignored * )
{
	char *pRecvTableName;  // line 65
	RecvTable &RecvTable;  // line 65
	RecvProp RecvProps[23];  // line 65
}

// game/client/portal2/c_trigger_tractorbeam.cpp:65
RecvTable g_RecvTable;

// game/client/portal2/c_trigger_tractorbeam.cpp:65
int g_RecvTableInit;

// game/client/portal2/c_trigger_tractorbeam.cpp:65 @0x53d990 _ZL35_C_Trigger_TractorBeam_CreateObjectii
IClientNetworkable *_C_Trigger_TractorBeam_CreateObject( int entnum, int serialNum )
{
	C_Trigger_TractorBeam *pRet;  // line 65
	// inlined C_Trigger_TractorBeam::C_Trigger_TractorBeam() at line 65
}

// game/client/portal2/c_trigger_tractorbeam.cpp:65
ClientClass __g_C_Trigger_TractorBeamClientClass;

// game/client/portal2/c_trigger_tractorbeam.cpp:65
void C_Trigger_TractorBeam::m_pClassRecvTable;

// game/client/portal2/c_trigger_tractorbeam.cpp:91 @0x53acb0 _ZN21C_Trigger_TractorBeam14GetPredDescMapEv
datamap_t *C_Trigger_TractorBeam::GetPredDescMap()
{
}

// game/client/portal2/c_trigger_tractorbeam.cpp:91 (declaration)
datamap_t *PredMapInit<C_Trigger_TractorBeam>( C_Trigger_TractorBeam * );

// game/client/portal2/c_trigger_tractorbeam.cpp:91 @0x53acc0 _Z11PredMapInitI21C_Trigger_TractorBeamEP9datamap_tPT_
datamap_t *PredMapInit<C_Trigger_TractorBeam>( C_Trigger_TractorBeam * )
{
}

// game/client/portal2/c_trigger_tractorbeam.cpp:91
datamap_t *g_PredMapHolder;

// game/client/portal2/c_trigger_tractorbeam.cpp:91
void C_Trigger_TractorBeam::m_PredMap;

// game/client/portal2/c_trigger_tractorbeam.cpp:96 sizeof=0x1 (i386)
struct Ctrigger_tractorbeamFoo
{
public:
	Ctrigger_tractorbeamFoo();  // line 96
};

// game/client/portal2/c_trigger_tractorbeam.cpp:96 @0x53d9f0 _ZL29CC_Trigger_TractorBeamFactoryv
C_BaseEntity *CC_Trigger_TractorBeamFactory()
{
	// inlined C_Trigger_TractorBeam::C_Trigger_TractorBeam() at line 96
}

// game/client/portal2/c_trigger_tractorbeam.cpp:98 (declaration)
void C_Trigger_TractorBeam();

// game/client/portal2/c_trigger_tractorbeam.cpp:98 @0x53d7a0 _ZN21C_Trigger_TractorBeamC2Ev
C_Trigger_TractorBeam::C_Trigger_TractorBeam()
{
	// inlined CUtlVector<C_Trigger_TractorBeam*,CUtlMemory<C_Trigger_TractorBeam*, int> >::AddToTail() at line 102
	// inlined CUtlReference<CNewParticleEffect>::CUtlReference() at line 98
	// inlined CHandle<C_ProjectedTractorBeamEntity>::CHandle() at line 98
	// inlined C_BaseVPhysicsTrigger::C_BaseVPhysicsTrigger() at line 98
	// inlined CUtlReference<CNewParticleEffect>::~CUtlReference() at line 103
	// inlined C_BaseVPhysicsTrigger::~C_BaseVPhysicsTrigger() at line 103
}

// game/client/portal2/c_trigger_tractorbeam.cpp:98 @0x53d980 _ZN21C_Trigger_TractorBeamC1Ev
C_Trigger_TractorBeam::C_Trigger_TractorBeam()
{
}

// game/client/portal2/c_trigger_tractorbeam.cpp:105 (declaration)
~C_Trigger_TractorBeam();

// game/client/portal2/c_trigger_tractorbeam.cpp:105 @0x53d540 _ZN21C_Trigger_TractorBeamD2Ev
C_Trigger_TractorBeam::~C_Trigger_TractorBeam()
{
	// inlined CUtlVector<C_Trigger_TractorBeam*,CUtlMemory<C_Trigger_TractorBeam*, int> >::FindAndRemove() at line 107
	// inlined CUtlReference<CNewParticleEffect>::~CUtlReference() at line 108
	// inlined C_BaseVPhysicsTrigger::~C_BaseVPhysicsTrigger() at line 108
	// inlined CUtlReference<CNewParticleEffect>::~CUtlReference() at line 108
	// inlined C_BaseVPhysicsTrigger::~C_BaseVPhysicsTrigger() at line 108
}

// game/client/portal2/c_trigger_tractorbeam.cpp:105 @0x53da30 _ZN21C_Trigger_TractorBeamD0Ev
C_Trigger_TractorBeam::~C_Trigger_TractorBeam()
{
	// inlined CUtlVector<C_Trigger_TractorBeam*,CUtlMemory<C_Trigger_TractorBeam*, int> >::FindAndRemove() at line 107
	// inlined CUtlReference<CNewParticleEffect>::~CUtlReference() at line 108
	// inlined C_BaseVPhysicsTrigger::~C_BaseVPhysicsTrigger() at line 108
	// inlined CUtlReference<CNewParticleEffect>::~CUtlReference() at line 108
	// inlined C_BaseVPhysicsTrigger::~C_BaseVPhysicsTrigger() at line 108
}

// game/client/portal2/c_trigger_tractorbeam.cpp:105 @0x53dca0 _ZN21C_Trigger_TractorBeamD1Ev
C_Trigger_TractorBeam::~C_Trigger_TractorBeam()
{
	// inlined CUtlVector<C_Trigger_TractorBeam*,CUtlMemory<C_Trigger_TractorBeam*, int> >::FindAndRemove() at line 107
	// inlined CUtlReference<CNewParticleEffect>::~CUtlReference() at line 108
	// inlined C_BaseVPhysicsTrigger::~C_BaseVPhysicsTrigger() at line 108
	// inlined CUtlReference<CNewParticleEffect>::~CUtlReference() at line 108
	// inlined C_BaseVPhysicsTrigger::~C_BaseVPhysicsTrigger() at line 108
}

// game/client/portal2/c_trigger_tractorbeam.cpp:110 @0x53ad60 _ZN21C_Trigger_TractorBeam5SpawnEv
void C_Trigger_TractorBeam::Spawn()
{
}

// game/client/portal2/c_trigger_tractorbeam.cpp:132 @0x53b2e0 _ZN21C_Trigger_TractorBeam15CreateParticlesEv
void C_Trigger_TractorBeam::CreateParticles()
{
	{
		Vector vDir;  // line 149
		Vector vRight;  // line 152
		Vector vUp;  // line 152
		Vector vVelocity;  // line 163
		Vector color;  // line 166
		// inlined Vector::operator VectorByValue&() at line 181
		// inlined Vector::Vector() at line 181
		// inlined Vector::operator*() at line 163
		// inlined C_BaseEntity::Forward() at line 163
		// inlined Vector::operator VectorByValue&() at line 160
		// inlined Vector::operator-() at line 160
		// inlined Vector::NormalizeInPlace() at line 150
		// inlined Vector::operator-() at line 149
	}
	// inlined CUtlReference<CNewParticleEffect>::operator=() at line 142
	// inlined CUtlReference<CNewParticleEffect>::operator=() at line 138
	// inlined CUtlReference<CNewParticleEffect>::operator CNewParticleEffect*() at line 135
}

// game/client/portal2/c_trigger_tractorbeam.cpp:185 @0x53b980 _ZN21C_Trigger_TractorBeam14UpdateOnRemoveEv
void C_Trigger_TractorBeam::UpdateOnRemove()
{
	// inlined CUtlReference<CNewParticleEffect>::operator=() at line 209
	// inlined CUtlReference<CNewParticleEffect>::operator CNewParticleEffect*() at line 206
	{
		const int nNumObjects;  // line 190
		IPhysicsObject **pObjects;  // line 191
		{
			int i;  // line 194
		}
	}
}

// game/client/portal2/c_trigger_tractorbeam.cpp:215 @0x53b8d0 _ZN21C_Trigger_TractorBeam13OnDataChangedE16DataUpdateType_t
void C_Trigger_TractorBeam::OnDataChanged( DataUpdateType_t updateType )
{
}

// game/client/portal2/c_trigger_tractorbeam.cpp:242 @0x53bf70 _ZN21C_Trigger_TractorBeam18GetPredictionOwnerEv
C_BasePlayer *C_Trigger_TractorBeam::GetPredictionOwner()
{
	// inlined C_BaseEntity::GetSimulatingPlayer() at line 244
}

// game/client/portal2/c_trigger_tractorbeam.cpp:247 @0x53acf0 _ZN21C_Trigger_TractorBeam24UpdatePartitionListEntryEv
void C_Trigger_TractorBeam::UpdatePartitionListEntry()
{
}

// game/client/portal2/c_trigger_tractorbeam.cpp:259 @0x53bdd0 _ZN21C_Trigger_TractorBeam22GetSoundSpatializationER20SpatializationInfo_t
bool C_Trigger_TractorBeam::GetSoundSpatialization( SpatializationInfo_t &info )
{
	// inlined VectorCopy() at line 287
	{
		C_BasePlayer *pPlayer;  // line 267
		{
			float t;  // line 270
			// inlined Vector::operator=() at line 278
			// inlined Vector::operator=() at line 274
		}
	}
}

// game/client/portal2/c_trigger_tractorbeam.cpp:293 @0x53c050 _ZN21C_Trigger_TractorBeam10StartTouchEP12C_BaseEntity
void C_Trigger_TractorBeam::StartTouch( C_BaseEntity *pOther )
{
	{
		IPhysicsObject *pPhysObject;  // line 297
		// inlined C_BaseEntity::VPhysicsGetObject() at line 297
	}
	{
		C_BasePlayer *pPlayer;  // line 306
		// inlined ToBasePlayer() at line 306
		// inlined C_BaseEntity::SetGravity() at line 311
	}
}

// game/client/portal2/c_trigger_tractorbeam.cpp:318 @0x53bd40 _ZN21C_Trigger_TractorBeam8EndTouchEP12C_BaseEntity
void C_Trigger_TractorBeam::EndTouch( C_BaseEntity *pOther )
{
	{
		IPhysicsObject *pPhysObject;  // line 324
		// inlined C_BaseEntity::VPhysicsGetObject() at line 324
	}
	{
		C_BasePlayer *pPlayer;  // line 333
		// inlined ToBasePlayer() at line 333
		// inlined C_BaseEntity::SetGravity() at line 338
	}
}

// game/client/portal2/c_trigger_tractorbeam.cpp:346 @0x53ae70 _ZN21C_Trigger_TractorBeam15PhysicsSimulateEv
void C_Trigger_TractorBeam::PhysicsSimulate()
{
}

// game/client/portal2/c_trigger_tractorbeam.cpp:358 @0x53c0f0 _ZN21C_Trigger_TractorBeam17DrawColumnSegmentER12CMeshBuilderR6VectorRKS2_fS5_S5_fff
void C_Trigger_TractorBeam::DrawColumnSegment( CMeshBuilder &meshBuilder, Vector &vecStart, const Vector &vDir, float flLength, const Vector &vecXAxis, const Vector &vecYAxis, float flRadius, float flAlpha, float flTextureOffset )
{
	Vector vStart;  // line 361
	Vector vecPosition;  // line 362
	Vector vecLastPosition;  // line 363
	const float flRadStep;  // line 365
	float flSin;  // line 367
	float flCos;  // line 367
	float flV;  // line 368
	int r;  // line 370
	int g;  // line 370
	int b;  // line 370
	float flLastV;  // line 384
	// inlined Vector::operator=() at line 441
	// inlined Vector::operator+() at line 441
	// inlined Vector::operator*() at line 441
	{
		int i;  // line 387
		{
			Vector vert;  // line 399
			const float flU;  // line 401
			float pColors[4];  // line 404
			// inlined SinCos() at line 394
			// inlined Vector::operator*() at line 395
			// inlined Vector::operator*() at line 395
			// inlined Vector::operator*() at line 395
			// inlined Vector::operator*() at line 395
			// inlined Vector::operator+() at line 395
			// inlined Vector::operator+() at line 395
			// inlined Vector::operator=() at line 395
			// inlined CMeshBuilder::Color4fv() at line 410
			// inlined CMeshBuilder::TexCoord2f() at line 411
			// inlined CMeshBuilder::Position3fv() at line 412
			// inlined CMeshBuilder::AdvanceVertex() at line 413
			// inlined Vector::operator*() at line 416
			// inlined Vector::operator+() at line 416
			// inlined Vector::operator=() at line 416
			// inlined CMeshBuilder::Color4fv() at line 418
			// inlined CMeshBuilder::TexCoord2f() at line 419
			// inlined CMeshBuilder::Position3fv() at line 420
			// inlined CMeshBuilder::AdvanceVertex() at line 421
			// inlined Vector::operator*() at line 424
			// inlined Vector::operator+() at line 424
			// inlined Vector::operator=() at line 424
			// inlined CMeshBuilder::Color4fv() at line 426
			// inlined CMeshBuilder::TexCoord2f() at line 427
			// inlined CMeshBuilder::Position3fv() at line 428
			// inlined CMeshBuilder::AdvanceVertex() at line 429
			// inlined Vector::operator=() at line 432
			// inlined CMeshBuilder::Color4fv() at line 434
			// inlined CMeshBuilder::TexCoord2f() at line 435
			// inlined CMeshBuilder::Position3fv() at line 436
			// inlined CMeshBuilder::AdvanceVertex() at line 437
		}
	}
	// inlined Vector::operator+() at line 361
	// inlined Vector::operator*() at line 361
}

// game/client/portal2/c_trigger_tractorbeam.cpp:449 @0x53cac0 _ZN21C_Trigger_TractorBeam10DrawColumnEP9IMaterialRK6VectorS4_fS4_S4_ffbbf
void C_Trigger_TractorBeam::DrawColumn( IMaterial *pMaterial, const Vector &vecStart, const Vector &vDir, float flLength, const Vector &vecXAxis, const Vector &vecYAxis, float flRadius, float flAlpha, bool bPinchIn, bool bPinchOut, float flTextureOffset )
{
	CMatRenderContextPtr pRenderContext;  // line 451
	IMesh *pMesh;  // line 454
	CMeshBuilder meshBuilder;  // line 455
	int nTotalSegments;  // line 457
	float flMainLength;  // line 467
	Vector vStart;  // line 475
	// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 515
	// inlined CMeshBuilder::~CMeshBuilder() at line 515
	// inlined CMeshBuilder::End() at line 514
	// inlined CMeshBuilder::Begin() at line 465
	// inlined CMeshBuilder::CMeshBuilder() at line 455
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 454
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 452
	// inlined CMatRenderContextPtr::CMatRenderContextPtr() at line 451
	// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 515
	// inlined CMeshBuilder::~CMeshBuilder() at line 515
}

// game/client/portal2/c_trigger_tractorbeam.cpp:519 @0x53ad50 _ZN21C_Trigger_TractorBeam10ShouldDrawEv
bool C_Trigger_TractorBeam::ShouldDraw()
{
}

// game/client/portal2/c_trigger_tractorbeam.cpp:525 @0x53d1f0 _ZN21C_Trigger_TractorBeam9DrawModelEiRK20RenderableInstance_t
int C_Trigger_TractorBeam::DrawModel( int flags, const RenderableInstance_t &instance )
{
	Vector vDir;  // line 528
	float flLength;  // line 529
	float flStartDelta;  // line 532
	QAngle angles;  // line 539
	float flRadius;  // line 542
	matrix3x4_t xform;  // line 545
	Vector xAxis;  // line 547
	Vector yAxis;  // line 547
	// inlined SimpleSplineRemapValClamped() at line 535
	// inlined Vector::operator-() at line 528
}

// game/client/portal2/c_trigger_tractorbeam.cpp:564 @0x53bb80 _ZN21C_Trigger_TractorBeam21GetToolRecordingStateEP9KeyValues
void C_Trigger_TractorBeam::GetToolRecordingState( KeyValues *msg )
{
	KeyValues *pKV;  // line 568
	Vector vStart;  // line 577
	Vector vEnd;  // line 583
	// inlined C_Trigger_TractorBeam::GetStartPoint() at line 577
	// inlined Vector::operator VectorByValue&() at line 577
	// inlined C_Trigger_TractorBeam::GetEndPoint() at line 583
	// inlined Vector::operator VectorByValue&() at line 583
}

// game/client/portal2/c_trigger_tractorbeam.cpp:589 @0x53b0c0 _ZN21C_Trigger_TractorBeam26RestoreToToolRecordedStateEP9KeyValues
void C_Trigger_TractorBeam::RestoreToToolRecordedState( KeyValues *pKV )
{
	// inlined Vector::Vector() at line 603
	// inlined Vector::operator VectorByValue&() at line 603
	// inlined Vector::operator-() at line 603
	// inlined Vector::operator VectorByValue&() at line 603
}

// game/client/portal2/c_trigger_tractorbeam.cpp:611 sizeof=0x20 (i386)
struct C_Trigger_TractorBeam_NonConformantDataHandler : public CIFM_EntityKeyValuesHandler_RecreateEntities
{
public:
	C_Trigger_TractorBeam_NonConformantDataHandler();  // line 613
	virtual void *CreateInstance();  // line 617
	virtual void DestroyInstance( void * );  // line 622
	virtual void HandleInstance( void *, KeyValues * );  // line 629
};

// game/client/portal2/c_trigger_tractorbeam.cpp:611 (declaration)
~C_Trigger_TractorBeam_NonConformantDataHandler();

// game/client/portal2/c_trigger_tractorbeam.cpp:617 @0x53e390 _ZN46C_Trigger_TractorBeam_NonConformantDataHandler14CreateInstanceEv
void *C_Trigger_TractorBeam_NonConformantDataHandler::CreateInstance()
{
	// inlined C_Trigger_TractorBeam::C_Trigger_TractorBeam() at line 619
}

// game/client/portal2/c_trigger_tractorbeam.cpp:622 @0x53df10 _ZN46C_Trigger_TractorBeam_NonConformantDataHandler15DestroyInstanceEPv
void C_Trigger_TractorBeam_NonConformantDataHandler::DestroyInstance( void *pEntity )
{
	C_Trigger_TractorBeam *pCastEntity;  // line 624
}

// game/client/portal2/c_trigger_tractorbeam.cpp:629 @0x53e250 _ZN46C_Trigger_TractorBeam_NonConformantDataHandler14HandleInstanceEPvP9KeyValues
void C_Trigger_TractorBeam_NonConformantDataHandler::HandleInstance( void *pEntity, KeyValues *pKeyValues )
{
	C_Trigger_TractorBeam *pCastEntity;  // line 631
}

// game/client/portal2/c_trigger_tractorbeam.cpp:641
static C_Trigger_TractorBeam_NonConformantDataHandler s_TractorBeamEntityIFMHandler;

// game/client/portal2/c_trigger_tractorbeam.cpp:643 @0x53b050 _ZN21C_Trigger_TractorBeam19OnNewParticleEffectEPKcP18CNewParticleEffect
void C_Trigger_TractorBeam::OnNewParticleEffect( const char *pszParticleName, CNewParticleEffect *pNewParticleEffect )
{
}

// game/client/portal2/c_trigger_tractorbeam.cpp:653 sizeof=0xc (i386)
struct CTractorBeamProxy : public CResultProxy
{
public:
	virtual bool Init( IMaterial *, KeyValues * );  // line 655
	virtual void OnBind( void * );  // line 656
};

// game/client/portal2/c_trigger_tractorbeam.cpp:653 (declaration)
~CTractorBeamProxy();

// game/client/portal2/c_trigger_tractorbeam.cpp:653 @0x53e1f0 _ZN17CTractorBeamProxyD0Ev
CTractorBeamProxy::~CTractorBeamProxy()
{
}

// game/client/portal2/c_trigger_tractorbeam.cpp:653 @0x53e230 _ZN17CTractorBeamProxyD1Ev
CTractorBeamProxy::~CTractorBeamProxy()
{
}

// game/client/portal2/c_trigger_tractorbeam.cpp:653 (declaration)
void CTractorBeamProxy();

// game/client/portal2/c_trigger_tractorbeam.cpp:655 @0x53b020 _ZN17CTractorBeamProxy4InitEP9IMaterialP9KeyValues
bool CTractorBeamProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
}

// game/client/portal2/c_trigger_tractorbeam.cpp:656 @0x53af90 _ZN17CTractorBeamProxy6OnBindEPv
void CTractorBeamProxy::OnBind( void *pC_BaseEntity )
{
	C_BaseEntity *pEntity;  // line 676
	C_Trigger_TractorBeam *pBeam;  // line 678
	{
		float flSpeed;  // line 681
	}
}

// game/client/portal2/c_trigger_tractorbeam.cpp:687 sizeof=0x1 (i386)
struct CTractorBeamFoo
{
public:
	CTractorBeamFoo();  // line 687
};

// game/client/portal2/c_trigger_tractorbeam.cpp:687 @0x53af30 _ZL25CCTractorBeamProxyFactoryv
IMaterialProxy *CCTractorBeamProxyFactory()
{
	// inlined CTractorBeamProxy::CTractorBeamProxy() at line 687
}

// public/tier1/utlmemory.h:707 @0x53df80 _ZN10CUtlMemoryIP21C_Trigger_TractorBeamiE4GrowEi
void CUtlMemory<C_Trigger_TractorBeam*,int>::Grow( int num )
{
	int nAllocationRequested;  // line 720
	int nNewAllocationCount;  // line 724
	// inlined UtlMemory_CalcNewAllocationCount() at line 724
	// inlined CUtlMemory<C_Trigger_TractorBeam*,int>::IsExternallyAllocated() at line 711
	// inlined MemAlloc_Alloc() at line 761
}
