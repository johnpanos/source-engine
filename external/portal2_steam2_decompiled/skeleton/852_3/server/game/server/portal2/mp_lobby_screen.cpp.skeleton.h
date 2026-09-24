// DWARF declaration skeleton for game/server/portal2/mp_lobby_screen.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xc1190 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSteamID::CSteamID() at line 650
	// inlined CSteamID::CSteamID() at line 654
	// inlined CSteamID::CSteamID() at line 656
	// inlined CSteamID::CSteamID() at line 659
	// inlined CSteamID::CSteamID() at line 662
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
	// inlined CEntityFactory<CMP_LobbyScreen>::CEntityFactory() at line 56
	// inlined ServerClass::ServerClass() at line 69
}

// game/server/portal2/mp_lobby_screen.cpp:14 sizeof=0x3d8 (i386)
struct CMP_LobbyScreen : public CBaseEntity
{
public:
	virtual ~CMP_LobbyScreen();  // line 21
	virtual bool KeyValue( const char *, const char * );  // line 23
	virtual int UpdateTransmitState();  // line 25
	virtual void SetTransmit( CCheckTransmitInfo *, bool );  // line 26
	virtual void Spawn();  // line 28
	virtual void Precache();  // line 29
	virtual void OnRestore();  // line 30
	void ScreenVisible( bool );  // line 32
	void Disable();  // line 34
	void Enable();  // line 35
	void InputDisable( inputdata_t & );  // line 37
	void InputEnable( inputdata_t & );  // line 38
private:
	void GetControlPanelInfo( int, const char *& );  // line 43
	void GetControlPanelClassName( int, const char *& );  // line 44
	void SpawnControlPanels();  // line 45
	void RestoreControlPanels();  // line 46
	CNetworkVar( bool, m_bEnabled ); // +0x3c0  // line 50
	CUtlVector<CHandle<CVGuiScreen>,CUtlMemory<CHandle<CVGuiScreen>, int> > m_hScreens; // +0x3c4  // line 53
};

// game/server/portal2/mp_lobby_screen.cpp:14 (declaration)
void CMP_LobbyScreen();

// game/server/portal2/mp_lobby_screen.cpp:18 @0x6bd940 _ZN15CMP_LobbyScreen14GetDataDescMapEv
datamap_t *CMP_LobbyScreen::GetDataDescMap()
{
}

// game/server/portal2/mp_lobby_screen.cpp:18 @0x6bd950 _ZN15CMP_LobbyScreen10GetBaseMapEv
datamap_t *CMP_LobbyScreen::GetBaseMap()
{
}

// game/server/portal2/mp_lobby_screen.cpp:19 @0x6bd960 _ZN15CMP_LobbyScreen14GetServerClassEv
ServerClass *CMP_LobbyScreen::GetServerClass()
{
}

// game/server/portal2/mp_lobby_screen.cpp:19 @0x6bd970 _ZN15CMP_LobbyScreen40YouForgotToImplementOrDeclareServerClassEv
int CMP_LobbyScreen::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/mp_lobby_screen.cpp:21 @0x6be300 _ZN15CMP_LobbyScreenD2Ev
CMP_LobbyScreen::~CMP_LobbyScreen()
{
	{
		int i;  // line 76
		// inlined CHandle<CVGuiScreen>::Get() at line 80
		// inlined CUtlVector<CHandle<CVGuiScreen>,CUtlMemory<CHandle<CVGuiScreen>, int> >::Count() at line 78
	}
	// inlined CUtlVector<CHandle<CVGuiScreen>,CUtlMemory<CHandle<CVGuiScreen>, int> >::~CUtlVector() at line 83
	// inlined CUtlVector<CHandle<CVGuiScreen>,CUtlMemory<CHandle<CVGuiScreen>, int> >::~CUtlVector() at line 83
}

// game/server/portal2/mp_lobby_screen.cpp:21 @0x6be510 _ZN15CMP_LobbyScreenD1Ev
CMP_LobbyScreen::~CMP_LobbyScreen()
{
}

// game/server/portal2/mp_lobby_screen.cpp:21 @0x6be520 _ZN15CMP_LobbyScreenD0Ev
CMP_LobbyScreen::~CMP_LobbyScreen()
{
	{
		int i;  // line 76
		// inlined CHandle<CVGuiScreen>::Get() at line 80
		// inlined CUtlVector<CHandle<CVGuiScreen>,CUtlMemory<CHandle<CVGuiScreen>, int> >::Count() at line 78
	}
	// inlined CUtlVector<CHandle<CVGuiScreen>,CUtlMemory<CHandle<CVGuiScreen>, int> >::~CUtlVector() at line 83
	// inlined CUtlVector<CHandle<CVGuiScreen>,CUtlMemory<CHandle<CVGuiScreen>, int> >::~CUtlVector() at line 83
}

// game/server/portal2/mp_lobby_screen.cpp:23 @0x6bda00 _ZN15CMP_LobbyScreen8KeyValueEPKcS1_
bool CMP_LobbyScreen::KeyValue( const char *szKeyName, const char *szValue )
{
	char *s;  // line 92
	{
		QAngle angles;  // line 102
		VMatrix mat;  // line 107
		VMatrix rotation;  // line 107
		VMatrix tmp;  // line 107
		// inlined QAngle::Base() at line 103
		// inlined Vector::Vector() at line 109
		// inlined Vector::operator VectorByValue&() at line 109
	}
	// inlined FStrEq() at line 99
}

// game/server/portal2/mp_lobby_screen.cpp:25 @0x6bd9e0 _ZN15CMP_LobbyScreen19UpdateTransmitStateEv
int CMP_LobbyScreen::UpdateTransmitState()
{
}

// game/server/portal2/mp_lobby_screen.cpp:26 @0x6bdb50 _ZN15CMP_LobbyScreen11SetTransmitEP18CCheckTransmitInfob
void CMP_LobbyScreen::SetTransmit( CCheckTransmitInfo *pInfo, bool bAlways )
{
	{
		int i;  // line 136
		{
			CVGuiScreen *pScreen;  // line 138
			// inlined CHandle<CVGuiScreen>::Get() at line 138
		}
	}
	// inlined CBaseEntity::entindex() at line 130
}

// game/server/portal2/mp_lobby_screen.cpp:28 @0x6be110 _ZN15CMP_LobbyScreen5SpawnEv
void CMP_LobbyScreen::Spawn()
{
	// inlined CNetworkVarBase<bool,CMP_LobbyScreen::NetworkVar_m_bEnabled>::operator=<bool>() at line 149
}

// game/server/portal2/mp_lobby_screen.cpp:29 @0x6bd9c0 _ZN15CMP_LobbyScreen8PrecacheEv
void CMP_LobbyScreen::Precache()
{
}

// game/server/portal2/mp_lobby_screen.cpp:30 @0x6be0d0 _ZN15CMP_LobbyScreen9OnRestoreEv
void CMP_LobbyScreen::OnRestore()
{
}

// game/server/portal2/mp_lobby_screen.cpp:32 @0x6bdf20 _ZN15CMP_LobbyScreen13ScreenVisibleEb
void CMP_LobbyScreen::ScreenVisible( bool bVisible )
{
	{
		int iScreen;  // line 174
		{
			CVGuiScreen *pScreen;  // line 176
			// inlined CHandle<CVGuiScreen>::Get() at line 176
			// inlined CBaseEntity::RemoveEffects() at line 178
		}
	}
}

// game/server/portal2/mp_lobby_screen.cpp:34 @0x6be060 _ZN15CMP_LobbyScreen7DisableEv
void CMP_LobbyScreen::Disable()
{
	// inlined CNetworkVarBase<bool,CMP_LobbyScreen::NetworkVar_m_bEnabled>::operator=<bool>() at line 189
}

// game/server/portal2/mp_lobby_screen.cpp:35 @0x6be210 _ZN15CMP_LobbyScreen6EnableEv
void CMP_LobbyScreen::Enable()
{
	// inlined CNetworkVarBase<bool,CMP_LobbyScreen::NetworkVar_m_bEnabled>::operator=<bool>() at line 199
}

// game/server/portal2/mp_lobby_screen.cpp:37 @0x6be280 _ZN15CMP_LobbyScreen12InputDisableER11inputdata_t
void CMP_LobbyScreen::InputDisable( inputdata_t &inputdata )
{
	// inlined CMP_LobbyScreen::Disable() at line 207
}

// game/server/portal2/mp_lobby_screen.cpp:38 @0x6be1a0 _ZN15CMP_LobbyScreen11InputEnableER11inputdata_t
void CMP_LobbyScreen::InputEnable( inputdata_t &inputdata )
{
	// inlined CMP_LobbyScreen::Enable() at line 212
}

// game/server/portal2/mp_lobby_screen.cpp:43 @0x6bd980 _ZN15CMP_LobbyScreen19GetControlPanelInfoEiRPKc
void CMP_LobbyScreen::GetControlPanelInfo( int nPanelIndex, const char *&pPanelName )
{
}

// game/server/portal2/mp_lobby_screen.cpp:44 @0x6bd9a0 _ZN15CMP_LobbyScreen24GetControlPanelClassNameEiRPKc
void CMP_LobbyScreen::GetControlPanelClassName( int nPanelIndex, const char *&pPanelName )
{
}

// game/server/portal2/mp_lobby_screen.cpp:45 @0x6bdc40 _ZN15CMP_LobbyScreen18SpawnControlPanelsEv
void CMP_LobbyScreen::SpawnControlPanels()
{
	int nPanel;  // line 230
	{
		const char *pScreenName;  // line 233
		const char *pScreenClassname;  // line 238
		float flWidth;  // line 244
		float flHeight;  // line 245
		CVGuiScreen *pScreen;  // line 251
		int nScreen;  // line 257
		// inlined CHandle<CVGuiScreen>::Set() at line 258
		// inlined CUtlVector<CHandle<CVGuiScreen>,CUtlMemory<CHandle<CVGuiScreen>, int> >::operator[]() at line 258
		// inlined CUtlVector<CHandle<CVGuiScreen>,CUtlMemory<CHandle<CVGuiScreen>, int> >::AddToTail() at line 257
	}
}

// game/server/portal2/mp_lobby_screen.cpp:46 @0x6bdd90 _ZN15CMP_LobbyScreen20RestoreControlPanelsEv
void CMP_LobbyScreen::RestoreControlPanels()
{
	int nPanel;  // line 266
	{
		const char *pScreenName;  // line 269
		const char *pScreenClassname;  // line 274
		CVGuiScreen *pScreen;  // line 279
		{
			int nScreen;  // line 288
			// inlined CHandle<CVGuiScreen>::Set() at line 289
			// inlined CUtlVector<CHandle<CVGuiScreen>,CUtlMemory<CHandle<CVGuiScreen>, int> >::operator[]() at line 289
			// inlined CUtlVector<CHandle<CVGuiScreen>,CUtlMemory<CHandle<CVGuiScreen>, int> >::AddToTail() at line 288
		}
		// inlined CBaseEntity::GetOwnerEntity() at line 281
	}
}

// game/server/portal2/mp_lobby_screen.cpp:56
static CEntityFactory<CMP_LobbyScreen> vgui_mp_lobby_display;

// game/server/portal2/mp_lobby_screen.cpp:61 @0xc10c0 _Z11DataMapInitI15CMP_LobbyScreenEP9datamap_tPT_
datamap_t *DataMapInit<CMP_LobbyScreen>( CMP_LobbyScreen * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 61
	CDatadescGeneratedNameHolder nameHolder;  // line 61
	typedescription_t dataDesc[4];  // line 61
}

// game/server/portal2/mp_lobby_screen.cpp:61
datamap_t *g_DataMapHolder;

// game/server/portal2/mp_lobby_screen.cpp:69 @0xc0f00 _Z15ServerClassInitIN17DT_MP_LobbyScreen7ignoredEEiPT_
int ServerClassInit<DT_MP_LobbyScreen::ignored>( DT_MP_LobbyScreen::ignored * )
{
	SendTable &sendTable;  // line 69
	char *const g_pSendTableName;  // line 69
	SendProp g_SendProps[3];  // line 69
}

// game/server/portal2/mp_lobby_screen.cpp:69
SendTable g_SendTable;

// game/server/portal2/mp_lobby_screen.cpp:69
int g_SendTableInit;

// game/server/portal2/mp_lobby_screen.cpp:69
static ServerClass g_CMP_LobbyScreen_ClassReg;

// game/server/util.h:99 @0x6be7d0 _ZN14CEntityFactoryI15CMP_LobbyScreenE6CreateEPKc
IServerNetworkable *CEntityFactory<CMP_LobbyScreen>::Create( const char *pClassName )
{
	CMP_LobbyScreen *pEnt;  // line 101
	// inlined _CreateEntityTemplate<CMP_LobbyScreen>() at line 101
}

// game/server/util.h:105 @0x6be790 _ZN14CEntityFactoryI15CMP_LobbyScreenE7DestroyEP18IServerNetworkable
void CEntityFactory<CMP_LobbyScreen>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:113 @0x6be7c0 _ZN14CEntityFactoryI15CMP_LobbyScreenE13GetEntitySizeEv
size_t CEntityFactory<CMP_LobbyScreen>::GetEntitySize()
{
}
