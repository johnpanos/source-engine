// DWARF declaration skeleton for game/client/portal2/vgui/surveypanel.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal2/vgui/surveypanel.h:21 sizeof=0x184 (i386)
struct CSurveyQuestion : public vgui::EditablePanel
{
public:
	// game/client/portal2/vgui/surveypanel.h:23 sizeof=0x1 (i386)
	struct CSurveyQuestion_RegisterMap
	{
	public:
		CSurveyQuestion_RegisterMap();  // line 23
	};  // line 23
	// game/client/portal2/vgui/surveypanel.h:23 sizeof=0x1 (i386)
	struct CSurveyQuestion_Register
	{
	public:
		CSurveyQuestion_Register();  // line 23
	};  // line 23
	// game/client/portal2/vgui/surveypanel.h:23 sizeof=0x1 (i386)
	struct CSurveyQuestion_RegisterKBMap
	{
	public:
		CSurveyQuestion_RegisterKBMap();  // line 23
	};  // line 23
	CSurveyQuestion::CSurveyQuestion_RegisterMap m_RegisterClass; // +0x17c  // line 23
	CSurveyQuestion::CSurveyQuestion_Register m_RegisterAnimationClass; // +0x17d  // line 23
	CSurveyQuestion::CSurveyQuestion_RegisterKBMap m_RegisterClassKB; // +0x17e  // line 23
	static void AddToMap( const char *, MessageMapItem_t::MessageFunc_t, int, int, const char *, int, const char * );  // line 23
	static void ChainToMap();  // line 23
	virtual vgui::PanelMessageMap *GetMessageMap();  // line 23
	static void AddToAnimationMap( const char *, const char *, const char *, const char *, bool, PANELLOOKUPFUNC );  // line 23
	static void ChainToAnimationMap();  // line 23
	virtual PanelAnimationMap *GetAnimMap();  // line 23
	static void KB_AddToMap( const char *, KeyCode, int, MessageMapItem_t::MessageFunc_t, const char *, const char *, bool );  // line 23
	static void KB_ChainToMap();  // line 23
	static void KB_AddBoundKey( const char *, int, int );  // line 23
	virtual vgui::PanelKeyBindingMap *GetKBMap();  // line 23
	static const char *GetPanelClassName();  // line 23
	static const char *GetPanelBaseClassName();  // line 23
	CSurveyQuestion( vgui::Panel *, const char * );  // line 25
	// game/client/portal2/vgui/surveypanel.h:27
	enum questiontype_t
	{
		SVQ_ONE_TO_TEN = 0,
		SVQ_ESSAY = 1,
		SVQ_RADIOCHOICES = 2,
		SVQ_COUNT = 3,
	};  // line 27
	virtual void ApplySchemeSettings( vgui::IScheme * );  // line 36
	virtual void GetQuestionText( char *, int );  // line 39
	virtual void GetAnswerText( char *, int );  // line 40
protected:
	CSurveyQuestion::questiontype_t m_questiontype; // +0x180  // line 43
};

// game/client/portal2/vgui/surveypanel.h:21 (declaration)
~CSurveyQuestion();

// game/client/portal2/vgui/surveypanel.h:21 @0x61a5a0 _ZN15CSurveyQuestionD0Ev
CSurveyQuestion::~CSurveyQuestion()
{
}

// game/client/portal2/vgui/surveypanel.h:21 @0x61a5e0 _ZN15CSurveyQuestionD1Ev
CSurveyQuestion::~CSurveyQuestion()
{
}

// game/client/portal2/vgui/surveypanel.h:23 @0x61a450 _ZN15CSurveyQuestion17GetPanelClassNameEv
const char *CSurveyQuestion::GetPanelClassName()
{
}

// game/client/portal2/vgui/surveypanel.h:23 @0x61a780 _ZN15CSurveyQuestion13KB_ChainToMapEv
void CSurveyQuestion::KB_ChainToMap()
{
	vgui::PanelKeyBindingMap *map;  // line 23
}

// game/client/portal2/vgui/surveypanel.h:23 @0x61a960 _ZN15CSurveyQuestion19ChainToAnimationMapEv
void CSurveyQuestion::ChainToAnimationMap()
{
	PanelAnimationMap *map;  // line 23
}

// game/client/portal2/vgui/surveypanel.h:23 @0x61ab40 _ZN15CSurveyQuestion10ChainToMapEv
void CSurveyQuestion::ChainToMap()
{
	vgui::PanelMessageMap *map;  // line 23
}

// game/client/portal2/vgui/surveypanel.h:23 @0x61ae70 _ZN15CSurveyQuestion10GetAnimMapEv
PanelAnimationMap *CSurveyQuestion::GetAnimMap()
{
}

// game/client/portal2/vgui/surveypanel.h:23 @0x61ae90 _ZN15CSurveyQuestion13GetMessageMapEv
vgui::PanelMessageMap *CSurveyQuestion::GetMessageMap()
{
	vgui::PanelMessageMap *s_pMap;  // line 23
}

// game/client/portal2/vgui/surveypanel.h:23 @0x61af10 _ZN15CSurveyQuestion8GetKBMapEv
vgui::PanelKeyBindingMap *CSurveyQuestion::GetKBMap()
{
	vgui::PanelKeyBindingMap *s_pMap;  // line 23
}

// game/client/portal2/vgui/surveypanel.h:39 @0x61a460 _ZN15CSurveyQuestion15GetQuestionTextEPci
void CSurveyQuestion::GetQuestionText( char *pBuffOut, int iBuffLen )
{
}

// game/client/portal2/vgui/surveypanel.h:40 @0x61a470 _ZN15CSurveyQuestion13GetAnswerTextEPci
void CSurveyQuestion::GetAnswerText( char *pBuffOut, int iBuffLen )
{
}

// game/client/portal2/vgui/surveypanel.h:47 sizeof=0x334 (i386)
struct CSurveyPanel : public vgui::Frame, public IViewPortPanel
{
public:
	// game/client/portal2/vgui/surveypanel.h:49 sizeof=0x1 (i386)
	struct CSurveyPanel_RegisterMap
	{
	public:
		CSurveyPanel_RegisterMap();  // line 49
	};  // line 49
	// game/client/portal2/vgui/surveypanel.h:49 sizeof=0x1 (i386)
	struct CSurveyPanel_Register
	{
	public:
		CSurveyPanel_Register();  // line 49
	};  // line 49
	// game/client/portal2/vgui/surveypanel.h:49 sizeof=0x1 (i386)
	struct CSurveyPanel_RegisterKBMap
	{
	public:
		CSurveyPanel_RegisterKBMap();  // line 49
	};  // line 49
	CSurveyPanel::CSurveyPanel_RegisterMap m_RegisterClass; // +0x214  // line 49
	CSurveyPanel::CSurveyPanel_Register m_RegisterAnimationClass; // +0x215  // line 49
	CSurveyPanel::CSurveyPanel_RegisterKBMap m_RegisterClassKB; // +0x216  // line 49
	static void AddToMap( const char *, MessageMapItem_t::MessageFunc_t, int, int, const char *, int, const char * );  // line 49
	static void ChainToMap();  // line 49
	virtual vgui::PanelMessageMap *GetMessageMap();  // line 49
	static void AddToAnimationMap( const char *, const char *, const char *, const char *, bool, PANELLOOKUPFUNC );  // line 49
	static void ChainToAnimationMap();  // line 49
	virtual PanelAnimationMap *GetAnimMap();  // line 49
	static void KB_AddToMap( const char *, KeyCode, int, MessageMapItem_t::MessageFunc_t, const char *, const char *, bool );  // line 49
	static void KB_ChainToMap();  // line 49
	static void KB_AddBoundKey( const char *, int, int );  // line 49
	virtual vgui::PanelKeyBindingMap *GetKBMap();  // line 49
	static const char *GetPanelClassName();  // line 49
	static const char *GetPanelBaseClassName();  // line 49
	CSurveyPanel( IViewPort * );  // line 52
	virtual ~CSurveyPanel();  // line 53
	virtual const char *GetName();  // line 56
	virtual void SetData( KeyValues * );  // line 57
	virtual void Reset();  // line 58
	virtual void Update();  // line 59
	virtual bool NeedsUpdate();  // line 60
	virtual bool HasInputElements();  // line 61
	virtual void ShowPanel( bool );  // line 62
	virtual bool WantsBackgroundBlurred();  // line 63
	virtual IBaseClientDLL::VPANEL GetVPanel();  // line 65
	virtual bool IsVisible();  // line 66
	virtual void SetParent( IBaseClientDLL::VPANEL );  // line 67
	virtual void OnCommand( const char * );  // line 69
	EHANDLE m_hPointSurveyEntity; // +0x218  // line 71
	void SendSurveyDoneCmd();  // line 72
protected:
	bool WriteSurveyDataToFile( const char * );  // line 74
	IViewPort *m_pViewPort; // +0x21c  // line 76
	char m_szSurveyName[260]; // +0x220  // line 79
	int id; // +0x324  // line 85
	int version; // +0x328  // line 86
	vgui::PanelListPanel *m_pQuestionsList; // +0x32c  // line 88
	vgui::Button *m_pSubmitButton; // +0x330  // line 90
};

// game/client/portal2/vgui/surveypanel.h:49 @0x61a480 _ZN12CSurveyPanel17GetPanelClassNameEv
const char *CSurveyPanel::GetPanelClassName()
{
}

// game/client/portal2/vgui/surveypanel.h:49 @0x61a720 _ZN12CSurveyPanel13KB_ChainToMapEv
void CSurveyPanel::KB_ChainToMap()
{
	vgui::PanelKeyBindingMap *map;  // line 49
}

// game/client/portal2/vgui/surveypanel.h:49 @0x61a900 _ZN12CSurveyPanel19ChainToAnimationMapEv
void CSurveyPanel::ChainToAnimationMap()
{
	PanelAnimationMap *map;  // line 49
}

// game/client/portal2/vgui/surveypanel.h:49 @0x61aae0 _ZN12CSurveyPanel10ChainToMapEv
void CSurveyPanel::ChainToMap()
{
	vgui::PanelMessageMap *map;  // line 49
}

// game/client/portal2/vgui/surveypanel.h:49 @0x61ad50 _ZN12CSurveyPanel8GetKBMapEv
vgui::PanelKeyBindingMap *CSurveyPanel::GetKBMap()
{
	vgui::PanelKeyBindingMap *s_pMap;  // line 49
}

// game/client/portal2/vgui/surveypanel.h:49 @0x61add0 _ZN12CSurveyPanel10GetAnimMapEv
PanelAnimationMap *CSurveyPanel::GetAnimMap()
{
}

// game/client/portal2/vgui/surveypanel.h:49 @0x61adf0 _ZN12CSurveyPanel13GetMessageMapEv
vgui::PanelMessageMap *CSurveyPanel::GetMessageMap()
{
	vgui::PanelMessageMap *s_pMap;  // line 49
}

// game/client/portal2/vgui/surveypanel.h:56 @0x61a4a0 _ZN12CSurveyPanel7GetNameEv
const char *CSurveyPanel::GetName()
{
}

// game/client/portal2/vgui/surveypanel.h:61 @0x61a4c0 _ZN12CSurveyPanel16HasInputElementsEv
bool CSurveyPanel::HasInputElements()
{
}

// game/client/portal2/vgui/surveypanel.h:63 @0x61a4e0 _ZN12CSurveyPanel22WantsBackgroundBlurredEv
bool CSurveyPanel::WantsBackgroundBlurred()
{
}

// game/client/portal2/vgui/surveypanel.h:65 @0x61b1e0 _ZN12CSurveyPanel9GetVPanelEv
IBaseClientDLL::VPANEL CSurveyPanel::GetVPanel()
{
}

// game/client/portal2/vgui/surveypanel.h:66 @0x61a560 _ZN12CSurveyPanel9IsVisibleEv
bool CSurveyPanel::IsVisible()
{
}

// game/client/portal2/vgui/surveypanel.h:67 @0x61a590 _ZN12CSurveyPanel9SetParentEj
void CSurveyPanel::SetParent( IBaseClientDLL::VPANEL parent )
{
}
