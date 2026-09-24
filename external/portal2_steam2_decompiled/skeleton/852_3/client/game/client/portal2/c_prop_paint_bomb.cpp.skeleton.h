// DWARF declaration skeleton for game/client/portal2/c_prop_paint_bomb.cpp
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x78820 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
	// inlined ClientClass::ClientClass() at line 15
}

// game/client/portal2/c_prop_paint_bomb.cpp:15 @0x536bc0 _ZN15C_PropPaintBomb40YouForgotToImplementOrDeclareClientClassEv
int C_PropPaintBomb::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/portal2/c_prop_paint_bomb.cpp:15 @0x536be0 _ZN15C_PropPaintBomb14GetClientClassEv
ClientClass *C_PropPaintBomb::GetClientClass()
{
}

// game/client/portal2/c_prop_paint_bomb.cpp:15 @0x786d0 _Z15ClientClassInitIN16DT_PropPaintBomb7ignoredEEiPT_
int ClientClassInit<DT_PropPaintBomb::ignored>( DT_PropPaintBomb::ignored * )
{
	char *pRecvTableName;  // line 15
	RecvTable &RecvTable;  // line 15
	RecvProp RecvProps[3];  // line 15
}

// game/client/portal2/c_prop_paint_bomb.cpp:15
RecvTable g_RecvTable;

// game/client/portal2/c_prop_paint_bomb.cpp:15
int g_RecvTableInit;

// game/client/portal2/c_prop_paint_bomb.cpp:15 @0x5373c0 _ZL29_C_PropPaintBomb_CreateObjectii
IClientNetworkable *_C_PropPaintBomb_CreateObject( int entnum, int serialNum )
{
	C_PropPaintBomb *pRet;  // line 15
	// inlined C_PropPaintBomb::C_PropPaintBomb() at line 15
}

// game/client/portal2/c_prop_paint_bomb.cpp:15
ClientClass __g_C_PropPaintBombClientClass;

// game/client/portal2/c_prop_paint_bomb.cpp:15
void C_PropPaintBomb::m_pClassRecvTable;

// game/client/portal2/c_prop_paint_bomb.cpp:22
ConVar cl_draw_paint_bomb_with_blobs;

// game/client/portal2/c_prop_paint_bomb.cpp:24 (declaration)
void C_PropPaintBomb();

// game/client/portal2/c_prop_paint_bomb.cpp:24 @0x537230 _ZN15C_PropPaintBombC2Ev
C_PropPaintBomb::C_PropPaintBomb()
{
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::CUtlVector() at line 27
}

// game/client/portal2/c_prop_paint_bomb.cpp:24 @0x5372f0 _ZN15C_PropPaintBombC1Ev
C_PropPaintBomb::C_PropPaintBomb()
{
}

// game/client/portal2/c_prop_paint_bomb.cpp:33 (declaration)
~C_PropPaintBomb();

// game/client/portal2/c_prop_paint_bomb.cpp:33 @0x537420 _ZN15C_PropPaintBombD2Ev
C_PropPaintBomb::~C_PropPaintBomb()
{
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::~CUtlVector() at line 35
}

// game/client/portal2/c_prop_paint_bomb.cpp:33 @0x537ae0 _ZN15C_PropPaintBombD0Ev
C_PropPaintBomb::~C_PropPaintBomb()
{
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::~CUtlVector() at line 35
}

// game/client/portal2/c_prop_paint_bomb.cpp:33 @0x537c30 _ZN15C_PropPaintBombD1Ev
C_PropPaintBomb::~C_PropPaintBomb()
{
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::~CUtlVector() at line 35
}

// game/client/portal2/c_prop_paint_bomb.cpp:38 @0x537170 _ZN15C_PropPaintBomb8PrecacheEv
void C_PropPaintBomb::Precache()
{
}

// game/client/portal2/c_prop_paint_bomb.cpp:44 @0x537110 _ZN15C_PropPaintBomb5SpawnEv
void C_PropPaintBomb::Spawn()
{
}

// game/client/portal2/c_prop_paint_bomb.cpp:54 @0x537300 _ZN15C_PropPaintBomb14UpdateOnRemoveEv
void C_PropPaintBomb::UpdateOnRemove()
{
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::PurgeAndDeleteElements() at line 57
}

// game/client/portal2/c_prop_paint_bomb.cpp:63 @0x5370e0 _ZN15C_PropPaintBomb16OnPreDataChangedE16DataUpdateType_t
void C_PropPaintBomb::OnPreDataChanged( DataUpdateType_t updateType )
{
}

// game/client/portal2/c_prop_paint_bomb.cpp:72 @0x537190 _ZN15C_PropPaintBomb13OnDataChangedE16DataUpdateType_t
void C_PropPaintBomb::OnDataChanged( DataUpdateType_t updateType )
{
	// inlined Vector::operator=() at line 83
}

// game/client/portal2/c_prop_paint_bomb.cpp:93 @0x537090 _ZN15C_PropPaintBomb14PostDataUpdateE16DataUpdateType_t
void C_PropPaintBomb::PostDataUpdate( DataUpdateType_t updateType )
{
}

// game/client/portal2/c_prop_paint_bomb.cpp:105 @0x536c00 _ZN15C_PropPaintBomb11UpdateBlobsEf
void C_PropPaintBomb::UpdateBlobs( float flDeltaTime )
{
	Vector vecNewBombPos;  // line 107
	float flMaxDistanceFromBomb;  // line 108
	int numBlobs;  // line 110
	// inlined ConVar::GetFloat() at line 108
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::Count() at line 110
	{
		int i;  // line 111
		{
			CBasePaintBlob *pBlob;  // line 113
			Vector vecBombPosOffset;  // line 115
			Vector vecNewBlobPos;  // line 118
			Vector vecBlobDir;  // line 121
			// inlined Vector::operator=() at line 129
			// inlined Vector::operator+() at line 129
			// inlined Vector::operator*() at line 129
			// inlined Vector::operator VectorByValue&() at line 126
			// inlined Vector::operator-() at line 115
			// inlined Vector::operator+() at line 118
			// inlined Vector::operator-() at line 122
			// inlined Vector::operator-() at line 122
			// inlined Vector::operator-() at line 126
		}
	}
	// inlined Vector::operator=() at line 134
}

// game/client/portal2/c_prop_paint_bomb.cpp:139 @0x536fb0 _ZN15C_PropPaintBomb11ClientThinkEv
void C_PropPaintBomb::ClientThink()
{
	float flDeltaTime;  // line 145
}

// game/client/portal2/c_prop_paint_bomb.cpp:170 @0x537570 _ZN15C_PropPaintBomb9DrawModelEiRK20RenderableInstance_t
int C_PropPaintBomb::DrawModel( int flags, const RenderableInstance_t &instance )
{
	int nCount;  // line 188
	Vector vCenter;  // line 194
	C_BasePlayer *pPlayer;  // line 212
	Vector vecPlayerPos;  // line 213
	float flDistThreshold;  // line 214
	int iNumParticles;  // line 216
	IMaterial *pMaterial;  // line 237
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::Count() at line 188
	{
		int i;  // line 195
		// inlined Vector::operator+=() at line 197
	}
	// inlined Vector::operator/=() at line 199
	// inlined GetSplitScreenViewPlayer() at line 212
	// inlined Vector::operator VectorByValue&() at line 213
	// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::Count() at line 217
	// inlined CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants, 16> >::SetCount() at line 217
	{
		int i;  // line 219
		{
			CBasePaintBlob *pBlob;  // line 221
			Vector vecWorldCenter;  // line 222
			{
				ImpParticleWithOneInterpolant *imp_particle;  // line 226
				// inlined Point3D::set() at line 229
				// inlined ImpParticle::setFieldScale() at line 228
				// inlined Point3D::operator=() at line 227
			}
			// inlined Vector::DistToSqr() at line 224
		}
	}
	// inlined CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants, 16> >::SetCountNonDestructively() at line 235
	// inlined ConVar::GetFloat() at line 238
}

// game/client/portal2/c_prop_paint_bomb.cpp:244 (declaration)
void CleansePaintPower();

// game/client/portal2/c_prop_paint_bomb.cpp:244 @0x536bf0 _ZN15C_PropPaintBomb17CleansePaintPowerEv
void C_PropPaintBomb::CleansePaintPower()
{
}
