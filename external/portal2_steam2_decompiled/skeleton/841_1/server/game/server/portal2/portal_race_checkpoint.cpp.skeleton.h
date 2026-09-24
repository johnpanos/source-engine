// DWARF declaration skeleton for game/server/portal2/portal_race_checkpoint.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xb1ea0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined Color::Color() at line 16
	// inlined Color::Color() at line 17
	// inlined CEntityFactory<CPortalRaceCheckpoint>::CEntityFactory() at line 89
}

// game/server/portal2/portal_race_checkpoint.cpp:16
Color checkpointActiveColor;

// game/server/portal2/portal_race_checkpoint.cpp:17
Color checkpointInactiveColor;

// game/server/portal2/portal_race_checkpoint.cpp:19
ConVar sv_debug_portal_race_checkpoint;

// game/server/portal2/portal_race_checkpoint.cpp:20
ConVar sv_portal_race_checkpoint_model_scale;

// game/server/portal2/portal_race_checkpoint.cpp:23
char *g_szCheckpointStartSpinningThinkContext;

// game/server/portal2/portal_race_checkpoint.cpp:26 sizeof=0x554 (i386)
struct CPortalRaceCheckpoint : public CBaseAnimating
{
public:
	CPortalRaceCheckpoint();  // line 32
	virtual ~CPortalRaceCheckpoint();  // line 33
	virtual void Precache();  // line 35
	virtual void Spawn();  // line 36
	virtual void StartTouch( CBaseEntity * );  // line 38
	void ActivatedThink();  // line 40
	void StartSpinningThink();  // line 41
	void Lock();  // line 43
	void Unlock();  // line 44
private:
	void ActivateCheckpoint();  // line 46
	void OnCheckpointActivated();  // line 48
	void OnCheckpointReset( bool );  // line 49
	COutputEvent m_OnCheckpointActivated; // +0x514  // line 51
	COutputEvent m_OnCheckpointReset; // +0x52c  // line 52
	void InputActivate( inputdata_t & );  // line 54
	void InputCancelActivate( inputdata_t & );  // line 55
	void InputLock( inputdata_t & );  // line 57
	void InputUnlock( inputdata_t & );  // line 58
	float m_fResetTime; // +0x544  // line 60
	float m_fActivatedTime; // +0x548  // line 61
	float m_fLastTimerSound; // +0x54c  // line 62
	CNetworkVar( bool, m_bCheckpointActive ); // +0x550  // line 64
	bool m_bLocked; // +0x551  // line 65
};

// game/server/portal2/portal_race_checkpoint.cpp:29 @0x7469b0 _ZN21CPortalRaceCheckpoint14GetDataDescMapEv
datamap_t *CPortalRaceCheckpoint::GetDataDescMap()
{
}

// game/server/portal2/portal_race_checkpoint.cpp:29 @0x7469c0 _ZN21CPortalRaceCheckpoint10GetBaseMapEv
datamap_t *CPortalRaceCheckpoint::GetBaseMap()
{
}

// game/server/portal2/portal_race_checkpoint.cpp:32 @0x7473d0 _ZN21CPortalRaceCheckpointC2Ev
CPortalRaceCheckpoint::CPortalRaceCheckpoint()
{
	// inlined CNetworkVarBase<bool,CPortalRaceCheckpoint::NetworkVar_m_bCheckpointActive>::CNetworkVarBase() at line 94
	// inlined COutputEvent::COutputEvent() at line 94
	// inlined COutputEvent::COutputEvent() at line 94
	// inlined COutputEvent::~COutputEvent() at line 96
	// inlined COutputEvent::~COutputEvent() at line 96
}

// game/server/portal2/portal_race_checkpoint.cpp:32 @0x7475c0 _ZN21CPortalRaceCheckpointC1Ev
CPortalRaceCheckpoint::CPortalRaceCheckpoint()
{
}

// game/server/portal2/portal_race_checkpoint.cpp:33 @0x746ad0 _ZN21CPortalRaceCheckpointD0Ev
CPortalRaceCheckpoint::~CPortalRaceCheckpoint()
{
	// inlined COutputEvent::~COutputEvent() at line 101
	// inlined COutputEvent::~COutputEvent() at line 101
	// inlined COutputEvent::~COutputEvent() at line 101
}

// game/server/portal2/portal_race_checkpoint.cpp:33 @0x746b50 _ZN21CPortalRaceCheckpointD1Ev
CPortalRaceCheckpoint::~CPortalRaceCheckpoint()
{
	// inlined COutputEvent::~COutputEvent() at line 101
	// inlined COutputEvent::~COutputEvent() at line 101
	// inlined COutputEvent::~COutputEvent() at line 101
}

// game/server/portal2/portal_race_checkpoint.cpp:33 @0x746bc0 _ZN21CPortalRaceCheckpointD2Ev
CPortalRaceCheckpoint::~CPortalRaceCheckpoint()
{
	// inlined COutputEvent::~COutputEvent() at line 101
	// inlined COutputEvent::~COutputEvent() at line 101
	// inlined COutputEvent::~COutputEvent() at line 101
}

// game/server/portal2/portal_race_checkpoint.cpp:35 @0x746a30 _ZN21CPortalRaceCheckpoint8PrecacheEv
void CPortalRaceCheckpoint::Precache()
{
}

// game/server/portal2/portal_race_checkpoint.cpp:36 @0x746c90 _ZN21CPortalRaceCheckpoint5SpawnEv
void CPortalRaceCheckpoint::Spawn()
{
	float flModelScale;  // line 130
	float fScaledExtent;  // line 132
	const Vector vModelOffset;  // line 133
	// inlined Vector::operator VectorByValue&() at line 136
	// inlined Vector::Vector() at line 136
	// inlined Vector::operator VectorByValue&() at line 136
	// inlined Vector::Vector() at line 136
	// inlined Vector::operator VectorByValue&() at line 134
	// inlined Vector::operator+() at line 134
	// inlined CBaseEntity::GetAbsOrigin() at line 134
	// inlined ConVar::GetFloat() at line 130
	// inlined CBaseEntity::SetRenderColor() at line 126
	// inlined Color::r() at line 126
	// inlined Color::g() at line 126
	// inlined Color::b() at line 126
	// inlined CBaseEntity::SetSolidFlags() at line 123
	// inlined CBaseEntity::SetSolid() at line 122
}

// game/server/portal2/portal_race_checkpoint.cpp:38 @0x748580 _ZN21CPortalRaceCheckpoint10StartTouchEP11CBaseEntity
void CPortalRaceCheckpoint::StartTouch( CBaseEntity *pOther )
{
	// inlined CPortalRaceCheckpoint::ActivateCheckpoint() at line 237
}

// game/server/portal2/portal_race_checkpoint.cpp:40 @0x747c40 _ZN21CPortalRaceCheckpoint14ActivatedThinkEv
void CPortalRaceCheckpoint::ActivatedThink()
{
}

// game/server/portal2/portal_race_checkpoint.cpp:41 @0x746aa0 _ZN21CPortalRaceCheckpoint18StartSpinningThinkEv
void CPortalRaceCheckpoint::StartSpinningThink()
{
	// inlined Vector::Vector() at line 148
	// inlined Vector::operator VectorByValue&() at line 148
}

// game/server/portal2/portal_race_checkpoint.cpp:43 @0x7469e0 _ZN21CPortalRaceCheckpoint4LockEv
void CPortalRaceCheckpoint::Lock()
{
	QAngle angImpulse;  // line 276
	// inlined Vector::Vector() at line 277
	// inlined AngularImpulseToQAngle() at line 277
}

// game/server/portal2/portal_race_checkpoint.cpp:44 @0x7469d0 _ZN21CPortalRaceCheckpoint6UnlockEv
void CPortalRaceCheckpoint::Unlock()
{
}

// game/server/portal2/portal_race_checkpoint.cpp:46 @0x748460 _ZN21CPortalRaceCheckpoint18ActivateCheckpointEv
void CPortalRaceCheckpoint::ActivateCheckpoint()
{
}

// game/server/portal2/portal_race_checkpoint.cpp:48 @0x747d10 _ZN21CPortalRaceCheckpoint21OnCheckpointActivatedEv
void CPortalRaceCheckpoint::OnCheckpointActivated()
{
	// inlined CNetworkVarBase<bool,CPortalRaceCheckpoint::NetworkVar_m_bCheckpointActive>::operator=<bool>() at line 154
	// inlined Color::b() at line 160
	// inlined Color::g() at line 160
	// inlined Color::r() at line 160
	// inlined CBaseEntity::SetRenderColor() at line 160
}

// game/server/portal2/portal_race_checkpoint.cpp:49 @0x7475d0 _ZN21CPortalRaceCheckpoint17OnCheckpointResetEb
void CPortalRaceCheckpoint::OnCheckpointReset( bool bFireOutput )
{
	// inlined CNetworkVarBase<bool,CPortalRaceCheckpoint::NetworkVar_m_bCheckpointActive>::operator=<bool>() at line 169
	// inlined Color::b() at line 178
	// inlined Color::g() at line 178
	// inlined Color::r() at line 178
	// inlined CBaseEntity::SetRenderColor() at line 178
}

// game/server/portal2/portal_race_checkpoint.cpp:54 @0x7484f0 _ZN21CPortalRaceCheckpoint13InputActivateER11inputdata_t
void CPortalRaceCheckpoint::InputActivate( inputdata_t &inputData )
{
	// inlined CPortalRaceCheckpoint::ActivateCheckpoint() at line 306
}

// game/server/portal2/portal_race_checkpoint.cpp:55 @0x747c30 _ZN21CPortalRaceCheckpoint19InputCancelActivateER11inputdata_t
void CPortalRaceCheckpoint::InputCancelActivate( inputdata_t &inputData )
{
}

// game/server/portal2/portal_race_checkpoint.cpp:57 @0x746c40 _ZN21CPortalRaceCheckpoint9InputLockER11inputdata_t
void CPortalRaceCheckpoint::InputLock( inputdata_t &inputData )
{
	// inlined CPortalRaceCheckpoint::Lock() at line 294
}

// game/server/portal2/portal_race_checkpoint.cpp:58 @0x746c30 _ZN21CPortalRaceCheckpoint11InputUnlockER11inputdata_t
void CPortalRaceCheckpoint::InputUnlock( inputdata_t &inputData )
{
	// inlined CPortalRaceCheckpoint::Unlock() at line 300
}

// game/server/portal2/portal_race_checkpoint.cpp:68 @0xb1b90 _Z11DataMapInitI21CPortalRaceCheckpointEP9datamap_tPT_
datamap_t *DataMapInit<CPortalRaceCheckpoint>( CPortalRaceCheckpoint * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 83
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 83
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 68
	CDatadescGeneratedNameHolder nameHolder;  // line 68
	typedescription_t dataDesc[11];  // line 68
}

// game/server/portal2/portal_race_checkpoint.cpp:68
datamap_t *g_DataMapHolder;

// game/server/portal2/portal_race_checkpoint.cpp:89
static CEntityFactory<CPortalRaceCheckpoint> portal_race_checkpoint;

// game/server/util.h:100 @0x7486b0 _ZN14CEntityFactoryI21CPortalRaceCheckpointE6CreateEPKc
IServerNetworkable *CEntityFactory<CPortalRaceCheckpoint>::Create( const char *pClassName )
{
	CPortalRaceCheckpoint *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CPortalRaceCheckpoint>() at line 102
}

// game/server/util.h:106 @0x748670 _ZN14CEntityFactoryI21CPortalRaceCheckpointE7DestroyEP18IServerNetworkable
void CEntityFactory<CPortalRaceCheckpoint>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:114 @0x7486a0 _ZN14CEntityFactoryI21CPortalRaceCheckpointE13GetEntitySizeEv
size_t CEntityFactory<CPortalRaceCheckpoint>::GetEntitySize()
{
}
