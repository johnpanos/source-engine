// DWARF declaration skeleton for game/server/portal2/prop_paint_bomb.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xb75b0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined ServerClass::ServerClass() at line 56
	// inlined CEntityFactory<CPropPaintBomb>::CEntityFactory() at line 60
}

// game/server/portal2/prop_paint_bomb.cpp:25
ConVar paintbomb_explosion_radius;

// game/server/portal2/prop_paint_bomb.cpp:27
ConVar paintbomb_streak_speed_min;

// game/server/portal2/prop_paint_bomb.cpp:28
ConVar paintbomb_streak_speed_max;

// game/server/portal2/prop_paint_bomb.cpp:30
ConVar paintbomb_blobs_min_streak_time;

// game/server/portal2/prop_paint_bomb.cpp:31
ConVar paintbomb_blobs_max_streak_time;

// game/server/portal2/prop_paint_bomb.cpp:32
ConVar paintbomb_blobs_min_streak_speed_dampen;

// game/server/portal2/prop_paint_bomb.cpp:33
ConVar paintbomb_blobs_max_streak_speed_dampen;

// game/server/portal2/prop_paint_bomb.cpp:35
ConVar paintbomb_vertical_angle_split;

// game/server/portal2/prop_paint_bomb.cpp:36
ConVar paintbomb_horizontal_angle_split;

// game/server/portal2/prop_paint_bomb.cpp:40
ConVar debug_paintbomb_explosion;

// game/server/portal2/prop_paint_bomb.cpp:44 (declaration)
void GetBaseMap();

// game/server/portal2/prop_paint_bomb.cpp:44 @0x7609c0 _ZN14CPropPaintBomb14GetDataDescMapEv
datamap_t *CPropPaintBomb::GetDataDescMap()
{
}

// game/server/portal2/prop_paint_bomb.cpp:44 @0x7609d0 _ZN14CPropPaintBomb10GetBaseMapEv
datamap_t *CPropPaintBomb::GetBaseMap()
{
}

// game/server/portal2/prop_paint_bomb.cpp:44 @0xb7480 _Z11DataMapInitI14CPropPaintBombEP9datamap_tPT_
datamap_t *DataMapInit<CPropPaintBomb>( CPropPaintBomb * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 44
	CDatadescGeneratedNameHolder nameHolder;  // line 44
	typedescription_t dataDesc[7];  // line 44
}

// game/server/portal2/prop_paint_bomb.cpp:44
datamap_t *g_DataMapHolder;

// game/server/portal2/prop_paint_bomb.cpp:44
void CPropPaintBomb::m_DataMap;

// game/server/portal2/prop_paint_bomb.cpp:56 @0x7609e0 _ZN14CPropPaintBomb14GetServerClassEv
ServerClass *CPropPaintBomb::GetServerClass()
{
}

// game/server/portal2/prop_paint_bomb.cpp:56 @0x7609f0 _ZN14CPropPaintBomb40YouForgotToImplementOrDeclareServerClassEv
int CPropPaintBomb::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/prop_paint_bomb.cpp:56 @0xb72a0 _Z15ServerClassInitIN16DT_PropPaintBomb7ignoredEEiPT_
int ServerClassInit<DT_PropPaintBomb::ignored>( DT_PropPaintBomb::ignored * )
{
	SendTable &sendTable;  // line 56
	char *const g_pSendTableName;  // line 56
	SendProp g_SendProps[3];  // line 56
}

// game/server/portal2/prop_paint_bomb.cpp:56
SendTable g_SendTable;

// game/server/portal2/prop_paint_bomb.cpp:56
int g_SendTableInit;

// game/server/portal2/prop_paint_bomb.cpp:56
static ServerClass g_CPropPaintBomb_ClassReg;

// game/server/portal2/prop_paint_bomb.cpp:56
void CPropPaintBomb::m_pClassSendTable;

// game/server/portal2/prop_paint_bomb.cpp:60
static CEntityFactory<CPropPaintBomb> prop_paint_bomb;

// game/server/portal2/prop_paint_bomb.cpp:62 (declaration)
void CPropPaintBomb();

// game/server/portal2/prop_paint_bomb.cpp:62 @0x760f70 _ZN14CPropPaintBombC2Ev
CPropPaintBomb::CPropPaintBomb()
{
	// inlined CNetworkVarBase<int,CPropPaintBomb::NetworkVar_m_nPaintPowerType>::CNetworkVarBase() at line 63
	// inlined COutputEvent::COutputEvent() at line 63
	// inlined COutputEvent::COutputEvent() at line 63
	// inlined COutputEvent::~COutputEvent() at line 65
	// inlined COutputEvent::~COutputEvent() at line 65
}

// game/server/portal2/prop_paint_bomb.cpp:62 @0x761180 _ZN14CPropPaintBombC1Ev
CPropPaintBomb::CPropPaintBomb()
{
}

// game/server/portal2/prop_paint_bomb.cpp:68 (declaration)
~CPropPaintBomb();

// game/server/portal2/prop_paint_bomb.cpp:68 @0x760d80 _ZN14CPropPaintBombD2Ev
CPropPaintBomb::~CPropPaintBomb()
{
	// inlined COutputEvent::~COutputEvent() at line 70
	// inlined COutputEvent::~COutputEvent() at line 70
	// inlined COutputEvent::~COutputEvent() at line 70
}

// game/server/portal2/prop_paint_bomb.cpp:68 @0x760e20 _ZN14CPropPaintBombD0Ev
CPropPaintBomb::~CPropPaintBomb()
{
	// inlined COutputEvent::~COutputEvent() at line 70
	// inlined COutputEvent::~COutputEvent() at line 70
	// inlined COutputEvent::~COutputEvent() at line 70
}

// game/server/portal2/prop_paint_bomb.cpp:68 @0x760ed0 _ZN14CPropPaintBombD1Ev
CPropPaintBomb::~CPropPaintBomb()
{
	// inlined COutputEvent::~COutputEvent() at line 70
	// inlined COutputEvent::~COutputEvent() at line 70
	// inlined COutputEvent::~COutputEvent() at line 70
}

// game/server/portal2/prop_paint_bomb.cpp:72 @0x760cd0 _ZN14CPropPaintBomb8PrecacheEv
void CPropPaintBomb::Precache()
{
}

// game/server/portal2/prop_paint_bomb.cpp:88 @0x761190 _ZN14CPropPaintBomb5SpawnEv
void CPropPaintBomb::Spawn()
{
	int nPaintBombSurfaceIndex;  // line 104
	IPhysicsObject *pPhysObject;  // line 105
	// inlined CBaseEntity::VPhysicsGetObject() at line 105
	// inlined CBaseEntity::AddSpawnFlags() at line 98
}

// game/server/util.h:100 @0x7631c0 _ZN14CEntityFactoryI14CPropPaintBombE6CreateEPKc
IServerNetworkable *CEntityFactory<CPropPaintBomb>::Create( const char *pClassName )
{
	CPropPaintBomb *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CPropPaintBomb>() at line 102
}

// game/server/util.h:106 @0x763180 _ZN14CEntityFactoryI14CPropPaintBombE7DestroyEP18IServerNetworkable
void CEntityFactory<CPropPaintBomb>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/portal2/prop_paint_bomb.cpp:112 @0x760cb0 _ZN14CPropPaintBomb19UpdateTransmitStateEv
int CPropPaintBomb::UpdateTransmitState()
{
}

// game/server/util.h:114 @0x7631b0 _ZN14CEntityFactoryI14CPropPaintBombE13GetEntitySizeEv
size_t CEntityFactory<CPropPaintBomb>::GetEntitySize()
{
}

// game/server/portal2/prop_paint_bomb.cpp:118 @0x762690 _ZN14CPropPaintBomb17SetPaintPowerTypeE14PaintPowerType
void CPropPaintBomb::SetPaintPowerType( PaintPowerType paintType )
{
	Color color;  // line 122
	// inlined CBaseEntity::SetRenderColor() at line 123
	// inlined CNetworkVarBase<int,CPropPaintBomb::NetworkVar_m_nPaintPowerType>::operator=<PaintPowerType>() at line 120
}

// game/server/portal2/prop_paint_bomb.cpp:127 @0x761390 _Z22GenerateBombDirectionsR10CUtlVectorI6Vector10CUtlMemoryIS0_iEE
GenerateBombDirections( CUtlVector<Vector,CUtlMemory<Vector, int> > &directions )
{
	float flVerticalAngle;  // line 135
	float flHorizontalAngle;  // line 136
	Vector vecTemp;  // line 139
	VMatrix matRotation;  // line 140
	int nVerticalSplit;  // line 142
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::AddToTail() at line 165
	// inlined Vector::Vector() at line 165
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::AddToHead() at line 164
	// inlined Vector::Vector() at line 164
	{
		int x;  // line 152
		{
			int startIndex;  // line 154
			{
				int y;  // line 155
				// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::AddToTail() at line 159
				// inlined Vector::operator-() at line 160
				// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::AddToHead() at line 160
				// inlined Vector::operator=() at line 157
				// inlined VMatrix::operator*() at line 158
				// inlined Vector::operator=() at line 158
			}
			// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::Count() at line 154
		}
	}
	{
		int i;  // line 143
		// inlined Vector::operator=() at line 145
		// inlined VMatrix::operator*() at line 145
		// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::AddToHead() at line 147
		// inlined Vector::operator-() at line 147
		// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::AddToTail() at line 146
	}
	// inlined ConVar::GetInt() at line 142
	// inlined Vector::operator VectorByValue&() at line 141
	// inlined Vector::Vector() at line 141
	// inlined Vector::operator VectorByValue&() at line 139
	// inlined Vector::Vector() at line 139
	// inlined ConVar::GetFloat() at line 136
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::Purge() at line 132
}

// game/server/portal2/prop_paint_bomb.cpp:169 @0x760b40 _Z29DispatchDryBombParticleEffectRK6Vector14PaintPowerType6QAngle
DispatchDryBombParticleEffect( const Vector &pos, PaintPowerType paintType, QAngle angSplash )
{
}

// game/server/portal2/prop_paint_bomb.cpp:192 @0x761c80 _Z24CreatePaintBombExplosion14PaintPowerTypeRK6Vectorb
CreatePaintBombExplosion( PaintPowerType paintType, const Vector &vecExplosionPos, bool bSpawnBlobs )
{
	CUtlVector<Vector,CUtlMemory<Vector, int> > dirs;  // line 203
	QAngle angSplashAngle;  // line 204
	const float radius;  // line 207
	// inlined QAngle::QAngle() at line 198
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::CUtlVector() at line 203
	// inlined QAngle::QAngle() at line 204
	// inlined ConVar::GetFloat() at line 207
	{
		int i;  // line 208
		{
			Vector vecBlobFireDir;  // line 211
			trace_t tr;  // line 213
			CTraceFilterSimpleClassnameList filter;  // line 214
			Ray_t ray;  // line 217
			CPortal_Base2D *pPortal;  // line 219
			// inlined Vector::operator VectorByValue&() at line 227
			// inlined Vector::operator-() at line 227
			// inlined Ray_t::Init() at line 218
			// inlined Vector::operator+() at line 218
			// inlined operator*() at line 218
			// inlined Vector::operator VectorByValue&() at line 211
			// inlined Vector::Normalized() at line 211
			// inlined CTraceFilterSimpleClassnameList::~CTraceFilterSimpleClassnameList() at line 208
			// inlined CTraceFilterSimpleClassnameList::~CTraceFilterSimpleClassnameList() at line 208
		}
	}
	{
		int i;  // line 233
		{
			CPortal_Player *pPlayer;  // line 235
			{
				const Vector &playerCenter;  // line 240
				const Vector playerToBomb;  // line 241
				// inlined Vector::operator-() at line 241
			}
			// inlined ToPortalPlayer() at line 235
		}
	}
	{
		int i;  // line 255
		{
			Vector vecBlobFireDir;  // line 258
			// inlined Vector::operator VectorByValue&() at line 259
			// inlined Vector::Normalized() at line 258
			// inlined Vector::operator VectorByValue&() at line 258
			// inlined Vector::operator*() at line 259
			// inlined Vector::operator+() at line 259
		}
	}
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::~CUtlVector() at line 261
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::~CUtlVector() at line 261
}

// game/server/portal2/prop_paint_bomb.cpp:265 @0x762550 _ZN14CPropPaintBomb12Event_KilledERK15CTakeDamageInfo
void CPropPaintBomb::Event_Killed( const CTakeDamageInfo &info )
{
	Vector vecPos;  // line 270
	bool bSpawnBlobs;  // line 273
	// inlined FClassnameIs() at line 273
	// inlined CTakeDamageInfo::GetInflictor() at line 273
	// inlined CBaseEntity::GetAbsOrigin() at line 270
	// inlined CTakeDamageInfo::GetInflictor() at line 268
}

// game/server/portal2/prop_paint_bomb.cpp:283 @0x763160 _ZN14CPropPaintBomb17CleansePaintPowerEv
void CPropPaintBomb::CleansePaintPower()
{
}

// game/server/portal2/prop_paint_bomb.cpp:289 @0x760af0 _ZN14CPropPaintBomb5BreakEP11CBaseEntityRK15CTakeDamageInfo
void CPropPaintBomb::Break( CBaseEntity *pBreaker, const CTakeDamageInfo &info )
{
}

// game/server/portal2/prop_paint_bomb.cpp:296 @0x760ac0 _ZN14CPropPaintBomb9OnFizzledEv
void CPropPaintBomb::OnFizzled()
{
}

// game/server/portal2/prop_paint_bomb.cpp:302 @0x760a00 _ZN14CPropPaintBomb13InputDissolveER11inputdata_t
void CPropPaintBomb::InputDissolve( inputdata_t &in )
{
}

// game/server/portal2/prop_paint_bomb.cpp:308 @0x760aa0 _ZN14CPropPaintBomb24InputDisablePortalFunnelER11inputdata_t
void CPropPaintBomb::InputDisablePortalFunnel( inputdata_t &in )
{
}

// game/server/portal2/prop_paint_bomb.cpp:313 @0x760ab0 _ZN14CPropPaintBomb23InputEnablePortalFunnelER11inputdata_t
void CPropPaintBomb::InputEnablePortalFunnel( inputdata_t &in )
{
}

// game/server/portal2/prop_paint_bomb.cpp:318 @0x762dc0 _Z15CreatePaintBomb14PaintPowerType
CreatePaintBomb( PaintPowerType paintType )
{
	CMDLCacheCriticalSection cacheCriticalSection;  // line 320
	bool bAllowPrecache;  // line 322
	CPropPaintBomb *pPaintBomb;  // line 326
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 351
	{
		CBasePlayer *pPlayer;  // line 334
		trace_t tr;  // line 335
		Vector vecForward;  // line 336
		// inlined UTIL_TraceLine() at line 338
		// inlined Vector::operator+() at line 338
		// inlined Vector::operator*() at line 338
	}
	// inlined CMDLCacheCriticalSection::CMDLCacheCriticalSection() at line 320
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 351
}

// game/server/portal2/prop_paint_bomb.cpp:354 @0x763150 _ZL26ent_create_paint_bomb_jumpRK8CCommand
ent_create_paint_bomb_jump( const CCommand &args )
{
}

// game/server/portal2/prop_paint_bomb.cpp:354
static ConCommand ent_create_paint_bomb_jump_command;

// game/server/portal2/prop_paint_bomb.cpp:359 @0x763140 _ZL27ent_create_paint_bomb_speedRK8CCommand
ent_create_paint_bomb_speed( const CCommand &args )
{
}

// game/server/portal2/prop_paint_bomb.cpp:359
static ConCommand ent_create_paint_bomb_speed_command;

// game/server/portal2/prop_paint_bomb.cpp:364 @0x763130 _ZL27ent_create_paint_bomb_stickRK8CCommand
ent_create_paint_bomb_stick( const CCommand &args )
{
}

// game/server/portal2/prop_paint_bomb.cpp:364
static ConCommand ent_create_paint_bomb_stick_command;

// game/server/portal2/prop_paint_bomb.cpp:369 @0x763120 _ZL28ent_create_paint_bomb_portalRK8CCommand
ent_create_paint_bomb_portal( const CCommand &args )
{
}

// game/server/portal2/prop_paint_bomb.cpp:369
static ConCommand ent_create_paint_bomb_portal_command;

// game/server/portal2/prop_paint_bomb.cpp:374 @0x763110 _ZL27ent_create_paint_bomb_eraseRK8CCommand
ent_create_paint_bomb_erase( const CCommand &args )
{
}

// game/server/portal2/prop_paint_bomb.cpp:374
static ConCommand ent_create_paint_bomb_erase_command;
