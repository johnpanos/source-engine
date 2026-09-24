// DWARF declaration skeleton for game/server/portal2/trigger_paint_cleanser.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/trigger_paint_cleanser.h:9 sizeof=0x4e8 (i386)
struct CTriggerPaintCleanser : public CBaseTrigger
{
public:
	CTriggerPaintCleanser();  // line 14
	virtual void Spawn();  // line 16
	virtual void Activate();  // line 17
	virtual void Enable();  // line 18
	bool IsEnabled() const;  // line 20
	virtual void Touch( CBaseEntity * );  // line 22
	virtual int UpdateTransmitState();  // line 24
	virtual void UpdateOnRemove();  // line 26
	static bool PaintCleanserVisibilityCallback( CBaseEntity *, CBasePlayer * );  // line 28
	static bool PaintCleanserVisibilityFilter( CBaseEntity *, CBasePlayer * );  // line 29
};

// game/server/portal2/trigger_paint_cleanser.h:9 (declaration)
~CTriggerPaintCleanser();

// game/server/portal2/trigger_paint_cleanser.h:9 @0x775400 _ZN21CTriggerPaintCleanserD0Ev
CTriggerPaintCleanser::~CTriggerPaintCleanser()
{
}

// game/server/portal2/trigger_paint_cleanser.h:9 @0x775440 _ZN21CTriggerPaintCleanserD1Ev
CTriggerPaintCleanser::~CTriggerPaintCleanser()
{
}

// game/server/portal2/trigger_paint_cleanser.h:34 @0x775200 _ZN21CTriggerPaintCleanser31NetworkStateChanged_m_bDisabledEv
void CTriggerPaintCleanser::NetworkStateChanged_m_bDisabled()
{
	// inlined CBaseEntity::NetworkStateChanged() at line 34
}

// game/server/portal2/trigger_paint_cleanser.h:34 @0x775240 _ZN21CTriggerPaintCleanser31NetworkStateChanged_m_bDisabledEPv
void CTriggerPaintCleanser::NetworkStateChanged_m_bDisabled( void *pVar )
{
	// inlined CBaseEntity::NetworkStateChanged() at line 34
}
