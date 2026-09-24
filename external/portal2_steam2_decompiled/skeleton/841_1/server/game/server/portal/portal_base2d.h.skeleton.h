// DWARF declaration skeleton for game/server/portal/portal_base2d.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal/portal_base2d.h:25 sizeof=0x2a98 (i386)
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
	CNetworkVector( Vector, m_ptOrigin ); // +0x644  // line 61
	Vector m_vForward; // +0x650  // line 62
	Vector m_vUp; // +0x65c  // line 62
	Vector m_vRight; // +0x668  // line 62
	CNetworkQAngle( QAngle, m_qAbsAngle ); // +0x674  // line 63
	cplane_t m_plane_Origin; // +0x680  // line 64
	CPhysicsCloneArea *m_pAttachedCloningArea; // +0x694  // line 66
	bool IsPortal2() const;  // line 68
	void SetIsPortal2( bool );  // line 69
	const VMatrix &MatrixThisToLinked() const;  // line 70
	virtual int UpdateTransmitState();  // line 72
	virtual void Spawn();  // line 77
	virtual void Activate();  // line 78
	virtual void OnRestore();  // line 79
	virtual bool IsActive() const;  // line 80
	virtual bool GetOldActiveState() const;  // line 81
	virtual void SetActive( bool );  // line 82
	virtual void UpdateOnRemove();  // line 84
	void TestRestingSurfaceThink();  // line 86
	static const char *s_szTestRestingSurfaceThinkContext;  // line 87
	void DeactivatePortalOnThink();  // line 89
	void DeactivatePortalNow();  // line 90
	static const char *s_szDeactivatePortalNowContext;  // line 91
	virtual void OnPortalDeactivated();  // line 93
	bool IsActivedAndLinked() const;  // line 95
	void WakeNearbyEntities();  // line 97
	void ForceEntityToFitInPortalWall( CBaseEntity * );  // line 99
	virtual void NewLocation( const Vector &, const QAngle & );  // line 101
	void PunchPenetratingPlayer( CBaseEntity * );  // line 103
	void PunchAllPenetratingPlayers();  // line 104
	virtual void StartTouch( CBaseEntity * );  // line 106
	virtual void Touch( CBaseEntity * );  // line 107
	virtual void EndTouch( CBaseEntity * );  // line 108
	bool ShouldTeleportTouchingEntity( CBaseEntity * );  // line 109
	void TeleportTouchingEntity( CBaseEntity * );  // line 110
	virtual void PreTeleportTouchingEntity( CBaseEntity * );  // line 111
	virtual void PostTeleportTouchingEntity( CBaseEntity * );  // line 112
	virtual void PhysicsSimulate();  // line 114
	virtual void UpdatePortalLinkage();  // line 116
	void UpdatePortalTeleportMatrix();  // line 117
	bool SharedEnvironmentCheck( CBaseEntity * );  // line 121
	Vector m_vPortalCorners[4]; // +0x698  // line 124
	CPortal_Base2D::NetworkVar_m_PortalSimulator m_PortalSimulator; // +0x6c8  // line 126
	static int GetOffset_m_PortalSimulator();  // line 126
	virtual bool TestCollision( const Ray_t &, unsigned int, trace_t & );  // line 131
	virtual void PortalSimulator_TookOwnershipOfEntity( CBaseEntity * );  // line 133
	virtual void PortalSimulator_ReleasedOwnershipOfEntity( CBaseEntity * );  // line 134
	virtual void CreateMicAndSpeaker();  // line 136
	void AddPortalEventListener( EHANDLE );  // line 139
	void RemovePortalEventListener( EHANDLE );  // line 140
	void OnEntityTeleportedToPortal( CBaseEntity * );  // line 142
	void OnEntityTeleportedFromPortal( CBaseEntity * );  // line 143
protected:
	CNetworkVar( bool, m_bActivated ); // +0x2a64  // line 146
	CNetworkVar( bool, m_bOldActivatedState ); // +0x2a65  // line 147
	void BroadcastPortalEvent( PortalEvent_t );  // line 149
	CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> > m_PortalEventListeners; // +0x2a68  // line 151
	void RemovePortalMicAndSpeaker();  // line 153
	void UpdateCorners();  // line 154
	void UpdateClientCheckPVS();  // line 155
	void UpdateCollisionShape();  // line 156
	CNetworkVar( float, m_fNetworkHalfWidth ); // +0x2a7c  // line 158
	CNetworkVar( float, m_fNetworkHalfHeight ); // +0x2a80  // line 159
	CNetworkVar( bool, m_bIsMobile ); // +0x2a84  // line 160
	CPhysCollide *m_pCollisionShape; // +0x2a88  // line 162
public:
	CPortal_Base2D *GetLinkedPortal();  // line 165
	float GetHalfWidth() const;  // line 167
	float GetHalfHeight() const;  // line 168
	Vector GetLocalMins() const;  // line 169
	Vector GetLocalMaxs() const;  // line 170
	bool IsMobile() const;  // line 173
	void SetMobileState( bool );  // line 174
	void Resize( float, float );  // line 176
	virtual CServerNetworkProperty *GetExtenderNetworkProp();  // line 179
	virtual const edict_t *GetExtenderEdict() const;  // line 180
	virtual Vector GetExtensionPVSOrigin();  // line 181
	virtual bool IsExtenderValid();  // line 183
	virtual int GetPolyVertCount();  // line 186
	virtual int ComputeFrustumThroughPolygon( const Vector &, const VPlane *, int, VPlane *, int );  // line 187
	virtual void ComputeSubVisibility( CPVS_Extender **, int, unsigned char *, int, const Vector &, const VPlane *, int, CPVS_Extender::VisExtensionChain_t *, int *, int );  // line 190
	virtual float GetMinimumExitSpeed( bool, bool, bool );  // line 193
	virtual float GetMaximumExitSpeed( bool, bool, bool );  // line 194
	static void GetExitSpeedRange( CPortal_Base2D *, bool, float &, float & );  // line 197
private:
	Vector m_vPortalSpawnLocation; // +0x2a8c  // line 200
};
