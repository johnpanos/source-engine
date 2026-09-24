// DWARF declaration skeleton for game/server/portal2/paint_bot_temp.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xad070 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
}

// game/server/portal2/paint_bot_temp.cpp:30
ConVar bot_forcefireweapon;

// game/server/portal2/paint_bot_temp.cpp:31
ConVar bot_forceattack2;

// game/server/portal2/paint_bot_temp.cpp:32
ConVar bot_forceattackon;

// game/server/portal2/paint_bot_temp.cpp:33
ConVar bot_flipout;

// game/server/portal2/paint_bot_temp.cpp:34
ConVar bot_defend;

// game/server/portal2/paint_bot_temp.cpp:35
ConVar bot_dontmove;

// game/server/portal2/paint_bot_temp.cpp:36
static ConVar bot_mimic;

// game/server/portal2/paint_bot_temp.cpp:37
static ConVar bot_mimic_yaw_offset;

// game/server/portal2/paint_bot_temp.cpp:38
ConVar bot_selectweaponslot;

// game/server/portal2/paint_bot_temp.cpp:39
ConVar bot_selectweaponsubtype;

// game/server/portal2/paint_bot_temp.cpp:40
ConVar bot_randomnames;

// game/server/portal2/paint_bot_temp.cpp:41
ConVar bot_jump;

// game/server/portal2/paint_bot_temp.cpp:42
ConVar bot_requestswap;

// game/server/portal2/paint_bot_temp.cpp:43
ConVar bot_follow;

// game/server/portal2/paint_bot_temp.cpp:44
ConVar bot_following_distance;

// game/server/portal2/paint_bot_temp.cpp:45
ConVar bot_look;

// game/server/portal2/paint_bot_temp.cpp:46
ConVar bot_throw;

// game/server/portal2/paint_bot_temp.cpp:48 @0x72f810 _ZL19cc_bot_selectweaponRK8CCommand
cc_bot_selectweapon( const CCommand &args )
{
	CBasePlayer *pBot;  // line 57
	CBaseCombatWeapon *pWpn;  // line 64
	// inlined CCommand::operator[]() at line 64
	// inlined CCommand::operator[]() at line 60
	// inlined CCommand::operator[]() at line 72
	// inlined CCommand::operator[]() at line 72
}

// game/server/portal2/paint_bot_temp.cpp:75
ConCommand bot_selectweapon;

// game/server/portal2/paint_bot_temp.cpp:77
static int BotNumber;

// game/server/portal2/paint_bot_temp.cpp:80 sizeof=0x34 (i386)
struct $_361
{
public:
	bool backwards; // +0x0  // line 81
	float nextturntime; // +0x4  // line 83
	bool lastturntoright; // +0x8  // line 84
	float nextstrafetime; // +0xc  // line 86
	float sidemove; // +0x10  // line 87
	QAngle forwardAngle; // +0x14  // line 89
	QAngle lastAngles; // +0x20  // line 90
	bool m_bChoseWeapon; // +0x2c  // line 92
	bool m_bWasDead; // +0x2d  // line 94
	float m_flDeadTime; // +0x30  // line 95
};

// game/server/portal2/paint_bot_temp.cpp:96
typedef $_361 botdata_t;

// game/server/portal2/paint_bot_temp.cpp:98
static botdata_t g_BotData[33];

// game/server/portal2/paint_bot_temp.cpp:105 @0x72f4f0 _Z14BotPutInServerbPKc
CBasePlayer *BotPutInServer( bool bFrozen, const char *pszCustomName )
{
	char botname[64];  // line 107
	edict_t *pEdict;  // line 129
	CPortal_Player *pPlayer;  // line 138
	botdata_t *pBot;  // line 147
	// inlined CBaseEntity::Instance() at line 138
	// inlined CBaseEntity::AddEFlags() at line 143
	// inlined CBaseEntity::entindex() at line 147
}

// game/server/portal2/paint_bot_temp.cpp:156 @0x72f720 _ZL3botRK8CCommand
bot( const CCommand &args )
{
	bool bFrozen;  // line 163
	const char *pName;  // line 165
}

// game/server/portal2/paint_bot_temp.cpp:156
static ConCommand bot_command;

// game/server/portal2/paint_bot_temp.cpp:174 @0x731980 _Z10Bot_RunAllv
Bot_RunAll()
{
	{
		int i;  // line 176
		{
			CPortal_Player *pPlayer;  // line 178
			// inlined ToPortalPlayer() at line 178
		}
	}
}

// game/server/portal2/paint_bot_temp.cpp:187 (declaration)
bool RunMimicCommand( CUserCmd &cmd );

// game/server/portal2/paint_bot_temp.cpp:187 @0x72fde0 _Z15RunMimicCommandR8CUserCmd
bool RunMimicCommand( CUserCmd &cmd )
{
	CBasePlayer *pPlayer;  // line 196
	// inlined ConVar::GetInt() at line 189
	// inlined CUserCmd::operator=() at line 203
}

// game/server/portal2/paint_bot_temp.cpp:220 (declaration)
void RunPlayerMove( CPortal_Player *fakeclient, const QAngle &viewangles, float forwardmove, float sidemove, float upmove, short unsigned int buttons, uint8 impulse, float frametime );

// game/server/portal2/paint_bot_temp.cpp:271 @0x72ff40 _Z9Bot_ThinkP14CPortal_Player
Bot_Think( CPortal_Player *pBot )
{
	botdata_t *botdata;  // line 276
	QAngle vecViewAngles;  // line 278
	float forwardmove;  // line 279
	float sidemove;  // line 280
	float upmove;  // line 281
	short unsigned int buttons;  // line 282
	uint8 impulse;  // line 283
	float frametime;  // line 284
	CMDLCacheCriticalSection cacheCriticalSection;  // line 288
	CBasePlayer *pPlayer;  // line 499
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 541
	// inlined RunPlayerMove() at line 541
	{
		Vector dir;  // line 522
		float dist;  // line 523
		{
			float speed;  // line 527
			Vector forward;  // line 529
			Vector right;  // line 529
			Vector up;  // line 529
			// inlined CBasePlayer::MaxSpeed() at line 527
			// inlined Vector::Normalized() at line 532
			// inlined Vector::Normalized() at line 533
			// inlined Vector::Normalized() at line 534
		}
		// inlined Vector::NormalizeInPlace() at line 524
		// inlined Vector::Length() at line 523
		// inlined Vector::operator-() at line 522
		// inlined CBaseEntity::GetAbsOrigin() at line 522
		// inlined CBaseEntity::GetAbsOrigin() at line 522
	}
	{
		Vector forward;  // line 502
		Ray_t ray;  // line 504
		trace_t tr;  // line 506
		// inlined Vector::operator=() at line 510
		// inlined Vector::operator-() at line 510
		// inlined UTIL_TraceRay() at line 507
		// inlined Ray_t::Init() at line 505
		// inlined Vector::operator+() at line 505
		// inlined operator*() at line 505
		// inlined Ray_t::Ray_t() at line 504
	}
	// inlined CBaseEntity::entindex() at line 499
	{
		float speed;  // line 479
		// inlined CBaseEntity::entindex() at line 476
		// inlined CBaseEntity::entindex() at line 480
		// inlined QAngle::operator=() at line 484
		// inlined CBaseEntity::entindex() at line 493
	}
	// inlined CBaseEntity::GetSolid() at line 290
	// inlined CMDLCacheCriticalSection::CMDLCacheCriticalSection() at line 288
	// inlined QAngle::operator=() at line 286
	// inlined ENTINDEX() at line 276
	// inlined CBaseEntity::edict() at line 276
	{
		QAngle angOffset;  // line 450
		// inlined QAngle::operator=() at line 472
		{
			int i;  // line 454
			// inlined QAngle::operator[]() at line 456
		}
		// inlined QAngle::operator+=() at line 452
		// inlined RandomAngle() at line 450
	}
	{
		trace_t trace;  // line 292
		int iSubType;  // line 334
		// inlined ConVar::GetInt() at line 417
		// inlined ConVar::GetInt() at line 320
		{
			int slot;  // line 322
			CBaseCombatWeapon *pWpn;  // line 324
		}
		// inlined ConVar::GetString() at line 348
		{
			CBaseCombatWeapon *pWeapon;  // line 351
			const char *pszWeapon;  // line 352
			{
				CBaseCombatWeapon *pActiveWeapon;  // line 392
			}
			{
				int i;  // line 379
				// inlined FClassnameIs() at line 381
			}
			// inlined ConVar::GetString() at line 352
			{
				CUtlVector<CBaseCombatWeapon*,CUtlMemory<CBaseCombatWeapon*, int> > weapons;  // line 361
				// inlined CUtlVector<CBaseCombatWeapon*,CUtlMemory<CBaseCombatWeapon*, int> >::Count() at line 370
				{
					int i;  // line 362
					// inlined CUtlVector<CBaseCombatWeapon*,CUtlMemory<CBaseCombatWeapon*, int> >::AddToTail() at line 366
				}
				// inlined CUtlVector<CBaseCombatWeapon*,CUtlMemory<CBaseCombatWeapon*, int> >::CUtlVector() at line 361
				// inlined CUtlVector<CBaseCombatWeapon*,CUtlMemory<CBaseCombatWeapon*, int> >::~CUtlVector() at line 373
				// inlined CUtlVector<CBaseCombatWeapon*,CUtlMemory<CBaseCombatWeapon*, int> >::~CUtlVector() at line 373
			}
		}
		{
			CWeaponPaintGun *pPaintGun;  // line 337
		}
	}
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 541
}

// game/server/portal2/paint_bot_temp.cpp:547 @0x72fc40 _Z18cc_bot_sendcommandRK8CCommand
cc_bot_sendcommand( const CCommand &args )
{
	CBasePlayer *pPlayer;  // line 556
	const char *commandline;  // line 562
	int iSize;  // line 568
	char *pBuf;  // line 569
	CCommand command;  // line 578
	// inlined CCommand::GetCommandString() at line 562
	// inlined CCommand::operator[]() at line 565
	// inlined CCommand::operator[]() at line 559
}

// game/server/portal2/paint_bot_temp.cpp:583
static ConCommand bot_sendcommand;

// game/server/portal2/paint_bot_temp.cpp:588 @0x72f780 _Z11cc_bot_killRK8CCommand
cc_bot_kill( const CCommand &args )
{
	CBasePlayer *pPlayer;  // line 591
	// inlined CCommand::operator[]() at line 591
	// inlined CCommand::operator[]() at line 599
}

// game/server/portal2/paint_bot_temp.cpp:602
static ConCommand bot_kill;

// game/server/portal2/paint_bot_temp.cpp:606 @0x72f4e0 _ZL10bot_refillRK8CCommand
bot_refill( const CCommand &args )
{
}

// game/server/portal2/paint_bot_temp.cpp:606
static ConCommand bot_refill_command;

// game/server/portal2/paint_bot_temp.cpp:613 @0x72fab0 _ZL12bot_teleportRK8CCommand
bot_teleport( const CCommand &args )
{
	CBasePlayer *pBot;  // line 622
	Vector vecPos;  // line 629
	QAngle vecAng;  // line 630
	// inlined CCommand::operator[]() at line 629
	// inlined CCommand::operator[]() at line 629
	// inlined CCommand::operator[]() at line 629
	// inlined Vector::Vector() at line 629
	// inlined CCommand::operator[]() at line 630
	// inlined CCommand::operator[]() at line 630
	// inlined CCommand::operator[]() at line 630
	// inlined QAngle::QAngle() at line 630
	// inlined CCommand::operator[]() at line 625
}

// game/server/portal2/paint_bot_temp.cpp:613
static ConCommand bot_teleport_command;

// game/server/portal2/paint_bot_temp.cpp:635 @0x72f920 _Z12cc_bot_equipRK8CCommand
cc_bot_equip( const CCommand &args )
{
	CBasePlayer *pBot;  // line 644
	CBaseEntity *pExisting;  // line 651
	// inlined CCommand::operator[]() at line 654
	// inlined CCommand::operator[]() at line 654
	// inlined CCommand::operator[]() at line 651
	// inlined CCommand::operator[]() at line 647
	{
		CBaseCombatWeapon *pWpn;  // line 658
		// inlined CCommand::operator[]() at line 658
		// inlined CCommand::operator[]() at line 665
	}
}

// game/server/portal2/paint_bot_temp.cpp:669
static ConCommand bot_equip;

// public/tier1/utlmemory.h:707 @0x731a00 _ZN10CUtlMemoryIP17CBaseCombatWeaponiE4GrowEi
void CUtlMemory<CBaseCombatWeapon*,int>::Grow( int num )
{
	int nAllocationRequested;  // line 720
	int nNewAllocationCount;  // line 724
	// inlined UtlMemory_CalcNewAllocationCount() at line 724
	// inlined CUtlMemory<CBaseCombatWeapon*,int>::IsExternallyAllocated() at line 711
	// inlined MemAlloc_Alloc() at line 761
}
