// DWARF declaration skeleton for game/server/portal2/portal_playerlocaldata.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xb12a0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined Vector::Vector() at line 119
}

// game/server/portal2/portal_playerlocaldata.cpp:16 @0xb0340 _Z15ServerClassInitIN14DT_PortalLocal7ignoredEEiPT_
int ServerClassInit<DT_PortalLocal::ignored>( DT_PortalLocal::ignored * )
{
	SendTable &sendTable;  // line 16
	char *const g_pSendTableName;  // line 16
	SendProp g_SendProps[36];  // line 16
}

// game/server/portal2/portal_playerlocaldata.cpp:16
SendTable g_SendTable;

// game/server/portal2/portal_playerlocaldata.cpp:16
int g_SendTableInit;

// game/server/portal2/portal_playerlocaldata.cpp:68 (declaration)
void GetBaseMap();

// game/server/portal2/portal_playerlocaldata.cpp:68 @0x743970 _ZN22CPortalPlayerLocalData10GetBaseMapEv
datamap_t *CPortalPlayerLocalData::GetBaseMap()
{
}

// game/server/portal2/portal_playerlocaldata.cpp:68 @0xb0270 _Z11DataMapInitI22CPortalPlayerLocalDataEP9datamap_tPT_
datamap_t *DataMapInit<CPortalPlayerLocalData>( CPortalPlayerLocalData * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 68
	CDatadescGeneratedNameHolder nameHolder;  // line 68
	typedescription_t dataDesc[36];  // line 68
}

// game/server/portal2/portal_playerlocaldata.cpp:68
datamap_t *g_DataMapHolder;

// game/server/portal2/portal_playerlocaldata.cpp:68
void CPortalPlayerLocalData::m_DataMap;

// game/server/portal2/portal_playerlocaldata.cpp:119
static const Vector DEFAULT_SURFACE_NORMAL;

// game/server/portal2/portal_playerlocaldata.cpp:121 (declaration)
void CPortalPlayerLocalData();

// game/server/portal2/portal_playerlocaldata.cpp:121 @0x743980 _ZN22CPortalPlayerLocalDataC2Ev
CPortalPlayerLocalData::CPortalPlayerLocalData()
{
	// inlined CNetworkVarBase<bool,CPortalPlayerLocalData::NetworkVar_m_bInTractorBeam>::operator=<bool>() at line 189
	// inlined CNetworkVarBase<bool,CPortalPlayerLocalData::NetworkVar_m_bBounced>::operator=<bool>() at line 187
	// inlined CNetworkVarBase<bool,CPortalPlayerLocalData::NetworkVar_m_bDuckedInAir>::operator=<bool>() at line 185
	// inlined CNetworkVarBase<bool,CPortalPlayerLocalData::NetworkVar_m_bDoneCorrectPitch>::operator=<bool>() at line 183
	// inlined CNetworkVarBase<bool,CPortalPlayerLocalData::NetworkVar_m_bDoneStickInterp>::operator=<bool>() at line 182
	// inlined CNetworkVarBase<bool,CPortalPlayerLocalData::NetworkVar_m_bJumpedThisFrame>::operator=<bool>() at line 180
	// inlined CNetworkVarBase<InAirState,CPortalPlayerLocalData::NetworkVar_m_InAirState>::operator=<InAirState>() at line 179
	// inlined CNetworkVarBase<StickCameraState,CPortalPlayerLocalData::NetworkVar_m_nStickCameraState>::operator=<StickCameraState>() at line 177
	// inlined CNetworkVarBase<float,CPortalPlayerLocalData::NetworkVar_m_flCurrentStickTime>::operator=<float>() at line 175
	// inlined ConVar::GetFloat() at line 175
	// inlined CNetworkVarBase<float,CPortalPlayerLocalData::NetworkVar_m_flAirInputScale>::operator=<float>() at line 174
	// inlined CountdownTimer::Invalidate() at line 172
	// inlined CNetworkVarBase<PaintPowerType,CPortalPlayerLocalData::NetworkVar_m_PaintedPowerType>::operator=<PaintPowerType>() at line 171
	// inlined CNetworkVectorBase<QAngle,CPortalPlayerLocalData::NetworkVar_m_qQuaternionPunch>::operator=() at line 169
	// inlined QAngle::operator QAngleByValue&() at line 169
	// inlined QAngle::QAngle() at line 169
	// inlined CNetworkVectorBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeUpOffset>::operator=() at line 168
	// inlined Vector::operator VectorByValue&() at line 168
	// inlined Vector::Vector() at line 168
	// inlined CNetworkVectorBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeOffset>::operator=() at line 167
	// inlined Vector::operator VectorByValue&() at line 167
	// inlined Vector::Vector() at line 167
	// inlined CNetworkVectorBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vLocalUp>::operator=() at line 166
	// inlined Vector::operator VectorByValue&() at line 166
	// inlined Vector::Vector() at line 166
	// inlined CNetworkVarBase<bool,CPortalPlayerLocalData::NetworkVar_m_bAttemptHullResize>::operator=<bool>() at line 164
	// inlined CNetworkVectorBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMaxAttempt>::operator=() at line 163
	// inlined CNetworkVectorBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMinAttempt>::operator=() at line 162
	// inlined CNetworkVectorBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMaxAttempt>::operator=() at line 161
	// inlined CNetworkVectorBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMinAttempt>::operator=() at line 160
	// inlined CNetworkVectorBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMax>::operator=() at line 159
	// inlined CNetworkVectorBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMin>::operator=() at line 158
	// inlined CNetworkVectorBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMax>::operator=() at line 157
	// inlined CNetworkVectorBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMin>::operator=() at line 156
	// inlined CNetworkVectorBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vStickRotationAxis>::operator=() at line 154
	// inlined Vector::operator VectorByValue&() at line 154
	// inlined Vector::Vector() at line 154
	// inlined CNetworkVectorBase<Vector,CPortalPlayerLocalData::NetworkVar_m_Up>::operator=() at line 153
	// inlined CNetworkVectorBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vPreUpdateVelocity>::operator=() at line 152
	// inlined CNetworkVectorBase<Vector,CPortalPlayerLocalData::NetworkVar_m_OldStickNormal>::operator=() at line 151
	// inlined CNetworkVectorBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StickNormal>::operator=() at line 150
	{
		unsigned int i;  // line 142
		// inlined CPortalPlayerLocalData::NetworkVar_m_PaintPowerHudInfoValidity::Set() at line 146
		// inlined CPortalPlayerLocalData::NetworkVar_m_PaintPowerHudInfoNormal::Set() at line 145
		// inlined CPortalPlayerLocalData::NetworkVar_m_PaintPowerHudInfoPosition::Set() at line 144
	}
	// inlined CNetworkVarBase<bool,CPortalPlayerLocalData::NetworkVar_m_bPlacingPhoto>::operator=<bool>() at line 138
	{
		int i;  // line 133
		// inlined CPortalPlayerLocalData::NetworkVar_m_nLocatorEntityIndices::GetForModify() at line 135
	}
	// inlined CNetworkVarBase<float,CPortalPlayerLocalData::NetworkVar_m_flAirControlSupressionTime>::operator=<float>() at line 130
	// inlined CPortalPlayerLocalData::NetworkVar_m_PaintedPowerTimer::NetworkVar_m_PaintedPowerTimer() at line 121
}

// game/server/portal2/portal_playerlocaldata.cpp:121 @0x744ca0 _ZN22CPortalPlayerLocalDataC1Ev
CPortalPlayerLocalData::CPortalPlayerLocalData()
{
}
