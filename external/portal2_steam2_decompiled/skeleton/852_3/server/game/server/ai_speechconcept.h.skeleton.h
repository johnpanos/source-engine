// DWARF declaration skeleton for game/server/ai_speechconcept.h
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/ai_speechconcept.h:18 sizeof=0x8 (i386)
struct CAI_Concept : public ResponseRules::CRR_Concept
{
public:
	CAI_Concept();  // line 20
	CAI_Concept( const char * );  // line 22
	EHANDLE GetSpeaker() const;  // line 25
	void SetSpeaker( EHANDLE );  // line 26
protected:
	EHANDLE m_hSpeaker; // +0x4  // line 36
};
