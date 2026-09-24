// DWARF declaration skeleton for game/client/portal2/c_prop_paint_bomb.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x78e80 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined ClientClass::ClientClass() at line 16
}

// game/client/portal2/c_prop_paint_bomb.cpp:16 @0x538b00 _ZN15C_PropPaintBomb40YouForgotToImplementOrDeclareClientClassEv
int C_PropPaintBomb::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/portal2/c_prop_paint_bomb.cpp:16 @0x538b20 _ZN15C_PropPaintBomb14GetClientClassEv
ClientClass *C_PropPaintBomb::GetClientClass()
{
}

// game/client/portal2/c_prop_paint_bomb.cpp:16 @0x78d30 _Z15ClientClassInitIN16DT_PropPaintBomb7ignoredEEiPT_
int ClientClassInit<DT_PropPaintBomb::ignored>( DT_PropPaintBomb::ignored * )
{
	char *pRecvTableName;  // line 16
	RecvTable &RecvTable;  // line 16
	RecvProp RecvProps[3];  // line 16
}

// game/client/portal2/c_prop_paint_bomb.cpp:16
RecvTable g_RecvTable;

// game/client/portal2/c_prop_paint_bomb.cpp:16
int g_RecvTableInit;

// game/client/portal2/c_prop_paint_bomb.cpp:16 @0x539320 _ZL29_C_PropPaintBomb_CreateObjectii
IClientNetworkable *_C_PropPaintBomb_CreateObject( int entnum, int serialNum )
{
	C_PropPaintBomb *pRet;  // line 16
	// inlined C_PropPaintBomb::C_PropPaintBomb() at line 16
}

// game/client/portal2/c_prop_paint_bomb.cpp:16
ClientClass __g_C_PropPaintBombClientClass;

// game/client/portal2/c_prop_paint_bomb.cpp:16
void C_PropPaintBomb::m_pClassRecvTable;

// game/client/portal2/c_prop_paint_bomb.cpp:23
ConVar cl_draw_paint_bomb_with_blobs;

// game/client/portal2/c_prop_paint_bomb.cpp:25 (declaration)
void C_PropPaintBomb();

// game/client/portal2/c_prop_paint_bomb.cpp:25 @0x539170 _ZN15C_PropPaintBombC2Ev
C_PropPaintBomb::C_PropPaintBomb()
{
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::CUtlVector() at line 28
}

// game/client/portal2/c_prop_paint_bomb.cpp:25 @0x539230 _ZN15C_PropPaintBombC1Ev
C_PropPaintBomb::C_PropPaintBomb()
{
}

// game/client/portal2/c_prop_paint_bomb.cpp:34 (declaration)
~C_PropPaintBomb();

// game/client/portal2/c_prop_paint_bomb.cpp:34 @0x539380 _ZN15C_PropPaintBombD2Ev
C_PropPaintBomb::~C_PropPaintBomb()
{
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::~CUtlVector() at line 36
}

// game/client/portal2/c_prop_paint_bomb.cpp:34 @0x539b80 _ZN15C_PropPaintBombD0Ev
C_PropPaintBomb::~C_PropPaintBomb()
{
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::~CUtlVector() at line 36
}

// game/client/portal2/c_prop_paint_bomb.cpp:34 @0x539cd0 _ZN15C_PropPaintBombD1Ev
C_PropPaintBomb::~C_PropPaintBomb()
{
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::~CUtlVector() at line 36
}

// game/client/portal2/c_prop_paint_bomb.cpp:39 @0x5390b0 _ZN15C_PropPaintBomb8PrecacheEv
void C_PropPaintBomb::Precache()
{
}

// game/client/portal2/c_prop_paint_bomb.cpp:45 @0x539050 _ZN15C_PropPaintBomb5SpawnEv
void C_PropPaintBomb::Spawn()
{
}

// game/client/portal2/c_prop_paint_bomb.cpp:55 @0x539240 _ZN15C_PropPaintBomb14UpdateOnRemoveEv
void C_PropPaintBomb::UpdateOnRemove()
{
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::PurgeAndDeleteElements() at line 59
}

// game/client/portal2/c_prop_paint_bomb.cpp:65 @0x539020 _ZN15C_PropPaintBomb16OnPreDataChangedE16DataUpdateType_t
void C_PropPaintBomb::OnPreDataChanged( DataUpdateType_t updateType )
{
}

// game/client/portal2/c_prop_paint_bomb.cpp:74 @0x5390d0 _ZN15C_PropPaintBomb13OnDataChangedE16DataUpdateType_t
void C_PropPaintBomb::OnDataChanged( DataUpdateType_t updateType )
{
	// inlined Vector::operator=() at line 85
}

// game/client/portal2/c_prop_paint_bomb.cpp:95 @0x538fd0 _ZN15C_PropPaintBomb14PostDataUpdateE16DataUpdateType_t
void C_PropPaintBomb::PostDataUpdate( DataUpdateType_t updateType )
{
}

// game/client/portal2/c_prop_paint_bomb.cpp:107 @0x538b40 _ZN15C_PropPaintBomb11UpdateBlobsEf
void C_PropPaintBomb::UpdateBlobs( float flDeltaTime )
{
	Vector vecNewBombPos;  // line 109
	float flMaxDistanceFromBomb;  // line 110
	int numBlobs;  // line 112
	// inlined ConVar::GetFloat() at line 110
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::Count() at line 112
	{
		int i;  // line 113
		{
			CBasePaintBlob *pBlob;  // line 115
			Vector vecBombPosOffset;  // line 117
			Vector vecNewBlobPos;  // line 120
			Vector vecBlobDir;  // line 123
			// inlined Vector::operator=() at line 131
			// inlined Vector::operator+() at line 131
			// inlined Vector::operator*() at line 131
			// inlined Vector::operator VectorByValue&() at line 128
			// inlined Vector::operator-() at line 117
			// inlined Vector::operator+() at line 120
			// inlined Vector::operator-() at line 124
			// inlined Vector::operator-() at line 124
			// inlined Vector::operator-() at line 128
		}
	}
	// inlined Vector::operator=() at line 136
}

// game/client/portal2/c_prop_paint_bomb.cpp:141 @0x538ef0 _ZN15C_PropPaintBomb11ClientThinkEv
void C_PropPaintBomb::ClientThink()
{
	float flDeltaTime;  // line 147
}

// game/client/portal2/c_prop_paint_bomb.cpp:172 @0x5394d0 _ZN15C_PropPaintBomb9DrawModelEiRK20RenderableInstance_t
int C_PropPaintBomb::DrawModel( int flags, const RenderableInstance_t &instance )
{
	int nCount;  // line 190
	Vector vCenter;  // line 196
	C_BasePlayer *pPlayer;  // line 214
	Vector vecPlayerPos;  // line 215
	float flDistThreshold;  // line 216
	int iNumParticles;  // line 218
	IMaterial *pMaterial;  // line 239
	PortalMatrixList_t portalMatrixList;  // line 240
	// inlined CUtlVector<matrix3x4_t,CUtlMemory<matrix3x4_t, int> >::~CUtlVector() at line 243
	// inlined ConVar::GetFloat() at line 241
	// inlined CUtlVector<matrix3x4_t,CUtlMemory<matrix3x4_t, int> >::CUtlVector() at line 240
	// inlined CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants, 16> >::SetCountNonDestructively() at line 237
	{
		int i;  // line 221
		{
			CBasePaintBlob *pBlob;  // line 223
			Vector vecWorldCenter;  // line 224
			{
				ImpParticleWithOneInterpolant *imp_particle;  // line 228
				// inlined Point3D::set() at line 231
				// inlined ImpParticle::setFieldScale() at line 230
				// inlined Point3D::operator=() at line 229
			}
			// inlined Vector::DistToSqr() at line 226
		}
	}
	// inlined CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants, 16> >::SetCount() at line 219
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::Count() at line 219
	// inlined Vector::operator VectorByValue&() at line 215
	// inlined GetSplitScreenViewPlayer() at line 214
	// inlined Vector::operator/=() at line 201
	{
		int i;  // line 197
		// inlined Vector::operator+=() at line 199
	}
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::Count() at line 190
	// inlined CUtlVector<matrix3x4_t,CUtlMemory<matrix3x4_t, int> >::~CUtlVector() at line 243
}

// game/client/portal2/c_prop_paint_bomb.cpp:247 (declaration)
void CleansePaintPower();

// game/client/portal2/c_prop_paint_bomb.cpp:247 @0x538b30 _ZN15C_PropPaintBomb17CleansePaintPowerEv
void C_PropPaintBomb::CleansePaintPower()
{
}
