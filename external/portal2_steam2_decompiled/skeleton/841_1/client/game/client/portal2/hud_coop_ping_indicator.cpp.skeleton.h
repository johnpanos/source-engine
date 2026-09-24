// DWARF declaration skeleton for game/client/portal2/hud_coop_ping_indicator.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x80590 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
}

// game/client/portal2/hud_coop_ping_indicator.cpp:26
ConVar cl_coop_ping_indicator_scale;

// game/client/portal2/hud_coop_ping_indicator.cpp:29 sizeof=0x198 (i386)
struct CHudCoopPingIndicator : public CHudElement, public vgui::Panel
{
public:
	// game/client/portal2/hud_coop_ping_indicator.cpp:30 sizeof=0x1 (i386)
	struct CHudCoopPingIndicator_RegisterMap
	{
	public:
		CHudCoopPingIndicator_RegisterMap();  // line 30
	};  // line 30
	// game/client/portal2/hud_coop_ping_indicator.cpp:30 sizeof=0x1 (i386)
	struct CHudCoopPingIndicator_Register
	{
	public:
		CHudCoopPingIndicator_Register();  // line 30
	};  // line 30
	// game/client/portal2/hud_coop_ping_indicator.cpp:30 sizeof=0x1 (i386)
	struct CHudCoopPingIndicator_RegisterKBMap
	{
	public:
		CHudCoopPingIndicator_RegisterKBMap();  // line 30
	};  // line 30
	CHudCoopPingIndicator::CHudCoopPingIndicator_RegisterMap m_RegisterClass; // +0x180  // line 30
	CHudCoopPingIndicator::CHudCoopPingIndicator_Register m_RegisterAnimationClass; // +0x181  // line 30
	CHudCoopPingIndicator::CHudCoopPingIndicator_RegisterKBMap m_RegisterClassKB; // +0x182  // line 30
	static void AddToMap( const char *, MessageMapItem_t::MessageFunc_t, int, int, const char *, int, const char * );  // line 30
	static void ChainToMap();  // line 30
	virtual vgui::PanelMessageMap *GetMessageMap();  // line 30
	static void AddToAnimationMap( const char *, const char *, const char *, const char *, bool, PANELLOOKUPFUNC );  // line 30
	static void ChainToAnimationMap();  // line 30
	virtual PanelAnimationMap *GetAnimMap();  // line 30
	static void KB_AddToMap( const char *, KeyCode, int, MessageMapItem_t::MessageFunc_t, const char *, const char *, bool );  // line 30
	static void KB_ChainToMap();  // line 30
	static void KB_AddBoundKey( const char *, int, int );  // line 30
	virtual vgui::PanelKeyBindingMap *GetKBMap();  // line 30
	static const char *GetPanelClassName();  // line 30
	static const char *GetPanelBaseClassName();  // line 30
	CHudCoopPingIndicator( const char * );  // line 33
	virtual void Init();  // line 34
	virtual void LevelInit();  // line 35
	virtual void ApplySchemeSettings( vgui::IScheme * );  // line 36
	virtual bool ShouldDraw();  // line 37
	virtual void Paint();  // line 38
	void MsgFunc_HudPingIndicator( bf_read & );  // line 40
private:
	void DrawIndicatorHint();  // line 43
	float m_flStartDisplayTime; // +0x184  // line 45
	Vector m_vecPingLocation; // +0x188  // line 46
	int m_nArrowTexture; // +0x194  // line 48
};

// game/client/portal2/hud_coop_ping_indicator.cpp:29 (declaration)
~CHudCoopPingIndicator();

// game/client/portal2/hud_coop_ping_indicator.cpp:29 @0x60c350 _ZN21CHudCoopPingIndicatorD0Ev
CHudCoopPingIndicator::~CHudCoopPingIndicator()
{
	// inlined CHudElement::operator delete() at line 29
}

// game/client/portal2/hud_coop_ping_indicator.cpp:29 @0x60c3e0 _ZN21CHudCoopPingIndicatorD1Ev
CHudCoopPingIndicator::~CHudCoopPingIndicator()
{
}

// game/client/portal2/hud_coop_ping_indicator.cpp:30 @0x60c0c0 _ZN21CHudCoopPingIndicator17GetPanelClassNameEv
const char *CHudCoopPingIndicator::GetPanelClassName()
{
}

// game/client/portal2/hud_coop_ping_indicator.cpp:30 @0x60c0d0 _ZN21CHudCoopPingIndicator13KB_ChainToMapEv
void CHudCoopPingIndicator::KB_ChainToMap()
{
	vgui::PanelKeyBindingMap *map;  // line 30
}

// game/client/portal2/hud_coop_ping_indicator.cpp:30 @0x60c130 _ZN21CHudCoopPingIndicator19ChainToAnimationMapEv
void CHudCoopPingIndicator::ChainToAnimationMap()
{
	PanelAnimationMap *map;  // line 30
}

// game/client/portal2/hud_coop_ping_indicator.cpp:30 @0x60c190 _ZN21CHudCoopPingIndicator10ChainToMapEv
void CHudCoopPingIndicator::ChainToMap()
{
	vgui::PanelMessageMap *map;  // line 30
}

// game/client/portal2/hud_coop_ping_indicator.cpp:30 @0x60c200 _ZN21CHudCoopPingIndicator13GetMessageMapEv
vgui::PanelMessageMap *CHudCoopPingIndicator::GetMessageMap()
{
	vgui::PanelMessageMap *s_pMap;  // line 30
}

// game/client/portal2/hud_coop_ping_indicator.cpp:30 @0x60c290 _ZN21CHudCoopPingIndicator8GetKBMapEv
vgui::PanelKeyBindingMap *CHudCoopPingIndicator::GetKBMap()
{
	vgui::PanelKeyBindingMap *s_pMap;  // line 30
}

// game/client/portal2/hud_coop_ping_indicator.cpp:30 @0x60c320 _ZN21CHudCoopPingIndicator10GetAnimMapEv
PanelAnimationMap *CHudCoopPingIndicator::GetAnimMap()
{
}

// game/client/portal2/hud_coop_ping_indicator.cpp:33 @0x60beb0 _ZN21CHudCoopPingIndicatorC2EPKc
CHudCoopPingIndicator::CHudCoopPingIndicator( const char *pElementName )
{
	{
		vgui::Panel *pParent;  // line 57
	}
	// inlined CHudCoopPingIndicator::CHudCoopPingIndicator_RegisterKBMap::CHudCoopPingIndicator_RegisterKBMap() at line 55
	// inlined CHudCoopPingIndicator::CHudCoopPingIndicator_Register::CHudCoopPingIndicator_Register() at line 55
	// inlined CHudCoopPingIndicator::CHudCoopPingIndicator_RegisterMap::CHudCoopPingIndicator_RegisterMap() at line 55
}

// game/client/portal2/hud_coop_ping_indicator.cpp:33 @0x60c020 _ZN21CHudCoopPingIndicatorC1EPKc
CHudCoopPingIndicator::CHudCoopPingIndicator( const char *pElementName )
{
}

// game/client/portal2/hud_coop_ping_indicator.cpp:34 @0x60be20 _ZN21CHudCoopPingIndicator4InitEv
void CHudCoopPingIndicator::Init()
{
	// inlined vgui::surface() at line 72
	// inlined vgui::surface() at line 73
}

// game/client/portal2/hud_coop_ping_indicator.cpp:35 @0x60b2a0 _ZN21CHudCoopPingIndicator9LevelInitEv
void CHudCoopPingIndicator::LevelInit()
{
}

// game/client/portal2/hud_coop_ping_indicator.cpp:36 @0x60bcf0 _ZN21CHudCoopPingIndicator19ApplySchemeSettingsEPN4vgui7ISchemeE
void CHudCoopPingIndicator::ApplySchemeSettings( vgui::IScheme *scheme )
{
}

// game/client/portal2/hud_coop_ping_indicator.cpp:37 @0x60bd80 _ZN21CHudCoopPingIndicator10ShouldDrawEv
bool CHudCoopPingIndicator::ShouldDraw()
{
	C_BasePlayer *player;  // line 90
	// inlined GameRules() at line 94
}

// game/client/portal2/hud_coop_ping_indicator.cpp:38 @0x60bcd0 _ZN21CHudCoopPingIndicator5PaintEv
void CHudCoopPingIndicator::Paint()
{
}

// game/client/portal2/hud_coop_ping_indicator.cpp:40 @0x60b2d0 _ZN21CHudCoopPingIndicator24MsgFunc_HudPingIndicatorER7bf_read
void CHudCoopPingIndicator::MsgFunc_HudPingIndicator( bf_read &msg )
{
	C_Portal_Player *pPlayer;  // line 183
	// inlined CBitRead::ReadFloat() at line 190
	// inlined CBitRead::ReadFloat() at line 189
	// inlined CBitRead::ReadFloat() at line 188
}

// game/client/portal2/hud_coop_ping_indicator.cpp:43 @0x60b760 _ZN21CHudCoopPingIndicator17DrawIndicatorHintEv
void CHudCoopPingIndicator::DrawIndicatorHint()
{
	Vector vecScreen;  // line 111
	float xCenter;  // line 114
	float yCenter;  // line 115
	float x;  // line 117
	float y;  // line 118
	float flDist;  // line 125
	const float flInnerCircle;  // line 128
	float flAng;  // line 129
	float sa;  // line 131
	float ca;  // line 131
	const float flScale;  // line 138
	Vertex_t verts[4];  // line 140
	Vector2D center;  // line 143
	Vector2D axis[2];  // line 146
	Color color;  // line 166
	// inlined vgui::surface() at line 171
	// inlined vgui::surface() at line 170
	// inlined vgui::surface() at line 169
	// inlined RemapValClamped() at line 167
	// inlined Vector2DMA() at line 163
	// inlined Vector2D::Init() at line 162
	// inlined Vector2DMA() at line 160
	// inlined Vector2D::Init() at line 159
	// inlined Vector2DMA() at line 157
	// inlined Vector2D::Init() at line 156
	// inlined Vector2DMA() at line 154
	// inlined Vector2DMA() at line 153
	// inlined Vector2D::Init() at line 152
	// inlined ConVar::GetFloat() at line 138
	// inlined SinCos() at line 132
	// inlined Vector::operator==() at line 108
}

// game/client/portal2/hud_coop_ping_indicator.cpp:51 @0x60c030 _ZL28Create_CHudCoopPingIndicatorv
CHudElement *Create_CHudCoopPingIndicator()
{
	// inlined CHudElement::operator new() at line 51
	// inlined CHudCoopPingIndicator::CHudCoopPingIndicator() at line 51
	// inlined CHudElement::operator delete() at line 51
}

// game/client/portal2/hud_coop_ping_indicator.cpp:51
static CHudElementHelper g_CHudCoopPingIndicator_Helper;

// game/client/portal2/hud_coop_ping_indicator.cpp:52 @0x60bd30 _Z48__MsgFunc_CHudCoopPingIndicator_HudPingIndicatorR7bf_read
__MsgFunc_CHudCoopPingIndicator_HudPingIndicator( bf_read &msg )
{
	CHudElement *pElement;  // line 52
}
