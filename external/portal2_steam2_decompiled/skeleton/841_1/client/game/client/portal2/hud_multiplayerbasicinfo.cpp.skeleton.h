// DWARF declaration skeleton for game/client/portal2/hud_multiplayerbasicinfo.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x807e0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
}

// game/client/portal2/hud_multiplayerbasicinfo.cpp:37 sizeof=0x1b8 (i386)
struct CHudMultiplayerBasicInfo : public vgui::EditablePanel, public CHudElement
{
public:
	// game/client/portal2/hud_multiplayerbasicinfo.cpp:38 sizeof=0x1 (i386)
	struct CHudMultiplayerBasicInfo_RegisterMap
	{
	public:
		CHudMultiplayerBasicInfo_RegisterMap();  // line 38
	};  // line 38
	// game/client/portal2/hud_multiplayerbasicinfo.cpp:38 sizeof=0x1 (i386)
	struct CHudMultiplayerBasicInfo_Register
	{
	public:
		CHudMultiplayerBasicInfo_Register();  // line 38
	};  // line 38
	// game/client/portal2/hud_multiplayerbasicinfo.cpp:38 sizeof=0x1 (i386)
	struct CHudMultiplayerBasicInfo_RegisterKBMap
	{
	public:
		CHudMultiplayerBasicInfo_RegisterKBMap();  // line 38
	};  // line 38
	CHudMultiplayerBasicInfo::CHudMultiplayerBasicInfo_RegisterMap m_RegisterClass; // +0x1b0  // line 38
	CHudMultiplayerBasicInfo::CHudMultiplayerBasicInfo_Register m_RegisterAnimationClass; // +0x1b1  // line 38
	CHudMultiplayerBasicInfo::CHudMultiplayerBasicInfo_RegisterKBMap m_RegisterClassKB; // +0x1b2  // line 38
	static void AddToMap( const char *, MessageMapItem_t::MessageFunc_t, int, int, const char *, int, const char * );  // line 38
	static void ChainToMap();  // line 38
	virtual vgui::PanelMessageMap *GetMessageMap();  // line 38
	static void AddToAnimationMap( const char *, const char *, const char *, const char *, bool, PANELLOOKUPFUNC );  // line 38
	static void ChainToAnimationMap();  // line 38
	virtual PanelAnimationMap *GetAnimMap();  // line 38
	static void KB_AddToMap( const char *, KeyCode, int, MessageMapItem_t::MessageFunc_t, const char *, const char *, bool );  // line 38
	static void KB_ChainToMap();  // line 38
	static void KB_AddBoundKey( const char *, int, int );  // line 38
	virtual vgui::PanelKeyBindingMap *GetKBMap();  // line 38
	static const char *GetPanelClassName();  // line 38
	static const char *GetPanelBaseClassName();  // line 38
	CHudMultiplayerBasicInfo( const char * );  // line 41
protected:
	virtual void Reset();  // line 46
	virtual void Init();  // line 47
	virtual void ApplySchemeSettings( vgui::IScheme * );  // line 48
	virtual void Paint();  // line 49
	virtual bool ShouldDraw();  // line 50
public:
	// game/client/portal2/hud_multiplayerbasicinfo.cpp:52 sizeof=0x1 (i386)
	struct PanelAnimationVar_m_hTextFont
	{
	public:
		static void InitVar();  // line 52
		PanelAnimationVar_m_hTextFont();  // line 52
	};  // line 52
protected:
	CHudMultiplayerBasicInfo::PanelAnimationVar_m_hTextFont m_m_hTextFont_register; // +0x1b3  // line 52
	FontHandle_t m_hTextFont; // +0x1b4  // line 52
	static void *GetVar_m_hTextFont( vgui::Panel * );  // line 52
};

// game/client/portal2/hud_multiplayerbasicinfo.cpp:37 (declaration)
~CHudMultiplayerBasicInfo();

// game/client/portal2/hud_multiplayerbasicinfo.cpp:37 @0x60f1a0 _ZN24CHudMultiplayerBasicInfoD1Ev
CHudMultiplayerBasicInfo::~CHudMultiplayerBasicInfo()
{
}

// game/client/portal2/hud_multiplayerbasicinfo.cpp:37 @0x60f580 _ZN24CHudMultiplayerBasicInfoD0Ev
CHudMultiplayerBasicInfo::~CHudMultiplayerBasicInfo()
{
	// inlined CHudElement::operator delete() at line 37
}

// game/client/portal2/hud_multiplayerbasicinfo.cpp:38 @0x60f170 _ZN24CHudMultiplayerBasicInfo17GetPanelClassNameEv
const char *CHudMultiplayerBasicInfo::GetPanelClassName()
{
}

// game/client/portal2/hud_multiplayerbasicinfo.cpp:38 @0x60f210 _ZN24CHudMultiplayerBasicInfo13KB_ChainToMapEv
void CHudMultiplayerBasicInfo::KB_ChainToMap()
{
	vgui::PanelKeyBindingMap *map;  // line 38
}

// game/client/portal2/hud_multiplayerbasicinfo.cpp:38 @0x60f270 _ZN24CHudMultiplayerBasicInfo19ChainToAnimationMapEv
void CHudMultiplayerBasicInfo::ChainToAnimationMap()
{
	PanelAnimationMap *map;  // line 38
}

// game/client/portal2/hud_multiplayerbasicinfo.cpp:38 @0x60f2d0 _ZN24CHudMultiplayerBasicInfo10ChainToMapEv
void CHudMultiplayerBasicInfo::ChainToMap()
{
	vgui::PanelMessageMap *map;  // line 38
}

// game/client/portal2/hud_multiplayerbasicinfo.cpp:38 @0x60f330 _ZN24CHudMultiplayerBasicInfo8GetKBMapEv
vgui::PanelKeyBindingMap *CHudMultiplayerBasicInfo::GetKBMap()
{
	vgui::PanelKeyBindingMap *s_pMap;  // line 38
}

// game/client/portal2/hud_multiplayerbasicinfo.cpp:38 @0x60f3b0 _ZN24CHudMultiplayerBasicInfo10GetAnimMapEv
PanelAnimationMap *CHudMultiplayerBasicInfo::GetAnimMap()
{
}

// game/client/portal2/hud_multiplayerbasicinfo.cpp:38 @0x60f3d0 _ZN24CHudMultiplayerBasicInfo13GetMessageMapEv
vgui::PanelMessageMap *CHudMultiplayerBasicInfo::GetMessageMap()
{
	vgui::PanelMessageMap *s_pMap;  // line 38
}

// game/client/portal2/hud_multiplayerbasicinfo.cpp:41 @0x60ee10 _ZN24CHudMultiplayerBasicInfoC2EPKc
CHudMultiplayerBasicInfo::CHudMultiplayerBasicInfo( const char *pElementName )
{
	{
		vgui::Panel *pParent;  // line 63
	}
	// inlined CHudMultiplayerBasicInfo::PanelAnimationVar_m_hTextFont::PanelAnimationVar_m_hTextFont() at line 61
	// inlined CHudMultiplayerBasicInfo::CHudMultiplayerBasicInfo_RegisterKBMap::CHudMultiplayerBasicInfo_RegisterKBMap() at line 61
	// inlined CHudMultiplayerBasicInfo::CHudMultiplayerBasicInfo_Register::CHudMultiplayerBasicInfo_Register() at line 61
	// inlined CHudMultiplayerBasicInfo::CHudMultiplayerBasicInfo_RegisterMap::CHudMultiplayerBasicInfo_RegisterMap() at line 61
}

// game/client/portal2/hud_multiplayerbasicinfo.cpp:41 @0x60f0d0 _ZN24CHudMultiplayerBasicInfoC1EPKc
CHudMultiplayerBasicInfo::CHudMultiplayerBasicInfo( const char *pElementName )
{
}

// game/client/portal2/hud_multiplayerbasicinfo.cpp:46 @0x60e7d0 _ZN24CHudMultiplayerBasicInfo5ResetEv
void CHudMultiplayerBasicInfo::Reset()
{
}

// game/client/portal2/hud_multiplayerbasicinfo.cpp:47 @0x60e910 _ZN24CHudMultiplayerBasicInfo4InitEv
void CHudMultiplayerBasicInfo::Init()
{
	int screenWide;  // line 76
	int screenTall;  // line 76
}

// game/client/portal2/hud_multiplayerbasicinfo.cpp:48 @0x60e860 _ZN24CHudMultiplayerBasicInfo19ApplySchemeSettingsEPN4vgui7ISchemeE
void CHudMultiplayerBasicInfo::ApplySchemeSettings( vgui::IScheme *scheme )
{
}

// game/client/portal2/hud_multiplayerbasicinfo.cpp:49 @0x60e970 _ZN24CHudMultiplayerBasicInfo5PaintEv
void CHudMultiplayerBasicInfo::Paint()
{
	C_BaseHLPlayer *pPlayer;  // line 116
	Color clrPanelBg;  // line 121
	Color clrText;  // line 139
	int iTextY;  // line 143
	{
		int iOtherTeam;  // line 172
		wchar_t text[32];  // line 173
		Color clrEnemyTeam;  // line 176
		// inlined vgui::surface() at line 187
		// inlined vgui::surface() at line 185
		// inlined vgui::surface() at line 184
		// inlined vgui::surface() at line 183
		// inlined vgui::surface() at line 182
	}
	{
		wchar_t text[32];  // line 159
		// inlined vgui::surface() at line 162
		// inlined vgui::surface() at line 163
		// inlined vgui::surface() at line 164
		// inlined vgui::surface() at line 165
		// inlined vgui::surface() at line 167
	}
	{
		wchar_t text[32];  // line 146
		// inlined vgui::surface() at line 149
		// inlined vgui::surface() at line 150
		// inlined vgui::surface() at line 151
		// inlined vgui::surface() at line 152
		// inlined vgui::surface() at line 154
	}
	// inlined Color::Color() at line 139
}

// game/client/portal2/hud_multiplayerbasicinfo.cpp:50 @0x60e800 _ZN24CHudMultiplayerBasicInfo10ShouldDrawEv
bool CHudMultiplayerBasicInfo::ShouldDraw()
{
	// inlined PortalMPGameRules() at line 108
}

// game/client/portal2/hud_multiplayerbasicinfo.cpp:52 @0x60f180 _ZN24CHudMultiplayerBasicInfo18GetVar_m_hTextFontEPN4vgui5PanelE
void *CHudMultiplayerBasicInfo::GetVar_m_hTextFont( vgui::Panel *panel )
{
}

// game/client/portal2/hud_multiplayerbasicinfo.cpp:52 @0x60f450 _ZN24CHudMultiplayerBasicInfo29PanelAnimationVar_m_hTextFont7InitVarEv
void CHudMultiplayerBasicInfo::PanelAnimationVar_m_hTextFont::InitVar()
{
	// inlined CHudMultiplayerBasicInfo::AddToAnimationMap() at line 52
}

// game/client/portal2/hud_multiplayerbasicinfo.cpp:55 @0x60f0e0 _ZL31Create_CHudMultiplayerBasicInfov
CHudElement *Create_CHudMultiplayerBasicInfo()
{
	// inlined CHudElement::operator new() at line 55
	// inlined CHudMultiplayerBasicInfo::CHudMultiplayerBasicInfo() at line 55
	// inlined CHudElement::operator delete() at line 55
}

// game/client/portal2/hud_multiplayerbasicinfo.cpp:55
static CHudElementHelper g_CHudMultiplayerBasicInfo_Helper;
