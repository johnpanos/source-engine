// DWARF declaration skeleton for game/client/portal/c_portal_base2d.h
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal/c_portal_base2d.h:29 sizeof=0x33c8 (i386)
struct C_Portal_Base2D : public CPortalRenderable_FlatBasic, public CPortal_Base2D_Shared, public CPortalSimulatorEventCallbacks, public CSignifierTarget
{
public:
	static typedescription_t m_PredDesc[];  // line 33
	C_Portal_Base2D();  // line 35
	virtual ~C_Portal_Base2D();  // line 36
	virtual void GetToolRecordingState( KeyValues * );  // line 39
	CHandle<C_Portal_Base2D> m_hLinkedPortal; // +0xff4  // line 41
	bool m_bSharedEnvironmentConfiguration; // +0xff8  // line 43
	cplane_t m_plane_Origin; // +0xffc  // line 45
	virtual void Spawn();  // line 47
	virtual void Activate();  // line 48
	virtual bool Simulate();  // line 50
	virtual void UpdateOnRemove();  // line 52
	virtual bool IsActive() const;  // line 53
	virtual bool GetOldActiveState() const;  // line 54
	virtual void SetActive( bool );  // line 55
	virtual void OnNewParticleEffect( const char *, CNewParticleEffect * );  // line 57
	virtual bool UseSelectionGlow();  // line 59
	// game/client/portal/c_portal_base2d.h:62 sizeof=0x24 (i386)
	struct Portal_PreDataChanged
	{
	public:
		bool m_bActivated; // +0x0  // line 63
		bool m_bOldActivatedState; // +0x1  // line 64
		bool m_bIsPortal2; // +0x2  // line 65
		Vector m_vOrigin; // +0x4  // line 66
		QAngle m_qAngles; // +0x10  // line 67
		CHandle<C_Portal_Base2D> m_hLinkedTo; // +0x1c  // line 68
		bool m_bIsMobile; // +0x20  // line 69
	};  // line 62
	C_Portal_Base2D::Portal_PreDataChanged PreDataChanged; // +0x1010  // line 70
	virtual void OnPreDataChanged( DataUpdateType_t );  // line 72
	virtual void OnDataChanged( DataUpdateType_t );  // line 73
	virtual bool ShouldRegenerateOriginFromCellBits() const;  // line 76
	virtual void OnPortalMoved();  // line 78
	virtual void OnActiveStateChanged();  // line 79
	virtual void OnLinkageChanged( C_Portal_Base2D * );  // line 80
	virtual bool ShouldDraw();  // line 81
	virtual void StartTouch( C_BaseEntity * );  // line 82
	virtual void Touch( C_BaseEntity * );  // line 83
	virtual void EndTouch( C_BaseEntity * );  // line 84
	bool ShouldTeleportTouchingEntity( C_BaseEntity * );  // line 85
	void TeleportTouchingEntity( C_BaseEntity * );  // line 86
	virtual void PreTeleportTouchingEntity( C_BaseEntity * );  // line 87
	virtual void PostTeleportTouchingEntity( C_BaseEntity * );  // line 88
	virtual void UpdatePartitionListEntry();  // line 89
	virtual bool TestCollision( const Ray_t &, unsigned int, trace_t & );  // line 90
	virtual void PortalSimulator_TookOwnershipOfEntity( C_BaseEntity * );  // line 92
	virtual void PortalSimulator_ReleasedOwnershipOfEntity( C_BaseEntity * );  // line 93
	void UpdateTeleportMatrix();  // line 96
	void UpdateGhostRenderables();  // line 97
	void SetIsPortal2( bool );  // line 99
	bool IsActivedAndLinked() const;  // line 101
	CPortalSimulator m_PortalSimulator; // +0x1034  // line 103
	virtual C_BaseEntity *PortalRenderable_GetPairedEntity();  // line 105
	bool IsMobile() const;  // line 107
	float GetHalfWidth() const;  // line 109
	float GetHalfHeight() const;  // line 110
	Vector GetLocalMins() const;  // line 111
	Vector GetLocalMaxs() const;  // line 112
	void UpdateCollisionShape();  // line 114
	virtual void NewLocation( const Vector &, const QAngle & );  // line 116
	virtual void DrawStencilMask( IMatRenderContext * );  // line 117
	virtual float GetMinimumExitSpeed( bool, bool, bool );  // line 120
	virtual float GetMaximumExitSpeed( bool, bool, bool );  // line 121
	static void GetExitSpeedRange( C_Portal_Base2D *, bool, float &, float & );  // line 124
protected:
	bool m_bActivated; // +0x337c  // line 127
	bool m_bOldActivatedState; // +0x337d  // line 128
	CUtlVector<CHandle<C_BaseEntity>,CUtlMemory<CHandle<C_BaseEntity>, int> > m_hGhostingEntities; // +0x3380  // line 130
	CUtlVector<C_PortalGhostRenderable*,CUtlMemory<C_PortalGhostRenderable*, int> > m_GhostRenderables; // +0x3394  // line 131
	float m_fGhostRenderablesClip[4]; // +0x33a8  // line 132
	float m_fNetworkHalfWidth; // +0x33b8  // line 134
	float m_fNetworkHalfHeight; // +0x33bc  // line 134
	bool m_bIsMobile; // +0x33c0  // line 135
	CPhysCollide *m_pCollisionShape; // +0x33c4  // line 136
	static const PortalDrawingMaterials &m_Materials;  // line 138
};
