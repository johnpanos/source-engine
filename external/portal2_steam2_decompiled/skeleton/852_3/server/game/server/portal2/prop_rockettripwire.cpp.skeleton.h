// DWARF declaration skeleton for game/server/portal2/prop_rockettripwire.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xcdd70 _Z41__static_initialization_and_destruction_0ii
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
	// inlined CEntityFactory<CPropRocketTripwire>::CEntityFactory() at line 34
	// inlined ServerClass::ServerClass() at line 66
}

// game/server/portal2/prop_rockettripwire.cpp:34
static CEntityFactory<CPropRocketTripwire> prop_rocket_tripwire;

// game/server/portal2/prop_rockettripwire.cpp:36 (declaration)
void GetBaseMap();

// game/server/portal2/prop_rockettripwire.cpp:36 @0x6f3df0 _ZN19CPropRocketTripwire14GetDataDescMapEv
datamap_t *CPropRocketTripwire::GetDataDescMap()
{
}

// game/server/portal2/prop_rockettripwire.cpp:36 @0x6f3e00 _ZN19CPropRocketTripwire10GetBaseMapEv
datamap_t *CPropRocketTripwire::GetBaseMap()
{
}

// game/server/portal2/prop_rockettripwire.cpp:36 @0xcda70 _Z11DataMapInitI19CPropRocketTripwireEP9datamap_tPT_
datamap_t *DataMapInit<CPropRocketTripwire>( CPropRocketTripwire * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 64
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 64
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 36
	CDatadescGeneratedNameHolder nameHolder;  // line 36
	typedescription_t dataDesc[21];  // line 36
}

// game/server/portal2/prop_rockettripwire.cpp:36
datamap_t *g_DataMapHolder;

// game/server/portal2/prop_rockettripwire.cpp:36
void CPropRocketTripwire::m_DataMap;

// game/server/portal2/prop_rockettripwire.cpp:66 @0x6f3e10 _ZN19CPropRocketTripwire14GetServerClassEv
ServerClass *CPropRocketTripwire::GetServerClass()
{
}

// game/server/portal2/prop_rockettripwire.cpp:66 @0x6f3e20 _ZN19CPropRocketTripwire40YouForgotToImplementOrDeclareServerClassEv
int CPropRocketTripwire::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/prop_rockettripwire.cpp:66 @0xcd7d0 _Z15ServerClassInitIN21DT_PropRocketTripwire7ignoredEEiPT_
int ServerClassInit<DT_PropRocketTripwire::ignored>( DT_PropRocketTripwire::ignored * )
{
	SendTable &sendTable;  // line 66
	char *const g_pSendTableName;  // line 66
	SendProp g_SendProps[6];  // line 66
}

// game/server/portal2/prop_rockettripwire.cpp:66
SendTable g_SendTable;

// game/server/portal2/prop_rockettripwire.cpp:66
int g_SendTableInit;

// game/server/portal2/prop_rockettripwire.cpp:66
static ServerClass g_CPropRocketTripwire_ClassReg;

// game/server/portal2/prop_rockettripwire.cpp:66
void CPropRocketTripwire::m_pClassSendTable;

// game/server/portal2/prop_rockettripwire.cpp:75 (declaration)
void CPropRocketTripwire();

// game/server/portal2/prop_rockettripwire.cpp:75 @0x6f4200 _ZN19CPropRocketTripwireC2Ev
CPropRocketTripwire::CPropRocketTripwire()
{
	// inlined CNetworkVarBase<CHandle<CBaseEntity>,CPropRocketTripwire::NetworkVar_m_hRocket>::CNetworkVarBase() at line 81
	// inlined CNetworkVarBase<bool,CPropRocketTripwire::NetworkVar_m_bDrawBeams>::CNetworkVarBase() at line 81
	// inlined CHandle<CRocket_Turret_Projectile>::CHandle() at line 81
	// inlined CHandle<CInfoPlacementHelper>::CHandle() at line 81
	// inlined COutputEvent::COutputEvent() at line 81
	// inlined COutputEvent::COutputEvent() at line 81
	// inlined COutputEvent::~COutputEvent() at line 83
	// inlined COutputEvent::~COutputEvent() at line 83
}

// game/server/portal2/prop_rockettripwire.cpp:75 @0x6f4320 _ZN19CPropRocketTripwireC1Ev
CPropRocketTripwire::CPropRocketTripwire()
{
}

// game/server/portal2/prop_rockettripwire.cpp:85 (declaration)
~CPropRocketTripwire();

// game/server/portal2/prop_rockettripwire.cpp:85 @0x6f3f80 _ZN19CPropRocketTripwireD2Ev
CPropRocketTripwire::~CPropRocketTripwire()
{
	// inlined CHandle<CInfoPlacementHelper>::operator CInfoPlacementHelper*() at line 87
	// inlined COutputEvent::~COutputEvent() at line 88
	// inlined COutputEvent::~COutputEvent() at line 88
	// inlined COutputEvent::~COutputEvent() at line 88
	// inlined COutputEvent::~COutputEvent() at line 88
}

// game/server/portal2/prop_rockettripwire.cpp:85 @0x6f4040 _ZN19CPropRocketTripwireD1Ev
CPropRocketTripwire::~CPropRocketTripwire()
{
}

// game/server/portal2/prop_rockettripwire.cpp:85 @0x6f4140 _ZN19CPropRocketTripwireD0Ev
CPropRocketTripwire::~CPropRocketTripwire()
{
	// inlined CHandle<CInfoPlacementHelper>::operator CInfoPlacementHelper*() at line 87
	// inlined COutputEvent::~COutputEvent() at line 88
	// inlined COutputEvent::~COutputEvent() at line 88
	// inlined COutputEvent::~COutputEvent() at line 88
	// inlined COutputEvent::~COutputEvent() at line 88
}

// game/server/portal2/prop_rockettripwire.cpp:90 @0x6f4330 _ZN19CPropRocketTripwire8PrecacheEv
void CPropRocketTripwire::Precache()
{
	// inlined CNetworkVarBase<int,CPropRocketTripwire::NetworkVar_m_nBeamEndpointTexture>::operator=<int>() at line 94
}

// game/server/util.h:99 @0x6f5310 _ZN14CEntityFactoryI19CPropRocketTripwireE6CreateEPKc
IServerNetworkable *CEntityFactory<CPropRocketTripwire>::Create( const char *pClassName )
{
	CPropRocketTripwire *pEnt;  // line 101
	// inlined _CreateEntityTemplate<CPropRocketTripwire>() at line 101
}

// game/server/util.h:105 @0x6f52d0 _ZN14CEntityFactoryI19CPropRocketTripwireE7DestroyEP18IServerNetworkable
void CEntityFactory<CPropRocketTripwire>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/portal2/prop_rockettripwire.cpp:105 @0x6f4a30 _ZN19CPropRocketTripwire5SpawnEv
void CPropRocketTripwire::Spawn()
{
	Vector vecMuzzle;  // line 111
	QAngle angMuzzleDir;  // line 112
	Vector vecAimDir;  // line 114
	// inlined CPropRocketTripwire::DisableTripwire() at line 128
	// inlined CHandle<CInfoPlacementHelper>::operator!=() at line 118
	// inlined CHandle<CInfoPlacementHelper>::operator=() at line 117
	// inlined CNetworkVarBase<int,CPropRocketTripwire::NetworkVar_m_iMuzzleAttachment>::operator=<int>() at line 110
	// inlined CPropRocketTripwire::EnableTripwire() at line 130
	// inlined CHandle<CInfoPlacementHelper>::operator->() at line 121
	// inlined CHandle<CInfoPlacementHelper>::Get() at line 122
	// inlined CHandle<CInfoPlacementHelper>::operator->() at line 123
}

// game/server/util.h:113 @0x6f5300 _ZN14CEntityFactoryI19CPropRocketTripwireE13GetEntitySizeEv
size_t CEntityFactory<CPropRocketTripwire>::GetEntitySize()
{
}

// game/server/portal2/prop_rockettripwire.cpp:136 @0x6f4090 _ZN19CPropRocketTripwire11EyePositionEv
Vector CPropRocketTripwire::EyePosition()
{
	Vector vMuzzlePos;  // line 138
	Vector vMuzzlePos;  // line 138
}

// game/server/portal2/prop_rockettripwire.cpp:143 @0x6f4050 _ZN19CPropRocketTripwire12EyeDirectionEv
Vector CPropRocketTripwire::EyeDirection()
{
	Vector vMuzzleDir;  // line 145
	Vector vMuzzlePos;  // line 145
	Vector vMuzzleDir;  // line 145
}

// game/server/portal2/prop_rockettripwire.cpp:150 @0x6f40d0 _ZN19CPropRocketTripwire14UpdateOnRemoveEv
void CPropRocketTripwire::UpdateOnRemove()
{
	// inlined CHandle<CInfoPlacementHelper>::Get() at line 152
}

// game/server/portal2/prop_rockettripwire.cpp:159 @0x6f5180 _ZN19CPropRocketTripwire12WaitingThinkEv
void CPropRocketTripwire::WaitingThink()
{
}

// game/server/portal2/prop_rockettripwire.cpp:177 @0x6f4e10 _ZN19CPropRocketTripwire11ActiveThinkEv
void CPropRocketTripwire::ActiveThink()
{
	Ray_t rayDmg;  // line 179
	Vector vForward;  // line 180
	Vector vEndPoint;  // line 181
	trace_t traceDmg;  // line 184
	CTraceFilterSimple subfilter;  // line 185
	CTraceFilterTranslateClones filter;  // line 186
	float flRequiredParameter;  // line 187
	CPortal_Base2D *pFirstPortal;  // line 188
	// inlined CPropRocketTripwire::PrepareToFireRocket() at line 195
	// inlined CPropRocketTripwire::IsAlternativeTripwireTarget() at line 193
	// inlined CTraceFilterTranslateClones::CTraceFilterTranslateClones() at line 186
	// inlined Ray_t::Init() at line 182
	// inlined Vector::operator+() at line 181
	// inlined Vector::operator*() at line 181
	// inlined Vector::operator VectorByValue&() at line 180
	// inlined Ray_t::Ray_t() at line 179
}

// game/server/portal2/prop_rockettripwire.cpp:204 (declaration)
void PrepareToFireRocket();

// game/server/portal2/prop_rockettripwire.cpp:204 @0x6f3ed0 _ZN19CPropRocketTripwire19PrepareToFireRocketEv
void CPropRocketTripwire::PrepareToFireRocket()
{
}

// game/server/portal2/prop_rockettripwire.cpp:220 @0x6f46f0 _ZN19CPropRocketTripwire15FireRocketThinkEv
void CPropRocketTripwire::FireRocketThink()
{
}

// game/server/portal2/prop_rockettripwire.cpp:225 @0x6f4410 _ZN19CPropRocketTripwire10FireRocketEv
void CPropRocketTripwire::FireRocket()
{
	Vector vForward;  // line 232
	QAngle spawnAngles;  // line 233
	CRocket_Turret_Projectile *pRocket;  // line 235
	// inlined CHandle<CBaseEntity>::CHandle() at line 242
	// inlined Vector::operator VectorByValue&() at line 241
	// inlined Vector::operator*() at line 241
	// inlined CNetworkVarBase<CHandle<CBaseEntity>,CPropRocketTripwire::NetworkVar_m_hRocket>::operator=<CRocket_Turret_Projectile*>() at line 239
	// inlined CHandle<CRocket_Turret_Projectile>::operator=() at line 237
	// inlined CHandle<CRocket_Turret_Projectile>::operator CRocket_Turret_Projectile*() at line 227
}

// game/server/portal2/prop_rockettripwire.cpp:255 (declaration)
void IsAlternativeTripwireTarget( CBaseEntity *pEnt );

// game/server/portal2/prop_rockettripwire.cpp:255 @0x6f3e30 _ZN19CPropRocketTripwire27IsAlternativeTripwireTargetEP11CBaseEntity
bool CPropRocketTripwire::IsAlternativeTripwireTarget( CBaseEntity *pEnt )
{
	{
		Vector vVel;  // line 269
		// inlined Vector::LengthSqr() at line 272
	}
}

// game/server/portal2/prop_rockettripwire.cpp:282 @0x6f47e0 _ZN19CPropRocketTripwire14PreFiringThinkEv
void CPropRocketTripwire::PreFiringThink()
{
	CRocket_Turret_Projectile *pRocket;  // line 284
	// inlined CNetworkVarBase<bool,CPropRocketTripwire::NetworkVar_m_bDrawBeams>::operator=<bool>() at line 313
	// inlined CHandle<CRocket_Turret_Projectile>::Get() at line 284
	// inlined CNetworkVarBase<bool,CPropRocketTripwire::NetworkVar_m_bDrawBeams>::operator=<bool>() at line 301
}

// game/server/portal2/prop_rockettripwire.cpp:321 @0x6f5220 _ZN19CPropRocketTripwire14InputForceFireER11inputdata_t
void CPropRocketTripwire::InputForceFire( inputdata_t &inputdata )
{
	// inlined CPropRocketTripwire::PrepareToFireRocket() at line 323
}

// game/server/portal2/prop_rockettripwire.cpp:327 @0x6f49c0 _ZN19CPropRocketTripwire11InputEnableER11inputdata_t
void CPropRocketTripwire::InputEnable( inputdata_t &inputdata )
{
	// inlined CPropRocketTripwire::EnableTripwire() at line 329
}

// game/server/portal2/prop_rockettripwire.cpp:333 @0x6f4da0 _ZN19CPropRocketTripwire12InputDisableER11inputdata_t
void CPropRocketTripwire::InputDisable( inputdata_t &inputdata )
{
	// inlined CPropRocketTripwire::DisableTripwire() at line 335
}

// game/server/portal2/prop_rockettripwire.cpp:339 (declaration)
void EnableTripwire();

// game/server/portal2/prop_rockettripwire.cpp:339 @0x6f4770 _ZN19CPropRocketTripwire14EnableTripwireEv
void CPropRocketTripwire::EnableTripwire()
{
	// inlined CNetworkVarBase<bool,CPropRocketTripwire::NetworkVar_m_bDrawBeams>::operator=<bool>() at line 342
}

// game/server/portal2/prop_rockettripwire.cpp:347 (declaration)
void DisableTripwire();

// game/server/portal2/prop_rockettripwire.cpp:347 @0x6f4700 _ZN19CPropRocketTripwire15DisableTripwireEv
void CPropRocketTripwire::DisableTripwire()
{
	// inlined CNetworkVarBase<bool,CPropRocketTripwire::NetworkVar_m_bDrawBeams>::operator=<bool>() at line 350
}
