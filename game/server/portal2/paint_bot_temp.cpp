//========= Portal 2 reconstruction ============================================//
//
// Purpose: Basic test bots for the paint build.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "paint_bot_temp.h"
#include "player.h"
#include "portal_player.h"
#include "weapon_paintgun.h"
#include "in_buttons.h"
#include "movehelper_server.h"
#include "gamerules.h"
#include "serverbenchmark_base.h"
#include "datacache/imdlcache.h"
#include "debugoverlay_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void Bot_Think( CPortal_Player *pBot );


ConVar bot_forcefireweapon( "bot_forcefireweapon", "", 0, "Force bots with the specified weapon to fire." );
ConVar bot_forceattack2( "bot_forceattack2", "0", 0, "When firing, use attack2." );
ConVar bot_forceattackon( "bot_forceattackon", "0", 0, "When firing, don't tap fire, hold it down." );
ConVar bot_flipout( "bot_flipout", "0", 0, "When on, all bots fire their guns." );
ConVar bot_defend( "bot_defend", "0", 0, "Set to a team number, and that team will all keep their combat shields raised." );
ConVar bot_dontmove( "bot_dontmove", "0", FCVAR_CHEAT );
static ConVar bot_mimic( "bot_mimic", "0", 0, "Bot uses usercmd of player by index." );
static ConVar bot_mimic_yaw_offset( "bot_mimic_yaw_offset", "180", 0, "Offsets the bot yaw." );
ConVar bot_selectweaponslot( "bot_selectweaponslot", "-1", FCVAR_CHEAT, "set to weapon slot that bot should switch to." );
ConVar bot_selectweaponsubtype( "bot_selectweaponsubtype", "-1", FCVAR_CHEAT, "set to weapon subtype that bot should switch to." );
ConVar bot_randomnames( "bot_randomnames", "0", FCVAR_CHEAT );
ConVar bot_jump( "bot_jump", "0", FCVAR_CHEAT, "Force all bots to repeatedly jump." );
ConVar bot_requestswap( "bot_requestswap", "0", FCVAR_CHEAT, "Force bot to request swap weapon from player." );
ConVar bot_follow( "bot_follow", "0", FCVAR_CHEAT, "When on, bot will follow the player." );
ConVar bot_following_distance( "bot_following_distance", "200.f", FCVAR_CHEAT, "distance that bot will follow the player." );
ConVar bot_look( "bot_look", "0", FCVAR_CHEAT, "When on, bot will look at what the player is looking." );
ConVar bot_throw( "bot_throw", "0", FCVAR_CHEAT, "When on, bot will throw current weapon." );

void cc_bot_selectweapon( const CCommand &args )
{
	if ( args.ArgC() < 3 )
	{
		Msg( "Too few parameters.  Usage: bot_selectweapon <bot name> <weapon name>\n" );
		return;
	}

	CBasePlayer *pBot = UTIL_PlayerByName( args[1] );
	if ( !pBot )
	{
		Msg( "No bot with name %s\n", args[1] );
		return;
	}

	CBaseCombatWeapon *pWpn = pBot->Weapon_OwnsThisType( args[2] );
	if ( pWpn )
	{
		pBot->Weapon_Switch( pWpn );
	}
	else
	{
		Msg( "%s doesn't have %s\n", args[1], args[2] );
	}
}

ConCommand bot_selectweapon( "bot_selectweapon", cc_bot_selectweapon, "", FCVAR_CHEAT );

static int BotNumber = 1;

typedef struct
{
	bool			backwards;

	float			nextturntime;
	bool			lastturntoright;

	float			nextstrafetime;
	float			sidemove;

	QAngle			forwardAngle;
	QAngle			lastAngles;

	bool			m_bChoseWeapon;

	bool			m_bWasDead;
	float			m_flDeadTime;
} botdata_t;

static botdata_t g_BotData[ MAX_PLAYERS ];


//-----------------------------------------------------------------------------
// Purpose: Create a new Bot and put it in the game.
// Output : Pointer to the new Bot, or NULL if there's no free clients.
//-----------------------------------------------------------------------------
CBasePlayer *BotPutInServer( bool bFrozen, const char *pszCustomName )
{
	char botname[ 64 ];
	if ( pszCustomName && pszCustomName[0] )
	{
		Q_strncpy( botname, pszCustomName, sizeof( botname ) );
	}
	else if ( bot_randomnames.GetBool() )
	{
		// Exercise the name handling with names of assorted lengths
		switch( g_pServerBenchmark->RandomInt( 0, 5 ) )
		{
		case 0:	Q_snprintf( botname, sizeof( botname ), "Bot%02i", BotNumber ); break;
		case 1:	Q_snprintf( botname, sizeof( botname ), "This is a medium Bot%02i", BotNumber ); break;
		case 2:	Q_snprintf( botname, sizeof( botname ), "This is a super long bot name that is too long for the game to allow%02i", BotNumber ); break;
		case 3:	Q_snprintf( botname, sizeof( botname ), "Another bot%02i", BotNumber ); break;
		case 4:	Q_snprintf( botname, sizeof( botname ), "Yet more Bot names, medium sized%02i", BotNumber ); break;
		default: Q_snprintf( botname, sizeof( botname ), "B%02i", BotNumber ); break;
		}
	}
	else
	{
		Q_snprintf( botname, sizeof( botname ), "Bot%02i", BotNumber );
	}

	edict_t *pEdict = engine->CreateFakeClient( botname );
	if ( !pEdict )
	{
		Msg( "Failed to create Bot.\n" );
		return NULL;
	}

	// Allocate a player entity for the bot, and call spawn
	CPortal_Player *pPlayer = ( (CPortal_Player *)CBaseEntity::Instance( pEdict ) );

	pPlayer->ClearFlags();
	pPlayer->AddFlag( FL_CLIENT | FL_FAKECLIENT );

	if ( bFrozen )
		pPlayer->AddEFlags( EFL_BOT_FROZEN );

	BotNumber++;

	botdata_t *pBot = &g_BotData[ pPlayer->entindex() - 1 ];
	pBot->m_bWasDead = false;
	pBot->m_bChoseWeapon = false;

	return pPlayer;
}

static void bot( const CCommand &args )
{
	// Look at -frozen.
	bool bFrozen = !!args.FindArg( "-frozen" );

	const char *pName = args.FindArg( "-name" );

	BotPutInServer( bFrozen, pName );
}
static ConCommand bot_command( "bot", bot, "Add a bot.", FCVAR_CHEAT );


//-----------------------------------------------------------------------------
// Purpose: Run through all the Bots in the game and let them think.
//-----------------------------------------------------------------------------
void Bot_RunAll( void )
{
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CPortal_Player *pPlayer = ToPortalPlayer( UTIL_PlayerByIndex( i ) );

		if ( pPlayer && ( pPlayer->GetFlags() & FL_FAKECLIENT ) )
		{
			Bot_Think( pPlayer );
		}
	}
}

bool RunMimicCommand( CUserCmd& cmd )
{
	if ( bot_mimic.GetInt() <= 0 )
		return false;

	if ( bot_mimic.GetInt() > gpGlobals->maxClients )
		return false;

	CBasePlayer *pPlayer = UTIL_PlayerByIndex( bot_mimic.GetInt() );
	if ( !pPlayer )
		return false;

	if ( !pPlayer->GetLastUserCommand() )
		return false;

	cmd = *pPlayer->GetLastUserCommand();
	cmd.viewangles[YAW] += bot_mimic_yaw_offset.GetFloat();
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Simulates a single frame of movement for a player
// Input  : *fakeclient -
//			*viewangles -
//			forwardmove -
//			sidemove -
//			upmove -
//			buttons -
//			impulse -
//			msec -
// Output : 	virtual void
//-----------------------------------------------------------------------------
static void RunPlayerMove( CPortal_Player *fakeclient, const QAngle& viewangles, float forwardmove, float sidemove, float upmove, unsigned short buttons, byte impulse, float frametime )
{
	if ( !fakeclient )
		return;

	CUserCmd cmd;

	// Store off the globals.. they're gonna get whacked
	float flOldFrametime = gpGlobals->frametime;
	float flOldCurtime = gpGlobals->curtime;

	float flTimeBase = gpGlobals->curtime + gpGlobals->frametime - frametime;
	fakeclient->SetTimeBase( flTimeBase );

	Q_memset( &cmd, 0, sizeof( cmd ) );

	if ( !RunMimicCommand( cmd ) )
	{
		VectorCopy( viewangles, cmd.viewangles );
		cmd.forwardmove = forwardmove;
		cmd.sidemove = sidemove;
		cmd.upmove = upmove;
		cmd.buttons = buttons;
		cmd.impulse = impulse;
		cmd.random_seed = g_pServerBenchmark->RandomInt( 0, 0x7fffffff );
	}

	if ( bot_dontmove.GetBool() )
	{
		cmd.forwardmove = 0;
		cmd.sidemove = 0;
		cmd.upmove = 0;
	}

	MoveHelperServer()->SetHost( fakeclient );
	fakeclient->PlayerRunCommand( &cmd, MoveHelperServer() );

	// save off the last good usercmd
	fakeclient->SetLastUserCommand( cmd );

	// Clear out any fixangle that has been set
	fakeclient->pl.fixangle = FIXANGLE_NONE;

	// Restore the globals..
	gpGlobals->frametime = flOldFrametime;
	gpGlobals->curtime = flOldCurtime;
}

//-----------------------------------------------------------------------------
// Purpose: Run this Bot's AI for one frame.
//-----------------------------------------------------------------------------
void Bot_Think( CPortal_Player *pBot )
{
	// Make sure we stay being a bot
	pBot->AddFlag( FL_FAKECLIENT );

	botdata_t *botdata = &g_BotData[ ENTINDEX( pBot->edict() ) - 1 ];

	QAngle vecViewAngles;
	float forwardmove = 0.0;
	float sidemove = botdata->sidemove;
	float upmove = 0.0;
	unsigned short buttons = 0;
	byte  impulse = 0;
	float frametime = gpGlobals->frametime;

	vecViewAngles = pBot->EyeAngles();

	MDLCACHE_CRITICAL_SECTION();

	if ( pBot->IsAlive() && ( pBot->GetSolid() == SOLID_BBOX ) )
	{
		trace_t trace;

		botdata->m_bWasDead = false;

		// Ask the other player to swap guns
		if ( bot_requestswap.GetBool() )
		{
			buttons |= IN_ALT1;
		}

		// Throw the current weapon (one shot)
		if ( bot_throw.GetBool() )
		{
			buttons |= IN_ALT2;
			bot_throw.SetValue( 0 );
		}

		if ( !pBot->IsEFlagSet( EFL_BOT_FROZEN ) && bot_jump.GetBool() && ( pBot->GetFlags() & FL_ONGROUND ) )
		{
			buttons |= IN_JUMP;
		}

		if ( bot_selectweaponslot.GetInt() >= 0 )
		{
			int slot = bot_selectweaponslot.GetInt();

			CBaseCombatWeapon *pWpn = pBot->Weapon_GetSlot( slot );

			if ( pWpn )
			{
				pBot->Weapon_Switch( pWpn );
			}

			bot_selectweaponslot.SetValue( -1 );
		}

		// Pick the paint on the paint gun
		int iSubType = bot_selectweaponsubtype.GetInt();
		if ( iSubType >= 0 && iSubType < PAINT_POWER_TYPE_COUNT_PLUS_NO_POWER )
		{
			CWeaponPaintGun *pPaintGun = dynamic_cast< CWeaponPaintGun* >( pBot->GetActiveWeapon() );
			if ( pPaintGun )
			{
				pPaintGun->SetCurrentPaint( (PaintPowerType)iSubType );
			}

			bot_selectweaponsubtype.SetValue( -1 );
		}

		// If bots are being forced to fire a weapon, see if I have it
		if ( bot_forcefireweapon.GetString() )
		{
			CBaseCombatWeapon *pWeapon = NULL;
			const char *pszWeapon = bot_forcefireweapon.GetString();

			if ( g_pServerBenchmark->IsBenchmarkRunning() )
			{
				// Switch to a random weapon once
				if ( !botdata->m_bChoseWeapon )
				{
					botdata->m_bChoseWeapon = true;

					CUtlVector<CBaseCombatWeapon*> weapons;
					for ( int i = 0; i < MAX_WEAPONS; i++ )
					{
						if ( pBot->GetWeapon( i ) )
						{
							weapons.AddToTail( pBot->GetWeapon( i ) );
						}
					}

					if ( weapons.Count() > 0 )
					{
						pWeapon = weapons[ g_pServerBenchmark->RandomInt( 0, weapons.Count() - 1 ) ];
					}
				}
			}
			else
			{
				// Manually look through weapons to ignore subtype
				for ( int i = 0; i < MAX_WEAPONS; i++ )
				{
					if ( pBot->GetWeapon( i ) && FClassnameIs( pBot->GetWeapon( i ), pszWeapon ) )
					{
						pWeapon = pBot->GetWeapon( i );
						break;
					}
				}
			}

			if ( pWeapon )
			{
				// Switch to it if we don't have it out
				CBaseCombatWeapon *pActiveWeapon = pBot->GetActiveWeapon();
				if ( pActiveWeapon != pWeapon )
				{
					pBot->Weapon_Switch( pWeapon );
				}
			}
		}

		if ( pBot->GetActiveWeapon() )
		{
			if ( bot_forceattackon.GetBool() )
			{
				buttons |= IN_ATTACK;
			}

			if ( bot_forceattack2.GetBool() )
			{
				buttons |= IN_ATTACK2;
			}
		}

		if ( bot_flipout.GetInt() )
		{
			// Some weapons require releases, so randomise firing
			if ( bot_forceattackon.GetBool() || ( g_pServerBenchmark->RandomFloat( 0.0, 1.0 ) > 0.5 ) )
			{
				buttons |= bot_forceattack2.GetBool() ? IN_ATTACK2 : IN_ATTACK;
			}
		}
	}
	else
	{
		// Wait for Reinforcement wave
		if ( !pBot->IsAlive() )
		{
			if ( botdata->m_bWasDead )
			{
				// Wait for a few seconds before respawning.
				if ( gpGlobals->curtime - botdata->m_flDeadTime > 3 )
				{
					// Respawn the bot
					buttons |= IN_JUMP;
				}
			}
			else
			{
				// Start a timer to respawn them in a few seconds.
				botdata->m_bWasDead = true;
				botdata->m_flDeadTime = gpGlobals->curtime;
			}
		}
	}

	if ( bot_flipout.GetInt() == 2 )
	{
		QAngle angOffset = RandomAngle( -1, 1 );

		botdata->lastAngles += angOffset;

		for ( int i = 0 ; i < 2; i++ )
		{
			if ( fabs( botdata->lastAngles[ i ] - botdata->forwardAngle[ i ] ) > 15.0f )
			{
				if ( botdata->lastAngles[ i ] > botdata->forwardAngle[ i ] )
				{
					botdata->lastAngles[ i ] = botdata->forwardAngle[ i ] + 15;
				}
				else
				{
					botdata->lastAngles[ i ] = botdata->forwardAngle[ i ] - 15;
				}
			}
		}

		botdata->lastAngles[ 2 ] = 0;

		pBot->SetLocalAngles( botdata->lastAngles );
		vecViewAngles = botdata->lastAngles;
		forwardmove = 0.0;
	}
	else if ( bot_flipout.GetInt() == 3 )
	{
		// Spin around in circles
		botdata->lastAngles[ YAW ] = AngleNormalize( ( pBot->entindex() + gpGlobals->curtime * 1.7 ) * 45 );
		botdata->lastAngles[ ROLL ] = 0;

		// Reconstruction note: the sin/cos arguments are not recoverable from
		// the decompiled output; both depend on the bot's entindex and curtime.
		float speed = 300;
		forwardmove = sin( pBot->entindex() + gpGlobals->curtime ) * speed;
		sidemove = cos( pBot->entindex() + gpGlobals->curtime ) * speed;

		pBot->SetLocalAngles( botdata->lastAngles );
		vecViewAngles = botdata->lastAngles;

		// Look up and down while shooting
		if ( g_pServerBenchmark->IsBenchmarkRunning() )
		{
			botdata->lastAngles[ PITCH ] = 0;
		}
		else
		{
			botdata->lastAngles[ PITCH ] = sin( pBot->entindex() + gpGlobals->curtime ) * 90;
			buttons &= ~( IN_ATTACK | IN_ATTACK2 );
		}
	}

	// The player that joined before this bot is the one being assisted
	CBasePlayer *pPlayer = UTIL_PlayerByIndex( pBot->entindex() - 1 );

	// Look where the player is looking
	if ( bot_look.GetBool() && pPlayer )
	{
		Vector forward;
		AngleVectors( pPlayer->EyeAngles(), &forward );

		Ray_t ray;
		ray.Init( pPlayer->EyePosition(), pPlayer->EyePosition() + forward * 1000.f );
		trace_t tr;
		UTIL_TraceRay( ray, MASK_SOLID, NULL, COLLISION_GROUP_NONE, &tr );

		forward = tr.endpos - pBot->EyePosition();
		VectorAngles( forward, vecViewAngles );

		NDebugOverlay::Sphere( tr.endpos, 5.f, 255, 0, 0, false, 0.1f );
		NDebugOverlay::Line( pBot->EyePosition(), tr.endpos, 0, 255, 0, false, 0.1f );
		NDebugOverlay::Line( pPlayer->EyePosition(), tr.endpos, 0, 0, 255, false, 0.1f );
	}

	// Fly after the player
	if ( bot_follow.GetBool() && pPlayer )
	{
		pBot->SetMoveType( MOVETYPE_NOCLIP );

		Vector dir = pPlayer->GetAbsOrigin() - pBot->GetAbsOrigin();
		float dist = dir.Length();
		dir.NormalizeInPlace();

		upmove = 0.0;
		if ( dist > bot_following_distance.GetFloat() )
		{
			float speed = pBot->MaxSpeed();

			Vector forward, right, up;
			AngleVectors( pBot->EyeAngles(), &forward, &right, &up );

			forwardmove = speed * DotProduct( forward.Normalized(), dir );
			sidemove = speed * DotProduct( right.Normalized(), dir );
			upmove = speed * DotProduct( up.Normalized(), dir );
		}
	}
	else
	{
		pBot->SetMoveType( MOVETYPE_WALK );
		upmove = 0.0;
	}

	RunPlayerMove( pBot, vecViewAngles, forwardmove, sidemove, upmove, buttons, impulse, frametime );
}

//------------------------------------------------------------------------------
// Purpose: sends the specified command from a bot
//------------------------------------------------------------------------------
void cc_bot_sendcommand( const CCommand &args )
{
	if ( args.ArgC() < 3 )
	{
		Msg( "Too few parameters.  Usage: bot_command <bot name> <command string...>\n" );
		return;
	}

	// get the bot's player object
	CBasePlayer *pPlayer = UTIL_PlayerByName( args[1] );
	if ( !pPlayer )
	{
		Msg( "No bot with name %s\n", args[1] );
		return;
	}

	const char *commandline = args.GetCommandString();

	// find the rest of the command line past the bot index
	commandline = strstr( commandline, args[2] );
	Assert( commandline );

	int iSize = Q_strlen( commandline ) + 1;
	char *pBuf = (char *)malloc( iSize );
	Q_snprintf( pBuf, iSize, "%s", commandline );

	if ( pBuf[iSize - 2] == '"' )
	{
		pBuf[iSize - 2] = '\0';
	}

	// make a command object with the intended command line
	CCommand command;
	command.Tokenize( pBuf );

	// send the command
	g_pGameRules->ClientCommand( pPlayer, command );
}
static ConCommand bot_sendcommand( "bot_command", cc_bot_sendcommand, "<bot name> <command string...>.  Sends specified command on behalf of specified bot", FCVAR_CHEAT );

//------------------------------------------------------------------------------
// Purpose: sends the specified command from a bot
//------------------------------------------------------------------------------
void cc_bot_kill( const CCommand &args )
{
	// get the bot's player object
	CBasePlayer *pPlayer = UTIL_PlayerByName( args[1] );
	if ( pPlayer )
	{
		pPlayer->CommitSuicide();
	}
	else
	{
		Msg( "No bot with name %s\n", args[1] );
	}
}
static ConCommand bot_kill( "bot_kill", cc_bot_kill, "<bot id>.  Kills bot.", FCVAR_CHEAT );

// Reconstruction note: both 2010 builds register this command with an empty
// body; there is no ammo to refill in the paint build.
static void bot_refill( const CCommand &args )
{
}
static ConCommand bot_refill_command( "bot_refill", bot_refill, "Refill all bot ammo counts", FCVAR_CHEAT );

static void bot_teleport( const CCommand &args )
{
	if ( args.ArgC() < 8 )
	{
		Msg( "Too few parameters.  bot_teleport <bot name> <X> <Y> <Z> <Pitch> <Yaw> <Roll>\n" );
		return;
	}

	CBasePlayer *pBot = UTIL_PlayerByName( args[1] );
	if ( !pBot )
	{
		Msg( "No bot with name %s\n", args[1] );
		return;
	}

	Vector vecPos( atof( args[2] ), atof( args[3] ), atof( args[4] ) );
	QAngle vecAng( atof( args[5] ), atof( args[6] ), atof( args[7] ) );
	pBot->Teleport( &vecPos, &vecAng, NULL );
}
static ConCommand bot_teleport_command( "bot_teleport", bot_teleport, "Teleport the specified bot to the specified position & angles.\n\tFormat: bot_teleport <bot name> <X> <Y> <Z> <Pitch> <Yaw> <Roll>", FCVAR_CHEAT );

void cc_bot_equip( const CCommand &args )
{
	if ( args.ArgC() < 3 )
	{
		Msg( "Too few parameters.  bot_equip <bot name> <item name>\n" );
		return;
	}

	CBasePlayer *pBot = UTIL_PlayerByName( args[1] );
	if ( !pBot )
	{
		Msg( "No bot with name %s\n", args[1] );
		return;
	}

	CBaseEntity *pExisting = pBot->Weapon_OwnsThisType( args[2] );
	if ( pExisting )
	{
		Msg( "%s already has %s\n", args[1], args[2] );
		return;
	}

	CBaseCombatWeapon *pWpn = dynamic_cast< CBaseCombatWeapon* >( CreateEntityByName( args[2] ) );
	if ( pWpn )
	{
		pBot->Weapon_Switch( pWpn );
		return;
	}

	Msg( "Failed to create item name %s\n", args[2] );
}
static ConCommand bot_equip( "bot_equip", cc_bot_equip, "Generate an item and have the bot equip it.\n\tFormat: bot_equip <bot name> <item name>", FCVAR_CHEAT );
