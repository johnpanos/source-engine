// DWARF declaration skeleton for game/client/portal2/vgui/vgui_mp_lobby_screen.cpp
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x83070 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
	// inlined CPanelFactory<CObjectControlPanel,VGuiScreenInitData_t>::CPanelFactory() at line 85
	// inlined CPanelFactory<CVGUI_MP_LobbyScreen,VGuiScreenInitData_t>::CPanelFactory() at line 253
	// inlined CPanelFactory<CVGUI_MP_LevelSelectScreen,VGuiScreenInitData_t>::CPanelFactory() at line 451
	// inlined CPanelFactory<CVGUI_MP_LevelSelectScreen2,VGuiScreenInitData_t>::CPanelFactory() at line 630
	// inlined CPanelFactory<CVGUI_MP_LevelSelectScreen3,VGuiScreenInitData_t>::CPanelFactory() at line 651
	// inlined CPanelFactory<CVGUI_MP_LevelSelectScreen4,VGuiScreenInitData_t>::CPanelFactory() at line 672
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:27 sizeof=0x188 (i386)
struct CObjectControlPanel : public CVGuiScreenPanel
{
public:
	CObjectControlPanel( vgui::Panel *, const char * );  // line 31
	virtual bool Init( KeyValues *, VGuiScreenInitData_t * );  // line 33
	virtual void OnCommand( const char * );  // line 34
	virtual void OnTick();  // line 35
protected:
	vgui::Panel *GetActivePanel();  // line 39
	virtual void OnTickActive( C_BaseEntity *, C_Portal_Player * );  // line 42
	C_BaseEntity *GetOwningObject() const;  // line 44
	virtual vgui::Panel *TickCurrentPanel();  // line 47
	void SendToServerObject( const char * );  // line 50
private:
	vgui::EditablePanel *m_pActivePanel; // +0x180  // line 54
	vgui::Panel *m_pCurrentPanel; // +0x184  // line 56
};

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:27 (declaration)
~CObjectControlPanel();

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:27 @0x61f540 _ZN19CObjectControlPanelD0Ev
CObjectControlPanel::~CObjectControlPanel()
{
	// inlined CVGuiScreenPanel::~CVGuiScreenPanel() at line 27
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:27 @0x61f590 _ZN19CObjectControlPanelD1Ev
CObjectControlPanel::~CObjectControlPanel()
{
	// inlined CVGuiScreenPanel::~CVGuiScreenPanel() at line 27
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:31 @0x61eee0 _ZN19CObjectControlPanelC2EPN4vgui5PanelEPKc
CObjectControlPanel::CObjectControlPanel( vgui::Panel *parent, const char *panelName )
{
	// inlined CCommandChainingPanel::CCommandChainingPanel() at line 98
	// inlined CVGuiScreenPanel::~CVGuiScreenPanel() at line 104
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:31 @0x61f0f0 _ZN19CObjectControlPanelC1EPN4vgui5PanelEPKc
CObjectControlPanel::CObjectControlPanel( vgui::Panel *parent, const char *panelName )
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:33 @0x61dad0 _ZN19CObjectControlPanel4InitEP9KeyValuesP20VGuiScreenInitData_t
bool CObjectControlPanel::Init( KeyValues *pKeyValues, VGuiScreenInitData_t *pInitData )
{
	int x;  // line 121
	int y;  // line 121
	int w;  // line 121
	int h;  // line 121
	// inlined vgui::ivgui() at line 113
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:34 @0x61d8e0 _ZN19CObjectControlPanel9OnCommandEPKc
void CObjectControlPanel::OnCommand( const char *command )
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:35 @0x61e830 _ZN19CObjectControlPanel6OnTickEv
void CObjectControlPanel::OnTick()
{
	C_BaseEntity *pObj;  // line 193
	// inlined CObjectControlPanel::GetOwningObject() at line 193
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:42 @0x61d8c0 _ZN19CObjectControlPanel12OnTickActiveEP12C_BaseEntityP15C_Portal_Player
void CObjectControlPanel::OnTickActive( C_BaseEntity *pObj, C_Portal_Player *pLocalPlayer )
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:44 @0x61d850 _ZNK19CObjectControlPanel15GetOwningObjectEv
C_BaseEntity *CObjectControlPanel::GetOwningObject()
{
	C_BaseEntity *pScreenEnt;  // line 138
	C_BaseEntity *pObj;  // line 142
	// inlined C_BaseEntity::GetOwnerEntity() at line 142
	// inlined CVGuiScreenPanel::GetEntity() at line 138
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:47 @0x61e710 _ZN19CObjectControlPanel16TickCurrentPanelEv
vgui::Panel *CObjectControlPanel::TickCurrentPanel()
{
	CHackForGetLocalPlayerAccessAllowedGuard g_HackGLPGuard;  // line 160
	C_Portal_Player *pLocalPlayer;  // line 161
	C_BaseEntity *pObj;  // line 162
	// inlined CObjectControlPanel::GetActivePanel() at line 164
	// inlined CObjectControlPanel::GetOwningObject() at line 162
	// inlined C_Portal_Player::GetLocalPortalPlayer() at line 161
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:50 @0x61e940 _ZN19CObjectControlPanel18SendToServerObjectEPKc
void CObjectControlPanel::SendToServerObject( const char *pMsg )
{
	C_BaseEntity *pObj;  // line 176
	{
		int nSlot;  // line 179
		CSetActiveSplitScreenPlayerGuard g_SSGuard;  // line 180
		// inlined vgui::ipanel() at line 179
	}
	// inlined CObjectControlPanel::GetOwningObject() at line 176
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:62 sizeof=0x17c (i386)
struct CCommandChainingPanel : public vgui::EditablePanel
{
public:
	CCommandChainingPanel( vgui::Panel *, const char * );  // line 66
	virtual void OnCommand( const char * );  // line 72
};

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:62 (declaration)
~CCommandChainingPanel();

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:62 @0x61f100 _ZN21CCommandChainingPanelD0Ev
CCommandChainingPanel::~CCommandChainingPanel()
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:62 @0x61f140 _ZN21CCommandChainingPanelD1Ev
CCommandChainingPanel::~CCommandChainingPanel()
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:72 @0x61f160 _ZN21CCommandChainingPanel9OnCommandEPKc
void CCommandChainingPanel::OnCommand( const char *command )
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:85
CPanelFactory<CObjectControlPanel,VGuiScreenInitData_t> g_CObjectControlPanelFactory;

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:217 sizeof=0x1a8 (i386)
struct CVGUI_MP_LobbyScreen : public CObjectControlPanel
{
public:
	CVGUI_MP_LobbyScreen( vgui::Panel *, const char * );  // line 221
	virtual void ApplySchemeSettings( vgui::IScheme * );  // line 223
	virtual bool Init( KeyValues *, VGuiScreenInitData_t * );  // line 225
	virtual void OnTick();  // line 226
	virtual void OnCommand( const char * );  // line 227
private:
	void Update( C_MP_LobbyScreen * );  // line 230
	void UpdateDayLabel();  // line 231
	void NextDay();  // line 233
	void PreviousDay();  // line 234
	void SelectDay();  // line 235
	vgui::ImagePanel *m_pBackgroundImage; // +0x188  // line 239
	vgui::Label *m_pDayTextLabel; // +0x18c  // line 240
	Color m_cDefault; // +0x190  // line 242
	Color m_cInvisible; // +0x194  // line 243
	bool bIsAlreadyVisible; // +0x198  // line 245
	int m_nStartDay; // +0x19c  // line 248
	int m_nCurrentDay; // +0x1a0  // line 249
	int m_nMaxDays; // +0x1a4  // line 250
};

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:217 (declaration)
~CVGUI_MP_LobbyScreen();

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:217 @0x61f5c0 _ZN20CVGUI_MP_LobbyScreenD1Ev
CVGUI_MP_LobbyScreen::~CVGUI_MP_LobbyScreen()
{
	// inlined CObjectControlPanel::~CObjectControlPanel() at line 217
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:217 @0x61f750 _ZN20CVGUI_MP_LobbyScreenD0Ev
CVGUI_MP_LobbyScreen::~CVGUI_MP_LobbyScreen()
{
	// inlined CObjectControlPanel::~CObjectControlPanel() at line 217
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:221 @0x61eff0 _ZN20CVGUI_MP_LobbyScreenC1EPN4vgui5PanelEPKc
CVGUI_MP_LobbyScreen::CVGUI_MP_LobbyScreen( vgui::Panel *parent, const char *panelName )
{
	// inlined Color::Color() at line 260
	// inlined Color::Color() at line 260
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:221 @0x61f070 _ZN20CVGUI_MP_LobbyScreenC2EPN4vgui5PanelEPKc
CVGUI_MP_LobbyScreen::CVGUI_MP_LobbyScreen( vgui::Panel *parent, const char *panelName )
{
	// inlined Color::Color() at line 260
	// inlined Color::Color() at line 260
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:223 @0x61ddb0 _ZN20CVGUI_MP_LobbyScreen19ApplySchemeSettingsEPN4vgui7ISchemeE
void CVGUI_MP_LobbyScreen::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	// inlined Color::operator=() at line 295
	// inlined Color::Color() at line 296
	// inlined Color::operator=() at line 296
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:225 @0x61dbd0 _ZN20CVGUI_MP_LobbyScreen4InitEP9KeyValuesP20VGuiScreenInitData_t
bool CVGUI_MP_LobbyScreen::Init( KeyValues *pKeyValues, VGuiScreenInitData_t *pInitData )
{
	// inlined vgui::ivgui() at line 309
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:226 @0x61e8f0 _ZN20CVGUI_MP_LobbyScreen6OnTickEv
void CVGUI_MP_LobbyScreen::OnTick()
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:227 @0x61ead0 _ZN20CVGUI_MP_LobbyScreen9OnCommandEPKc
void CVGUI_MP_LobbyScreen::OnCommand( const char *pCommand )
{
	// inlined CVGUI_MP_LobbyScreen::NextDay() at line 366
	// inlined CVGUI_MP_LobbyScreen::PreviousDay() at line 371
	// inlined CVGUI_MP_LobbyScreen::SelectDay() at line 372
	// inlined CObjectControlPanel::OnCommand() at line 379
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:230 @0x61d8d0 _ZN20CVGUI_MP_LobbyScreen6UpdateEP16C_MP_LobbyScreen
void CVGUI_MP_LobbyScreen::Update( C_MP_LobbyScreen *pLobbyScreen )
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:231 @0x61deb0 _ZN20CVGUI_MP_LobbyScreen14UpdateDayLabelEv
void CVGUI_MP_LobbyScreen::UpdateDayLabel()
{
	char szDayName[128];  // line 272
	// inlined PortalMPGameRules() at line 273
	// inlined PortalMPGameRules() at line 274
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:233 @0x61e020 _ZN20CVGUI_MP_LobbyScreen7NextDayEv
void CVGUI_MP_LobbyScreen::NextDay()
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:234 @0x61dff0 _ZN20CVGUI_MP_LobbyScreen11PreviousDayEv
void CVGUI_MP_LobbyScreen::PreviousDay()
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:235 @0x61ea50 _ZN20CVGUI_MP_LobbyScreen9SelectDayEv
void CVGUI_MP_LobbyScreen::SelectDay()
{
	char szServerMsg[32];  // line 354
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:253
CPanelFactory<CVGUI_MP_LobbyScreen,VGuiScreenInitData_t> g_CVGUI_MP_LobbyScreenFactory;

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:421 sizeof=0x1dc (i386)
struct CVGUI_MP_LevelSelectScreen : public CVGuiScreenPanel
{
public:
	CVGUI_MP_LevelSelectScreen( vgui::Panel *, const char * );  // line 425
	virtual void ApplySchemeSettings( vgui::IScheme * );  // line 427
	virtual void PerformLayout();  // line 428
	virtual bool Init( KeyValues *, VGuiScreenInitData_t * );  // line 430
	virtual void OnTick();  // line 431
protected:
	virtual void UpdateLevelLabel();  // line 434
	int m_nDay; // +0x180  // line 437
private:
	vgui::Label *m_pBranchTitleLabel; // +0x184  // line 440
	vgui::Label *m_pSelectLevelLabel; // +0x188  // line 441
	vgui::Label *m_pLevelNumLabel; // +0x18c  // line 442
	vgui::Label *m_pLevelTextLabel; // +0x190  // line 443
	vgui::ImagePanel *m_pLevelIconPanels[16]; // +0x194  // line 444
	bool bIsAlreadyVisible; // +0x1d4  // line 445
	bool m_bInitializedLevelLabel; // +0x1d5  // line 446
	int m_nCurrentLevel; // +0x1d8  // line 448
};

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:421 (declaration)
~CVGUI_MP_LevelSelectScreen();

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:421 @0x61f200 _ZN26CVGUI_MP_LevelSelectScreenD1Ev
CVGUI_MP_LevelSelectScreen::~CVGUI_MP_LevelSelectScreen()
{
	// inlined CVGuiScreenPanel::~CVGuiScreenPanel() at line 421
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:421 @0x61f280 _ZN26CVGUI_MP_LevelSelectScreenD0Ev
CVGUI_MP_LevelSelectScreen::~CVGUI_MP_LevelSelectScreen()
{
	// inlined CVGuiScreenPanel::~CVGuiScreenPanel() at line 421
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:425 @0x61ec00 _ZN26CVGUI_MP_LevelSelectScreenC2EPN4vgui5PanelEPKc
CVGUI_MP_LevelSelectScreen::CVGUI_MP_LevelSelectScreen( vgui::Panel *parent, const char *panelName )
{
	{
		int i;  // line 462
	}
	// inlined CVGuiScreenPanel::~CVGuiScreenPanel() at line 466
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:425 @0x61eed0 _ZN26CVGUI_MP_LevelSelectScreenC1EPN4vgui5PanelEPKc
CVGUI_MP_LevelSelectScreen::CVGUI_MP_LevelSelectScreen( vgui::Panel *parent, const char *panelName )
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:427 @0x61dc40 _ZN26CVGUI_MP_LevelSelectScreen19ApplySchemeSettingsEPN4vgui7ISchemeE
void CVGUI_MP_LevelSelectScreen::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	{
		int i;  // line 570
	}
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:428 @0x61d940 _ZN26CVGUI_MP_LevelSelectScreen13PerformLayoutEv
void CVGUI_MP_LevelSelectScreen::PerformLayout()
{
	{
		int i;  // line 584
	}
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:430 @0x61de40 _ZN26CVGUI_MP_LevelSelectScreen4InitEP9KeyValuesP20VGuiScreenInitData_t
bool CVGUI_MP_LevelSelectScreen::Init( KeyValues *pKeyValues, VGuiScreenInitData_t *pInitData )
{
	// inlined vgui::ivgui() at line 596
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:431 @0x61d8f0 _ZN26CVGUI_MP_LevelSelectScreen6OnTickEv
void CVGUI_MP_LevelSelectScreen::OnTick()
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:434 @0x61e050 _ZN26CVGUI_MP_LevelSelectScreen16UpdateLevelLabelEv
void CVGUI_MP_LevelSelectScreen::UpdateLevelLabel()
{
	int nThisDay;  // line 476
	int nTotalLevels;  // line 477
	int nLevel;  // line 479
	char szLevelNumber[128];  // line 486
	char szLevelName[128];  // line 488
	int w;  // line 497
	int h;  // line 497
	{
		int i;  // line 499
		{
			int nBlockSize;  // line 501
			int nSelectExtra;  // line 502
			bool bSelected;  // line 504
			int x;  // line 531
			int y;  // line 531
			int new_x;  // line 538
			// inlined Color::Color() at line 521
			// inlined PortalMPGameRules() at line 514
			// inlined PortalMPGameRules() at line 509
			// inlined Color::Color() at line 516
		}
	}
	// inlined PortalMPGameRules() at line 489
	// inlined C_PortalMPGameRules::GetCoopBranchLevel() at line 479
	// inlined C_PortalMPGameRules::GetBranchTotalLevelCount() at line 477
	// inlined PortalMPGameRules() at line 477
	// inlined PortalMPGameRules() at line 473
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:451
CPanelFactory<CVGUI_MP_LevelSelectScreen,VGuiScreenInitData_t> g_CVGUI_MP_LevelSelectScreenFactory;

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:624 sizeof=0x1dc (i386)
struct CVGUI_MP_LevelSelectScreen2 : public CVGUI_MP_LevelSelectScreen
{
public:
	CVGUI_MP_LevelSelectScreen2( vgui::Panel *, const char * );  // line 627
};

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:624 (declaration)
~CVGUI_MP_LevelSelectScreen2();

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:624 @0x61f230 _ZN27CVGUI_MP_LevelSelectScreen2D0Ev
CVGUI_MP_LevelSelectScreen2::~CVGUI_MP_LevelSelectScreen2()
{
	// inlined CVGUI_MP_LevelSelectScreen::~CVGUI_MP_LevelSelectScreen() at line 624
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:624 @0x61f7a0 _ZN27CVGUI_MP_LevelSelectScreen2D1Ev
CVGUI_MP_LevelSelectScreen2::~CVGUI_MP_LevelSelectScreen2()
{
	// inlined CVGUI_MP_LevelSelectScreen::~CVGUI_MP_LevelSelectScreen() at line 624
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:627 @0x61ee30 _ZN27CVGUI_MP_LevelSelectScreen2C1EPN4vgui5PanelEPKc
CVGUI_MP_LevelSelectScreen2::CVGUI_MP_LevelSelectScreen2( vgui::Panel *parent, const char *panelName )
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:627 @0x61ee80 _ZN27CVGUI_MP_LevelSelectScreen2C2EPN4vgui5PanelEPKc
CVGUI_MP_LevelSelectScreen2::CVGUI_MP_LevelSelectScreen2( vgui::Panel *parent, const char *panelName )
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:630
CPanelFactory<CVGUI_MP_LevelSelectScreen2,VGuiScreenInitData_t> g_CVGUI_MP_LevelSelectScreen2Factory;

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:645 sizeof=0x1dc (i386)
struct CVGUI_MP_LevelSelectScreen3 : public CVGUI_MP_LevelSelectScreen
{
public:
	CVGUI_MP_LevelSelectScreen3( vgui::Panel *, const char * );  // line 648
};

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:645 (declaration)
~CVGUI_MP_LevelSelectScreen3();

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:645 @0x61f7d0 _ZN27CVGUI_MP_LevelSelectScreen3D0Ev
CVGUI_MP_LevelSelectScreen3::~CVGUI_MP_LevelSelectScreen3()
{
	// inlined CVGUI_MP_LevelSelectScreen::~CVGUI_MP_LevelSelectScreen() at line 645
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:645 @0x61f820 _ZN27CVGUI_MP_LevelSelectScreen3D1Ev
CVGUI_MP_LevelSelectScreen3::~CVGUI_MP_LevelSelectScreen3()
{
	// inlined CVGUI_MP_LevelSelectScreen::~CVGUI_MP_LevelSelectScreen() at line 645
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:648 @0x61ed90 _ZN27CVGUI_MP_LevelSelectScreen3C1EPN4vgui5PanelEPKc
CVGUI_MP_LevelSelectScreen3::CVGUI_MP_LevelSelectScreen3( vgui::Panel *parent, const char *panelName )
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:648 @0x61ede0 _ZN27CVGUI_MP_LevelSelectScreen3C2EPN4vgui5PanelEPKc
CVGUI_MP_LevelSelectScreen3::CVGUI_MP_LevelSelectScreen3( vgui::Panel *parent, const char *panelName )
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:651
CPanelFactory<CVGUI_MP_LevelSelectScreen3,VGuiScreenInitData_t> g_CVGUI_MP_LevelSelectScreen3Factory;

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:666 sizeof=0x1dc (i386)
struct CVGUI_MP_LevelSelectScreen4 : public CVGUI_MP_LevelSelectScreen
{
public:
	CVGUI_MP_LevelSelectScreen4( vgui::Panel *, const char * );  // line 669
};

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:666 (declaration)
~CVGUI_MP_LevelSelectScreen4();

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:666 @0x61f1d0 _ZN27CVGUI_MP_LevelSelectScreen4D1Ev
CVGUI_MP_LevelSelectScreen4::~CVGUI_MP_LevelSelectScreen4()
{
	// inlined CVGUI_MP_LevelSelectScreen::~CVGUI_MP_LevelSelectScreen() at line 666
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:666 @0x61f850 _ZN27CVGUI_MP_LevelSelectScreen4D0Ev
CVGUI_MP_LevelSelectScreen4::~CVGUI_MP_LevelSelectScreen4()
{
	// inlined CVGUI_MP_LevelSelectScreen::~CVGUI_MP_LevelSelectScreen() at line 666
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:669 @0x61ecf0 _ZN27CVGUI_MP_LevelSelectScreen4C1EPN4vgui5PanelEPKc
CVGUI_MP_LevelSelectScreen4::CVGUI_MP_LevelSelectScreen4( vgui::Panel *parent, const char *panelName )
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:669 @0x61ed40 _ZN27CVGUI_MP_LevelSelectScreen4C2EPN4vgui5PanelEPKc
CVGUI_MP_LevelSelectScreen4::CVGUI_MP_LevelSelectScreen4( vgui::Panel *parent, const char *panelName )
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:672
CPanelFactory<CVGUI_MP_LevelSelectScreen4,VGuiScreenInitData_t> g_CVGUI_MP_LevelSelectScreen4Factory;
