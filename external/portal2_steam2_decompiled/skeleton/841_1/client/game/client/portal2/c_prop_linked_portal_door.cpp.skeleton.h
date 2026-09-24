// DWARF declaration skeleton for game/client/portal2/c_prop_linked_portal_door.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x78bf0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined ClientClass::ClientClass() at line 29
}

// game/client/portal2/c_prop_linked_portal_door.cpp:14 sizeof=0xd88 (i386)
struct C_PropLinkedPortalDoor : public C_BaseAnimating, public CSignifierTarget
{
public:
	virtual bool OverrideSignifierPosition();  // line 19
	virtual bool GetSignifierPosition( const Vector &, Vector &, Vector & );  // line 20
};

// game/client/portal2/c_prop_linked_portal_door.cpp:14 (declaration)
~C_PropLinkedPortalDoor();

// game/client/portal2/c_prop_linked_portal_door.cpp:14 @0x5389a0 _ZN22C_PropLinkedPortalDoorD0Ev
C_PropLinkedPortalDoor::~C_PropLinkedPortalDoor()
{
}

// game/client/portal2/c_prop_linked_portal_door.cpp:14 @0x538a20 _ZN22C_PropLinkedPortalDoorD1Ev
C_PropLinkedPortalDoor::~C_PropLinkedPortalDoor()
{
}

// game/client/portal2/c_prop_linked_portal_door.cpp:14 (declaration)
void C_PropLinkedPortalDoor();

// game/client/portal2/c_prop_linked_portal_door.cpp:17 @0x538890 _ZN22C_PropLinkedPortalDoor40YouForgotToImplementOrDeclareClientClassEv
int C_PropLinkedPortalDoor::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/portal2/c_prop_linked_portal_door.cpp:17 @0x5388b0 _ZN22C_PropLinkedPortalDoor14GetClientClassEv
ClientClass *C_PropLinkedPortalDoor::GetClientClass()
{
}

// game/client/portal2/c_prop_linked_portal_door.cpp:19 @0x538990 _ZN22C_PropLinkedPortalDoor25OverrideSignifierPositionEv
bool C_PropLinkedPortalDoor::OverrideSignifierPosition()
{
}

// game/client/portal2/c_prop_linked_portal_door.cpp:20 @0x538aa0 _ZN22C_PropLinkedPortalDoor20GetSignifierPositionERK6VectorRS0_S3_
bool C_PropLinkedPortalDoor::GetSignifierPosition( const Vector &vSource, Vector &vPositionOut, Vector &vNormalOut )
{
	// inlined Vector::operator=() at line 23
}

// game/client/portal2/c_prop_linked_portal_door.cpp:29 @0x5388c0 _ZL36_C_PropLinkedPortalDoor_CreateObjectii
IClientNetworkable *_C_PropLinkedPortalDoor_CreateObject( int entnum, int serialNum )
{
	C_PropLinkedPortalDoor *pRet;  // line 29
	// inlined C_PropLinkedPortalDoor::C_PropLinkedPortalDoor() at line 29
}

// game/client/portal2/c_prop_linked_portal_door.cpp:29 @0x78ad0 _Z15ClientClassInitIN23DT_PropLinkedPortalDoor7ignoredEEiPT_
int ClientClassInit<DT_PropLinkedPortalDoor::ignored>( DT_PropLinkedPortalDoor::ignored * )
{
	char *pRecvTableName;  // line 29
	RecvTable &RecvTable;  // line 29
	RecvProp RecvProps[2];  // line 29
}

// game/client/portal2/c_prop_linked_portal_door.cpp:29
RecvTable g_RecvTable;

// game/client/portal2/c_prop_linked_portal_door.cpp:29
int g_RecvTableInit;

// game/client/portal2/c_prop_linked_portal_door.cpp:29
ClientClass __g_C_PropLinkedPortalDoorClientClass;
