// DWARF declaration skeleton for game/client/portal2/c_npc_hover_turret.cpp
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x74c20 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
	// inlined ClientClass::ClientClass() at line 56
}

// game/client/portal2/c_npc_hover_turret.cpp:25 sizeof=0x12ec (i386)
struct C_NPC_Hover_Turret : public C_AI_BaseNPC
{
public:
	virtual ~C_NPC_Hover_Turret();  // line 30
	virtual void Spawn();  // line 32
	virtual void ClientThink();  // line 33
	bool IsLaserOn();  // line 35
	void LaserOff();  // line 36
	void LaserOn();  // line 37
	float LaserEndPointSize();  // line 38
private:
	C_Beam *m_pBeam; // +0x12cc  // line 41
	int m_iLastLaserState; // +0x12d0  // line 44
	int m_iLaserState; // +0x12d4  // line 45
	int m_sLaserHaloSprite; // +0x12d8  // line 46
	float m_fPulseOffset; // +0x12dc  // line 47
	float m_flLastUpdateTime; // +0x12e0  // line 48
	float m_bBeamFlickerOff; // +0x12e4  // line 50
	float m_fBeamFlickerTime; // +0x12e8  // line 51
};

// game/client/portal2/c_npc_hover_turret.cpp:25 (declaration)
void C_NPC_Hover_Turret();

// game/client/portal2/c_npc_hover_turret.cpp:28 @0x5228c0 _ZN18C_NPC_Hover_Turret40YouForgotToImplementOrDeclareClientClassEv
int C_NPC_Hover_Turret::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/portal2/c_npc_hover_turret.cpp:28 @0x5228e0 _ZN18C_NPC_Hover_Turret14GetClientClassEv
ClientClass *C_NPC_Hover_Turret::GetClientClass()
{
}

// game/client/portal2/c_npc_hover_turret.cpp:30 @0x523080 _ZN18C_NPC_Hover_TurretD0Ev
C_NPC_Hover_Turret::~C_NPC_Hover_Turret()
{
	// inlined C_NPC_Hover_Turret::LaserOff() at line 67
	// inlined C_AI_BaseNPC::~C_AI_BaseNPC() at line 72
	// inlined C_AI_BaseNPC::~C_AI_BaseNPC() at line 72
}

// game/client/portal2/c_npc_hover_turret.cpp:30 @0x5231d0 _ZN18C_NPC_Hover_TurretD2Ev
C_NPC_Hover_Turret::~C_NPC_Hover_Turret()
{
	// inlined C_NPC_Hover_Turret::LaserOff() at line 67
	// inlined C_AI_BaseNPC::~C_AI_BaseNPC() at line 72
	// inlined C_AI_BaseNPC::~C_AI_BaseNPC() at line 72
}

// game/client/portal2/c_npc_hover_turret.cpp:30 @0x523310 _ZN18C_NPC_Hover_TurretD1Ev
C_NPC_Hover_Turret::~C_NPC_Hover_Turret()
{
	// inlined C_NPC_Hover_Turret::LaserOff() at line 67
	// inlined C_AI_BaseNPC::~C_AI_BaseNPC() at line 72
	// inlined C_AI_BaseNPC::~C_AI_BaseNPC() at line 72
}

// game/client/portal2/c_npc_hover_turret.cpp:32 @0x522a70 _ZN18C_NPC_Hover_Turret5SpawnEv
void C_NPC_Hover_Turret::Spawn()
{
}

// game/client/portal2/c_npc_hover_turret.cpp:33 @0x523040 _ZN18C_NPC_Hover_Turret11ClientThinkEv
void C_NPC_Hover_Turret::ClientThink()
{
	// inlined C_NPC_Hover_Turret::LaserOff() at line 101
}

// game/client/portal2/c_npc_hover_turret.cpp:36 @0x522a40 _ZN18C_NPC_Hover_Turret8LaserOffEv
void C_NPC_Hover_Turret::LaserOff()
{
}

// game/client/portal2/c_npc_hover_turret.cpp:37 @0x522b00 _ZN18C_NPC_Hover_Turret7LaserOnEv
void C_NPC_Hover_Turret::LaserOn()
{
	Vector vecMuzzle;  // line 120
	QAngle angMuzzleDir;  // line 121
	Vector vecEye;  // line 124
	QAngle angEyeDir;  // line 125
	Vector vecMuzzleDir;  // line 128
	Vector vEndPoint;  // line 163
	float fEndFraction;  // line 164
	Ray_t rayPath;  // line 165
	CTraceFilterSkipClassname traceFilter;  // line 169
	// inlined C_NPC_Hover_Turret::LaserEndPointSize() at line 180
	// inlined Vector::operator=() at line 172
	// inlined Vector::operator+() at line 172
	// inlined Vector::operator*() at line 172
	// inlined Ray_t::Init() at line 167
	// inlined Vector::operator+() at line 167
	// inlined Vector::operator*() at line 167
	// inlined C_Beam::SetHaloScale() at line 180
	// inlined C_NPC_Hover_Turret::LaserOff() at line 116
	// inlined C_BaseEntity::RemoveEffects() at line 157
	// inlined Vector::operator*() at line 174
	// inlined Vector::operator*() at line 174
	// inlined Vector::operator+() at line 174
	// inlined Vector::operator=() at line 174
	// inlined C_Beam::SetWidth() at line 151
	// inlined C_Beam::SetEndWidth() at line 152
	// inlined C_Beam::SetColor() at line 143
	// inlined C_Beam::SetHaloTexture() at line 144
	// inlined C_Beam::SetHaloScale() at line 145
}

// game/client/portal2/c_npc_hover_turret.cpp:38 @0x5229a0 _ZN18C_NPC_Hover_Turret17LaserEndPointSizeEv
float C_NPC_Hover_Turret::LaserEndPointSize()
{
}

// game/client/portal2/c_npc_hover_turret.cpp:56 @0x5228f0 _ZL32_C_NPC_Hover_Turret_CreateObjectii
IClientNetworkable *_C_NPC_Hover_Turret_CreateObject( int entnum, int serialNum )
{
	C_NPC_Hover_Turret *pRet;  // line 56
	// inlined C_NPC_Hover_Turret::C_NPC_Hover_Turret() at line 56
}

// game/client/portal2/c_npc_hover_turret.cpp:56 @0x74a90 _Z15ClientClassInitIN18DT_NPC_HoverTurret7ignoredEEiPT_
int ClientClassInit<DT_NPC_HoverTurret::ignored>( DT_NPC_HoverTurret::ignored * )
{
	char *pRecvTableName;  // line 56
	RecvTable &RecvTable;  // line 56
	RecvProp RecvProps[4];  // line 56
}

// game/client/portal2/c_npc_hover_turret.cpp:56
RecvTable g_RecvTable;

// game/client/portal2/c_npc_hover_turret.cpp:56
int g_RecvTableInit;

// game/client/portal2/c_npc_hover_turret.cpp:56
ClientClass __g_C_NPC_Hover_TurretClientClass;
