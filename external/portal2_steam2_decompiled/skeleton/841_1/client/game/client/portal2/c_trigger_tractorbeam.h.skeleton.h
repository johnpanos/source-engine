// DWARF declaration skeleton for game/client/portal2/c_trigger_tractorbeam.h
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal2/c_trigger_tractorbeam.h:16 sizeof=0x4 (i386)
struct ITriggerTractorBeamAutoList
{
public:
	int (**_vptr$ITriggerTractorBeamAutoList)(); // +0x0  // line 0
private:
	static CUtlVector<ITriggerTractorBeamAutoList*,CUtlMemory<ITriggerTractorBeamAutoList*, int> > m_ITriggerTractorBeamAutoListAutoList;  // line 16
public:
	ITriggerTractorBeamAutoList( bool );  // line 16
	virtual ~ITriggerTractorBeamAutoList();  // line 16
	virtual C_BaseEntity *GetEntity();  // line 16
	static void Add( ITriggerTractorBeamAutoList * );  // line 16
	static void Remove( ITriggerTractorBeamAutoList * );  // line 16
	static const CUtlVector<ITriggerTractorBeamAutoList*,CUtlMemory<ITriggerTractorBeamAutoList*, int> > &AutoList();  // line 16
};

// game/client/portal2/c_trigger_tractorbeam.h:22 sizeof=0xaec (i386)
struct C_Trigger_TractorBeam : public C_BaseVPhysicsTrigger, public IMotionEvent, public ITriggerTractorBeamAutoList
{
public:
	static typedescription_t m_PredDesc[];  // line 25
	virtual C_BaseEntity *GetEntity();  // line 29
	C_Trigger_TractorBeam();  // line 31
	virtual ~C_Trigger_TractorBeam();  // line 32
	virtual void Spawn();  // line 34
	virtual void OnNewParticleEffect( const char *, CNewParticleEffect * );  // line 35
	virtual int DrawModel( int, const RenderableInstance_t & );  // line 36
	virtual bool ShouldDraw();  // line 37
	virtual void UpdateOnRemove();  // line 38
	virtual void OnDataChanged( DataUpdateType_t );  // line 39
	virtual C_BasePlayer *GetPredictionOwner();  // line 41
	virtual void UpdatePartitionListEntry();  // line 44
	void OnProjected();  // line 46
	virtual void StartTouch( C_BaseEntity * );  // line 48
	virtual void EndTouch( C_BaseEntity * );  // line 49
	virtual void PhysicsSimulate();  // line 51
	virtual IMotionEvent::simresult_e Simulate( IPhysicsMotionController *, IPhysicsObject *, float, Vector &, AngularImpulse & );  // line 53
	void UpdateBeam();  // line 55
	void SetDirection( const Vector &, const Vector & );  // line 56
	virtual RenderableTranslucencyType_t ComputeTranslucencyType();  // line 58
	float GetSpeed();  // line 60
	static void RecvProxy_Start( const CRecvProxyData *, void *, void * );  // line 66
	static void RecvProxy_End( const CRecvProxyData *, void *, void * );  // line 67
	float LinearLimit();  // line 69
	bool HasGravityScale();  // line 70
	bool HasAirDensity();  // line 71
	bool HasLinearLimit();  // line 72
	bool HasLinearScale();  // line 73
	bool HasAngularLimit();  // line 74
	bool HasAngularScale();  // line 75
	bool HasLinearForce();  // line 76
	void SetProxyEntity( C_ProjectedTractorBeamEntity * );  // line 78
	Vector GetStartPoint();  // line 80
	Vector GetEndPoint();  // line 81
	bool IsReversed();  // line 82
	bool IsFromPortal();  // line 83
	bool IsToPortal();  // line 84
	virtual void GetToolRecordingState( KeyValues * );  // line 86
	void RestoreToToolRecordedState( KeyValues * );  // line 87
	virtual bool GetSoundSpatialization( SpatializationInfo_t & );  // line 88
	const float &GetBeamRadius();  // line 90
protected:
	void CreateParticles();  // line 94
	void DrawColumn( IMaterial *, const Vector &, const Vector &, float, const Vector &, const Vector &, float, float, bool, bool, float );  // line 96
	void DrawColumnSegment( CMeshBuilder &, Vector &, const Vector &, float, const Vector &, const Vector &, float, float, float );  // line 97
	Vector m_vStart; // +0xa6c  // line 99
	Vector m_vEnd; // +0xa78  // line 100
	float m_linearForce; // +0xa84  // line 101
	float m_flRadius; // +0xa88  // line 102
	bool m_bReversed; // +0xa8c  // line 103
	bool m_bFromPortal; // +0xa8d  // line 104
	bool m_bToPortal; // +0xa8e  // line 105
	bool m_bDisablePlayerMove; // +0xa8f  // line 106
	IMaterial *m_pMaterial1; // +0xa90  // line 108
	IMaterial *m_pMaterial2; // +0xa94  // line 109
	IMaterial *m_pMaterial3; // +0xa98  // line 110
	bool m_bRecreateParticles; // +0xa9c  // line 111
	CHandle<C_ProjectedTractorBeamEntity> m_hProxyEntity; // +0xaa0  // line 113
	CUtlReference<CNewParticleEffect> m_hCoreEffect; // +0xaa4  // line 114
	IPhysicsMotionController *m_pController; // +0xab0  // line 116
	float m_gravityScale; // +0xab4  // line 119
	float m_addAirDensity; // +0xab8  // line 120
	float m_linearLimit; // +0xabc  // line 121
	float m_linearLimitDelta; // +0xac0  // line 122
	float m_linearLimitTime; // +0xac4  // line 123
	float m_linearLimitStart; // +0xac8  // line 124
	float m_linearLimitStartTime; // +0xacc  // line 125
	float m_linearScale; // +0xad0  // line 126
	float m_angularLimit; // +0xad4  // line 127
	float m_angularScale; // +0xad8  // line 128
	QAngle m_linearForceAngles; // +0xadc  // line 129
private:
	float m_flStartTime; // +0xae8  // line 132
};

// game/client/portal2/c_trigger_tractorbeam.h:29 @0x540700 _ZN21C_Trigger_TractorBeam9GetEntityEv
C_BaseEntity *C_Trigger_TractorBeam::GetEntity()
{
}

// game/client/portal2/c_trigger_tractorbeam.h:42 @0x540aa0 _ZN21C_Trigger_TractorBeam13ShouldPredictEv
bool C_Trigger_TractorBeam::ShouldPredict()
{
	C_BasePlayer *pPredOwner;  // line 42
}

// game/client/portal2/c_trigger_tractorbeam.h:58 @0x540710 _ZN21C_Trigger_TractorBeam23ComputeTranslucencyTypeEv
RenderableTranslucencyType_t C_Trigger_TractorBeam::ComputeTranslucencyType()
{
}

// game/client/portal2/c_trigger_tractorbeam.h:135
typedef C_Trigger_TractorBeam CTrigger_TractorBeam;

// game/client/portal2/c_trigger_tractorbeam.h:143 sizeof=0xac4 (i386)
struct C_ProjectedTractorBeamEntity : public C_BaseProjectedEntity
{
public:
	static typedescription_t m_PredDesc[];  // line 146
	virtual void GetProjectionExtents( Vector &, Vector & );  // line 150
	virtual void OnProjected();  // line 160
private:
	CHandle<C_Trigger_TractorBeam> m_hTractorBeamTrigger; // +0xac0  // line 182
};

// game/client/portal2/c_trigger_tractorbeam.h:143 (declaration)
~C_ProjectedTractorBeamEntity();

// game/client/portal2/c_trigger_tractorbeam.h:143 @0x540860 _ZN28C_ProjectedTractorBeamEntityD0Ev
C_ProjectedTractorBeamEntity::~C_ProjectedTractorBeamEntity()
{
}

// game/client/portal2/c_trigger_tractorbeam.h:143 @0x5408d0 _ZN28C_ProjectedTractorBeamEntityD1Ev
C_ProjectedTractorBeamEntity::~C_ProjectedTractorBeamEntity()
{
}

// game/client/portal2/c_trigger_tractorbeam.h:143 (declaration)
void C_ProjectedTractorBeamEntity();

// game/client/portal2/c_trigger_tractorbeam.h:150 @0x540a30 _ZN28C_ProjectedTractorBeamEntity20GetProjectionExtentsER6VectorS1_
void C_ProjectedTractorBeamEntity::GetProjectionExtents( Vector &outMins, Vector &outMaxs )
{
	const int nBeamWidth;  // line 152
	Vector scratchMins;  // line 153
	Vector scratchMaxs;  // line 154
	// inlined Vector::Vector() at line 153
	// inlined Vector::Vector() at line 154
}

// game/client/portal2/c_trigger_tractorbeam.h:160 @0x540790 _ZN28C_ProjectedTractorBeamEntity11OnProjectedEv
void C_ProjectedTractorBeamEntity::OnProjected()
{
	C_Trigger_TractorBeam *pBeam;  // line 164
	// inlined C_BaseEntity::GetSimulatingPlayer() at line 170
	// inlined CHandle<C_Trigger_TractorBeam>::Get() at line 164
}
