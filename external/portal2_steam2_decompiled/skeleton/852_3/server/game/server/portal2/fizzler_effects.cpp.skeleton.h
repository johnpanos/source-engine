// DWARF declaration skeleton for game/server/portal2/fizzler_effects.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xbe3f0 _Z41__static_initialization_and_destruction_0ii
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
	// inlined CEntityFactory<FizzlerEffects>::CEntityFactory() at line 16
	// inlined ServerClass::ServerClass() at line 22
}

// game/server/portal2/fizzler_effects.cpp:13
ConVar fizzler_fx_model_target_scale;

// game/server/portal2/fizzler_effects.cpp:16
static CEntityFactory<FizzlerEffects> env_fizzler_effects;

// game/server/portal2/fizzler_effects.cpp:18 (declaration)
void GetBaseMap();

// game/server/portal2/fizzler_effects.cpp:18 @0x6b6250 _ZN14FizzlerEffects14GetDataDescMapEv
datamap_t *FizzlerEffects::GetDataDescMap()
{
}

// game/server/portal2/fizzler_effects.cpp:18 @0x6b6260 _ZN14FizzlerEffects10GetBaseMapEv
datamap_t *FizzlerEffects::GetBaseMap()
{
}

// game/server/portal2/fizzler_effects.cpp:18 @0xbe1e0 _Z11DataMapInitI14FizzlerEffectsEP9datamap_tPT_
datamap_t *DataMapInit<FizzlerEffects>( FizzlerEffects * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 20
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 18
	CDatadescGeneratedNameHolder nameHolder;  // line 18
	typedescription_t dataDesc[2];  // line 18
}

// game/server/portal2/fizzler_effects.cpp:18
datamap_t *g_DataMapHolder;

// game/server/portal2/fizzler_effects.cpp:18
void FizzlerEffects::m_DataMap;

// game/server/portal2/fizzler_effects.cpp:22 @0x6b6270 _ZN14FizzlerEffects14GetServerClassEv
ServerClass *FizzlerEffects::GetServerClass()
{
}

// game/server/portal2/fizzler_effects.cpp:22 @0x6b6280 _ZN14FizzlerEffects40YouForgotToImplementOrDeclareServerClassEv
int FizzlerEffects::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/fizzler_effects.cpp:22 @0xbe050 _Z15ServerClassInitIN17DT_FizzlerEffects7ignoredEEiPT_
int ServerClassInit<DT_FizzlerEffects::ignored>( DT_FizzlerEffects::ignored * )
{
	SendTable &sendTable;  // line 22
	char *const g_pSendTableName;  // line 22
	SendProp g_SendProps[2];  // line 22
}

// game/server/portal2/fizzler_effects.cpp:22
SendTable g_SendTable;

// game/server/portal2/fizzler_effects.cpp:22
int g_SendTableInit;

// game/server/portal2/fizzler_effects.cpp:22
static ServerClass g_FizzlerEffects_ClassReg;

// game/server/portal2/fizzler_effects.cpp:22
void FizzlerEffects::m_pClassSendTable;

// game/server/portal2/fizzler_effects.cpp:25 @0x6b6440 _ZN14FizzlerEffects6CreateEP11CBaseEntity
FizzlerEffects *FizzlerEffects::Create( CBaseEntity *pAttachTo )
{
	FizzlerEffects *pFizzlerEffects;  // line 27
	{
		CBaseAnimating *pDissolvingAnimating;  // line 38
		// inlined CBaseAnimating::SequenceDuration() at line 48
		// inlined FizzlerEffects::AttachToEntity() at line 32
	}
}

// game/server/portal2/fizzler_effects.cpp:55 @0x6b62f0 _ZN14FizzlerEffects5SpawnEv
void FizzlerEffects::Spawn()
{
}

// game/server/portal2/fizzler_effects.cpp:63 (declaration)
void AttachToEntity( CBaseEntity *pAttachTo );

// game/server/portal2/fizzler_effects.cpp:63 @0x6b6290 _ZN14FizzlerEffects14AttachToEntityEP11CBaseEntity
void FizzlerEffects::AttachToEntity( CBaseEntity *pAttachTo )
{
}

// game/server/portal2/fizzler_effects.cpp:71 @0x6b6370 _ZN14FizzlerEffects11FizzleThinkEv
void FizzlerEffects::FizzleThink()
{
	CBaseAnimating *pDissolving;  // line 73
	// inlined CBaseEntity::GetMoveParent() at line 73
}

// game/server/util.h:99 @0x6b6660 _ZN14CEntityFactoryI14FizzlerEffectsE6CreateEPKc
IServerNetworkable *CEntityFactory<FizzlerEffects>::Create( const char *pClassName )
{
	FizzlerEffects *pEnt;  // line 101
	// inlined _CreateEntityTemplate<FizzlerEffects>() at line 101
}

// game/server/util.h:105 @0x6b65a0 _ZN14CEntityFactoryI14FizzlerEffectsE7DestroyEP18IServerNetworkable
void CEntityFactory<FizzlerEffects>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:113 @0x6b65d0 _ZN14CEntityFactoryI14FizzlerEffectsE13GetEntitySizeEv
size_t CEntityFactory<FizzlerEffects>::GetEntitySize()
{
}
