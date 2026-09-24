// DWARF declaration skeleton for game/shared/portal2/portal_pointpush_shared.cpp
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x16740 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
}

// game/shared/portal2/portal_pointpush_shared.cpp:24
ConVar portal_pointpush_think_rate;

// game/shared/portal2/portal_pointpush_shared.cpp:25
ConVar portal_pointpush_debug;

// game/shared/portal2/portal_pointpush_shared.cpp:28 @0x1bdb40 _ZN18C_Portal_PointPush9PushThinkEv
void C_Portal_PointPush::PushThink()
{
	PortalRadiusExtensionVector portalRadiusExtensions;  // line 31
	// inlined CUtlVector<PortalRadiusExtension_t,CUtlMemory<PortalRadiusExtension_t, int> >::~CUtlVector() at line 41
	{
		int i;  // line 34
		// inlined CUtlVector<PortalRadiusExtension_t,CUtlMemory<PortalRadiusExtension_t, int> >::operator[]() at line 36
		// inlined C_Portal_PointPush::PushAtPoint() at line 36
	}
	// inlined CUtlVector<PortalRadiusExtension_t,CUtlMemory<PortalRadiusExtension_t, int> >::CUtlVector() at line 31
	// inlined CUtlVector<PortalRadiusExtension_t,CUtlMemory<PortalRadiusExtension_t, int> >::~CUtlVector() at line 41
}

// game/shared/portal2/portal_pointpush_shared.cpp:49 (declaration)
void PushAtPoint( const Vector &vecOrigin, const QAngle &vecAngles, C_Portal_Base2D *pPortalFrom, C_Portal_Base2D *pPortalTo );

// game/shared/portal2/portal_pointpush_shared.cpp:80 (declaration)
void TestWithinCone( const Vector &vecOrigin, const Vector &vecEndPos, const QAngle &vecAngles, float flMinAngle );

// game/shared/portal2/portal_pointpush_shared.cpp:80 @0x1bd630 _ZN18C_Portal_PointPush14TestWithinConeERK6VectorS2_RK6QAnglef
bool C_Portal_PointPush::TestWithinCone( const Vector &vecOrigin, const Vector &vecEndPos, const QAngle &vecAngles, float flMinAngle )
{
	Vector vecDirToTarget;  // line 83
	Vector vecDirection;  // line 86
	float flDot;  // line 90
	// inlined Vector::operator-() at line 83
}

// game/shared/portal2/portal_pointpush_shared.cpp:100 @0x1bd6d0 _ZN18C_Portal_PointPush21PushPaintBlobsAtPointERK6VectorRK6QAngleP15C_Portal_Base2D
void C_Portal_PointPush::PushPaintBlobsAtPoint( const Vector &vecOrigin, const QAngle &vecAngles, C_Portal_Base2D *pPortal )
{
	float flRadiusSqr;  // line 102
	Vector vecPushPoint;  // line 103
	{
		int i;  // line 106
		{
			C_PaintStream *pStream;  // line 109
			int numBlobs;  // line 117
			{
				int j;  // line 118
				{
					CBasePaintBlob *pBlob;  // line 120
					Vector vecBlobPos;  // line 130
					float flDistSqr;  // line 131
					Vector vecOldBlobVelocity;  // line 153
					Vector vecPushDir;  // line 154
					float flDist;  // line 155
					float flFalloff;  // line 157
					Vector vecPushVelocity;  // line 159
					// inlined Vector::operator+() at line 160
					// inlined Vector::operator*() at line 159
					// inlined operator*() at line 159
					// inlined Vector::operator-() at line 154
					// inlined Vector::DistToSqr() at line 131
					// inlined Vector::operator VectorByValue&() at line 160
					// inlined C_Portal_PointPush::TestWithinCone() at line 148
					{
						float flMinAngle;  // line 140
						// inlined C_Portal_PointPush::TestWithinCone() at line 141
					}
				}
			}
		}
	}
}
