// DWARF declaration skeleton for game/client/portal2/c_trigger_tractorbeam.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal2/c_trigger_tractorbeam.h:18 sizeof=0xae4 (i386)
struct C_Trigger_TractorBeam : public C_BaseVPhysicsTrigger, public IMotionEvent
{
public:
	static typedescription_t m_PredDesc[];  // line 21
	C_Trigger_TractorBeam();  // line 24
	virtual ~C_Trigger_TractorBeam();  // line 25
	virtual void Spawn();  // line 27
	virtual void OnNewParticleEffect( const char *, CNewParticleEffect * );  // line 28
	virtual int DrawModel( int, const RenderableInstance_t & );  // line 29
	virtual bool ShouldDraw();  // line 30
	virtual void UpdateOnRemove();  // line 31
	virtual void OnDataChanged( DataUpdateType_t );  // line 32
	virtual C_BasePlayer *GetPredictionOwner();  // line 34
	virtual void UpdatePartitionListEntry();  // line 37
	void OnProjected();  // line 39
	virtual void StartTouch( C_BaseEntity * );  // line 41
	virtual void EndTouch( C_BaseEntity * );  // line 42
	virtual void PhysicsSimulate();  // line 44
	virtual IMotionEvent::simresult_e Simulate( IPhysicsMotionController *, IPhysicsObject *, float, Vector &, AngularImpulse & );  // line 46
	void UpdateBeam();  // line 48
	void SetDirection( const Vector &, const Vector & );  // line 49
	virtual RenderableTranslucencyType_t ComputeTranslucencyType();  // line 51
	float GetSpeed();  // line 53
	static void RecvProxy_Start( const CRecvProxyData *, void *, void * );  // line 59
	static void RecvProxy_End( const CRecvProxyData *, void *, void * );  // line 60
	float LinearLimit();  // line 62
	bool HasGravityScale();  // line 63
	bool HasAirDensity();  // line 64
	bool HasLinearLimit();  // line 65
	bool HasLinearScale();  // line 66
	bool HasAngularLimit();  // line 67
	bool HasAngularScale();  // line 68
	bool HasLinearForce();  // line 69
	void SetProxyEntity( C_ProjectedTractorBeamEntity * );  // line 71
	Vector GetStartPoint();  // line 73
	Vector GetEndPoint();  // line 74
	bool IsReversed();  // line 75
	bool IsFromPortal();  // line 76
	bool IsToPortal();  // line 77
	virtual void GetToolRecordingState( KeyValues * );  // line 79
	void RestoreToToolRecordedState( KeyValues * );  // line 80
	virtual bool GetSoundSpatialization( SpatializationInfo_t & );  // line 81
	const float &GetBeamRadius();  // line 83
protected:
	void CreateParticles();  // line 87
	void DrawColumn( IMaterial *, const Vector &, const Vector &, float, const Vector &, const Vector &, float, float, bool, bool, float );  // line 89
	void DrawColumnSegment( CMeshBuilder &, Vector &, const Vector &, float, const Vector &, const Vector &, float, float, float );  // line 90
	Vector m_vStart; // +0xa64  // line 92
	Vector m_vEnd; // +0xa70  // line 93
	float m_linearForce; // +0xa7c  // line 94
	float m_flRadius; // +0xa80  // line 95
	bool m_bReversed; // +0xa84  // line 96
	bool m_bFromPortal; // +0xa85  // line 97
	bool m_bToPortal; // +0xa86  // line 98
	bool m_bDisablePlayerMove; // +0xa87  // line 99
	IMaterial *m_pMaterial1; // +0xa88  // line 101
	IMaterial *m_pMaterial2; // +0xa8c  // line 102
	IMaterial *m_pMaterial3; // +0xa90  // line 103
	bool m_bRecreateParticles; // +0xa94  // line 104
	CHandle<C_ProjectedTractorBeamEntity> m_hProxyEntity; // +0xa98  // line 106
	CUtlReference<CNewParticleEffect> m_hCoreEffect; // +0xa9c  // line 107
	IPhysicsMotionController *m_pController; // +0xaa8  // line 109
	float m_gravityScale; // +0xaac  // line 112
	float m_addAirDensity; // +0xab0  // line 113
	float m_linearLimit; // +0xab4  // line 114
	float m_linearLimitDelta; // +0xab8  // line 115
	float m_linearLimitTime; // +0xabc  // line 116
	float m_linearLimitStart; // +0xac0  // line 117
	float m_linearLimitStartTime; // +0xac4  // line 118
	float m_linearScale; // +0xac8  // line 119
	float m_angularLimit; // +0xacc  // line 120
	float m_angularScale; // +0xad0  // line 121
	QAngle m_linearForceAngles; // +0xad4  // line 122
private:
	float m_flStartTime; // +0xae0  // line 125
};

// game/client/portal2/c_trigger_tractorbeam.h:35 @0x53e360 _ZN21C_Trigger_TractorBeam13ShouldPredictEv
bool C_Trigger_TractorBeam::ShouldPredict()
{
	C_BasePlayer *pPredOwner;  // line 35
}

// game/client/portal2/c_trigger_tractorbeam.h:51 @0x53df00 _ZN21C_Trigger_TractorBeam23ComputeTranslucencyTypeEv
RenderableTranslucencyType_t C_Trigger_TractorBeam::ComputeTranslucencyType()
{
}

// game/client/portal2/c_trigger_tractorbeam.h:128
typedef C_Trigger_TractorBeam CTrigger_TractorBeam;

// game/client/portal2/c_trigger_tractorbeam.h:136 sizeof=0xabc (i386)
struct C_ProjectedTractorBeamEntity : public C_BaseProjectedEntity
{
public:
	static typedescription_t m_PredDesc[];  // line 139
	virtual void GetProjectionExtents( Vector &, Vector & );  // line 143
	virtual void OnProjected();  // line 153
private:
	CHandle<C_Trigger_TractorBeam> m_hTractorBeamTrigger; // +0xab8  // line 175
};

// game/client/portal2/c_trigger_tractorbeam.h:136 (declaration)
~C_ProjectedTractorBeamEntity();

// game/client/portal2/c_trigger_tractorbeam.h:136 @0x53e130 _ZN28C_ProjectedTractorBeamEntityD0Ev
C_ProjectedTractorBeamEntity::~C_ProjectedTractorBeamEntity()
{
}

// game/client/portal2/c_trigger_tractorbeam.h:136 @0x53e1a0 _ZN28C_ProjectedTractorBeamEntityD1Ev
C_ProjectedTractorBeamEntity::~C_ProjectedTractorBeamEntity()
{
}

// game/client/portal2/c_trigger_tractorbeam.h:136 (declaration)
void C_ProjectedTractorBeamEntity();

// game/client/portal2/c_trigger_tractorbeam.h:143 @0x53e2f0 _ZN28C_ProjectedTractorBeamEntity20GetProjectionExtentsER6VectorS1_
void C_ProjectedTractorBeamEntity::GetProjectionExtents( Vector &outMins, Vector &outMaxs )
{
	const int nBeamWidth;  // line 145
	Vector scratchMins;  // line 146
	Vector scratchMaxs;  // line 147
	// inlined Vector::Vector() at line 146
	// inlined Vector::Vector() at line 147
}

// game/client/portal2/c_trigger_tractorbeam.h:153 @0x53e060 _ZN28C_ProjectedTractorBeamEntity11OnProjectedEv
void C_ProjectedTractorBeamEntity::OnProjected()
{
	C_Trigger_TractorBeam *pBeam;  // line 157
	// inlined C_BaseEntity::GetSimulatingPlayer() at line 163
	// inlined CHandle<C_Trigger_TractorBeam>::Get() at line 157
}
