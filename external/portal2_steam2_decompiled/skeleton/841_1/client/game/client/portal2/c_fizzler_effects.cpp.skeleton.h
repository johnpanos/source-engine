// DWARF declaration skeleton for game/client/portal2/c_fizzler_effects.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal2/c_fizzler_effects.cpp:0 @0x51b1f0 _ZN16C_FizzlerEffects40YouForgotToImplementOrDeclareClientClassEv
int C_FizzlerEffects::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/portal2/c_fizzler_effects.cpp:0 @0x51b210 _ZN16C_FizzlerEffects14GetClientClassEv
ClientClass *C_FizzlerEffects::GetClientClass()
{
}

// None:0 @0x73660 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined Cenv_fizzler_effectsFoo::Cenv_fizzler_effectsFoo() at line 14
	// inlined ClientClass::ClientClass() at line 16
}

// game/client/portal2/c_fizzler_effects.cpp:0
void C_FizzlerEffects::m_pClassRecvTable;

// game/client/portal2/c_fizzler_effects.cpp:14 sizeof=0x1 (i386)
struct Cenv_fizzler_effectsFoo
{
public:
	Cenv_fizzler_effectsFoo();  // line 14
};

// game/client/portal2/c_fizzler_effects.cpp:14 @0x51b220 _ZL24CC_FizzlerEffectsFactoryv
C_BaseEntity *CC_FizzlerEffectsFactory()
{
	// inlined C_FizzlerEffects::C_FizzlerEffects() at line 14
}

// game/client/portal2/c_fizzler_effects.cpp:16 @0x73540 _Z15ClientClassInitIN17DT_FizzlerEffects7ignoredEEiPT_
int ClientClassInit<DT_FizzlerEffects::ignored>( DT_FizzlerEffects::ignored * )
{
	char *pRecvTableName;  // line 16
	RecvTable &RecvTable;  // line 16
	RecvProp RecvProps[2];  // line 16
}

// game/client/portal2/c_fizzler_effects.cpp:16
RecvTable g_RecvTable;

// game/client/portal2/c_fizzler_effects.cpp:16
int g_RecvTableInit;

// game/client/portal2/c_fizzler_effects.cpp:16 @0x51b2b0 _ZL30_C_FizzlerEffects_CreateObjectii
IClientNetworkable *_C_FizzlerEffects_CreateObject( int entnum, int serialNum )
{
	C_FizzlerEffects *pRet;  // line 16
	// inlined C_FizzlerEffects::C_FizzlerEffects() at line 16
}

// game/client/portal2/c_fizzler_effects.cpp:16
ClientClass __g_C_FizzlerEffectsClientClass;
