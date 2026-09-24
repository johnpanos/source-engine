// DWARF declaration skeleton for game/server/portal2/pointsurvey.h
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/pointsurvey.h:13 sizeof=0x3dc (i386)
struct CPointSurvey : public CBaseEntity
{
public:
	virtual void Activate();  // line 19
	void OnSurveyCompleted();  // line 23
protected:
	void InputShowSurvey( inputdata_t & );  // line 27
	CNetworkVar( string_t, m_iszSurveyName ); // +0x3c0  // line 29
	COutputEvent m_OnSurveyComplete; // +0x3c4  // line 30
};

// game/server/portal2/pointsurvey.h:13 (declaration)
~CPointSurvey();

// game/server/portal2/pointsurvey.h:13 @0x6d2ba0 _ZN12CPointSurveyD1Ev
CPointSurvey::~CPointSurvey()
{
	// inlined COutputEvent::~COutputEvent() at line 13
}

// game/server/portal2/pointsurvey.h:13 (declaration)
void CPointSurvey();

// game/server/portal2/pointsurvey.h:13 @0x6d2c90 _ZN12CPointSurveyD0Ev
CPointSurvey::~CPointSurvey()
{
	// inlined COutputEvent::~COutputEvent() at line 13
}
