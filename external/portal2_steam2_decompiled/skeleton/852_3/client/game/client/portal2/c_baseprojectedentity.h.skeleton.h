// DWARF declaration skeleton for game/client/portal2/c_baseprojectedentity.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal2/c_baseprojectedentity.h:19 sizeof=0xab8 (i386)
struct C_BaseProjectedEntity : public C_BaseEntity
{
public:
	static typedescription_t m_PredDesc[];  // line 23
	C_BaseProjectedEntity();  // line 25
	virtual ~C_BaseProjectedEntity();  // line 26
	virtual const Vector &GetStartPoint() const;  // line 28
	virtual const Vector &GetEndPoint() const;  // line 29
	virtual Vector GetLengthVector();  // line 30
	void RecursiveProjection( bool, C_BaseEntity *, C_Portal_Base2D *, const Vector &, const QAngle &, int );  // line 32
	virtual bool IsHittingPortal( Vector *, QAngle *, C_Portal_Base2D ** );  // line 33
	virtual void TestForProjectionChanges();  // line 34
	virtual bool DidRedirectionPortalMove( C_Portal_Base2D * );  // line 36
	virtual void FindProjectedEndpoints();  // line 37
	virtual void SetHitPortal( C_Portal_Base2D * );  // line 39
	virtual C_Portal_Base2D *GetHitPortal();  // line 40
	virtual void SetSourcePortal( C_Portal_Base2D * );  // line 42
	virtual C_Portal_Base2D *GetSourcePortal();  // line 43
	virtual void GetProjectionExtents( Vector &, Vector & );  // line 45
	virtual void OnProjected();  // line 46
	virtual void PostDataUpdate( DataUpdateType_t );  // line 48
	virtual C_BasePlayer *GetPredictionOwner();  // line 50
	static void TestAllForProjectionChanges();  // line 53
	CHandle<C_Portal_Base2D> m_hHitPortal; // +0xa5c  // line 55
	CHandle<C_Portal_Base2D> m_hSourcePortal; // +0xa60  // line 56
	Vector m_vecSourcePortalCenter; // +0xa64  // line 57
	Vector m_vecSourcePortalRemoteCenter; // +0xa70  // line 58
	QAngle m_vecSourcePortalAngle; // +0xa7c  // line 59
	QAngle m_vecSourcePortalRemoteAngle; // +0xa88  // line 60
	Vector m_vecStartPoint; // +0xa94  // line 61
	Vector m_vecEndPoint; // +0xaa0  // line 62
	CHandle<C_BaseProjectedEntity> m_hChildSegment; // +0xaac  // line 63
	CHandle<C_BaseEntity> m_hPlacementHelper; // +0xab0  // line 64
	int m_iMaxRemainingRecursions; // +0xab4  // line 66
};

// game/client/portal2/c_baseprojectedentity.h:28 @0x520750 _ZNK21C_BaseProjectedEntity13GetStartPointEv
const Vector &C_BaseProjectedEntity::GetStartPoint()
{
}

// game/client/portal2/c_baseprojectedentity.h:29 @0x520760 _ZNK21C_BaseProjectedEntity11GetEndPointEv
const Vector &C_BaseProjectedEntity::GetEndPoint()
{
}

// game/client/portal2/c_baseprojectedentity.h:30 @0x520770 _ZN21C_BaseProjectedEntity15GetLengthVectorEv
Vector C_BaseProjectedEntity::GetLengthVector()
{
	// inlined Vector::operator-() at line 30
}

// game/client/portal2/c_baseprojectedentity.h:51 @0x5207d0 _ZN21C_BaseProjectedEntity13ShouldPredictEv
bool C_BaseProjectedEntity::ShouldPredict()
{
	C_BasePlayer *pPredOwner;  // line 51
}
