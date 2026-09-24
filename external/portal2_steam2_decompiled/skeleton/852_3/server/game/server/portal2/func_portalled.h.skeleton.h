// DWARF declaration skeleton for game/server/portal2/func_portalled.h
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/func_portalled.h:20 sizeof=0x500 (i386)
struct CFunc_Portalled : public CFuncPortalDetector
{
public:
	COutputEvent m_OnEntityPrePortalled; // +0x4cc  // line 25
	COutputEvent m_OnEntityPostPortalled; // +0x4e4  // line 26
	virtual void OnPrePortalled( CBaseEntity *, bool );  // line 28
	virtual void OnPostPortalled( CBaseEntity *, bool );  // line 29
protected:
	virtual void PortalPlacedInsideBounds( CProp_Portal * );  // line 33
	bool m_bFireOnPlayer; // +0x4fc  // line 35
	bool m_bFireOnDeparture; // +0x4fd  // line 36
	bool m_bFireOnArrival; // +0x4fe  // line 37
};

// game/server/portal2/func_portalled.h:20 (declaration)
void CFunc_Portalled();

// game/server/portal2/func_portalled.h:20 (declaration)
~CFunc_Portalled();

// game/server/portal2/func_portalled.h:20 @0x6b6bf0 _ZN15CFunc_PortalledD1Ev
CFunc_Portalled::~CFunc_Portalled()
{
	// inlined COutputEvent::~COutputEvent() at line 20
	// inlined COutputEvent::~COutputEvent() at line 20
	// inlined COutputEvent::~COutputEvent() at line 20
}

// game/server/portal2/func_portalled.h:20 @0x6b6c60 _ZN15CFunc_PortalledD0Ev
CFunc_Portalled::~CFunc_Portalled()
{
	// inlined COutputEvent::~COutputEvent() at line 20
	// inlined COutputEvent::~COutputEvent() at line 20
	// inlined COutputEvent::~COutputEvent() at line 20
}
