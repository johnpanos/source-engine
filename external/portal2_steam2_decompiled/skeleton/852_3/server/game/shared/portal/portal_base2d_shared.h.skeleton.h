// DWARF declaration skeleton for game/shared/portal/portal_base2d_shared.h
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/shared/portal/portal_base2d_shared.h:29 sizeof=0x4 (i386)
struct CPortal_Base2D_Shared
{
public:
	static void UpdatePortalTransformationMatrix( const matrix3x4_t &, const matrix3x4_t &, VMatrix * );  // line 31
	static bool IsEntityTeleportable( CBaseEntity * );  // line 33
	static CUtlVector<CPortal_Base2D*,CUtlMemory<CPortal_Base2D*, int> > AllPortals;  // line 38
protected:
	bool ShouldTeleportTouchingEntity( CBaseEntity * );  // line 43
	void TeleportTouchingEntity( CBaseEntity * );  // line 44
	void UpdateCollisionShape();  // line 47
	CPhysCollide *m_pCollisionShape; // +0x0  // line 48
};
