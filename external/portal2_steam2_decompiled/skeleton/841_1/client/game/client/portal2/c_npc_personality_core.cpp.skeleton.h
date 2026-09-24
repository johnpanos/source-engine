// DWARF declaration skeleton for game/client/portal2/c_npc_personality_core.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x74d80 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined ClientClass::ClientClass() at line 187
	// inlined CLightedMouthFoo::CLightedMouthFoo() at line 231
}

// game/client/portal2/c_npc_personality_core.cpp:18 sizeof=0x12e8 (i386)
struct C_NPC_Personality_Core : public C_AI_BaseNPC
{
public:
	virtual void UpdateOnRemove();  // line 26
	virtual void ControlMouth( CStudioHdr * );  // line 46
	virtual void OnDataChanged( DataUpdateType_t );  // line 77
	virtual void ClientThink();  // line 95
	float GetMouthAmount() const;  // line 175
private:
	float m_flMouthAmount; // +0x12cc  // line 178
	float m_flInvDim; // +0x12d0  // line 179
	bool m_bFlashlightEnabled; // +0x12d4  // line 181
	CUtlReference<CNewParticleEffect> m_pFlashlightEffect; // +0x12d8  // line 183
	dlight_t *m_pELight; // +0x12e4  // line 184
};

// game/client/portal2/c_npc_personality_core.cpp:18 (declaration)
void C_NPC_Personality_Core();

// game/client/portal2/c_npc_personality_core.cpp:18 (declaration)
~C_NPC_Personality_Core();

// game/client/portal2/c_npc_personality_core.cpp:18 @0x51eb20 _ZN22C_NPC_Personality_CoreD0Ev
C_NPC_Personality_Core::~C_NPC_Personality_Core()
{
	// inlined CUtlReference<CNewParticleEffect>::~CUtlReference() at line 18
	// inlined C_AI_BaseNPC::~C_AI_BaseNPC() at line 18
}

// game/client/portal2/c_npc_personality_core.cpp:18 @0x51ec60 _ZN22C_NPC_Personality_CoreD1Ev
C_NPC_Personality_Core::~C_NPC_Personality_Core()
{
	// inlined CUtlReference<CNewParticleEffect>::~CUtlReference() at line 18
	// inlined C_AI_BaseNPC::~C_AI_BaseNPC() at line 18
}

// game/client/portal2/c_npc_personality_core.cpp:21 @0x51de90 _ZN22C_NPC_Personality_Core40YouForgotToImplementOrDeclareClientClassEv
int C_NPC_Personality_Core::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/portal2/c_npc_personality_core.cpp:21 @0x51deb0 _ZN22C_NPC_Personality_Core14GetClientClassEv
ClientClass *C_NPC_Personality_Core::GetClientClass()
{
}

// game/client/portal2/c_npc_personality_core.cpp:26 @0x51e270 _ZN22C_NPC_Personality_Core14UpdateOnRemoveEv
void C_NPC_Personality_Core::UpdateOnRemove()
{
	// inlined CUtlReference<CNewParticleEffect>::operator CNewParticleEffect*() at line 28
	// inlined CUtlReference<CNewParticleEffect>::operator=() at line 31
}

// game/client/portal2/c_npc_personality_core.cpp:46 @0x51e0a0 _ZN22C_NPC_Personality_Core12ControlMouthEP10CStudioHdr
void C_NPC_Personality_Core::ControlMouth( CStudioHdr *pStudioHdr )
{
	float flOpenAmount;  // line 56
	float flDarken;  // line 60
	// inlined ExponentialDecay() at line 68
}

// game/client/portal2/c_npc_personality_core.cpp:77 @0x51e350 _ZN22C_NPC_Personality_Core13OnDataChangedE16DataUpdateType_t
void C_NPC_Personality_Core::OnDataChanged( DataUpdateType_t updateType )
{
	// inlined CMouthInfo::ActivateEnvelope() at line 85
}

// game/client/portal2/c_npc_personality_core.cpp:95 @0x51e3c0 _ZN22C_NPC_Personality_Core11ClientThinkEv
void C_NPC_Personality_Core::ClientThink()
{
	// inlined CUtlReference<CNewParticleEffect>::operator=() at line 161
	// inlined CUtlReference<CNewParticleEffect>::operator CNewParticleEffect*() at line 158
	{
		Vector vecOrigin;  // line 100
		QAngle angles;  // line 101
		int iAttachment;  // line 102
		Vector vForward;  // line 106
		Vector vRight;  // line 107
		Vector vUp;  // line 108
		trace_t tr;  // line 111
		// inlined Vector::operator=() at line 151
		// inlined Vector::operator+() at line 151
		// inlined Vector::operator*() at line 151
		// inlined CUtlReference<CNewParticleEffect>::operator CNewParticleEffect*() at line 116
		// inlined UTIL_TraceLine() at line 112
		// inlined Vector::operator+() at line 112
		// inlined Vector::operator*() at line 112
		// inlined CUtlReference<CNewParticleEffect>::operator=() at line 118
		// inlined CUtlReference<CNewParticleEffect>::operator CNewParticleEffect*() at line 124
	}
}

// game/client/portal2/c_npc_personality_core.cpp:187 @0x51df20 _ZL36_C_NPC_Personality_Core_CreateObjectii
IClientNetworkable *_C_NPC_Personality_Core_CreateObject( int entnum, int serialNum )
{
	C_NPC_Personality_Core *pRet;  // line 187
	// inlined C_NPC_Personality_Core::C_NPC_Personality_Core() at line 187
}

// game/client/portal2/c_npc_personality_core.cpp:187 @0x74c30 _Z15ClientClassInitIN23DT_NPC_Personality_Core7ignoredEEiPT_
int ClientClassInit<DT_NPC_Personality_Core::ignored>( DT_NPC_Personality_Core::ignored * )
{
	char *pRecvTableName;  // line 187
	RecvTable &RecvTable;  // line 187
	RecvProp RecvProps[3];  // line 187
}

// game/client/portal2/c_npc_personality_core.cpp:187
RecvTable g_RecvTable;

// game/client/portal2/c_npc_personality_core.cpp:187
int g_RecvTableInit;

// game/client/portal2/c_npc_personality_core.cpp:187
ClientClass __g_C_NPC_Personality_CoreClientClass;

// game/client/portal2/c_npc_personality_core.cpp:193 sizeof=0xc (i386)
struct CLightedMouthProxy : public CResultProxy
{
public:
	virtual bool Init( IMaterial *, KeyValues * );  // line 195
	virtual void OnBind( void * );  // line 196
};

// game/client/portal2/c_npc_personality_core.cpp:193 (declaration)
~CLightedMouthProxy();

// game/client/portal2/c_npc_personality_core.cpp:193 @0x51e210 _ZN18CLightedMouthProxyD0Ev
CLightedMouthProxy::~CLightedMouthProxy()
{
}

// game/client/portal2/c_npc_personality_core.cpp:193 @0x51e250 _ZN18CLightedMouthProxyD1Ev
CLightedMouthProxy::~CLightedMouthProxy()
{
}

// game/client/portal2/c_npc_personality_core.cpp:193 (declaration)
void CLightedMouthProxy();

// game/client/portal2/c_npc_personality_core.cpp:195 @0x51e070 _ZN18CLightedMouthProxy4InitEP9IMaterialP9KeyValues
bool CLightedMouthProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
}

// game/client/portal2/c_npc_personality_core.cpp:196 @0x51dfe0 _ZN18CLightedMouthProxy6OnBindEPv
void CLightedMouthProxy::OnBind( void *pC_BaseEntity )
{
	C_BaseEntity *pEntity;  // line 216
	C_NPC_Personality_Core *core;  // line 218
	{
		float amt;  // line 221
	}
}

// game/client/portal2/c_npc_personality_core.cpp:231 sizeof=0x1 (i386)
struct CLightedMouthFoo
{
public:
	CLightedMouthFoo();  // line 231
};

// game/client/portal2/c_npc_personality_core.cpp:231 @0x51dec0 _ZL26CCLightedMouthProxyFactoryv
IMaterialProxy *CCLightedMouthProxyFactory()
{
	// inlined CLightedMouthProxy::CLightedMouthProxy() at line 231
}
