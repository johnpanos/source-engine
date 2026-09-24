// DWARF declaration skeleton for game/client/portal2/polygonbutton.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal2/polygonbutton.h:19 sizeof=0x240 (i386)
struct CPolygonButton : public vgui::Button
{
public:
	// game/client/portal2/polygonbutton.h:20 sizeof=0x1 (i386)
	struct CPolygonButton_RegisterMap
	{
	public:
		CPolygonButton_RegisterMap();  // line 20
	};  // line 20
	// game/client/portal2/polygonbutton.h:20 sizeof=0x1 (i386)
	struct CPolygonButton_Register
	{
	public:
		CPolygonButton_Register();  // line 20
	};  // line 20
	// game/client/portal2/polygonbutton.h:20 sizeof=0x1 (i386)
	struct CPolygonButton_RegisterKBMap
	{
	public:
		CPolygonButton_RegisterKBMap();  // line 20
	};  // line 20
	CPolygonButton::CPolygonButton_RegisterMap m_RegisterClass; // +0x1f1  // line 20
	CPolygonButton::CPolygonButton_Register m_RegisterAnimationClass; // +0x1f2  // line 20
	CPolygonButton::CPolygonButton_RegisterKBMap m_RegisterClassKB; // +0x1f3  // line 20
	static void AddToMap( const char *, MessageMapItem_t::MessageFunc_t, int, int, const char *, int, const char * );  // line 20
	static void ChainToMap();  // line 20
	virtual vgui::PanelMessageMap *GetMessageMap();  // line 20
	static void AddToAnimationMap( const char *, const char *, const char *, const char *, bool, PANELLOOKUPFUNC );  // line 20
	static void ChainToAnimationMap();  // line 20
	virtual PanelAnimationMap *GetAnimMap();  // line 20
	static void KB_AddToMap( const char *, KeyCode, int, MessageMapItem_t::MessageFunc_t, const char *, const char *, bool );  // line 20
	static void KB_ChainToMap();  // line 20
	static void KB_AddBoundKey( const char *, int, int );  // line 20
	virtual vgui::PanelKeyBindingMap *GetKBMap();  // line 20
	static const char *GetPanelClassName();  // line 20
	static const char *GetPanelBaseClassName();  // line 20
	CPolygonButton( vgui::Panel *, const char * );  // line 24
	virtual void ApplySettings( KeyValues * );  // line 26
	virtual IBaseClientDLL::VPANEL IsWithinTraverse( int, int, bool );  // line 31
	virtual void PerformLayout();  // line 37
	virtual void ComputeAlignment( int &, int &, int &, int & );  // line 43
	virtual void PaintBackground();  // line 44
	virtual void PaintBorder();  // line 45
	virtual void ApplySchemeSettings( vgui::IScheme * );  // line 46
	virtual void UpdateHotspots( KeyValues * );  // line 48
protected:
	int m_nWhiteMaterial; // +0x1f4  // line 51
	CUtlVector<Vector2D,CUtlMemory<Vector2D, int> > m_unscaledHotspotPoints; // +0x1f8  // line 53
	CUtlVector<Vector2D,CUtlMemory<Vector2D, int> > m_unscaledVisibleHotspotPoints; // +0x20c  // line 54
	Vertex_t *m_hotspotPoints; // +0x220  // line 55
	int m_numHotspotPoints; // +0x224  // line 56
	Vertex_t *m_visibleHotspotPoints; // +0x228  // line 57
	int m_numVisibleHotspotPoints; // +0x22c  // line 58
	Vector2D m_hotspotMins; // +0x230  // line 60
	Vector2D m_hotspotMaxs; // +0x238  // line 61
};

// game/client/portal2/polygonbutton.h:19 (declaration)
~CPolygonButton();

// game/client/portal2/polygonbutton.h:19 @0x608630 _ZN14CPolygonButtonD1Ev
CPolygonButton::~CPolygonButton()
{
	// inlined CUtlVector<Vector2D,CUtlMemory<Vector2D, int> >::~CUtlVector() at line 19
	// inlined CUtlVector<Vector2D,CUtlMemory<Vector2D, int> >::~CUtlVector() at line 19
	// inlined CUtlVector<Vector2D,CUtlMemory<Vector2D, int> >::~CUtlVector() at line 19
}

// game/client/portal2/polygonbutton.h:19 @0x608870 _ZN14CPolygonButtonD0Ev
CPolygonButton::~CPolygonButton()
{
	// inlined CUtlVector<Vector2D,CUtlMemory<Vector2D, int> >::~CUtlVector() at line 19
	// inlined CUtlVector<Vector2D,CUtlMemory<Vector2D, int> >::~CUtlVector() at line 19
	// inlined CUtlVector<Vector2D,CUtlMemory<Vector2D, int> >::~CUtlVector() at line 19
}

// game/client/portal2/polygonbutton.h:20 @0x608300 _ZN14CPolygonButton17GetPanelClassNameEv
const char *CPolygonButton::GetPanelClassName()
{
}

// game/client/portal2/polygonbutton.h:20 @0x608310 _ZN14CPolygonButton13KB_ChainToMapEv
void CPolygonButton::KB_ChainToMap()
{
	vgui::PanelKeyBindingMap *map;  // line 20
}

// game/client/portal2/polygonbutton.h:20 @0x608370 _ZN14CPolygonButton19ChainToAnimationMapEv
void CPolygonButton::ChainToAnimationMap()
{
	PanelAnimationMap *map;  // line 20
}

// game/client/portal2/polygonbutton.h:20 @0x6083d0 _ZN14CPolygonButton10ChainToMapEv
void CPolygonButton::ChainToMap()
{
	vgui::PanelMessageMap *map;  // line 20
}

// game/client/portal2/polygonbutton.h:20 @0x608430 _ZN14CPolygonButton8GetKBMapEv
vgui::PanelKeyBindingMap *CPolygonButton::GetKBMap()
{
	vgui::PanelKeyBindingMap *s_pMap;  // line 20
}

// game/client/portal2/polygonbutton.h:20 @0x6084b0 _ZN14CPolygonButton10GetAnimMapEv
PanelAnimationMap *CPolygonButton::GetAnimMap()
{
}

// game/client/portal2/polygonbutton.h:20 @0x6084d0 _ZN14CPolygonButton13GetMessageMapEv
vgui::PanelMessageMap *CPolygonButton::GetMessageMap()
{
	vgui::PanelMessageMap *s_pMap;  // line 20
}
