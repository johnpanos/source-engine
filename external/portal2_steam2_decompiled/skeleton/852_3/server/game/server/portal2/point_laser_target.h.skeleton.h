// DWARF declaration skeleton for game/server/portal2/point_laser_target.h
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/point_laser_target.h:26 sizeof=0x4 (i386)
struct ICatcher
{
public:
	int (**_vptr$ICatcher)(); // +0x0  // line 0
	virtual void OnPowered();  // line 28
	virtual void OnUnPowered();  // line 29
};

// game/server/portal2/point_laser_target.h:36 sizeof=0x4 (i386)
struct CCatcher : public ICatcher
{
public:
	virtual void OnPowered();  // line 38
	virtual void OnUnPowered();  // line 39
};

// game/server/portal2/point_laser_target.h:38 @0x6d3f60 _ZN8CCatcher9OnPoweredEv
void CCatcher::OnPowered()
{
}

// game/server/portal2/point_laser_target.h:39 @0x6d3f70 _ZN8CCatcher11OnUnPoweredEv
void CCatcher::OnUnPowered()
{
}

// game/server/portal2/point_laser_target.h:46 sizeof=0x3fc (i386)
struct CPortalLaserTarget : public CBaseEntity
{
public:
	virtual void Spawn();  // line 50
	virtual int OnTakeDamage( const CTakeDamageInfo & );  // line 52
	bool IsTerminalPoint() const;  // line 54
	void SetCatcher( CCatcher * );  // line 56
	bool IsPowered();  // line 58
protected:
	void DisableThink();  // line 63
	COutputEvent m_OnPowered; // +0x3c0  // line 65
	COutputEvent m_OnUnpowered; // +0x3d8  // line 66
	bool m_bPowered; // +0x3f0  // line 67
	bool m_bTerminalPoint; // +0x3f1  // line 68
	CCatcher *m_pCatcher; // +0x3f4  // line 69
	string_t m_ModelName; // +0x3f8  // line 70
};

// game/server/portal2/point_laser_target.h:46 (declaration)
void CPortalLaserTarget();

// game/server/portal2/point_laser_target.h:46 (declaration)
~CPortalLaserTarget();

// game/server/portal2/point_laser_target.h:46 @0x6d4240 _ZN18CPortalLaserTargetD0Ev
CPortalLaserTarget::~CPortalLaserTarget()
{
	// inlined COutputEvent::~COutputEvent() at line 46
	// inlined COutputEvent::~COutputEvent() at line 46
	// inlined COutputEvent::~COutputEvent() at line 46
}

// game/server/portal2/point_laser_target.h:46 @0x6d4350 _ZN18CPortalLaserTargetD1Ev
CPortalLaserTarget::~CPortalLaserTarget()
{
	// inlined COutputEvent::~COutputEvent() at line 46
	// inlined COutputEvent::~COutputEvent() at line 46
	// inlined COutputEvent::~COutputEvent() at line 46
}

// game/server/portal2/point_laser_target.h:83 sizeof=0x55c (i386)
struct CLaserCatcher : public CBaseAnimating, public CCatcher
{
public:
	CLaserCatcher();  // line 88
	virtual void Precache();  // line 90
	virtual void Spawn();  // line 91
	virtual bool CreateVPhysics();  // line 92
	virtual void Activate();  // line 93
	virtual void AnimateThink();  // line 94
	virtual void UpdateOnRemove();  // line 95
	virtual void OnPowered();  // line 97
	virtual void OnUnPowered();  // line 98
	virtual bool PoweringDown();  // line 100
	virtual bool PoweringUp();  // line 101
	virtual void CreateHelperEntities();  // line 103
	virtual const char *GetCatcherModelName();  // line 104
	virtual bool IsTerminalPoint();  // line 105
	virtual float SpinUp( float );  // line 107
	virtual float SpinDown( float );  // line 108
	virtual void OnRestore();  // line 109
protected:
	CPortalLaserTarget *m_pCatcherLaserTarget; // +0x518  // line 114
	COutputEvent m_OnPowered; // +0x51c  // line 115
	COutputEvent m_OnUnpowered; // +0x534  // line 116
	int m_iPowerState; // +0x54c  // line 118
private:
	int m_IdleSequence; // +0x550  // line 122
	int m_PowerOnSequence; // +0x554  // line 123
	int m_iTargetAttachment; // +0x558  // line 125
};

// game/server/portal2/point_laser_target.h:83 (declaration)
~CLaserCatcher();

// game/server/portal2/point_laser_target.h:83 @0x6d42c0 _ZN13CLaserCatcherD1Ev
CLaserCatcher::~CLaserCatcher()
{
	// inlined COutputEvent::~COutputEvent() at line 83
	// inlined COutputEvent::~COutputEvent() at line 83
	// inlined COutputEvent::~COutputEvent() at line 83
}

// game/server/portal2/point_laser_target.h:83 @0x6d43c0 _ZN13CLaserCatcherD0Ev
CLaserCatcher::~CLaserCatcher()
{
	// inlined COutputEvent::~COutputEvent() at line 83
	// inlined COutputEvent::~COutputEvent() at line 83
	// inlined COutputEvent::~COutputEvent() at line 83
}

// game/server/portal2/point_laser_target.h:133 sizeof=0x55c (i386)
struct CLaserRelay : public CLaserCatcher
{
private:
	virtual const char *GetCatcherModelName();  // line 137
	virtual bool IsTerminalPoint();  // line 138
};

// game/server/portal2/point_laser_target.h:133 (declaration)
~CLaserRelay();

// game/server/portal2/point_laser_target.h:133 @0x6d4040 _ZN11CLaserRelayD1Ev
CLaserRelay::~CLaserRelay()
{
	// inlined CLaserCatcher::~CLaserCatcher() at line 133
}

// game/server/portal2/point_laser_target.h:133 (declaration)
void CLaserRelay();

// game/server/portal2/point_laser_target.h:133 @0x6d4450 _ZN11CLaserRelayD0Ev
CLaserRelay::~CLaserRelay()
{
	// inlined CLaserCatcher::~CLaserCatcher() at line 133
}
