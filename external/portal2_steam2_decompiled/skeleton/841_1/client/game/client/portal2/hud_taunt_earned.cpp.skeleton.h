// DWARF declaration skeleton for game/client/portal2/hud_taunt_earned.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x808d0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
}

// game/client/portal2/hud_taunt_earned.cpp:20
ConVar cl_hud_taunt_earned_show;

// game/client/portal2/hud_taunt_earned.cpp:26 sizeof=0x1c4 (i386)
struct CHUDTauntEarned : public CHudElement, public vgui::EditablePanel
{
public:
	// game/client/portal2/hud_taunt_earned.cpp:27 sizeof=0x1 (i386)
	struct CHUDTauntEarned_RegisterMap
	{
	public:
		CHUDTauntEarned_RegisterMap();  // line 27
	};  // line 27
	// game/client/portal2/hud_taunt_earned.cpp:27 sizeof=0x1 (i386)
	struct CHUDTauntEarned_Register
	{
	public:
		CHUDTauntEarned_Register();  // line 27
	};  // line 27
	// game/client/portal2/hud_taunt_earned.cpp:27 sizeof=0x1 (i386)
	struct CHUDTauntEarned_RegisterKBMap
	{
	public:
		CHUDTauntEarned_RegisterKBMap();  // line 27
	};  // line 27
	CHUDTauntEarned::CHUDTauntEarned_RegisterMap m_RegisterClass; // +0x1b0  // line 27
	CHUDTauntEarned::CHUDTauntEarned_Register m_RegisterAnimationClass; // +0x1b1  // line 27
	CHUDTauntEarned::CHUDTauntEarned_RegisterKBMap m_RegisterClassKB; // +0x1b2  // line 27
	static void AddToMap( const char *, MessageMapItem_t::MessageFunc_t, int, int, const char *, int, const char * );  // line 27
	static void ChainToMap();  // line 27
	virtual vgui::PanelMessageMap *GetMessageMap();  // line 27
	static void AddToAnimationMap( const char *, const char *, const char *, const char *, bool, PANELLOOKUPFUNC );  // line 27
	static void ChainToAnimationMap();  // line 27
	virtual PanelAnimationMap *GetAnimMap();  // line 27
	static void KB_AddToMap( const char *, KeyCode, int, MessageMapItem_t::MessageFunc_t, const char *, const char *, bool );  // line 27
	static void KB_ChainToMap();  // line 27
	static void KB_AddBoundKey( const char *, int, int );  // line 27
	virtual vgui::PanelKeyBindingMap *GetKBMap();  // line 27
	static const char *GetPanelClassName();  // line 27
	static const char *GetPanelBaseClassName();  // line 27
	CHUDTauntEarned( const char * );  // line 29
	virtual void OnTick();  // line 31
	virtual void PaintBackground();  // line 33
	virtual void Paint();  // line 34
	virtual void ApplySchemeSettings( vgui::IScheme * );  // line 36
private:
	vgui::Label *m_pGestureLabel; // +0x1b4  // line 40
	vgui::ImagePanel *m_pGestureImage; // +0x1b8  // line 41
	Color m_rgbaCurrent; // +0x1bc  // line 42
	bool m_bActive; // +0x1c0  // line 44
};

// game/client/portal2/hud_taunt_earned.cpp:26 (declaration)
~CHUDTauntEarned();

// game/client/portal2/hud_taunt_earned.cpp:26 @0x610300 _ZN15CHUDTauntEarnedD0Ev
CHUDTauntEarned::~CHUDTauntEarned()
{
	// inlined CHudElement::operator delete() at line 26
}

// game/client/portal2/hud_taunt_earned.cpp:26 @0x610390 _ZN15CHUDTauntEarnedD1Ev
CHUDTauntEarned::~CHUDTauntEarned()
{
}

// game/client/portal2/hud_taunt_earned.cpp:27 @0x610070 _ZN15CHUDTauntEarned17GetPanelClassNameEv
const char *CHUDTauntEarned::GetPanelClassName()
{
}

// game/client/portal2/hud_taunt_earned.cpp:27 @0x610080 _ZN15CHUDTauntEarned13KB_ChainToMapEv
void CHUDTauntEarned::KB_ChainToMap()
{
	vgui::PanelKeyBindingMap *map;  // line 27
}

// game/client/portal2/hud_taunt_earned.cpp:27 @0x6100e0 _ZN15CHUDTauntEarned19ChainToAnimationMapEv
void CHUDTauntEarned::ChainToAnimationMap()
{
	PanelAnimationMap *map;  // line 27
}

// game/client/portal2/hud_taunt_earned.cpp:27 @0x610140 _ZN15CHUDTauntEarned10ChainToMapEv
void CHUDTauntEarned::ChainToMap()
{
	vgui::PanelMessageMap *map;  // line 27
}

// game/client/portal2/hud_taunt_earned.cpp:27 @0x6101b0 _ZN15CHUDTauntEarned8GetKBMapEv
vgui::PanelKeyBindingMap *CHUDTauntEarned::GetKBMap()
{
	vgui::PanelKeyBindingMap *s_pMap;  // line 27
}

// game/client/portal2/hud_taunt_earned.cpp:27 @0x610240 _ZN15CHUDTauntEarned10GetAnimMapEv
PanelAnimationMap *CHUDTauntEarned::GetAnimMap()
{
}

// game/client/portal2/hud_taunt_earned.cpp:27 @0x610270 _ZN15CHUDTauntEarned13GetMessageMapEv
vgui::PanelMessageMap *CHUDTauntEarned::GetMessageMap()
{
	vgui::PanelMessageMap *s_pMap;  // line 27
}

// game/client/portal2/hud_taunt_earned.cpp:29 @0x60f8c0 _ZN15CHUDTauntEarnedC2EPKc
CHUDTauntEarned::CHUDTauntEarned( const char *pElementName )
{
	// inlined vgui::ivgui() at line 56
	// inlined Color::Color() at line 51
	// inlined CHUDTauntEarned::CHUDTauntEarned_RegisterKBMap::CHUDTauntEarned_RegisterKBMap() at line 51
	// inlined CHUDTauntEarned::CHUDTauntEarned_Register::CHUDTauntEarned_Register() at line 51
	// inlined CHUDTauntEarned::CHUDTauntEarned_RegisterMap::CHUDTauntEarned_RegisterMap() at line 51
}

// game/client/portal2/hud_taunt_earned.cpp:29 @0x60fae0 _ZN15CHUDTauntEarnedC1EPKc
CHUDTauntEarned::CHUDTauntEarned( const char *pElementName )
{
}

// game/client/portal2/hud_taunt_earned.cpp:31 @0x60fb90 _ZN15CHUDTauntEarned6OnTickEv
void CHUDTauntEarned::OnTick()
{
	bool bVisible;  // line 75
	int nUnusedCount;  // line 77
	// inlined Color::operator=() at line 98
	// inlined Color::Color() at line 98
	// inlined ConVar::GetBool() at line 75
	{
		const TauntStatusData *pTaunt;  // line 81
		{
			int nFrame;  // line 86
		}
	}
	{
		CLocalPlayerFilter filter;  // line 105
		// inlined CLocalPlayerFilter::~CLocalPlayerFilter() at line 114
		// inlined CLocalPlayerFilter::~CLocalPlayerFilter() at line 114
	}
}

// game/client/portal2/hud_taunt_earned.cpp:33 @0x60f6a0 _ZN15CHUDTauntEarned15PaintBackgroundEv
void CHUDTauntEarned::PaintBackground()
{
	const int nSpacing;  // line 129
	const int nSize;  // line 130
	const int nYBottom;  // line 131
	const int nXRight;  // line 132
	int nX;  // line 134
	int nOffset;  // line 135
	int nY;  // line 154
	// inlined vgui::surface() at line 127
	{
		int nXStart;  // line 139
		// inlined vgui::surface() at line 142
		// inlined vgui::surface() at line 148
	}
	{
		int nYStart;  // line 158
		// inlined vgui::surface() at line 161
		// inlined vgui::surface() at line 167
	}
}

// game/client/portal2/hud_taunt_earned.cpp:34 @0x60f610 _ZN15CHUDTauntEarned5PaintEv
void CHUDTauntEarned::Paint()
{
}

// game/client/portal2/hud_taunt_earned.cpp:36 @0x60f630 _ZN15CHUDTauntEarned19ApplySchemeSettingsEPN4vgui7ISchemeE
void CHUDTauntEarned::ApplySchemeSettings( vgui::IScheme *pScheme )
{
}

// game/client/portal2/hud_taunt_earned.cpp:48 @0x60faf0 _ZL22Create_CHUDTauntEarnedv
CHudElement *Create_CHUDTauntEarned()
{
	// inlined CHudElement::operator new() at line 48
	// inlined CHUDTauntEarned::CHUDTauntEarned() at line 48
	// inlined CHudElement::operator delete() at line 48
}

// game/client/portal2/hud_taunt_earned.cpp:48
static CHudElementHelper g_CHUDTauntEarned_Helper;
