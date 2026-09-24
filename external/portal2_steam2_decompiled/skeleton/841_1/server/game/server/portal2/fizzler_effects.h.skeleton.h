// DWARF declaration skeleton for game/server/portal2/fizzler_effects.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/fizzler_effects.h:11 sizeof=0x3c8 (i386)
struct FizzlerEffects : public CBaseEntity
{
public:
	FizzlerEffects();  // line 17
	static FizzlerEffects *Create( CBaseEntity * );  // line 18
	virtual void Spawn();  // line 20
	virtual int UpdateTransmitState();  // line 21
	void AttachToEntity( CBaseEntity * );  // line 22
private:
	void FizzleThink();  // line 25
	float m_ElapsedAnimationTime; // +0x3c0  // line 27
	bool m_ScaleStarted; // +0x3c4  // line 28
};

// game/server/portal2/fizzler_effects.h:11 (declaration)
~FizzlerEffects();

// game/server/portal2/fizzler_effects.h:11 @0x712640 _ZN14FizzlerEffectsD0Ev
FizzlerEffects::~FizzlerEffects()
{
}

// game/server/portal2/fizzler_effects.h:11 @0x712680 _ZN14FizzlerEffectsD1Ev
FizzlerEffects::~FizzlerEffects()
{
}

// game/server/portal2/fizzler_effects.h:21 @0x712620 _ZN14FizzlerEffects19UpdateTransmitStateEv
int FizzlerEffects::UpdateTransmitState()
{
}
