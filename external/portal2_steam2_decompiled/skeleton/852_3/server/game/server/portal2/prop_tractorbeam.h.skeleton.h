// DWARF declaration skeleton for game/server/portal2/prop_tractorbeam.h
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/prop_tractorbeam.h:20 sizeof=0x52c (i386)
struct CPropTractorBeamProjector : public CBaseProjector
{
public:
	virtual void Spawn();  // line 26
	virtual void Precache();  // line 27
	virtual void Activate();  // line 28
	virtual void Project();  // line 30
	float GetLinearForce() const;  // line 32
protected:
	virtual CBaseProjectedEntity *CreateNewProjectedEntity();  // line 35
	void InputSetLinearForce( inputdata_t & );  // line 36
	CNetworkVar( float, m_flLinearForce ); // +0x51c  // line 38
	CNetworkVar( Vector, m_vEndPos ); // +0x520  // line 39
};

// game/server/portal2/prop_tractorbeam.h:20 (declaration)
~CPropTractorBeamProjector();

// game/server/portal2/prop_tractorbeam.h:20 @0x6f5750 _ZN25CPropTractorBeamProjectorD0Ev
CPropTractorBeamProjector::~CPropTractorBeamProjector()
{
}

// game/server/portal2/prop_tractorbeam.h:20 @0x6f5790 _ZN25CPropTractorBeamProjectorD1Ev
CPropTractorBeamProjector::~CPropTractorBeamProjector()
{
}

// game/server/portal2/prop_tractorbeam.h:20 (declaration)
void CPropTractorBeamProjector();
