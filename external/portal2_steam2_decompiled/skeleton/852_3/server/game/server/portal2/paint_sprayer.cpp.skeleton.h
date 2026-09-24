// DWARF declaration skeleton for game/server/portal2/paint_sprayer.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xc39f0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSteamID::CSteamID() at line 650
	// inlined CSteamID::CSteamID() at line 654
	// inlined CSteamID::CSteamID() at line 656
	// inlined CSteamID::CSteamID() at line 659
	// inlined CSteamID::CSteamID() at line 662
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
	// inlined ServerClass::ServerClass() at line 14
	// inlined CEntityFactory<CPaintSprayer>::CEntityFactory() at line 65
}

// game/server/portal2/paint_sprayer.cpp:14 @0x6d0700 _ZN13CPaintSprayer14GetServerClassEv
ServerClass *CPaintSprayer::GetServerClass()
{
}

// game/server/portal2/paint_sprayer.cpp:14 @0x6d0710 _ZN13CPaintSprayer40YouForgotToImplementOrDeclareServerClassEv
int CPaintSprayer::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/paint_sprayer.cpp:14 @0xc30a0 _Z15ServerClassInitIN15DT_PaintSprayer7ignoredEEiPT_
int ServerClassInit<DT_PaintSprayer::ignored>( DT_PaintSprayer::ignored * )
{
	SendTable &sendTable;  // line 14
	char *const g_pSendTableName;  // line 14
	SendProp g_SendProps[18];  // line 14
}

// game/server/portal2/paint_sprayer.cpp:14
SendTable g_SendTable;

// game/server/portal2/paint_sprayer.cpp:14
int g_SendTableInit;

// game/server/portal2/paint_sprayer.cpp:14
static ServerClass g_CPaintSprayer_ClassReg;

// game/server/portal2/paint_sprayer.cpp:14
void CPaintSprayer::m_pClassSendTable;

// game/server/portal2/paint_sprayer.cpp:39 (declaration)
void GetBaseMap();

// game/server/portal2/paint_sprayer.cpp:39 @0x6d0720 _ZN13CPaintSprayer14GetDataDescMapEv
datamap_t *CPaintSprayer::GetDataDescMap()
{
}

// game/server/portal2/paint_sprayer.cpp:39 @0x6d0730 _ZN13CPaintSprayer10GetBaseMapEv
datamap_t *CPaintSprayer::GetBaseMap()
{
}

// game/server/portal2/paint_sprayer.cpp:39 @0xc37d0 _Z11DataMapInitI13CPaintSprayerEP9datamap_tPT_
datamap_t *DataMapInit<CPaintSprayer>( CPaintSprayer * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 63
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 39
	CDatadescGeneratedNameHolder nameHolder;  // line 39
	typedescription_t dataDesc[19];  // line 39
}

// game/server/portal2/paint_sprayer.cpp:39
datamap_t *g_DataMapHolder;

// game/server/portal2/paint_sprayer.cpp:39
void CPaintSprayer::m_DataMap;

// game/server/portal2/paint_sprayer.cpp:65
static CEntityFactory<CPaintSprayer> info_paint_sprayer;

// game/server/portal2/paint_sprayer.cpp:67 (declaration)
void CPaintSprayer();

// game/server/portal2/paint_sprayer.cpp:67 @0x6d0a00 _ZN13CPaintSprayerC2Ev
CPaintSprayer::CPaintSprayer()
{
	// inlined CNetworkVarBase<float,CPaintSprayer::NetworkVar_m_flStartTime>::CNetworkVarBase() at line 73
	// inlined CNetworkVarBase<bool,CPaintSprayer::NetworkVar_m_bActive>::CNetworkVarBase() at line 73
}

// game/server/portal2/paint_sprayer.cpp:67 @0x6d0ae0 _ZN13CPaintSprayerC1Ev
CPaintSprayer::CPaintSprayer()
{
}

// game/server/portal2/paint_sprayer.cpp:78 (declaration)
~CPaintSprayer();

// game/server/portal2/paint_sprayer.cpp:78 @0x6d08a0 _ZN13CPaintSprayerD0Ev
CPaintSprayer::~CPaintSprayer()
{
}

// game/server/portal2/paint_sprayer.cpp:78 @0x6d08e0 _ZN13CPaintSprayerD1Ev
CPaintSprayer::~CPaintSprayer()
{
}

// game/server/portal2/paint_sprayer.cpp:78 @0x6d0900 _ZN13CPaintSprayerD2Ev
CPaintSprayer::~CPaintSprayer()
{
}

// game/server/portal2/paint_sprayer.cpp:83 @0x6d0810 _ZN13CPaintSprayer8PrecacheEv
void CPaintSprayer::Precache()
{
}

// game/server/util.h:99 @0x6d1240 _ZN14CEntityFactoryI13CPaintSprayerE6CreateEPKc
IServerNetworkable *CEntityFactory<CPaintSprayer>::Create( const char *pClassName )
{
	CPaintSprayer *pEnt;  // line 101
	// inlined _CreateEntityTemplate<CPaintSprayer>() at line 101
}

// game/server/portal2/paint_sprayer.cpp:100 @0x6d10e0 _ZN13CPaintSprayer5SpawnEv
void CPaintSprayer::Spawn()
{
}

// game/server/util.h:105 @0x6d1200 _ZN14CEntityFactoryI13CPaintSprayerE7DestroyEP18IServerNetworkable
void CEntityFactory<CPaintSprayer>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:113 @0x6d1230 _ZN14CEntityFactoryI13CPaintSprayerE13GetEntitySizeEv
size_t CEntityFactory<CPaintSprayer>::GetEntitySize()
{
}

// game/server/portal2/paint_sprayer.cpp:116 @0x6d1090 _ZN13CPaintSprayer8ActivateEv
void CPaintSprayer::Activate()
{
}

// game/server/portal2/paint_sprayer.cpp:129 @0x6d0af0 _ZN13CPaintSprayer19SetPaintSprayerDataEbffffffffff
void CPaintSprayer::SetPaintSprayerData( bool bSilent, float flBlobsPerSecond, float flMinSpeed, float flMaxSpeed, float flBlobSpreadRadius, float flBlobSpreadAngle, float flBlobStreakPercent, float flMinStreakTime, float flMaxStreakTime, float flMinStreakSpeedDampen, float flMaxStreakSpeedDampen )
{
	// inlined CNetworkVarBase<bool,CPaintSprayer::NetworkVar_m_bSilent>::operator=<bool>() at line 131
	// inlined CNetworkVarBase<float,CPaintSprayer::NetworkVar_m_flBlobsPerSecond>::operator=<float>() at line 133
	// inlined CNetworkVarBase<float,CPaintSprayer::NetworkVar_m_flMinSpeed>::operator=<float>() at line 134
	// inlined CNetworkVarBase<float,CPaintSprayer::NetworkVar_m_flMaxSpeed>::operator=<float>() at line 135
	// inlined CNetworkVarBase<float,CPaintSprayer::NetworkVar_m_flBlobSpreadRadius>::operator=<float>() at line 136
	// inlined CNetworkVarBase<float,CPaintSprayer::NetworkVar_m_flBlobSpreadAngle>::operator=<float>() at line 137
	// inlined CNetworkVarBase<float,CPaintSprayer::NetworkVar_m_flStreakPercentage>::operator=<float>() at line 139
	// inlined CNetworkVarBase<float,CPaintSprayer::NetworkVar_m_flMinStreakTime>::operator=<float>() at line 140
	// inlined CNetworkVarBase<float,CPaintSprayer::NetworkVar_m_flMaxStreakTime>::operator=<float>() at line 141
	// inlined CNetworkVarBase<float,CPaintSprayer::NetworkVar_m_flMinStreakSpeedDampen>::operator=<float>() at line 142
	// inlined CNetworkVarBase<float,CPaintSprayer::NetworkVar_m_flMaxStreakSpeedDampen>::operator=<float>() at line 143
}

// game/server/portal2/paint_sprayer.cpp:147 (declaration)
void SetPaintPowerType( PaintPowerType paintType );

// game/server/portal2/paint_sprayer.cpp:147 @0x6d09a0 _ZN13CPaintSprayer17SetPaintPowerTypeE14PaintPowerType
void CPaintSprayer::SetPaintPowerType( PaintPowerType paintType )
{
	// inlined CNetworkVarBase<int,CPaintSprayer::NetworkVar_m_PaintPowerType>::operator=<PaintPowerType>() at line 149
}

// game/server/portal2/paint_sprayer.cpp:153 @0x6d07c0 _ZN13CPaintSprayer19UpdateTransmitStateEv
int CPaintSprayer::UpdateTransmitState()
{
}

// game/server/portal2/paint_sprayer.cpp:163 @0x6d0740 _ZN13CPaintSprayer17PaintSprayerThinkEv
void CPaintSprayer::PaintSprayerThink()
{
	{
		float flDeltaTime;  // line 167
	}
}

// game/server/portal2/paint_sprayer.cpp:176 @0x6d1080 _ZN13CPaintSprayer10InputStartER11inputdata_t
void CPaintSprayer::InputStart( inputdata_t &inputData )
{
}

// game/server/portal2/paint_sprayer.cpp:182 @0x6d0920 _ZN13CPaintSprayer9InputStopER11inputdata_t
void CPaintSprayer::InputStop( inputdata_t &inputData )
{
	// inlined CPaintSprayer::StopPaintSpray() at line 184
}

// game/server/portal2/paint_sprayer.cpp:188 @0x6d1180 _ZN13CPaintSprayer20InputChangePaintTypeER11inputdata_t
void CPaintSprayer::InputChangePaintType( inputdata_t &inputData )
{
	// inlined variant_t::Int() at line 190
	// inlined CPaintSprayer::SetPaintPowerType() at line 190
}

// game/server/portal2/paint_sprayer.cpp:194 @0x6d0f60 _ZN13CPaintSprayer15StartPaintSprayEv
void CPaintSprayer::StartPaintSpray()
{
	// inlined CNetworkVarBase<bool,CPaintSprayer::NetworkVar_m_bActive>::operator=<bool>() at line 198
	// inlined CNetworkVarBase<float,CPaintSprayer::NetworkVar_m_flStartTime>::operator=<float>() at line 202
}

// game/server/portal2/paint_sprayer.cpp:210 (declaration)
void StopPaintSpray();

// game/server/portal2/paint_sprayer.cpp:210 @0x6d0ee0 _ZN13CPaintSprayer14StopPaintSprayEv
void CPaintSprayer::StopPaintSpray()
{
	// inlined CNetworkVarBase<bool,CPaintSprayer::NetworkVar_m_bActive>::operator=<bool>() at line 212
}
