// DWARF declaration skeleton for game/client/portal2/c_perfmonitor.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x76880 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
}

// game/client/portal2/c_perfmonitor.cpp:31
static ConVar cl_perfmon_desired_framerate;

// game/client/portal2/c_perfmonitor.cpp:32
static ConVar cl_perfmon_enabled;

// game/client/portal2/c_perfmonitor.cpp:33
static ConVar cl_perfmon_interval;

// game/client/portal2/c_perfmonitor.cpp:34
static ConVar cl_perfmon_frames_until_dump;

// game/client/portal2/c_perfmonitor.cpp:35
static ConVar cl_perfmon_frames_to_average;

// game/client/portal2/c_perfmonitor.cpp:44 (declaration)
void SendPerfMsg( const char *pMsg );

// game/client/portal2/c_perfmonitor.cpp:44 @0x52bd80 _Z11SendPerfMsgPKc
SendPerfMsg( const char *pMsg )
{
}

// game/client/portal2/c_perfmonitor.cpp:85 sizeof=0x844 (i386)
struct C_PerfMonitor : public CAutoGameSystemPerFrame
{
public:
	C_PerfMonitor();  // line 87
	virtual bool Init();  // line 91
	virtual void Shutdown();  // line 92
	virtual void LevelInitPostEntity();  // line 95
	virtual void LevelShutdownPreEntity();  // line 96
	virtual void PreRender();  // line 99
	virtual void Update( float );  // line 102
	virtual void PostRender();  // line 105
	uint64 GetLocalTimeInSeconds();  // line 107
	void UpdateFramesToAverage();  // line 109
	void Reset();  // line 111
private:
	uint32 m_uFramesBelowDesired; // +0xc  // line 116
	float m_fPreviousFrameTime; // +0x10  // line 117
	float m_fPeakFrametime; // +0x14  // line 118
	float m_fMinFrametime; // +0x18  // line 119
	float m_fLevelStartTime; // +0x1c  // line 120
	uint32 m_uTotalFrames; // +0x20  // line 121
	float m_fLastLowTime; // +0x24  // line 122
	float m_fLastLowFramerate; // +0x28  // line 123
	char m_szLastPerfMsg[2048]; // +0x2c  // line 124
	CUtlVector<float,CUtlMemory<float, int> > m_FrametimeHistory; // +0x82c  // line 125
	uint32 m_uOldestFrame; // +0x840  // line 126
};

// game/client/portal2/c_perfmonitor.cpp:85 (declaration)
~C_PerfMonitor();

// game/client/portal2/c_perfmonitor.cpp:85 @0x52c950 _ZN13C_PerfMonitorD0Ev
C_PerfMonitor::~C_PerfMonitor()
{
	// inlined CUtlVector<float,CUtlMemory<float, int> >::~CUtlVector() at line 85
	// inlined CAutoGameSystemPerFrame::~CAutoGameSystemPerFrame() at line 85
	// inlined CAutoGameSystemPerFrame::~CAutoGameSystemPerFrame() at line 85
}

// game/client/portal2/c_perfmonitor.cpp:85 @0x52ca80 _ZN13C_PerfMonitorD1Ev
C_PerfMonitor::~C_PerfMonitor()
{
	// inlined CUtlVector<float,CUtlMemory<float, int> >::~CUtlVector() at line 85
	// inlined CAutoGameSystemPerFrame::~CAutoGameSystemPerFrame() at line 85
	// inlined CAutoGameSystemPerFrame::~CAutoGameSystemPerFrame() at line 85
}

// game/client/portal2/c_perfmonitor.cpp:87 @0x76800 _ZN13C_PerfMonitorC1Ev
C_PerfMonitor::C_PerfMonitor()
{
	// inlined CUtlVector<float,CUtlMemory<float, int> >::CUtlVector() at line 133
}

// game/client/portal2/c_perfmonitor.cpp:87 @0x52c7f0 _ZN13C_PerfMonitorC2Ev
C_PerfMonitor::C_PerfMonitor()
{
	// inlined CUtlVector<float,CUtlMemory<float, int> >::CUtlVector() at line 133
}

// game/client/portal2/c_perfmonitor.cpp:91 @0x52c7d0 _ZN13C_PerfMonitor4InitEv
bool C_PerfMonitor::Init()
{
}

// game/client/portal2/c_perfmonitor.cpp:92 @0x52bd30 _ZN13C_PerfMonitor8ShutdownEv
void C_PerfMonitor::Shutdown()
{
}

// game/client/portal2/c_perfmonitor.cpp:95 @0x52c7c0 _ZN13C_PerfMonitor19LevelInitPostEntityEv
void C_PerfMonitor::LevelInitPostEntity()
{
}

// game/client/portal2/c_perfmonitor.cpp:96 @0x52c490 _ZN13C_PerfMonitor22LevelShutdownPreEntityEv
void C_PerfMonitor::LevelShutdownPreEntity()
{
	{
		char strTemp[2048];  // line 179
		// inlined C_PerfMonitor::GetLocalTimeInSeconds() at line 179
		// inlined SendPerfMsg() at line 179
	}
}

// game/client/portal2/c_perfmonitor.cpp:99 @0x52bd40 _ZN13C_PerfMonitor9PreRenderEv
void C_PerfMonitor::PreRender()
{
}

// game/client/portal2/c_perfmonitor.cpp:102 @0x52be40 _ZN13C_PerfMonitor6UpdateEf
void C_PerfMonitor::Update( float frametime )
{
	float fCurrTime;  // line 194
	float fElapsedTime;  // line 195
	float fFramerate;  // line 200
	float fAvgFrameTime;  // line 223
	int iFramesToAvg;  // line 224
	float fSMAFramerate;  // line 230
	char szLowFrameMsg[2048];  // line 248
	int iCurrChar;  // line 249
	// inlined ConVar::GetInt() at line 224
	{
		int i;  // line 225
	}
	// inlined C_PerfMonitor::GetLocalTimeInSeconds() at line 256
	{
		int hh;  // line 259
		{
			C_BasePlayer *pl;  // line 261
			Vector vPlayerPos;  // line 268
			QAngle vPlayerAng;  // line 269
			C_Portal_Player *pPortalPlayer;  // line 270
			Vector vBluePortalPos;  // line 273
			QAngle vBluePortalAng;  // line 274
			Vector vOrangePortalPos;  // line 275
			QAngle vOrangePortalAng;  // line 276
			C_WeaponPortalgun *pPortalgun;  // line 278
			// inlined QAngle::QAngle() at line 276
			// inlined Vector::Vector() at line 275
			// inlined QAngle::QAngle() at line 274
			// inlined Vector::Vector() at line 273
			// inlined ToPortalPlayer() at line 270
		}
	}
	{
		char strTemp[2048];  // line 216
		// inlined SendPerfMsg() at line 216
	}
}

// game/client/portal2/c_perfmonitor.cpp:105 @0x52bd50 _ZN13C_PerfMonitor10PostRenderEv
void C_PerfMonitor::PostRender()
{
}

// game/client/portal2/c_perfmonitor.cpp:107 @0x52bd60 _ZN13C_PerfMonitor21GetLocalTimeInSecondsEv
uint64 C_PerfMonitor::GetLocalTimeInSeconds()
{
	time_t now;  // line 332
}

// game/client/portal2/c_perfmonitor.cpp:109 @0x52bda0 _ZN13C_PerfMonitor21UpdateFramesToAverageEv
void C_PerfMonitor::UpdateFramesToAverage()
{
	// inlined ConVar::GetInt() at line 339
	// inlined CUtlVector<float,CUtlMemory<float, int> >::SetCount() at line 339
}

// game/client/portal2/c_perfmonitor.cpp:111 @0x52c6c0 _ZN13C_PerfMonitor5ResetEv
void C_PerfMonitor::Reset()
{
	double dbCurrTime;  // line 145
	{
		int i;  // line 141
	}
	// inlined C_PerfMonitor::UpdateFramesToAverage() at line 140
}

// game/client/portal2/c_perfmonitor.cpp:342
static C_PerfMonitor *s_pPerfMonitor;

// game/client/portal2/c_perfmonitor.cpp:344 @0x52c610 _Z38cl_perfmon_frames_to_average_changed_fP7IConVarPKcf
cl_perfmon_frames_to_average_changed_f( IConVar *var, const char *pOldValue, float flOldValue )
{
	// inlined C_PerfMonitor::UpdateFramesToAverage() at line 346
}

// public/tier1/utlmemory.h:707 @0x52c870 _ZN10CUtlMemoryIfiE4GrowEi
void CUtlMemory<float,int>::Grow( int num )
{
	int nAllocationRequested;  // line 720
	int nNewAllocationCount;  // line 724
	// inlined UtlMemory_CalcNewAllocationCount() at line 724
	// inlined CUtlMemory<float,int>::IsExternallyAllocated() at line 711
	// inlined MemAlloc_Alloc() at line 761
}
