// DWARF declaration skeleton for game/client/portal2/hud_viewfinder.cpp
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x82190 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
}

// game/client/portal2/hud_viewfinder.cpp:40 sizeof=0x19c (i386)
struct CHudViewfinder : public vgui::Panel, public CHudElement
{
public:
	// game/client/portal2/hud_viewfinder.cpp:41 sizeof=0x1 (i386)
	struct CHudViewfinder_RegisterMap
	{
	public:
		CHudViewfinder_RegisterMap();  // line 41
	};  // line 41
	// game/client/portal2/hud_viewfinder.cpp:41 sizeof=0x1 (i386)
	struct CHudViewfinder_Register
	{
	public:
		CHudViewfinder_Register();  // line 41
	};  // line 41
	// game/client/portal2/hud_viewfinder.cpp:41 sizeof=0x1 (i386)
	struct CHudViewfinder_RegisterKBMap
	{
	public:
		CHudViewfinder_RegisterKBMap();  // line 41
	};  // line 41
	CHudViewfinder::CHudViewfinder_RegisterMap m_RegisterClass; // +0x180  // line 41
	CHudViewfinder::CHudViewfinder_Register m_RegisterAnimationClass; // +0x181  // line 41
	CHudViewfinder::CHudViewfinder_RegisterKBMap m_RegisterClassKB; // +0x182  // line 41
	static void AddToMap( const char *, MessageMapItem_t::MessageFunc_t, int, int, const char *, int, const char * );  // line 41
	static void ChainToMap();  // line 41
	virtual vgui::PanelMessageMap *GetMessageMap();  // line 41
	static void AddToAnimationMap( const char *, const char *, const char *, const char *, bool, PANELLOOKUPFUNC );  // line 41
	static void ChainToAnimationMap();  // line 41
	virtual PanelAnimationMap *GetAnimMap();  // line 41
	static void KB_AddToMap( const char *, KeyCode, int, MessageMapItem_t::MessageFunc_t, const char *, const char *, bool );  // line 41
	static void KB_ChainToMap();  // line 41
	static void KB_AddBoundKey( const char *, int, int );  // line 41
	virtual vgui::PanelKeyBindingMap *GetKBMap();  // line 41
	static const char *GetPanelClassName();  // line 41
	static const char *GetPanelBaseClassName();  // line 41
	CHudViewfinder( const char * );  // line 44
	virtual void Init();  // line 46
protected:
	virtual void ApplySchemeSettings( vgui::IScheme * );  // line 49
	virtual void Paint();  // line 50
	virtual bool ShouldDraw();  // line 51
private:
	void PaintLocator( C_BaseEntity * );  // line 54
	void PaintLocators();  // line 55
	int m_iScopeTexture[4]; // +0x184  // line 57
	int m_nChickenIcon; // +0x194  // line 58
	int m_nArrowIcon; // +0x198  // line 59
};

// game/client/portal2/hud_viewfinder.cpp:40 (declaration)
~CHudViewfinder();

// game/client/portal2/hud_viewfinder.cpp:40 @0x6071a0 _ZN14CHudViewfinderD1Ev
CHudViewfinder::~CHudViewfinder()
{
}

// game/client/portal2/hud_viewfinder.cpp:40 @0x607460 _ZN14CHudViewfinderD0Ev
CHudViewfinder::~CHudViewfinder()
{
	// inlined CHudElement::operator delete() at line 40
}

// game/client/portal2/hud_viewfinder.cpp:41 @0x607180 _ZN14CHudViewfinder17GetPanelClassNameEv
const char *CHudViewfinder::GetPanelClassName()
{
}

// game/client/portal2/hud_viewfinder.cpp:41 @0x607210 _ZN14CHudViewfinder13KB_ChainToMapEv
void CHudViewfinder::KB_ChainToMap()
{
	vgui::PanelKeyBindingMap *map;  // line 41
}

// game/client/portal2/hud_viewfinder.cpp:41 @0x607270 _ZN14CHudViewfinder19ChainToAnimationMapEv
void CHudViewfinder::ChainToAnimationMap()
{
	PanelAnimationMap *map;  // line 41
}

// game/client/portal2/hud_viewfinder.cpp:41 @0x6072d0 _ZN14CHudViewfinder10ChainToMapEv
void CHudViewfinder::ChainToMap()
{
	vgui::PanelMessageMap *map;  // line 41
}

// game/client/portal2/hud_viewfinder.cpp:41 @0x607330 _ZN14CHudViewfinder8GetKBMapEv
vgui::PanelKeyBindingMap *CHudViewfinder::GetKBMap()
{
	vgui::PanelKeyBindingMap *s_pMap;  // line 41
}

// game/client/portal2/hud_viewfinder.cpp:41 @0x6073b0 _ZN14CHudViewfinder10GetAnimMapEv
PanelAnimationMap *CHudViewfinder::GetAnimMap()
{
}

// game/client/portal2/hud_viewfinder.cpp:41 @0x6073d0 _ZN14CHudViewfinder13GetMessageMapEv
vgui::PanelMessageMap *CHudViewfinder::GetMessageMap()
{
	vgui::PanelMessageMap *s_pMap;  // line 41
}

// game/client/portal2/hud_viewfinder.cpp:44 @0x605da0 _ZN14CHudViewfinderC2EPKc
CHudViewfinder::CHudViewfinder( const char *pElementName )
{
	{
		vgui::Panel *pParent;  // line 71
	}
	// inlined CHudViewfinder::CHudViewfinder_RegisterKBMap::CHudViewfinder_RegisterKBMap() at line 69
	// inlined CHudViewfinder::CHudViewfinder_Register::CHudViewfinder_Register() at line 69
	// inlined CHudViewfinder::CHudViewfinder_RegisterMap::CHudViewfinder_RegisterMap() at line 69
}

// game/client/portal2/hud_viewfinder.cpp:44 @0x605f30 _ZN14CHudViewfinderC1EPKc
CHudViewfinder::CHudViewfinder( const char *pElementName )
{
}

// game/client/portal2/hud_viewfinder.cpp:46 @0x605c10 _ZN14CHudViewfinder4InitEv
void CHudViewfinder::Init()
{
	int i;  // line 82
	// inlined vgui::surface() at line 85
	// inlined vgui::surface() at line 88
	// inlined vgui::surface() at line 89
	// inlined vgui::surface() at line 91
	// inlined vgui::surface() at line 92
	// inlined vgui::surface() at line 94
	// inlined vgui::surface() at line 95
}

// game/client/portal2/hud_viewfinder.cpp:49 @0x605b80 _ZN14CHudViewfinder19ApplySchemeSettingsEPN4vgui7ISchemeE
void CHudViewfinder::ApplySchemeSettings( vgui::IScheme *scheme )
{
	int screenWide;  // line 111
	int screenTall;  // line 111
}

// game/client/portal2/hud_viewfinder.cpp:50 @0x606490 _ZN14CHudViewfinder5PaintEv
void CHudViewfinder::Paint()
{
	int screenWide;  // line 336
	int screenTall;  // line 336
	int xMid;  // line 340
	int yMid;  // line 341
	int wide;  // line 344
	int xLeft;  // line 346
	int xRight;  // line 347
	int yTop;  // line 348
	int yBottom;  // line 349
	float uv1;  // line 351
	float uv2;  // line 351
	Vertex_t vert[4];  // line 353
	Vector2D uv11;  // line 355
	Vector2D uv12;  // line 356
	Vector2D uv21;  // line 357
	Vector2D uv22;  // line 358
	int tall;  // line 403
	int nInnerCircle;  // line 406
	int nOuterCircle;  // line 407
	const float circleScale;  // line 414
	const float flRegisterWeight;  // line 430
	const float flRegisterOffset;  // line 431
	const float flRegisterLength;  // line 432
	IClientPanel::HScheme scheme;  // line 452
	FontHandle_t hTFont;  // line 453
	int nAlpha;  // line 455
	float flAlpha;  // line 456
	wchar_t unicode[256];  // line 465
	int nWide;  // line 468
	int nTall;  // line 468
	// inlined vgui::surface() at line 471
	// inlined vgui::surface() at line 470
	// inlined vgui::surface() at line 469
	// inlined Color::Color() at line 463
	// inlined vgui::surface() at line 463
	// inlined vgui::surface() at line 462
	// inlined vgui::scheme() at line 453
	// inlined vgui::scheme() at line 452
	// inlined vgui::surface() at line 450
	// inlined vgui::surface() at line 449
	// inlined vgui::surface() at line 446
	// inlined vgui::surface() at line 445
	// inlined vgui::surface() at line 442
	// inlined vgui::surface() at line 441
	// inlined vgui::surface() at line 438
	// inlined vgui::surface() at line 437
	// inlined vgui::surface() at line 434
	// inlined vgui::surface() at line 427
	// inlined FontVertex_t::Init() at line 426
	// inlined FontVertex_t::Init() at line 425
	// inlined FontVertex_t::Init() at line 424
	// inlined FontVertex_t::Init() at line 423
	// inlined vgui::surface() at line 421
	// inlined vgui::surface() at line 419
	// inlined FontVertex_t::Init() at line 418
	// inlined FontVertex_t::Init() at line 417
	// inlined FontVertex_t::Init() at line 416
	// inlined FontVertex_t::Init() at line 415
	// inlined vgui::surface() at line 413
	// inlined vgui::surface() at line 412
	// inlined vgui::surface() at line 409
	// inlined vgui::surface() at line 408
	// inlined vgui::surface() at line 389
	// inlined FontVertex_t::Init() at line 388
	// inlined FontVertex_t::Init() at line 387
	// inlined FontVertex_t::Init() at line 386
	// inlined FontVertex_t::Init() at line 385
	// inlined vgui::surface() at line 382
	// inlined FontVertex_t::Init() at line 381
	// inlined FontVertex_t::Init() at line 380
	// inlined FontVertex_t::Init() at line 379
	// inlined FontVertex_t::Init() at line 378
	// inlined vgui::surface() at line 375
	// inlined FontVertex_t::Init() at line 374
	// inlined FontVertex_t::Init() at line 373
	// inlined FontVertex_t::Init() at line 372
	// inlined FontVertex_t::Init() at line 371
	// inlined vgui::surface() at line 368
	// inlined FontVertex_t::Init() at line 367
	// inlined FontVertex_t::Init() at line 366
	// inlined FontVertex_t::Init() at line 365
	// inlined FontVertex_t::Init() at line 364
	// inlined vgui::surface() at line 363
	// inlined vgui::surface() at line 360
	// inlined UpdateRefractTexture() at line 334
	// inlined CHudViewfinder::PaintLocators() at line 331
}

// game/client/portal2/hud_viewfinder.cpp:51 @0x605b30 _ZN14CHudViewfinder10ShouldDrawEv
bool CHudViewfinder::ShouldDraw()
{
	C_Portal_Player *pPlayer;  // line 122
	// inlined C_Portal_Player::GetLocalPlayer() at line 122
}

// game/client/portal2/hud_viewfinder.cpp:54 @0x605430 _ZN14CHudViewfinder12PaintLocatorEP12C_BaseEntity
void CHudViewfinder::PaintLocator( C_BaseEntity *pTarget )
{
	Vector vecScreen;  // line 245
	float centerX;  // line 249
	float centerY;  // line 250
	C_BasePlayer *pLocalPlayer;  // line 255
	float flZDiff;  // line 256
	float flDistSqr;  // line 257
	float flDistScale;  // line 258
	float screenMax;  // line 260
	const int nDropShadowDepth;  // line 264
	{
		int nWidth;  // line 280
		int nHeight;  // line 281
		// inlined vgui::surface() at line 286
		// inlined vgui::surface() at line 278
		{
			Vertex_t vert[4];  // line 290
			// inlined FontVertex_t::Init() at line 293
			// inlined FontVertex_t::Init() at line 294
			// inlined FontVertex_t::Init() at line 295
			// inlined FontVertex_t::Init() at line 296
			// inlined vgui::surface() at line 298
		}
	}
	// inlined vgui::surface() at line 273
	// inlined vgui::surface() at line 272
	// inlined vgui::surface() at line 269
	// inlined vgui::surface() at line 267
	// inlined vgui::surface() at line 262
	// inlined RemapValClamped() at line 258
	// inlined Vector::operator-() at line 257
}

// game/client/portal2/hud_viewfinder.cpp:55 @0x6059d0 _ZN14CHudViewfinder13PaintLocatorsEv
void CHudViewfinder::PaintLocators()
{
	C_Portal_Player *pPlayer;  // line 308
	{
		int i;  // line 311
		{
			int nEntityIndex;  // line 313
			C_BaseEntity *pEntity;  // line 317
		}
	}
}

// game/client/portal2/hud_viewfinder.cpp:127 @0x605a30 _Z24UTIL_WorldToScreenCoordsRK6VectorPiS2_
UTIL_WorldToScreenCoords( const Vector &vecWorld, int *pScreenX, int *pScreenY )
{
	Vector vecTransform;  // line 132
}

// game/client/portal2/hud_viewfinder.cpp:140 (declaration)
void UTIL_GenerateBoxVertices( const Vector &vOrigin, const Vector &vMins, const Vector &vMaxs, Vector *pVerts );

// game/client/portal2/hud_viewfinder.cpp:140 @0x6052a0 _Z24UTIL_GenerateBoxVerticesRK6VectorS1_S1_PS_
UTIL_GenerateBoxVertices( const Vector &vOrigin, const Vector &vMins, const Vector &vMaxs, Vector *pVerts )
{
	Vector vecPos;  // line 142
	{
		int i;  // line 143
		// inlined Vector::operator+() at line 148
		// inlined Vector::operator=() at line 148
	}
}

// game/client/portal2/hud_viewfinder.cpp:152 @0x605f40 _Z35UTIL_WorldSpaceToScreensSpaceBoundsRK6VectorS1_S1_P8Vector2DS3_
bool UTIL_WorldSpaceToScreensSpaceBounds( const Vector &vecCenter, const Vector &mins, const Vector &maxs, Vector2D *pMins, Vector2D *pMaxs )
{
	Vector vecBoxVerts[8];  // line 154
	int nMaxX;  // line 157
	int nMinX;  // line 158
	int nMaxY;  // line 159
	int nMinY;  // line 160
	int nX;  // line 162
	int nY;  // line 162
	{
		int i;  // line 163
	}
	// inlined UTIL_GenerateBoxVertices() at line 155
}

// game/client/portal2/hud_viewfinder.cpp:214 @0x606380 _Z24UTIL_EntityBoundsToSizesP12C_BaseEntityPiS1_S1_S1_
bool UTIL_EntityBoundsToSizes( C_BaseEntity *pTarget, int *pMinX, int *pMinY, int *pMaxX, int *pMaxY )
{
	Vector vOBBMins;  // line 216
	Vector vOBBMaxs;  // line 216
	Vector2D mins;  // line 221
	Vector2D maxs;  // line 221
	// inlined Vector::operator-=() at line 218
	// inlined Vector::operator-=() at line 219
}
