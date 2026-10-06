// DWARF declaration skeleton for game/client/in_main.cpp
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x61b50 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
}

// game/client/in_main.cpp:41
int in_impulse[2];

// game/client/in_main.cpp:42
static int in_cancel[2];

// game/client/in_main.cpp:44
ConVar cl_anglespeedkey;

// game/client/in_main.cpp:45
ConVar cl_yawspeed;

// game/client/in_main.cpp:46
ConVar cl_pitchspeed;

// game/client/in_main.cpp:47
ConVar cl_pitchdown;

// game/client/in_main.cpp:48
ConVar cl_pitchup;

// game/client/in_main.cpp:49
ConVar cl_sidespeed;

// game/client/in_main.cpp:50
ConVar cl_upspeed;

// game/client/in_main.cpp:51
ConVar cl_forwardspeed;

// game/client/in_main.cpp:52
ConVar cl_backspeed;

// game/client/in_main.cpp:53
ConVar lookspring;

// game/client/in_main.cpp:54
ConVar lookstrafe;

// game/client/in_main.cpp:57
ConVar in_joystick;

// game/client/in_main.cpp:59
ConVar thirdperson_platformer;

// game/client/in_main.cpp:60
ConVar thirdperson_screenspace;

// game/client/in_main.cpp:62
ConVar sv_noclipduringpause;

// game/client/in_main.cpp:63
static ConVar cl_lagcomp_errorcheck;

// game/client/in_main.cpp:69 (declaration)
bool UsingMouselook( int nSlot );

// game/client/in_main.cpp:69 @0x47f3e0 _Z14UsingMouselooki
bool UsingMouselook( int nSlot )
{
}

// game/client/in_main.cpp:75
ConVar in_forceuser;

// game/client/in_main.cpp:76
static ConVar ss_mimic;

// game/client/in_main.cpp:78 @0x47ef10 _ZL19SplitScreenTeleporti
SplitScreenTeleport( int nSlot )
{
	C_BasePlayer *pPlayer;  // line 80
	Vector vecOrigin;  // line 84
	QAngle angles;  // line 85
	int nOther;  // line 87
	{
		char cmd[256];  // line 91
	}
}

// game/client/in_main.cpp:100 @0x47f070 _ZL11ss_teleportRK8CCommand
ss_teleport( const CCommand &args )
{
}

// game/client/in_main.cpp:100
static ConCommand ss_teleport_command;

// game/client/in_main.cpp:126
kbutton_t in_speed;

// game/client/in_main.cpp:127
kbutton_t in_walk;

// game/client/in_main.cpp:128
kbutton_t in_jlook;

// game/client/in_main.cpp:129
kbutton_t in_strafe;

// game/client/in_main.cpp:130
kbutton_t in_commandermousemove;

// game/client/in_main.cpp:131
kbutton_t in_forward;

// game/client/in_main.cpp:132
kbutton_t in_back;

// game/client/in_main.cpp:133
kbutton_t in_moveleft;

// game/client/in_main.cpp:134
kbutton_t in_moveright;

// game/client/in_main.cpp:136
kbutton_t in_graph;

// game/client/in_main.cpp:137
kbutton_t in_joyspeed;

// game/client/in_main.cpp:138
kbutton_t in_ducktoggle;

// game/client/in_main.cpp:139
kbutton_t in_lookspin;

// game/client/in_main.cpp:141
kbutton_t in_attack;

// game/client/in_main.cpp:142
kbutton_t in_attack2;

// game/client/in_main.cpp:143
kbutton_t in_zoom;

// game/client/in_main.cpp:145
static kbutton_t in_klook;

// game/client/in_main.cpp:146
static kbutton_t in_left;

// game/client/in_main.cpp:147
static kbutton_t in_right;

// game/client/in_main.cpp:148
static kbutton_t in_lookup;

// game/client/in_main.cpp:149
static kbutton_t in_lookdown;

// game/client/in_main.cpp:150
static kbutton_t in_use;

// game/client/in_main.cpp:151
static kbutton_t in_jump;

// game/client/in_main.cpp:153
static kbutton_t in_up;

// game/client/in_main.cpp:154
static kbutton_t in_down;

// game/client/in_main.cpp:155
static kbutton_t in_duck;

// game/client/in_main.cpp:156
static kbutton_t in_reload;

// game/client/in_main.cpp:157
static kbutton_t in_alt1;

// game/client/in_main.cpp:158
static kbutton_t in_alt2;

// game/client/in_main.cpp:159
static kbutton_t in_score;

// game/client/in_main.cpp:160
static kbutton_t in_break;

// game/client/in_main.cpp:161
static kbutton_t in_grenade1;

// game/client/in_main.cpp:162
static kbutton_t in_grenade2;

// game/client/in_main.cpp:180 @0x4817d0 _Z15IN_CenterView_fv
IN_CenterView_f()
{
	QAngle viewangles;  // line 182
	// inlined UsingMouselook() at line 184
}

// game/client/in_main.cpp:200 @0x47eaa0 _Z22IN_Joystick_Advanced_fRK8CCommand
IN_Joystick_Advanced_f( const CCommand &args )
{
}

// game/client/in_main.cpp:206 @0x47eae0 _Z28IN_JoystickChangedCallback_fP7IConVarPKcf
IN_JoystickChangedCallback_f( IConVar *pConVar, const char *pOldString, float flOldValue )
{
}

// game/client/in_main.cpp:220 @0x47f1b0 _Z16KB_ConvertStringPcPS_
int KB_ConvertString( char *in, char **ppout )
{
	char sz[4096];  // line 222
	char binding[64];  // line 223
	char *p;  // line 224
	char *pOut;  // line 225
	char *pEnd;  // line 226
	const char *pBinding;  // line 227
	int maxlen;  // line 282
	// inlined MemAlloc_Alloc() at line 283
	// inlined isalnum() at line 240
}

// game/client/in_main.cpp:297 @0x47f160 _ZN6CInput7FindKeyEPKc
kbutton_t *CInput::FindKey( const char *name )
{
	CKeyboardKey *p;  // line 299
}

// game/client/in_main.cpp:320 (declaration)
void AddKeyButton( const char *name, kbutton_t *pkb );

// game/client/in_main.cpp:320 @0x47f0e0 _ZN6CInput12AddKeyButtonEPKcP9kbutton_t
void CInput::AddKeyButton( const char *name, kbutton_t *pkb )
{
	CKeyboardKey *p;  // line 322
	kbutton_t *kb;  // line 323
}

// game/client/in_main.cpp:342 (declaration)
void CInput();

// game/client/in_main.cpp:342 @0x61920 _ZN6CInputC2Ev
CInput::CInput()
{
	// inlined CInput::PerUserInput_t::PerUserInput_t() at line 342
	// inlined CThreadFastMutex::CThreadFastMutex() at line 342
	{
		int i;  // line 344
	}
}

// game/client/in_main.cpp:342 @0x47f500 _ZN6CInputC1Ev
CInput::CInput()
{
	// inlined CInput::PerUserInput_t::PerUserInput_t() at line 342
	// inlined CThreadFastMutex::CThreadFastMutex() at line 342
	{
		int i;  // line 344
	}
}

// game/client/in_main.cpp:355 (declaration)
~CInput();

// game/client/in_main.cpp:355 @0x47eb10 _ZN6CInputD2Ev
CInput::~CInput()
{
}

// game/client/in_main.cpp:355 @0x47eb30 _ZN6CInputD1Ev
CInput::~CInput()
{
}

// game/client/in_main.cpp:366 (declaration)
void Init_Keyboard();

// game/client/in_main.cpp:366 @0x4811a0 _ZN6CInput13Init_KeyboardEv
void CInput::Init_Keyboard()
{
	// inlined CInput::AddKeyButton() at line 370
	// inlined CInput::AddKeyButton() at line 371
}

// game/client/in_main.cpp:381 (declaration)
void Shutdown_Keyboard();

// game/client/in_main.cpp:381 @0x47eed0 _ZN6CInput17Shutdown_KeyboardEv
void CInput::Shutdown_Keyboard()
{
	CKeyboardKey *p;  // line 383
	CKeyboardKey *n;  // line 383
}

// game/client/in_main.cpp:394 (declaration)
void GetPerUser( int nSlot );

// game/client/in_main.cpp:394 @0x47eb50 _ZN9kbutton_t10GetPerUserEi
kbutton_t::Split_t &kbutton_t::GetPerUser( int nSlot )
{
}

// game/client/in_main.cpp:409 (declaration)
void KeyDown( kbutton_t *b, const char *c );

// game/client/in_main.cpp:409 @0x480300 _Z7KeyDownP9kbutton_tPKc
KeyDown( kbutton_t *b, const char *c )
{
	kbutton_t::Split_t &data;  // line 411
	int k;  // line 413
	// inlined kbutton_t::GetPerUser() at line 411
}

// game/client/in_main.cpp:445 (declaration)
void KeyUp( kbutton_t *b, const char *c );

// game/client/in_main.cpp:445 @0x480240 _Z5KeyUpP9kbutton_tPKc
KeyUp( kbutton_t *b, const char *c )
{
	kbutton_t::Split_t &data;  // line 447
	int k;  // line 455
	// inlined kbutton_t::GetPerUser() at line 447
}

// game/client/in_main.cpp:477 (declaration)
void IN_ClearDuckToggle();

// game/client/in_main.cpp:477 @0x483a80 _Z18IN_ClearDuckTogglev
IN_ClearDuckToggle()
{
	// inlined KeyUp() at line 481
}

// game/client/in_main.cpp:484 @0x485f20 _Z17IN_ForceSpeedDownv
IN_ForceSpeedDown()
{
	// inlined KeyDown() at line 484
}

// game/client/in_main.cpp:485 @0x483a30 _Z15IN_ForceSpeedUpv
IN_ForceSpeedUp()
{
	// inlined KeyUp() at line 485
}

// game/client/in_main.cpp:486 @0x485e00 _Z25IN_CommanderMouseMoveDownRK8CCommand
IN_CommanderMouseMoveDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 486
	// inlined KeyDown() at line 486
}

// game/client/in_main.cpp:487 @0x483930 _Z23IN_CommanderMouseMoveUpRK8CCommand
IN_CommanderMouseMoveUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 487
	// inlined KeyUp() at line 487
}

// game/client/in_main.cpp:488 @0x485ce0 _Z12IN_BreakDownRK8CCommand
IN_BreakDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 488
	// inlined KeyDown() at line 488
}

// game/client/in_main.cpp:489 @0x483830 _Z10IN_BreakUpRK8CCommand
IN_BreakUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 491
	// inlined KeyUp() at line 491
}

// game/client/in_main.cpp:496 @0x485bc0 _Z12IN_KLookDownRK8CCommand
IN_KLookDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 496
	// inlined KeyDown() at line 496
}

// game/client/in_main.cpp:497 @0x483730 _Z10IN_KLookUpRK8CCommand
IN_KLookUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 497
	// inlined KeyUp() at line 497
}

// game/client/in_main.cpp:498 @0x488010 _Z12IN_JLookDownRK8CCommand
IN_JLookDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 498
	// inlined KeyDown() at line 498
}

// game/client/in_main.cpp:499 @0x483630 _Z10IN_JLookUpRK8CCommand
IN_JLookUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 499
	// inlined KeyUp() at line 499
}

// game/client/in_main.cpp:500 @0x487ef0 _Z9IN_UpDownRK8CCommand
IN_UpDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 500
	// inlined KeyDown() at line 500
}

// game/client/in_main.cpp:501 @0x483530 _Z7IN_UpUpRK8CCommand
IN_UpUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 501
	// inlined KeyUp() at line 501
}

// game/client/in_main.cpp:502 @0x487dd0 _Z11IN_DownDownRK8CCommand
IN_DownDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 502
	// inlined KeyDown() at line 502
}

// game/client/in_main.cpp:503 @0x484910 _Z9IN_DownUpRK8CCommand
IN_DownUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 503
	// inlined KeyUp() at line 503
}

// game/client/in_main.cpp:504 @0x487cb0 _Z11IN_LeftDownRK8CCommand
IN_LeftDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 504
	// inlined KeyDown() at line 504
}

// game/client/in_main.cpp:505 @0x484810 _Z9IN_LeftUpRK8CCommand
IN_LeftUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 505
	// inlined KeyUp() at line 505
}

// game/client/in_main.cpp:506 @0x487b90 _Z12IN_RightDownRK8CCommand
IN_RightDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 506
	// inlined KeyDown() at line 506
}

// game/client/in_main.cpp:507 @0x484710 _Z10IN_RightUpRK8CCommand
IN_RightUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 507
	// inlined KeyUp() at line 507
}

// game/client/in_main.cpp:508 @0x487a70 _Z14IN_ForwardDownRK8CCommand
IN_ForwardDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 508
	// inlined KeyDown() at line 508
}

// game/client/in_main.cpp:509 @0x484610 _Z12IN_ForwardUpRK8CCommand
IN_ForwardUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 509
	// inlined KeyUp() at line 509
}

// game/client/in_main.cpp:510 @0x487950 _Z11IN_BackDownRK8CCommand
IN_BackDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 510
	// inlined KeyDown() at line 510
}

// game/client/in_main.cpp:511 @0x484510 _Z9IN_BackUpRK8CCommand
IN_BackUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 511
	// inlined KeyUp() at line 511
}

// game/client/in_main.cpp:512 @0x487830 _Z13IN_LookupDownRK8CCommand
IN_LookupDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 512
	// inlined KeyDown() at line 512
}

// game/client/in_main.cpp:513 @0x484410 _Z11IN_LookupUpRK8CCommand
IN_LookupUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 513
	// inlined KeyUp() at line 513
}

// game/client/in_main.cpp:514 @0x487710 _Z15IN_LookdownDownRK8CCommand
IN_LookdownDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 514
	// inlined KeyDown() at line 514
}

// game/client/in_main.cpp:515 @0x484310 _Z13IN_LookdownUpRK8CCommand
IN_LookdownUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 515
	// inlined KeyUp() at line 515
}

// game/client/in_main.cpp:516 @0x4875f0 _Z15IN_MoveleftDownRK8CCommand
IN_MoveleftDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 516
	// inlined KeyDown() at line 516
}

// game/client/in_main.cpp:517 @0x484210 _Z13IN_MoveleftUpRK8CCommand
IN_MoveleftUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 517
	// inlined KeyUp() at line 517
}

// game/client/in_main.cpp:518 @0x4874d0 _Z16IN_MoverightDownRK8CCommand
IN_MoverightDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 518
	// inlined KeyDown() at line 518
}

// game/client/in_main.cpp:519 @0x484110 _Z14IN_MoverightUpRK8CCommand
IN_MoverightUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 519
	// inlined KeyUp() at line 519
}

// game/client/in_main.cpp:520 @0x4873b0 _Z11IN_WalkDownRK8CCommand
IN_WalkDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 520
	// inlined KeyDown() at line 520
}

// game/client/in_main.cpp:521 @0x484010 _Z9IN_WalkUpRK8CCommand
IN_WalkUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 521
	// inlined KeyUp() at line 521
}

// game/client/in_main.cpp:522 @0x487290 _Z12IN_SpeedDownRK8CCommand
IN_SpeedDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 522
	// inlined KeyDown() at line 522
}

// game/client/in_main.cpp:523 @0x483f10 _Z10IN_SpeedUpRK8CCommand
IN_SpeedUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 523
	// inlined KeyUp() at line 523
}

// game/client/in_main.cpp:524 @0x487170 _Z13IN_StrafeDownRK8CCommand
IN_StrafeDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 524
	// inlined KeyDown() at line 524
}

// game/client/in_main.cpp:525 @0x483e10 _Z11IN_StrafeUpRK8CCommand
IN_StrafeUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 525
	// inlined KeyUp() at line 525
}

// game/client/in_main.cpp:526 @0x487050 _Z14IN_Attack2DownRK8CCommand
IN_Attack2Down( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 526
	// inlined KeyDown() at line 526
}

// game/client/in_main.cpp:527 @0x483d10 _Z12IN_Attack2UpRK8CCommand
IN_Attack2Up( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 527
	// inlined KeyUp() at line 527
}

// game/client/in_main.cpp:528 @0x486f30 _Z10IN_UseDownRK8CCommand
IN_UseDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 528
	// inlined KeyDown() at line 528
}

// game/client/in_main.cpp:529 @0x483c10 _Z8IN_UseUpRK8CCommand
IN_UseUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 529
	// inlined KeyUp() at line 529
}

// game/client/in_main.cpp:530 @0x486e10 _Z11IN_JumpDownRK8CCommand
IN_JumpDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 530
	// inlined KeyDown() at line 530
}

// game/client/in_main.cpp:531 @0x483b10 _Z9IN_JumpUpRK8CCommand
IN_JumpUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 533
	// inlined KeyUp() at line 533
}

// game/client/in_main.cpp:537 @0x486c40 _Z13IN_DuckToggleRK8CCommand
IN_DuckToggle( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 545
	// inlined KeyDown() at line 545
	// inlined IN_ClearDuckToggle() at line 541
}

// game/client/in_main.cpp:549 @0x486a90 _Z11IN_DuckDownRK8CCommand
IN_DuckDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 561
	// inlined KeyDown() at line 561
	// inlined IN_ClearDuckToggle() at line 562
}

// game/client/in_main.cpp:565 @0x4856a0 _Z9IN_DuckUpRK8CCommand
IN_DuckUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 577
	// inlined KeyUp() at line 577
	// inlined IN_ClearDuckToggle() at line 578
}

// game/client/in_main.cpp:582 @0x486970 _Z13IN_ReloadDownRK8CCommand
IN_ReloadDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 582
	// inlined KeyDown() at line 582
}

// game/client/in_main.cpp:583 @0x4855a0 _Z11IN_ReloadUpRK8CCommand
IN_ReloadUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 583
	// inlined KeyUp() at line 583
}

// game/client/in_main.cpp:584 @0x486850 _Z11IN_Alt1DownRK8CCommand
IN_Alt1Down( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 584
	// inlined KeyDown() at line 584
}

// game/client/in_main.cpp:585 @0x4854a0 _Z9IN_Alt1UpRK8CCommand
IN_Alt1Up( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 585
	// inlined KeyUp() at line 585
}

// game/client/in_main.cpp:586 @0x486730 _Z11IN_Alt2DownRK8CCommand
IN_Alt2Down( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 586
	// inlined KeyDown() at line 586
}

// game/client/in_main.cpp:587 @0x4853a0 _Z9IN_Alt2UpRK8CCommand
IN_Alt2Up( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 587
	// inlined KeyUp() at line 587
}

// game/client/in_main.cpp:588 @0x486610 _Z12IN_GraphDownRK8CCommand
IN_GraphDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 588
	// inlined KeyDown() at line 588
}

// game/client/in_main.cpp:589 @0x4852a0 _Z10IN_GraphUpRK8CCommand
IN_GraphUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 589
	// inlined KeyUp() at line 589
}

// game/client/in_main.cpp:590 @0x4864f0 _Z11IN_ZoomDownRK8CCommand
IN_ZoomDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 590
	// inlined KeyDown() at line 590
}

// game/client/in_main.cpp:591 @0x4851a0 _Z9IN_ZoomUpRK8CCommand
IN_ZoomUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 591
	// inlined KeyUp() at line 591
}

// game/client/in_main.cpp:592 @0x4850a0 _Z13IN_Grenade1UpRK8CCommand
IN_Grenade1Up( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 592
	// inlined KeyUp() at line 592
}

// game/client/in_main.cpp:593 @0x4863d0 _Z15IN_Grenade1DownRK8CCommand
IN_Grenade1Down( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 593
	// inlined KeyDown() at line 593
}

// game/client/in_main.cpp:594 @0x484fa0 _Z13IN_Grenade2UpRK8CCommand
IN_Grenade2Up( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 594
	// inlined KeyUp() at line 594
}

// game/client/in_main.cpp:595 @0x4862b0 _Z15IN_Grenade2DownRK8CCommand
IN_Grenade2Down( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 595
	// inlined KeyDown() at line 595
}

// game/client/in_main.cpp:596 @0x47ebb0 _Z11IN_XboxStubRK8CCommand
IN_XboxStub( const CCommand &args )
{
}

// game/client/in_main.cpp:616
kbutton_t in_remote_view_toggle;

// game/client/in_main.cpp:618
static bool g_bRemoteViewKeyWasUp;

// game/client/in_main.cpp:619 @0x484e90 _Z15IN_RemoteViewUpRK8CCommand
IN_RemoteViewUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 622
	// inlined KeyUp() at line 622
}

// game/client/in_main.cpp:624 @0x486110 _Z17IN_RemoteViewDownRK8CCommand
IN_RemoteViewDown( const CCommand &args )
{
	// inlined KeyDown() at line 635
	// inlined CCommand::operator[]() at line 635
	{
		IGameEvent *event;  // line 629
	}
}

// game/client/in_main.cpp:638
static ConCommand startremoteview;

// game/client/in_main.cpp:639
static ConCommand endremoteview;

// game/client/in_main.cpp:642 @0x47ebc0 _Z16IN_ShowPortalsUpRK8CCommand
IN_ShowPortalsUp( const CCommand &args )
{
}

// game/client/in_main.cpp:643 @0x47ebe0 _Z18IN_ShowPortalsDownRK8CCommand
IN_ShowPortalsDown( const CCommand &args )
{
}

// game/client/in_main.cpp:644
static ConCommand showportals;

// game/client/in_main.cpp:645
static ConCommand hideportals;

// game/client/in_main.cpp:647
kbutton_t in_coop_ping;

// game/client/in_main.cpp:649 @0x484d90 _Z13IN_CoopPingUpRK8CCommand
IN_CoopPingUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 649
	// inlined KeyUp() at line 649
}

// game/client/in_main.cpp:650 @0x485ff0 _Z15IN_CoopPingDownRK8CCommand
IN_CoopPingDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 650
	// inlined KeyDown() at line 650
}

// game/client/in_main.cpp:652
static ConCommand presscoopping;

// game/client/in_main.cpp:653
static ConCommand unpresscoopping;

// game/client/in_main.cpp:677 @0x485aa0 _Z13IN_AttackDownRK8CCommand
IN_AttackDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 679
	// inlined KeyDown() at line 679
}

// game/client/in_main.cpp:682 @0x484c70 _Z11IN_AttackUpRK8CCommand
IN_AttackUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 684
	// inlined KeyUp() at line 684
}

// game/client/in_main.cpp:690 @0x47ec00 _Z9IN_CancelRK8CCommand
IN_Cancel( const CCommand &args )
{
}

// game/client/in_main.cpp:696 @0x47f470 _Z10IN_ImpulseRK8CCommand
IN_Impulse( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 699
}

// game/client/in_main.cpp:702 @0x485930 _Z12IN_ScoreDownRK8CCommand
IN_ScoreDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 704
	// inlined KeyDown() at line 704
}

// game/client/in_main.cpp:711 @0x484b10 _Z10IN_ScoreUpRK8CCommand
IN_ScoreUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 713
	// inlined KeyUp() at line 713
}

// game/client/in_main.cpp:721 @0x485810 _Z15IN_LookSpinDownRK8CCommand
IN_LookSpinDown( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 721
	// inlined KeyDown() at line 721
}

// game/client/in_main.cpp:722 @0x484a10 _Z13IN_LookSpinUpRK8CCommand
IN_LookSpinUp( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 722
	// inlined KeyUp() at line 722
}

// game/client/in_main.cpp:731 @0x4810f0 _ZN6CInput8KeyEventEi12ButtonCode_tPKc
int CInput::KeyEvent( int down, ButtonCode_t code, const char *pszCurrentBinding )
{
	// inlined CInput::GetPerUser() at line 736
}

// game/client/in_main.cpp:758 @0x480150 _ZN6CInput8KeyStateEP9kbutton_t
float CInput::KeyState( kbutton_t *key )
{
	kbutton_t::Split_t &data;  // line 760
	float val;  // line 762
	int impulsedown;  // line 763
	int impulseup;  // line 763
	int down;  // line 763
	// inlined kbutton_t::GetPerUser() at line 760
}

// game/client/in_main.cpp:806 @0x47f0a0 _ZN6CInput16IN_SetSampleTimeEf
void CInput::IN_SetSampleTime( float frametime )
{
	{
		int i;  // line 808
	}
}

// game/client/in_main.cpp:820
static ConVar in_usekeyboardsampletime;

// game/client/in_main.cpp:822 (declaration)
void DetermineKeySpeed( int nSlot, float frametime );

// game/client/in_main.cpp:822 @0x481000 _ZN6CInput17DetermineKeySpeedEif
float CInput::DetermineKeySpeed( int nSlot, float frametime )
{
	float speed;  // line 835
	{
		CInput::PerUserInput_t &user;  // line 826
		// inlined CInput::GetPerUser() at line 826
	}
	// inlined kbutton_t::GetPerUser() at line 839
}

// game/client/in_main.cpp:853 @0x480cd0 _ZN6CInput9AdjustYawEifR6QAngle
void CInput::AdjustYaw( int nSlot, float speed, QAngle &viewangles )
{
	const CInput::PerUserInput_t &user;  // line 861
	{
		float side;  // line 867
		float forward;  // line 868
	}
	// inlined kbutton_t::GetPerUser() at line 855
	// inlined CInput::GetPerUser() at line 861
}

// game/client/in_main.cpp:887 @0x481280 _ZN6CInput11AdjustPitchEifR6QAngle
void CInput::AdjustPitch( int nSlot, float speed, QAngle &viewangles )
{
	// inlined UsingMouselook() at line 890
	{
		float up;  // line 892
		float down;  // line 892
		// inlined kbutton_t::GetPerUser() at line 894
	}
}

// game/client/in_main.cpp:920 (declaration)
void ClampAngles( QAngle &viewangles );

// game/client/in_main.cpp:920 @0x47ec40 _ZN6CInput11ClampAnglesER6QAngle
void CInput::ClampAngles( QAngle &viewangles )
{
	// inlined ConVar::GetFloat() at line 922
}

// game/client/in_main.cpp:951 @0x4814d0 _ZN6CInput12AdjustAnglesEif
void CInput::AdjustAngles( int nSlot, float frametime )
{
	float speed;  // line 953
	QAngle viewangles;  // line 954
	QAngle vecAnglesBeforeTilt;  // line 970
	// inlined CInput::ClampAngles() at line 984
	// inlined QAngle::operator=() at line 975
	// inlined CInput::GetPerUser() at line 975
	// inlined QAngle::operator-() at line 975
	// inlined QAngle::operator-=() at line 967
	// inlined CInput::GetPerUser() at line 967
	// inlined CInput::DetermineKeySpeed() at line 957
}

// game/client/in_main.cpp:996 @0x47fe80 _ZN6CInput15ComputeSideMoveEiP8CUserCmd
void CInput::ComputeSideMove( int nSlot, CUserCmd *cmd )
{
	{
		float ideal_yaw;  // line 1011
		float ideal_sin;  // line 1013
		float ideal_cos;  // line 1014
		float movement;  // line 1016
	}
	// inlined kbutton_t::GetPerUser() at line 1027
	// inlined ConVar::GetFloat() at line 1034
	// inlined ConVar::GetFloat() at line 1035
	// inlined ConVar::GetFloat() at line 1029
	// inlined ConVar::GetFloat() at line 1030
}

// game/client/in_main.cpp:1044 (declaration)
void ComputeUpwardMove( int nSlot, CUserCmd *cmd );

// game/client/in_main.cpp:1044 @0x47ec90 _ZN6CInput17ComputeUpwardMoveEiP8CUserCmd
void CInput::ComputeUpwardMove( int nSlot, CUserCmd *cmd )
{
	// inlined ConVar::GetFloat() at line 1046
	// inlined ConVar::GetFloat() at line 1047
}

// game/client/in_main.cpp:1056 @0x47fb80 _ZN6CInput18ComputeForwardMoveEiP8CUserCmd
void CInput::ComputeForwardMove( int nSlot, CUserCmd *cmd )
{
	{
		float movement;  // line 1062
	}
	// inlined kbutton_t::GetPerUser() at line 1093
	{
		float ideal_yaw;  // line 1078
		float ideal_sin;  // line 1080
		float ideal_cos;  // line 1081
		float movement;  // line 1083
	}
	// inlined ConVar::GetFloat() at line 1095
	// inlined ConVar::GetFloat() at line 1096
}

// game/client/in_main.cpp:1106 (declaration)
void ScaleMovements( CUserCmd *cmd );

// game/client/in_main.cpp:1106 @0x47ed60 _ZN6CInput14ScaleMovementsEP8CUserCmd
void CInput::ScaleMovements( CUserCmd *cmd )
{
}

// game/client/in_main.cpp:1142 (declaration)
void ControllerMove( int nSlot, float frametime, CUserCmd *cmd );

// game/client/in_main.cpp:1142 @0x480be0 _ZN6CInput14ControllerMoveEifP8CUserCmd
void CInput::ControllerMove( int nSlot, float frametime, CUserCmd *cmd )
{
	// inlined ConVar::GetInt() at line 1144
	// inlined CInput::GetPerUser() at line 1146
}

// game/client/in_main.cpp:1164 @0x480b60 _ZN6CInput19MakeWeaponSelectionEP18C_BaseCombatWeapon
void CInput::MakeWeaponSelection( C_BaseCombatWeapon *weapon )
{
	// inlined CInput::GetPerUser() at line 1166
	// inlined CHandle<C_BaseCombatWeapon>::operator=() at line 1166
}

// game/client/in_main.cpp:1169 (declaration)
void GetPerUser( int nSlot );

// game/client/in_main.cpp:1169 @0x47ed70 _ZN6CInput10GetPerUserEi
CInput::PerUserInput_t &CInput::GetPerUser( int nSlot )
{
}

// game/client/in_main.cpp:1179 @0x47edd0 _ZNK6CInput10GetPerUserEi
const CInput::PerUserInput_t &CInput::GetPerUser( int nSlot )
{
}

// game/client/in_main.cpp:1189 @0x4818d0 _ZN6CInput16ExtraMouseSampleEfb
void CInput::ExtraMouseSample( float frametime, bool active )
{
	int nSlot;  // line 1192
	CUserCmd *cmd;  // line 1195
	QAngle viewangles;  // line 1199
	// inlined VectorCopy() at line 1232
	// inlined CInput::GetPerUser() at line 1232
	// inlined VectorCopy() at line 1231
	// inlined CUserCmd::Reset() at line 1197
	// inlined CInput::ComputeUpwardMove() at line 1212
	// inlined CInput::ControllerMove() at line 1221
	// inlined CUserCmd::CUserCmd() at line 1194
	CUserCmd dummy[2];  // line 1194
}

// game/client/in_main.cpp:1257 @0x482b40 _ZN6CInput10CreateMoveEifb
void CInput::CreateMove( int sequence_number, float input_sample_frametime, bool active )
{
	int nSlot;  // line 1260
	CUserCmd *cmd;  // line 1262
	CVerifiedUserCmd *pVerified;  // line 1263
	QAngle viewangles;  // line 1270
	CUserCmd *pPlayer0Command;  // line 1355
	// inlined CUserCmd::GetChecksum() at line 1378
	// inlined CUserCmd::operator=() at line 1377
	// inlined CInput::GetPerUser() at line 1358
	// inlined VectorCopy() at line 1344
	// inlined CInput::GetPerUser() at line 1344
	// inlined VectorCopy() at line 1343
	// inlined CHandle<C_BaseCombatWeapon>::operator!=() at line 1315
	// inlined CInput::GetPerUser() at line 1315
	// inlined CInput::ControllerMove() at line 1296
	// inlined CUserCmd::Reset() at line 1265
	// inlined CInput::GetPerUser() at line 1262
	// inlined CInput::GetPerUser() at line 1301
	{
		float mx;  // line 1303
		float my;  // line 1303
	}
	{
		C_BaseCombatWeapon *weapon;  // line 1317
		// inlined CInput::GetPerUser() at line 1317
		// inlined CHandle<C_BaseCombatWeapon>::operator C_BaseCombatWeapon*() at line 1317
		// inlined CInput::GetPerUser() at line 1323
		// inlined CHandle<C_BaseCombatWeapon>::operator=() at line 1323
	}
	// inlined CInput::GetPerUser() at line 1263
	// inlined CInput::ComputeUpwardMove() at line 1285
}

// game/client/in_main.cpp:1381 @0x47f730 _ZN6CInput21CheckSplitScreenMimicEiP8CUserCmdS1_
void CInput::CheckSplitScreenMimic( int nSlot, CUserCmd *cmd, CUserCmd *pPlayer0Command )
{
	int nMimicMode;  // line 1384
	int nLeader;  // line 1398
	C_BasePlayer *pLeader;  // line 1400
	C_BasePlayer *pFollower;  // line 1401
	Vector leaderPos;  // line 1406
	Vector followerPos;  // line 1407
	float flFarDist;  // line 1409
	float flNearDist;  // line 1410
	Vector delta;  // line 1412
	float flLength2DSqr;  // line 1413
	// inlined Vector::Length2DSqr() at line 1413
	// inlined Vector::operator-() at line 1412
	// inlined CUserCmd::operator=() at line 1388
	{
		Vector lookDir;  // line 1423
		Vector rightDir;  // line 1424
		Vector moveDir;  // line 1428
		float fdot;  // line 1433
		float rdot;  // line 1434
		// inlined Vector::Dot() at line 1434
		// inlined Vector::Dot() at line 1433
		// inlined Vector::NormalizeInPlace() at line 1430
		// inlined Vector::NormalizeInPlace() at line 1427
	}
}

// game/client/in_main.cpp:1468 @0x47ee70 _ZN6CInput11CheckPausedEP8CUserCmd
void CInput::CheckPaused( CUserCmd *cmd )
{
}

// game/client/in_main.cpp:1487 @0x4803d0 _ZN6CInput21EncodeUserCmdToBufferEiR8bf_writei
void CInput::EncodeUserCmdToBuffer( int nSlot, bf_write &buf, int sequence_number )
{
	CUserCmd nullcmd;  // line 1489
	CUserCmd *cmd;  // line 1490
	// inlined CUserCmd::CUserCmd() at line 1489
}

// game/client/in_main.cpp:1501 @0x480a50 _ZN6CInput23DecodeUserCmdFromBufferEiR7bf_readi
void CInput::DecodeUserCmdFromBuffer( int nSlot, bf_read &buf, int sequence_number )
{
	CUserCmd nullcmd;  // line 1503
	CUserCmd *cmd;  // line 1504
	// inlined CInput::GetPerUser() at line 1504
	// inlined CUserCmd::CUserCmd() at line 1503
}

// game/client/in_main.cpp:1509 @0x4805e0 _ZN6CInput15ValidateUserCmdEP8CUserCmdi
void CInput::ValidateUserCmd( CUserCmd *usercmd, int sequence_number )
{
	CRC32_t crc;  // line 1512
	// inlined CUserCmd::GetChecksum() at line 1512
	// inlined CInput::GetPerUser() at line 1513
	// inlined CInput::GetPerUser() at line 1515
	// inlined CUserCmd::operator=() at line 1515
}

// game/client/in_main.cpp:1525 @0x4808d0 _ZN6CInput25WriteUsercmdDeltaToBufferEiP8bf_writeiib
bool CInput::WriteUsercmdDeltaToBuffer( int nSlot, bf_write *buf, int from, int to, bool isnewcommand )
{
	CUserCmd nullcmd;  // line 1529
	CUserCmd *f;  // line 1531
	CUserCmd *t;  // line 1531
	int startbit;  // line 1533
	{
		int endbit;  // line 1571
	}
	// inlined bf_write::GetNumBitsWritten() at line 1533
	// inlined CUserCmd::CUserCmd() at line 1529
}

// game/client/in_main.cpp:1587 @0x480540 _ZN6CInput10GetUserCmdEii
CUserCmd *CInput::GetUserCmd( int nSlot, int sequence_number )
{
	CUserCmd *usercmd;  // line 1591
	// inlined CInput::GetPerUser() at line 1591
}

// game/client/in_main.cpp:1610 (declaration)
void CalcButtonBits( int nSlot, int &bits, int in_button, int in_ignore, kbutton_t *button, bool reset );

// game/client/in_main.cpp:1643 @0x481de0 _ZN6CInput13GetButtonBitsEb
int CInput::GetButtonBits( bool bResetState )
{
	int nSlot;  // line 1646
	int bits;  // line 1648
	int ignore;  // line 1650
	// inlined CInput::GetPerUser() at line 1722
	// inlined CalcButtonBits() at line 1680
	// inlined CalcButtonBits() at line 1679
	// inlined CalcButtonBits() at line 1671
	// inlined CalcButtonBits() at line 1670
	// inlined CalcButtonBits() at line 1669
	// inlined CalcButtonBits() at line 1668
	// inlined CalcButtonBits() at line 1667
	// inlined CalcButtonBits() at line 1666
	// inlined CalcButtonBits() at line 1665
	// inlined CalcButtonBits() at line 1664
	// inlined CalcButtonBits() at line 1663
	// inlined CalcButtonBits() at line 1662
	// inlined CalcButtonBits() at line 1661
	// inlined CalcButtonBits() at line 1660
	// inlined CalcButtonBits() at line 1659
	// inlined CalcButtonBits() at line 1658
	// inlined CalcButtonBits() at line 1657
	// inlined CalcButtonBits() at line 1656
	// inlined CalcButtonBits() at line 1655
	// inlined CalcButtonBits() at line 1654
	// inlined CalcButtonBits() at line 1653
	// inlined CalcButtonBits() at line 1652
	// inlined CalcButtonBits() at line 1651
	// inlined CInput::GetPerUser() at line 1650
}

// game/client/in_main.cpp:1732 @0x4804d0 _ZN6CInput16ClearInputButtonEi
void CInput::ClearInputButton( int bits )
{
	// inlined CInput::GetPerUser() at line 1741
}

// game/client/in_main.cpp:1751 @0x47f4d0 _ZN6CInput13GetLookSpringEv
float CInput::GetLookSpring()
{
}

// game/client/in_main.cpp:1760 @0x480490 _ZN6CInput18GetLastForwardMoveEv
float CInput::GetLastForwardMove()
{
	// inlined CInput::GetPerUser() at line 1762
}

// game/client/in_main.cpp:1795
static ConCommand startcommandermousemove;

// game/client/in_main.cpp:1796
static ConCommand endcommandermousemove;

// game/client/in_main.cpp:1797
static ConCommand startmoveup;

// game/client/in_main.cpp:1798
static ConCommand endmoveup;

// game/client/in_main.cpp:1799
static ConCommand startmovedown;

// game/client/in_main.cpp:1800
static ConCommand endmovedown;

// game/client/in_main.cpp:1801
static ConCommand startleft;

// game/client/in_main.cpp:1802
static ConCommand endleft;

// game/client/in_main.cpp:1803
static ConCommand startright;

// game/client/in_main.cpp:1804
static ConCommand endright;

// game/client/in_main.cpp:1805
static ConCommand startforward;

// game/client/in_main.cpp:1806
static ConCommand endforward;

// game/client/in_main.cpp:1807
static ConCommand startback;

// game/client/in_main.cpp:1808
static ConCommand endback;

// game/client/in_main.cpp:1809
static ConCommand startlookup;

// game/client/in_main.cpp:1810
static ConCommand endlookup;

// game/client/in_main.cpp:1811
static ConCommand startlookdown;

// game/client/in_main.cpp:1812
static ConCommand lookdown;

// game/client/in_main.cpp:1813
static ConCommand startstrafe;

// game/client/in_main.cpp:1814
static ConCommand endstrafe;

// game/client/in_main.cpp:1815
static ConCommand startmoveleft;

// game/client/in_main.cpp:1816
static ConCommand endmoveleft;

// game/client/in_main.cpp:1817
static ConCommand startmoveright;

// game/client/in_main.cpp:1818
static ConCommand endmoveright;

// game/client/in_main.cpp:1819
static ConCommand startspeed;

// game/client/in_main.cpp:1820
static ConCommand endspeed;

// game/client/in_main.cpp:1821
static ConCommand startwalk;

// game/client/in_main.cpp:1822
static ConCommand endwalk;

// game/client/in_main.cpp:1823
static ConCommand startattack;

// game/client/in_main.cpp:1824
static ConCommand endattack;

// game/client/in_main.cpp:1825
static ConCommand startattack2;

// game/client/in_main.cpp:1826
static ConCommand endattack2;

// game/client/in_main.cpp:1827
static ConCommand startuse;

// game/client/in_main.cpp:1828
static ConCommand enduse;

// game/client/in_main.cpp:1829
static ConCommand startjump;

// game/client/in_main.cpp:1830
static ConCommand endjump;

// game/client/in_main.cpp:1831
static ConCommand impulse;

// game/client/in_main.cpp:1832
static ConCommand startklook;

// game/client/in_main.cpp:1833
static ConCommand endklook;

// game/client/in_main.cpp:1834
static ConCommand startjlook;

// game/client/in_main.cpp:1835
static ConCommand endjlook;

// game/client/in_main.cpp:1836
static ConCommand startduck;

// game/client/in_main.cpp:1837
static ConCommand endduck;

// game/client/in_main.cpp:1838
static ConCommand startreload;

// game/client/in_main.cpp:1839
static ConCommand endreload;

// game/client/in_main.cpp:1840
static ConCommand startalt1;

// game/client/in_main.cpp:1841
static ConCommand endalt1;

// game/client/in_main.cpp:1842
static ConCommand startalt2;

// game/client/in_main.cpp:1843
static ConCommand endalt2;

// game/client/in_main.cpp:1844
static ConCommand startscore;

// game/client/in_main.cpp:1845
static ConCommand endscore;

// game/client/in_main.cpp:1846
static ConCommand startshowscores;

// game/client/in_main.cpp:1847
static ConCommand endshowscores;

// game/client/in_main.cpp:1848
static ConCommand startgraph;

// game/client/in_main.cpp:1849
static ConCommand endgraph;

// game/client/in_main.cpp:1850
static ConCommand startbreak;

// game/client/in_main.cpp:1851
static ConCommand endbreak;

// game/client/in_main.cpp:1852
static ConCommand force_centerview;

// game/client/in_main.cpp:1853
static ConCommand joyadvancedupdate;

// game/client/in_main.cpp:1854
static ConCommand startzoom;

// game/client/in_main.cpp:1855
static ConCommand endzoom;

// game/client/in_main.cpp:1856
static ConCommand endgrenade1;

// game/client/in_main.cpp:1857
static ConCommand startgrenade1;

// game/client/in_main.cpp:1858
static ConCommand endgrenade2;

// game/client/in_main.cpp:1859
static ConCommand startgrenade2;

// game/client/in_main.cpp:1860
static ConCommand startlookspin;

// game/client/in_main.cpp:1861
static ConCommand endlookspin;

// game/client/in_main.cpp:1887
static ConCommand xboxmove;

// game/client/in_main.cpp:1888
static ConCommand xboxlook;

// game/client/in_main.cpp:1895 @0x4827e0 _ZN6CInput8Init_AllEv
void CInput::Init_All()
{
	// inlined CInput::Init_Keyboard() at line 1926
	{
		int i;  // line 1899
		// inlined CUserCmd::CUserCmd() at line 1904
		// inlined CVerifiedUserCmd::CVerifiedUserCmd() at line 1905
	}
}

// game/client/in_main.cpp:1941 @0x47fa60 _ZN6CInput12Shutdown_AllEv
void CInput::Shutdown_All()
{
	// inlined CInput::Shutdown_Keyboard() at line 1944
	{
		int i;  // line 1950
		// inlined CVerifiedUserCmd::~CVerifiedUserCmd() at line 1955
	}
}

// game/client/in_main.cpp:1960 @0x47eec0 _ZN6CInput9LevelInitEv
void CInput::LevelInit()
{
}
