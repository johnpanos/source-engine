// DWARF declaration skeleton for game/client/c_triggers.h
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/c_triggers.h:17 sizeof=0xa60 (i386)
struct C_BaseTrigger : public C_BaseToggle
{
public:
	bool m_bClientSidePredicted; // +0xa5d  // line 23
};

// game/client/c_triggers.h:17 (declaration)
~C_BaseTrigger();

// game/client/c_triggers.h:17 (declaration)
void C_BaseTrigger();

// game/client/c_triggers.h:27 sizeof=0xa64 (i386)
struct C_BaseVPhysicsTrigger : public C_BaseEntity
{
protected:
	bool m_bDisabled; // +0xa5d  // line 36
	string_t m_iFilterName; // +0xa60  // line 37
};

// game/client/c_triggers.h:27 (declaration)
void C_BaseVPhysicsTrigger();

// game/client/c_triggers.h:27 (declaration)
~C_BaseVPhysicsTrigger();
