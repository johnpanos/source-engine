// DWARF declaration skeleton for game/server/portal2/indicator_panel.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xaa100 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined ServerClass::ServerClass() at line 35
	// inlined CEntityFactory<CLabIndicatorPanel>::CEntityFactory() at line 46
	// inlined CEntityFactory<CPropIndicatorPanel>::CEntityFactory() at line 269
	// inlined CEntityFactory<CPropTicTacToePanel>::CEntityFactory() at line 448
}

// game/server/portal2/indicator_panel.cpp:18 (declaration)
void GetBaseMap();

// game/server/portal2/indicator_panel.cpp:18 @0x712d30 _ZN18CLabIndicatorPanel14GetDataDescMapEv
datamap_t *CLabIndicatorPanel::GetDataDescMap()
{
}

// game/server/portal2/indicator_panel.cpp:18 @0x712d40 _ZN18CLabIndicatorPanel10GetBaseMapEv
datamap_t *CLabIndicatorPanel::GetBaseMap()
{
}

// game/server/portal2/indicator_panel.cpp:18 @0xa9950 _Z11DataMapInitI18CLabIndicatorPanelEP9datamap_tPT_
datamap_t *DataMapInit<CLabIndicatorPanel>( CLabIndicatorPanel * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 18
	CDatadescGeneratedNameHolder nameHolder;  // line 18
	typedescription_t dataDesc[12];  // line 18
}

// game/server/portal2/indicator_panel.cpp:18
datamap_t *g_DataMapHolder;

// game/server/portal2/indicator_panel.cpp:18
void CLabIndicatorPanel::m_DataMap;

// game/server/portal2/indicator_panel.cpp:35 @0x712d50 _ZN18CLabIndicatorPanel14GetServerClassEv
ServerClass *CLabIndicatorPanel::GetServerClass()
{
}

// game/server/portal2/indicator_panel.cpp:35 @0x712d60 _ZN18CLabIndicatorPanel40YouForgotToImplementOrDeclareServerClassEv
int CLabIndicatorPanel::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/indicator_panel.cpp:35 @0xa9a20 _Z15ServerClassInitIN20DT_LabIndicatorPanel7ignoredEEiPT_
int ServerClassInit<DT_LabIndicatorPanel::ignored>( DT_LabIndicatorPanel::ignored * )
{
	SendTable &sendTable;  // line 35
	char *const g_pSendTableName;  // line 35
	SendProp g_SendProps[10];  // line 35
}

// game/server/portal2/indicator_panel.cpp:35
SendTable g_SendTable;

// game/server/portal2/indicator_panel.cpp:35
int g_SendTableInit;

// game/server/portal2/indicator_panel.cpp:35
static ServerClass g_CLabIndicatorPanel_ClassReg;

// game/server/portal2/indicator_panel.cpp:35
void CLabIndicatorPanel::m_pClassSendTable;

// game/server/portal2/indicator_panel.cpp:46
static CEntityFactory<CLabIndicatorPanel> vgui_indicator_panel;

// game/server/portal2/indicator_panel.cpp:51 (declaration)
~CLabIndicatorPanel();

// game/server/portal2/indicator_panel.cpp:51 @0x712e90 _ZN18CLabIndicatorPanelD0Ev
CLabIndicatorPanel::~CLabIndicatorPanel()
{
	// inlined CHandle<CVGuiScreen>::operator!=() at line 53
}

// game/server/portal2/indicator_panel.cpp:51 @0x712f30 _ZN18CLabIndicatorPanelD2Ev
CLabIndicatorPanel::~CLabIndicatorPanel()
{
	// inlined CHandle<CVGuiScreen>::operator!=() at line 53
}

// game/server/portal2/indicator_panel.cpp:51 @0x712fc0 _ZN18CLabIndicatorPanelD1Ev
CLabIndicatorPanel::~CLabIndicatorPanel()
{
}

// game/server/portal2/indicator_panel.cpp:59 @0x712e70 _ZN18CLabIndicatorPanel19UpdateTransmitStateEv
int CLabIndicatorPanel::UpdateTransmitState()
{
}

// game/server/portal2/indicator_panel.cpp:64 @0x713040 _ZN18CLabIndicatorPanel11SetTransmitEP18CCheckTransmitInfob
void CLabIndicatorPanel::SetTransmit( CCheckTransmitInfo *pInfo, bool bAlways )
{
	// inlined CBaseEntity::entindex() at line 67
	// inlined CHandle<CVGuiScreen>::operator!=() at line 73
}

// game/server/portal2/indicator_panel.cpp:79 @0x7144e0 _ZN18CLabIndicatorPanel5SpawnEv
void CLabIndicatorPanel::Spawn()
{
	// inlined CNetworkVarBase<bool,CLabIndicatorPanel::NetworkVar_m_bEnabled>::operator=<bool>() at line 88
	// inlined CNetworkVarBase<bool,CLabIndicatorPanel::NetworkVar_m_bCountingDown>::operator=<bool>() at line 92
	// inlined CNetworkVarBase<bool,CLabIndicatorPanel::NetworkVar_m_bStopped>::operator=<bool>() at line 93
	// inlined CNetworkVarBase<float,CLabIndicatorPanel::NetworkVar_m_flTimerStart>::operator=<float>() at line 94
}

// game/server/portal2/indicator_panel.cpp:97 @0x712e50 _ZN18CLabIndicatorPanel8PrecacheEv
void CLabIndicatorPanel::Precache()
{
}

// game/server/util.h:100 @0x717410 _ZN14CEntityFactoryI19CPropIndicatorPanelE6CreateEPKc
IServerNetworkable *CEntityFactory<CPropIndicatorPanel>::Create( const char *pClassName )
{
	CPropIndicatorPanel *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CPropIndicatorPanel>() at line 102
}

// game/server/util.h:100 @0x717480 _ZN14CEntityFactoryI19CPropTicTacToePanelE6CreateEPKc
IServerNetworkable *CEntityFactory<CPropTicTacToePanel>::Create( const char *pClassName )
{
	CPropTicTacToePanel *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CPropTicTacToePanel>() at line 102
}

// game/server/util.h:100 @0x717550 _ZN14CEntityFactoryI18CLabIndicatorPanelE6CreateEPKc
IServerNetworkable *CEntityFactory<CLabIndicatorPanel>::Create( const char *pClassName )
{
	CLabIndicatorPanel *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CLabIndicatorPanel>() at line 102
}

// game/server/portal2/indicator_panel.cpp:104 @0x7134a0 _ZN18CLabIndicatorPanel9OnRestoreEv
void CLabIndicatorPanel::OnRestore()
{
	// inlined CLabIndicatorPanel::RestoreControlPanels() at line 108
}

// game/server/util.h:106 @0x7172f0 _ZN14CEntityFactoryI19CPropTicTacToePanelE7DestroyEP18IServerNetworkable
void CEntityFactory<CPropTicTacToePanel>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:106 @0x717330 _ZN14CEntityFactoryI19CPropIndicatorPanelE7DestroyEP18IServerNetworkable
void CEntityFactory<CPropIndicatorPanel>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:106 @0x717370 _ZN14CEntityFactoryI18CLabIndicatorPanelE7DestroyEP18IServerNetworkable
void CEntityFactory<CLabIndicatorPanel>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/portal2/indicator_panel.cpp:112 @0x713280 _ZN18CLabIndicatorPanel13ScreenVisibleEb
void CLabIndicatorPanel::ScreenVisible( bool bVisible )
{
	// inlined CHandle<CVGuiScreen>::operator!=() at line 114
	// inlined CBaseEntity::RemoveEffects() at line 118
}

// game/server/util.h:114 @0x717320 _ZN14CEntityFactoryI19CPropTicTacToePanelE13GetEntitySizeEv
size_t CEntityFactory<CPropTicTacToePanel>::GetEntitySize()
{
}

// game/server/util.h:114 @0x717360 _ZN14CEntityFactoryI19CPropIndicatorPanelE13GetEntitySizeEv
size_t CEntityFactory<CPropIndicatorPanel>::GetEntitySize()
{
}

// game/server/util.h:114 @0x7173a0 _ZN14CEntityFactoryI18CLabIndicatorPanelE13GetEntitySizeEv
size_t CEntityFactory<CLabIndicatorPanel>::GetEntitySize()
{
}

// game/server/portal2/indicator_panel.cpp:127 (declaration)
void Disable();

// game/server/portal2/indicator_panel.cpp:127 @0x714140 _ZN18CLabIndicatorPanel7DisableEv
void CLabIndicatorPanel::Disable()
{
	// inlined CNetworkVarBase<bool,CLabIndicatorPanel::NetworkVar_m_bEnabled>::operator=<bool>() at line 132
}

// game/server/portal2/indicator_panel.cpp:137 (declaration)
void Enable();

// game/server/portal2/indicator_panel.cpp:137 @0x715010 _ZN18CLabIndicatorPanel6EnableEv
void CLabIndicatorPanel::Enable()
{
	// inlined CNetworkVarBase<bool,CLabIndicatorPanel::NetworkVar_m_bEnabled>::operator=<bool>() at line 142
}

// game/server/portal2/indicator_panel.cpp:148 @0x714300 _ZN18CLabIndicatorPanel12InputDisableER11inputdata_t
void CLabIndicatorPanel::InputDisable( inputdata_t &inputdata )
{
	// inlined CLabIndicatorPanel::Disable() at line 150
}

// game/server/portal2/indicator_panel.cpp:153 @0x7166d0 _ZN18CLabIndicatorPanel11InputEnableER11inputdata_t
void CLabIndicatorPanel::InputEnable( inputdata_t &inputdata )
{
	// inlined CLabIndicatorPanel::Enable() at line 155
}

// game/server/portal2/indicator_panel.cpp:158 (declaration)
void GetControlPanelInfo( int nPanelIndex, const char *&pPanelName );

// game/server/portal2/indicator_panel.cpp:158 @0x712d70 _ZN18CLabIndicatorPanel19GetControlPanelInfoEiRPKc
void CLabIndicatorPanel::GetControlPanelInfo( int nPanelIndex, const char *&pPanelName )
{
}

// game/server/portal2/indicator_panel.cpp:163 (declaration)
void GetControlPanelClassName( int nPanelIndex, const char *&pPanelName );

// game/server/portal2/indicator_panel.cpp:163 @0x712d90 _ZN18CLabIndicatorPanel24GetControlPanelClassNameEiRPKc
void CLabIndicatorPanel::GetControlPanelClassName( int nPanelIndex, const char *&pPanelName )
{
}

// game/server/portal2/indicator_panel.cpp:171 @0x713140 _ZN18CLabIndicatorPanel18SpawnControlPanelsEv
void CLabIndicatorPanel::SpawnControlPanels()
{
	const char *pScreenName;  // line 173
	const char *pScreenClassname;  // line 178
	CVGuiScreen *pScreen;  // line 183
	// inlined CHandle<CVGuiScreen>::operator=() at line 191
}

// game/server/portal2/indicator_panel.cpp:194 (declaration)
void RestoreControlPanels();

// game/server/portal2/indicator_panel.cpp:194 @0x713210 _ZN18CLabIndicatorPanel20RestoreControlPanelsEv
void CLabIndicatorPanel::RestoreControlPanels()
{
	// inlined CHandle<CVGuiScreen>::operator CVGuiScreen*() at line 196
}

// game/server/portal2/indicator_panel.cpp:206 @0x713790 _ZN18CLabIndicatorPanel10StartTimerEf
void CLabIndicatorPanel::StartTimer( float flDuration )
{
	// inlined CNetworkVarBase<float,CLabIndicatorPanel::NetworkVar_m_flTimerStart>::operator=<float>() at line 208
	// inlined CNetworkVarBase<bool,CLabIndicatorPanel::NetworkVar_m_bStopped>::operator=<bool>() at line 209
	// inlined CNetworkVarBase<bool,CLabIndicatorPanel::NetworkVar_m_bCountingDown>::operator=<bool>() at line 210
	// inlined CNetworkVarBase<bool,CLabIndicatorPanel::NetworkVar_m_bIsCountdownTimer>::operator=<bool>() at line 211
	// inlined CNetworkVarBase<float,CLabIndicatorPanel::NetworkVar_m_flTimerDuration>::operator=<float>() at line 212
}

// game/server/portal2/indicator_panel.cpp:215 (declaration)
void ResetTimer();

// game/server/portal2/indicator_panel.cpp:215 @0x7161d0 _ZN18CLabIndicatorPanel10ResetTimerEv
void CLabIndicatorPanel::ResetTimer()
{
	// inlined CNetworkVarBase<bool,CLabIndicatorPanel::NetworkVar_m_bStopped>::operator=<bool>() at line 217
	// inlined CNetworkVarBase<bool,CLabIndicatorPanel::NetworkVar_m_bCountingDown>::operator=<bool>() at line 218
	// inlined CNetworkVarBase<bool,CLabIndicatorPanel::NetworkVar_m_bIsCountdownTimer>::operator=<bool>() at line 219
}

// game/server/portal2/indicator_panel.cpp:222 @0x714ba0 _ZN18CLabIndicatorPanel10SetCheckedEb
void CLabIndicatorPanel::SetChecked( bool bIsChecked )
{
	// inlined CNetworkVarBase<bool,CLabIndicatorPanel::NetworkVar_m_bIsChecked>::operator=<bool>() at line 224
	// inlined CNetworkVarBase<bool,CLabIndicatorPanel::NetworkVar_m_bIsCountdownTimer>::operator=<bool>() at line 225
}

// game/server/portal2/indicator_panel.cpp:228 @0x7151d0 _ZN18CLabIndicatorPanel15SetPlayerPingedEi
void CLabIndicatorPanel::SetPlayerPinged( int iPlayerPinged )
{
	// inlined CNetworkVarBase<int,CLabIndicatorPanel::NetworkVar_m_iPlayerPinged>::operator=<int>() at line 230
	// inlined CNetworkVarBase<bool,CLabIndicatorPanel::NetworkVar_m_bIsCountdownTimer>::operator=<bool>() at line 231
}

// game/server/portal2/indicator_panel.cpp:234 (declaration)
void SetIsTimer( bool bIsTimer );

// game/server/portal2/indicator_panel.cpp:234 @0x716020 _ZN18CLabIndicatorPanel10SetIsTimerEb
void CLabIndicatorPanel::SetIsTimer( bool bIsTimer )
{
	// inlined CNetworkVarBase<bool,CLabIndicatorPanel::NetworkVar_m_bIsCountdownTimer>::operator=<bool>() at line 236
}

// game/server/portal2/indicator_panel.cpp:246 (declaration)
void GetBaseMap();

// game/server/portal2/indicator_panel.cpp:246 @0x712db0 _ZN19CPropIndicatorPanel14GetDataDescMapEv
datamap_t *CPropIndicatorPanel::GetDataDescMap()
{
}

// game/server/portal2/indicator_panel.cpp:246 @0x712dc0 _ZN19CPropIndicatorPanel10GetBaseMapEv
datamap_t *CPropIndicatorPanel::GetBaseMap()
{
}

// game/server/portal2/indicator_panel.cpp:246 @0xa9ef0 _Z11DataMapInitI19CPropIndicatorPanelEP9datamap_tPT_
datamap_t *DataMapInit<CPropIndicatorPanel>( CPropIndicatorPanel * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 267
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 246
	CDatadescGeneratedNameHolder nameHolder;  // line 246
	typedescription_t dataDesc[16];  // line 246
}

// game/server/portal2/indicator_panel.cpp:246
datamap_t *g_DataMapHolder;

// game/server/portal2/indicator_panel.cpp:246
void CPropIndicatorPanel::m_DataMap;

// game/server/portal2/indicator_panel.cpp:269
static CEntityFactory<CPropIndicatorPanel> prop_indicator_panel;

// game/server/portal2/indicator_panel.cpp:272 @0x712e20 _ZN19CPropIndicatorPanel8PrecacheEv
void CPropIndicatorPanel::Precache()
{
}

// game/server/portal2/indicator_panel.cpp:278 @0x715f10 _ZN19CPropIndicatorPanel5SpawnEv
void CPropIndicatorPanel::Spawn()
{
	// inlined CBaseEntity::AddSolidFlags() at line 282
	// inlined CBaseEntity::AddEFlags() at line 284
}

// game/server/portal2/indicator_panel.cpp:290 @0x712fd0 _ZN19CPropIndicatorPanel14UpdateOnRemoveEv
void CPropIndicatorPanel::UpdateOnRemove()
{
	// inlined CHandle<CLabIndicatorPanel>::operator CLabIndicatorPanel*() at line 292
}

// game/server/portal2/indicator_panel.cpp:297 @0x712dd0 _ZN19CPropIndicatorPanel16SetTimerDurationEf
void CPropIndicatorPanel::SetTimerDuration( float flDuration )
{
}

// game/server/portal2/indicator_panel.cpp:302 @0x714050 _ZN19CPropIndicatorPanel10StartTimerEv
void CPropIndicatorPanel::StartTimer()
{
	// inlined CHandle<CLabIndicatorPanel>::operator CLabIndicatorPanel*() at line 304
}

// game/server/portal2/indicator_panel.cpp:316 (declaration)
void StopTimer();

// game/server/portal2/indicator_panel.cpp:316 @0x712df0 _ZN19CPropIndicatorPanel9StopTimerEv
void CPropIndicatorPanel::StopTimer()
{
}

// game/server/portal2/indicator_panel.cpp:321 (declaration)
void ResetTimer();

// game/server/portal2/indicator_panel.cpp:321 @0x7168b0 _ZN19CPropIndicatorPanel10ResetTimerEv
void CPropIndicatorPanel::ResetTimer()
{
	// inlined CHandle<CLabIndicatorPanel>::operator CLabIndicatorPanel*() at line 323
	// inlined CLabIndicatorPanel::ResetTimer() at line 325
}

// game/server/portal2/indicator_panel.cpp:330 @0x714130 _ZN19CPropIndicatorPanel10InputStartER11inputdata_t
void CPropIndicatorPanel::InputStart( inputdata_t &indata )
{
}

// game/server/portal2/indicator_panel.cpp:336 @0x713130 _ZN19CPropIndicatorPanel9InputStopER11inputdata_t
void CPropIndicatorPanel::InputStop( inputdata_t &indata )
{
}

// game/server/portal2/indicator_panel.cpp:342 @0x716dd0 _ZN19CPropIndicatorPanel10InputResetER11inputdata_t
void CPropIndicatorPanel::InputReset( inputdata_t &indata )
{
	// inlined CPropIndicatorPanel::ResetTimer() at line 344
}

// game/server/portal2/indicator_panel.cpp:348 @0x714f70 _ZN19CPropIndicatorPanel10InputCheckER11inputdata_t
void CPropIndicatorPanel::InputCheck( inputdata_t &indata )
{
	// inlined CHandle<CLabIndicatorPanel>::operator CLabIndicatorPanel*() at line 350
}

// game/server/portal2/indicator_panel.cpp:360 @0x714ed0 _ZN19CPropIndicatorPanel12InputUncheckER11inputdata_t
void CPropIndicatorPanel::InputUncheck( inputdata_t &indata )
{
	// inlined CHandle<CLabIndicatorPanel>::operator CLabIndicatorPanel*() at line 362
}

// game/server/portal2/indicator_panel.cpp:372 @0x715880 _ZN19CPropIndicatorPanel20CreateIndicatorPanelEv
void CPropIndicatorPanel::CreateIndicatorPanel()
{
	QAngle angles;  // line 381
	VMatrix mat;  // line 385
	VMatrix rotation;  // line 385
	VMatrix tmp;  // line 385
	Vector vecForward;  // line 395
	Vector vecRight;  // line 395
	Vector vecUp;  // line 395
	Vector vecOffset;  // line 397
	// inlined CHandle<CLabIndicatorPanel>::operator==() at line 377
	// inlined CHandle<CLabIndicatorPanel>::operator=() at line 375
	// inlined CBaseEntity::GetAbsAngles() at line 381
	// inlined Vector::Vector() at line 387
	// inlined Vector::operator VectorByValue&() at line 387
	// inlined CHandle<CLabIndicatorPanel>::operator->() at line 392
	// inlined Vector::operator*() at line 397
	// inlined Vector::operator*() at line 397
	// inlined Vector::operator+() at line 397
	// inlined Vector::operator+() at line 397
	// inlined CBaseEntity::GetAbsOrigin() at line 398
	// inlined Vector::operator+() at line 398
	// inlined CHandle<CLabIndicatorPanel>::operator->() at line 398
	// inlined Vector::operator VectorByValue&() at line 398
	// inlined CHandle<CLabIndicatorPanel>::operator->() at line 401
	// inlined CHandle<CLabIndicatorPanel>::operator->() at line 404
	// inlined CHandle<CLabIndicatorPanel>::operator->() at line 405
	// inlined CHandle<CLabIndicatorPanel>::operator->() at line 406
	// inlined CLabIndicatorPanel::SetIsTimer() at line 406
	// inlined CHandle<CLabIndicatorPanel>::operator CLabIndicatorPanel*() at line 409
}

// game/server/portal2/indicator_panel.cpp:413 @0x713520 _ZN19CPropIndicatorPanel21ToggleIndicatorLightsEb
void CPropIndicatorPanel::ToggleIndicatorLights( bool bLightsOn )
{
	CBaseEntity *pLight;  // line 415
	// inlined CBaseEntity::SetTextureFrameIndex() at line 418
	// inlined CGlobalEntityList::FindEntityByName() at line 416
}

// game/server/portal2/indicator_panel.cpp:423 @0x713730 _ZN19CPropIndicatorPanel11ToggleThinkEv
void CPropIndicatorPanel::ToggleThink()
{
}

// game/server/portal2/indicator_panel.cpp:439 (declaration)
void GetBaseMap();

// game/server/portal2/indicator_panel.cpp:439 @0x712e00 _ZN19CPropTicTacToePanel14GetDataDescMapEv
datamap_t *CPropTicTacToePanel::GetDataDescMap()
{
}

// game/server/portal2/indicator_panel.cpp:439 @0x712e10 _ZN19CPropTicTacToePanel10GetBaseMapEv
datamap_t *CPropTicTacToePanel::GetBaseMap()
{
}

// game/server/portal2/indicator_panel.cpp:439 @0xa9dc0 _Z11DataMapInitI19CPropTicTacToePanelEP9datamap_tPT_
datamap_t *DataMapInit<CPropTicTacToePanel>( CPropTicTacToePanel * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 439
	CDatadescGeneratedNameHolder nameHolder;  // line 439
	typedescription_t dataDesc[4];  // line 439
}

// game/server/portal2/indicator_panel.cpp:439
datamap_t *g_DataMapHolder;

// game/server/portal2/indicator_panel.cpp:439
void CPropTicTacToePanel::m_DataMap;

// game/server/portal2/indicator_panel.cpp:448
static CEntityFactory<CPropTicTacToePanel> prop_tic_tac_toe_panel;

// game/server/portal2/indicator_panel.cpp:451 @0x715f80 _ZN19CPropTicTacToePanel5SpawnEv
void CPropTicTacToePanel::Spawn()
{
	// inlined CGameEventListener::ListenForGameEvent() at line 455
	// inlined CHandle<CLabIndicatorPanel>::operator->() at line 458
}

// game/server/portal2/indicator_panel.cpp:465 @0x715520 _ZN19CPropTicTacToePanel13FireGameEventEP10IGameEvent
void CPropTicTacToePanel::FireGameEvent( IGameEvent *event )
{
	const char *name;  // line 472
	{
		int playerId;  // line 475
		float flPingX;  // line 476
		float flPingY;  // line 477
		float flPingZ;  // line 478
		Vector vecMin;  // line 499
		Vector vecMax;  // line 499
		float temp;  // line 506
		// inlined CHandle<CLabIndicatorPanel>::operator->() at line 517
		// inlined CHandle<CLabIndicatorPanel>::operator->() at line 516
		// inlined Vector::operator VectorByValue&() at line 514
		// inlined Vector::Vector() at line 514
		// inlined Vector::operator+=() at line 512
		// inlined CBaseEntity::GetAbsOrigin() at line 512
		// inlined Vector::operator=() at line 511
		// inlined Vector::operator-() at line 511
		// inlined CBaseEntity::GetAbsOrigin() at line 511
		// inlined Vector::operator-=() at line 503
		// inlined CBaseEntity::GetAbsOrigin() at line 503
		// inlined CCollisionProperty::WorldSpaceAABB() at line 500
		{
			int i;  // line 480
			{
				CBasePlayer *pPlayer;  // line 482
				// inlined CBasePlayer::GetUserID() at line 487
			}
		}
	}
}
