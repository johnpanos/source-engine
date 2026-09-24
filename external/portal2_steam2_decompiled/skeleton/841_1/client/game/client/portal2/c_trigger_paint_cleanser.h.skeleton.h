// DWARF declaration skeleton for game/client/portal2/c_trigger_paint_cleanser.h
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal2/c_trigger_paint_cleanser.h:13 sizeof=0xa60 (i386)
struct C_TriggerPaintCleanser : public C_BaseTrigger
{
public:
	C_TriggerPaintCleanser();  // line 19
	virtual ~C_TriggerPaintCleanser();  // line 20
	virtual void Spawn();  // line 22
	virtual void UpdateOnRemove();  // line 24
	bool IsEnabled() const;  // line 26
private:
	bool m_bDisabled; // +0xa5e  // line 29
};
