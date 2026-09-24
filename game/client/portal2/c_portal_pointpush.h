//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of the portal point push; predicts pushing paint blobs
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef C_PORTAL_POINTPUSH_H
#define C_PORTAL_POINTPUSH_H
#ifdef _WIN32
#pragma once
#endif

class C_Portal_Base2D;

class C_Portal_PointPush : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_Portal_PointPush, C_BaseEntity );
	DECLARE_CLIENTCLASS();

	C_Portal_PointPush();

	virtual void Spawn( void );
	virtual void OnDataChanged( DataUpdateType_t type );
	virtual void ClientThink( void );

private:

	void PushThink( void );

	void PushAtPoint( const Vector &vecOrigin, const QAngle &vecAngles, C_Portal_Base2D *pPortalFrom, C_Portal_Base2D *pPortalTo );
	void PushPaintBlobsAtPoint( const Vector &vecOrigin, const QAngle &vecAngles, C_Portal_Base2D *pPortal );

	bool TestWithinCone( const Vector &vecOrigin, const Vector &vecEndPos, const QAngle &vecAngles, float flMinAngle );

	bool m_bEnabled;
	float m_flMagnitude;
	float m_flRadius;
	float m_flInnerRadius;
	float m_flConeOfInfluence;
};

// The shared implementation (portal_pointpush_shared.cpp) uses the server name
#define CPortal_PointPush C_Portal_PointPush

#endif // C_PORTAL_POINTPUSH_H
