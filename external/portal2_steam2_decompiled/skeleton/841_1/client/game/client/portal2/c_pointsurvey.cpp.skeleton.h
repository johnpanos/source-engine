// DWARF declaration skeleton for game/client/portal2/c_pointsurvey.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x76c50 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined ClientClass::ClientClass() at line 40
	// inlined C_PointSurveyHelper::C_PointSurveyHelper() at line 96
}

// game/client/portal2/c_pointsurvey.cpp:18 sizeof=0xb64 (i386)
struct C_PointSurvey : public C_BaseEntity
{
public:
	C_PointSurvey();  // line 23
	virtual void PostDataUpdate( DataUpdateType_t );  // line 25
	const char *GetSurveyName();  // line 33
protected:
	char m_iszSurveyName[260]; // +0xa5d  // line 36
};

// game/client/portal2/c_pointsurvey.cpp:18 (declaration)
~C_PointSurvey();

// game/client/portal2/c_pointsurvey.cpp:18 @0x52d050 _ZN13C_PointSurveyD0Ev
C_PointSurvey::~C_PointSurvey()
{
}

// game/client/portal2/c_pointsurvey.cpp:18 @0x52d0c0 _ZN13C_PointSurveyD1Ev
C_PointSurvey::~C_PointSurvey()
{
}

// game/client/portal2/c_pointsurvey.cpp:21 @0x52cba0 _ZN13C_PointSurvey40YouForgotToImplementOrDeclareClientClassEv
int C_PointSurvey::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/portal2/c_pointsurvey.cpp:21 @0x52cbc0 _ZN13C_PointSurvey14GetClientClassEv
ClientClass *C_PointSurvey::GetClientClass()
{
}

// game/client/portal2/c_pointsurvey.cpp:23 @0x52cbd0 _ZN13C_PointSurveyC1Ev
C_PointSurvey::C_PointSurvey()
{
}

// game/client/portal2/c_pointsurvey.cpp:23 @0x52cc40 _ZN13C_PointSurveyC2Ev
C_PointSurvey::C_PointSurvey()
{
}

// game/client/portal2/c_pointsurvey.cpp:25 @0x52d030 _ZN13C_PointSurvey14PostDataUpdateE16DataUpdateType_t
void C_PointSurvey::PostDataUpdate( DataUpdateType_t updateType )
{
}

// game/client/portal2/c_pointsurvey.cpp:40 @0x76af0 _Z15ClientClassInitIN14DT_PointSurvey7ignoredEEiPT_
int ClientClassInit<DT_PointSurvey::ignored>( DT_PointSurvey::ignored * )
{
	char *pRecvTableName;  // line 40
	RecvTable &RecvTable;  // line 40
	RecvProp RecvProps[3];  // line 40
}

// game/client/portal2/c_pointsurvey.cpp:40
RecvTable g_RecvTable;

// game/client/portal2/c_pointsurvey.cpp:40
int g_RecvTableInit;

// game/client/portal2/c_pointsurvey.cpp:40 @0x52cf70 _ZL27_C_PointSurvey_CreateObjectii
IClientNetworkable *_C_PointSurvey_CreateObject( int entnum, int serialNum )
{
	C_PointSurvey *pRet;  // line 40
	// inlined C_PointSurvey::C_PointSurvey() at line 40
}

// game/client/portal2/c_pointsurvey.cpp:40
ClientClass __g_C_PointSurveyClientClass;

// game/client/portal2/c_pointsurvey.cpp:48 @0x52ccb0 _Z21__MsgFunc_StartSurveyR7bf_read
__MsgFunc_StartSurvey( bf_read &msg )
{
	C_PointSurvey *pPointSurvey;  // line 50
	// inlined CBitRead::ReadLong() at line 50
	{
		CBaseViewport *pViewport;  // line 66
		CSurveyPanel *pSurveyPanel;  // line 67
		// inlined CHandle<C_BaseEntity>::operator=() at line 73
		{
			CUtlString strCmd;  // line 59
			int nEntityIndex;  // line 60
			// inlined CUtlString::~CUtlString() at line 63
			// inlined CUtlString::~CUtlString() at line 63
		}
	}
}

// game/client/portal2/c_pointsurvey.cpp:84 sizeof=0xc (i386)
struct C_PointSurveyHelper : public CAutoGameSystem
{
private:
	virtual bool Init();  // line 85
};

// game/client/portal2/c_pointsurvey.cpp:84 (declaration)
void C_PointSurveyHelper();

// game/client/portal2/c_pointsurvey.cpp:84 (declaration)
~C_PointSurveyHelper();

// game/client/portal2/c_pointsurvey.cpp:84 @0x52d230 _ZN19C_PointSurveyHelperD0Ev
C_PointSurveyHelper::~C_PointSurveyHelper()
{
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 84
}

// game/client/portal2/c_pointsurvey.cpp:84 @0x52d270 _ZN19C_PointSurveyHelperD1Ev
C_PointSurveyHelper::~C_PointSurveyHelper()
{
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 84
}

// game/client/portal2/c_pointsurvey.cpp:85 @0x52d120 _ZN19C_PointSurveyHelper4InitEv
bool C_PointSurveyHelper::Init()
{
	{
		int i;  // line 87
		{
			CSetActiveSplitScreenPlayerGuard g_SSGuard;  // line 89
		}
	}
}

// game/client/portal2/c_pointsurvey.cpp:96
static C_PointSurveyHelper s_PointSurveyHelper;
