// DWARF declaration skeleton for game/server/portal2/point_futbol_shooter.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xaf840 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CEntityFactory<CPointFutbolShooter>::CEntityFactory() at line 51
}

// game/server/portal2/point_futbol_shooter.cpp:15
ConVar futbol_shooter_distance_from_target;

// game/server/portal2/point_futbol_shooter.cpp:16
ConVar futbol_shooter_target_height_offset;

// game/server/portal2/point_futbol_shooter.cpp:19 sizeof=0x400 (i386)
struct CPointFutbolShooter : public CPointEntity
{
public:
	virtual void Spawn();  // line 24
	virtual void Precache();  // line 25
protected:
	void InputShootFutbol( inputdata_t & );  // line 29
	void InputSetTarget( inputdata_t & );  // line 30
	COutputEvent m_OutputShootSuccess; // +0x3c0  // line 32
	COutputEvent m_OutputShootFail; // +0x3d8  // line 33
	float m_flPhysicsSpeed; // +0x3f0  // line 35
	Vector m_vTarget; // +0x3f4  // line 36
};

// game/server/portal2/point_futbol_shooter.cpp:19 (declaration)
void CPointFutbolShooter();

// game/server/portal2/point_futbol_shooter.cpp:19 (declaration)
~CPointFutbolShooter();

// game/server/portal2/point_futbol_shooter.cpp:19 @0x7405b0 _ZN19CPointFutbolShooterD1Ev
CPointFutbolShooter::~CPointFutbolShooter()
{
	// inlined COutputEvent::~COutputEvent() at line 19
	// inlined COutputEvent::~COutputEvent() at line 19
	// inlined CPointEntity::~CPointEntity() at line 19
	// inlined COutputEvent::~COutputEvent() at line 19
	// inlined CPointEntity::~CPointEntity() at line 19
}

// game/server/portal2/point_futbol_shooter.cpp:19 @0x740640 _ZN19CPointFutbolShooterD0Ev
CPointFutbolShooter::~CPointFutbolShooter()
{
	// inlined COutputEvent::~COutputEvent() at line 19
	// inlined COutputEvent::~COutputEvent() at line 19
	// inlined CPointEntity::~CPointEntity() at line 19
	// inlined COutputEvent::~COutputEvent() at line 19
	// inlined CPointEntity::~CPointEntity() at line 19
}

// game/server/portal2/point_futbol_shooter.cpp:22 @0x73fd90 _ZN19CPointFutbolShooter14GetDataDescMapEv
datamap_t *CPointFutbolShooter::GetDataDescMap()
{
}

// game/server/portal2/point_futbol_shooter.cpp:22 @0x73fda0 _ZN19CPointFutbolShooter10GetBaseMapEv
datamap_t *CPointFutbolShooter::GetBaseMap()
{
}

// game/server/portal2/point_futbol_shooter.cpp:24 @0x73fdf0 _ZN19CPointFutbolShooter5SpawnEv
void CPointFutbolShooter::Spawn()
{
}

// game/server/portal2/point_futbol_shooter.cpp:25 @0x73fdb0 _ZN19CPointFutbolShooter8PrecacheEv
void CPointFutbolShooter::Precache()
{
}

// game/server/portal2/point_futbol_shooter.cpp:29 @0x73fe20 _ZN19CPointFutbolShooter16InputShootFutbolER11inputdata_t
void CPointFutbolShooter::InputShootFutbol( inputdata_t &data )
{
	Vector vecSourcePos;  // line 69
	Vector vecTargetPos;  // line 70
	Vector vecVelocity;  // line 71
	Vector vecForward;  // line 83
	CBaseEntity *pFutbol;  // line 95
	float flSpeed;  // line 106
	float flGravity;  // line 107
	float time;  // line 110
	IPhysicsObject *pPhysObject;  // line 117
	Vector color;  // line 140
	Vector vecReticulePos;  // line 141
	// inlined CBaseEntity::GetAbsOrigin() at line 69
	// inlined Vector::operator-() at line 71
	{
		Vector vecToTarget;  // line 77
		// inlined Vector::NormalizeInPlace() at line 79
		// inlined ConVar::GetFloat() at line 80
		// inlined Vector::operator*() at line 80
		// inlined Vector::operator-=() at line 80
	}
	// inlined CBaseEntity::GetAbsAngles() at line 84
	// inlined Vector::AsVector2D() at line 87
	// inlined ConVar::GetFloat() at line 107
	// inlined Vector::Length() at line 110
	// inlined Vector::operator*() at line 111
	// inlined Vector::operator=() at line 111
	// inlined CBaseEntity::VPhysicsGetObject() at line 117
	{
		AngularImpulse angImpulse;  // line 120
		float flNull;  // line 124
		CPhysicsProp *pProp;  // line 128
		// inlined RandomAngularImpulse() at line 120
	}
	// inlined Vector::Vector() at line 140
}

// game/server/portal2/point_futbol_shooter.cpp:30 @0x7403b0 _ZN19CPointFutbolShooter14InputSetTargetER11inputdata_t
void CPointFutbolShooter::InputSetTarget( inputdata_t &data )
{
	EHANDLE hTarget;  // line 148
	CBaseEntity *pTargetEnt;  // line 149
	// inlined Vector::operator=() at line 156
	// inlined CBaseEntity::GetAbsOrigin() at line 156
	// inlined variant_t::StringID() at line 148
	// inlined CGlobalEntityList::FindEntityByName() at line 148
	// inlined CHandle<CBaseEntity>::CHandle() at line 148
	// inlined CHandle<CBaseEntity>::Get() at line 149
}

// game/server/portal2/point_futbol_shooter.cpp:39 @0xaf710 _Z11DataMapInitI19CPointFutbolShooterEP9datamap_tPT_
datamap_t *DataMapInit<CPointFutbolShooter>( CPointFutbolShooter * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 39
	CDatadescGeneratedNameHolder nameHolder;  // line 39
	typedescription_t dataDesc[6];  // line 39
}

// game/server/portal2/point_futbol_shooter.cpp:39
datamap_t *g_DataMapHolder;

// game/server/portal2/point_futbol_shooter.cpp:51
static CEntityFactory<CPointFutbolShooter> point_futbol_shooter;

// game/server/util.h:100 @0x740510 _ZN14CEntityFactoryI19CPointFutbolShooterE6CreateEPKc
IServerNetworkable *CEntityFactory<CPointFutbolShooter>::Create( const char *pClassName )
{
	CPointFutbolShooter *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CPointFutbolShooter>() at line 102
}

// game/server/util.h:106 @0x7404d0 _ZN14CEntityFactoryI19CPointFutbolShooterE7DestroyEP18IServerNetworkable
void CEntityFactory<CPointFutbolShooter>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:114 @0x740500 _ZN14CEntityFactoryI19CPointFutbolShooterE13GetEntitySizeEv
size_t CEntityFactory<CPointFutbolShooter>::GetEntitySize()
{
}
