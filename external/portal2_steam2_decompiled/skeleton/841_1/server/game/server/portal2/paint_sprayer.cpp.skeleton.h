// DWARF declaration skeleton for game/server/portal2/paint_sprayer.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/paint_sprayer.cpp:0 @0x737450 _ZN13CPaintSprayer14GetServerClassEv
ServerClass *CPaintSprayer::GetServerClass()
{
}

// game/server/portal2/paint_sprayer.cpp:0 @0x737460 _ZN13CPaintSprayer40YouForgotToImplementOrDeclareServerClassEv
int CPaintSprayer::YouForgotToImplementOrDeclareServerClass()
{
}

// None:0 @0xae550 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined ServerClass::ServerClass() at line 14
	// inlined CEntityFactory<CPaintSprayer>::CEntityFactory() at line 73
}

// game/server/portal2/paint_sprayer.cpp:0
void CPaintSprayer::m_pClassSendTable;

// game/server/portal2/paint_sprayer.cpp:14 @0xadb70 _Z15ServerClassInitIN15DT_PaintSprayer7ignoredEEiPT_
int ServerClassInit<DT_PaintSprayer::ignored>( DT_PaintSprayer::ignored * )
{
	SendTable &sendTable;  // line 14
	char *const g_pSendTableName;  // line 14
	SendProp g_SendProps[20];  // line 14
}

// game/server/portal2/paint_sprayer.cpp:14
SendTable g_SendTable;

// game/server/portal2/paint_sprayer.cpp:14
int g_SendTableInit;

// game/server/portal2/paint_sprayer.cpp:14
static ServerClass g_CPaintSprayer_ClassReg;

// game/server/portal2/paint_sprayer.cpp:42 (declaration)
void GetBaseMap();

// game/server/portal2/paint_sprayer.cpp:42 @0x737470 _ZN13CPaintSprayer14GetDataDescMapEv
datamap_t *CPaintSprayer::GetDataDescMap()
{
}

// game/server/portal2/paint_sprayer.cpp:42 @0x737480 _ZN13CPaintSprayer10GetBaseMapEv
datamap_t *CPaintSprayer::GetBaseMap()
{
}

// game/server/portal2/paint_sprayer.cpp:42 @0xae330 _Z11DataMapInitI13CPaintSprayerEP9datamap_tPT_
datamap_t *DataMapInit<CPaintSprayer>( CPaintSprayer * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 71
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 42
	CDatadescGeneratedNameHolder nameHolder;  // line 42
	typedescription_t dataDesc[21];  // line 42
}

// game/server/portal2/paint_sprayer.cpp:42
datamap_t *g_DataMapHolder;

// game/server/portal2/paint_sprayer.cpp:42
void CPaintSprayer::m_DataMap;

// game/server/portal2/paint_sprayer.cpp:73
static CEntityFactory<CPaintSprayer> info_paint_sprayer;

// game/server/portal2/paint_sprayer.cpp:75 (declaration)
void CPaintSprayer();

// game/server/portal2/paint_sprayer.cpp:75 @0x738eb0 _ZN13CPaintSprayerC2Ev
CPaintSprayer::CPaintSprayer()
{
	// inlined CNetworkHandleBase<CPaintStream,CPaintSprayer::NetworkVar_m_hPaintStream>::CNetworkHandleBase() at line 81
	// inlined CNetworkVarBase<float,CPaintSprayer::NetworkVar_m_flStartTime>::CNetworkVarBase() at line 81
	// inlined CNetworkVarBase<bool,CPaintSprayer::NetworkVar_m_bActive>::CNetworkVarBase() at line 81
}

// game/server/portal2/paint_sprayer.cpp:75 @0x7391d0 _ZN13CPaintSprayerC1Ev
CPaintSprayer::CPaintSprayer()
{
}

// game/server/portal2/paint_sprayer.cpp:86 (declaration)
~CPaintSprayer();

// game/server/portal2/paint_sprayer.cpp:86 @0x737660 _ZN13CPaintSprayerD0Ev
CPaintSprayer::~CPaintSprayer()
{
}

// game/server/portal2/paint_sprayer.cpp:86 @0x7376a0 _ZN13CPaintSprayerD1Ev
CPaintSprayer::~CPaintSprayer()
{
}

// game/server/portal2/paint_sprayer.cpp:86 @0x7376c0 _ZN13CPaintSprayerD2Ev
CPaintSprayer::~CPaintSprayer()
{
}

// game/server/portal2/paint_sprayer.cpp:91 @0x7375d0 _ZN13CPaintSprayer8PrecacheEv
void CPaintSprayer::Precache()
{
}

// game/server/util.h:100 @0x73a020 _ZN14CEntityFactoryI13CPaintSprayerE6CreateEPKc
IServerNetworkable *CEntityFactory<CPaintSprayer>::Create( const char *pClassName )
{
	CPaintSprayer *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CPaintSprayer>() at line 102
}

// game/server/util.h:106 @0x739fe0 _ZN14CEntityFactoryI13CPaintSprayerE7DestroyEP18IServerNetworkable
void CEntityFactory<CPaintSprayer>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/portal2/paint_sprayer.cpp:108 @0x7397e0 _ZN13CPaintSprayer5SpawnEv
void CPaintSprayer::Spawn()
{
	{
		CPaintStream *pPaintStream;  // line 124
		// inlined CPaintStream::SetPaintType() at line 127
		// inlined CPaintStream::SetStreamRenderMode() at line 128
		// inlined CNetworkHandleBase<CPaintStream,CPaintSprayer::NetworkVar_m_hPaintStream>::operator=() at line 132
	}
	// inlined CNetworkHandleBase<CPaintStream,CPaintSprayer::NetworkVar_m_hPaintStream>::operator==() at line 122
}

// game/server/util.h:114 @0x73a010 _ZN14CEntityFactoryI13CPaintSprayerE13GetEntitySizeEv
size_t CEntityFactory<CPaintSprayer>::GetEntitySize()
{
}

// game/server/portal2/paint_sprayer.cpp:138 @0x739780 _ZN13CPaintSprayer8ActivateEv
void CPaintSprayer::Activate()
{
}

// game/server/portal2/paint_sprayer.cpp:153 @0x737560 _ZN13CPaintSprayer14UpdateOnRemoveEv
void CPaintSprayer::UpdateOnRemove()
{
	// inlined CNetworkHandleBase<CPaintStream,CPaintSprayer::NetworkVar_m_hPaintStream>::operator CPaintStream*() at line 155
}

// game/server/portal2/paint_sprayer.cpp:162 @0x737a50 _ZN13CPaintSprayer19SetPaintSprayerDataEbffffffffff
void CPaintSprayer::SetPaintSprayerData( bool bSilent, float flBlobsPerSecond, float flMinSpeed, float flMaxSpeed, float flBlobSpreadRadius, float flBlobSpreadAngle, float flBlobStreakPercent, float flMinStreakTime, float flMaxStreakTime, float flMinStreakSpeedDampen, float flMaxStreakSpeedDampen )
{
	// inlined CNetworkVarBase<bool,CPaintSprayer::NetworkVar_m_bSilent>::operator=<bool>() at line 164
	// inlined CNetworkVarBase<float,CPaintSprayer::NetworkVar_m_flBlobsPerSecond>::operator=<float>() at line 166
	// inlined CNetworkVarBase<float,CPaintSprayer::NetworkVar_m_flMinSpeed>::operator=<float>() at line 167
	// inlined CNetworkVarBase<float,CPaintSprayer::NetworkVar_m_flMaxSpeed>::operator=<float>() at line 168
	// inlined CNetworkVarBase<float,CPaintSprayer::NetworkVar_m_flBlobSpreadRadius>::operator=<float>() at line 169
	// inlined CNetworkVarBase<float,CPaintSprayer::NetworkVar_m_flBlobSpreadAngle>::operator=<float>() at line 170
	// inlined CNetworkVarBase<float,CPaintSprayer::NetworkVar_m_flStreakPercentage>::operator=<float>() at line 172
	// inlined CNetworkVarBase<float,CPaintSprayer::NetworkVar_m_flMinStreakTime>::operator=<float>() at line 173
	// inlined CNetworkVarBase<float,CPaintSprayer::NetworkVar_m_flMaxStreakTime>::operator=<float>() at line 174
	// inlined CNetworkVarBase<float,CPaintSprayer::NetworkVar_m_flMinStreakSpeedDampen>::operator=<float>() at line 175
	// inlined CNetworkVarBase<float,CPaintSprayer::NetworkVar_m_flMaxStreakSpeedDampen>::operator=<float>() at line 176
}

// game/server/portal2/paint_sprayer.cpp:180 (declaration)
void SetPaintPowerType( PaintPowerType paintType );

// game/server/portal2/paint_sprayer.cpp:180 @0x7378a0 _ZN13CPaintSprayer17SetPaintPowerTypeE14PaintPowerType
void CPaintSprayer::SetPaintPowerType( PaintPowerType paintType )
{
	// inlined CNetworkVarBase<int,CPaintSprayer::NetworkVar_m_PaintPowerType>::operator=<PaintPowerType>() at line 182
}

// game/server/portal2/paint_sprayer.cpp:186 @0x737510 _ZN13CPaintSprayer19UpdateTransmitStateEv
int CPaintSprayer::UpdateTransmitState()
{
}

// game/server/portal2/paint_sprayer.cpp:196 @0x737490 _ZN13CPaintSprayer17PaintSprayerThinkEv
void CPaintSprayer::PaintSprayerThink()
{
	{
		float flDeltaTime;  // line 200
	}
}

// game/server/portal2/paint_sprayer.cpp:209 @0x739770 _ZN13CPaintSprayer10InputStartER11inputdata_t
void CPaintSprayer::InputStart( inputdata_t &inputData )
{
}

// game/server/portal2/paint_sprayer.cpp:215 @0x7376e0 _ZN13CPaintSprayer9InputStopER11inputdata_t
void CPaintSprayer::InputStop( inputdata_t &inputData )
{
	// inlined CPaintSprayer::StopPaintSpray() at line 217
}

// game/server/portal2/paint_sprayer.cpp:221 @0x739e10 _ZN13CPaintSprayer20InputChangePaintTypeER11inputdata_t
void CPaintSprayer::InputChangePaintType( inputdata_t &inputData )
{
	// inlined variant_t::Int() at line 223
	// inlined CPaintSprayer::SetPaintPowerType() at line 223
}

// game/server/portal2/paint_sprayer.cpp:227 @0x7393a0 _ZN13CPaintSprayer15StartPaintSprayEv
void CPaintSprayer::StartPaintSpray()
{
	// inlined CNetworkVarBase<bool,CPaintSprayer::NetworkVar_m_bActive>::operator=<bool>() at line 231
	// inlined CNetworkVarBase<float,CPaintSprayer::NetworkVar_m_flStartTime>::operator=<float>() at line 235
}

// game/server/portal2/paint_sprayer.cpp:243 (declaration)
void StopPaintSpray();

// game/server/portal2/paint_sprayer.cpp:243 @0x7391e0 _ZN13CPaintSprayer14StopPaintSprayEv
void CPaintSprayer::StopPaintSpray()
{
	// inlined CNetworkVarBase<bool,CPaintSprayer::NetworkVar_m_bActive>::operator=<bool>() at line 245
}
