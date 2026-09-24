// DWARF declaration skeleton for game/client/portal/c_portal_base2d.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal/c_portal_base2d.h:29 sizeof=0x33bc (i386)
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
	virtual void PreDataUpdate( DataUpdateType_t );  // line 74
	virtual void PostDataUpdate( DataUpdateType_t );  // line 75
	virtual bool ShouldRegenerateOriginFromCellBits() const;  // line 78
	virtual void OnPortalMoved();  // line 80
	virtual void OnActiveStateChanged();  // line 81
	virtual void OnLinkageChanged( C_Portal_Base2D * );  // line 82
	virtual IClientModelRenderable *GetClientModelRenderable();  // line 83
	virtual int DrawModel( int, const RenderableInstance_t & );  // line 84
	virtual void StartTouch( C_BaseEntity * );  // line 85
	virtual void Touch( C_BaseEntity * );  // line 86
	virtual void EndTouch( C_BaseEntity * );  // line 87
	bool ShouldTeleportTouchingEntity( C_BaseEntity * );  // line 88
	void TeleportTouchingEntity( C_BaseEntity * );  // line 89
	virtual void PreTeleportTouchingEntity( C_BaseEntity * );  // line 90
	virtual void PostTeleportTouchingEntity( C_BaseEntity * );  // line 91
	virtual void UpdatePartitionListEntry();  // line 92
	virtual bool TestCollision( const Ray_t &, unsigned int, trace_t & );  // line 93
	virtual void PortalSimulator_TookOwnershipOfEntity( C_BaseEntity * );  // line 95
	virtual void PortalSimulator_ReleasedOwnershipOfEntity( C_BaseEntity * );  // line 96
	void UpdateTeleportMatrix();  // line 99
	void UpdateGhostRenderables();  // line 100
	void SetIsPortal2( bool );  // line 102
	bool IsActivedAndLinked() const;  // line 104
	CPortalSimulator m_PortalSimulator; // +0x1034  // line 106
	virtual C_BaseEntity *PortalRenderable_GetPairedEntity();  // line 108
	bool IsMobile() const;  // line 110
	float GetHalfWidth() const;  // line 112
	float GetHalfHeight() const;  // line 113
	Vector GetLocalMins() const;  // line 114
	Vector GetLocalMaxs() const;  // line 115
	void UpdateCollisionShape();  // line 117
	virtual void NewLocation( const Vector &, const QAngle & );  // line 119
	virtual void DrawStencilMask();  // line 120
	virtual float GetMinimumExitSpeed( bool, bool, bool );  // line 123
	virtual float GetMaximumExitSpeed( bool, bool, bool );  // line 124
	static void GetExitSpeedRange( C_Portal_Base2D *, bool, float &, float & );  // line 127
protected:
	bool m_bActivated; // +0x3370  // line 130
	bool m_bOldActivatedState; // +0x3371  // line 131
	CUtlVector<CHandle<C_BaseEntity>,CUtlMemory<CHandle<C_BaseEntity>, int> > m_hGhostingEntities; // +0x3374  // line 133
	CUtlVector<C_PortalGhostRenderable*,CUtlMemory<C_PortalGhostRenderable*, int> > m_GhostRenderables; // +0x3388  // line 134
	float m_fGhostRenderablesClip[4]; // +0x339c  // line 135
	float m_fNetworkHalfWidth; // +0x33ac  // line 137
	float m_fNetworkHalfHeight; // +0x33b0  // line 137
	bool m_bIsMobile; // +0x33b4  // line 138
	CPhysCollide *m_pCollisionShape; // +0x33b8  // line 139
	static const PortalDrawingMaterials &m_Materials;  // line 141
};
