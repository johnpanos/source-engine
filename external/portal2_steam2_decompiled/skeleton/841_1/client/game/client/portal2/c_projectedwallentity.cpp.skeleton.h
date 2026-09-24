// DWARF declaration skeleton for game/client/portal2/c_projectedwallentity.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x783c0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CResourcePrecacher::CResourcePrecacher() at line 52
	// inlined CUtlVector<IProjectedWallEntityAutoList*,CUtlMemory<IProjectedWallEntityAutoList*, int> >::CUtlVector() at line 55
	// inlined ClientClass::ClientClass() at line 58
	// inlined PredMapInit<C_ProjectedWallEntity>() at line 79
	// inlined Cprojected_wall_entityFoo::Cprojected_wall_entityFoo() at line 87
	// inlined CProjectedWallEntity_NonConformantDataHandler::CProjectedWallEntity_NonConformantDataHandler() at line 1268
	// inlined C_TEWallPaintedEvent::C_TEWallPaintedEvent() at line 1295
	// inlined ClientClass::ClientClass() at line 1295
}

// game/client/portal2/c_projectedwallentity.cpp:35
ConVar cl_projected_wall_projection_speed;

// game/client/portal2/c_projectedwallentity.cpp:36
ConVar cl_paintable_projected_wall_texture_wrap_rate;

// game/client/portal2/c_projectedwallentity.cpp:37
ConVar cl_draw_projected_wall_with_paint;

// game/client/portal2/c_projectedwallentity.cpp:39
ConVar debug_projected_wall_drawing;

// game/client/portal2/c_projectedwallentity.cpp:44 sizeof=0x10 (i386)
struct CResourcePrecacher : public CBaseResourcePrecacher
{
public:
	CResourcePrecacher();  // line 44
	virtual void Cache( IPrecacheHandler *, bool, ResourceList_t, bool );  // line 44
};

// game/client/portal2/c_projectedwallentity.cpp:44 @0x52ed50 _ZN41PrecacheEffectProjectedWallEntityPrecache18CResourcePrecacher5CacheEP16IPrecacheHandlerbP16ResourceList_t__b
void CResourcePrecacher::Cache( IPrecacheHandler *pPrecacheHandler, bool bPrecache, ResourceList_t hResourceList, bool bIgnoreConditionals )
{
}

// game/client/portal2/c_projectedwallentity.cpp:52
PrecacheEffectProjectedWallEntityPrecache::CResourcePrecacher s_ResourcePrecacher;

// game/client/portal2/c_projectedwallentity.cpp:55 (declaration)
~IProjectedWallEntityAutoList();

// game/client/portal2/c_projectedwallentity.cpp:55 @0x530160 _ZN28IProjectedWallEntityAutoListD1Ev
IProjectedWallEntityAutoList::~IProjectedWallEntityAutoList()
{
	// inlined IProjectedWallEntityAutoList::Remove() at line 55
}

// game/client/portal2/c_projectedwallentity.cpp:55 (declaration)
void IProjectedWallEntityAutoList( bool bAutoAdd );

// game/client/portal2/c_projectedwallentity.cpp:55 @0x530ad0 _ZN28IProjectedWallEntityAutoListC1Eb
IProjectedWallEntityAutoList::IProjectedWallEntityAutoList( bool bAutoAdd )
{
	// inlined IProjectedWallEntityAutoList::Add() at line 55
}

// game/client/portal2/c_projectedwallentity.cpp:55 @0x530c90 _ZN28IProjectedWallEntityAutoListD2Ev
IProjectedWallEntityAutoList::~IProjectedWallEntityAutoList()
{
	// inlined IProjectedWallEntityAutoList::Remove() at line 55
}

// game/client/portal2/c_projectedwallentity.cpp:55 @0x530d30 _ZN28IProjectedWallEntityAutoListD0Ev
IProjectedWallEntityAutoList::~IProjectedWallEntityAutoList()
{
	// inlined IProjectedWallEntityAutoList::Remove() at line 55
}

// game/client/portal2/c_projectedwallentity.cpp:55 @0x532300 _ZN28IProjectedWallEntityAutoListC2Eb
IProjectedWallEntityAutoList::IProjectedWallEntityAutoList( bool bAutoAdd )
{
	// inlined IProjectedWallEntityAutoList::Add() at line 55
}

// game/client/portal2/c_projectedwallentity.cpp:55
void IProjectedWallEntityAutoList::m_IProjectedWallEntityAutoListAutoList;

// game/client/portal2/c_projectedwallentity.cpp:58 @0x52ee80 _ZN21C_ProjectedWallEntity40YouForgotToImplementOrDeclareClientClassEv
int C_ProjectedWallEntity::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/portal2/c_projectedwallentity.cpp:58 @0x52eea0 _ZN21C_ProjectedWallEntity14GetClientClassEv
ClientClass *C_ProjectedWallEntity::GetClientClass()
{
}

// game/client/portal2/c_projectedwallentity.cpp:58 @0x78020 _Z15ClientClassInitIN22DT_ProjectedWallEntity7ignoredEEiPT_
int ClientClassInit<DT_ProjectedWallEntity::ignored>( DT_ProjectedWallEntity::ignored * )
{
	char *pRecvTableName;  // line 58
	RecvTable &RecvTable;  // line 58
	RecvProp RecvProps[14];  // line 58
}

// game/client/portal2/c_projectedwallentity.cpp:58
RecvTable g_RecvTable;

// game/client/portal2/c_projectedwallentity.cpp:58
int g_RecvTableInit;

// game/client/portal2/c_projectedwallentity.cpp:58 @0x532700 _ZL35_C_ProjectedWallEntity_CreateObjectii
IClientNetworkable *_C_ProjectedWallEntity_CreateObject( int entnum, int serialNum )
{
	C_ProjectedWallEntity *pRet;  // line 58
	// inlined C_ProjectedWallEntity::C_ProjectedWallEntity() at line 58
}

// game/client/portal2/c_projectedwallentity.cpp:58
ClientClass __g_C_ProjectedWallEntityClientClass;

// game/client/portal2/c_projectedwallentity.cpp:58
void C_ProjectedWallEntity::m_pClassRecvTable;

// game/client/portal2/c_projectedwallentity.cpp:79 @0x52eeb0 _ZN21C_ProjectedWallEntity14GetPredDescMapEv
datamap_t *C_ProjectedWallEntity::GetPredDescMap()
{
}

// game/client/portal2/c_projectedwallentity.cpp:79 (declaration)
datamap_t *PredMapInit<C_ProjectedWallEntity>( C_ProjectedWallEntity * );

// game/client/portal2/c_projectedwallentity.cpp:79 @0x52eec0 _Z11PredMapInitI21C_ProjectedWallEntityEP9datamap_tPT_
datamap_t *PredMapInit<C_ProjectedWallEntity>( C_ProjectedWallEntity * )
{
}

// game/client/portal2/c_projectedwallentity.cpp:79
datamap_t *g_PredMapHolder;

// game/client/portal2/c_projectedwallentity.cpp:79
void C_ProjectedWallEntity::m_PredMap;

// game/client/portal2/c_projectedwallentity.cpp:87 sizeof=0x1 (i386)
struct Cprojected_wall_entityFoo
{
public:
	Cprojected_wall_entityFoo();  // line 87
};

// game/client/portal2/c_projectedwallentity.cpp:87 @0x5326c0 _ZL29CC_ProjectedWallEntityFactoryv
C_BaseEntity *CC_ProjectedWallEntityFactory()
{
	// inlined C_ProjectedWallEntity::C_ProjectedWallEntity() at line 87
}

// game/client/portal2/c_projectedwallentity.cpp:90 (declaration)
void C_ProjectedWallEntity();

// game/client/portal2/c_projectedwallentity.cpp:90 @0x5324c0 _ZN21C_ProjectedWallEntityC2Ev
C_ProjectedWallEntity::C_ProjectedWallEntity()
{
	// inlined CUtlVector<PaintPowerType,CUtlMemory<PaintPowerType, int> >::CUtlVector() at line 90
	// inlined CHandle<C_BaseEntity>::CHandle() at line 90
	// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::CUtlVector() at line 90
	// inlined CPaintableEntity<C_BaseProjectedEntity>::CPaintableEntity() at line 90
	// inlined CPaintableEntity<C_BaseProjectedEntity>::~CPaintableEntity() at line 106
}

// game/client/portal2/c_projectedwallentity.cpp:90 @0x5326b0 _ZN21C_ProjectedWallEntityC1Ev
C_ProjectedWallEntity::C_ProjectedWallEntity()
{
}

// game/client/portal2/c_projectedwallentity.cpp:108 @0x530200 _ZN21C_ProjectedWallEntity14UpdateOnRemoveEv
void C_ProjectedWallEntity::UpdateOnRemove()
{
	// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::RemoveAll() at line 117
	{
		int i;  // line 112
	}
}

// game/client/portal2/c_projectedwallentity.cpp:122 @0x52eef0 _ZN21C_ProjectedWallEntity14GetCollideTypeEv
CollideType_t C_ProjectedWallEntity::GetCollideType()
{
}

// game/client/portal2/c_projectedwallentity.cpp:128 (declaration)
void DrawQuadHelper( CMeshBuilder *meshBuilder, const Vector &vOrigin, const Vector &vRight, const Vector &vUp, float flTextureScaleU, float flTextureScaleV, float flAlpha );

// game/client/portal2/c_projectedwallentity.cpp:183 @0x534cf0 _ZN21C_ProjectedWallEntity9DrawModelEiRK20RenderableInstance_t
int C_ProjectedWallEntity::DrawModel( int flags, const RenderableInstance_t &instance )
{
	{
		CMatRenderContextPtr pRenderContext;  // line 193
		IMesh *pMesh;  // line 195
		CMeshBuilder meshBuilder;  // line 196
		IMesh *pMeshMid;  // line 200
		CMeshBuilder meshBuilderPaintMid;  // line 201
		IMesh *pMeshEnd1;  // line 204
		CMeshBuilder meshBuilderPaintEnd1;  // line 205
		IMesh *pMeshEnd2;  // line 208
		CMeshBuilder meshBuilderPaintEnd2;  // line 209
		IMesh *pMeshSing;  // line 212
		CMeshBuilder meshBuilderPaintSing;  // line 213
		// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 214
		// inlined CMatRenderContextPtr::CMatRenderContextPtr() at line 193
		// inlined CMeshBuilder::CMeshBuilder() at line 196
		// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 200
		// inlined CMeshBuilder::CMeshBuilder() at line 201
		// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 204
		// inlined CMeshBuilder::CMeshBuilder() at line 205
		// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 208
		// inlined CMeshBuilder::CMeshBuilder() at line 209
		// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 212
		// inlined CMeshBuilder::CMeshBuilder() at line 213
		// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 214
	}
	{
		Vector vForward;  // line 218
		Vector vRight;  // line 218
		Vector vUp;  // line 218
		Vector vOrigin;  // line 220
		CMatRenderContextPtr pRenderContext;  // line 222
		IMesh *pMesh;  // line 224
		CMeshBuilder meshBuilder;  // line 225
		float flLengthTexRate;  // line 241
		float flWidthTextRate;  // line 242
		CMeshBuilder meshBuilder2;  // line 274
		// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 284
		// inlined CMeshBuilder::~CMeshBuilder() at line 284
		// inlined CMeshBuilder::End() at line 283
		// inlined CMeshBuilder::End() at line 266
		// inlined C_ProjectedWallEntity::DrawQuadHelper() at line 264
		// inlined Vector::operator+() at line 264
		// inlined CMeshBuilder::Begin() at line 244
		// inlined Vector::operator*=() at line 235
		// inlined Vector::operator*=() at line 234
		// inlined Vector::operator*=() at line 233
		// inlined GameRules() at line 227
		// inlined CMeshBuilder::CMeshBuilder() at line 225
		// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 224
		// inlined CMatRenderContextPtr::CMatRenderContextPtr() at line 222
		// inlined Vector::operator+() at line 220
		// inlined Vector::operator/() at line 220
		// inlined Vector::operator*() at line 220
		// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 273
		// inlined CMeshBuilder::CMeshBuilder() at line 274
		// inlined CMeshBuilder::Begin() at line 276
		// inlined Vector::operator*=() at line 279
		// inlined Vector::operator+() at line 280
		// inlined C_ProjectedWallEntity::DrawQuadHelper() at line 280
		// inlined Vector::operator-() at line 281
		// inlined C_ProjectedWallEntity::DrawQuadHelper() at line 281
		// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 284
		// inlined CMeshBuilder::~CMeshBuilder() at line 284
		// inlined CMeshBuilder::~CMeshBuilder() at line 284
	}
}

// game/client/portal2/c_projectedwallentity.cpp:310 @0x533c00 _ZN21C_ProjectedWallEntity18PaintWallWithPaintEP5IMesh12CMeshBuilder24ProjectedWallSegmentType
void C_ProjectedWallEntity::PaintWallWithPaint( IMesh *pMesh, CMeshBuilder &meshBuilder, ProjectedWallSegmentType type )
{
	Vector vecForward;  // line 312
	Vector vecRight;  // line 312
	Vector vecUp;  // line 312
	Vector vecForwardDir;  // line 312
	Vector vecRightDir;  // line 312
	Vector vecUpDir;  // line 312
	Vector vecOrigin;  // line 316
	bool bDebugging;  // line 318
	float flDrawnLength;  // line 326
	float flPrevSegmentLength;  // line 327
	float flCurrentSegmentLength;  // line 328
	Vector vecOriginalOrigin;  // line 343
	Vector vecOriginalForward;  // line 344
	{
		int i;  // line 346
		{
			float flOverDraw;  // line 409
			{
				int nPrevSeg;  // line 354
				int nNextSeg;  // line 355
				// inlined CMeshBuilder::~CMeshBuilder() at line 368
				// inlined CMeshBuilder::~CMeshBuilder() at line 363
				// inlined CMeshBuilder::~CMeshBuilder() at line 358
				// inlined CMeshBuilder::~CMeshBuilder() at line 373
				// inlined CMeshBuilder::~CMeshBuilder() at line 373
				// inlined CUtlVector<PaintPowerType,CUtlMemory<PaintPowerType, int> >::operator[]() at line 372
				// inlined CUtlVector<PaintPowerType,CUtlMemory<PaintPowerType, int> >::operator[]() at line 356
				// inlined CMeshBuilder::~CMeshBuilder() at line 358
				// inlined CMeshBuilder::~CMeshBuilder() at line 363
				// inlined CMeshBuilder::~CMeshBuilder() at line 368
			}
			// inlined CMeshBuilder::~CMeshBuilder() at line 350
			// inlined Vector::operator=() at line 420
			{
				Color color;  // line 385
				Vector vecMins;  // line 391
				Vector vecMaxs;  // line 391
				// inlined Color::Color() at line 385
				// inlined Color::Color() at line 388
				// inlined Color::operator=() at line 388
				// inlined Vector::operator-() at line 393
				// inlined Vector::operator*() at line 393
				// inlined Vector::operator/() at line 393
				// inlined Vector::operator*() at line 394
				// inlined Vector::operator/() at line 394
				// inlined Vector::operator-=() at line 394
				// inlined Vector::operator*() at line 395
				// inlined Vector::operator-=() at line 395
				// inlined Vector::operator*() at line 396
				// inlined Vector::operator/() at line 396
				// inlined Vector::operator/() at line 397
				// inlined Vector::operator+=() at line 397
				// inlined Vector::operator*() at line 398
				// inlined Vector::operator+=() at line 398
			}
			// inlined Vector::operator*() at line 420
			// inlined Vector::operator/() at line 420
			// inlined Vector::operator*() at line 421
			// inlined Vector::operator+=() at line 421
			// inlined CMeshBuilder::~CMeshBuilder() at line 350
		}
	}
	// inlined Vector::operator+=() at line 334
	// inlined Vector::operator*() at line 334
	// inlined Vector::operator=() at line 333
	// inlined Vector::operator*() at line 333
	// inlined Vector::operator=() at line 315
	// inlined Vector::operator/() at line 315
	// inlined Vector::operator*() at line 315
	// inlined Vector::operator=() at line 314
	// inlined Vector::operator/() at line 314
	// inlined Vector::operator*() at line 314
	// inlined Vector::operator*() at line 338
	// inlined Vector::operator=() at line 338
	// inlined Vector::operator*() at line 339
	// inlined Vector::operator+=() at line 339
	{
		Vector vecWallEnd;  // line 322
		// inlined Vector::operator*() at line 322
		// inlined Vector::operator+() at line 322
	}
}

// game/client/portal2/c_projectedwallentity.cpp:425 @0x532760 _ZN21C_ProjectedWallEntity25PaintWallWithPaintSegmentEP5IMesh12CMeshBuilder24ProjectedWallSegmentType6VectorS4_S4_S4_
void C_ProjectedWallEntity::PaintWallWithPaintSegment( IMesh *pMesh, CMeshBuilder &meshBuilder, ProjectedWallSegmentType type, Vector vecOrigin, Vector vecUp, Vector vecForward, Vector vecRight )
{
	float flLengthTexRate;  // line 427
	float flWidthTextRate;  // line 428
	Vector vecNewOrigin;  // line 430
	// inlined CMeshBuilder::End() at line 444
	// inlined C_ProjectedWallEntity::DrawQuadHelper() at line 442
	// inlined Vector::operator-() at line 442
	// inlined C_ProjectedWallEntity::DrawQuadHelper() at line 441
	// inlined Vector::operator+() at line 441
	// inlined C_ProjectedWallEntity::DrawQuadHelper() at line 439
	// inlined Vector::operator+() at line 439
	// inlined CMeshBuilder::Begin() at line 434
}

// game/client/portal2/c_projectedwallentity.cpp:448 @0x530830 _ZN21C_ProjectedWallEntity17ColorWallByPortalEP9IMaterial
void C_ProjectedWallEntity::ColorWallByPortal( IMaterial *pBody )
{
	float flColors[3];  // line 450
	// inlined CHandle<C_BaseEntity>::Get() at line 457
	{
		C_Prop_Portal *pPortal;  // line 459
		// inlined CHandle<C_BaseEntity>::Get() at line 462
		{
			Color clrPortal;  // line 464
		}
	}
}

// game/client/portal2/c_projectedwallentity.cpp:476 @0x52ef00 _ZN21C_ProjectedWallEntity13InitMaterialsEv
bool C_ProjectedWallEntity::InitMaterials()
{
	bool bFound;  // line 535
}

// game/client/portal2/c_projectedwallentity.cpp:575 @0x530290 _ZN21C_ProjectedWallEntity15GetRenderBoundsER6VectorS1_
void C_ProjectedWallEntity::GetRenderBounds( Vector &vecMins, Vector &vecMaxs )
{
	// inlined Vector::operator-() at line 577
	// inlined Vector::operator=() at line 577
	// inlined Vector::operator-() at line 578
	// inlined Vector::operator=() at line 578
}

// game/client/portal2/c_projectedwallentity.cpp:583 @0x52f270 _ZN21C_ProjectedWallEntity15GetRenderAnglesEv
const QAngle &C_ProjectedWallEntity::GetRenderAngles()
{
}

// game/client/portal2/c_projectedwallentity.cpp:588 @0x52f280 _ZN21C_ProjectedWallEntity23ComputeTranslucencyTypeEv
RenderableTranslucencyType_t C_ProjectedWallEntity::ComputeTranslucencyType()
{
}

// game/client/portal2/c_projectedwallentity.cpp:593 @0x52f340 _ZN21C_ProjectedWallEntity16OnPreDataChangedE16DataUpdateType_t
void C_ProjectedWallEntity::OnPreDataChanged( DataUpdateType_t type )
{
}

// game/client/portal2/c_projectedwallentity.cpp:600 @0x530530 _ZN21C_ProjectedWallEntity13OnDataChangedE16DataUpdateType_t
void C_ProjectedWallEntity::OnDataChanged( DataUpdateType_t updateType )
{
}

// game/client/portal2/c_projectedwallentity.cpp:622 @0x5322d0 _ZN21C_ProjectedWallEntity11OnProjectedEv
void C_ProjectedWallEntity::OnProjected()
{
}

// game/client/portal2/c_projectedwallentity.cpp:629 @0x530110 _ZN21C_ProjectedWallEntity5SpawnEv
void C_ProjectedWallEntity::Spawn()
{
}

// game/client/portal2/c_projectedwallentity.cpp:635 @0x52fd80 _ZN21C_ProjectedWallEntity11ClientThinkEv
void C_ProjectedWallEntity::ClientThink()
{
	float flDisplaySpeed;  // line 639
	// inlined ConVar::GetFloat() at line 639
}

// public/tier1/utlvector.h:655 @0x537800 _ZN10CUtlVectorI14PaintPowerType10CUtlMemoryIS0_iEE10GrowVectorEi
void CUtlVector<PaintPowerType,CUtlMemory<PaintPowerType, int> >::GrowVector( int num )
{
	// inlined CUtlMemory<PaintPowerType,int>::NumAllocated() at line 657
	// inlined CUtlMemory<PaintPowerType,int>::Grow() at line 660
	// inlined CUtlVector<PaintPowerType,CUtlMemory<PaintPowerType, int> >::ResetDbgInfo() at line 664
}

// public/tier1/utlvector.h:655 @0x537920 _ZN10CUtlVectorIN21C_ProjectedWallEntity23WallCollideableAtTime_tE10CUtlMemoryIS1_iEE10GrowVectorEi
void CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::GrowVector( int num )
{
	// inlined CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t,int>::NumAllocated() at line 657
	// inlined CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t,int>::Grow() at line 660
	// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::ResetDbgInfo() at line 664
}

// game/client/portal2/c_projectedwallentity.cpp:665 @0x530dd0 _ZN21C_ProjectedWallEntity11ProjectWallEv
void C_ProjectedWallEntity::ProjectWall()
{
	Vector vWallStartPoint;  // line 671
	Vector vWallEndPoint;  // line 672
	bool bCreate;  // line 686
	int iReplaceIndex;  // line 687
	int iValidIndex;  // line 688
	Vector vecForward;  // line 803
	Vector vecRight;  // line 803
	Vector vecUp;  // line 803
	// inlined CUtlVector<PaintPowerType,CUtlMemory<PaintPowerType, int> >::SetCount() at line 940
	// inlined Vector::Length() at line 928
	// inlined Vector::operator-() at line 928
	{
		Vector vSetMins;  // line 914
		Vector vSetMaxs;  // line 915
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 911
		// inlined Vector::operator=() at line 911
		// inlined Vector::operator=() at line 912
		// inlined Vector::operator-() at line 914
		// inlined Vector::operator-() at line 915
	}
	{
		CPhysCollide *pWallCollideable;  // line 808
		Vector vWorldMins;  // line 876
		Vector vWorldMaxs;  // line 876
		C_ProjectedWallEntity::WallCollideableAtTime_t temp;  // line 881
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::AddToTail() at line 905
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::Count() at line 904
		// inlined C_ProjectedWallEntity::WallCollideableAtTime_t::operator=() at line 896
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 896
		// inlined QAngle::operator=() at line 886
		// inlined Vector::operator=() at line 885
		// inlined Vector::operator=() at line 884
		// inlined Vector::operator=() at line 883
		// inlined Vector::operator=() at line 882
		{
			Vector vScaledRight;  // line 811
			Vector vStart;  // line 812
			Vector vEnd;  // line 813
			Vector vVerts[4];  // line 815
			Vector *pVerts[4];  // line 821
			CPhysConvex *pTempConvex;  // line 822
			// inlined Vector::operator*() at line 811
			// inlined Vector::operator+() at line 816
			// inlined Vector::operator=() at line 816
			// inlined Vector::operator-() at line 817
			// inlined Vector::operator=() at line 817
			// inlined Vector::operator-() at line 818
			// inlined Vector::operator=() at line 818
			// inlined Vector::operator+() at line 819
			// inlined Vector::operator=() at line 819
		}
		{
			float flWallPlanes[24];  // line 828
			Vector vWallPointRight;  // line 835
			Vector vWallPointLeft;  // line 839
			Vector vWallPointUp;  // line 843
			Vector vWallPointDown;  // line 847
			CPolyhedron *pTempPolyhedron;  // line 852
			CPhysConvex *pTempConvex;  // line 859
			// inlined Vector::operator-() at line 849
			// inlined Vector::CopyToArray() at line 848
			// inlined Vector::operator-() at line 848
			// inlined Vector::operator-() at line 847
			// inlined Vector::operator*() at line 847
			// inlined Vector::CopyToArray() at line 844
			// inlined Vector::operator+() at line 843
			// inlined Vector::operator*() at line 843
			// inlined Vector::operator-() at line 841
			// inlined Vector::CopyToArray() at line 840
			// inlined Vector::operator-() at line 840
			// inlined Vector::operator-() at line 839
			// inlined Vector::operator*() at line 839
			// inlined Vector::CopyToArray() at line 836
			// inlined Vector::operator+() at line 835
			// inlined Vector::operator*() at line 835
			// inlined Vector::CopyToArray() at line 832
			// inlined Vector::operator-() at line 830
			// inlined Vector::CopyToArray() at line 829
			// inlined Vector::operator-() at line 829
		}
	}
	{
		int hPlayer;  // line 792
		{
			C_BasePlayer *pPlayer;  // line 794
		}
	}
	{
		int i;  // line 692
		// inlined Vector::operator!=() at line 697
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 697
		// inlined Vector::operator!=() at line 697
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 697
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::InsertAfter() at line 708
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::Count() at line 692
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 697
		// inlined QAngle::operator!=() at line 697
	}
	{
		int i;  // line 741
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::RemoveMultipleFromHead() at line 755
		{
			int j;  // line 748
		}
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::Count() at line 741
	}
	// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 763
	// inlined QAngle::operator!=() at line 763
	// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 763
	// inlined Vector::operator!=() at line 763
	// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 763
	// inlined Vector::operator!=() at line 763
}

// game/client/portal2/c_projectedwallentity.cpp:950 @0x5300e0 _ZN21C_ProjectedWallEntity20GetProjectionExtentsER6VectorS1_
void C_ProjectedWallEntity::GetProjectionExtents( Vector &outMins, Vector &outMaxs )
{
}

// game/client/portal2/c_projectedwallentity.cpp:957
ConVar cl_projectedbridge_aabbhack;

// game/client/portal2/c_projectedwallentity.cpp:959 @0x5305d0 _ZN21C_ProjectedWallEntity31ComputeWorldSpaceSurroundingBoxEP6VectorS1_
void C_ProjectedWallEntity::ComputeWorldSpaceSurroundingBox( Vector *pWorldMins, Vector *pWorldMaxs )
{
	// inlined Vector::operator=() at line 983
	// inlined Vector::operator=() at line 982
	{
		Vector vMins;  // line 963
		Vector vMaxs;  // line 964
		{
			int i;  // line 965
			// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 967
			// inlined VectorMin() at line 967
			// inlined Vector::operator=() at line 967
			// inlined VectorMax() at line 968
			// inlined Vector::operator=() at line 968
		}
		// inlined Vector::operator=() at line 971
		// inlined Vector::operator=() at line 972
	}
	{
		int i;  // line 990
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::Count() at line 990
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 993
		// inlined Vector::operator=() at line 995
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 996
		// inlined Vector::operator=() at line 996
	}
	// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 1004
	// inlined Vector::operator=() at line 1004
	// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 1005
	// inlined Vector::operator=() at line 1005
}

// game/client/portal2/c_projectedwallentity.cpp:1010 @0x5309a0 _ZN21C_ProjectedWallEntity13TestCollisionERK5Ray_tjR10CGameTrace
bool C_ProjectedWallEntity::TestCollision( const Ray_t &ray, unsigned int mask, trace_t &trace )
{
	{
		int i;  // line 1021
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::Count() at line 1021
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 1024
	}
	// inlined CGameTrace::DidHit() at line 1036
}

// game/client/portal2/c_projectedwallentity.cpp:1039 @0x52f290 _ZN21C_ProjectedWallEntity12TestHitboxesERK5Ray_tjR10CGameTrace
bool C_ProjectedWallEntity::TestHitboxes( const Ray_t &ray, unsigned int fContentsMask, trace_t &tr )
{
}

// game/client/portal2/c_projectedwallentity.cpp:1045 @0x52f370 _ZN21C_ProjectedWallEntity20SetPaintSurfaceColorERK14PaintPowerType
bool C_ProjectedWallEntity::SetPaintSurfaceColor( const PaintPowerType &power )
{
	float flColors[3];  // line 1047
	bool bIsPainted;  // line 1054
	{
		Color color;  // line 1057
	}
}

// game/client/portal2/c_projectedwallentity.cpp:1074 (declaration)
void SetPaintPower( int nSegment, PaintPowerType power );

// game/client/portal2/c_projectedwallentity.cpp:1074 @0x52f2c0 _ZN21C_ProjectedWallEntity13SetPaintPowerEi14PaintPowerType
void C_ProjectedWallEntity::SetPaintPower( int nSegment, PaintPowerType power )
{
}

// game/client/portal2/c_projectedwallentity.cpp:1083 @0x530350 _ZN21C_ProjectedWallEntity21GetToolRecordingStateEP9KeyValues
void C_ProjectedWallEntity::GetToolRecordingState( KeyValues *msg )
{
	KeyValues *pKV;  // line 1087
	Vector vStart;  // line 1098
	QAngle qAngles;  // line 1103
	// inlined QAngle::operator=() at line 1104
}

// game/client/portal2/c_projectedwallentity.cpp:1110 @0x52fe60 _ZN21C_ProjectedWallEntity26RestoreToToolRecordedStateEP9KeyValues
void C_ProjectedWallEntity::RestoreToToolRecordedState( KeyValues *pKV )
{
	QAngle qKVAngles;  // line 1125
	matrix3x4_t *pMatrix;  // line 1130
	// inlined Vector::Vector() at line 1121
	// inlined Vector::operator=() at line 1121
	// inlined Vector::Vector() at line 1122
	// inlined Vector::operator=() at line 1122
	{
		Vector vPos;  // line 1133
		QAngle qAngles;  // line 1134
		// inlined MatrixAngles() at line 1135
		// inlined QAngle::operator=() at line 1136
	}
	// inlined CBitVecT<CFixedBitVecBase<2> >::SetAll() at line 1146
}

// game/client/portal2/c_projectedwallentity.cpp:1154 @0x52f470 _ZN21C_ProjectedWallEntity20ShouldSpawnParticlesEP15C_Portal_Base2D
bool C_ProjectedWallEntity::ShouldSpawnParticles( C_Portal_Base2D *pPortal )
{
	C_Prop_Portal *pPropPortal;  // line 1160
	Vector vPortalUp;  // line 1164
	Vector vWallUp;  // line 1164
	float flDot;  // line 1168
	// inlined DotProduct() at line 1168
}

// game/client/portal2/c_projectedwallentity.cpp:1177 @0x52f590 _ZN21C_ProjectedWallEntity18SetupWallParticlesEv
void C_ProjectedWallEntity::SetupWallParticles()
{
	const float flImpactPointOffset;  // line 1179
	C_Portal_Base2D *pSourcePortal;  // line 1184
	C_Portal_Base2D *pHitPortal;  // line 1185
	Vector vecParticlePos;  // line 1187
	Vector vWallEndPoint;  // line 1189
	Vector vWallStartPoint;  // line 1190
	QAngle qAngles;  // line 1193
	Vector vecForward;  // line 1194
	Vector vecRight;  // line 1194
	Vector vecUp;  // line 1194
	// inlined Vector::operator=() at line 1224
	// inlined Vector::operator-() at line 1224
	// inlined Vector::operator-() at line 1224
	// inlined Vector::operator*() at line 1224
	// inlined Vector::operator*() at line 1224
	// inlined Vector::operator=() at line 1220
	// inlined Vector::operator+() at line 1220
	// inlined Vector::operator-() at line 1220
	// inlined Vector::operator*() at line 1220
	// inlined Vector::operator*() at line 1220
	// inlined CHandle<C_Portal_Base2D>::Get() at line 1185
	// inlined CHandle<C_Portal_Base2D>::Get() at line 1184
	{
		C_Prop_Portal *pPortal;  // line 1209
		// inlined Vector::operator*() at line 1201
		// inlined Vector::operator*() at line 1201
		// inlined Vector::operator+() at line 1201
		// inlined Vector::operator+() at line 1201
		// inlined Vector::operator=() at line 1201
		// inlined Vector::operator*() at line 1205
		// inlined Vector::operator*() at line 1205
		// inlined Vector::operator+() at line 1205
		// inlined Vector::operator-() at line 1205
		// inlined Vector::operator=() at line 1205
		// inlined Vector::operator*() at line 1212
		// inlined Vector::operator+() at line 1212
		// inlined Vector::operator=() at line 1212
	}
	// inlined Vector::operator*() at line 1230
	// inlined Vector::operator-() at line 1230
	// inlined Vector::operator=() at line 1230
}

// game/client/portal2/c_projectedwallentity.cpp:1238 sizeof=0x20 (i386)
struct CProjectedWallEntity_NonConformantDataHandler : public CIFM_EntityKeyValuesHandler_RecreateEntities
{
public:
	CProjectedWallEntity_NonConformantDataHandler();  // line 1240
	virtual void *CreateInstance();  // line 1244
	virtual void DestroyInstance( void * );  // line 1249
	virtual void HandleInstance( void *, KeyValues * );  // line 1256
};

// game/client/portal2/c_projectedwallentity.cpp:1238 (declaration)
~CProjectedWallEntity_NonConformantDataHandler();

// game/client/portal2/c_projectedwallentity.cpp:1244 @0x537aa0 _ZN45CProjectedWallEntity_NonConformantDataHandler14CreateInstanceEv
void *CProjectedWallEntity_NonConformantDataHandler::CreateInstance()
{
	// inlined C_ProjectedWallEntity::C_ProjectedWallEntity() at line 1246
}

// game/client/portal2/c_projectedwallentity.cpp:1249 @0x537410 _ZN45CProjectedWallEntity_NonConformantDataHandler15DestroyInstanceEPv
void CProjectedWallEntity_NonConformantDataHandler::DestroyInstance( void *pEntity )
{
	C_ProjectedWallEntity *pCastEntity;  // line 1251
}

// game/client/portal2/c_projectedwallentity.cpp:1256 @0x5376c0 _ZN45CProjectedWallEntity_NonConformantDataHandler14HandleInstanceEPvP9KeyValues
void CProjectedWallEntity_NonConformantDataHandler::HandleInstance( void *pEntity, KeyValues *pKeyValues )
{
	C_ProjectedWallEntity *pCastEntity;  // line 1258
}

// game/client/portal2/c_projectedwallentity.cpp:1268
static CProjectedWallEntity_NonConformantDataHandler s_ProjectedWallEntityIFMHandler;

// game/client/portal2/c_projectedwallentity.cpp:1274 sizeof=0x1c (i386)
struct C_TEWallPaintedEvent : public C_BaseTempEntity
{
public:
	virtual void PostDataUpdate( DataUpdateType_t );  // line 1279
	CBaseHandle m_hEntity; // +0x10  // line 1290
	int m_colorIndex; // +0x14  // line 1291
	int m_nSegment; // +0x18  // line 1292
};

// game/client/portal2/c_projectedwallentity.cpp:1274 (declaration)
~C_TEWallPaintedEvent();

// game/client/portal2/c_projectedwallentity.cpp:1274 @0x537640 _ZN20C_TEWallPaintedEventD0Ev
C_TEWallPaintedEvent::~C_TEWallPaintedEvent()
{
}

// game/client/portal2/c_projectedwallentity.cpp:1274 @0x537690 _ZN20C_TEWallPaintedEventD1Ev
C_TEWallPaintedEvent::~C_TEWallPaintedEvent()
{
}

// game/client/portal2/c_projectedwallentity.cpp:1274 (declaration)
void C_TEWallPaintedEvent();

// game/client/portal2/c_projectedwallentity.cpp:1277 @0x52f2e0 _ZN20C_TEWallPaintedEvent40YouForgotToImplementOrDeclareClientClassEv
int C_TEWallPaintedEvent::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/portal2/c_projectedwallentity.cpp:1277 @0x52f300 _ZN20C_TEWallPaintedEvent14GetClientClassEv
ClientClass *C_TEWallPaintedEvent::GetClientClass()
{
}

// game/client/portal2/c_projectedwallentity.cpp:1279 @0x537770 _ZN20C_TEWallPaintedEvent14PostDataUpdateE16DataUpdateType_t
void C_TEWallPaintedEvent::PostDataUpdate( DataUpdateType_t updateType )
{
	C_ProjectedWallEntity *pWall;  // line 1281
	// inlined CBaseHandle::Get() at line 1281
	// inlined C_ProjectedWallEntity::SetPaintPower() at line 1285
}

// game/client/portal2/c_projectedwallentity.cpp:1295 @0x52f310 _ZL34_C_TEWallPaintedEvent_CreateObjectv
IClientNetworkable *_C_TEWallPaintedEvent_CreateObject()
{
}

// game/client/portal2/c_projectedwallentity.cpp:1295
ClientClass __g_C_TEWallPaintedEventClientClass;

// game/client/portal2/c_projectedwallentity.cpp:1295
static C_TEWallPaintedEvent __g_C_TEWallPaintedEvent;

// game/client/portal2/c_projectedwallentity.cpp:1297 @0x77ea0 _Z15ClientClassInitIN21DT_TEWallPaintedEvent7ignoredEEiPT_
int ClientClassInit<DT_TEWallPaintedEvent::ignored>( DT_TEWallPaintedEvent::ignored * )
{
	char *pRecvTableName;  // line 1297
	RecvTable &RecvTable;  // line 1297
	RecvProp RecvProps[4];  // line 1297
}

// game/client/portal2/c_projectedwallentity.cpp:1297
RecvTable g_RecvTable;

// game/client/portal2/c_projectedwallentity.cpp:1297
int g_RecvTableInit;
