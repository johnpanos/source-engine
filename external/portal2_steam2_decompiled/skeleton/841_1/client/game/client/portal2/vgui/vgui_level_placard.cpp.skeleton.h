// DWARF declaration skeleton for game/client/portal2/vgui/vgui_level_placard.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x81760 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CPanelFactory<CLevelPlacardScreen,VGuiScreenInitData_t>::CPanelFactory() at line 53
}

// game/client/portal2/vgui/vgui_level_placard.cpp:26 sizeof=0x18c (i386)
struct CLevelPlacardScreen : public CVGuiScreenPanel
{
public:
	CLevelPlacardScreen( vgui::Panel *, const char * );  // line 30
	virtual ~CLevelPlacardScreen();  // line 31
	virtual void ApplySchemeSettings( vgui::IScheme * );  // line 33
	virtual bool Init( KeyValues *, VGuiScreenInitData_t * );  // line 35
	virtual void OnTick();  // line 36
	virtual void Paint();  // line 37
private:
	bool IsActive();  // line 40
	CHandle<C_VGuiScreen> m_hVGUIScreen; // +0x180  // line 45
	int m_nTextureId; // +0x184  // line 47
	vgui::Label *m_pDisplayTextLabel; // +0x188  // line 50
};

// game/client/portal2/vgui/vgui_level_placard.cpp:30 @0x6297a0 _ZN19CLevelPlacardScreenC2EPN4vgui5PanelEPKc
CLevelPlacardScreen::CLevelPlacardScreen( vgui::Panel *parent, const char *panelName )
{
	// inlined CHandle<C_VGuiScreen>::CHandle() at line 59
	// inlined vgui::scheme() at line 59
	// inlined CVGuiScreenPanel::~CVGuiScreenPanel() at line 64
}

// game/client/portal2/vgui/vgui_level_placard.cpp:30 @0x6298b0 _ZN19CLevelPlacardScreenC1EPN4vgui5PanelEPKc
CLevelPlacardScreen::CLevelPlacardScreen( vgui::Panel *parent, const char *panelName )
{
}

// game/client/portal2/vgui/vgui_level_placard.cpp:31 @0x629720 _ZN19CLevelPlacardScreenD2Ev
CLevelPlacardScreen::~CLevelPlacardScreen()
{
	// inlined CVGuiScreenPanel::~CVGuiScreenPanel() at line 73
	// inlined CVGuiScreenPanel::~CVGuiScreenPanel() at line 73
}

// game/client/portal2/vgui/vgui_level_placard.cpp:31 @0x6298c0 _ZN19CLevelPlacardScreenD0Ev
CLevelPlacardScreen::~CLevelPlacardScreen()
{
	// inlined CVGuiScreenPanel::~CVGuiScreenPanel() at line 73
	// inlined CVGuiScreenPanel::~CVGuiScreenPanel() at line 73
}

// game/client/portal2/vgui/vgui_level_placard.cpp:31 @0x629950 _ZN19CLevelPlacardScreenD1Ev
CLevelPlacardScreen::~CLevelPlacardScreen()
{
	// inlined CVGuiScreenPanel::~CVGuiScreenPanel() at line 73
	// inlined CVGuiScreenPanel::~CVGuiScreenPanel() at line 73
}

// game/client/portal2/vgui/vgui_level_placard.cpp:33 @0x6294a0 _ZN19CLevelPlacardScreen19ApplySchemeSettingsEPN4vgui7ISchemeE
void CLevelPlacardScreen::ApplySchemeSettings( vgui::IScheme *pScheme )
{
}

// game/client/portal2/vgui/vgui_level_placard.cpp:35 @0x629520 _ZN19CLevelPlacardScreen4InitEP9KeyValuesP20VGuiScreenInitData_t
bool CLevelPlacardScreen::Init( KeyValues *pKeyValues, VGuiScreenInitData_t *pInitData )
{
	// inlined vgui::ivgui() at line 93
	// inlined CVGuiScreenPanel::GetEntity() at line 99
	// inlined CHandle<C_VGuiScreen>::operator=() at line 99
}

// game/client/portal2/vgui/vgui_level_placard.cpp:36 @0x629510 _ZN19CLevelPlacardScreen6OnTickEv
void CLevelPlacardScreen::OnTick()
{
}

// game/client/portal2/vgui/vgui_level_placard.cpp:37 @0x629620 _ZN19CLevelPlacardScreen5PaintEv
void CLevelPlacardScreen::Paint()
{
	// inlined vgui::surface() at line 143
	// inlined vgui::surface() at line 144
}

// game/client/portal2/vgui/vgui_level_placard.cpp:40 @0x6296c0 _ZN19CLevelPlacardScreen8IsActiveEv
bool CLevelPlacardScreen::IsActive()
{
	bool bScreenActive;  // line 114
	// inlined CHandle<C_VGuiScreen>::operator!=() at line 115
}

// game/client/portal2/vgui/vgui_level_placard.cpp:53
CPanelFactory<CLevelPlacardScreen,VGuiScreenInitData_t> g_CLevelPlacardScreenFactory;
