// DWARF declaration skeleton for game/client/portal2/hud_indicator.cpp
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x81f90 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
}

// game/client/portal2/hud_indicator.cpp:31 sizeof=0x1d0 (i386)
struct CHudIndicator : public vgui::EditablePanel, public CHudElement
{
public:
	// game/client/portal2/hud_indicator.cpp:32 sizeof=0x1 (i386)
	struct CHudIndicator_RegisterMap
	{
	public:
		CHudIndicator_RegisterMap();  // line 32
	};  // line 32
	// game/client/portal2/hud_indicator.cpp:32 sizeof=0x1 (i386)
	struct CHudIndicator_Register
	{
	public:
		CHudIndicator_Register();  // line 32
	};  // line 32
	// game/client/portal2/hud_indicator.cpp:32 sizeof=0x1 (i386)
	struct CHudIndicator_RegisterKBMap
	{
	public:
		CHudIndicator_RegisterKBMap();  // line 32
	};  // line 32
	CHudIndicator::CHudIndicator_RegisterMap m_RegisterClass; // +0x1b0  // line 32
	CHudIndicator::CHudIndicator_Register m_RegisterAnimationClass; // +0x1b1  // line 32
	CHudIndicator::CHudIndicator_RegisterKBMap m_RegisterClassKB; // +0x1b2  // line 32
	static void AddToMap( const char *, MessageMapItem_t::MessageFunc_t, int, int, const char *, int, const char * );  // line 32
	static void ChainToMap();  // line 32
	virtual vgui::PanelMessageMap *GetMessageMap();  // line 32
	static void AddToAnimationMap( const char *, const char *, const char *, const char *, bool, PANELLOOKUPFUNC );  // line 32
	static void ChainToAnimationMap();  // line 32
	virtual PanelAnimationMap *GetAnimMap();  // line 32
	static void KB_AddToMap( const char *, KeyCode, int, MessageMapItem_t::MessageFunc_t, const char *, const char *, bool );  // line 32
	static void KB_ChainToMap();  // line 32
	static void KB_AddBoundKey( const char *, int, int );  // line 32
	virtual vgui::PanelKeyBindingMap *GetKBMap();  // line 32
	static const char *GetPanelClassName();  // line 32
	static const char *GetPanelBaseClassName();  // line 32
	CHudIndicator( const char * );  // line 35
	void MsgFunc_IndicatorFlash( bf_read & );  // line 37
protected:
	virtual void ApplySchemeSettings( vgui::IScheme * );  // line 40
	virtual void Paint();  // line 41
	virtual void Init();  // line 42
	virtual bool ShouldDraw();  // line 43
	virtual void Reset();  // line 44
private:
	int m_nTexture[2]; // +0x1b4  // line 47
	int m_nIndicatorType; // +0x1bc  // line 48
	float m_flDisplayTime; // +0x1c0  // line 50
	float m_flStartTime; // +0x1c4  // line 51
	float m_flFadeInTime; // +0x1c8  // line 52
	float m_flFadeOutTime; // +0x1cc  // line 53
};

// game/client/portal2/hud_indicator.cpp:31 (declaration)
~CHudIndicator();

// game/client/portal2/hud_indicator.cpp:31 @0x6042b0 _ZN13CHudIndicatorD0Ev
CHudIndicator::~CHudIndicator()
{
	// inlined CHudElement::operator delete() at line 31
}

// game/client/portal2/hud_indicator.cpp:31 @0x604340 _ZN13CHudIndicatorD1Ev
CHudIndicator::~CHudIndicator()
{
}

// game/client/portal2/hud_indicator.cpp:32 @0x603d80 _ZN13CHudIndicator17GetPanelClassNameEv
const char *CHudIndicator::GetPanelClassName()
{
}

// game/client/portal2/hud_indicator.cpp:32 @0x603e80 _ZN13CHudIndicator13KB_ChainToMapEv
void CHudIndicator::KB_ChainToMap()
{
	vgui::PanelKeyBindingMap *map;  // line 32
}

// game/client/portal2/hud_indicator.cpp:32 @0x603f40 _ZN13CHudIndicator19ChainToAnimationMapEv
void CHudIndicator::ChainToAnimationMap()
{
	PanelAnimationMap *map;  // line 32
}

// game/client/portal2/hud_indicator.cpp:32 @0x604000 _ZN13CHudIndicator10ChainToMapEv
void CHudIndicator::ChainToMap()
{
	vgui::PanelMessageMap *map;  // line 32
}

// game/client/portal2/hud_indicator.cpp:32 @0x604180 _ZN13CHudIndicator8GetKBMapEv
vgui::PanelKeyBindingMap *CHudIndicator::GetKBMap()
{
	vgui::PanelKeyBindingMap *s_pMap;  // line 32
}

// game/client/portal2/hud_indicator.cpp:32 @0x604200 _ZN13CHudIndicator10GetAnimMapEv
PanelAnimationMap *CHudIndicator::GetAnimMap()
{
}

// game/client/portal2/hud_indicator.cpp:32 @0x604220 _ZN13CHudIndicator13GetMessageMapEv
vgui::PanelMessageMap *CHudIndicator::GetMessageMap()
{
	vgui::PanelMessageMap *s_pMap;  // line 32
}

// game/client/portal2/hud_indicator.cpp:35 @0x6039b0 _ZN13CHudIndicatorC2EPKc
CHudIndicator::CHudIndicator( const char *pElementName )
{
	{
		vgui::Panel *pParent;  // line 64
	}
	// inlined CHudIndicator::CHudIndicator_RegisterKBMap::CHudIndicator_RegisterKBMap() at line 62
	// inlined CHudIndicator::CHudIndicator_Register::CHudIndicator_Register() at line 62
	// inlined CHudIndicator::CHudIndicator_RegisterMap::CHudIndicator_RegisterMap() at line 62
}

// game/client/portal2/hud_indicator.cpp:35 @0x603b80 _ZN13CHudIndicatorC1EPKc
CHudIndicator::CHudIndicator( const char *pElementName )
{
}

// game/client/portal2/hud_indicator.cpp:37 @0x602130 _ZN13CHudIndicator22MsgFunc_IndicatorFlashER7bf_read
void CHudIndicator::MsgFunc_IndicatorFlash( bf_read &msg )
{
	// inlined CBitRead::ReadByte() at line 196
	// inlined CBitRead::ReadFloat() at line 200
}

// game/client/portal2/hud_indicator.cpp:40 @0x602c00 _ZN13CHudIndicator19ApplySchemeSettingsEPN4vgui7ISchemeE
void CHudIndicator::ApplySchemeSettings( vgui::IScheme *scheme )
{
	int screenWide;  // line 113
	int screenTall;  // line 113
	int nCenterX;  // line 116
	int nCenterY;  // line 117
}

// game/client/portal2/hud_indicator.cpp:41 @0x602f20 _ZN13CHudIndicator5PaintEv
void CHudIndicator::Paint()
{
	float flGlobalScale;  // line 143
	float flGlobalAlpha;  // line 144
	float flBump;  // line 155
	float flScale;  // line 156
	const float flOffset;  // line 157
	float xLeft;  // line 164
	float xRight;  // line 165
	float yTop;  // line 166
	float yBottom;  // line 167
	const int nShadowDepth;  // line 171
	Vertex_t vert[4];  // line 172
	// inlined vgui::surface() at line 185
	// inlined FontVertex_t::Init() at line 184
	// inlined FontVertex_t::Init() at line 183
	// inlined FontVertex_t::Init() at line 182
	// inlined FontVertex_t::Init() at line 181
	// inlined vgui::surface() at line 180
	// inlined vgui::surface() at line 177
	// inlined FontVertex_t::Init() at line 176
	// inlined FontVertex_t::Init() at line 175
	// inlined FontVertex_t::Init() at line 174
	// inlined FontVertex_t::Init() at line 173
	// inlined vgui::surface() at line 170
	// inlined vgui::surface() at line 161
	// inlined SimpleSplineRemapValClamped() at line 148
	// inlined SimpleSplineRemapValClamped() at line 152
}

// game/client/portal2/hud_indicator.cpp:42 @0x602e30 _ZN13CHudIndicator4InitEv
void CHudIndicator::Init()
{
	// inlined vgui::surface() at line 82
	// inlined vgui::surface() at line 83
	// inlined vgui::surface() at line 85
	// inlined vgui::surface() at line 86
}

// game/client/portal2/hud_indicator.cpp:43 @0x602100 _ZN13CHudIndicator10ShouldDrawEv
bool CHudIndicator::ShouldDraw()
{
}

// game/client/portal2/hud_indicator.cpp:44 @0x6020d0 _ZN13CHudIndicator5ResetEv
void CHudIndicator::Reset()
{
}

// game/client/portal2/hud_indicator.cpp:57 @0x602bb0 _Z38__MsgFunc_CHudIndicator_IndicatorFlashR7bf_read
__MsgFunc_CHudIndicator_IndicatorFlash( bf_read &msg )
{
	CHudElement *pElement;  // line 57
}

// game/client/portal2/hud_indicator.cpp:206
ConVar hud_show_control_helper;

// game/client/portal2/hud_indicator.cpp:212 sizeof=0x1d8 (i386)
struct CHudControlHelper : public vgui::EditablePanel, public CHudElement
{
public:
	// game/client/portal2/hud_indicator.cpp:213 sizeof=0x1 (i386)
	struct CHudControlHelper_RegisterMap
	{
	public:
		CHudControlHelper_RegisterMap();  // line 213
	};  // line 213
	// game/client/portal2/hud_indicator.cpp:213 sizeof=0x1 (i386)
	struct CHudControlHelper_Register
	{
	public:
		CHudControlHelper_Register();  // line 213
	};  // line 213
	// game/client/portal2/hud_indicator.cpp:213 sizeof=0x1 (i386)
	struct CHudControlHelper_RegisterKBMap
	{
	public:
		CHudControlHelper_RegisterKBMap();  // line 213
	};  // line 213
	CHudControlHelper::CHudControlHelper_RegisterMap m_RegisterClass; // +0x1b0  // line 213
	CHudControlHelper::CHudControlHelper_Register m_RegisterAnimationClass; // +0x1b1  // line 213
	CHudControlHelper::CHudControlHelper_RegisterKBMap m_RegisterClassKB; // +0x1b2  // line 213
	static void AddToMap( const char *, MessageMapItem_t::MessageFunc_t, int, int, const char *, int, const char * );  // line 213
	static void ChainToMap();  // line 213
	virtual vgui::PanelMessageMap *GetMessageMap();  // line 213
	static void AddToAnimationMap( const char *, const char *, const char *, const char *, bool, PANELLOOKUPFUNC );  // line 213
	static void ChainToAnimationMap();  // line 213
	virtual PanelAnimationMap *GetAnimMap();  // line 213
	static void KB_AddToMap( const char *, KeyCode, int, MessageMapItem_t::MessageFunc_t, const char *, const char *, bool );  // line 213
	static void KB_ChainToMap();  // line 213
	static void KB_AddBoundKey( const char *, int, int );  // line 213
	virtual vgui::PanelKeyBindingMap *GetKBMap();  // line 213
	static const char *GetPanelClassName();  // line 213
	static const char *GetPanelBaseClassName();  // line 213
	CHudControlHelper( const char * );  // line 216
	void MsgFunc_ControlHelperAnimate( bf_read & );  // line 218
protected:
	virtual void ApplySchemeSettings( vgui::IScheme * );  // line 221
	virtual void Paint();  // line 222
	virtual void Init();  // line 223
	virtual bool ShouldDraw();  // line 224
	virtual void Reset();  // line 225
private:
	void DrawIcon( int, int, float, float, int, int );  // line 228
	int m_nTexture[3]; // +0x1b4  // line 230
	int m_nFrontIcon; // +0x1c0  // line 231
	float m_flDisplayTime; // +0x1c4  // line 233
	float m_flStartTime; // +0x1c8  // line 234
	float m_flFadeInTime; // +0x1cc  // line 235
	float m_flFadeOutTime; // +0x1d0  // line 236
	bool m_bHoldIndefinitely; // +0x1d4  // line 237
};

// game/client/portal2/hud_indicator.cpp:212 (declaration)
~CHudControlHelper();

// game/client/portal2/hud_indicator.cpp:212 @0x603db0 _ZN17CHudControlHelperD1Ev
CHudControlHelper::~CHudControlHelper()
{
}

// game/client/portal2/hud_indicator.cpp:212 @0x6043c0 _ZN17CHudControlHelperD0Ev
CHudControlHelper::~CHudControlHelper()
{
	// inlined CHudElement::operator delete() at line 212
}

// game/client/portal2/hud_indicator.cpp:213 @0x603d90 _ZN17CHudControlHelper17GetPanelClassNameEv
const char *CHudControlHelper::GetPanelClassName()
{
}

// game/client/portal2/hud_indicator.cpp:213 @0x603e20 _ZN17CHudControlHelper13KB_ChainToMapEv
void CHudControlHelper::KB_ChainToMap()
{
	vgui::PanelKeyBindingMap *map;  // line 213
}

// game/client/portal2/hud_indicator.cpp:213 @0x603ee0 _ZN17CHudControlHelper19ChainToAnimationMapEv
void CHudControlHelper::ChainToAnimationMap()
{
	PanelAnimationMap *map;  // line 213
}

// game/client/portal2/hud_indicator.cpp:213 @0x603fa0 _ZN17CHudControlHelper10ChainToMapEv
void CHudControlHelper::ChainToMap()
{
	vgui::PanelMessageMap *map;  // line 213
}

// game/client/portal2/hud_indicator.cpp:213 @0x604060 _ZN17CHudControlHelper8GetKBMapEv
vgui::PanelKeyBindingMap *CHudControlHelper::GetKBMap()
{
	vgui::PanelKeyBindingMap *s_pMap;  // line 213
}

// game/client/portal2/hud_indicator.cpp:213 @0x6040e0 _ZN17CHudControlHelper10GetAnimMapEv
PanelAnimationMap *CHudControlHelper::GetAnimMap()
{
}

// game/client/portal2/hud_indicator.cpp:213 @0x604100 _ZN17CHudControlHelper13GetMessageMapEv
vgui::PanelMessageMap *CHudControlHelper::GetMessageMap()
{
	vgui::PanelMessageMap *s_pMap;  // line 213
}

// game/client/portal2/hud_indicator.cpp:216 @0x603b90 _ZN17CHudControlHelperC2EPKc
CHudControlHelper::CHudControlHelper( const char *pElementName )
{
	{
		vgui::Panel *pParent;  // line 252
	}
	// inlined CHudControlHelper::CHudControlHelper_RegisterKBMap::CHudControlHelper_RegisterKBMap() at line 250
	// inlined CHudControlHelper::CHudControlHelper_Register::CHudControlHelper_Register() at line 250
	// inlined CHudControlHelper::CHudControlHelper_RegisterMap::CHudControlHelper_RegisterMap() at line 250
}

// game/client/portal2/hud_indicator.cpp:216 @0x603d70 _ZN17CHudControlHelperC1EPKc
CHudControlHelper::CHudControlHelper( const char *pElementName )
{
}

// game/client/portal2/hud_indicator.cpp:218 @0x602730 _ZN17CHudControlHelper28MsgFunc_ControlHelperAnimateER7bf_read
void CHudControlHelper::MsgFunc_ControlHelperAnimate( bf_read &msg )
{
	bool bClear;  // line 452
	// inlined CBitRead::ReadByte() at line 452
	// inlined CBitRead::ReadByte() at line 462
}

// game/client/portal2/hud_indicator.cpp:221 @0x602aa0 _ZN17CHudControlHelper19ApplySchemeSettingsEPN4vgui7ISchemeE
void CHudControlHelper::ApplySchemeSettings( vgui::IScheme *scheme )
{
	int nCenterY;  // line 308
}

// game/client/portal2/hud_indicator.cpp:222 @0x6033e0 _ZN17CHudControlHelper5PaintEv
void CHudControlHelper::Paint()
{
	float flGlobalScale;  // line 371
	float flGlobalAlpha;  // line 372
	bool bScalingUp;  // line 374
	bool bScalingDown;  // line 375
	int iconCenterX;  // line 388
	int iconY;  // line 389
	float flMoveScale;  // line 392
	int nBaseOffset;  // line 394
	int xOffset[3];  // line 395
	int nCurIcon;  // line 402
	{
		int i;  // line 403
		{
			float flScaleFactor;  // line 405
			int nDepth;  // line 407
			int xPos;  // line 409
			float flScale;  // line 424
		}
	}
	// inlined SimpleSplineRemapValClamped() at line 379
	// inlined SimpleSplineRemapValClamped() at line 384
}

// game/client/portal2/hud_indicator.cpp:223 @0x602ce0 _ZN17CHudControlHelper4InitEv
void CHudControlHelper::Init()
{
	// inlined vgui::surface() at line 274
	// inlined vgui::surface() at line 275
	// inlined vgui::surface() at line 277
	// inlined vgui::surface() at line 278
	// inlined vgui::surface() at line 280
	// inlined vgui::surface() at line 281
}

// game/client/portal2/hud_indicator.cpp:224 @0x6024a0 _ZN17CHudControlHelper10ShouldDrawEv
bool CHudControlHelper::ShouldDraw()
{
}

// game/client/portal2/hud_indicator.cpp:225 @0x602470 _ZN17CHudControlHelper5ResetEv
void CHudControlHelper::Reset()
{
}

// game/client/portal2/hud_indicator.cpp:228 @0x6024f0 _ZN17CHudControlHelper8DrawIconEiiffii
void CHudControlHelper::DrawIcon( int x, int y, float flScale, float flAlpha, int nIconID, int nDepth )
{
	const float flOffset;  // line 338
	float xLeft;  // line 341
	float xRight;  // line 342
	float yTop;  // line 343
	float yBottom;  // line 344
	const int nShadowDepth;  // line 348
	Vertex_t vert[4];  // line 349
	float flColor;  // line 357
	// inlined vgui::surface() at line 336
	// inlined vgui::surface() at line 347
	// inlined FontVertex_t::Init() at line 350
	// inlined FontVertex_t::Init() at line 351
	// inlined FontVertex_t::Init() at line 352
	// inlined FontVertex_t::Init() at line 353
	// inlined vgui::surface() at line 354
	// inlined vgui::surface() at line 358
	// inlined FontVertex_t::Init() at line 359
	// inlined FontVertex_t::Init() at line 360
	// inlined FontVertex_t::Init() at line 361
	// inlined FontVertex_t::Init() at line 362
	// inlined vgui::surface() at line 363
}

// game/client/portal2/hud_indicator.cpp:241 @0x602b60 _Z48__MsgFunc_CHudControlHelper_ControlHelperAnimateR7bf_read
__MsgFunc_CHudControlHelper_ControlHelperAnimate( bf_read &msg )
{
	CHudElement *pElement;  // line 241
}
