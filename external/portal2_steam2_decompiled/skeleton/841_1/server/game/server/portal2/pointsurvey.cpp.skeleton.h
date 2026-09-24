// DWARF declaration skeleton for game/server/portal2/pointsurvey.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xaf230 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CEntityFactory<CPointSurvey>::CEntityFactory() at line 16
	// inlined ServerClass::ServerClass() at line 28
}

// game/server/portal2/pointsurvey.cpp:14
ConVar cl_disable_survey_panel;

// game/server/portal2/pointsurvey.cpp:16
static CEntityFactory<CPointSurvey> point_survey;

// game/server/portal2/pointsurvey.cpp:19 (declaration)
void GetBaseMap();

// game/server/portal2/pointsurvey.cpp:19 @0x73f480 _ZN12CPointSurvey14GetDataDescMapEv
datamap_t *CPointSurvey::GetDataDescMap()
{
}

// game/server/portal2/pointsurvey.cpp:19 @0x73f490 _ZN12CPointSurvey10GetBaseMapEv
datamap_t *CPointSurvey::GetBaseMap()
{
}

// game/server/portal2/pointsurvey.cpp:19 @0xaf110 _Z11DataMapInitI12CPointSurveyEP9datamap_tPT_
datamap_t *DataMapInit<CPointSurvey>( CPointSurvey * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 19
	CDatadescGeneratedNameHolder nameHolder;  // line 19
	typedescription_t dataDesc[4];  // line 19
}

// game/server/portal2/pointsurvey.cpp:19
datamap_t *g_DataMapHolder;

// game/server/portal2/pointsurvey.cpp:19
void CPointSurvey::m_DataMap;

// game/server/portal2/pointsurvey.cpp:28 @0x73f4a0 _ZN12CPointSurvey14GetServerClassEv
ServerClass *CPointSurvey::GetServerClass()
{
}

// game/server/portal2/pointsurvey.cpp:28 @0x73f4b0 _ZN12CPointSurvey40YouForgotToImplementOrDeclareServerClassEv
int CPointSurvey::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/pointsurvey.cpp:28 @0xaef50 _Z15ServerClassInitIN14DT_PointSurvey7ignoredEEiPT_
int ServerClassInit<DT_PointSurvey::ignored>( DT_PointSurvey::ignored * )
{
	SendTable &sendTable;  // line 28
	char *const g_pSendTableName;  // line 28
	SendProp g_SendProps[3];  // line 28
}

// game/server/portal2/pointsurvey.cpp:28
SendTable g_SendTable;

// game/server/portal2/pointsurvey.cpp:28
int g_SendTableInit;

// game/server/portal2/pointsurvey.cpp:28
static ServerClass g_CPointSurvey_ClassReg;

// game/server/portal2/pointsurvey.cpp:28
void CPointSurvey::m_pClassSendTable;

// game/server/portal2/pointsurvey.cpp:34 @0x73f520 _ZN12CPointSurvey15InputShowSurveyER11inputdata_t
void CPointSurvey::InputShowSurvey( inputdata_t &inputdata )
{
	CBroadcastRecipientFilter filter;  // line 42
	// inlined CPointSurvey::OnSurveyCompleted() at line 38
	// inlined CBroadcastRecipientFilter::CBroadcastRecipientFilter() at line 42
	// inlined CBroadcastRecipientFilter::~CBroadcastRecipientFilter() at line 46
	// inlined CBroadcastRecipientFilter::~CBroadcastRecipientFilter() at line 46
}

// game/server/portal2/pointsurvey.cpp:49 (declaration)
void OnSurveyCompleted();

// game/server/portal2/pointsurvey.cpp:49 @0x73f4f0 _ZN12CPointSurvey17OnSurveyCompletedEv
void CPointSurvey::OnSurveyCompleted()
{
}

// game/server/portal2/pointsurvey.cpp:55 @0x73f4c0 _ZN12CPointSurvey8ActivateEv
void CPointSurvey::Activate()
{
}

// game/server/util.h:100 @0x73f6b0 _ZN14CEntityFactoryI12CPointSurveyE6CreateEPKc
IServerNetworkable *CEntityFactory<CPointSurvey>::Create( const char *pClassName )
{
	CPointSurvey *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CPointSurvey>() at line 102
}

// game/server/util.h:106 @0x73f610 _ZN14CEntityFactoryI12CPointSurveyE7DestroyEP18IServerNetworkable
void CEntityFactory<CPointSurvey>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:114 @0x73f640 _ZN14CEntityFactoryI12CPointSurveyE13GetEntitySizeEv
size_t CEntityFactory<CPointSurvey>::GetEntitySize()
{
}
