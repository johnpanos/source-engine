// DWARF declaration skeleton for game/server/portal2/baseprojectedentity.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/baseprojectedentity.h:20 sizeof=0x420 (i386)
struct CBaseProjectedEntity : public CBaseEntity
{
public:
	CBaseProjectedEntity();  // line 26
	virtual void Spawn();  // line 28
	virtual void FindProjectedEndpoints();  // line 30
	virtual void SetHitPortal( CPortal_Base2D * );  // line 32
	virtual CPortal_Base2D *GetHitPortal();  // line 33
	virtual void SetSourcePortal( CPortal_Base2D * );  // line 35
	virtual CPortal_Base2D *GetSourcePortal();  // line 36
	virtual bool DidRedirectionPortalMove( CPortal_Base2D * );  // line 38
	virtual const Vector &GetStartPoint() const;  // line 40
	virtual const Vector &GetEndPoint() const;  // line 41
	virtual Vector GetLengthVector();  // line 42
	virtual void GetProjectionExtents( Vector &, Vector & );  // line 43
	void RecursiveProjection( bool, CBaseProjector *, CPortal_Base2D *, const Vector &, const QAngle &, int, bool );  // line 47
	virtual bool IsHittingPortal( Vector *, QAngle *, CPortal_Base2D ** );  // line 48
	virtual void TestForProjectionChanges();  // line 49
	virtual void UpdateOnRemove();  // line 51
	virtual void OnRestore();  // line 52
	virtual CBaseProjectedEntity *CreateNewProjectedEntity();  // line 55
	virtual void PlacePlacementHelper( CInfoPlacementHelper * );  // line 56
	virtual void OnProjected();  // line 58
protected:
	CNetworkHandle( CPortal_Base2D, m_hHitPortal ); // +0x3c0  // line 62
	CNetworkHandle( CPortal_Base2D, m_hSourcePortal ); // +0x3c4  // line 63
	CNetworkVector( Vector, m_vecSourcePortalCenter ); // +0x3c8  // line 64
	CNetworkVector( Vector, m_vecSourcePortalRemoteCenter ); // +0x3d4  // line 65
	CNetworkQAngle( QAngle, m_vecSourcePortalAngle ); // +0x3e0  // line 66
	CNetworkQAngle( QAngle, m_vecSourcePortalRemoteAngle ); // +0x3ec  // line 67
	CNetworkVector( Vector, m_vecStartPoint ); // +0x3f8  // line 68
	CNetworkVector( Vector, m_vecEndPoint ); // +0x404  // line 69
	CNetworkHandle( CBaseProjectedEntity, m_hChildSegment ); // +0x410  // line 71
	CNetworkHandle( CInfoPlacementHelper, m_hPlacementHelper ); // +0x414  // line 72
	bool m_bCreatePlacementHelper; // +0x418  // line 74
private:
	CNetworkVar( int, m_iMaxRemainingRecursions ); // +0x41c  // line 77
};

// game/server/portal2/baseprojectedentity.h:20 (declaration)
~CBaseProjectedEntity();

// game/server/portal2/baseprojectedentity.h:20 @0x707630 _ZN20CBaseProjectedEntityD0Ev
CBaseProjectedEntity::~CBaseProjectedEntity()
{
}

// game/server/portal2/baseprojectedentity.h:20 @0x707670 _ZN20CBaseProjectedEntityD1Ev
CBaseProjectedEntity::~CBaseProjectedEntity()
{
}

// game/server/portal2/baseprojectedentity.h:40 @0x707610 _ZNK20CBaseProjectedEntity13GetStartPointEv
const Vector &CBaseProjectedEntity::GetStartPoint()
{
}

// game/server/portal2/baseprojectedentity.h:41 @0x707620 _ZNK20CBaseProjectedEntity11GetEndPointEv
const Vector &CBaseProjectedEntity::GetEndPoint()
{
}

// game/server/portal2/baseprojectedentity.h:42 @0x707690 _ZN20CBaseProjectedEntity15GetLengthVectorEv
Vector CBaseProjectedEntity::GetLengthVector()
{
	// inlined Vector::operator-() at line 42
}
