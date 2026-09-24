// DWARF declaration skeleton for game/server/portal2/fizzler_effects.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xa9250 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CEntityFactory<FizzlerEffects>::CEntityFactory() at line 18
	// inlined ServerClass::ServerClass() at line 24
}

// game/server/portal2/fizzler_effects.cpp:13
ConVar fizzler_fx_model_end_scale;

// game/server/portal2/fizzler_effects.cpp:14
ConVar fizzler_fx_model_scale_end_time_offset;

// game/server/portal2/fizzler_effects.cpp:15
ConVar fizzler_fx_model_scale_start_delay_time;

// game/server/portal2/fizzler_effects.cpp:18
static CEntityFactory<FizzlerEffects> env_fizzler_effects;

// game/server/portal2/fizzler_effects.cpp:20 (declaration)
void GetBaseMap();

// game/server/portal2/fizzler_effects.cpp:20 @0x712150 _ZN14FizzlerEffects14GetDataDescMapEv
datamap_t *FizzlerEffects::GetDataDescMap()
{
}

// game/server/portal2/fizzler_effects.cpp:20 @0x712160 _ZN14FizzlerEffects10GetBaseMapEv
datamap_t *FizzlerEffects::GetBaseMap()
{
}

// game/server/portal2/fizzler_effects.cpp:20 @0xa9040 _Z11DataMapInitI14FizzlerEffectsEP9datamap_tPT_
datamap_t *DataMapInit<FizzlerEffects>( FizzlerEffects * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 22
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 20
	CDatadescGeneratedNameHolder nameHolder;  // line 20
	typedescription_t dataDesc[2];  // line 20
}

// game/server/portal2/fizzler_effects.cpp:20
datamap_t *g_DataMapHolder;

// game/server/portal2/fizzler_effects.cpp:20
void FizzlerEffects::m_DataMap;

// game/server/portal2/fizzler_effects.cpp:24 @0x712170 _ZN14FizzlerEffects14GetServerClassEv
ServerClass *FizzlerEffects::GetServerClass()
{
}

// game/server/portal2/fizzler_effects.cpp:24 @0x712180 _ZN14FizzlerEffects40YouForgotToImplementOrDeclareServerClassEv
int FizzlerEffects::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/fizzler_effects.cpp:24 @0xa8eb0 _Z15ServerClassInitIN17DT_FizzlerEffects7ignoredEEiPT_
int ServerClassInit<DT_FizzlerEffects::ignored>( DT_FizzlerEffects::ignored * )
{
	SendTable &sendTable;  // line 24
	char *const g_pSendTableName;  // line 24
	SendProp g_SendProps[2];  // line 24
}

// game/server/portal2/fizzler_effects.cpp:24
SendTable g_SendTable;

// game/server/portal2/fizzler_effects.cpp:24
int g_SendTableInit;

// game/server/portal2/fizzler_effects.cpp:24
static ServerClass g_FizzlerEffects_ClassReg;

// game/server/portal2/fizzler_effects.cpp:24
void FizzlerEffects::m_pClassSendTable;

// game/server/portal2/fizzler_effects.cpp:27 (declaration)
void FizzlerEffects();

// game/server/portal2/fizzler_effects.cpp:27 @0x712270 _ZN14FizzlerEffectsC1Ev
FizzlerEffects::FizzlerEffects()
{
}

// game/server/portal2/fizzler_effects.cpp:27 @0x7122c0 _ZN14FizzlerEffectsC2Ev
FizzlerEffects::FizzlerEffects()
{
}

// game/server/portal2/fizzler_effects.cpp:33 @0x7124f0 _ZN14FizzlerEffects6CreateEP11CBaseEntity
FizzlerEffects *FizzlerEffects::Create( CBaseEntity *pAttachTo )
{
	FizzlerEffects *pFizzlerEffects;  // line 35
	{
		CBaseAnimating *pDissolvingAnimating;  // line 46
		// inlined FizzlerEffects::AttachToEntity() at line 40
	}
}

// game/server/portal2/fizzler_effects.cpp:60 @0x7121f0 _ZN14FizzlerEffects5SpawnEv
void FizzlerEffects::Spawn()
{
}

// game/server/portal2/fizzler_effects.cpp:68 (declaration)
void AttachToEntity( CBaseEntity *pAttachTo );

// game/server/portal2/fizzler_effects.cpp:68 @0x712190 _ZN14FizzlerEffects14AttachToEntityEP11CBaseEntity
void FizzlerEffects::AttachToEntity( CBaseEntity *pAttachTo )
{
}

// game/server/portal2/fizzler_effects.cpp:76 @0x712310 _ZN14FizzlerEffects11FizzleThinkEv
void FizzlerEffects::FizzleThink()
{
	CBaseAnimating *pDissolving;  // line 78
	// inlined CBaseEntity::GetMoveParent() at line 78
	{
		const float scaleDelayTime;  // line 87
		// inlined CBaseAnimating::SequenceDuration() at line 91
		// inlined ConVar::GetFloat() at line 87
	}
}

// game/server/util.h:100 @0x7126a0 _ZN14CEntityFactoryI14FizzlerEffectsE6CreateEPKc
IServerNetworkable *CEntityFactory<FizzlerEffects>::Create( const char *pClassName )
{
	FizzlerEffects *pEnt;  // line 102
	// inlined _CreateEntityTemplate<FizzlerEffects>() at line 102
}

// game/server/util.h:106 @0x7125e0 _ZN14CEntityFactoryI14FizzlerEffectsE7DestroyEP18IServerNetworkable
void CEntityFactory<FizzlerEffects>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:114 @0x712610 _ZN14CEntityFactoryI14FizzlerEffectsE13GetEntitySizeEv
size_t CEntityFactory<FizzlerEffects>::GetEntitySize()
{
}
