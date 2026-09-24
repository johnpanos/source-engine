// DWARF declaration skeleton for game/client/portal2/vgui/portalclientscoreboard.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal2/vgui/portalclientscoreboard.h:20 sizeof=0x2d8 (i386)
struct CPortalClientScoreBoardDialog : public CClientScoreBoardDialog
{
public:
	// game/client/portal2/vgui/portalclientscoreboard.h:22 sizeof=0x1 (i386)
	struct CPortalClientScoreBoardDialog_RegisterMap
	{
	public:
		CPortalClientScoreBoardDialog_RegisterMap();  // line 22
	};  // line 22
	// game/client/portal2/vgui/portalclientscoreboard.h:22 sizeof=0x1 (i386)
	struct CPortalClientScoreBoardDialog_Register
	{
	public:
		CPortalClientScoreBoardDialog_Register();  // line 22
	};  // line 22
	// game/client/portal2/vgui/portalclientscoreboard.h:22 sizeof=0x1 (i386)
	struct CPortalClientScoreBoardDialog_RegisterKBMap
	{
	public:
		CPortalClientScoreBoardDialog_RegisterKBMap();  // line 22
	};  // line 22
	CPortalClientScoreBoardDialog::CPortalClientScoreBoardDialog_RegisterMap m_RegisterClass; // +0x2ac  // line 22
	CPortalClientScoreBoardDialog::CPortalClientScoreBoardDialog_Register m_RegisterAnimationClass; // +0x2ad  // line 22
	CPortalClientScoreBoardDialog::CPortalClientScoreBoardDialog_RegisterKBMap m_RegisterClassKB; // +0x2ae  // line 22
	static void AddToMap( const char *, MessageMapItem_t::MessageFunc_t, int, int, const char *, int, const char * );  // line 22
	static void ChainToMap();  // line 22
	virtual vgui::PanelMessageMap *GetMessageMap();  // line 22
	static void AddToAnimationMap( const char *, const char *, const char *, const char *, bool, PANELLOOKUPFUNC );  // line 22
	static void ChainToAnimationMap();  // line 22
	virtual PanelAnimationMap *GetAnimMap();  // line 22
	static void KB_AddToMap( const char *, KeyCode, int, MessageMapItem_t::MessageFunc_t, const char *, const char *, bool );  // line 22
	static void KB_ChainToMap();  // line 22
	static void KB_AddBoundKey( const char *, int, int );  // line 22
	virtual vgui::PanelKeyBindingMap *GetKBMap();  // line 22
	static const char *GetPanelClassName();  // line 22
	static const char *GetPanelBaseClassName();  // line 22
	CPortalClientScoreBoardDialog( IViewPort * );  // line 25
	virtual ~CPortalClientScoreBoardDialog();  // line 26
	virtual void Reset();  // line 28
	virtual void Update();  // line 29
	virtual void PaintBackground();  // line 32
	virtual void PaintBorder();  // line 33
	virtual void ApplySchemeSettings( vgui::IScheme * );  // line 34
private:
	void InitPlayerList( vgui::SectionedListPanel *, int );  // line 37
	virtual void UpdateTeamInfo();  // line 38
	void UpdatePlayerList();  // line 39
	void UpdateSpectatorList();  // line 40
	virtual bool GetPlayerScoreInfo( int, KeyValues * );  // line 41
	bool ShouldShowAsSpectator( int );  // line 43
	virtual void FireGameEvent( IGameEvent * );  // line 44
	static bool PortalPlayerSortFunc( vgui::SectionedListPanel *, int, int );  // line 46
	Color m_bgColor; // +0x2af  // line 49
	Color m_borderColor; // +0x2b3  // line 50
	vgui::SectionedListPanel *m_pPlayerListT; // +0x2b8  // line 53
	vgui::SectionedListPanel *m_pPlayerListCT; // +0x2bc  // line 54
	vgui::Label *m_pPlayerCountLabel_T; // +0x2c0  // line 56
	vgui::Label *m_pScoreLabel_T; // +0x2c4  // line 57
	vgui::Label *m_pPingLabel_T; // +0x2c8  // line 58
	vgui::Label *m_pPlayerCountLabel_CT; // +0x2cc  // line 59
	vgui::Label *m_pScoreLabel_CT; // +0x2d0  // line 60
	vgui::Label *m_pPingLabel_CT; // +0x2d4  // line 61
};

// game/client/portal2/vgui/portalclientscoreboard.h:22 @0x617d00 _ZN29CPortalClientScoreBoardDialog17GetPanelClassNameEv
const char *CPortalClientScoreBoardDialog::GetPanelClassName()
{
}

// game/client/portal2/vgui/portalclientscoreboard.h:22 @0x617d10 _ZN29CPortalClientScoreBoardDialog13KB_ChainToMapEv
void CPortalClientScoreBoardDialog::KB_ChainToMap()
{
	vgui::PanelKeyBindingMap *map;  // line 22
}

// game/client/portal2/vgui/portalclientscoreboard.h:22 @0x617d70 _ZN29CPortalClientScoreBoardDialog19ChainToAnimationMapEv
void CPortalClientScoreBoardDialog::ChainToAnimationMap()
{
	PanelAnimationMap *map;  // line 22
}

// game/client/portal2/vgui/portalclientscoreboard.h:22 @0x617dd0 _ZN29CPortalClientScoreBoardDialog10ChainToMapEv
void CPortalClientScoreBoardDialog::ChainToMap()
{
	vgui::PanelMessageMap *map;  // line 22
}

// game/client/portal2/vgui/portalclientscoreboard.h:22 @0x617e30 _ZN29CPortalClientScoreBoardDialog8GetKBMapEv
vgui::PanelKeyBindingMap *CPortalClientScoreBoardDialog::GetKBMap()
{
	vgui::PanelKeyBindingMap *s_pMap;  // line 22
}

// game/client/portal2/vgui/portalclientscoreboard.h:22 @0x617eb0 _ZN29CPortalClientScoreBoardDialog10GetAnimMapEv
PanelAnimationMap *CPortalClientScoreBoardDialog::GetAnimMap()
{
}

// game/client/portal2/vgui/portalclientscoreboard.h:22 @0x617ed0 _ZN29CPortalClientScoreBoardDialog13GetMessageMapEv
vgui::PanelMessageMap *CPortalClientScoreBoardDialog::GetMessageMap()
{
	vgui::PanelMessageMap *s_pMap;  // line 22
}
