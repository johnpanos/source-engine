// DWARF declaration skeleton for game/client/portal2/vgui/portalclientscoreboard.cpp
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x82aa0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
	// inlined CSteamID::CSteamID() at line 650
	// inlined CSteamID::CSteamID() at line 654
	// inlined CSteamID::CSteamID() at line 656
	// inlined CSteamID::CSteamID() at line 659
	// inlined CSteamID::CSteamID() at line 662
}

// game/client/portal2/vgui/portalclientscoreboard.cpp:34 (declaration)
void CPortalClientScoreBoardDialog( IViewPort *pViewPort );

// game/client/portal2/vgui/portalclientscoreboard.cpp:34 @0x6179a0 _ZN29CPortalClientScoreBoardDialogC2EP9IViewPort
CPortalClientScoreBoardDialog::CPortalClientScoreBoardDialog( IViewPort *pViewPort )
{
	// inlined CGameEventListener::ListenForGameEvent() at line 46
	// inlined Color::Color() at line 34
	// inlined Color::Color() at line 34
	// inlined CPortalClientScoreBoardDialog::CPortalClientScoreBoardDialog_RegisterKBMap::CPortalClientScoreBoardDialog_RegisterKBMap() at line 34
	// inlined CPortalClientScoreBoardDialog::CPortalClientScoreBoardDialog_Register::CPortalClientScoreBoardDialog_Register() at line 34
	// inlined CPortalClientScoreBoardDialog::CPortalClientScoreBoardDialog_RegisterMap::CPortalClientScoreBoardDialog_RegisterMap() at line 34
}

// game/client/portal2/vgui/portalclientscoreboard.cpp:34 @0x617cf0 _ZN29CPortalClientScoreBoardDialogC1EP9IViewPort
CPortalClientScoreBoardDialog::CPortalClientScoreBoardDialog( IViewPort *pViewPort )
{
}

// game/client/portal2/vgui/portalclientscoreboard.cpp:54 (declaration)
~CPortalClientScoreBoardDialog();

// game/client/portal2/vgui/portalclientscoreboard.cpp:54 @0x616f30 _ZN29CPortalClientScoreBoardDialogD0Ev
CPortalClientScoreBoardDialog::~CPortalClientScoreBoardDialog()
{
}

// game/client/portal2/vgui/portalclientscoreboard.cpp:54 @0x616fb0 _ZN29CPortalClientScoreBoardDialogD1Ev
CPortalClientScoreBoardDialog::~CPortalClientScoreBoardDialog()
{
}

// game/client/portal2/vgui/portalclientscoreboard.cpp:54 @0x617000 _ZN29CPortalClientScoreBoardDialogD2Ev
CPortalClientScoreBoardDialog::~CPortalClientScoreBoardDialog()
{
}

// game/client/portal2/vgui/portalclientscoreboard.cpp:61 @0x616f00 _ZN29CPortalClientScoreBoardDialog15PaintBackgroundEv
void CPortalClientScoreBoardDialog::PaintBackground()
{
}

// game/client/portal2/vgui/portalclientscoreboard.cpp:73 @0x616ef0 _ZN29CPortalClientScoreBoardDialog11PaintBorderEv
void CPortalClientScoreBoardDialog::PaintBorder()
{
}

// game/client/portal2/vgui/portalclientscoreboard.cpp:85 @0x617050 _ZN29CPortalClientScoreBoardDialog19ApplySchemeSettingsEPN4vgui7ISchemeE
void CPortalClientScoreBoardDialog::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	// inlined Color::operator=() at line 91
	// inlined Color::operator=() at line 92
	// inlined Color::Color() at line 121
	// inlined Color::Color() at line 122
	// inlined Color::Color() at line 123
}

// game/client/portal2/vgui/portalclientscoreboard.cpp:140 @0x616d80 _ZN29CPortalClientScoreBoardDialog5ResetEv
void CPortalClientScoreBoardDialog::Reset()
{
}

// game/client/portal2/vgui/portalclientscoreboard.cpp:149 @0x616dd0 _ZN29CPortalClientScoreBoardDialog20PortalPlayerSortFuncEPN4vgui18SectionedListPanelEii
bool CPortalClientScoreBoardDialog::PortalPlayerSortFunc( vgui::SectionedListPanel *list, int itemID1, int itemID2 )
{
	KeyValues *it1;  // line 151
	KeyValues *it2;  // line 152
	int v1;  // line 156
	int v2;  // line 157
	int iPlayerIndex1;  // line 172
	int iPlayerIndex2;  // line 173
}

// game/client/portal2/vgui/portalclientscoreboard.cpp:180 @0x616b00 _ZN29CPortalClientScoreBoardDialog14InitPlayerListEPN4vgui18SectionedListPanelEi
void CPortalClientScoreBoardDialog::InitPlayerList( vgui::SectionedListPanel *pPlayerList, int teamNumber )
{
}

// game/client/portal2/vgui/portalclientscoreboard.cpp:209 @0x617940 _ZN29CPortalClientScoreBoardDialog6UpdateEv
void CPortalClientScoreBoardDialog::Update()
{
}

// game/client/portal2/vgui/portalclientscoreboard.cpp:223 @0x6166e0 _ZN29CPortalClientScoreBoardDialog14UpdateTeamInfoEv
void CPortalClientScoreBoardDialog::UpdateTeamInfo()
{
	{
		int teamIndex;  // line 226
		{
			wchar_t *teamName;  // line 228
			C_Team *team;  // line 229
			{
				const char *pDialogVarTeamScore;  // line 233
				const char *pDialogVarTeamPlayerCount;  // line 234
				const char *pDialogVarTeamPing;  // line 235
				wchar_t name[64];  // line 255
				wchar_t string1[1024];  // line 256
				wchar_t wNumPlayers[6];  // line 257
				int pingsum;  // line 279
				int numcounted;  // line 280
				{
					int ping;  // line 297
				}
				{
					int playerIndex;  // line 281
					{
						int ping;  // line 285
					}
				}
			}
		}
	}
}

// game/client/portal2/vgui/portalclientscoreboard.cpp:311 @0x616530 _ZN29CPortalClientScoreBoardDialog16UpdatePlayerListEv
void CPortalClientScoreBoardDialog::UpdatePlayerList()
{
	C_Portal_Player *pLocalPlayer;  // line 319
	{
		int playerIndex;  // line 323
		{
			vgui::SectionedListPanel *pPlayerList;  // line 327
			KeyValues *pKeyValues;  // line 343
			int itemID;  // line 346
			Color clr;  // line 347
		}
	}
	// inlined C_Portal_Player::GetLocalPlayer() at line 319
}

// game/client/portal2/vgui/portalclientscoreboard.cpp:358 @0x617690 _ZN29CPortalClientScoreBoardDialog19UpdateSpectatorListEv
void CPortalClientScoreBoardDialog::UpdateSpectatorList()
{
	char szSpectatorList[512];  // line 360
	int nSpectators;  // line 361
	wchar_t wzSpectators[512];  // line 376
	{
		int playerIndex;  // line 362
		// inlined CPortalClientScoreBoardDialog::ShouldShowAsSpectator() at line 364
	}
	{
		const char *pchFormat;  // line 379
		wchar_t wzSpectatorCount[16];  // line 381
		wchar_t wzSpectatorList[1024];  // line 382
	}
}

// game/client/portal2/vgui/portalclientscoreboard.cpp:394 (declaration)
void ShouldShowAsSpectator( int iPlayerIndex );

// game/client/portal2/vgui/portalclientscoreboard.cpp:394 @0x616370 _ZN29CPortalClientScoreBoardDialog21ShouldShowAsSpectatorEi
bool CPortalClientScoreBoardDialog::ShouldShowAsSpectator( int iPlayerIndex )
{
	{
		int iTeam;  // line 403
	}
}

// game/client/portal2/vgui/portalclientscoreboard.cpp:413 @0x6163f0 _ZN29CPortalClientScoreBoardDialog13FireGameEventEP10IGameEvent
void CPortalClientScoreBoardDialog::FireGameEvent( IGameEvent *event )
{
	const char *type;  // line 415
	{
		const char *hostname;  // line 420
		wchar_t wzHostName[256];  // line 421
		wchar_t wzServerLabel[256];  // line 422
	}
}

// game/client/portal2/vgui/portalclientscoreboard.cpp:437 @0x617360 _ZN29CPortalClientScoreBoardDialog18GetPlayerScoreInfoEiP9KeyValues
bool CPortalClientScoreBoardDialog::GetPlayerScoreInfo( int playerIndex, KeyValues *kv )
{
	const char *oldName;  // line 440
	int bufsize;  // line 441
	char *newName;  // line 442
	C_Portal_Player *me;  // line 469
	// inlined C_Portal_Player::GetLocalPlayer() at line 469
	{
		char numspecs[32];  // line 483
	}
}
