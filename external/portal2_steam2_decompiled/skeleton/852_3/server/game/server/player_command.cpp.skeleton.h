// DWARF declaration skeleton for game/server/player_command.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xa9a60 _Z41__static_initialization_and_destruction_0ii
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

// game/server/player_command.cpp:28 (declaration)
void CPlayerMove();

// game/server/player_command.cpp:28 @0xa9a40 _ZN11CPlayerMoveC2Ev
CPlayerMove::CPlayerMove()
{
}

// game/server/player_command.cpp:28 @0x63fcc0 _ZN11CPlayerMoveC1Ev
CPlayerMove::CPlayerMove()
{
}

// game/server/player_command.h:31 @0x641f40 _ZN11CPlayerMoveD0Ev
CPlayerMove::~CPlayerMove()
{
}

// game/server/player_command.h:31 @0x641f60 _ZN11CPlayerMoveD1Ev
CPlayerMove::~CPlayerMove()
{
}

// game/server/player_command.cpp:38 @0x640f90 _ZN11CPlayerMove12StartCommandEP11CBasePlayerP8CUserCmd
void CPlayerMove::StartCommand( CBasePlayer *player, CUserCmd *cmd )
{
	CVProfScope VProf_;  // line 40
	// inlined CVProfScope::~CVProfScope() at line 48
	// inlined CBaseEntity::SetPredictionPlayer() at line 48
	// inlined CVProfScope::CVProfScope() at line 40
	// inlined CVProfScope::~CVProfScope() at line 48
}

// game/server/player_command.cpp:75 @0x640640 _ZN11CPlayerMove13FinishCommandEP11CBasePlayer
void CPlayerMove::FinishCommand( CBasePlayer *player )
{
	CVProfScope VProf_;  // line 77
	// inlined CVProfScope::~CVProfScope() at line 81
	// inlined CBaseEntity::SetPredictionPlayer() at line 81
	// inlined CVProfScope::CVProfScope() at line 77
	// inlined CVProfScope::~CVProfScope() at line 81
}

// game/server/player_command.cpp:90 @0x640200 _ZN11CPlayerMove17CheckMovingGroundEP11CBasePlayerd
void CPlayerMove::CheckMovingGround( CBasePlayer *player, double frametime )
{
	CVProfScope VProf_;  // line 92
	CBaseEntity *groundentity;  // line 94
	{
		Vector vecNewVelocity;  // line 101
		// inlined CBaseEntity::SetBaseVelocity() at line 107
		// inlined Vector::operator+=() at line 105
	}
	// inlined CVProfScope::CVProfScope() at line 92
	// inlined CBaseEntity::GetFlags() at line 96
	// inlined operator*() at line 115
	// inlined Vector::operator VectorByValue&() at line 115
	// inlined CBaseEntity::SetBaseVelocity() at line 116
	// inlined CVProfScope::~CVProfScope() at line 119
	// inlined CVProfScope::~CVProfScope() at line 119
}

// game/server/player_command.cpp:130 @0x63fce0 _ZN11CPlayerMove9SetupMoveEP11CBasePlayerP8CUserCmdP11IMoveHelperP9CMoveData
void CPlayerMove::SetupMove( CBasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move )
{
	CVProfScope VProf_;  // line 132
	CBaseEntity *pMoveParent;  // line 146
	// inlined CVProfScope::CVProfScope() at line 132
	// inlined CBaseEntity::GetAbsOrigin() at line 137
	// inlined Vector::operator!=() at line 137
	// inlined QAngle::operator=() at line 144
	// inlined CBaseEntity::GetMoveParent() at line 146
	// inlined QAngle::operator=() at line 149
	// inlined QAngle::operator=() at line 178
	// inlined CBaseEntity::GetAbsVelocity() at line 180
	// inlined Vector::operator=() at line 180
	// inlined CBaseHandle::operator=() at line 182
	// inlined CBaseEntity::GetAbsOrigin() at line 184
	// inlined CMoveData::SetAbsOrigin() at line 184
	// inlined CNetworkHandleBase<CBaseEntity,CBasePlayer::NetworkVar_m_hConstraintEntity>::Get() at line 187
	// inlined Vector::operator=() at line 190
	// inlined CVProfScope::~CVProfScope() at line 196
	// inlined CBaseEntity::GetAbsOrigin() at line 188
	// inlined Vector::operator=() at line 188
	{
		matrix3x4_t viewToParent;  // line 153
		matrix3x4_t viewToWorld;  // line 153
		// inlined CBaseEntity::EntityToWorldTransform() at line 155
		// inlined MatrixAngles() at line 156
	}
	// inlined CVProfScope::~CVProfScope() at line 196
}

// game/server/player_command.cpp:206 @0x641110 _ZN11CPlayerMove10FinishMoveEP11CBasePlayerP8CUserCmdP9CMoveData
void CPlayerMove::FinishMove( CBasePlayer *player, CUserCmd *ucmd, CMoveData *move )
{
	CVProfScope VProf_;  // line 208
	float pitch;  // line 218
	// inlined CVProfScope::CVProfScope() at line 208
	// inlined CNetworkVarBase<float,CBasePlayer::NetworkVar_m_flMaxspeed>::operator=<float>() at line 210
	// inlined CVProfScope::~CVProfScope() at line 239
	// inlined CVProfScope::~CVProfScope() at line 239
}

// game/server/player_command.cpp:247 @0x640ae0 _ZN11CPlayerMove11RunPreThinkEP11CBasePlayer
void CPlayerMove::RunPreThink( CBasePlayer *player )
{
	CVProfScope VProf_;  // line 249
	// inlined CVProfScope::CVProfScope() at line 249
	{
		CVProfScope VProf_;  // line 252
		// inlined CVProfScope::CVProfScope() at line 252
		// inlined CVProfScope::~CVProfScope() at line 254
	}
	{
		CVProfScope VProf_;  // line 257
		// inlined CVProfScope::CVProfScope() at line 257
		// inlined CVProfScope::~CVProfScope() at line 259
	}
	{
		CVProfScope VProf_;  // line 262
		// inlined CVProfScope::~CVProfScope() at line 263
		// inlined CVProfScope::~CVProfScope() at line 263
		// inlined CVProfScope::CVProfScope() at line 262
	}
	// inlined CVProfScope::~CVProfScope() at line 264
	// inlined CVProfScope::~CVProfScope() at line 264
}

// game/server/player_command.cpp:277 @0x640930 _ZN11CPlayerMove8RunThinkEP11CBasePlayerd
void CPlayerMove::RunThink( CBasePlayer *player, double frametime )
{
	CVProfScope VProf_;  // line 279
	int thinktick;  // line 280
	// inlined CVProfScope::~CVProfScope() at line 289
	// inlined CVProfScope::CVProfScope() at line 279
	// inlined CVProfScope::~CVProfScope() at line 289
}

// game/server/player_command.cpp:298 @0x6407c0 _ZN11CPlayerMove12RunPostThinkEP11CBasePlayer
void CPlayerMove::RunPostThink( CBasePlayer *player )
{
	CVProfScope VProf_;  // line 300
	// inlined CVProfScope::~CVProfScope() at line 303
	// inlined CVProfScope::CVProfScope() at line 300
	// inlined CVProfScope::~CVProfScope() at line 303
}

// game/server/player_command.cpp:315 @0x6413c0 _ZN11CPlayerMove10RunCommandEP11CBasePlayerP8CUserCmdP11IMoveHelper
void CPlayerMove::RunCommand( CBasePlayer *player, CUserCmd *ucmd, IMoveHelper *moveHelper )
{
	const float serverCurTime;  // line 320
	const float serverFrameTime;  // line 321
	const float playerCurTime;  // line 322
	const float playerFrameTime;  // line 323
	IServerVehicle *pVehicle;  // line 367
	// inlined QAngle::operator=() at line 396
	// inlined QAngle::operator+() at line 396
	{
		CBaseCombatWeapon *weapon;  // line 359
		{
			CVProfScope VProf_;  // line 362
			// inlined CVProfScope::CVProfScope() at line 362
			// inlined CVProfScope::~CVProfScope() at line 363
			// inlined CVProfScope::~CVProfScope() at line 363
		}
		// inlined CBaseEntity::Instance() at line 359
	}
	{
		CVProfScope VProf_;  // line 381
		// inlined CVProfScope::~CVProfScope() at line 382
		// inlined CVProfScope::CVProfScope() at line 381
	}
	// inlined QAngle::operator=() at line 387
	// inlined QAngle::operator=() at line 392
	// inlined CBaseEntity::SetEyeAngleOffset() at line 400
	// inlined CBaseEntity::SetEyeOffset() at line 401
	{
		CVProfScope VProf_;  // line 422
		// inlined CVProfScope::~CVProfScope() at line 423
		// inlined CVProfScope::~CVProfScope() at line 423
		// inlined CVProfScope::CVProfScope() at line 422
	}
	{
		CVProfScope VProf_;  // line 434
		// inlined CVProfScope::~CVProfScope() at line 435
		// inlined CVProfScope::~CVProfScope() at line 435
		// inlined CVProfScope::CVProfScope() at line 434
	}
	// inlined CNetworkVarBase<int,CBasePlayer::NetworkVar_m_nTickBase>::operator++() at line 448
	{
		CVProfScope VProf_;  // line 416
		// inlined CVProfScope::CVProfScope() at line 416
	}
}
