// DWARF declaration skeleton for game/client/portal2/c_projectedwallentity.h
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal2/c_projectedwallentity.h:15 sizeof=0x4 (i386)
struct IProjectedWallEntityAutoList
{
public:
	int (**_vptr$IProjectedWallEntityAutoList)(); // +0x0  // line 0
private:
	static CUtlVector<IProjectedWallEntityAutoList*,CUtlMemory<IProjectedWallEntityAutoList*, int> > m_IProjectedWallEntityAutoListAutoList;  // line 15
public:
	IProjectedWallEntityAutoList( bool );  // line 15
	virtual ~IProjectedWallEntityAutoList();  // line 15
	virtual C_BaseEntity *GetEntity();  // line 15
	static void Add( IProjectedWallEntityAutoList * );  // line 15
	static void Remove( IProjectedWallEntityAutoList * );  // line 15
	static const CUtlVector<IProjectedWallEntityAutoList*,CUtlMemory<IProjectedWallEntityAutoList*, int> > &AutoList();  // line 15
};

// game/client/portal2/c_projectedwallentity.h:17
enum ProjectedWallSegmentType
{
	BASE_SEGMENT = 0,
	PAINT_END_1_SEGMENT = 1,
	PAINT_END_2_SEGMENT = 2,
	PAINT_MIDDLE_SEGMENT = 3,
	PAINT_SINGLE_SEGMENT = 4,
};

// game/client/portal2/c_projectedwallentity.h:27 sizeof=0xb5c (i386)
struct C_ProjectedWallEntity : public CPaintableEntity<C_BaseProjectedEntity>, public IProjectedWallEntityAutoList
{
public:
	static typedescription_t m_PredDesc[];  // line 31
	virtual C_BaseEntity *GetEntity();  // line 34
	C_ProjectedWallEntity();  // line 36
	virtual void UpdateOnRemove();  // line 38
	virtual CollideType_t GetCollideType();  // line 39
	virtual int DrawModel( int, const RenderableInstance_t & );  // line 40
	virtual bool ShouldDraw();  // line 41
	virtual void GetRenderBounds( Vector &, Vector & );  // line 42
	virtual const QAngle &GetRenderAngles();  // line 43
	virtual RenderableTranslucencyType_t ComputeTranslucencyType();  // line 44
	virtual void ComputeWorldSpaceSurroundingBox( Vector *, Vector * );  // line 45
	virtual void OnPreDataChanged( DataUpdateType_t );  // line 46
	virtual void OnDataChanged( DataUpdateType_t );  // line 47
	virtual void Spawn();  // line 48
	virtual void ClientThink();  // line 49
	virtual bool TestCollision( const Ray_t &, unsigned int, trace_t & );  // line 50
	virtual bool TestHitboxes( const Ray_t &, unsigned int, trace_t & );  // line 51
	virtual void OnProjected();  // line 53
	virtual void GetProjectionExtents( Vector &, Vector & );  // line 54
	void GetExtents( Vector &, Vector &, float );  // line 55
	void ProjectWall();  // line 57
	virtual void Touch( C_BaseEntity * );  // line 60
	virtual PaintPowerType GetPaintPowerAtPoint( const Vector & ) const;  // line 62
	virtual void Paint( PaintPowerType, const Vector & );  // line 63
	virtual void CleansePaint();  // line 64
	void SetPaintPower( int, PaintPowerType );  // line 66
	virtual void GetToolRecordingState( KeyValues * );  // line 68
	void RestoreToToolRecordedState( KeyValues * );  // line 69
private:
	void DisplaceObstructingEntities();  // line 72
	void DrawQuadHelper( CMeshBuilder *, const Vector &, const Vector &, const Vector &, float, float, float );  // line 73
	virtual bool InitMaterials();  // line 75
	void ColorWallByPortal( IMaterial * );  // line 77
	bool ShouldSpawnParticles( C_Portal_Base2D * );  // line 79
	void SetupWallParticles();  // line 80
	void PaintWallWithPaint( IMesh *, CMeshBuilder, ProjectedWallSegmentType );  // line 82
	void PaintWallWithPaintSegment( IMesh *, CMeshBuilder, ProjectedWallSegmentType, Vector, Vector, Vector, Vector );  // line 83
	IMaterial *m_pBodyMaterial; // +0xac8  // line 85
	IMaterial *m_pPaintMaterialMid; // +0xacc  // line 86
	IMaterial *m_pPaintMaterialEnd1; // +0xad0  // line 87
	IMaterial *m_pPaintMaterialEnd2; // +0xad4  // line 88
	IMaterial *m_pPaintMaterialSing; // +0xad8  // line 89
	IMaterialVar *m_pPaintColorMid; // +0xadc  // line 90
	IMaterialVar *m_pPaintColorEnd1; // +0xae0  // line 91
	IMaterialVar *m_pPaintColorEnd2; // +0xae4  // line 92
	IMaterialVar *m_pPaintColorSing; // +0xae8  // line 93
	IMaterial *m_pSideRailMaterial; // +0xaec  // line 94
	Vector m_vWorldSpace_WallMins; // +0xaf0  // line 97
	Vector m_vWorldSpace_WallMaxs; // +0xafc  // line 98
public:
	// game/client/portal2/c_projectedwallentity.h:102 sizeof=0x4c (i386)
	struct WallCollideableAtTime_t
	{
	public:
		Vector vStart; // +0x0  // line 103
		Vector vEnd; // +0xc  // line 103
		Vector vWorldMins; // +0x18  // line 104
		Vector vWorldMaxs; // +0x24  // line 104
		QAngle qAngles; // +0x30  // line 105
		float flTime[2]; // +0x3c  // line 106
		CPhysCollide *pCollideable; // +0x44  // line 107
		bool bIsPredicted; // +0x48  // line 108
	};  // line 102
private:
	CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> > m_WallCollideables; // +0xb08  // line 110
	float m_flLength; // +0xb1c  // line 112
	float m_flWidth; // +0xb20  // line 113
	float m_flHeight; // +0xb24  // line 114
	bool m_bIsHorizontal; // +0xb28  // line 115
	EHANDLE m_hColorPortal; // +0xb2c  // line 116
	float m_flCurDisplayLength; // +0xb30  // line 118
	float m_fLastProjectedTime; // +0xb34  // line 119
	bool SetPaintSurfaceColor( const PaintPowerType & );  // line 122
	int m_nNumSegments; // +0xb38  // line 124
	float m_flSegmentLength; // +0xb3c  // line 125
	CUtlVector<PaintPowerType,CUtlMemory<PaintPowerType, int> > m_PaintPowers; // +0xb40  // line 127
	float m_flParticleUpdateTime; // +0xb54  // line 131
	float m_flPrevParticleUpdateTime; // +0xb58  // line 132
	int ComputeSegmentIndex( const Vector & ) const;  // line 134
};

// game/client/portal2/c_projectedwallentity.h:27 (declaration)
~C_ProjectedWallEntity();

// game/client/portal2/c_projectedwallentity.h:27 @0x537b00 _ZN21C_ProjectedWallEntityD0Ev
C_ProjectedWallEntity::~C_ProjectedWallEntity()
{
	// inlined CUtlVector<PaintPowerType,CUtlMemory<PaintPowerType, int> >::~CUtlVector() at line 27
	// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::~CUtlVector() at line 27
	// inlined IProjectedWallEntityAutoList::~IProjectedWallEntityAutoList() at line 27
	// inlined CPaintableEntity<C_BaseProjectedEntity>::~CPaintableEntity() at line 27
	// inlined IProjectedWallEntityAutoList::~IProjectedWallEntityAutoList() at line 27
	// inlined CPaintableEntity<C_BaseProjectedEntity>::~CPaintableEntity() at line 27
	// inlined CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t, int> >::~CUtlVector() at line 27
}

// game/client/portal2/c_projectedwallentity.h:27 @0x538040 _ZN21C_ProjectedWallEntityD1Ev
C_ProjectedWallEntity::~C_ProjectedWallEntity()
{
	// inlined C_ProjectedWallEntity::~C_ProjectedWallEntity() at line 27
}

// game/client/portal2/c_projectedwallentity.h:34 @0x5373e0 _ZN21C_ProjectedWallEntity9GetEntityEv
C_BaseEntity *C_ProjectedWallEntity::GetEntity()
{
}

// game/client/portal2/c_projectedwallentity.h:41 @0x537400 _ZN21C_ProjectedWallEntity10ShouldDrawEv
bool C_ProjectedWallEntity::ShouldDraw()
{
}

// game/client/portal2/c_projectedwallentity.h:102 (declaration)
void operator=( const C_ProjectedWallEntity::WallCollideableAtTime_t & );
