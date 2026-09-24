// DWARF declaration skeleton for game/server/portal2/prop_paint_bomb.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/prop_paint_bomb.h:11 sizeof=0x79c (i386)
struct CPropPaintBomb : public CPhysicsProp
{
public:
	CPropPaintBomb();  // line 17
	virtual ~CPropPaintBomb();  // line 18
	virtual void Precache();  // line 20
	virtual void Spawn();  // line 21
	virtual void Event_Killed( const CTakeDamageInfo & );  // line 23
	virtual void OnFizzled();  // line 24
	virtual void Break( CBaseEntity *, const CTakeDamageInfo & );  // line 26
	void SetPaintPowerType( PaintPowerType );  // line 28
	void CleansePaintPower();  // line 29
	virtual int UpdateTransmitState();  // line 31
private:
	void InputDissolve( inputdata_t & );  // line 34
	void InputDisablePortalFunnel( inputdata_t & );  // line 35
	void InputEnablePortalFunnel( inputdata_t & );  // line 36
	COutputEvent m_OnFizzled; // +0x768  // line 38
	COutputEvent m_OnExploded; // +0x780  // line 39
	CNetworkVar( int, m_nPaintPowerType ); // +0x798  // line 41
};
