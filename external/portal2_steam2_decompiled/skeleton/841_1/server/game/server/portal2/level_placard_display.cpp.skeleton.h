// DWARF declaration skeleton for game/server/portal2/level_placard_display.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xabaa0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CEntityFactory<CLevelPlacardDisplay>::CEntityFactory() at line 63
	// inlined ServerClass::ServerClass() at line 83
}

// game/server/portal2/level_placard_display.cpp:17 sizeof=0x448 (i386)
struct CLevelPlacardDisplay : public CBaseEntity
{
public:
	virtual ~CLevelPlacardDisplay();  // line 24
	virtual bool KeyValue( const char *, const char * );  // line 26
	virtual int UpdateTransmitState();  // line 28
	virtual void SetTransmit( CCheckTransmitInfo *, bool );  // line 29
	virtual void Spawn();  // line 31
	virtual void Precache();  // line 32
	virtual void OnRestore();  // line 33
	void ScreenVisible( bool );  // line 35
	void Disable();  // line 37
	void Enable();  // line 38
	void InputDisable( inputdata_t & );  // line 40
	void InputEnable( inputdata_t & );  // line 41
	void InputSetDisplayText( inputdata_t & );  // line 43
private:
	void GetControlPanelInfo( int, const char *& );  // line 48
	void GetControlPanelClassName( int, const char *& );  // line 49
	void SpawnControlPanels();  // line 50
	void RestoreControlPanels();  // line 51
	CNetworkVar( bool, m_bEnabled ); // +0x3c0  // line 54
	CLevelPlacardDisplay::NetworkVar_m_szDisplayText m_szDisplayText; // +0x3c1  // line 56
	bool m_bDoFullTransmit; // +0x441  // line 58
	CHandle<CVGuiScreen> m_hScreen; // +0x444  // line 60
};

// game/server/portal2/level_placard_display.cpp:17 (declaration)
void CLevelPlacardDisplay();

// game/server/portal2/level_placard_display.cpp:21 @0x71d770 _ZN20CLevelPlacardDisplay14GetDataDescMapEv
datamap_t *CLevelPlacardDisplay::GetDataDescMap()
{
}

// game/server/portal2/level_placard_display.cpp:21 @0x71d780 _ZN20CLevelPlacardDisplay10GetBaseMapEv
datamap_t *CLevelPlacardDisplay::GetBaseMap()
{
}

// game/server/portal2/level_placard_display.cpp:22 @0x71d790 _ZN20CLevelPlacardDisplay14GetServerClassEv
ServerClass *CLevelPlacardDisplay::GetServerClass()
{
}

// game/server/portal2/level_placard_display.cpp:22 @0x71d7a0 _ZN20CLevelPlacardDisplay40YouForgotToImplementOrDeclareServerClassEv
int CLevelPlacardDisplay::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/level_placard_display.cpp:24 @0x71daa0 _ZN20CLevelPlacardDisplayD2Ev
CLevelPlacardDisplay::~CLevelPlacardDisplay()
{
	// inlined CHandle<CVGuiScreen>::Get() at line 89
}

// game/server/portal2/level_placard_display.cpp:24 @0x71dc70 _ZN20CLevelPlacardDisplayD0Ev
CLevelPlacardDisplay::~CLevelPlacardDisplay()
{
	// inlined CHandle<CVGuiScreen>::Get() at line 89
}

// game/server/portal2/level_placard_display.cpp:24 @0x71dd00 _ZN20CLevelPlacardDisplayD1Ev
CLevelPlacardDisplay::~CLevelPlacardDisplay()
{
	// inlined CHandle<CVGuiScreen>::Get() at line 89
}

// game/server/portal2/level_placard_display.cpp:26 @0x71db30 _ZN20CLevelPlacardDisplay8KeyValueEPKcS1_
bool CLevelPlacardDisplay::KeyValue( const char *szKeyName, const char *szValue )
{
	{
		QAngle angles;  // line 101
		VMatrix mat;  // line 106
		VMatrix rotation;  // line 106
		VMatrix tmp;  // line 106
		// inlined QAngle::Base() at line 102
		// inlined Vector::Vector() at line 108
		// inlined Vector::operator VectorByValue&() at line 108
	}
	// inlined FStrEq() at line 98
}

// game/server/portal2/level_placard_display.cpp:28 @0x71da60 _ZN20CLevelPlacardDisplay19UpdateTransmitStateEv
int CLevelPlacardDisplay::UpdateTransmitState()
{
}

// game/server/portal2/level_placard_display.cpp:29 @0x71d970 _ZN20CLevelPlacardDisplay11SetTransmitEP18CCheckTransmitInfob
void CLevelPlacardDisplay::SetTransmit( CCheckTransmitInfo *pInfo, bool bAlways )
{
	// inlined CBaseEntity::entindex() at line 141
	// inlined CHandle<CVGuiScreen>::operator->() at line 147
}

// game/server/portal2/level_placard_display.cpp:31 @0x71e180 _ZN20CLevelPlacardDisplay5SpawnEv
void CLevelPlacardDisplay::Spawn()
{
	// inlined CNetworkVarBase<bool,CLevelPlacardDisplay::NetworkVar_m_bEnabled>::operator=<bool>() at line 159
}

// game/server/portal2/level_placard_display.cpp:32 @0x71d950 _ZN20CLevelPlacardDisplay8PrecacheEv
void CLevelPlacardDisplay::Precache()
{
}

// game/server/portal2/level_placard_display.cpp:33 @0x71e140 _ZN20CLevelPlacardDisplay9OnRestoreEv
void CLevelPlacardDisplay::OnRestore()
{
}

// game/server/portal2/level_placard_display.cpp:35 @0x71dea0 _ZN20CLevelPlacardDisplay13ScreenVisibleEb
void CLevelPlacardDisplay::ScreenVisible( bool bVisible )
{
	// inlined CHandle<CVGuiScreen>::operator->() at line 196
	// inlined CHandle<CVGuiScreen>::operator->() at line 200
	// inlined CBaseEntity::RemoveEffects() at line 200
	// inlined CHandle<CVGuiScreen>::operator->() at line 204
}

// game/server/portal2/level_placard_display.cpp:37 @0x71e380 _ZN20CLevelPlacardDisplay7DisableEv
void CLevelPlacardDisplay::Disable()
{
	// inlined CNetworkVarBase<bool,CLevelPlacardDisplay::NetworkVar_m_bEnabled>::operator=<bool>() at line 216
}

// game/server/portal2/level_placard_display.cpp:38 @0x71e720 _ZN20CLevelPlacardDisplay6EnableEv
void CLevelPlacardDisplay::Enable()
{
	// inlined CNetworkVarBase<bool,CLevelPlacardDisplay::NetworkVar_m_bEnabled>::operator=<bool>() at line 229
}

// game/server/portal2/level_placard_display.cpp:40 @0x71e540 _ZN20CLevelPlacardDisplay12InputDisableER11inputdata_t
void CLevelPlacardDisplay::InputDisable( inputdata_t &inputdata )
{
	// inlined CLevelPlacardDisplay::Disable() at line 239
}

// game/server/portal2/level_placard_display.cpp:41 @0x71e8e0 _ZN20CLevelPlacardDisplay11InputEnableER11inputdata_t
void CLevelPlacardDisplay::InputEnable( inputdata_t &inputdata )
{
	// inlined CLevelPlacardDisplay::Enable() at line 247
}

// game/server/portal2/level_placard_display.cpp:43 @0x71d8c0 _ZN20CLevelPlacardDisplay19InputSetDisplayTextER11inputdata_t
void CLevelPlacardDisplay::InputSetDisplayText( inputdata_t &inputdata )
{
	// inlined variant_t::String() at line 255
	// inlined CLevelPlacardDisplay::NetworkVar_m_szDisplayText::GetForModify() at line 255
}

// game/server/portal2/level_placard_display.cpp:48 @0x71d7b0 _ZN20CLevelPlacardDisplay19GetControlPanelInfoEiRPKc
void CLevelPlacardDisplay::GetControlPanelInfo( int nPanelIndex, const char *&pPanelName )
{
}

// game/server/portal2/level_placard_display.cpp:49 @0x71d7d0 _ZN20CLevelPlacardDisplay24GetControlPanelClassNameEiRPKc
void CLevelPlacardDisplay::GetControlPanelClassName( int nPanelIndex, const char *&pPanelName )
{
}

// game/server/portal2/level_placard_display.cpp:50 @0x71d7f0 _ZN20CLevelPlacardDisplay18SpawnControlPanelsEv
void CLevelPlacardDisplay::SpawnControlPanels()
{
	int nPanel;  // line 279
	{
		const char *pScreenName;  // line 282
		const char *pScreenClassname;  // line 287
		float flWidth;  // line 292
		float flHeight;  // line 293
		CVGuiScreen *pScreen;  // line 295
		// inlined CHandle<CVGuiScreen>::operator=() at line 301
	}
}

// game/server/portal2/level_placard_display.cpp:51 @0x71dd90 _ZN20CLevelPlacardDisplay20RestoreControlPanelsEv
void CLevelPlacardDisplay::RestoreControlPanels()
{
	int nPanel;  // line 312
	{
		const char *pScreenName;  // line 315
		const char *pScreenClassname;  // line 320
		CVGuiScreen *pScreen;  // line 325
		// inlined CHandle<CVGuiScreen>::operator->() at line 335
		// inlined CHandle<CVGuiScreen>::operator=() at line 334
		// inlined CBaseEntity::GetOwnerEntity() at line 327
	}
}

// game/server/portal2/level_placard_display.cpp:63
static CEntityFactory<CLevelPlacardDisplay> vgui_level_placard_display;

// game/server/portal2/level_placard_display.cpp:68 @0xab9d0 _Z11DataMapInitI20CLevelPlacardDisplayEP9datamap_tPT_
datamap_t *DataMapInit<CLevelPlacardDisplay>( CLevelPlacardDisplay * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 68
	CDatadescGeneratedNameHolder nameHolder;  // line 68
	typedescription_t dataDesc[8];  // line 68
}

// game/server/portal2/level_placard_display.cpp:68
datamap_t *g_DataMapHolder;

// game/server/portal2/level_placard_display.cpp:83 @0xab810 _Z15ServerClassInitIN22DT_LevelPlacardDisplay7ignoredEEiPT_
int ServerClassInit<DT_LevelPlacardDisplay::ignored>( DT_LevelPlacardDisplay::ignored * )
{
	SendTable &sendTable;  // line 83
	char *const g_pSendTableName;  // line 83
	SendProp g_SendProps[3];  // line 83
}

// game/server/portal2/level_placard_display.cpp:83
SendTable g_SendTable;

// game/server/portal2/level_placard_display.cpp:83
int g_SendTableInit;

// game/server/portal2/level_placard_display.cpp:83
static ServerClass g_CLevelPlacardDisplay_ClassReg;

// game/server/util.h:100 @0x71eb00 _ZN14CEntityFactoryI20CLevelPlacardDisplayE6CreateEPKc
IServerNetworkable *CEntityFactory<CLevelPlacardDisplay>::Create( const char *pClassName )
{
	CLevelPlacardDisplay *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CLevelPlacardDisplay>() at line 102
}

// game/server/util.h:106 @0x71eac0 _ZN14CEntityFactoryI20CLevelPlacardDisplayE7DestroyEP18IServerNetworkable
void CEntityFactory<CLevelPlacardDisplay>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:114 @0x71eaf0 _ZN14CEntityFactoryI20CLevelPlacardDisplayE13GetEntitySizeEv
size_t CEntityFactory<CLevelPlacardDisplay>::GetEntitySize()
{
}
