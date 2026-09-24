// DWARF declaration skeleton for game/client/portal2/vgui/vgui_mp_lobby_screen.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x81860 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CPanelFactory<CObjectControlPanel,VGuiScreenInitData_t>::CPanelFactory() at line 86
	// inlined CPanelFactory<CVGUI_MP_LobbyScreen,VGuiScreenInitData_t>::CPanelFactory() at line 254
	// inlined CPanelFactory<CVGUI_MP_ProgressSignScreen,VGuiScreenInitData_t>::CPanelFactory() at line 453
	// inlined CPanelFactory<CVGUI_MP_LevelSelectScreen,VGuiScreenInitData_t>::CPanelFactory() at line 699
	// inlined CPanelFactory<CVGUI_MP_LevelSelectScreen2,VGuiScreenInitData_t>::CPanelFactory() at line 878
	// inlined CPanelFactory<CVGUI_MP_LevelSelectScreen3,VGuiScreenInitData_t>::CPanelFactory() at line 899
	// inlined CPanelFactory<CVGUI_MP_LevelSelectScreen4,VGuiScreenInitData_t>::CPanelFactory() at line 920
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:28 sizeof=0x188 (i386)
struct CObjectControlPanel : public CVGuiScreenPanel
{
public:
	CObjectControlPanel( vgui::Panel *, const char * );  // line 32
	virtual bool Init( KeyValues *, VGuiScreenInitData_t * );  // line 34
	virtual void OnCommand( const char * );  // line 35
	virtual void OnTick();  // line 36
protected:
	vgui::Panel *GetActivePanel();  // line 40
	virtual void OnTickActive( C_BaseEntity *, C_Portal_Player * );  // line 43
	C_BaseEntity *GetOwningObject() const;  // line 45
	virtual vgui::Panel *TickCurrentPanel();  // line 48
	void SendToServerObject( const char * );  // line 51
private:
	vgui::EditablePanel *m_pActivePanel; // +0x180  // line 55
	vgui::Panel *m_pCurrentPanel; // +0x184  // line 57
};

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:28 (declaration)
~CObjectControlPanel();

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:28 @0x62c360 _ZN19CObjectControlPanelD0Ev
CObjectControlPanel::~CObjectControlPanel()
{
	// inlined CVGuiScreenPanel::~CVGuiScreenPanel() at line 28
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:28 @0x62c510 _ZN19CObjectControlPanelD1Ev
CObjectControlPanel::~CObjectControlPanel()
{
	// inlined CVGuiScreenPanel::~CVGuiScreenPanel() at line 28
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:32 @0x62bc10 _ZN19CObjectControlPanelC2EPN4vgui5PanelEPKc
CObjectControlPanel::CObjectControlPanel( vgui::Panel *parent, const char *panelName )
{
	// inlined CCommandChainingPanel::CCommandChainingPanel() at line 99
	// inlined CVGuiScreenPanel::~CVGuiScreenPanel() at line 105
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:32 @0x62be20 _ZN19CObjectControlPanelC1EPN4vgui5PanelEPKc
CObjectControlPanel::CObjectControlPanel( vgui::Panel *parent, const char *panelName )
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:34 @0x62a020 _ZN19CObjectControlPanel4InitEP9KeyValuesP20VGuiScreenInitData_t
bool CObjectControlPanel::Init( KeyValues *pKeyValues, VGuiScreenInitData_t *pInitData )
{
	int x;  // line 122
	int y;  // line 122
	int w;  // line 122
	int h;  // line 122
	// inlined vgui::ivgui() at line 114
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:35 @0x629af0 _ZN19CObjectControlPanel9OnCommandEPKc
void CObjectControlPanel::OnCommand( const char *command )
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:36 @0x62b460 _ZN19CObjectControlPanel6OnTickEv
void CObjectControlPanel::OnTick()
{
	C_BaseEntity *pObj;  // line 194
	// inlined CObjectControlPanel::GetOwningObject() at line 194
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:43 @0x629ad0 _ZN19CObjectControlPanel12OnTickActiveEP12C_BaseEntityP15C_Portal_Player
void CObjectControlPanel::OnTickActive( C_BaseEntity *pObj, C_Portal_Player *pLocalPlayer )
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:45 @0x629a60 _ZNK19CObjectControlPanel15GetOwningObjectEv
C_BaseEntity *CObjectControlPanel::GetOwningObject()
{
	C_BaseEntity *pScreenEnt;  // line 139
	C_BaseEntity *pObj;  // line 143
	// inlined C_BaseEntity::GetOwnerEntity() at line 143
	// inlined CVGuiScreenPanel::GetEntity() at line 139
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:48 @0x62b340 _ZN19CObjectControlPanel16TickCurrentPanelEv
vgui::Panel *CObjectControlPanel::TickCurrentPanel()
{
	CHackForGetLocalPlayerAccessAllowedGuard g_HackGLPGuard;  // line 161
	C_Portal_Player *pLocalPlayer;  // line 162
	C_BaseEntity *pObj;  // line 163
	// inlined CObjectControlPanel::GetActivePanel() at line 165
	// inlined CObjectControlPanel::GetOwningObject() at line 163
	// inlined C_Portal_Player::GetLocalPortalPlayer() at line 162
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:51 @0x62b570 _ZN19CObjectControlPanel18SendToServerObjectEPKc
void CObjectControlPanel::SendToServerObject( const char *pMsg )
{
	C_BaseEntity *pObj;  // line 177
	{
		int nSlot;  // line 180
		CSetActiveSplitScreenPlayerGuard g_SSGuard;  // line 181
		// inlined vgui::ipanel() at line 180
	}
	// inlined CObjectControlPanel::GetOwningObject() at line 177
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:63 sizeof=0x17c (i386)
struct CCommandChainingPanel : public vgui::EditablePanel
{
public:
	CCommandChainingPanel( vgui::Panel *, const char * );  // line 67
	virtual void OnCommand( const char * );  // line 73
};

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:63 (declaration)
~CCommandChainingPanel();

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:63 @0x62be30 _ZN21CCommandChainingPanelD0Ev
CCommandChainingPanel::~CCommandChainingPanel()
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:63 @0x62be70 _ZN21CCommandChainingPanelD1Ev
CCommandChainingPanel::~CCommandChainingPanel()
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:73 @0x62be90 _ZN21CCommandChainingPanel9OnCommandEPKc
void CCommandChainingPanel::OnCommand( const char *command )
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:86
CPanelFactory<CObjectControlPanel,VGuiScreenInitData_t> g_CObjectControlPanelFactory;

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:218 sizeof=0x1a8 (i386)
struct CVGUI_MP_LobbyScreen : public CObjectControlPanel
{
public:
	CVGUI_MP_LobbyScreen( vgui::Panel *, const char * );  // line 222
	virtual void ApplySchemeSettings( vgui::IScheme * );  // line 224
	virtual bool Init( KeyValues *, VGuiScreenInitData_t * );  // line 226
	virtual void OnTick();  // line 227
	virtual void OnCommand( const char * );  // line 228
private:
	void Update( C_MP_LobbyScreen * );  // line 231
	void UpdateDayLabel();  // line 232
	void NextDay();  // line 234
	void PreviousDay();  // line 235
	void SelectDay();  // line 236
	vgui::ImagePanel *m_pBackgroundImage; // +0x188  // line 240
	vgui::Label *m_pDayTextLabel; // +0x18c  // line 241
	Color m_cDefault; // +0x190  // line 243
	Color m_cInvisible; // +0x194  // line 244
	bool bIsAlreadyVisible; // +0x198  // line 246
	int m_nStartDay; // +0x19c  // line 249
	int m_nCurrentDay; // +0x1a0  // line 250
	int m_nMaxDays; // +0x1a4  // line 251
};

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:218 (declaration)
~CVGUI_MP_LobbyScreen();

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:218 @0x62c540 _ZN20CVGUI_MP_LobbyScreenD1Ev
CVGUI_MP_LobbyScreen::~CVGUI_MP_LobbyScreen()
{
	// inlined CObjectControlPanel::~CObjectControlPanel() at line 218
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:218 @0x62c570 _ZN20CVGUI_MP_LobbyScreenD0Ev
CVGUI_MP_LobbyScreen::~CVGUI_MP_LobbyScreen()
{
	// inlined CObjectControlPanel::~CObjectControlPanel() at line 218
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:222 @0x62bd20 _ZN20CVGUI_MP_LobbyScreenC1EPN4vgui5PanelEPKc
CVGUI_MP_LobbyScreen::CVGUI_MP_LobbyScreen( vgui::Panel *parent, const char *panelName )
{
	// inlined Color::Color() at line 261
	// inlined Color::Color() at line 261
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:222 @0x62bda0 _ZN20CVGUI_MP_LobbyScreenC2EPN4vgui5PanelEPKc
CVGUI_MP_LobbyScreen::CVGUI_MP_LobbyScreen( vgui::Panel *parent, const char *panelName )
{
	// inlined Color::Color() at line 261
	// inlined Color::Color() at line 261
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:224 @0x62b2b0 _ZN20CVGUI_MP_LobbyScreen19ApplySchemeSettingsEPN4vgui7ISchemeE
void CVGUI_MP_LobbyScreen::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	// inlined Color::operator=() at line 296
	// inlined Color::Color() at line 297
	// inlined Color::operator=() at line 297
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:226 @0x62a120 _ZN20CVGUI_MP_LobbyScreen4InitEP9KeyValuesP20VGuiScreenInitData_t
bool CVGUI_MP_LobbyScreen::Init( KeyValues *pKeyValues, VGuiScreenInitData_t *pInitData )
{
	// inlined vgui::ivgui() at line 310
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:227 @0x62b520 _ZN20CVGUI_MP_LobbyScreen6OnTickEv
void CVGUI_MP_LobbyScreen::OnTick()
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:228 @0x62b700 _ZN20CVGUI_MP_LobbyScreen9OnCommandEPKc
void CVGUI_MP_LobbyScreen::OnCommand( const char *pCommand )
{
	// inlined CVGUI_MP_LobbyScreen::NextDay() at line 367
	// inlined CVGUI_MP_LobbyScreen::PreviousDay() at line 372
	// inlined CVGUI_MP_LobbyScreen::SelectDay() at line 373
	// inlined CObjectControlPanel::OnCommand() at line 380
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:231 @0x629ae0 _ZN20CVGUI_MP_LobbyScreen6UpdateEP16C_MP_LobbyScreen
void CVGUI_MP_LobbyScreen::Update( C_MP_LobbyScreen *pLobbyScreen )
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:232 @0x629ec0 _ZN20CVGUI_MP_LobbyScreen14UpdateDayLabelEv
void CVGUI_MP_LobbyScreen::UpdateDayLabel()
{
	char szDayName[128];  // line 273
	// inlined PortalMPGameRules() at line 274
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:234 @0x629ff0 _ZN20CVGUI_MP_LobbyScreen7NextDayEv
void CVGUI_MP_LobbyScreen::NextDay()
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:235 @0x629fc0 _ZN20CVGUI_MP_LobbyScreen11PreviousDayEv
void CVGUI_MP_LobbyScreen::PreviousDay()
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:236 @0x62b680 _ZN20CVGUI_MP_LobbyScreen9SelectDayEv
void CVGUI_MP_LobbyScreen::SelectDay()
{
	char szServerMsg[32];  // line 355
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:254
CPanelFactory<CVGUI_MP_LobbyScreen,VGuiScreenInitData_t> g_CVGUI_MP_LobbyScreenFactory;

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:422 sizeof=0x1dc (i386)
struct CVGUI_MP_ProgressSignScreen : public CVGuiScreenPanel
{
public:
	CVGUI_MP_ProgressSignScreen( vgui::Panel *, const char * );  // line 426
	virtual void ApplySchemeSettings( vgui::IScheme * );  // line 428
	virtual void PerformLayout();  // line 429
	virtual void Paint();  // line 430
	virtual bool Init( KeyValues *, VGuiScreenInitData_t * );  // line 432
	virtual void OnTick();  // line 433
protected:
	virtual void UpdateLevelLabel();  // line 436
	int m_nDay; // +0x180  // line 439
private:
	vgui::Label *m_pBranchNumLabel; // +0x184  // line 442
	vgui::Label *m_pLevelProgressNumLabel; // +0x188  // line 443
	vgui::ImagePanel *m_pLevelIconPanels[16]; // +0x18c  // line 444
	bool bIsAlreadyVisible; // +0x1cc  // line 445
	bool m_bInitializedLevelLabel; // +0x1cd  // line 446
	int m_nBackgroundTextureID; // +0x1d0  // line 447
	int m_nNumberTextureID; // +0x1d4  // line 448
	int m_nCurrentLevel; // +0x1d8  // line 450
};

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:422 (declaration)
~CVGUI_MP_ProgressSignScreen();

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:422 @0x62bf30 _ZN27CVGUI_MP_ProgressSignScreenD0Ev
CVGUI_MP_ProgressSignScreen::~CVGUI_MP_ProgressSignScreen()
{
	// inlined CVGuiScreenPanel::~CVGuiScreenPanel() at line 422
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:422 @0x62bf80 _ZN27CVGUI_MP_ProgressSignScreenD1Ev
CVGUI_MP_ProgressSignScreen::~CVGUI_MP_ProgressSignScreen()
{
	// inlined CVGuiScreenPanel::~CVGuiScreenPanel() at line 422
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:426 @0x62bb10 _ZN27CVGUI_MP_ProgressSignScreenC2EPN4vgui5PanelEPKc
CVGUI_MP_ProgressSignScreen::CVGUI_MP_ProgressSignScreen( vgui::Panel *parent, const char *panelName )
{
	{
		int i;  // line 465
	}
	// inlined CVGuiScreenPanel::~CVGuiScreenPanel() at line 469
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:426 @0x62bc00 _ZN27CVGUI_MP_ProgressSignScreenC1EPN4vgui5PanelEPKc
CVGUI_MP_ProgressSignScreen::CVGUI_MP_ProgressSignScreen( vgui::Panel *parent, const char *panelName )
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:428 @0x62a300 _ZN27CVGUI_MP_ProgressSignScreen19ApplySchemeSettingsEPN4vgui7ISchemeE
void CVGUI_MP_ProgressSignScreen::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	{
		int i;  // line 607
	}
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:429 @0x629d30 _ZN27CVGUI_MP_ProgressSignScreen13PerformLayoutEv
void CVGUI_MP_ProgressSignScreen::PerformLayout()
{
	{
		int i;  // line 621
	}
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:430 @0x62a3f0 _ZN27CVGUI_MP_ProgressSignScreen5PaintEv
void CVGUI_MP_ProgressSignScreen::Paint()
{
	int w;  // line 473
	int h;  // line 473
	int nNumHeight;  // line 481
	int nNumWidth;  // line 482
	int nNumPerLine;  // line 484
	float flNumWidthFrac;  // line 485
	float flNumX;  // line 492
	float flNumY;  // line 493
	int nMod;  // line 494
	// inlined vgui::surface() at line 502
	// inlined vgui::surface() at line 501
	// inlined vgui::surface() at line 490
	// inlined vgui::surface() at line 487
	// inlined vgui::surface() at line 479
	// inlined vgui::surface() at line 478
	// inlined vgui::surface() at line 477
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:432 @0x62a690 _ZN27CVGUI_MP_ProgressSignScreen4InitEP9KeyValuesP20VGuiScreenInitData_t
bool CVGUI_MP_ProgressSignScreen::Init( KeyValues *pKeyValues, VGuiScreenInitData_t *pInitData )
{
	// inlined vgui::surface() at line 633
	// inlined vgui::surface() at line 634
	// inlined vgui::surface() at line 636
	// inlined vgui::surface() at line 637
	// inlined vgui::ivgui() at line 640
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:433 @0x629b50 _ZN27CVGUI_MP_ProgressSignScreen6OnTickEv
void CVGUI_MP_ProgressSignScreen::OnTick()
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:436 @0x62ad30 _ZN27CVGUI_MP_ProgressSignScreen16UpdateLevelLabelEv
void CVGUI_MP_ProgressSignScreen::UpdateLevelLabel()
{
	int nDay;  // line 515
	int nTotalLevels;  // line 519
	int nLevel;  // line 521
	char szBranchNumber[128];  // line 529
	char szProgressNumbers[128];  // line 532
	int w;  // line 540
	int h;  // line 540
	// inlined PortalMPGameRules() at line 512
	// inlined PortalMPGameRules() at line 515
	// inlined C_PortalMPGameRules::GetCoopSection() at line 515
	// inlined C_PortalMPGameRules::GetBranchTotalLevelCount() at line 519
	// inlined C_PortalMPGameRules::GetCoopBranchLevel() at line 521
	{
		int i;  // line 543
		{
			int nBlockSize;  // line 545
			int nSelectExtra;  // line 546
			bool bSelected;  // line 548
			int x;  // line 575
			int y;  // line 575
			int new_x;  // line 582
			// inlined Color::Color() at line 560
			// inlined Color::Color() at line 565
		}
	}
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:453
CPanelFactory<CVGUI_MP_ProgressSignScreen,VGuiScreenInitData_t> g_CVGUI_MP_ProgressSignScreenFactory;

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:669 sizeof=0x1dc (i386)
struct CVGUI_MP_LevelSelectScreen : public CVGuiScreenPanel
{
public:
	CVGUI_MP_LevelSelectScreen( vgui::Panel *, const char * );  // line 673
	virtual void ApplySchemeSettings( vgui::IScheme * );  // line 675
	virtual void PerformLayout();  // line 676
	virtual bool Init( KeyValues *, VGuiScreenInitData_t * );  // line 678
	virtual void OnTick();  // line 679
protected:
	virtual void UpdateLevelLabel();  // line 682
	int m_nDay; // +0x180  // line 685
private:
	vgui::Label *m_pBranchTitleLabel; // +0x184  // line 688
	vgui::Label *m_pSelectLevelLabel; // +0x188  // line 689
	vgui::Label *m_pLevelNumLabel; // +0x18c  // line 690
	vgui::Label *m_pLevelTextLabel; // +0x190  // line 691
	vgui::ImagePanel *m_pLevelIconPanels[16]; // +0x194  // line 692
	bool bIsAlreadyVisible; // +0x1d4  // line 693
	bool m_bInitializedLevelLabel; // +0x1d5  // line 694
	int m_nCurrentLevel; // +0x1d8  // line 696
};

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:669 (declaration)
~CVGUI_MP_LevelSelectScreen();

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:669 @0x62bfb0 _ZN26CVGUI_MP_LevelSelectScreenD0Ev
CVGUI_MP_LevelSelectScreen::~CVGUI_MP_LevelSelectScreen()
{
	// inlined CVGuiScreenPanel::~CVGuiScreenPanel() at line 669
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:669 @0x62c000 _ZN26CVGUI_MP_LevelSelectScreenD1Ev
CVGUI_MP_LevelSelectScreen::~CVGUI_MP_LevelSelectScreen()
{
	// inlined CVGuiScreenPanel::~CVGuiScreenPanel() at line 669
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:673 @0x62b830 _ZN26CVGUI_MP_LevelSelectScreenC2EPN4vgui5PanelEPKc
CVGUI_MP_LevelSelectScreen::CVGUI_MP_LevelSelectScreen( vgui::Panel *parent, const char *panelName )
{
	{
		int i;  // line 710
	}
	// inlined CVGuiScreenPanel::~CVGuiScreenPanel() at line 714
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:673 @0x62bb00 _ZN26CVGUI_MP_LevelSelectScreenC1EPN4vgui5PanelEPKc
CVGUI_MP_LevelSelectScreen::CVGUI_MP_LevelSelectScreen( vgui::Panel *parent, const char *panelName )
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:675 @0x62a190 _ZN26CVGUI_MP_LevelSelectScreen19ApplySchemeSettingsEPN4vgui7ISchemeE
void CVGUI_MP_LevelSelectScreen::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	{
		int i;  // line 818
	}
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:676 @0x629ba0 _ZN26CVGUI_MP_LevelSelectScreen13PerformLayoutEv
void CVGUI_MP_LevelSelectScreen::PerformLayout()
{
	{
		int i;  // line 832
	}
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:678 @0x62a620 _ZN26CVGUI_MP_LevelSelectScreen4InitEP9KeyValuesP20VGuiScreenInitData_t
bool CVGUI_MP_LevelSelectScreen::Init( KeyValues *pKeyValues, VGuiScreenInitData_t *pInitData )
{
	// inlined vgui::ivgui() at line 844
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:679 @0x629b00 _ZN26CVGUI_MP_LevelSelectScreen6OnTickEv
void CVGUI_MP_LevelSelectScreen::OnTick()
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:682 @0x62a780 _ZN26CVGUI_MP_LevelSelectScreen16UpdateLevelLabelEv
void CVGUI_MP_LevelSelectScreen::UpdateLevelLabel()
{
	int nThisDay;  // line 724
	int nTotalLevels;  // line 725
	int nLevel;  // line 727
	char szLevelNumber[128];  // line 734
	char szLevelName[128];  // line 736
	int w;  // line 745
	int h;  // line 745
	// inlined PortalMPGameRules() at line 721
	// inlined PortalMPGameRules() at line 725
	// inlined C_PortalMPGameRules::GetBranchTotalLevelCount() at line 725
	// inlined C_PortalMPGameRules::GetCoopBranchLevel() at line 727
	{
		int i;  // line 747
		{
			int nBlockSize;  // line 749
			int nSelectExtra;  // line 750
			bool bSelected;  // line 752
			int x;  // line 779
			int y;  // line 779
			int new_x;  // line 786
			// inlined Color::Color() at line 764
			// inlined Color::Color() at line 769
		}
	}
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:699
CPanelFactory<CVGUI_MP_LevelSelectScreen,VGuiScreenInitData_t> g_CVGUI_MP_LevelSelectScreenFactory;

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:872 sizeof=0x1dc (i386)
struct CVGUI_MP_LevelSelectScreen2 : public CVGUI_MP_LevelSelectScreen
{
public:
	CVGUI_MP_LevelSelectScreen2( vgui::Panel *, const char * );  // line 875
};

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:872 (declaration)
~CVGUI_MP_LevelSelectScreen2();

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:872 @0x62c5c0 _ZN27CVGUI_MP_LevelSelectScreen2D0Ev
CVGUI_MP_LevelSelectScreen2::~CVGUI_MP_LevelSelectScreen2()
{
	// inlined CVGUI_MP_LevelSelectScreen::~CVGUI_MP_LevelSelectScreen() at line 872
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:872 @0x62c610 _ZN27CVGUI_MP_LevelSelectScreen2D1Ev
CVGUI_MP_LevelSelectScreen2::~CVGUI_MP_LevelSelectScreen2()
{
	// inlined CVGUI_MP_LevelSelectScreen::~CVGUI_MP_LevelSelectScreen() at line 872
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:875 @0x62ba60 _ZN27CVGUI_MP_LevelSelectScreen2C1EPN4vgui5PanelEPKc
CVGUI_MP_LevelSelectScreen2::CVGUI_MP_LevelSelectScreen2( vgui::Panel *parent, const char *panelName )
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:875 @0x62bab0 _ZN27CVGUI_MP_LevelSelectScreen2C2EPN4vgui5PanelEPKc
CVGUI_MP_LevelSelectScreen2::CVGUI_MP_LevelSelectScreen2( vgui::Panel *parent, const char *panelName )
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:878
CPanelFactory<CVGUI_MP_LevelSelectScreen2,VGuiScreenInitData_t> g_CVGUI_MP_LevelSelectScreen2Factory;

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:893 sizeof=0x1dc (i386)
struct CVGUI_MP_LevelSelectScreen3 : public CVGUI_MP_LevelSelectScreen
{
public:
	CVGUI_MP_LevelSelectScreen3( vgui::Panel *, const char * );  // line 896
};

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:893 (declaration)
~CVGUI_MP_LevelSelectScreen3();

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:893 @0x62bf00 _ZN27CVGUI_MP_LevelSelectScreen3D1Ev
CVGUI_MP_LevelSelectScreen3::~CVGUI_MP_LevelSelectScreen3()
{
	// inlined CVGUI_MP_LevelSelectScreen::~CVGUI_MP_LevelSelectScreen() at line 893
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:893 @0x62c640 _ZN27CVGUI_MP_LevelSelectScreen3D0Ev
CVGUI_MP_LevelSelectScreen3::~CVGUI_MP_LevelSelectScreen3()
{
	// inlined CVGUI_MP_LevelSelectScreen::~CVGUI_MP_LevelSelectScreen() at line 893
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:896 @0x62b9c0 _ZN27CVGUI_MP_LevelSelectScreen3C1EPN4vgui5PanelEPKc
CVGUI_MP_LevelSelectScreen3::CVGUI_MP_LevelSelectScreen3( vgui::Panel *parent, const char *panelName )
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:896 @0x62ba10 _ZN27CVGUI_MP_LevelSelectScreen3C2EPN4vgui5PanelEPKc
CVGUI_MP_LevelSelectScreen3::CVGUI_MP_LevelSelectScreen3( vgui::Panel *parent, const char *panelName )
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:899
CPanelFactory<CVGUI_MP_LevelSelectScreen3,VGuiScreenInitData_t> g_CVGUI_MP_LevelSelectScreen3Factory;

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:914 sizeof=0x1dc (i386)
struct CVGUI_MP_LevelSelectScreen4 : public CVGUI_MP_LevelSelectScreen
{
public:
	CVGUI_MP_LevelSelectScreen4( vgui::Panel *, const char * );  // line 917
};

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:914 (declaration)
~CVGUI_MP_LevelSelectScreen4();

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:914 @0x62c030 _ZN27CVGUI_MP_LevelSelectScreen4D1Ev
CVGUI_MP_LevelSelectScreen4::~CVGUI_MP_LevelSelectScreen4()
{
	// inlined CVGUI_MP_LevelSelectScreen::~CVGUI_MP_LevelSelectScreen() at line 914
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:914 @0x62c690 _ZN27CVGUI_MP_LevelSelectScreen4D0Ev
CVGUI_MP_LevelSelectScreen4::~CVGUI_MP_LevelSelectScreen4()
{
	// inlined CVGUI_MP_LevelSelectScreen::~CVGUI_MP_LevelSelectScreen() at line 914
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:917 @0x62b920 _ZN27CVGUI_MP_LevelSelectScreen4C1EPN4vgui5PanelEPKc
CVGUI_MP_LevelSelectScreen4::CVGUI_MP_LevelSelectScreen4( vgui::Panel *parent, const char *panelName )
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:917 @0x62b970 _ZN27CVGUI_MP_LevelSelectScreen4C2EPN4vgui5PanelEPKc
CVGUI_MP_LevelSelectScreen4::CVGUI_MP_LevelSelectScreen4( vgui::Panel *parent, const char *panelName )
{
}

// game/client/portal2/vgui/vgui_mp_lobby_screen.cpp:920
CPanelFactory<CVGUI_MP_LevelSelectScreen4,VGuiScreenInitData_t> g_CVGUI_MP_LevelSelectScreen4Factory;
