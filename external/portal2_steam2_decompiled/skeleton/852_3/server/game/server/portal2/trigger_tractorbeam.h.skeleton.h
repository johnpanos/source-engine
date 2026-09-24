// DWARF declaration skeleton for game/server/portal2/trigger_tractorbeam.h
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/trigger_tractorbeam.h:24 sizeof=0x438 (i386)
struct CTrigger_TractorBeam : public CBaseVPhysicsTrigger, public IMotionEvent
{
public:
	CTrigger_TractorBeam();  // line 30
	virtual ~CTrigger_TractorBeam();  // line 31
	virtual void Spawn();  // line 33
	virtual void Precache();  // line 34
	virtual void Activate();  // line 35
	virtual void UpdateOnRemove();  // line 36
	virtual void StopLoopingSounds();  // line 37
	virtual bool CreateVPhysics();  // line 38
	virtual void OnRestore();  // line 39
	virtual void StartTouch( CBaseEntity * );  // line 44
	virtual void EndTouch( CBaseEntity * );  // line 45
	void InputSetVelocityLimitTime( inputdata_t & );  // line 47
	float LinearLimit();  // line 49
	void SetLinearForce( const Vector &, float );  // line 51
	void SetLinearForce( float );  // line 57
	void SetAsReversed( bool );  // line 62
	void SetGravityScale( float );  // line 64
	void SetAirDensity( float );  // line 65
	void SetLinearLimit( float );  // line 66
	void SetAngularLimt( float );  // line 67
	void SetProxyEntity( CProjectedTractorBeamEntity * );  // line 68
	void DisablePlayerMovement( bool );  // line 69
	bool HasGravityScale();  // line 71
	bool HasAirDensity();  // line 72
	bool HasLinearLimit();  // line 73
	bool HasLinearScale();  // line 74
	bool HasAngularLimit();  // line 75
	bool HasAngularScale();  // line 76
	bool HasLinearForce();  // line 77
	void SetDirection( const Vector &, const Vector & );  // line 79
	static CTrigger_TractorBeam *CreateTractorBeam( const Vector &, const Vector &, CBaseEntity * );  // line 81
	void UpdateBeam();  // line 82
	virtual IMotionEvent::simresult_e Simulate( IPhysicsMotionController *, IPhysicsObject *, float, Vector &, AngularImpulse & );  // line 84
	void WakeTouchingObjects();  // line 85
	Vector GetStartPoint();  // line 87
	Vector GetEndPoint();  // line 88
	bool IsReversed();  // line 89
	bool IsFromPortal();  // line 90
	bool IsToPortal();  // line 91
	void SetBeamRadius( float );  // line 93
	const float &GetBeamRadius();  // line 94
private:
	void TractorThink();  // line 97
	IPhysicsMotionController *m_pController; // +0x3d0  // line 99
	CNetworkVar( float, m_gravityScale ); // +0x3d4  // line 103
	CNetworkVar( float, m_addAirDensity ); // +0x3d8  // line 104
	CNetworkVar( float, m_linearLimit ); // +0x3dc  // line 105
	CNetworkVar( float, m_linearLimitDelta ); // +0x3e0  // line 106
	CNetworkVar( float, m_linearLimitTime ); // +0x3e4  // line 107
	CNetworkVar( float, m_linearLimitStart ); // +0x3e8  // line 108
	CNetworkVar( float, m_linearLimitStartTime ); // +0x3ec  // line 109
	CNetworkVar( float, m_linearScale ); // +0x3f0  // line 110
	CNetworkVar( float, m_angularLimit ); // +0x3f4  // line 111
	CNetworkVar( float, m_angularScale ); // +0x3f8  // line 112
	CNetworkVar( QAngle, m_linearForceAngles ); // +0x3fc  // line 113
	CNetworkHandle( CProjectedTractorBeamEntity, m_hProxyEntity ); // +0x408  // line 115
	CNetworkVar( Vector, m_vStart ); // +0x40c  // line 117
	CNetworkVar( Vector, m_vEnd ); // +0x418  // line 118
	CNetworkVar( float, m_linearForce ); // +0x424  // line 119
	CNetworkVar( float, m_flRadius ); // +0x428  // line 120
	CNetworkVar( bool, m_bReversed ); // +0x42c  // line 121
	CNetworkVar( bool, m_bFromPortal ); // +0x42d  // line 122
	CNetworkVar( bool, m_bToPortal ); // +0x42e  // line 123
	CNetworkVar( bool, m_bDisablePlayerMove ); // +0x42f  // line 124
	CSoundPatch *m_sndPlayerInBeam; // +0x430  // line 126
	CSoundPatch *m_sndAmbient; // +0x434  // line 127
};

// game/server/portal2/trigger_tractorbeam.h:101 @0x6fc740 _ZN20CTrigger_TractorBeam31NetworkStateChanged_m_bDisabledEv
void CTrigger_TractorBeam::NetworkStateChanged_m_bDisabled()
{
	// inlined CBaseEntity::NetworkStateChanged() at line 101
}

// game/server/portal2/trigger_tractorbeam.h:101 @0x6fca90 _ZN20CTrigger_TractorBeam31NetworkStateChanged_m_bDisabledEPv
void CTrigger_TractorBeam::NetworkStateChanged_m_bDisabled( void *pVar )
{
	// inlined CBaseEntity::NetworkStateChanged() at line 101
}

// game/server/portal2/trigger_tractorbeam.h:138 sizeof=0x420 (i386)
struct CProjectedTractorBeamEntity : public CBaseProjectedEntity
{
public:
	virtual void Spawn();  // line 145
	virtual void UpdateOnRemove();  // line 156
	virtual void GetProjectionExtents( Vector &, Vector & );  // line 166
	float GetLinearForce();  // line 176
	static CProjectedTractorBeamEntity *CreateNewInstance();  // line 185
	virtual CBaseProjectedEntity *CreateNewProjectedEntity();  // line 186
	virtual void OnProjected();  // line 187
private:
	CNetworkHandle( CTrigger_TractorBeam, m_hTractorBeamTrigger ); // +0x41c  // line 190
};

// game/server/portal2/trigger_tractorbeam.h:138 (declaration)
~CProjectedTractorBeamEntity();

// game/server/portal2/trigger_tractorbeam.h:138 @0x6fc780 _ZN27CProjectedTractorBeamEntityD0Ev
CProjectedTractorBeamEntity::~CProjectedTractorBeamEntity()
{
	// inlined CBaseProjectedEntity::~CBaseProjectedEntity() at line 138
}

// game/server/portal2/trigger_tractorbeam.h:138 (declaration)
void CProjectedTractorBeamEntity();

// game/server/portal2/trigger_tractorbeam.h:138 @0x6fca60 _ZN27CProjectedTractorBeamEntityD1Ev
CProjectedTractorBeamEntity::~CProjectedTractorBeamEntity()
{
	// inlined CBaseProjectedEntity::~CBaseProjectedEntity() at line 138
}

// game/server/portal2/trigger_tractorbeam.h:145 @0x6fcad0 _ZN27CProjectedTractorBeamEntity5SpawnEv
void CProjectedTractorBeamEntity::Spawn()
{
	// inlined CTrigger_TractorBeam::CreateTractorBeam() at line 151
	// inlined CNetworkHandleBase<CTrigger_TractorBeam,CProjectedTractorBeamEntity::NetworkVar_m_hTractorBeamTrigger>::operator=() at line 151
}

// game/server/portal2/trigger_tractorbeam.h:156 @0x6fc840 _ZN27CProjectedTractorBeamEntity14UpdateOnRemoveEv
void CProjectedTractorBeamEntity::UpdateOnRemove()
{
	// inlined CNetworkHandleBase<CTrigger_TractorBeam,CProjectedTractorBeamEntity::NetworkVar_m_hTractorBeamTrigger>::operator CTrigger_TractorBeam*() at line 158
}

// game/server/portal2/trigger_tractorbeam.h:166 @0x6fc8b0 _ZN27CProjectedTractorBeamEntity20GetProjectionExtentsER6VectorS1_
void CProjectedTractorBeamEntity::GetProjectionExtents( Vector &outMins, Vector &outMaxs )
{
	const int nBeamWidth;  // line 168
	Vector scratchMins;  // line 169
	Vector scratchMaxs;  // line 170
	// inlined CBaseEntity::GetAbsAngles() at line 173
	// inlined CBaseEntity::GetAbsAngles() at line 172
	// inlined Vector::Vector() at line 170
	// inlined Vector::Vector() at line 169
}
