// DWARF declaration skeleton for game/client/portal2/c_portal_pointpush.h
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal2/c_portal_pointpush.h:8 sizeof=0xa70 (i386)
struct C_Portal_PointPush : public C_BaseEntity
{
public:
	C_Portal_PointPush();  // line 13
	virtual void Spawn();  // line 15
	virtual void OnDataChanged( DataUpdateType_t );  // line 16
	virtual void ClientThink();  // line 17
private:
	void PushThink();  // line 21
	void PushAtPoint( const Vector &, const QAngle &, C_Portal_Base2D *, C_Portal_Base2D * );  // line 23
	void PushPaintBlobsAtPoint( const Vector &, const QAngle &, C_Portal_Base2D * );  // line 24
	bool TestWithinCone( const Vector &, const Vector &, const QAngle &, float );  // line 26
	bool m_bEnabled; // +0xa5d  // line 28
	float m_flMagnitude; // +0xa60  // line 29
	float m_flRadius; // +0xa64  // line 30
	float m_flInnerRadius; // +0xa68  // line 31
	float m_flConeOfInfluence; // +0xa6c  // line 32
};

// game/client/portal2/c_portal_pointpush.h:8 (declaration)
~C_Portal_PointPush();

// game/client/portal2/c_portal_pointpush.h:8 @0x52da20 _ZN18C_Portal_PointPushD0Ev
C_Portal_PointPush::~C_Portal_PointPush()
{
}

// game/client/portal2/c_portal_pointpush.h:8 @0x52da90 _ZN18C_Portal_PointPushD1Ev
C_Portal_PointPush::~C_Portal_PointPush()
{
}
