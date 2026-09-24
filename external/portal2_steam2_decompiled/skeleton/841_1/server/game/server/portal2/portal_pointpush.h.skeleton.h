// DWARF declaration skeleton for game/server/portal2/portal_pointpush.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/portal_pointpush.h:23 sizeof=0x3d4 (i386)
struct CPortal_PointPush : public CBaseEntity
{
public:
	virtual void Activate();  // line 29
	void PushThink();  // line 31
	void InputEnable( inputdata_t & );  // line 33
	void InputDisable( inputdata_t & );  // line 34
	virtual int UpdateTransmitState();  // line 36
	virtual void DrawDebugGeometryOverlays();  // line 38
private:
	virtual void NotifyPortalEvent( PortalEvent_t, CPortal_Base2D * );  // line 41
	void PushAtPoint( const Vector &, const QAngle &, CPortal_Base2D *, CPortal_Base2D * );  // line 43
	void PushEntitiesAtPoint( const Vector &, const QAngle &, CPortal_Base2D * );  // line 44
	void PushPaintBlobsAtPoint( const Vector &, const QAngle &, CPortal_Base2D * );  // line 45
	void PushEntity( CBaseEntity *, const Vector & );  // line 46
	void CreatePortalVortexEffect( const Vector &, CPortal_Base2D * );  // line 48
	bool TestWithinCone( const Vector &, const Vector &, const QAngle &, float );  // line 50
	CNetworkVar( bool, m_bEnabled ); // +0x3c0  // line 52
	CNetworkVar( float, m_flMagnitude ); // +0x3c4  // line 53
	CNetworkVar( float, m_flRadius ); // +0x3c8  // line 54
	CNetworkVar( float, m_flInnerRadius ); // +0x3cc  // line 55
	CNetworkVar( float, m_flConeOfInfluence ); // +0x3d0  // line 56
};

// game/server/portal2/portal_pointpush.h:23 (declaration)
void CPortal_PointPush();

// game/server/portal2/portal_pointpush.h:23 (declaration)
~CPortal_PointPush();

// game/server/portal2/portal_pointpush.h:23 @0x746630 _ZN17CPortal_PointPushD0Ev
CPortal_PointPush::~CPortal_PointPush()
{
}

// game/server/portal2/portal_pointpush.h:23 @0x746670 _ZN17CPortal_PointPushD1Ev
CPortal_PointPush::~CPortal_PointPush()
{
}
