// DWARF declaration skeleton for game/shared/gameeventlistener.h
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/shared/gameeventlistener.h:22 sizeof=0xc (i386)
struct CGameEventListener : public IGameEventListener2
{
public:
	CGameEventListener();  // line 24
	virtual ~CGameEventListener();  // line 29
	void ListenForGameEvent( const char * );  // line 35
	void StopListeningForAllEvents();  // line 48
	virtual void FireGameEvent( IGameEvent * );  // line 61
	int m_nDebugID; // +0x4  // line 62
	virtual int GetEventDebugID();  // line 63
private:
	bool m_bRegisteredForEvents; // +0x8  // line 68
};
