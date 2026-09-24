// DWARF declaration skeleton for game/shared/portal2/portal_pointpush_shared.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x1c9d0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
}

// game/shared/portal2/portal_pointpush_shared.cpp:22
ConVar portal_pointpush_think_rate;

// game/shared/portal2/portal_pointpush_shared.cpp:23
ConVar portal_pointpush_debug;

// game/shared/portal2/portal_pointpush_shared.cpp:26 @0x258cb0 _ZN17CPortal_PointPush9PushThinkEv
void CPortal_PointPush::PushThink()
{
	PortalRadiusExtensionVector portalRadiusExtensions;  // line 29
	{
		int i;  // line 32
		// inlined CPortal_PointPush::PushAtPoint() at line 34
		// inlined CUtlVector<PortalRadiusExtension_t,CUtlMemory<PortalRadiusExtension_t, int> >::operator[]() at line 34
	}
	// inlined CUtlVector<PortalRadiusExtension_t,CUtlMemory<PortalRadiusExtension_t, int> >::~CUtlVector() at line 42
	// inlined CBaseEntity::GetAbsAngles() at line 30
	// inlined CUtlVector<PortalRadiusExtension_t,CUtlMemory<PortalRadiusExtension_t, int> >::CUtlVector() at line 29
	// inlined CBaseEntity::GetAbsOrigin() at line 30
	// inlined CUtlVector<PortalRadiusExtension_t,CUtlMemory<PortalRadiusExtension_t, int> >::~CUtlVector() at line 42
}

// game/shared/portal2/portal_pointpush_shared.cpp:47 (declaration)
void PushAtPoint( const Vector &vecOrigin, const QAngle &vecAngles, CPortal_Base2D *pPortalFrom, CPortal_Base2D *pPortalTo );

// game/shared/portal2/portal_pointpush_shared.cpp:78 (declaration)
void TestWithinCone( const Vector &vecOrigin, const Vector &vecEndPos, const QAngle &vecAngles, float flMinAngle );

// game/shared/portal2/portal_pointpush_shared.cpp:78 @0x258740 _ZN17CPortal_PointPush14TestWithinConeERK6VectorS2_RK6QAnglef
bool CPortal_PointPush::TestWithinCone( const Vector &vecOrigin, const Vector &vecEndPos, const QAngle &vecAngles, float flMinAngle )
{
	Vector vecDirToTarget;  // line 81
	Vector vecDirection;  // line 84
	float flDot;  // line 88
	// inlined Vector::operator-() at line 81
}

// game/shared/portal2/portal_pointpush_shared.cpp:98 @0x2587e0 _ZN17CPortal_PointPush21PushPaintBlobsAtPointERK6VectorRK6QAngleP14CPortal_Base2D
void CPortal_PointPush::PushPaintBlobsAtPoint( const Vector &vecOrigin, const QAngle &vecAngles, CPortal_Base2D *pPortal )
{
	float flRadiusSqr;  // line 100
	Vector vecPushPoint;  // line 101
	{
		int i;  // line 104
		{
			CPaintStream *pStream;  // line 107
			int numBlobs;  // line 115
			{
				int j;  // line 116
				{
					CBasePaintBlob *pBlob;  // line 118
					Vector vecBlobPos;  // line 128
					float flDistSqr;  // line 129
					Vector vecOldBlobVelocity;  // line 151
					Vector vecPushDir;  // line 152
					float flDist;  // line 153
					float flFalloff;  // line 155
					Vector vecPushVelocity;  // line 157
					// inlined Vector::operator+() at line 158
					// inlined Vector::operator*() at line 157
					// inlined operator*() at line 157
					// inlined Vector::operator-() at line 152
					// inlined CBaseEntity::GetAbsAngles() at line 146
					// inlined Vector::DistToSqr() at line 129
					// inlined Vector::operator VectorByValue&() at line 158
					// inlined CPortal_PointPush::TestWithinCone() at line 146
					{
						float flMinAngle;  // line 138
						// inlined CPortal_PointPush::TestWithinCone() at line 139
					}
					// inlined CBaseEntity::GetAbsOrigin() at line 146
				}
			}
		}
	}
}
