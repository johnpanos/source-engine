// DWARF declaration skeleton for game/client/PanelMetaClassMgr.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/PanelMetaClassMgr.h:37 sizeof=0x4 (i386)
struct IPanelFactory
{
public:
	int (**_vptr$IPanelFactory)(); // +0x0  // line 0
	virtual vgui::Panel *Create( const char *, KeyValues *, void *, vgui::Panel * );  // line 40
};

// game/client/PanelMetaClassMgr.h:96 sizeof=0x4 (i386)
struct CPanelFactory<CIndicatorScreen,VGuiScreenInitData_t> : public IPanelFactory
{
public:
	void CPanelFactory( const char * );  // line 98
	virtual vgui::Panel *Create( const char *, KeyValues *, void *, vgui::Panel * );  // line 106
};

// game/client/PanelMetaClassMgr.h:96 sizeof=0x4 (i386)
struct CPanelFactory<CLevelPlacardScreen,VGuiScreenInitData_t> : public IPanelFactory
{
public:
	void CPanelFactory( const char * );  // line 98
	virtual vgui::Panel *Create( const char *, KeyValues *, void *, vgui::Panel * );  // line 106
};

// game/client/PanelMetaClassMgr.h:96 sizeof=0x4 (i386)
struct CPanelFactory<CObjectControlPanel,VGuiScreenInitData_t> : public IPanelFactory
{
public:
	void CPanelFactory( const char * );  // line 98
	virtual vgui::Panel *Create( const char *, KeyValues *, void *, vgui::Panel * );  // line 106
};

// game/client/PanelMetaClassMgr.h:96 sizeof=0x4 (i386)
struct CPanelFactory<CVGUI_MP_LobbyScreen,VGuiScreenInitData_t> : public IPanelFactory
{
public:
	void CPanelFactory( const char * );  // line 98
	virtual vgui::Panel *Create( const char *, KeyValues *, void *, vgui::Panel * );  // line 106
};

// game/client/PanelMetaClassMgr.h:96 sizeof=0x4 (i386)
struct CPanelFactory<CVGUI_MP_LevelSelectScreen,VGuiScreenInitData_t> : public IPanelFactory
{
public:
	void CPanelFactory( const char * );  // line 98
	virtual vgui::Panel *Create( const char *, KeyValues *, void *, vgui::Panel * );  // line 106
};

// game/client/PanelMetaClassMgr.h:96 sizeof=0x4 (i386)
struct CPanelFactory<CVGUI_MP_LevelSelectScreen2,VGuiScreenInitData_t> : public IPanelFactory
{
public:
	void CPanelFactory( const char * );  // line 98
	virtual vgui::Panel *Create( const char *, KeyValues *, void *, vgui::Panel * );  // line 106
};

// game/client/PanelMetaClassMgr.h:96 sizeof=0x4 (i386)
struct CPanelFactory<CVGUI_MP_LevelSelectScreen3,VGuiScreenInitData_t> : public IPanelFactory
{
public:
	void CPanelFactory( const char * );  // line 98
	virtual vgui::Panel *Create( const char *, KeyValues *, void *, vgui::Panel * );  // line 106
};

// game/client/PanelMetaClassMgr.h:96 sizeof=0x4 (i386)
struct CPanelFactory<CVGUI_MP_LevelSelectScreen4,VGuiScreenInitData_t> : public IPanelFactory
{
public:
	void CPanelFactory( const char * );  // line 98
	virtual vgui::Panel *Create( const char *, KeyValues *, void *, vgui::Panel * );  // line 106
};

// game/client/PanelMetaClassMgr.h:106 @0x61d190 _ZN13CPanelFactoryI16CIndicatorScreen20VGuiScreenInitData_tE6CreateEPKcP9KeyValuesPvPN4vgui5PanelE
vgui::Panel *CPanelFactory<CIndicatorScreen,VGuiScreenInitData_t>::Create( const char *pMetaClassName, KeyValues *pKeyValues, void *pVoidInitData, vgui::Panel *pParent )
{
	CIndicatorScreen *pPanel;  // line 110
	{
		VGuiScreenInitData_t *pInitData;  // line 114
	}
	// inlined CIndicatorScreen::CIndicatorScreen() at line 110
}

// game/client/PanelMetaClassMgr.h:106 @0x61d7c0 _ZN13CPanelFactoryI19CLevelPlacardScreen20VGuiScreenInitData_tE6CreateEPKcP9KeyValuesPvPN4vgui5PanelE
vgui::Panel *CPanelFactory<CLevelPlacardScreen,VGuiScreenInitData_t>::Create( const char *pMetaClassName, KeyValues *pKeyValues, void *pVoidInitData, vgui::Panel *pParent )
{
	CLevelPlacardScreen *pPanel;  // line 110
	// inlined CLevelPlacardScreen::CLevelPlacardScreen() at line 110
	{
		VGuiScreenInitData_t *pInitData;  // line 114
	}
}

// game/client/PanelMetaClassMgr.h:106 @0x61f2d0 _ZN13CPanelFactoryI26CVGUI_MP_LevelSelectScreen20VGuiScreenInitData_tE6CreateEPKcP9KeyValuesPvPN4vgui5PanelE
vgui::Panel *CPanelFactory<CVGUI_MP_LevelSelectScreen,VGuiScreenInitData_t>::Create( const char *pMetaClassName, KeyValues *pKeyValues, void *pVoidInitData, vgui::Panel *pParent )
{
	CVGUI_MP_LevelSelectScreen *pPanel;  // line 110
	// inlined CVGUI_MP_LevelSelectScreen::CVGUI_MP_LevelSelectScreen() at line 110
	{
		VGuiScreenInitData_t *pInitData;  // line 114
	}
}

// game/client/PanelMetaClassMgr.h:106 @0x61f360 _ZN13CPanelFactoryI27CVGUI_MP_LevelSelectScreen220VGuiScreenInitData_tE6CreateEPKcP9KeyValuesPvPN4vgui5PanelE
vgui::Panel *CPanelFactory<CVGUI_MP_LevelSelectScreen2,VGuiScreenInitData_t>::Create( const char *pMetaClassName, KeyValues *pKeyValues, void *pVoidInitData, vgui::Panel *pParent )
{
	CVGUI_MP_LevelSelectScreen2 *pPanel;  // line 110
	{
		VGuiScreenInitData_t *pInitData;  // line 114
	}
	// inlined CVGUI_MP_LevelSelectScreen2::CVGUI_MP_LevelSelectScreen2() at line 110
}

// game/client/PanelMetaClassMgr.h:106 @0x61f400 _ZN13CPanelFactoryI27CVGUI_MP_LevelSelectScreen320VGuiScreenInitData_tE6CreateEPKcP9KeyValuesPvPN4vgui5PanelE
vgui::Panel *CPanelFactory<CVGUI_MP_LevelSelectScreen3,VGuiScreenInitData_t>::Create( const char *pMetaClassName, KeyValues *pKeyValues, void *pVoidInitData, vgui::Panel *pParent )
{
	CVGUI_MP_LevelSelectScreen3 *pPanel;  // line 110
	{
		VGuiScreenInitData_t *pInitData;  // line 114
	}
	// inlined CVGUI_MP_LevelSelectScreen3::CVGUI_MP_LevelSelectScreen3() at line 110
}

// game/client/PanelMetaClassMgr.h:106 @0x61f4a0 _ZN13CPanelFactoryI27CVGUI_MP_LevelSelectScreen420VGuiScreenInitData_tE6CreateEPKcP9KeyValuesPvPN4vgui5PanelE
vgui::Panel *CPanelFactory<CVGUI_MP_LevelSelectScreen4,VGuiScreenInitData_t>::Create( const char *pMetaClassName, KeyValues *pKeyValues, void *pVoidInitData, vgui::Panel *pParent )
{
	CVGUI_MP_LevelSelectScreen4 *pPanel;  // line 110
	{
		VGuiScreenInitData_t *pInitData;  // line 114
	}
	// inlined CVGUI_MP_LevelSelectScreen4::CVGUI_MP_LevelSelectScreen4() at line 110
}

// game/client/PanelMetaClassMgr.h:106 @0x61f5f0 _ZN13CPanelFactoryI19CObjectControlPanel20VGuiScreenInitData_tE6CreateEPKcP9KeyValuesPvPN4vgui5PanelE
vgui::Panel *CPanelFactory<CObjectControlPanel,VGuiScreenInitData_t>::Create( const char *pMetaClassName, KeyValues *pKeyValues, void *pVoidInitData, vgui::Panel *pParent )
{
	CObjectControlPanel *pPanel;  // line 110
	// inlined CObjectControlPanel::CObjectControlPanel() at line 110
	{
		VGuiScreenInitData_t *pInitData;  // line 114
	}
}

// game/client/PanelMetaClassMgr.h:106 @0x61f680 _ZN13CPanelFactoryI20CVGUI_MP_LobbyScreen20VGuiScreenInitData_tE6CreateEPKcP9KeyValuesPvPN4vgui5PanelE
vgui::Panel *CPanelFactory<CVGUI_MP_LobbyScreen,VGuiScreenInitData_t>::Create( const char *pMetaClassName, KeyValues *pKeyValues, void *pVoidInitData, vgui::Panel *pParent )
{
	CVGUI_MP_LobbyScreen *pPanel;  // line 110
	{
		VGuiScreenInitData_t *pInitData;  // line 114
	}
	// inlined CVGUI_MP_LobbyScreen::CVGUI_MP_LobbyScreen() at line 110
}

// game/client/PanelMetaClassMgr.h:142 sizeof=0x4 (i386)
struct CPanelWrapper
{
public:
	CPanelWrapper();  // line 144
	~CPanelWrapper();  // line 145
	void Activate( const char *, vgui::Panel *, int, void * );  // line 146
	void Deactivate();  // line 147
	vgui::Panel *GetPanel();  // line 148
private:
	vgui::Panel *m_pPanel; // +0x0  // line 151
};
