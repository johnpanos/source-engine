// DWARF declaration skeleton for game/server/inforemarkable.h
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/inforemarkable.h:24 sizeof=0x3c8 (i386)
struct CInfoRemarkable : public CPointEntity
{
public:
	static CUtlLinkedList<CInfoRemarkable*,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CInfoRemarkable*, short unsigned int>, short unsigned int> > *GetListOfAllThatIsRemarkable();  // line 29
	virtual ~CInfoRemarkable();  // line 31
	virtual void Spawn();  // line 32
	void ResetCount();  // line 33
	const char *GetRemarkContext() const;  // line 35
	int m_iTimesRemarkedUpon; // +0x3c0  // line 38
	string_t m_szRemarkContext; // +0x3c4  // line 41
};
