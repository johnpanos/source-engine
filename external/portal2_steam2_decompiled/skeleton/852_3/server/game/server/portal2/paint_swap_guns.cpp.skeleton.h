// DWARF declaration skeleton for game/server/portal2/paint_swap_guns.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xc3c80 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSteamID::CSteamID() at line 650
	// inlined CSteamID::CSteamID() at line 654
	// inlined CSteamID::CSteamID() at line 656
	// inlined CSteamID::CSteamID() at line 659
	// inlined CSteamID::CSteamID() at line 662
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
}

// game/server/portal2/paint_swap_guns.cpp:15
ConVar sv_max_swap_distance;

// game/server/portal2/paint_swap_guns.cpp:16
ConVar sv_max_swap_look_angle;

// game/server/portal2/paint_swap_guns.cpp:17
ConVar sv_max_swap_portal_look_angle;

// game/server/portal2/paint_swap_guns.cpp:18
ConVar g_debug_swap_guns_trace;

// game/server/portal2/paint_swap_guns.cpp:22
enum SwapGunTraceResult
{
	SWAP_TRACE_HIT_NOTHING = 0,
	SWAP_TRACE_HIT_SOMETHING = 1,
};

// game/server/portal2/paint_swap_guns.cpp:29 @0x6d13c0 _Z17CheckSwapPaintGunP17CBaseCombatWeapon
CheckSwapPaintGun( CBaseCombatWeapon *pWeapon )
{
	CWeaponPaintGun *pPaintGun;  // line 31
	// inlined ToBasePlayer() at line 34
}

// game/server/portal2/paint_swap_guns.cpp:39 @0x6d1440 _Z22SwapPaintAndPortalGunsP14CPortal_PlayerS0_
SwapPaintAndPortalGuns( CPortal_Player *pPlayer1, CPortal_Player *pPlayer2 )
{
	CBaseCombatWeapon *pWeapon1;  // line 41
	CBaseCombatWeapon *pWeapon2;  // line 42
	CWeaponPortalgun *pPortalGun;  // line 44
	char linkageID;  // line 55
	// inlined CWeaponPortalgun::GetLinkageGroupID() at line 55
	// inlined FClassnameIs() at line 46
	// inlined FClassnameIs() at line 50
}

// game/server/portal2/paint_swap_guns.cpp:76 @0x6d1680 _Z17SwapGunsTraceLineRK6VectorS1_P11CBaseEntityP10CGameTrace
SwapGunTraceResult SwapGunsTraceLine( const Vector &vecStartPos, const Vector &vecEndPos, CBaseEntity *pStartEnt, trace_t *trace )
{
	CTraceFilterSimpleList filter;  // line 78
	CBaseEntity *list[1024];  // line 92
	Ray_t ray;  // line 94
	int nCount;  // line 97
	// inlined CTraceFilterSimpleList::~CTraceFilterSimpleList() at line 118
	// inlined CTraceFilterSimpleList::~CTraceFilterSimpleList() at line 118
	// inlined CGameTrace::DidHit() at line 112
	{
		int i;  // line 99
		// inlined FClassnameIs() at line 101
		// inlined FClassnameIs() at line 105
	}
	// inlined UTIL_EntitiesAlongRay() at line 97
	// inlined Ray_t::Init() at line 95
	// inlined UTIL_TraceLine() at line 81
}

// game/server/portal2/paint_swap_guns.cpp:122 @0x6d1bd0 _Z23CheckSwapThroughPortalsP14CPortal_PlayerP12CProp_PortalS0_S2_
bool CheckSwapThroughPortals( CPortal_Player *pPlayer1, CProp_Portal *pPlayer1Portal, CPortal_Player *pPlayer2, CProp_Portal *pPlayer2Portal )
{
	trace_t tr;  // line 125
	SwapGunTraceResult result;  // line 126
	{
		trace_t tr2;  // line 132
		// inlined CBaseEntity::GetAbsOrigin() at line 133
	}
	// inlined CBaseEntity::GetAbsOrigin() at line 126
}

// game/server/portal2/paint_swap_guns.cpp:146 (declaration)
bool CanSeePositionThroughPortal( const Vector &vecStartPos, const Vector &vecForwardDir, const Vector &vecEndPos, CProp_Portal *pPortal );

// game/server/portal2/paint_swap_guns.cpp:146 @0x6d1290 _Z27CanSeePositionThroughPortalRK6VectorS1_S1_P12CProp_Portal
bool CanSeePositionThroughPortal( const Vector &vecStartPos, const Vector &vecForwardDir, const Vector &vecEndPos, CProp_Portal *pPortal )
{
	Vector vecNewEndPos;  // line 148
	Vector vecNewEndDir;  // line 151
	float flAngleDiff;  // line 154
	// inlined Vector::operator-() at line 151
	// inlined CNetworkHandleBase<CPortal_Base2D,CPortal_Base2D::NetworkVar_m_hLinkedPortal>::operator->() at line 149
}

// game/server/portal2/paint_swap_guns.cpp:160 @0x6d1cf0 _Z18CheckSwapProximityP14CPortal_PlayerS0_
bool CheckSwapProximity( CPortal_Player *pPlayer1, CPortal_Player *pPlayer2 )
{
	Vector vecPaintGunPlayerPos;  // line 163
	Vector vecPortalGunPlayerPos;  // line 164
	Vector vecPaintPlayerForward;  // line 167
	Vector vecPortalPlayerForward;  // line 167
	Vector vecStraightLineDir;  // line 174
	float flStraightLineDistance;  // line 175
	float flMaxSwapDistance;  // line 179
	CWeaponPortalgun *pPortalGun;  // line 204
	CProp_Portal *pPortal1;  // line 210
	CProp_Portal *pPortal2;  // line 211
	{
		Vector vecPortal1PaintGunPlayerDir;  // line 216
		Vector vecPortal2PaintGunPlayerDir;  // line 218
		float flPaintPlayerPortal1Angle;  // line 221
		float flPaintPlayerPortal2Angle;  // line 222
		Vector vecPortal1PortalGunPlayerDir;  // line 225
		Vector vecPortal2PortalGunPlayerDir;  // line 227
		float flPortalPlayerPortal1Angle;  // line 230
		float flPortalPlayerPortal2Angle;  // line 231
		{
			bool bCanSeeOtherPlayer;  // line 248
			float flThroughPortal2Distance;  // line 251
			// inlined CanSeePositionThroughPortal() at line 248
		}
		// inlined ConVar::GetFloat() at line 234
		// inlined DotProduct() at line 231
		// inlined DotProduct() at line 230
		// inlined Vector::operator-() at line 227
		// inlined CBaseEntity::GetAbsOrigin() at line 227
		// inlined Vector::operator-() at line 225
		// inlined CBaseEntity::GetAbsOrigin() at line 225
		// inlined DotProduct() at line 222
		// inlined DotProduct() at line 221
		// inlined Vector::operator-() at line 218
		// inlined CBaseEntity::GetAbsOrigin() at line 218
		// inlined Vector::operator-() at line 216
		// inlined CBaseEntity::GetAbsOrigin() at line 216
		{
			bool bCanSeeOtherPlayer;  // line 236
			float flThroughPortal1Distance;  // line 239
			// inlined CanSeePositionThroughPortal() at line 236
		}
	}
	{
		float flPaintPlayerAngleDiff;  // line 185
		float flPortalPlayerAngleDiff;  // line 186
		// inlined ConVar::GetFloat() at line 189
		// inlined DotProduct() at line 186
		// inlined Vector::operator-() at line 186
		// inlined DotProduct() at line 185
		{
			trace_t trace;  // line 192
			SwapGunTraceResult result;  // line 193
		}
	}
	// inlined Vector::LengthSqr() at line 175
	// inlined Vector::operator-() at line 174
}

// game/server/portal2/paint_swap_guns.cpp:263 @0x6d1600 _ZL26swap_paint_and_portal_gunsRK8CCommand
swap_paint_and_portal_guns( const CCommand &args )
{
	CPortal_Player *pPlayer1;  // line 273
	CPortal_Player *pPlayer2;  // line 274
	// inlined GetPortalPlayer() at line 273
	// inlined GetPortalPlayer() at line 274
}

// game/server/portal2/paint_swap_guns.cpp:263
static ConCommand swap_paint_and_portal_guns_command;
