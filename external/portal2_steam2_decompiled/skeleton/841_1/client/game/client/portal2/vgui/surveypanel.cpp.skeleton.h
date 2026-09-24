// DWARF declaration skeleton for game/client/portal2/vgui/surveypanel.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x81470 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CTimeInMapHelper::CTimeInMapHelper() at line 52
}

// game/client/portal2/vgui/surveypanel.cpp:20
static char *const g_szUnansweredQuestion;

// game/client/portal2/vgui/surveypanel.cpp:22
ConVar cl_disable_survey_panel;

// game/client/portal2/vgui/surveypanel.cpp:23
ConVar cl_survey_panel_dont_submit_answers;

// game/client/portal2/vgui/surveypanel.cpp:26 sizeof=0x114 (i386)
struct CTimeInMapHelper : public CAutoGameSystem
{
private:
	virtual const char *Name();  // line 27
	virtual bool Init();  // line 29
	virtual void LevelInitPreEntity();  // line 35
public:
	float TimeSpentInCurrentMap();  // line 46
protected:
	float m_flTimeMapStart; // +0xc  // line 49
	char m_szCurMapName[260]; // +0x10  // line 50
};

// game/client/portal2/vgui/surveypanel.cpp:26 (declaration)
void CTimeInMapHelper();

// game/client/portal2/vgui/surveypanel.cpp:26 (declaration)
~CTimeInMapHelper();

// game/client/portal2/vgui/surveypanel.cpp:26 @0x627400 _ZN16CTimeInMapHelperD1Ev
CTimeInMapHelper::~CTimeInMapHelper()
{
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 26
}

// game/client/portal2/vgui/surveypanel.cpp:26 @0x6274f0 _ZN16CTimeInMapHelperD0Ev
CTimeInMapHelper::~CTimeInMapHelper()
{
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 26
}

// game/client/portal2/vgui/surveypanel.cpp:27 @0x626700 _ZN16CTimeInMapHelper4NameEv
const char *CTimeInMapHelper::Name()
{
}

// game/client/portal2/vgui/surveypanel.cpp:29 @0x626710 _ZN16CTimeInMapHelper4InitEv
bool CTimeInMapHelper::Init()
{
}

// game/client/portal2/vgui/surveypanel.cpp:35 @0x626db0 _ZN16CTimeInMapHelper18LevelInitPreEntityEv
void CTimeInMapHelper::LevelInitPreEntity()
{
}

// game/client/portal2/vgui/surveypanel.cpp:52
static CTimeInMapHelper g_TimeInMapHelper;

// game/client/portal2/vgui/surveypanel.cpp:54 (declaration)
void ApplySchemeSettings( vgui::IScheme *pScheme );

// game/client/portal2/vgui/surveypanel.cpp:54 @0x6255b0 _ZN15CSurveyQuestion19ApplySchemeSettingsEPN4vgui7ISchemeE
void CSurveyQuestion::ApplySchemeSettings( vgui::IScheme *pScheme )
{
}

// game/client/portal2/vgui/surveypanel.cpp:61 sizeof=0x190 (i386)
struct CEssayQuestion : public CSurveyQuestion
{
private:
	static const int ESSAY_QUESTION_MAX_CHARS;  // line 62
public:
	// game/client/portal2/vgui/surveypanel.cpp:63 sizeof=0x1 (i386)
	struct CEssayQuestion_RegisterMap
	{
	public:
		CEssayQuestion_RegisterMap();  // line 63
	};  // line 63
	// game/client/portal2/vgui/surveypanel.cpp:63 sizeof=0x1 (i386)
	struct CEssayQuestion_Register
	{
	public:
		CEssayQuestion_Register();  // line 63
	};  // line 63
	// game/client/portal2/vgui/surveypanel.cpp:63 sizeof=0x1 (i386)
	struct CEssayQuestion_RegisterKBMap
	{
	public:
		CEssayQuestion_RegisterKBMap();  // line 63
	};  // line 63
	CEssayQuestion::CEssayQuestion_RegisterMap m_RegisterClass; // +0x184  // line 63
	CEssayQuestion::CEssayQuestion_Register m_RegisterAnimationClass; // +0x185  // line 63
	CEssayQuestion::CEssayQuestion_RegisterKBMap m_RegisterClassKB; // +0x186  // line 63
	static void AddToMap( const char *, MessageMapItem_t::MessageFunc_t, int, int, const char *, int, const char * );  // line 63
	static void ChainToMap();  // line 63
	virtual vgui::PanelMessageMap *GetMessageMap();  // line 63
	static void AddToAnimationMap( const char *, const char *, const char *, const char *, bool, PANELLOOKUPFUNC );  // line 63
	static void ChainToAnimationMap();  // line 63
	virtual PanelAnimationMap *GetAnimMap();  // line 63
	static void KB_AddToMap( const char *, KeyCode, int, MessageMapItem_t::MessageFunc_t, const char *, const char *, bool );  // line 63
	static void KB_ChainToMap();  // line 63
	static void KB_AddBoundKey( const char *, int, int );  // line 63
	virtual vgui::PanelKeyBindingMap *GetKBMap();  // line 63
	static const char *GetPanelClassName();  // line 63
	static const char *GetPanelBaseClassName();  // line 63
	CEssayQuestion( vgui::Panel *, const char * );  // line 65
	virtual ~CEssayQuestion();  // line 77
	virtual void ApplySchemeSettings( vgui::IScheme * );  // line 83
	bool SetData( KeyValues * );  // line 91
	virtual void GetQuestionText( char *, int );  // line 93
	virtual void GetAnswerText( char *, int );  // line 94
protected:
	vgui::Label *m_pQuestionText; // +0x188  // line 97
	vgui::TextEntry *m_pAnswer; // +0x18c  // line 98
};

// game/client/portal2/vgui/surveypanel.cpp:63 @0x626730 _ZN14CEssayQuestion17GetPanelClassNameEv
const char *CEssayQuestion::GetPanelClassName()
{
}

// game/client/portal2/vgui/surveypanel.cpp:63 @0x6268d0 _ZN14CEssayQuestion13KB_ChainToMapEv
void CEssayQuestion::KB_ChainToMap()
{
	vgui::PanelKeyBindingMap *map;  // line 63
}

// game/client/portal2/vgui/surveypanel.cpp:63 @0x626ab0 _ZN14CEssayQuestion19ChainToAnimationMapEv
void CEssayQuestion::ChainToAnimationMap()
{
	PanelAnimationMap *map;  // line 63
}

// game/client/portal2/vgui/surveypanel.cpp:63 @0x626c90 _ZN14CEssayQuestion10ChainToMapEv
void CEssayQuestion::ChainToMap()
{
	vgui::PanelMessageMap *map;  // line 63
}

// game/client/portal2/vgui/surveypanel.cpp:63 @0x627100 _ZN14CEssayQuestion8GetKBMapEv
vgui::PanelKeyBindingMap *CEssayQuestion::GetKBMap()
{
	vgui::PanelKeyBindingMap *s_pMap;  // line 63
}

// game/client/portal2/vgui/surveypanel.cpp:63 @0x627180 _ZN14CEssayQuestion10GetAnimMapEv
PanelAnimationMap *CEssayQuestion::GetAnimMap()
{
}

// game/client/portal2/vgui/surveypanel.cpp:63 @0x6271a0 _ZN14CEssayQuestion13GetMessageMapEv
vgui::PanelMessageMap *CEssayQuestion::GetMessageMap()
{
	vgui::PanelMessageMap *s_pMap;  // line 63
}

// game/client/portal2/vgui/surveypanel.cpp:77 @0x627aa0 _ZN14CEssayQuestionD0Ev
CEssayQuestion::~CEssayQuestion()
{
	// inlined CSurveyQuestion::~CSurveyQuestion() at line 81
	// inlined CSurveyQuestion::~CSurveyQuestion() at line 81
}

// game/client/portal2/vgui/surveypanel.cpp:77 @0x627b30 _ZN14CEssayQuestionD1Ev
CEssayQuestion::~CEssayQuestion()
{
	// inlined CSurveyQuestion::~CSurveyQuestion() at line 81
	// inlined CSurveyQuestion::~CSurveyQuestion() at line 81
}

// game/client/portal2/vgui/surveypanel.cpp:83 @0x627420 _ZN14CEssayQuestion19ApplySchemeSettingsEPN4vgui7ISchemeE
void CEssayQuestion::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	// inlined CSurveyQuestion::ApplySchemeSettings() at line 88
}

// game/client/portal2/vgui/surveypanel.cpp:91 @0x6252a0 _ZN14CEssayQuestion7SetDataEP9KeyValues
bool CEssayQuestion::SetData( KeyValues *pKeys )
{
	KeyValues *pQuestionData;  // line 104
}

// game/client/portal2/vgui/surveypanel.cpp:93 @0x624160 _ZN14CEssayQuestion15GetQuestionTextEPci
void CEssayQuestion::GetQuestionText( char *pBuffOut, int iBuffLen )
{
}

// game/client/portal2/vgui/surveypanel.cpp:94 @0x624180 _ZN14CEssayQuestion13GetAnswerTextEPci
void CEssayQuestion::GetAnswerText( char *pBuffOut, int iBuffLen )
{
}

// game/client/portal2/vgui/surveypanel.cpp:126 sizeof=0x1b8 (i386)
struct CRadioChoicesQuestion : public CSurveyQuestion
{
private:
	static const int MAX_RADIO_BUTTONS;  // line 127
	static const int RADIO_BUTTON_CHOICE_HEIGHT;  // line 128
public:
	// game/client/portal2/vgui/surveypanel.cpp:129 sizeof=0x1 (i386)
	struct CRadioChoicesQuestion_RegisterMap
	{
	public:
		CRadioChoicesQuestion_RegisterMap();  // line 129
	};  // line 129
	// game/client/portal2/vgui/surveypanel.cpp:129 sizeof=0x1 (i386)
	struct CRadioChoicesQuestion_Register
	{
	public:
		CRadioChoicesQuestion_Register();  // line 129
	};  // line 129
	// game/client/portal2/vgui/surveypanel.cpp:129 sizeof=0x1 (i386)
	struct CRadioChoicesQuestion_RegisterKBMap
	{
	public:
		CRadioChoicesQuestion_RegisterKBMap();  // line 129
	};  // line 129
	CRadioChoicesQuestion::CRadioChoicesQuestion_RegisterMap m_RegisterClass; // +0x184  // line 129
	CRadioChoicesQuestion::CRadioChoicesQuestion_Register m_RegisterAnimationClass; // +0x185  // line 129
	CRadioChoicesQuestion::CRadioChoicesQuestion_RegisterKBMap m_RegisterClassKB; // +0x186  // line 129
	static void AddToMap( const char *, MessageMapItem_t::MessageFunc_t, int, int, const char *, int, const char * );  // line 129
	static void ChainToMap();  // line 129
	virtual vgui::PanelMessageMap *GetMessageMap();  // line 129
	static void AddToAnimationMap( const char *, const char *, const char *, const char *, bool, PANELLOOKUPFUNC );  // line 129
	static void ChainToAnimationMap();  // line 129
	virtual PanelAnimationMap *GetAnimMap();  // line 129
	static void KB_AddToMap( const char *, KeyCode, int, MessageMapItem_t::MessageFunc_t, const char *, const char *, bool );  // line 129
	static void KB_ChainToMap();  // line 129
	static void KB_AddBoundKey( const char *, int, int );  // line 129
	virtual vgui::PanelKeyBindingMap *GetKBMap();  // line 129
	static const char *GetPanelClassName();  // line 129
	static const char *GetPanelBaseClassName();  // line 129
	CRadioChoicesQuestion( vgui::Panel *, const char * );  // line 131
	virtual ~CRadioChoicesQuestion();  // line 140
	bool SetData( KeyValues * );  // line 152
	virtual void GetQuestionText( char *, int );  // line 154
	virtual void GetAnswerText( char *, int );  // line 155
protected:
	vgui::Label *m_pQuestionText; // +0x188  // line 157
	vgui::RadioButton *m_pRadioButtons[10]; // +0x18c  // line 158
	int m_iNumRadioButtons; // +0x1b4  // line 159
};

// game/client/portal2/vgui/surveypanel.cpp:129 @0x626740 _ZN21CRadioChoicesQuestion17GetPanelClassNameEv
const char *CRadioChoicesQuestion::GetPanelClassName()
{
}

// game/client/portal2/vgui/surveypanel.cpp:129 @0x626870 _ZN21CRadioChoicesQuestion13KB_ChainToMapEv
void CRadioChoicesQuestion::KB_ChainToMap()
{
	vgui::PanelKeyBindingMap *map;  // line 129
}

// game/client/portal2/vgui/surveypanel.cpp:129 @0x626a50 _ZN21CRadioChoicesQuestion19ChainToAnimationMapEv
void CRadioChoicesQuestion::ChainToAnimationMap()
{
	PanelAnimationMap *map;  // line 129
}

// game/client/portal2/vgui/surveypanel.cpp:129 @0x626c30 _ZN21CRadioChoicesQuestion10ChainToMapEv
void CRadioChoicesQuestion::ChainToMap()
{
	vgui::PanelMessageMap *map;  // line 129
}

// game/client/portal2/vgui/surveypanel.cpp:129 @0x627220 _ZN21CRadioChoicesQuestion8GetKBMapEv
vgui::PanelKeyBindingMap *CRadioChoicesQuestion::GetKBMap()
{
	vgui::PanelKeyBindingMap *s_pMap;  // line 129
}

// game/client/portal2/vgui/surveypanel.cpp:129 @0x6272a0 _ZN21CRadioChoicesQuestion10GetAnimMapEv
PanelAnimationMap *CRadioChoicesQuestion::GetAnimMap()
{
}

// game/client/portal2/vgui/surveypanel.cpp:129 @0x6272c0 _ZN21CRadioChoicesQuestion13GetMessageMapEv
vgui::PanelMessageMap *CRadioChoicesQuestion::GetMessageMap()
{
	vgui::PanelMessageMap *s_pMap;  // line 129
}

// game/client/portal2/vgui/surveypanel.cpp:140 @0x627810 _ZN21CRadioChoicesQuestionD0Ev
CRadioChoicesQuestion::~CRadioChoicesQuestion()
{
	{
		int i;  // line 143
	}
	// inlined CSurveyQuestion::~CSurveyQuestion() at line 150
	// inlined CSurveyQuestion::~CSurveyQuestion() at line 150
}

// game/client/portal2/vgui/surveypanel.cpp:140 @0x627960 _ZN21CRadioChoicesQuestionD1Ev
CRadioChoicesQuestion::~CRadioChoicesQuestion()
{
	{
		int i;  // line 143
	}
	// inlined CSurveyQuestion::~CSurveyQuestion() at line 150
	// inlined CSurveyQuestion::~CSurveyQuestion() at line 150
}

// game/client/portal2/vgui/surveypanel.cpp:152 @0x625360 _ZN21CRadioChoicesQuestion7SetDataEP9KeyValues
bool CRadioChoicesQuestion::SetData( KeyValues *pKeys )
{
	KeyValues *pQuestionData;  // line 164
	{
		int iDummy;  // line 175
		int iLeftPadding;  // line 175
	}
}

// game/client/portal2/vgui/surveypanel.cpp:154 @0x6241a0 _ZN21CRadioChoicesQuestion15GetQuestionTextEPci
void CRadioChoicesQuestion::GetQuestionText( char *pBuffOut, int iBuffLen )
{
}

// game/client/portal2/vgui/surveypanel.cpp:155 @0x624410 _ZN21CRadioChoicesQuestion13GetAnswerTextEPci
void CRadioChoicesQuestion::GetAnswerText( char *pBuffOut, int iBuffLen )
{
	bool bFound;  // line 198
	{
		int i;  // line 199
	}
}

// game/client/portal2/vgui/surveypanel.cpp:215 sizeof=0x1bc (i386)
struct COneToTenQuestion : public CSurveyQuestion
{
private:
	static const int MAX_RADIO_BUTTONS;  // line 216
	static const int RADIO_BUTTON_TALL;  // line 217
public:
	// game/client/portal2/vgui/surveypanel.cpp:218 sizeof=0x1 (i386)
	struct COneToTenQuestion_RegisterMap
	{
	public:
		COneToTenQuestion_RegisterMap();  // line 218
	};  // line 218
	// game/client/portal2/vgui/surveypanel.cpp:218 sizeof=0x1 (i386)
	struct COneToTenQuestion_Register
	{
	public:
		COneToTenQuestion_Register();  // line 218
	};  // line 218
	// game/client/portal2/vgui/surveypanel.cpp:218 sizeof=0x1 (i386)
	struct COneToTenQuestion_RegisterKBMap
	{
	public:
		COneToTenQuestion_RegisterKBMap();  // line 218
	};  // line 218
	COneToTenQuestion::COneToTenQuestion_RegisterMap m_RegisterClass; // +0x184  // line 218
	COneToTenQuestion::COneToTenQuestion_Register m_RegisterAnimationClass; // +0x185  // line 218
	COneToTenQuestion::COneToTenQuestion_RegisterKBMap m_RegisterClassKB; // +0x186  // line 218
	static void AddToMap( const char *, MessageMapItem_t::MessageFunc_t, int, int, const char *, int, const char * );  // line 218
	static void ChainToMap();  // line 218
	virtual vgui::PanelMessageMap *GetMessageMap();  // line 218
	static void AddToAnimationMap( const char *, const char *, const char *, const char *, bool, PANELLOOKUPFUNC );  // line 218
	static void ChainToAnimationMap();  // line 218
	virtual PanelAnimationMap *GetAnimMap();  // line 218
	static void KB_AddToMap( const char *, KeyCode, int, MessageMapItem_t::MessageFunc_t, const char *, const char *, bool );  // line 218
	static void KB_ChainToMap();  // line 218
	static void KB_AddBoundKey( const char *, int, int );  // line 218
	virtual vgui::PanelKeyBindingMap *GetKBMap();  // line 218
	static const char *GetPanelClassName();  // line 218
	static const char *GetPanelBaseClassName();  // line 218
	COneToTenQuestion( vgui::Panel *, const char * );  // line 220
	virtual ~COneToTenQuestion();  // line 240
	bool SetData( KeyValues * );  // line 251
	virtual void GetQuestionText( char *, int );  // line 253
	virtual void GetAnswerText( char *, int );  // line 254
protected:
	vgui::RadioButton *m_pRadioButtons[10]; // +0x188  // line 257
	vgui::Label *m_pQuestionText; // +0x1b0  // line 258
	vgui::Label *m_pLabelAtOne; // +0x1b4  // line 259
	vgui::Label *m_pLabelAtTen; // +0x1b8  // line 260
};

// game/client/portal2/vgui/surveypanel.cpp:218 @0x626750 _ZN17COneToTenQuestion17GetPanelClassNameEv
const char *COneToTenQuestion::GetPanelClassName()
{
}

// game/client/portal2/vgui/surveypanel.cpp:218 @0x626810 _ZN17COneToTenQuestion13KB_ChainToMapEv
void COneToTenQuestion::KB_ChainToMap()
{
	vgui::PanelKeyBindingMap *map;  // line 218
}

// game/client/portal2/vgui/surveypanel.cpp:218 @0x6269f0 _ZN17COneToTenQuestion19ChainToAnimationMapEv
void COneToTenQuestion::ChainToAnimationMap()
{
	PanelAnimationMap *map;  // line 218
}

// game/client/portal2/vgui/surveypanel.cpp:218 @0x626bd0 _ZN17COneToTenQuestion10ChainToMapEv
void COneToTenQuestion::ChainToMap()
{
	vgui::PanelMessageMap *map;  // line 218
}

// game/client/portal2/vgui/surveypanel.cpp:218 @0x626e40 _ZN17COneToTenQuestion13GetMessageMapEv
vgui::PanelMessageMap *COneToTenQuestion::GetMessageMap()
{
	vgui::PanelMessageMap *s_pMap;  // line 218
}

// game/client/portal2/vgui/surveypanel.cpp:218 @0x627340 _ZN17COneToTenQuestion8GetKBMapEv
vgui::PanelKeyBindingMap *COneToTenQuestion::GetKBMap()
{
	vgui::PanelKeyBindingMap *s_pMap;  // line 218
}

// game/client/portal2/vgui/surveypanel.cpp:218 @0x6273c0 _ZN17COneToTenQuestion10GetAnimMapEv
PanelAnimationMap *COneToTenQuestion::GetAnimMap()
{
}

// game/client/portal2/vgui/surveypanel.cpp:240 @0x627530 _ZN17COneToTenQuestionD0Ev
COneToTenQuestion::~COneToTenQuestion()
{
	{
		int i;  // line 245
	}
	// inlined CSurveyQuestion::~CSurveyQuestion() at line 249
	// inlined CSurveyQuestion::~CSurveyQuestion() at line 249
}

// game/client/portal2/vgui/surveypanel.cpp:240 @0x6276a0 _ZN17COneToTenQuestionD1Ev
COneToTenQuestion::~COneToTenQuestion()
{
	// inlined COneToTenQuestion::~COneToTenQuestion() at line 249
}

// game/client/portal2/vgui/surveypanel.cpp:251 @0x625120 _ZN17COneToTenQuestion7SetDataEP9KeyValues
bool COneToTenQuestion::SetData( KeyValues *pKeys )
{
	KeyValues *pQuestionData;  // line 265
}

// game/client/portal2/vgui/surveypanel.cpp:253 @0x6241c0 _ZN17COneToTenQuestion15GetQuestionTextEPci
void COneToTenQuestion::GetQuestionText( char *pBuffOut, int iBuffLen )
{
}

// game/client/portal2/vgui/surveypanel.cpp:254 @0x624250 _ZN17COneToTenQuestion13GetAnswerTextEPci
void COneToTenQuestion::GetAnswerText( char *pBuffOut, int iBuffLen )
{
	bool bFound;  // line 292
	{
		int i;  // line 293
	}
}

// game/client/portal2/vgui/surveypanel.cpp:309 (declaration)
void CSurveyPanel( IViewPort *pViewport );

// game/client/portal2/vgui/surveypanel.cpp:309 @0x625800 _ZN12CSurveyPanelC2EP9IViewPort
CSurveyPanel::CSurveyPanel( IViewPort *pViewport )
{
	// inlined CHandle<C_BaseEntity>::CHandle() at line 311
	// inlined CSurveyPanel::CSurveyPanel_RegisterKBMap::CSurveyPanel_RegisterKBMap() at line 311
	// inlined CSurveyPanel::CSurveyPanel_Register::CSurveyPanel_Register() at line 311
	// inlined CSurveyPanel::CSurveyPanel_RegisterMap::CSurveyPanel_RegisterMap() at line 311
	// inlined IViewPortPanel::~IViewPortPanel() at line 329
}

// game/client/portal2/vgui/surveypanel.cpp:309 @0x625a90 _ZN12CSurveyPanelC1EP9IViewPort
CSurveyPanel::CSurveyPanel( IViewPort *pViewport )
{
}

// game/client/portal2/vgui/surveypanel.cpp:331 (declaration)
~CSurveyPanel();

// game/client/portal2/vgui/surveypanel.cpp:331 @0x624f80 _ZN12CSurveyPanelD0Ev
CSurveyPanel::~CSurveyPanel()
{
}

// game/client/portal2/vgui/surveypanel.cpp:331 @0x625020 _ZN12CSurveyPanelD1Ev
CSurveyPanel::~CSurveyPanel()
{
}

// game/client/portal2/vgui/surveypanel.cpp:331 @0x6250a0 _ZN12CSurveyPanelD2Ev
CSurveyPanel::~CSurveyPanel()
{
}

// game/client/portal2/vgui/surveypanel.cpp:338 @0x625ab0 _ZN12CSurveyPanel7SetDataEP9KeyValues
void CSurveyPanel::SetData( KeyValues *data )
{
	KeyValues *pQuestionEntry;  // line 347
	{
		COneToTenQuestion *pOneToTen;  // line 352
		// inlined COneToTenQuestion::COneToTenQuestion() at line 352
	}
	{
		CRadioChoicesQuestion *pRadioChoices;  // line 364
		// inlined CRadioChoicesQuestion::CRadioChoicesQuestion() at line 364
	}
	{
		CEssayQuestion *pEssayQuestion;  // line 358
		// inlined CEssayQuestion::CEssayQuestion() at line 358
	}
}

// game/client/portal2/vgui/surveypanel.cpp:374 @0x6241f0 _ZN12CSurveyPanel5ResetEv
void CSurveyPanel::Reset()
{
}

// game/client/portal2/vgui/surveypanel.cpp:379 @0x624220 _ZN12CSurveyPanel6UpdateEv
void CSurveyPanel::Update()
{
}

// game/client/portal2/vgui/surveypanel.cpp:383 @0x624240 _ZN12CSurveyPanel11NeedsUpdateEv
bool CSurveyPanel::NeedsUpdate()
{
}

// game/client/portal2/vgui/surveypanel.cpp:388 @0x625710 _ZN12CSurveyPanel9ShowPanelEb
void CSurveyPanel::ShowPanel( bool bShow )
{
}

// game/client/portal2/vgui/surveypanel.cpp:413 @0x6255f0 _ZN12CSurveyPanel17SendSurveyDoneCmdEv
void CSurveyPanel::SendSurveyDoneCmd()
{
	CUtlString strCmd;  // line 415
	int nEntityIndex;  // line 416
	// inlined CUtlString::~CUtlString() at line 418
	// inlined CHandle<C_BaseEntity>::operator C_BaseEntity*() at line 416
	// inlined CUtlString::~CUtlString() at line 418
}

// game/client/portal2/vgui/surveypanel.cpp:422 @0x624740 _ZN12CSurveyPanel21WriteSurveyDataToFileEPKc
bool CSurveyPanel::WriteSurveyDataToFile( const char *file )
{
	FileHandle_t handle;  // line 428
	tm t;  // line 435
	char pszMap[64];  // line 437
	C_Portal_Player *pPlayer;  // line 450
	{
		int i;  // line 456
		{
			CSurveyQuestion *pQuestion;  // line 458
			char szAnswerBuff[260];  // line 460
			char szOutputBuff[8192];  // line 463
		}
	}
	// inlined CTimeInMapHelper::TimeSpentInCurrentMap() at line 453
	// inlined C_Portal_Player::GetLocalPortalPlayer() at line 450
}

// game/client/portal2/vgui/surveypanel.cpp:474 @0x624b30 _ZN12CSurveyPanel9OnCommandEPKc
void CSurveyPanel::OnCommand( const char *command )
{
	{
		char pszFileName[256];  // line 496
		char pszMap[64];  // line 497
		tm t;  // line 498
		char pszFilePath[512];  // line 503
		{
			char szDifficulty[260];  // line 482
			char szEnjoyment[260];  // line 483
			char szEssay[260];  // line 484
		}
	}
}

// game/client/portal2/vgui/surveypanel.cpp:531 @0x624710 _ZL28cl_show_portal_puzzle_surveyRK8CCommand
cl_show_portal_puzzle_survey( const CCommand &args )
{
	const char *pszSurveyName;  // line 539
}

// game/client/portal2/vgui/surveypanel.cpp:531
static ConCommand cl_show_portal_puzzle_survey_command;

// game/client/portal2/vgui/surveypanel.cpp:544 @0x6244b0 _Z15ShowSurveyPanelPKc
bool ShowSurveyPanel( const char *pszSurveyName )
{
	CBaseViewport *pViewport;  // line 553
	IViewPortPanel *pSurveyPanel;  // line 554
	char pszFilePath[260];  // line 573
	KeyValues *pKV;  // line 576
}
