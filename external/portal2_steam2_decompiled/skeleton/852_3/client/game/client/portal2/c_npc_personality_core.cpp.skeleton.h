// DWARF declaration skeleton for game/client/portal2/c_npc_personality_core.cpp
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x74eb0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
	// inlined ClientClass::ClientClass() at line 184
	// inlined CLightedMouthFoo::CLightedMouthFoo() at line 227
}

// game/client/portal2/c_npc_personality_core.cpp:18 sizeof=0x12e8 (i386)
struct C_NPC_Personality_Core : public C_AI_BaseNPC
{
public:
	virtual void UpdateOnRemove();  // line 26
	virtual void ControlMouth( CStudioHdr * );  // line 46
	virtual void OnDataChanged( DataUpdateType_t );  // line 75
	virtual void ClientThink();  // line 93
	float GetMouthAmount() const;  // line 173
private:
	float m_flMouthAmount; // +0x12cc  // line 176
	float m_flMouthHoldTime; // +0x12d0  // line 177
	bool m_bFlashlightEnabled; // +0x12d4  // line 178
	CUtlReference<CNewParticleEffect> m_pFlashlightEffect; // +0x12d8  // line 180
	dlight_t *m_pELight; // +0x12e4  // line 181
};

// game/client/portal2/c_npc_personality_core.cpp:18 (declaration)
void C_NPC_Personality_Core();

// game/client/portal2/c_npc_personality_core.cpp:18 (declaration)
~C_NPC_Personality_Core();

// game/client/portal2/c_npc_personality_core.cpp:18 @0x5240c0 _ZN22C_NPC_Personality_CoreD0Ev
C_NPC_Personality_Core::~C_NPC_Personality_Core()
{
	// inlined CUtlReference<CNewParticleEffect>::~CUtlReference() at line 18
	// inlined C_AI_BaseNPC::~C_AI_BaseNPC() at line 18
}

// game/client/portal2/c_npc_personality_core.cpp:18 @0x524200 _ZN22C_NPC_Personality_CoreD1Ev
C_NPC_Personality_Core::~C_NPC_Personality_Core()
{
	// inlined CUtlReference<CNewParticleEffect>::~CUtlReference() at line 18
	// inlined C_AI_BaseNPC::~C_AI_BaseNPC() at line 18
}

// game/client/portal2/c_npc_personality_core.cpp:21 @0x523450 _ZN22C_NPC_Personality_Core40YouForgotToImplementOrDeclareClientClassEv
int C_NPC_Personality_Core::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/portal2/c_npc_personality_core.cpp:21 @0x523470 _ZN22C_NPC_Personality_Core14GetClientClassEv
ClientClass *C_NPC_Personality_Core::GetClientClass()
{
}

// game/client/portal2/c_npc_personality_core.cpp:26 @0x523810 _ZN22C_NPC_Personality_Core14UpdateOnRemoveEv
void C_NPC_Personality_Core::UpdateOnRemove()
{
	// inlined CUtlReference<CNewParticleEffect>::operator CNewParticleEffect*() at line 28
	// inlined CUtlReference<CNewParticleEffect>::operator=() at line 31
}

// game/client/portal2/c_npc_personality_core.cpp:46 @0x523680 _ZN22C_NPC_Personality_Core12ControlMouthEP10CStudioHdr
void C_NPC_Personality_Core::ControlMouth( CStudioHdr *pStudioHdr )
{
	float flOpenAmount;  // line 56
	float flStepTime;  // line 57
	// inlined Approach() at line 69
}

// game/client/portal2/c_npc_personality_core.cpp:75 @0x5238f0 _ZN22C_NPC_Personality_Core13OnDataChangedE16DataUpdateType_t
void C_NPC_Personality_Core::OnDataChanged( DataUpdateType_t updateType )
{
	// inlined CMouthInfo::ActivateEnvelope() at line 83
}

// game/client/portal2/c_npc_personality_core.cpp:93 @0x523960 _ZN22C_NPC_Personality_Core11ClientThinkEv
void C_NPC_Personality_Core::ClientThink()
{
	// inlined CUtlReference<CNewParticleEffect>::operator=() at line 159
	// inlined CUtlReference<CNewParticleEffect>::operator CNewParticleEffect*() at line 156
	{
		Vector vecOrigin;  // line 98
		QAngle angles;  // line 99
		int iAttachment;  // line 100
		Vector vForward;  // line 104
		Vector vRight;  // line 105
		Vector vUp;  // line 106
		trace_t tr;  // line 109
		// inlined Vector::operator=() at line 149
		// inlined Vector::operator+() at line 149
		// inlined Vector::operator*() at line 149
		// inlined CUtlReference<CNewParticleEffect>::operator CNewParticleEffect*() at line 114
		// inlined UTIL_TraceLine() at line 110
		// inlined Vector::operator+() at line 110
		// inlined Vector::operator*() at line 110
		// inlined CUtlReference<CNewParticleEffect>::operator=() at line 116
		// inlined CUtlReference<CNewParticleEffect>::operator CNewParticleEffect*() at line 122
	}
}

// game/client/portal2/c_npc_personality_core.cpp:184 @0x5234e0 _ZL36_C_NPC_Personality_Core_CreateObjectii
IClientNetworkable *_C_NPC_Personality_Core_CreateObject( int entnum, int serialNum )
{
	C_NPC_Personality_Core *pRet;  // line 184
	// inlined C_NPC_Personality_Core::C_NPC_Personality_Core() at line 184
}

// game/client/portal2/c_npc_personality_core.cpp:184 @0x74d60 _Z15ClientClassInitIN23DT_NPC_Personality_Core7ignoredEEiPT_
int ClientClassInit<DT_NPC_Personality_Core::ignored>( DT_NPC_Personality_Core::ignored * )
{
	char *pRecvTableName;  // line 184
	RecvTable &RecvTable;  // line 184
	RecvProp RecvProps[3];  // line 184
}

// game/client/portal2/c_npc_personality_core.cpp:184
RecvTable g_RecvTable;

// game/client/portal2/c_npc_personality_core.cpp:184
int g_RecvTableInit;

// game/client/portal2/c_npc_personality_core.cpp:184
ClientClass __g_C_NPC_Personality_CoreClientClass;

// game/client/portal2/c_npc_personality_core.cpp:190 sizeof=0xc (i386)
struct CLightedMouthProxy : public CResultProxy
{
public:
	virtual bool Init( IMaterial *, KeyValues * );  // line 192
	virtual void OnBind( void * );  // line 193
};

// game/client/portal2/c_npc_personality_core.cpp:190 (declaration)
~CLightedMouthProxy();

// game/client/portal2/c_npc_personality_core.cpp:190 @0x5237b0 _ZN18CLightedMouthProxyD0Ev
CLightedMouthProxy::~CLightedMouthProxy()
{
}

// game/client/portal2/c_npc_personality_core.cpp:190 @0x5237f0 _ZN18CLightedMouthProxyD1Ev
CLightedMouthProxy::~CLightedMouthProxy()
{
}

// game/client/portal2/c_npc_personality_core.cpp:190 (declaration)
void CLightedMouthProxy();

// game/client/portal2/c_npc_personality_core.cpp:192 @0x523650 _ZN18CLightedMouthProxy4InitEP9IMaterialP9KeyValues
bool CLightedMouthProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
}

// game/client/portal2/c_npc_personality_core.cpp:193 @0x5235b0 _ZN18CLightedMouthProxy6OnBindEPv
void CLightedMouthProxy::OnBind( void *pC_BaseEntity )
{
	C_BaseEntity *pEntity;  // line 213
	C_NPC_Personality_Core *core;  // line 215
}

// game/client/portal2/c_npc_personality_core.cpp:227 sizeof=0x1 (i386)
struct CLightedMouthFoo
{
public:
	CLightedMouthFoo();  // line 227
};

// game/client/portal2/c_npc_personality_core.cpp:227 @0x523480 _ZL26CCLightedMouthProxyFactoryv
IMaterialProxy *CCLightedMouthProxyFactory()
{
	// inlined CLightedMouthProxy::CLightedMouthProxy() at line 227
}
