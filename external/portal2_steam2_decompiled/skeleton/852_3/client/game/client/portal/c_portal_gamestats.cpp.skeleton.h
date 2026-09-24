// DWARF declaration skeleton for game/client/portal/c_portal_gamestats.cpp
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x6aea0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
	// inlined CSteamID::CSteamID() at line 650
	// inlined CSteamID::CSteamID() at line 654
	// inlined CSteamID::CSteamID() at line 656
	// inlined CSteamID::CSteamID() at line 659
	// inlined CSteamID::CSteamID() at line 662
	// inlined C_CPortalGameStats::C_CPortalGameStats() at line 18
	// inlined CUtlVector<IGameStatTracker::IStatContainer*,CUtlMemory<IGameStatTracker::IStatContainer*, int> >::CUtlVector() at line 19
}

// game/client/portal/c_portal_gamestats.cpp:18
C_CPortalGameStats g_PortalGameStats;

// game/client/portal/c_portal_gamestats.cpp:19
void C_CPortalGameStats::s_StatLists;

// game/client/portal/c_portal_gamestats.cpp:27 @0x4de950 _ZN18C_CPortalGameStats4InitEv
bool C_CPortalGameStats::Init()
{
}

// game/client/portal/c_portal_gamestats.cpp:45 @0x4de9d0 _ZN18C_CPortalGameStats8ShutdownEv
void C_CPortalGameStats::Shutdown()
{
}

// game/client/portal/c_portal_gamestats.cpp:60 @0x4debf0 _ZN18C_CPortalGameStats19Event_AddSurveyDataEjjPKc
void C_CPortalGameStats::Event_AddSurveyData( uint32 difficulty, uint32 enjoyment, const char *essay )
{
	// inlined SSurveyData::SSurveyData() at line 62
	// inlined CUtlVector<SSurveyData*,CUtlMemory<SSurveyData*, int> >::AddToTail() at line 62
}

// game/client/portal/c_portal_gamestats.cpp:71 @0x4deb70 _ZN18C_CPortalGameStats15SubmitGameStatsEP9KeyValues
void C_CPortalGameStats::SubmitGameStats( KeyValues *pKV )
{
	int listCount;  // line 73
	// inlined CUtlVector<IGameStatTracker::IStatContainer*,CUtlMemory<IGameStatTracker::IStatContainer*, int> >::Count() at line 73
	{
		int i;  // line 74
	}
}

// game/client/portal/c_portal_gamestats.cpp:88 @0x4df760 _ZN18C_CPortalGameStats14AddDataForSendEP9KeyValues14StatSendType_t
bool C_CPortalGameStats::AddDataForSend( KeyValues *pKV, StatSendType_t sendType )
{
}

// game/client/portal/c_portal_gamestats.cpp:110 @0x4df370 _ZN18C_CPortalGameStats7AddDataEv
void C_CPortalGameStats::AddData()
{
	KeyValues *pKV;  // line 112
	const char *pzMapName;  // line 116
	{
		int k;  // line 124
		// inlined IGameStatTracker::SubmitStat<SSurveyData>() at line 125
	}
	// inlined CUtlVector<SSurveyData*,CUtlMemory<SSurveyData*, int> >::Purge() at line 131
	// inlined CUtlVector<CPortal_StuckPlayer*,CUtlMemory<CPortal_StuckPlayer*, int> >::Purge() at line 132
}

// game/client/portal/c_portal_gamestats.cpp:143 @0x4df340 _ZN18C_CPortalGameStats14OnSessionStartEv
void C_CPortalGameStats::OnSessionStart()
{
	// inlined CSteamWorksGameStatsUploader::OnSessionStart() at line 147
}

// game/client/portal/c_portal_gamestats.cpp:156 @0x4deb20 _ZN18C_CPortalGameStats12OnSessionEndEv
void C_CPortalGameStats::OnSessionEnd()
{
	// inlined CSteamWorksGameStatsUploader::OnSessionEnd() at line 160
}

// game/client/portal/c_portal_gamestats.cpp:169 @0x4deaa0 _ZN18C_CPortalGameStats15Event_LevelInitEv
void C_CPortalGameStats::Event_LevelInit()
{
	const char *name1;  // line 171
}

// game/client/portal/c_portal_gamestats.cpp:188 @0x4ded00 _ZN18C_CPortalGameStats13GetConfigDataEv
void C_CPortalGameStats::GetConfigData()
{
	ConVar in_joystick;  // line 190
	// inlined IGameStatTracker::SubmitStat<SConfigStats>() at line 260
	{
		const char *countryCode;  // line 202
		const char *audioLanguage;  // line 211
		// inlined CSteamAPIContext::SteamApps() at line 211
	}
	// inlined CSteamAPIContext::SteamUtils() at line 199
	ConVarRef m_pitch;  // line 227
	ConVarRef con_enable;  // line 228
	ConVarRef closecaption;  // line 229
	ConVarRef cl_colorblind;  // line 230
	ConVarRef voice_vox;  // line 231
}

// game/client/portal/c_portal_gamestats.cpp:271 @0x4dea30 _ZN18C_CPortalGameStats7OnEventEP9KeyValues
void C_CPortalGameStats::OnEvent( KeyValues *pEvent )
{
	const char *szEvent;  // line 273
}

// public/tier1/utlvector.h:655 @0x4dff30 _ZN10CUtlVectorIP12SConfigStats10CUtlMemoryIS1_iEE10GrowVectorEi
void CUtlVector<SConfigStats*,CUtlMemory<SConfigStats*, int> >::GrowVector( int num )
{
	// inlined CUtlMemory<SConfigStats*,int>::NumAllocated() at line 657
	// inlined CUtlMemory<SConfigStats*,int>::Grow() at line 660
	// inlined CUtlVector<SConfigStats*,CUtlMemory<SConfigStats*, int> >::ResetDbgInfo() at line 664
}

// public/tier1/utlvector.h:655 @0x4e0240 _ZN10CUtlVectorIP11SSurveyData10CUtlMemoryIS1_iEE10GrowVectorEi
void CUtlVector<SSurveyData*,CUtlMemory<SSurveyData*, int> >::GrowVector( int num )
{
	// inlined CUtlMemory<SSurveyData*,int>::NumAllocated() at line 657
	// inlined CUtlMemory<SSurveyData*,int>::Grow() at line 660
	// inlined CUtlVector<SSurveyData*,CUtlMemory<SSurveyData*, int> >::ResetDbgInfo() at line 664
}
