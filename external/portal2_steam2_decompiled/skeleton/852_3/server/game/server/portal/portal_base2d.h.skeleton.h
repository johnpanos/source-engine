// DWARF declaration skeleton for game/server/portal/portal_base2d.h
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal/portal_base2d.h:25 sizeof=0x2a8c (i386)
struct CPortal_Base2D : public CBaseAnimating, public CPortalSimulatorEventCallbacks, public CPVS_Extender, public CPortal_Base2D_Shared
{
public:
	CPortal_Base2D();  // line 31
	virtual ~CPortal_Base2D();  // line 32
	CNetworkHandle( CPortal_Base2D, m_hLinkedPortal ); // +0x524  // line 34
	VMatrix m_matrixThisToLinked; // +0x528  // line 37
	CNetworkVar( bool, m_bIsPortal2 ); // +0x568  // line 38
	Vector m_vPrevForward; // +0x56c  // line 39
	bool m_bSharedEnvironmentConfiguration; // +0x578  // line 41
	EHANDLE m_hMicrophone; // +0x57c  // line 43
	EHANDLE m_hSpeaker; // +0x580  // line 44
	bool m_bMicAndSpeakersLinkedToRemote; // +0x584  // line 45
	Vector m_vAudioOrigin; // +0x588  // line 47
	Vector m_vDelayedPosition; // +0x594  // line 48
	QAngle m_qDelayedAngles; // +0x5a0  // line 49
	int m_iDelayedFailure; // +0x5ac  // line 50
	Vector m_vOldPosition; // +0x5b0  // line 51
	QAngle m_qOldAngles; // +0x5bc  // line 52
	EHANDLE m_hPlacedBy; // +0x5c8  // line 53
	COutputEvent m_OnPlacedSuccessfully; // +0x5cc  // line 55
	COutputEvent m_OnEntityTeleportFromMe; // +0x5e4  // line 56
	COutputEvent m_OnPlayerTeleportFromMe; // +0x5fc  // line 57
	COutputEvent m_OnEntityTeleportToMe; // +0x614  // line 58
	COutputEvent m_OnPlayerTeleportToMe; // +0x62c  // line 59
	Vector m_ptOrigin; // +0x644  // line 61
	Vector m_vForward; // +0x650  // line 61
	Vector m_vUp; // +0x65c  // line 61
	Vector m_vRight; // +0x668  // line 61
	QAngle m_qAbsAngle; // +0x674  // line 62
	cplane_t m_plane_Origin; // +0x680  // line 63
	CPhysicsCloneArea *m_pAttachedCloningArea; // +0x694  // line 65
	bool IsPortal2() const;  // line 67
	void SetIsPortal2( bool );  // line 68
	const VMatrix &MatrixThisToLinked() const;  // line 69
	virtual int UpdateTransmitState();  // line 71
	virtual void Spawn();  // line 76
	virtual void Activate();  // line 77
	virtual void OnRestore();  // line 78
	virtual bool IsActive() const;  // line 79
	virtual bool GetOldActiveState() const;  // line 80
	virtual void SetActive( bool );  // line 81
	virtual void UpdateOnRemove();  // line 83
	void TestRestingSurfaceThink();  // line 85
	static const char *s_szTestRestingSurfaceThinkContext;  // line 86
	void DeactivatePortalOnThink();  // line 88
	void DeactivatePortalNow();  // line 89
	static const char *s_szDeactivatePortalNowContext;  // line 90
	virtual void OnPortalDeactivated();  // line 92
	bool IsActivedAndLinked() const;  // line 94
	void WakeNearbyEntities();  // line 96
	void ForceEntityToFitInPortalWall( CBaseEntity * );  // line 98
	virtual void NewLocation( const Vector &, const QAngle & );  // line 100
	void PunchPenetratingPlayer( CBaseEntity * );  // line 102
	void PunchAllPenetratingPlayers();  // line 103
	virtual void StartTouch( CBaseEntity * );  // line 105
	virtual void Touch( CBaseEntity * );  // line 106
	virtual void EndTouch( CBaseEntity * );  // line 107
	bool ShouldTeleportTouchingEntity( CBaseEntity * );  // line 108
	void TeleportTouchingEntity( CBaseEntity * );  // line 109
	virtual void PreTeleportTouchingEntity( CBaseEntity * );  // line 110
	virtual void PostTeleportTouchingEntity( CBaseEntity * );  // line 111
	virtual void PhysicsSimulate();  // line 113
	virtual void UpdatePortalLinkage();  // line 115
	void UpdatePortalTeleportMatrix();  // line 116
	bool SharedEnvironmentCheck( CBaseEntity * );  // line 120
	Vector m_vPortalCorners[4]; // +0x698  // line 123
	CPortal_Base2D::NetworkVar_m_PortalSimulator m_PortalSimulator; // +0x6c8  // line 125
	static int GetOffset_m_PortalSimulator();  // line 125
	virtual bool TestCollision( const Ray_t &, unsigned int, trace_t & );  // line 130
	virtual void PortalSimulator_TookOwnershipOfEntity( CBaseEntity * );  // line 132
	virtual void PortalSimulator_ReleasedOwnershipOfEntity( CBaseEntity * );  // line 133
	virtual void CreateMicAndSpeaker();  // line 135
	void AddPortalEventListener( EHANDLE );  // line 138
	void RemovePortalEventListener( EHANDLE );  // line 139
	void OnEntityTeleportedToPortal( CBaseEntity * );  // line 141
	void OnEntityTeleportedFromPortal( CBaseEntity * );  // line 142
protected:
	CNetworkVar( bool, m_bActivated ); // +0x2a58  // line 145
	CNetworkVar( bool, m_bOldActivatedState ); // +0x2a59  // line 146
	void BroadcastPortalEvent( PortalEvent_t );  // line 148
	CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> > m_PortalEventListeners; // +0x2a5c  // line 150
	void RemovePortalMicAndSpeaker();  // line 152
	void UpdateCorners();  // line 153
	void UpdateClientCheckPVS();  // line 154
	void UpdateCollisionShape();  // line 155
	CNetworkVar( float, m_fNetworkHalfWidth ); // +0x2a70  // line 157
	CNetworkVar( float, m_fNetworkHalfHeight ); // +0x2a74  // line 158
	CNetworkVar( bool, m_bIsMobile ); // +0x2a78  // line 159
	CPhysCollide *m_pCollisionShape; // +0x2a7c  // line 161
public:
	CPortal_Base2D *GetLinkedPortal();  // line 164
	float GetHalfWidth() const;  // line 166
	float GetHalfHeight() const;  // line 167
	Vector GetLocalMins() const;  // line 168
	Vector GetLocalMaxs() const;  // line 169
	bool IsMobile() const;  // line 172
	void SetMobileState( bool );  // line 173
	void Resize( float, float );  // line 175
	virtual CServerNetworkProperty *GetExtenderNetworkProp();  // line 178
	virtual const edict_t *GetExtenderEdict() const;  // line 179
	virtual Vector GetExtensionPVSOrigin();  // line 180
	virtual bool IsExtenderValid();  // line 182
	virtual int GetPolyVertCount();  // line 185
	virtual int ComputeFrustumThroughPolygon( const Vector &, const VPlane *, int, VPlane *, int );  // line 186
	virtual void ComputeSubVisibility( CPVS_Extender **, int, unsigned char *, int, const Vector &, const VPlane *, int, CPVS_Extender::VisExtensionChain_t *, int *, int );  // line 189
	virtual float GetMinimumExitSpeed( bool, bool, bool );  // line 192
	virtual float GetMaximumExitSpeed( bool, bool, bool );  // line 193
	static void GetExitSpeedRange( CPortal_Base2D *, bool, float &, float & );  // line 196
private:
	Vector m_vPortalSpawnLocation; // +0x2a80  // line 199
};
