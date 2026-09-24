// DWARF declaration skeleton for game/server/portal2/info_placement_helper.h
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/info_placement_helper.h:16 sizeof=0x408 (i386)
struct CInfoPlacementHelper : public CPointEntity
{
public:
	CInfoPlacementHelper();  // line 22
	virtual void Spawn();  // line 23
	virtual void UpdateOnRemove();  // line 24
	virtual void DrawDebugGeometryOverlays();  // line 25
	virtual void NotifyPortalEvent( PortalEvent_t, CPortal_Base2D * );  // line 26
	virtual void OnRestore();  // line 27
	virtual int UpdateTransmitState();  // line 29
	virtual int ObjectCaps();  // line 34
	float GetTargetRadius();  // line 40
	void OnObjectPlaced( CBaseEntity * );  // line 42
	CBaseEntity *GetTargetOverride();  // line 48
	const Vector &GetTargetOrigin();  // line 56
	const QAngle &GetTargetAngles();  // line 71
	bool ShouldUseHelperAngles();  // line 88
	bool ShouldForcePlacement();  // line 89
	void Enable();  // line 91
	void Disable();  // line 92
	void Disable( float );  // line 93
	bool IsEnabled();  // line 94
	void BindToPortal( CPortal_Base2D * );  // line 95
	void InputEnable( inputdata_t & );  // line 97
	void InputDisable( inputdata_t & );  // line 98
	COutputEvent m_OnObjectPlaced; // +0x3c0  // line 100
	COutputFloat m_ObjectPlacedSize; // +0x3d8  // line 101
private:
	CNetworkVar( string_t, m_strTargetProxy ); // +0x3f0  // line 104
	CNetworkVar( string_t, m_strTargetEntity ); // +0x3f4  // line 105
	CNetworkVar( float, m_flRadius ); // +0x3f8  // line 106
	CNetworkVar( bool, m_bSnapToHelperAngles ); // +0x3fc  // line 107
	CNetworkVar( bool, m_bForcePlacement ); // +0x3fd  // line 108
	CNetworkVar( bool, m_bDisabled ); // +0x3fe  // line 109
	CNetworkVar( float, m_flDisableTime ); // +0x400  // line 110
	CNetworkVar( bool, m_bDeferringToPortal ); // +0x404  // line 111
};

// game/server/portal2/info_placement_helper.h:16 (declaration)
~CInfoPlacementHelper();

// game/server/portal2/info_placement_helper.h:16 @0x6bac90 _ZN20CInfoPlacementHelperD0Ev
CInfoPlacementHelper::~CInfoPlacementHelper()
{
	// inlined CEntityOutputTemplate<float,FIELD_FLOAT>::~CEntityOutputTemplate() at line 16
	// inlined COutputEvent::~COutputEvent() at line 16
	// inlined CPointEntity::~CPointEntity() at line 16
	// inlined COutputEvent::~COutputEvent() at line 16
	// inlined CPointEntity::~CPointEntity() at line 16
}

// game/server/portal2/info_placement_helper.h:16 @0x6bad30 _ZN20CInfoPlacementHelperD1Ev
CInfoPlacementHelper::~CInfoPlacementHelper()
{
	// inlined CEntityOutputTemplate<float,FIELD_FLOAT>::~CEntityOutputTemplate() at line 16
	// inlined COutputEvent::~COutputEvent() at line 16
	// inlined CPointEntity::~CPointEntity() at line 16
	// inlined COutputEvent::~COutputEvent() at line 16
	// inlined CPointEntity::~CPointEntity() at line 16
}

// game/server/portal2/info_placement_helper.h:29 @0x6bac10 _ZN20CInfoPlacementHelper19UpdateTransmitStateEv
int CInfoPlacementHelper::UpdateTransmitState()
{
}

// game/server/portal2/info_placement_helper.h:34 @0x6baba0 _ZN20CInfoPlacementHelper10ObjectCapsEv
int CInfoPlacementHelper::ObjectCaps()
{
	int flags;  // line 36
	// inlined CPointEntity::ObjectCaps() at line 37
	// inlined CBaseEntity::GetOwnerEntity() at line 36
}
