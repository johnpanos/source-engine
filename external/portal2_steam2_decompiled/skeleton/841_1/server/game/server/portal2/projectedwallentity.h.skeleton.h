// DWARF declaration skeleton for game/server/portal2/projectedwallentity.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/projectedwallentity.h:25 sizeof=0x474 (i386)
struct CProjectedWallEntity : public CPaintableEntity<CBaseProjectedEntity>
{
public:
	CProjectedWallEntity();  // line 31
	virtual ~CProjectedWallEntity();  // line 32
	virtual void Precache();  // line 34
	virtual void Spawn();  // line 35
	virtual void OnRestore();  // line 36
	virtual bool CreateVPhysics();  // line 37
	virtual bool ShouldSavePhysics();  // line 38
	virtual void UpdateOnRemove();  // line 39
	void ProjectWall();  // line 41
	virtual void GetProjectionExtents( Vector &, Vector & );  // line 44
	void GetExtents( Vector &, Vector &, float );  // line 45
	virtual bool TestCollision( const Ray_t &, unsigned int, trace_t & );  // line 48
	virtual void ComputeWorldSpaceSurroundingBox( Vector *, Vector * );  // line 49
	virtual void NotifyPortalEvent( PortalEvent_t, CPortal_Base2D * );  // line 51
	virtual void SetSourcePortal( CPortal_Base2D * );  // line 53
	virtual void SetHitPortal( CPortal_Base2D * );  // line 54
	bool IsHorizontal();  // line 56
	virtual void Touch( CBaseEntity * );  // line 59
	virtual PaintPowerType GetPaintPowerAtPoint( const Vector & ) const;  // line 61
	virtual void Paint( PaintPowerType, const Vector & );  // line 62
	virtual void CleansePaint();  // line 63
	bool IsWallPainted( const Vector & ) const;  // line 65
	int GetNumSegments() const;  // line 66
	float GetSegmentLength() const;  // line 67
	PaintPowerType GetPaintPowerAtSegment( int ) const;  // line 68
	static CProjectedWallEntity *CreateNewInstance();  // line 70
	virtual CBaseProjectedEntity *CreateNewProjectedEntity();  // line 71
	virtual void PlacePlacementHelper( CInfoPlacementHelper * );  // line 73
	virtual void OnProjected();  // line 75
private:
	void CleanupWall();  // line 78
	void DisplaceObstructingEntities();  // line 79
	void TraceForWallEndpoint( Vector * );  // line 80
	void DrawWallBoundsHighlight();  // line 81
	CPhysCollide *m_pWallCollideable; // +0x424  // line 82
	CNetworkVar( Vector, m_vWorldSpace_WallMins ); // +0x428  // line 85
	CNetworkVar( Vector, m_vWorldSpace_WallMaxs ); // +0x434  // line 86
	CNetworkVar( CHandle<CPortal_Base2D>, m_hColorPortal ); // +0x440  // line 89
	CNetworkVar( float, m_flLength ); // +0x444  // line 92
	CNetworkVar( float, m_flHeight ); // +0x448  // line 93
	CNetworkVar( float, m_flWidth ); // +0x44c  // line 94
	CNetworkVar( bool, m_bIsHorizontal ); // +0x450  // line 96
	CNetworkVar( int, m_nNumSegments ); // +0x454  // line 99
	CNetworkVar( float, m_flSegmentLength ); // +0x458  // line 100
	CNetworkVar( float, m_flParticleUpdateTime ); // +0x45c  // line 102
	CUtlVector<PaintPowerType,CUtlMemory<PaintPowerType, int> > m_PaintPowers; // +0x460  // line 104
	int ComputeSegmentIndex( const Vector & ) const;  // line 106
};

// game/server/portal2/projectedwallentity.h:38 @0x74ccf0 _ZN20CProjectedWallEntity17ShouldSavePhysicsEv
bool CProjectedWallEntity::ShouldSavePhysics()
{
}
