// DWARF declaration skeleton for game/server/portal2/fizzler_effects.h
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/fizzler_effects.h:11 sizeof=0x3c0 (i386)
struct FizzlerEffects : public CBaseEntity
{
public:
	static FizzlerEffects *Create( CBaseEntity * );  // line 17
	virtual void Spawn();  // line 19
	virtual int UpdateTransmitState();  // line 20
	void AttachToEntity( CBaseEntity * );  // line 21
private:
	void FizzleThink();  // line 24
};

// game/server/portal2/fizzler_effects.h:11 (declaration)
~FizzlerEffects();

// game/server/portal2/fizzler_effects.h:11 @0x6b6600 _ZN14FizzlerEffectsD0Ev
FizzlerEffects::~FizzlerEffects()
{
}

// game/server/portal2/fizzler_effects.h:11 @0x6b6640 _ZN14FizzlerEffectsD1Ev
FizzlerEffects::~FizzlerEffects()
{
}

// game/server/portal2/fizzler_effects.h:11 (declaration)
void FizzlerEffects();

// game/server/portal2/fizzler_effects.h:20 @0x6b65e0 _ZN14FizzlerEffects19UpdateTransmitStateEv
int FizzlerEffects::UpdateTransmitState()
{
}
