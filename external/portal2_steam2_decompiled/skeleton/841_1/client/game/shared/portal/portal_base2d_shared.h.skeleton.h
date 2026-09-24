// DWARF declaration skeleton for game/shared/portal/portal_base2d_shared.h
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/shared/portal/portal_base2d_shared.h:29 sizeof=0x4 (i386)
struct CPortal_Base2D_Shared
{
public:
	static void UpdatePortalTransformationMatrix( const matrix3x4_t &, const matrix3x4_t &, VMatrix * );  // line 31
	static bool IsEntityTeleportable( C_BaseEntity * );  // line 33
	static CUtlVector<C_Portal_Base2D*,CUtlMemory<C_Portal_Base2D*, int> > AllPortals;  // line 36
protected:
	bool ShouldTeleportTouchingEntity( C_BaseEntity * );  // line 43
	void TeleportTouchingEntity( C_BaseEntity * );  // line 44
	void UpdateCollisionShape();  // line 47
	CPhysCollide *m_pCollisionShape; // +0x0  // line 48
};
