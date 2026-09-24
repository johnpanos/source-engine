// DWARF declaration skeleton for game/client/portal2/c_portal_playerlocaldata.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x77890 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined PredMapInit<C_PortalPlayerLocalData>() at line 67
	// inlined Vector::Vector() at line 107
}

// game/client/portal2/c_portal_playerlocaldata.cpp:15 @0x76dd0 _Z15ClientClassInitIN14DT_PortalLocal7ignoredEEiPT_
int ClientClassInit<DT_PortalLocal::ignored>( DT_PortalLocal::ignored * )
{
	char *pRecvTableName;  // line 15
	RecvTable &RecvTable;  // line 15
	RecvProp RecvProps[36];  // line 15
}

// game/client/portal2/c_portal_playerlocaldata.cpp:15
RecvTable g_RecvTable;

// game/client/portal2/c_portal_playerlocaldata.cpp:15
int g_RecvTableInit;

// game/client/portal2/c_portal_playerlocaldata.cpp:67 @0x52d290 _ZN23C_PortalPlayerLocalData14GetPredDescMapEv
datamap_t *C_PortalPlayerLocalData::GetPredDescMap()
{
}

// game/client/portal2/c_portal_playerlocaldata.cpp:67 (declaration)
datamap_t *PredMapInit<C_PortalPlayerLocalData>( C_PortalPlayerLocalData * );

// game/client/portal2/c_portal_playerlocaldata.cpp:67 @0x52d2a0 _Z11PredMapInitI23C_PortalPlayerLocalDataEP9datamap_tPT_
datamap_t *PredMapInit<C_PortalPlayerLocalData>( C_PortalPlayerLocalData * )
{
}

// game/client/portal2/c_portal_playerlocaldata.cpp:67
datamap_t *g_PredMapHolder;

// game/client/portal2/c_portal_playerlocaldata.cpp:67
void C_PortalPlayerLocalData::m_PredMap;

// game/client/portal2/c_portal_playerlocaldata.cpp:107
static const Vector DEFAULT_SURFACE_NORMAL;

// game/client/portal2/c_portal_playerlocaldata.cpp:109 (declaration)
void C_PortalPlayerLocalData();

// game/client/portal2/c_portal_playerlocaldata.cpp:109 @0x52d2d0 _ZN23C_PortalPlayerLocalDataC2Ev
C_PortalPlayerLocalData::C_PortalPlayerLocalData()
{
	// inlined Vector::Vector() at line 138
	// inlined Vector::Vector() at line 138
	// inlined Vector::Vector() at line 138
	// inlined Vector::Vector() at line 138
	// inlined QAngle::QAngle() at line 138
	// inlined CountdownTimer::CountdownTimer() at line 138
	{
		unsigned int i;  // line 153
		// inlined Vector::operator=() at line 155
		// inlined Vector::operator=() at line 156
	}
}

// game/client/portal2/c_portal_playerlocaldata.cpp:109 @0x52d770 _ZN23C_PortalPlayerLocalDataC1Ev
C_PortalPlayerLocalData::C_PortalPlayerLocalData()
{
}
