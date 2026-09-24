// DWARF declaration skeleton for game/client/portal2/c_level_placard_display.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x74590 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined ClientClass::ClientClass() at line 26
}

// game/client/portal2/c_level_placard_display.cpp:12 sizeof=0xa60 (i386)
struct C_LevelPlacardDisplay : public C_BaseEntity
{
public:
	C_LevelPlacardDisplay();  // line 17
	virtual ~C_LevelPlacardDisplay();  // line 18
	bool IsEnabled() const;  // line 20
private:
	bool m_bEnabled; // +0xa5d  // line 23
};

// game/client/portal2/c_level_placard_display.cpp:15 @0x51cd20 _ZN21C_LevelPlacardDisplay40YouForgotToImplementOrDeclareClientClassEv
int C_LevelPlacardDisplay::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/portal2/c_level_placard_display.cpp:15 @0x51cd40 _ZN21C_LevelPlacardDisplay14GetClientClassEv
ClientClass *C_LevelPlacardDisplay::GetClientClass()
{
}

// game/client/portal2/c_level_placard_display.cpp:17 @0x51ce80 _ZN21C_LevelPlacardDisplayC1Ev
C_LevelPlacardDisplay::C_LevelPlacardDisplay()
{
}

// game/client/portal2/c_level_placard_display.cpp:17 @0x51cef0 _ZN21C_LevelPlacardDisplayC2Ev
C_LevelPlacardDisplay::C_LevelPlacardDisplay()
{
}

// game/client/portal2/c_level_placard_display.cpp:18 @0x51cd50 _ZN21C_LevelPlacardDisplayD0Ev
C_LevelPlacardDisplay::~C_LevelPlacardDisplay()
{
}

// game/client/portal2/c_level_placard_display.cpp:18 @0x51cdc0 _ZN21C_LevelPlacardDisplayD1Ev
C_LevelPlacardDisplay::~C_LevelPlacardDisplay()
{
}

// game/client/portal2/c_level_placard_display.cpp:18 @0x51ce20 _ZN21C_LevelPlacardDisplayD2Ev
C_LevelPlacardDisplay::~C_LevelPlacardDisplay()
{
}

// game/client/portal2/c_level_placard_display.cpp:26 @0x74440 _Z15ClientClassInitIN22DT_LevelPlacardDisplay7ignoredEEiPT_
int ClientClassInit<DT_LevelPlacardDisplay::ignored>( DT_LevelPlacardDisplay::ignored * )
{
	char *pRecvTableName;  // line 26
	RecvTable &RecvTable;  // line 26
	RecvProp RecvProps[3];  // line 26
}

// game/client/portal2/c_level_placard_display.cpp:26
RecvTable g_RecvTable;

// game/client/portal2/c_level_placard_display.cpp:26
int g_RecvTableInit;

// game/client/portal2/c_level_placard_display.cpp:26 @0x51cf60 _ZL35_C_LevelPlacardDisplay_CreateObjectii
IClientNetworkable *_C_LevelPlacardDisplay_CreateObject( int entnum, int serialNum )
{
	C_LevelPlacardDisplay *pRet;  // line 26
	// inlined C_LevelPlacardDisplay::C_LevelPlacardDisplay() at line 26
}

// game/client/portal2/c_level_placard_display.cpp:26
ClientClass __g_C_LevelPlacardDisplayClientClass;
