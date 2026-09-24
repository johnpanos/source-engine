// DWARF declaration skeleton for game/server/portal2/baseprojector.h
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/baseprojector.h:22 sizeof=0x51c (i386)
struct CBaseProjector : public CBaseAnimating
{
public:
	CBaseProjector();  // line 26
	virtual ~CBaseProjector();  // line 27
	virtual void Spawn();  // line 29
	virtual void Activate();  // line 30
	virtual void UpdateOnRemove();  // line 31
	virtual void OnRestore();  // line 32
	virtual void Project();  // line 34
	virtual CBaseProjectedEntity *CreateNewProjectedEntity();  // line 37
	void EnableProjection( bool );  // line 39
	void TestProjectionChangesThink();  // line 42
	static void TestAllForProjectionChanges();  // line 43
	CNetworkVar( bool, m_bEnabled ); // +0x514  // line 45
protected:
	void Cleanup();  // line 48
	virtual int DrawDebugTextOverlays();  // line 49
	void DrawBoundsHighlight();  // line 50
	void TestForPortalChanges();  // line 51
	void InputEnable( inputdata_t & );  // line 53
	void InputDisable( inputdata_t & );  // line 54
	CHandle<CBaseProjectedEntity> m_hFirstChild; // +0x518  // line 56
};
