// DWARF declaration skeleton for game/client/portal2/c_projectedwallentity.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal2/c_projectedwallentity.h:15 sizeof=0xb38 (i386)
struct C_ProjectedWallEntity : public CPaintableEntity<C_BaseProjectedEntity>, public CSignifierTarget
{
public:
	static typedescription_t m_PredDesc[];  // line 19
	C_ProjectedWallEntity();  // line 21
	virtual void UpdateOnRemove();  // line 23
	virtual CollideType_t GetCollideType();  // line 24
	virtual int DrawModel( int, const RenderableInstance_t & );  // line 25
	virtual bool ShouldDraw();  // line 26
	virtual void GetRenderBounds( Vector &, Vector & );  // line 27
	virtual const QAngle &GetRenderAngles();  // line 28
	virtual RenderableTranslucencyType_t ComputeTranslucencyType();  // line 29
	virtual void ComputeWorldSpaceSurroundingBox( Vector *, Vector * );  // line 30
	virtual void OnPreDataChanged( DataUpdateType_t );  // line 31
	virtual void OnDataChanged( DataUpdateType_t );  // line 32
	virtual void Spawn();  // line 33
	virtual void ClientThink();  // line 34
	virtual bool TestCollision( const Ray_t &, unsigned int, trace_t & );  // line 35
	virtual bool TestHitboxes( const Ray_t &, unsigned int, trace_t & );  // line 36
	virtual void OnProjected();  // line 38
	virtual void GetProjectionExtents( Vector &, Vector & );  // line 39
	void GetExtents( Vector &, Vector &, float );  // line 40
	virtual bool OverrideSignifierPosition();  // line 42
	virtual bool GetSignifierPosition( const Vector &, Vector &, Vector & );  // line 43
	void ProjectWall();  // line 51
	virtual void Touch( C_BaseEntity * );  // line 54
	virtual PaintPowerType GetPaintPowerAtPoint( const Vector & ) const;  // line 56
	virtual void Paint( PaintPowerType, const Vector & );  // line 57
	virtual void CleansePaint();  // line 58
	void SetPaintPower( int, PaintPowerType );  // line 60
	virtual void GetToolRecordingState( KeyValues * );  // line 62
	void RestoreToToolRecordedState( KeyValues * );  // line 63
private:
	void DisplaceObstructingEntities();  // line 66
	void DrawQuadHelper( CMeshBuilder *, const Vector &, const Vector &, const Vector &, float, float, float );  // line 67
	virtual bool InitMaterials();  // line 69
	void ColorWallByPortal( IMaterial * );  // line 71
	bool ShouldSpawnParticles( C_Portal_Base2D * );  // line 73
	void SetupWallParticles();  // line 74
	IMaterial *m_pBodyMaterial; // +0xac0  // line 76
	IMaterial *m_pSideRailMaterial; // +0xac4  // line 77
	IMaterialVar *m_pMyColor; // +0xac8  // line 79
	Vector m_vWorldSpace_WallMins; // +0xacc  // line 81
	Vector m_vWorldSpace_WallMaxs; // +0xad8  // line 82
public:
	// game/client/portal2/c_projectedwallentity.h:86 sizeof=0x4c (i386)
	struct WallCollideableAtTime_t
	{
	public:
		Vector vStart; // +0x0  // line 87
		Vector vEnd; // +0xc  // line 87
		Vector vWorldMins; // +0x18  // line 88
		Vector vWorldMaxs; // +0x24  // line 88
		QAngle qAngles; // +0x30  // line 89
		float flTime[2]; // +0x3c  // line 90
		CPhysCollide *pCollideable; // +0x44  // line 91
		bool bIsPredicted; // +0x48  // line 92
	};  // line 86
private:
	CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> > m_WallCollideables; // +0xae4  // line 94
	float m_flLength; // +0xaf8  // line 96
	float m_flWidth; // +0xafc  // line 97
	float m_flHeight; // +0xb00  // line 98
	bool m_bIsHorizontal; // +0xb04  // line 99
	EHANDLE m_hColorPortal; // +0xb08  // line 100
	float m_flCurDisplayLength; // +0xb0c  // line 102
	float m_fLastProjectedTime; // +0xb10  // line 103
	void SetWallColor( const PaintPowerType & );  // line 106
	int m_nNumSegments; // +0xb14  // line 108
	float m_flSegmentLength; // +0xb18  // line 109
	CUtlVector<PaintPowerType,CUtlMemory<PaintPowerType, int> > m_PaintPowers; // +0xb1c  // line 111
	float m_flParticleUpdateTime; // +0xb30  // line 113
	float m_flPrevParticleUpdateTime; // +0xb34  // line 114
	int ComputeSegmentIndex( const Vector & ) const;  // line 116
};

// game/client/portal2/c_projectedwallentity.h:15 (declaration)
~C_ProjectedWallEntity();

// game/client/portal2/c_projectedwallentity.h:15 @0x535fa0 _ZN21C_ProjectedWallEntityD0Ev
C_ProjectedWallEntity::~C_ProjectedWallEntity()
{
	// inlined CUtlVector<PaintPowerType,CUtlMemory<PaintPowerType, int> >::~CUtlVector() at line 15
	// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::~CUtlVector() at line 15
	// inlined CPaintableEntity<C_BaseProjectedEntity>::~CPaintableEntity() at line 15
	// inlined CPaintableEntity<C_BaseProjectedEntity>::~CPaintableEntity() at line 15
	// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::~CUtlVector() at line 15
}

// game/client/portal2/c_projectedwallentity.h:15 @0x5362e0 _ZN21C_ProjectedWallEntityD1Ev
C_ProjectedWallEntity::~C_ProjectedWallEntity()
{
	// inlined CUtlVector<PaintPowerType,CUtlMemory<PaintPowerType, int> >::~CUtlVector() at line 15
	// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::~CUtlVector() at line 15
	// inlined CPaintableEntity<C_BaseProjectedEntity>::~CPaintableEntity() at line 15
	// inlined CPaintableEntity<C_BaseProjectedEntity>::~CPaintableEntity() at line 15
	// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::~CUtlVector() at line 15
}

// game/client/portal2/c_projectedwallentity.h:26 @0x535830 _ZN21C_ProjectedWallEntity10ShouldDrawEv
bool C_ProjectedWallEntity::ShouldDraw()
{
}

// game/client/portal2/c_projectedwallentity.h:42 @0x535850 _ZN21C_ProjectedWallEntity25OverrideSignifierPositionEv
bool C_ProjectedWallEntity::OverrideSignifierPosition()
{
}

// game/client/portal2/c_projectedwallentity.h:43 @0x535cd0 _ZN21C_ProjectedWallEntity20GetSignifierPositionERK6VectorRS0_S3_
bool C_ProjectedWallEntity::GetSignifierPosition( const Vector &vSource, Vector &vPositionOut, Vector &vNormalOut )
{
	// inlined Vector::operator=() at line 45
}

// game/client/portal2/c_projectedwallentity.h:86 (declaration)
void operator=( const C_ProjectedWallEntity::WallCollideableAtTime_t & );
