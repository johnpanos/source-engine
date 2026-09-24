// DWARF declaration skeleton for game/client/portal2/c_projectedwallentity.cpp
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x77da0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
	// inlined CResourcePrecacher::CResourcePrecacher() at line 46
	// inlined ClientClass::ClientClass() at line 48
	// inlined PredMapInit<C_ProjectedWallEntity>() at line 69
	// inlined Cprojected_wall_entityFoo::Cprojected_wall_entityFoo() at line 77
	// inlined CProjectedWallEntity_NonConformantDataHandler::CProjectedWallEntity_NonConformantDataHandler() at line 1115
	// inlined C_TEWallPaintedEvent::C_TEWallPaintedEvent() at line 1142
	// inlined ClientClass::ClientClass() at line 1142
}

// game/client/portal2/c_projectedwallentity.cpp:34
ConVar cl_projected_wall_projection_speed;

// game/client/portal2/c_projectedwallentity.cpp:35
ConVar cl_paintable_projected_wall_texture_wrap_rate;

// game/client/portal2/c_projectedwallentity.cpp:36
ConVar cl_draw_projected_wall_with_paint;

// game/client/portal2/c_projectedwallentity.cpp:38
ConVar debug_projected_wall_drawing;

// game/client/portal2/c_projectedwallentity.cpp:42 sizeof=0x10 (i386)
struct CResourcePrecacher : public CBaseResourcePrecacher
{
public:
	CResourcePrecacher();  // line 42
	virtual void Cache( IPrecacheHandler *, bool, ResourceList_t, bool );  // line 42
};

// game/client/portal2/c_projectedwallentity.cpp:42 @0x52de10 _ZN41PrecacheEffectProjectedWallEntityPrecache18CResourcePrecacher5CacheEP16IPrecacheHandlerbP16ResourceList_t__b
void CResourcePrecacher::Cache( IPrecacheHandler *pPrecacheHandler, bool bPrecache, ResourceList_t hResourceList, bool bIgnoreConditionals )
{
}

// game/client/portal2/c_projectedwallentity.cpp:46
PrecacheEffectProjectedWallEntityPrecache::CResourcePrecacher s_ResourcePrecacher;

// game/client/portal2/c_projectedwallentity.cpp:48 @0x52dea0 _ZN21C_ProjectedWallEntity40YouForgotToImplementOrDeclareClientClassEv
int C_ProjectedWallEntity::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/portal2/c_projectedwallentity.cpp:48 @0x52dec0 _ZN21C_ProjectedWallEntity14GetClientClassEv
ClientClass *C_ProjectedWallEntity::GetClientClass()
{
}

// game/client/portal2/c_projectedwallentity.cpp:48 @0x77a00 _Z15ClientClassInitIN22DT_ProjectedWallEntity7ignoredEEiPT_
int ClientClassInit<DT_ProjectedWallEntity::ignored>( DT_ProjectedWallEntity::ignored * )
{
	char *pRecvTableName;  // line 48
	RecvTable &RecvTable;  // line 48
	RecvProp RecvProps[14];  // line 48
}

// game/client/portal2/c_projectedwallentity.cpp:48
RecvTable g_RecvTable;

// game/client/portal2/c_projectedwallentity.cpp:48
int g_RecvTableInit;

// game/client/portal2/c_projectedwallentity.cpp:48 @0x530db0 _ZL35_C_ProjectedWallEntity_CreateObjectii
IClientNetworkable *_C_ProjectedWallEntity_CreateObject( int entnum, int serialNum )
{
	C_ProjectedWallEntity *pRet;  // line 48
	// inlined C_ProjectedWallEntity::C_ProjectedWallEntity() at line 48
}

// game/client/portal2/c_projectedwallentity.cpp:48
ClientClass __g_C_ProjectedWallEntityClientClass;

// game/client/portal2/c_projectedwallentity.cpp:48
void C_ProjectedWallEntity::m_pClassRecvTable;

// game/client/portal2/c_projectedwallentity.cpp:69 @0x52ded0 _ZN21C_ProjectedWallEntity14GetPredDescMapEv
datamap_t *C_ProjectedWallEntity::GetPredDescMap()
{
}

// game/client/portal2/c_projectedwallentity.cpp:69 (declaration)
datamap_t *PredMapInit<C_ProjectedWallEntity>( C_ProjectedWallEntity * );

// game/client/portal2/c_projectedwallentity.cpp:69 @0x52dee0 _Z11PredMapInitI21C_ProjectedWallEntityEP9datamap_tPT_
datamap_t *PredMapInit<C_ProjectedWallEntity>( C_ProjectedWallEntity * )
{
}

// game/client/portal2/c_projectedwallentity.cpp:69
datamap_t *g_PredMapHolder;

// game/client/portal2/c_projectedwallentity.cpp:69
void C_ProjectedWallEntity::m_PredMap;

// game/client/portal2/c_projectedwallentity.cpp:77 sizeof=0x1 (i386)
struct Cprojected_wall_entityFoo
{
public:
	Cprojected_wall_entityFoo();  // line 77
};

// game/client/portal2/c_projectedwallentity.cpp:77 @0x530d70 _ZL29CC_ProjectedWallEntityFactoryv
C_BaseEntity *CC_ProjectedWallEntityFactory()
{
	// inlined C_ProjectedWallEntity::C_ProjectedWallEntity() at line 77
}

// game/client/portal2/c_projectedwallentity.cpp:80 (declaration)
void C_ProjectedWallEntity();

// game/client/portal2/c_projectedwallentity.cpp:80 @0x530c50 _ZN21C_ProjectedWallEntityC2Ev
C_ProjectedWallEntity::C_ProjectedWallEntity()
{
	// inlined CPaintableEntity<C_BaseProjectedEntity>::CPaintableEntity() at line 80
	// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::CUtlVector() at line 80
	// inlined CHandle<C_BaseEntity>::CHandle() at line 80
	// inlined CUtlVector<PaintPowerType,CUtlMemory<PaintPowerType, int> >::CUtlVector() at line 80
}

// game/client/portal2/c_projectedwallentity.cpp:80 @0x530d60 _ZN21C_ProjectedWallEntityC1Ev
C_ProjectedWallEntity::C_ProjectedWallEntity()
{
}

// game/client/portal2/c_projectedwallentity.cpp:94 @0x52edf0 _ZN21C_ProjectedWallEntity14UpdateOnRemoveEv
void C_ProjectedWallEntity::UpdateOnRemove()
{
	// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::RemoveAll() at line 103
	{
		int i;  // line 98
	}
}

// game/client/portal2/c_projectedwallentity.cpp:108 @0x52df10 _ZN21C_ProjectedWallEntity14GetCollideTypeEv
CollideType_t C_ProjectedWallEntity::GetCollideType()
{
}

// game/client/portal2/c_projectedwallentity.cpp:114 (declaration)
void DrawQuadHelper( CMeshBuilder *meshBuilder, const Vector &vOrigin, const Vector &vRight, const Vector &vUp, float flTextureScaleU, float flTextureScaleV, float flAlpha );

// game/client/portal2/c_projectedwallentity.cpp:163 @0x530e20 _ZN21C_ProjectedWallEntity9DrawModelEiRK20RenderableInstance_t
int C_ProjectedWallEntity::DrawModel( int flags, const RenderableInstance_t &instance )
{
	bool bDebugging;  // line 171
	{
		CMatRenderContextPtr pRenderContext;  // line 175
		IMesh *pMesh;  // line 177
		CMeshBuilder meshBuilder;  // line 178
		Vector vecForward;  // line 180
		Vector vecRight;  // line 180
		Vector vecUp;  // line 180
		Vector vecForwardDir;  // line 180
		Vector vecRightDir;  // line 180
		Vector vecUpDir;  // line 180
		Vector vecOrigin;  // line 184
		float flLengthTexRate;  // line 194
		float flWidthTextRate;  // line 195
		float flDrawnLength;  // line 196
		float flPrevSegmentLength;  // line 197
		float flCurrentSegmentLength;  // line 198
		{
			Vector vecWallEnd;  // line 189
			// inlined Vector::operator*() at line 189
			// inlined Vector::operator+() at line 189
		}
		// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 267
		// inlined CMeshBuilder::~CMeshBuilder() at line 267
		// inlined Vector::operator/() at line 183
		// inlined Vector::operator*() at line 183
		// inlined Vector::operator/() at line 182
		// inlined Vector::operator*() at line 182
		// inlined CMeshBuilder::CMeshBuilder() at line 178
		// inlined CMatRenderContextPtr::CMatRenderContextPtr() at line 175
		// inlined Vector::operator*() at line 208
		// inlined Vector::operator=() at line 208
		// inlined Vector::operator*() at line 209
		// inlined Vector::operator+=() at line 209
		{
			int i;  // line 213
			{
				float flOverDraw;  // line 254
				// inlined CMeshBuilder::End() at line 225
				// inlined C_ProjectedWallEntity::DrawQuadHelper() at line 223
				// inlined Vector::operator-() at line 223
				// inlined C_ProjectedWallEntity::DrawQuadHelper() at line 222
				// inlined Vector::operator+() at line 222
				// inlined C_ProjectedWallEntity::DrawQuadHelper() at line 220
				// inlined Vector::operator+() at line 220
				// inlined CMeshBuilder::Begin() at line 217
				// inlined C_ProjectedWallEntity::SetWallColor() at line 215
				{
					Color color;  // line 230
					Vector vecMins;  // line 236
					Vector vecMaxs;  // line 236
					// inlined Color::Color() at line 230
					// inlined Color::Color() at line 233
					// inlined Color::operator=() at line 233
					// inlined Vector::operator-() at line 238
					// inlined Vector::operator*() at line 238
					// inlined Vector::operator/() at line 238
					// inlined Vector::operator*() at line 239
					// inlined Vector::operator/() at line 239
					// inlined Vector::operator-=() at line 239
					// inlined Vector::operator*() at line 240
					// inlined Vector::operator-=() at line 240
					// inlined Vector::operator*() at line 241
					// inlined Vector::operator/() at line 241
					// inlined Vector::operator/() at line 242
					// inlined Vector::operator+=() at line 242
					// inlined Vector::operator*() at line 243
					// inlined Vector::operator+=() at line 243
					// inlined Color::b() at line 245
					// inlined Color::g() at line 245
					// inlined Color::r() at line 245
				}
				// inlined Vector::operator*() at line 265
				// inlined Vector::operator/() at line 265
				// inlined Vector::operator=() at line 265
				// inlined Vector::operator*() at line 266
				// inlined Vector::operator+=() at line 266
			}
		}
		// inlined Vector::operator*() at line 203
		// inlined Vector::operator=() at line 203
		// inlined Vector::operator*() at line 204
		// inlined Vector::operator+=() at line 204
		// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 267
		// inlined CMeshBuilder::~CMeshBuilder() at line 267
	}
	// inlined ConVar::GetBool() at line 171
	{
		Vector vForward;  // line 271
		Vector vRight;  // line 271
		Vector vUp;  // line 271
		Vector vOrigin;  // line 273
		CMatRenderContextPtr pRenderContext;  // line 275
		IMesh *pMesh;  // line 277
		CMeshBuilder meshBuilder;  // line 278
		float flLengthTexRate;  // line 294
		float flWidthTextRate;  // line 295
		CMeshBuilder meshBuilder2;  // line 327
		// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 337
		// inlined CMeshBuilder::~CMeshBuilder() at line 337
		// inlined CMeshBuilder::~CMeshBuilder() at line 337
		// inlined CMeshBuilder::End() at line 336
		// inlined CMeshBuilder::End() at line 319
		// inlined C_ProjectedWallEntity::DrawQuadHelper() at line 317
		// inlined Vector::operator+() at line 317
		// inlined CMeshBuilder::Begin() at line 297
		// inlined Vector::operator*=() at line 288
		// inlined Vector::operator*=() at line 287
		// inlined Vector::operator*=() at line 286
		// inlined GameRules() at line 280
		// inlined CMeshBuilder::CMeshBuilder() at line 278
		// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 277
		// inlined CMatRenderContextPtr::CMatRenderContextPtr() at line 275
		// inlined Vector::operator+() at line 273
		// inlined Vector::operator/() at line 273
		// inlined Vector::operator*() at line 273
		// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 326
		// inlined CMeshBuilder::CMeshBuilder() at line 327
		// inlined CMeshBuilder::Begin() at line 329
		// inlined Vector::operator*=() at line 332
		// inlined Vector::operator+() at line 333
		// inlined C_ProjectedWallEntity::DrawQuadHelper() at line 333
		// inlined Vector::operator-() at line 334
		// inlined C_ProjectedWallEntity::DrawQuadHelper() at line 334
		// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 337
		// inlined CMeshBuilder::~CMeshBuilder() at line 337
		// inlined CMeshBuilder::~CMeshBuilder() at line 337
	}
}

// game/client/portal2/c_projectedwallentity.cpp:365 @0x52f510 _ZN21C_ProjectedWallEntity17ColorWallByPortalEP9IMaterial
void C_ProjectedWallEntity::ColorWallByPortal( IMaterial *pBody )
{
	float flColors[3];  // line 367
	// inlined CHandle<C_BaseEntity>::Get() at line 374
	{
		C_Prop_Portal *pPortal;  // line 376
		// inlined CHandle<C_BaseEntity>::Get() at line 379
		{
			Color clrPortal;  // line 381
		}
	}
}

// game/client/portal2/c_projectedwallentity.cpp:391 @0x52df20 _ZN21C_ProjectedWallEntity13InitMaterialsEv
bool C_ProjectedWallEntity::InitMaterials()
{
	bool bFound;  // line 419
}

// game/client/portal2/c_projectedwallentity.cpp:432 @0x52ee80 _ZN21C_ProjectedWallEntity15GetRenderBoundsER6VectorS1_
void C_ProjectedWallEntity::GetRenderBounds( Vector &vecMins, Vector &vecMaxs )
{
	// inlined Vector::operator-() at line 434
	// inlined Vector::operator=() at line 434
	// inlined Vector::operator-() at line 435
	// inlined Vector::operator=() at line 435
}

// game/client/portal2/c_projectedwallentity.cpp:440 @0x52e050 _ZN21C_ProjectedWallEntity15GetRenderAnglesEv
const QAngle &C_ProjectedWallEntity::GetRenderAngles()
{
}

// game/client/portal2/c_projectedwallentity.cpp:445 @0x52e060 _ZN21C_ProjectedWallEntity23ComputeTranslucencyTypeEv
RenderableTranslucencyType_t C_ProjectedWallEntity::ComputeTranslucencyType()
{
}

// game/client/portal2/c_projectedwallentity.cpp:450 @0x52e120 _ZN21C_ProjectedWallEntity16OnPreDataChangedE16DataUpdateType_t
void C_ProjectedWallEntity::OnPreDataChanged( DataUpdateType_t type )
{
}

// game/client/portal2/c_projectedwallentity.cpp:457 @0x52f380 _ZN21C_ProjectedWallEntity13OnDataChangedE16DataUpdateType_t
void C_ProjectedWallEntity::OnDataChanged( DataUpdateType_t updateType )
{
}

// game/client/portal2/c_projectedwallentity.cpp:479 @0x530c20 _ZN21C_ProjectedWallEntity11OnProjectedEv
void C_ProjectedWallEntity::OnProjected()
{
}

// game/client/portal2/c_projectedwallentity.cpp:486 @0x52eda0 _ZN21C_ProjectedWallEntity5SpawnEv
void C_ProjectedWallEntity::Spawn()
{
}

// game/client/portal2/c_projectedwallentity.cpp:492 @0x52f430 _ZN21C_ProjectedWallEntity11ClientThinkEv
void C_ProjectedWallEntity::ClientThink()
{
	float flDisplaySpeed;  // line 496
	// inlined ConVar::GetFloat() at line 496
}

// game/client/portal2/c_projectedwallentity.cpp:522 @0x52f760 _ZN21C_ProjectedWallEntity11ProjectWallEv
void C_ProjectedWallEntity::ProjectWall()
{
	Vector vWallStartPoint;  // line 528
	Vector vWallEndPoint;  // line 529
	bool bCreate;  // line 543
	int iReplaceIndex;  // line 544
	int iValidIndex;  // line 545
	Vector vecForward;  // line 660
	Vector vecRight;  // line 660
	Vector vecUp;  // line 660
	// inlined Vector::Length() at line 785
	// inlined Vector::operator-() at line 785
	{
		Vector vSetMins;  // line 771
		Vector vSetMaxs;  // line 772
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 768
		// inlined Vector::operator=() at line 768
		// inlined Vector::operator=() at line 769
		// inlined Vector::operator-() at line 771
		// inlined Vector::operator-() at line 772
	}
	{
		CPhysCollide *pWallCollideable;  // line 665
		Vector vWorldMins;  // line 733
		Vector vWorldMaxs;  // line 733
		C_ProjectedWallEntity::WallCollideableAtTime_t temp;  // line 738
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::AddToTail() at line 762
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::Count() at line 761
		// inlined C_ProjectedWallEntity::WallCollideableAtTime_t::operator=() at line 753
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 753
		// inlined QAngle::operator=() at line 743
		// inlined Vector::operator=() at line 742
		// inlined Vector::operator=() at line 741
		// inlined Vector::operator=() at line 740
		// inlined Vector::operator=() at line 739
		{
			Vector vScaledRight;  // line 668
			Vector vStart;  // line 669
			Vector vEnd;  // line 670
			Vector vVerts[4];  // line 672
			Vector *pVerts[4];  // line 678
			CPhysConvex *pTempConvex;  // line 679
			// inlined Vector::operator*() at line 668
			// inlined Vector::operator+() at line 673
			// inlined Vector::operator=() at line 673
			// inlined Vector::operator-() at line 674
			// inlined Vector::operator=() at line 674
			// inlined Vector::operator-() at line 675
			// inlined Vector::operator=() at line 675
			// inlined Vector::operator+() at line 676
			// inlined Vector::operator=() at line 676
		}
		{
			float flWallPlanes[24];  // line 685
			Vector vWallPointRight;  // line 692
			Vector vWallPointLeft;  // line 696
			Vector vWallPointUp;  // line 700
			Vector vWallPointDown;  // line 704
			CPolyhedron *pTempPolyhedron;  // line 709
			CPhysConvex *pTempConvex;  // line 716
			// inlined Vector::operator-() at line 706
			// inlined Vector::CopyToArray() at line 705
			// inlined Vector::operator-() at line 705
			// inlined Vector::operator-() at line 704
			// inlined Vector::operator*() at line 704
			// inlined Vector::CopyToArray() at line 701
			// inlined Vector::operator+() at line 700
			// inlined Vector::operator*() at line 700
			// inlined Vector::operator-() at line 698
			// inlined Vector::CopyToArray() at line 697
			// inlined Vector::operator-() at line 697
			// inlined Vector::operator-() at line 696
			// inlined Vector::operator*() at line 696
			// inlined Vector::CopyToArray() at line 693
			// inlined Vector::operator+() at line 692
			// inlined Vector::operator*() at line 692
			// inlined Vector::CopyToArray() at line 689
			// inlined Vector::operator-() at line 687
			// inlined Vector::CopyToArray() at line 686
			// inlined Vector::operator-() at line 686
		}
	}
	{
		int hPlayer;  // line 649
		{
			C_BasePlayer *pPlayer;  // line 651
		}
	}
	{
		int i;  // line 549
		// inlined Vector::operator!=() at line 554
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 554
		// inlined Vector::operator!=() at line 554
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 554
		// inlined QAngle::operator!=() at line 554
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 554
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::Count() at line 549
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::InsertAfter() at line 565
	}
	{
		int i;  // line 598
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::RemoveMultipleFromHead() at line 612
		{
			int j;  // line 605
		}
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::Count() at line 598
	}
	// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 620
	// inlined QAngle::operator!=() at line 620
	// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 620
	// inlined Vector::operator!=() at line 620
	// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 620
	// inlined Vector::operator!=() at line 620
}

// public/tier1/utlvector.h:655 @0x5358d0 _ZN10CUtlVectorIN21C_ProjectedWallEntity23WallCollideableAtTime_tE10CUtlMemoryIS1_iEE10GrowVectorEi
void CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::GrowVector( int num )
{
	// inlined CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t,int>::NumAllocated() at line 657
	// inlined CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t,int>::Grow() at line 660
	// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::ResetDbgInfo() at line 664
}

// game/client/portal2/c_projectedwallentity.cpp:805 @0x52ed70 _ZN21C_ProjectedWallEntity20GetProjectionExtentsER6VectorS1_
void C_ProjectedWallEntity::GetProjectionExtents( Vector &outMins, Vector &outMaxs )
{
}

// game/client/portal2/c_projectedwallentity.cpp:812
ConVar cl_projectedbridge_aabbhack;

// game/client/portal2/c_projectedwallentity.cpp:814 @0x52ef40 _ZN21C_ProjectedWallEntity31ComputeWorldSpaceSurroundingBoxEP6VectorS1_
void C_ProjectedWallEntity::ComputeWorldSpaceSurroundingBox( Vector *pWorldMins, Vector *pWorldMaxs )
{
	// inlined Vector::operator=() at line 838
	// inlined Vector::operator=() at line 837
	{
		Vector vMins;  // line 818
		Vector vMaxs;  // line 819
		{
			int i;  // line 820
			// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 822
			// inlined VectorMin() at line 822
			// inlined Vector::operator=() at line 822
			// inlined VectorMax() at line 823
			// inlined Vector::operator=() at line 823
		}
		// inlined Vector::operator=() at line 826
		// inlined Vector::operator=() at line 827
	}
	{
		int i;  // line 845
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::Count() at line 845
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 848
		// inlined Vector::operator=() at line 850
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 851
		// inlined Vector::operator=() at line 851
	}
	// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 859
	// inlined Vector::operator=() at line 859
	// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 860
	// inlined Vector::operator=() at line 860
}

// game/client/portal2/c_projectedwallentity.cpp:865 @0x52f630 _ZN21C_ProjectedWallEntity13TestCollisionERK5Ray_tjR10CGameTrace
bool C_ProjectedWallEntity::TestCollision( const Ray_t &ray, unsigned int mask, trace_t &trace )
{
	{
		int i;  // line 876
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::Count() at line 876
		// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::operator[]() at line 879
	}
	// inlined CGameTrace::DidHit() at line 891
}

// public/tier1/utlvector.h:865 @0x535dc0 _ZN10CUtlVectorI14PaintPowerType10CUtlMemoryIS0_iEE8SetCountEi
void CUtlVector<PaintPowerType,CUtlMemory<PaintPowerType, int> >::SetCount( int count )
{
	// inlined CUtlVector<PaintPowerType,CUtlMemory<PaintPowerType, int> >::RemoveAll() at line 867
	// inlined CUtlVector<PaintPowerType,CUtlMemory<PaintPowerType, int> >::AddMultipleToTail() at line 868
}

// game/client/portal2/c_projectedwallentity.cpp:894 @0x52e070 _ZN21C_ProjectedWallEntity12TestHitboxesERK5Ray_tjR10CGameTrace
bool C_ProjectedWallEntity::TestHitboxes( const Ray_t &ray, unsigned int fContentsMask, trace_t &tr )
{
}

// game/client/portal2/c_projectedwallentity.cpp:900 (declaration)
void SetWallColor( const PaintPowerType &power );

// game/client/portal2/c_projectedwallentity.cpp:900 @0x52e150 _ZN21C_ProjectedWallEntity12SetWallColorERK14PaintPowerType
void C_ProjectedWallEntity::SetWallColor( const PaintPowerType &power )
{
	float flColors[3];  // line 902
	{
		Color color;  // line 911
	}
}

// game/client/portal2/c_projectedwallentity.cpp:921 (declaration)
void SetPaintPower( int nSegment, PaintPowerType power );

// game/client/portal2/c_projectedwallentity.cpp:921 @0x52e0a0 _ZN21C_ProjectedWallEntity13SetPaintPowerEi14PaintPowerType
void C_ProjectedWallEntity::SetPaintPower( int nSegment, PaintPowerType power )
{
}

// game/client/portal2/c_projectedwallentity.cpp:930 @0x52f1a0 _ZN21C_ProjectedWallEntity21GetToolRecordingStateEP9KeyValues
void C_ProjectedWallEntity::GetToolRecordingState( KeyValues *msg )
{
	KeyValues *pKV;  // line 934
	Vector vStart;  // line 945
	QAngle qAngles;  // line 950
	// inlined QAngle::operator=() at line 951
}

// game/client/portal2/c_projectedwallentity.cpp:957 @0x52eaf0 _ZN21C_ProjectedWallEntity26RestoreToToolRecordedStateEP9KeyValues
void C_ProjectedWallEntity::RestoreToToolRecordedState( KeyValues *pKV )
{
	QAngle qKVAngles;  // line 972
	matrix3x4_t *pMatrix;  // line 977
	// inlined Vector::Vector() at line 968
	// inlined Vector::operator=() at line 968
	// inlined Vector::Vector() at line 969
	// inlined Vector::operator=() at line 969
	{
		Vector vPos;  // line 980
		QAngle qAngles;  // line 981
		// inlined MatrixAngles() at line 982
		// inlined QAngle::operator=() at line 983
	}
	// inlined CBitVecT<CFixedBitVecBase<2> >::SetAll() at line 993
}

// game/client/portal2/c_projectedwallentity.cpp:1001 @0x52e1f0 _ZN21C_ProjectedWallEntity20ShouldSpawnParticlesEP15C_Portal_Base2D
bool C_ProjectedWallEntity::ShouldSpawnParticles( C_Portal_Base2D *pPortal )
{
	C_Prop_Portal *pPropPortal;  // line 1007
	Vector vPortalUp;  // line 1011
	Vector vWallUp;  // line 1011
	float flDot;  // line 1015
	// inlined DotProduct() at line 1015
}

// game/client/portal2/c_projectedwallentity.cpp:1024 @0x52e310 _ZN21C_ProjectedWallEntity18SetupWallParticlesEv
void C_ProjectedWallEntity::SetupWallParticles()
{
	const float flImpactPointOffset;  // line 1026
	C_Portal_Base2D *pSourcePortal;  // line 1031
	C_Portal_Base2D *pHitPortal;  // line 1032
	Vector vecParticlePos;  // line 1034
	Vector vWallEndPoint;  // line 1036
	Vector vWallStartPoint;  // line 1037
	QAngle qAngles;  // line 1040
	Vector vecForward;  // line 1041
	Vector vecRight;  // line 1041
	Vector vecUp;  // line 1041
	// inlined Vector::operator=() at line 1071
	// inlined Vector::operator-() at line 1071
	// inlined Vector::operator-() at line 1071
	// inlined Vector::operator*() at line 1071
	// inlined Vector::operator*() at line 1071
	// inlined Vector::operator=() at line 1067
	// inlined Vector::operator+() at line 1067
	// inlined Vector::operator-() at line 1067
	// inlined Vector::operator*() at line 1067
	// inlined Vector::operator*() at line 1067
	// inlined CHandle<C_Portal_Base2D>::Get() at line 1032
	// inlined CHandle<C_Portal_Base2D>::Get() at line 1031
	{
		C_Prop_Portal *pPortal;  // line 1056
		// inlined Vector::operator*() at line 1048
		// inlined Vector::operator*() at line 1048
		// inlined Vector::operator+() at line 1048
		// inlined Vector::operator+() at line 1048
		// inlined Vector::operator=() at line 1048
		// inlined Vector::operator*() at line 1052
		// inlined Vector::operator*() at line 1052
		// inlined Vector::operator+() at line 1052
		// inlined Vector::operator-() at line 1052
		// inlined Vector::operator=() at line 1052
		// inlined Vector::operator*() at line 1059
		// inlined Vector::operator+() at line 1059
		// inlined Vector::operator=() at line 1059
	}
	// inlined Vector::operator*() at line 1077
	// inlined Vector::operator-() at line 1077
	// inlined Vector::operator=() at line 1077
}

// game/client/portal2/c_projectedwallentity.cpp:1085 sizeof=0x20 (i386)
struct CProjectedWallEntity_NonConformantDataHandler : public CIFM_EntityKeyValuesHandler_RecreateEntities
{
public:
	CProjectedWallEntity_NonConformantDataHandler();  // line 1087
	virtual void *CreateInstance();  // line 1091
	virtual void DestroyInstance( void * );  // line 1096
	virtual void HandleInstance( void *, KeyValues * );  // line 1103
};

// game/client/portal2/c_projectedwallentity.cpp:1085 (declaration)
~CProjectedWallEntity_NonConformantDataHandler();

// game/client/portal2/c_projectedwallentity.cpp:1091 @0x535ef0 _ZN45CProjectedWallEntity_NonConformantDataHandler14CreateInstanceEv
void *CProjectedWallEntity_NonConformantDataHandler::CreateInstance()
{
	// inlined C_ProjectedWallEntity::C_ProjectedWallEntity() at line 1093
}

// game/client/portal2/c_projectedwallentity.cpp:1096 @0x535860 _ZN45CProjectedWallEntity_NonConformantDataHandler15DestroyInstanceEPv
void CProjectedWallEntity_NonConformantDataHandler::DestroyInstance( void *pEntity )
{
	C_ProjectedWallEntity *pCastEntity;  // line 1098
}

// game/client/portal2/c_projectedwallentity.cpp:1103 @0x535c20 _ZN45CProjectedWallEntity_NonConformantDataHandler14HandleInstanceEPvP9KeyValues
void CProjectedWallEntity_NonConformantDataHandler::HandleInstance( void *pEntity, KeyValues *pKeyValues )
{
	C_ProjectedWallEntity *pCastEntity;  // line 1105
}

// game/client/portal2/c_projectedwallentity.cpp:1115
static CProjectedWallEntity_NonConformantDataHandler s_ProjectedWallEntityIFMHandler;

// game/client/portal2/c_projectedwallentity.cpp:1121 sizeof=0x1c (i386)
struct C_TEWallPaintedEvent : public C_BaseTempEntity
{
public:
	virtual void PostDataUpdate( DataUpdateType_t );  // line 1126
	CBaseHandle m_hEntity; // +0x10  // line 1137
	int m_colorIndex; // +0x14  // line 1138
	int m_nSegment; // +0x18  // line 1139
};

// game/client/portal2/c_projectedwallentity.cpp:1121 (declaration)
~C_TEWallPaintedEvent();

// game/client/portal2/c_projectedwallentity.cpp:1121 @0x535ba0 _ZN20C_TEWallPaintedEventD0Ev
C_TEWallPaintedEvent::~C_TEWallPaintedEvent()
{
}

// game/client/portal2/c_projectedwallentity.cpp:1121 @0x535bf0 _ZN20C_TEWallPaintedEventD1Ev
C_TEWallPaintedEvent::~C_TEWallPaintedEvent()
{
}

// game/client/portal2/c_projectedwallentity.cpp:1121 (declaration)
void C_TEWallPaintedEvent();

// game/client/portal2/c_projectedwallentity.cpp:1124 @0x52e0c0 _ZN20C_TEWallPaintedEvent40YouForgotToImplementOrDeclareClientClassEv
int C_TEWallPaintedEvent::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/portal2/c_projectedwallentity.cpp:1124 @0x52e0e0 _ZN20C_TEWallPaintedEvent14GetClientClassEv
ClientClass *C_TEWallPaintedEvent::GetClientClass()
{
}

// game/client/portal2/c_projectedwallentity.cpp:1126 @0x535d30 _ZN20C_TEWallPaintedEvent14PostDataUpdateE16DataUpdateType_t
void C_TEWallPaintedEvent::PostDataUpdate( DataUpdateType_t updateType )
{
	C_ProjectedWallEntity *pWall;  // line 1128
	// inlined CBaseHandle::Get() at line 1128
	// inlined C_ProjectedWallEntity::SetPaintPower() at line 1132
}

// game/client/portal2/c_projectedwallentity.cpp:1142 @0x52e0f0 _ZL34_C_TEWallPaintedEvent_CreateObjectv
IClientNetworkable *_C_TEWallPaintedEvent_CreateObject()
{
}

// game/client/portal2/c_projectedwallentity.cpp:1142
ClientClass __g_C_TEWallPaintedEventClientClass;

// game/client/portal2/c_projectedwallentity.cpp:1142
static C_TEWallPaintedEvent __g_C_TEWallPaintedEvent;

// game/client/portal2/c_projectedwallentity.cpp:1144 @0x77880 _Z15ClientClassInitIN21DT_TEWallPaintedEvent7ignoredEEiPT_
int ClientClassInit<DT_TEWallPaintedEvent::ignored>( DT_TEWallPaintedEvent::ignored * )
{
	char *pRecvTableName;  // line 1144
	RecvTable &RecvTable;  // line 1144
	RecvProp RecvProps[4];  // line 1144
}

// game/client/portal2/c_projectedwallentity.cpp:1144
RecvTable g_RecvTable;

// game/client/portal2/c_projectedwallentity.cpp:1144
int g_RecvTableInit;
