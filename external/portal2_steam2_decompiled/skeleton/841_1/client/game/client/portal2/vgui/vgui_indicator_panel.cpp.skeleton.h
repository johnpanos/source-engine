// DWARF declaration skeleton for game/client/portal2/vgui/vgui_indicator_panel.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x81660 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CPanelFactory<CIndicatorScreen,VGuiScreenInitData_t>::CPanelFactory() at line 66
}

// game/client/portal2/vgui/vgui_indicator_panel.cpp:27 sizeof=0x1b0 (i386)
struct CIndicatorScreen : public CVGuiScreenPanel
{
public:
	CIndicatorScreen( vgui::Panel *, const char * );  // line 31
	virtual void ApplySchemeSettings( vgui::IScheme * );  // line 33
	virtual bool Init( KeyValues *, VGuiScreenInitData_t * );  // line 35
	virtual void OnTick();  // line 36
	virtual void Paint();  // line 37
private:
	void PaintCountdownTimer();  // line 40
	void PaintIndicator();  // line 41
	int m_nCheckTextureID; // +0x180  // line 43
	int m_nUncheckedTextureID; // +0x184  // line 44
	int m_nXTextureID; // +0x188  // line 46
	int m_nOTextureID; // +0x18c  // line 47
	int m_nTimerBackgroundTextureID; // +0x190  // line 49
	int m_nTimerBackgroundActiveTextureID; // +0x194  // line 52
	int m_nTimerUpperSliceTextureID; // +0x198  // line 53
	int m_nTimerLowerSliceTextureID; // +0x19c  // line 54
	bool m_bWasCounting; // +0x1a0  // line 57
	float m_flFadeUpTime; // +0x1a4  // line 58
	CHandle<C_VGuiScreen> m_hVGUIScreen; // +0x1a8  // line 61
	CHandle<C_LabIndicatorPanel> m_hScreenEntity; // +0x1ac  // line 62
};

// game/client/portal2/vgui/vgui_indicator_panel.cpp:27 (declaration)
~CIndicatorScreen();

// game/client/portal2/vgui/vgui_indicator_panel.cpp:27 @0x629320 _ZN16CIndicatorScreenD1Ev
CIndicatorScreen::~CIndicatorScreen()
{
	// inlined CVGuiScreenPanel::~CVGuiScreenPanel() at line 27
}

// game/client/portal2/vgui/vgui_indicator_panel.cpp:27 @0x629350 _ZN16CIndicatorScreenD0Ev
CIndicatorScreen::~CIndicatorScreen()
{
	// inlined CVGuiScreenPanel::~CVGuiScreenPanel() at line 27
}

// game/client/portal2/vgui/vgui_indicator_panel.cpp:31 @0x627db0 _ZN16CIndicatorScreenC2EPN4vgui5PanelEPKc
CIndicatorScreen::CIndicatorScreen( vgui::Panel *parent, const char *panelName )
{
	// inlined vgui::scheme() at line 72
	// inlined CHandle<C_VGuiScreen>::CHandle() at line 72
	// inlined CHandle<C_LabIndicatorPanel>::CHandle() at line 72
}

// game/client/portal2/vgui/vgui_indicator_panel.cpp:31 @0x629260 _ZN16CIndicatorScreenC1EPN4vgui5PanelEPKc
CIndicatorScreen::CIndicatorScreen( vgui::Panel *parent, const char *panelName )
{
	// inlined CIndicatorScreen::CIndicatorScreen() at line 76
}

// game/client/portal2/vgui/vgui_indicator_panel.cpp:33 @0x627bc0 _ZN16CIndicatorScreen19ApplySchemeSettingsEPN4vgui7ISchemeE
void CIndicatorScreen::ApplySchemeSettings( vgui::IScheme *pScheme )
{
}

// game/client/portal2/vgui/vgui_indicator_panel.cpp:35 @0x628e90 _ZN16CIndicatorScreen4InitEP9KeyValuesP20VGuiScreenInitData_t
bool CIndicatorScreen::Init( KeyValues *pKeyValues, VGuiScreenInitData_t *pInitData )
{
	// inlined vgui::ivgui() at line 89
	// inlined CVGuiScreenPanel::GetEntity() at line 95
	// inlined CHandle<C_VGuiScreen>::operator=() at line 95
	// inlined vgui::surface() at line 111
	// inlined vgui::surface() at line 112
	// inlined vgui::surface() at line 114
	// inlined vgui::surface() at line 115
	// inlined vgui::surface() at line 118
	// inlined vgui::surface() at line 119
	// inlined vgui::surface() at line 121
	// inlined vgui::surface() at line 122
	// inlined vgui::surface() at line 127
	// inlined vgui::surface() at line 128
	// inlined vgui::surface() at line 130
	// inlined vgui::surface() at line 131
	// inlined vgui::surface() at line 133
	// inlined vgui::surface() at line 134
	// inlined vgui::surface() at line 136
	// inlined vgui::surface() at line 137
	// inlined CHandle<C_VGuiScreen>::operator!=() at line 96
	// inlined C_BaseEntity::GetOwnerEntity() at line 99
	// inlined CHandle<C_LabIndicatorPanel>::operator=() at line 99
}

// game/client/portal2/vgui/vgui_indicator_panel.cpp:36 @0x627da0 _ZN16CIndicatorScreen6OnTickEv
void CIndicatorScreen::OnTick()
{
}

// game/client/portal2/vgui/vgui_indicator_panel.cpp:37 @0x628df0 _ZN16CIndicatorScreen5PaintEv
void CIndicatorScreen::Paint()
{
	// inlined CHandle<C_LabIndicatorPanel>::operator==() at line 398
}

// game/client/portal2/vgui/vgui_indicator_panel.cpp:40 @0x627e70 _ZN16CIndicatorScreen19PaintCountdownTimerEv
void CIndicatorScreen::PaintCountdownTimer()
{
	const int NUM_PIE_SECTIONS;  // line 197
	float flPerc;  // line 200
	int nWhole;  // line 201
	float flFraction;  // line 202
	float flAlpha;  // line 203
	int cx;  // line 206
	int cy;  // line 207
	bool bTimedOut;  // line 212
	float flGlobalAlpha;  // line 225
	// inlined vgui::surface() at line 354
	// inlined vgui::surface() at line 353
	// inlined Color::Color() at line 352
	// inlined vgui::surface() at line 352
	// inlined vgui::surface() at line 216
	// inlined vgui::surface() at line 251
	// inlined RemapValClamped() at line 239
	// inlined vgui::surface() at line 223
	// inlined vgui::surface() at line 220
	// inlined CHandle<C_LabIndicatorPanel>::operator->() at line 214
	// inlined vgui::surface() at line 210
	// inlined RemapValClamped() at line 203
	// inlined CHandle<C_LabIndicatorPanel>::operator->() at line 200
	// inlined Color::Color() at line 251
	// inlined vgui::surface() at line 252
	// inlined vgui::surface() at line 253
	// inlined vgui::surface() at line 265
	// inlined Color::Color() at line 265
	// inlined vgui::surface() at line 266
	// inlined vgui::surface() at line 267
	// inlined vgui::surface() at line 294
	// inlined Color::Color() at line 294
	// inlined vgui::surface() at line 295
	// inlined vgui::surface() at line 296
	// inlined vgui::surface() at line 280
	// inlined Color::Color() at line 280
	// inlined vgui::surface() at line 281
	// inlined vgui::surface() at line 282
	// inlined vgui::surface() at line 323
	// inlined Color::Color() at line 323
	// inlined vgui::surface() at line 324
	// inlined vgui::surface() at line 325
	// inlined vgui::surface() at line 309
	// inlined Color::Color() at line 309
	// inlined vgui::surface() at line 310
	// inlined vgui::surface() at line 311
	// inlined vgui::surface() at line 338
	// inlined Color::Color() at line 338
	// inlined vgui::surface() at line 339
	// inlined vgui::surface() at line 340
	// inlined vgui::surface() at line 346
	// inlined Color::Color() at line 346
	// inlined vgui::surface() at line 332
	// inlined Color::Color() at line 332
	// inlined vgui::surface() at line 317
	// inlined Color::Color() at line 317
	// inlined vgui::surface() at line 303
	// inlined Color::Color() at line 303
	// inlined vgui::surface() at line 288
	// inlined Color::Color() at line 288
	// inlined vgui::surface() at line 289
	// inlined vgui::surface() at line 290
	// inlined vgui::surface() at line 274
	// inlined Color::Color() at line 274
	// inlined vgui::surface() at line 275
	// inlined vgui::surface() at line 276
	// inlined vgui::surface() at line 259
	// inlined Color::Color() at line 259
	// inlined vgui::surface() at line 260
	// inlined vgui::surface() at line 261
}

// game/client/portal2/vgui/vgui_indicator_panel.cpp:41 @0x627bd0 _ZN16CIndicatorScreen14PaintIndicatorEv
void CIndicatorScreen::PaintIndicator()
{
	int nTextureID;  // line 365
	// inlined vgui::surface() at line 387
	// inlined vgui::surface() at line 386
	// inlined vgui::surface() at line 385
	// inlined CHandle<C_LabIndicatorPanel>::operator->() at line 380
	// inlined CHandle<C_LabIndicatorPanel>::operator->() at line 367
	// inlined vgui::surface() at line 370
	// inlined vgui::surface() at line 371
}

// game/client/portal2/vgui/vgui_indicator_panel.cpp:66
CPanelFactory<CIndicatorScreen,VGuiScreenInitData_t> g_CIndicatorScreenFactory;
