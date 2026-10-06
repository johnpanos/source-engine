// Class layouts from /home/john/Downloads/portal2-steam2-research/852_3/portal2/bin/client.dylib.dSYM/Contents/Resources/DWARF/client.dylib
// Unit game/client/portal/c_portal_player.cpp; i386 offsets; reconstruction aid.

// game/client/portal/c_portal_player.h:41
// game/client/portal/c_portal_player.h:41 sizeof=0x2134 (i386)
struct C_Portal_Player : public PaintPowerUser<CPaintableEntity<C_BasePlayer> >
{
public:
	static typedescription_t m_PredDesc[];  // line 46
	C_Portal_Player();  // line 50
	virtual ~C_Portal_Player();  // line 51
	virtual void UpdateOnRemove();  // line 53
	virtual void Precache();  // line 54
	virtual void PostThink();  // line 56
	virtual void ClientThink();  // line 57
	static C_Portal_Player *GetLocalPortalPlayer();  // line 59
	static C_Portal_Player *GetLocalPlayer();  // line 64
	virtual const Vector &GetRenderOrigin();  // line 69
	virtual const QAngle &GetRenderAngles();  // line 70
	virtual void SetAnimation( PLAYER_ANIM );  // line 72
	virtual void UpdateClientSideAnimation();  // line 74
	void DoAnimationEvent( PlayerAnimEvent_t, int );  // line 75
	virtual const char *GetPlayerModelName();  // line 77
	virtual int DrawModel( int, const RenderableInstance_t & );  // line 78
	virtual bool Simulate();  // line 79
	QAngle GetAnimEyeAngles();  // line 81
	Vector GetAttackSpread( C_BaseCombatWeapon *, C_BaseEntity * );  // line 82
	virtual ShadowType_t ShadowCastType();  // line 85
	virtual C_BaseAnimating *BecomeRagdollOnClient();  // line 86
	virtual bool ShouldDraw();  // line 87
	virtual const QAngle &EyeAngles();  // line 88
	virtual void OnPreDataChanged( DataUpdateType_t );  // line 89
	virtual void PreDataUpdate( DataUpdateType_t );  // line 90
	virtual void OnDataChanged( DataUpdateType_t );  // line 91
	virtual void PostDataUpdate( DataUpdateType_t );  // line 92
	virtual float GetFOV();  // line 93
	virtual CStudioHdr *OnNewModel();  // line 94
	virtual void TraceAttack( const CTakeDamageInfo &, const Vector &, trace_t * );  // line 95
	virtual void ItemPreFrame();  // line 96
	virtual void ItemPostFrame();  // line 97
	virtual float GetMinFOV() const;  // line 98
	virtual Vector GetAutoaimVector( float );  // line 99
	virtual bool ShouldReceiveProjectedTextures( int );  // line 100
	virtual void GetStepSoundVelocities( float *, float * );  // line 101
	virtual void PlayStepSound( Vector &, surfacedata_t *, float, bool );  // line 102
	virtual void PreThink();  // line 103
	virtual void DoImpactEffect( trace_t &, int );  // line 104
	virtual bool CreateMove( float, CUserCmd * );  // line 105
	virtual bool ShouldCollide( int, int ) const;  // line 106
	virtual const Vector &WorldSpaceCenter() const;  // line 108
	virtual Vector EyePosition();  // line 110
	virtual Vector EyeFootPosition( const QAngle & );  // line 111
	Vector EyeFootPosition();  // line 112
	void PlayerPortalled( C_Portal_Base2D *, float );  // line 113
	void CheckPlayerAboutToTouchPortal();  // line 114
	bool IsTaunting();  // line 116
	int GetTeamTauntState() const;  // line 117
	bool IsRemoteViewTaunt() const;  // line 118
	QAngle GetTeamTauntAngles();  // line 119
	bool IsInterpolatingTauntAngles() const;  // line 120
	C_BaseEntity *FindPlayerUseEntity();  // line 122
	virtual bool IsUseableEntity( C_BaseEntity *, unsigned int );  // line 123
	virtual void CalcView( Vector &, QAngle &, float &, float &, float & );  // line 125
	void CalcPortalView( Vector &, QAngle & );  // line 126
	virtual void CalcViewModelView( const Vector &, const QAngle & );  // line 127
	bool IsInvalidHandoff( C_BaseEntity * );  // line 129
	C_BaseEntity *FindUseEntity( C_Portal_Base2D ** );  // line 130
	C_BaseEntity *FindUseEntityThroughPortal();  // line 131
	bool IsCloseToPortal();  // line 133
	virtual bool StartSceneEvent( CSceneEventInfo *, CChoreoScene *, CChoreoEvent *, CChoreoActor *, C_BaseEntity * );  // line 139
	virtual bool StartGestureSceneEvent( CSceneEventInfo *, CChoreoScene *, CChoreoEvent *, CChoreoActor *, C_BaseEntity * );  // line 140
	bool IsSuppressingAirControl();  // line 142
	void UpdateLookAt();  // line 144
	void Initialize();  // line 145
	int GetIDTarget() const;  // line 146
	void UpdateIDTarget();  // line 147
	C_Portal_Base2D *GetHeldObjectPortal();  // line 149
	$_295 TranslateActivity( $_295, bool * );  // line 151
	C_WeaponPortalBase *GetActivePortalWeapon() const;  // line 152
	static void DrawPlayerSilhouttes();  // line 158
	// game/client/portal/c_portal_player.h:161 sizeof=0x8 (i386)
	struct PredictedPortalTeleportation_t
	{
	public:
		float flTime; // +0x0  // line 162
		C_Portal_Base2D *pEnteredPortal; // +0x4  // line 163
	};  // line 161
	CUtlVector<C_Portal_Player::PredictedPortalTeleportation_t,CUtlMemory<C_Portal_Player::PredictedPortalTeleportation_t, int> > m_PredictedPortalTeleportations; // +0x1b60  // line 165
	void FinishMove( CMoveData * );  // line 167
	bool IsHoldingSomething() const;  // line 169
	virtual void ApplyTransformToInterpolators( const VMatrix &, float );  // line 171
	virtual void ApplyUnpredictedPortalTeleportation( const C_Portal_Base2D * );  // line 174
	virtual void ApplyPredictedPortalTeleportation( const C_Portal_Base2D *, CMoveData * );  // line 177
	virtual void UndoPredictedPortalTeleportation( const C_Portal_Base2D *, float );  // line 178
	void UnrollPredictedTeleportations( float );  // line 180
	virtual bool TestHitboxes( const Ray_t &, unsigned int, trace_t & );  // line 182
	float GetImplicitVerticalStepSpeed() const;  // line 184
	void SetImplicitVerticalStepSpeed( float );  // line 185
	virtual void ForceDuckThisFrame();  // line 187
	const C_PortalPlayerLocalData &GetPortalPlayerLocalData() const;  // line 189
	bool m_bPitchReorientation; // +0x1b74  // line 191
	float m_fReorientationRate; // +0x1b78  // line 192
	bool m_bEyePositionIsTransformedByPortal; // +0x1b7c  // line 193
	CHandle<C_Portal_Base2D> m_hPortalEnvironment; // +0x1b80  // line 195
	CHandle<C_Func_LiquidPortal> m_hSurroundingLiquidPortal; // +0x1b84  // line 196
	CGrabController &GetGrabController();  // line 198
	void ToggleHeldObjectOnOppositeSideOfPortal();  // line 203
	void SetHeldObjectOnOppositeSideOfPortal( bool );  // line 204
	bool IsHeldObjectOnOppositeSideOfPortal();  // line 205
	void SetHeldObjectPortal( C_Portal_Base2D * );  // line 209
	void SetUsingVMGrabState( bool );  // line 210
	bool IsUsingVMGrab();  // line 211
	bool WantsVMGrab();  // line 212
	bool IsForcingDrop();  // line 213
	EHANDLE m_hGrabbedEntity; // +0x1b88  // line 215
	EHANDLE m_hPortalThroughWhichGrabOccured; // +0x1b8c  // line 216
	bool m_bSilentDropAndPickup; // +0x1b90  // line 217
	void ForceDropOfCarriedPhysObjects( C_BaseEntity * );  // line 218
	void PickupObject( C_BaseEntity *, bool );  // line 219
	void SetInTractorBeam( bool );  // line 221
protected:
	C_PortalPlayerLocalData m_PortalLocal; // +0x1b94  // line 226
	Vector m_vWorldSpaceCenterHolder; // +0x1d38  // line 228
	bool PortalledMessageIsPending() const;  // line 230
private:
	void AvoidPlayers( CUserCmd * );  // line 234
	void ClientPlayerRespawn();  // line 236
	void TurnOnTauntCam();  // line 239
	void TurnOffTauntCam();  // line 240
	void TurnOffTauntCam_Finish();  // line 241
	void TauntCamInterpolation();  // line 242
	void HandleTaunting();  // line 243
	bool m_bTauntInterpolating; // +0x1d44  // line 245
	bool m_bTauntInterpolatingAngles; // +0x1d45  // line 246
	float m_flTauntCamCurrentDist; // +0x1d48  // line 247
	float m_flTauntCamTargetDist; // +0x1d4c  // line 248
	C_Portal_Player( const C_Portal_Player & );  // line 250
	void UpdatePortalEyeInterpolation();  // line 252
	CPortalPlayerAnimState *m_PlayerAnimState; // +0x1d50  // line 254
	QAngle m_angEyeAngles; // +0x1d54  // line 256
	CInterpolatedVar<QAngle> m_iv_angEyeAngles; // +0x1d60  // line 257
	float m_flHullHeight; // +0x1d8c  // line 260
	CInterpolatedVar<float> m_iv_flHullHeight; // +0x1d90  // line 261
	virtual IRagdoll *GetRepresentativeRagdoll() const;  // line 263
	EHANDLE m_hRagdoll; // +0x1dbc  // line 264
	int m_headYawPoseParam; // +0x1dc0  // line 266
	int m_headPitchPoseParam; // +0x1dc4  // line 267
	float m_headYawMin; // +0x1dc8  // line 268
	float m_headYawMax; // +0x1dcc  // line 269
	float m_headPitchMin; // +0x1dd0  // line 270
	float m_headPitchMax; // +0x1dd4  // line 271
	float m_flPitchFixup; // +0x1dd8  // line 273
	float m_flUprightRotDist; // +0x1ddc  // line 274
	bool m_isInit; // +0x1de0  // line 276
	Vector m_vLookAtTarget; // +0x1de4  // line 277
	float m_flLastBodyYaw; // +0x1df0  // line 279
	float m_flCurrentHeadYaw; // +0x1df4  // line 280
	float m_flCurrentHeadPitch; // +0x1df8  // line 281
	float m_flStartLookTime; // +0x1dfc  // line 282
	int m_iIDEntIndex; // +0x1e00  // line 284
	CountdownTimer m_blinkTimer; // +0x1e04  // line 286
	int m_iSpawnInterpCounter; // +0x1e10  // line 288
	int m_iSpawnInterpCounterCache; // +0x1e14  // line 289
	int m_iPlayerSoundType; // +0x1e18  // line 291
	bool m_bHeldObjectOnOppositeSideOfPortal; // +0x1e1c  // line 293
	C_Portal_Base2D *m_pHeldObjectPortal; // +0x1e20  // line 294
	int m_iForceNoDrawInPortalSurface; // +0x1e24  // line 296
public:
	// game/client/portal/c_portal_player.h:299 sizeof=0x28 (i386)
	struct PortalEyeInterpolation_t
	{
	public:
		bool m_bEyePositionIsInterpolating; // +0x0  // line 300
		Vector m_vEyePosition_Interpolated; // +0x4  // line 301
		Vector m_vEyePosition_Uninterpolated; // +0x10  // line 302
		int m_iTickLastUpdated; // +0x1c  // line 306
		float m_fTickInterpolationAmountLastUpdated; // +0x20  // line 307
		bool m_bDisableFreeMovement; // +0x24  // line 308
		bool m_bUpdatePosition_FreeMove; // +0x25  // line 309
		PortalEyeInterpolation_t();  // line 311
	};  // line 299
private:
	C_Portal_Player::PortalEyeInterpolation_t PortalEyeInterpolation; // +0x1e28  // line 312
public:
	// game/client/portal/c_portal_player.h:315 sizeof=0x14 (i386)
	struct PreDataChanged_Backup_t
	{
	public:
		CHandle<C_Portal_Base2D> m_hPortalEnvironment; // +0x0  // line 316
		CHandle<C_Func_LiquidPortal> m_hSurroundingLiquidPortal; // +0x4  // line 317
		QAngle m_qEyeAngles; // +0x8  // line 319
	};  // line 315
private:
	C_Portal_Player::PreDataChanged_Backup_t PreDataChanged_Backup; // +0x1e50  // line 320
	bool m_bPortalledMessagePending; // +0x1e64  // line 322
	VMatrix m_PendingPortalMatrix; // +0x1e68  // line 323
	bool m_bIsHoldingSomething; // +0x1ea8  // line 325
	bool m_bWasTaunting; // +0x1ea9  // line 327
	bool m_bGibbed; // +0x1eaa  // line 328
	CameraThirdData_t m_TauntCameraData; // +0x1eac  // line 329
	bool m_bTauntRemoteView; // +0x1ed4  // line 330
	Vector m_vecRemoteViewOrigin; // +0x1ed8  // line 331
	QAngle m_vecRemoteViewAngles; // +0x1ee4  // line 332
	float m_fTauntCameraDistance; // +0x1ef0  // line 333
	float m_fTeamTauntStartTime; // +0x1ef4  // line 335
	int m_nOldTeamTauntState; // +0x1ef8  // line 336
	int m_nTeamTauntState; // +0x1efc  // line 337
	Vector m_vTauntPosition; // +0x1f00  // line 338
	QAngle m_vTauntAngles; // +0x1f0c  // line 339
	QAngle m_angTauntPredViewAngles; // +0x1f18  // line 341
	QAngle m_angTauntEngViewAngles; // +0x1f24  // line 342
	int m_nLastFrameDrawn; // +0x1f30  // line 344
	int m_nLastDrawnStudioFlags; // +0x1f34  // line 345
	float m_flUseKeyStartTime; // +0x1f38  // line 347
	int m_nUseKeyEntFoundCommandNum; // +0x1f3c  // line 348
	int m_nUseKeyEntClearCommandNum; // +0x1f40  // line 349
	int m_nLastRecivedCommandNum; // +0x1f44  // line 350
	EHANDLE m_hUseEntToSend; // +0x1f48  // line 351
	bool m_bForcingDrop; // +0x1f4c  // line 353
	bool m_bUseVMGrab; // +0x1f4d  // line 354
	bool m_bUsingVMGrabState; // +0x1f4e  // line 355
	EHANDLE m_hAttachedObject; // +0x1f50  // line 357
	EHANDLE m_hOldAttachedObject; // +0x1f54  // line 358
	CGrabController m_GrabController; // +0x1f58  // line 360
public:
	QAngle m_vecCarriedObjectAngles; // +0x2018  // line 363
	C_PlayerHeldObjectClone *m_pHeldEntityClone; // +0x2024  // line 364
private:
	EHANDLE m_hUseEntThroughPortal; // +0x2028  // line 367
	bool m_bUseWasDown; // +0x202c  // line 368
	void PollForUseEntity( CUserCmd * );  // line 369
	float m_flImplicitVerticalStepSpeed; // +0x2030  // line 371
	Vector m_vRenderOrigin; // +0x2034  // line 375
	Vector m_vTempRenderOrigin; // +0x2040  // line 377
	QAngle m_TempRenderAngles; // +0x204c  // line 378
	bool m_iSpawnCounter; // +0x2058  // line 380
	bool m_iOldSpawnCounter; // +0x2059  // line 381
	float m_fLatestServerTeleport; // +0x205c  // line 383
public:
	static bool RenderLocalScreenSpaceEffect( PortalScreenSpaceEffect, IMatRenderContext *, int, int, int, int );  // line 386
	virtual void SharedSpawn();  // line 388
	virtual void Touch( C_BaseEntity * );  // line 389
	virtual Vector Weapon_ShootPosition();  // line 391
	virtual C_BaseCombatWeapon *Weapon_OwnsThisType( const char *, int ) const;  // line 393
	virtual void SelectItem( const char *, int );  // line 394
	bool IsPressingJumpKey() const;  // line 396
	bool IsHoldingJumpKey() const;  // line 397
	bool IsTryingToSuperJump( const PaintPowerInfo_t * ) const;  // line 398
	void SetJumpedThisFrame( bool );  // line 399
	bool JumpedThisFrame() const;  // line 400
	InAirState GetInAirState() const;  // line 401
	bool WantsToSwapGuns();  // line 403
	bool IsUsingPostTeleportationBox() const;  // line 405
	const Vector &GetInputVector() const;  // line 407
	void SetInputVector( const Vector & );  // line 408
	const Vector &GetPrevGroundNormal() const;  // line 410
	void SetPrevGroundNormal( const Vector & );  // line 411
	Vector GetPaintGunShootPosition();  // line 413
	EHANDLE GetAttachedObject();  // line 415
	virtual PaintPowerType GetPaintPowerAtPoint( const Vector & ) const;  // line 417
	virtual void Paint( PaintPowerType, const Vector & );  // line 418
	virtual void CleansePaint();  // line 419
	virtual bool RenderScreenSpaceEffect( PortalScreenSpaceEffect, IMatRenderContext *, int, int, int, int );  // line 421
	bool ScreenSpacePaintEffectIsActive() const;  // line 423
	void SetScreenSpacePaintEffectColors( IMaterialVar *, IMaterialVar * ) const;  // line 424
	void Reorient( QAngle & );  // line 426
	float GetReorientationProgress() const;  // line 427
	bool IsDoneReorienting() const;  // line 428
	virtual void UpdateCollisionBounds();  // line 430
	virtual const Vector GetPlayerMins() const;  // line 432
	virtual const Vector GetPlayerMaxs() const;  // line 433
	const Vector &GetHullMins() const;  // line 434
	const Vector &GetHullMaxs() const;  // line 435
	const Vector &GetStandHullMins() const;  // line 436
	const Vector &GetStandHullMaxs() const;  // line 437
	const Vector &GetDuckHullMins() const;  // line 438
	const Vector &GetDuckHullMaxs() const;  // line 439
	float GetHullHeight() const;  // line 441
	float GetHullWidth() const;  // line 442
	float GetStandHullHeight() const;  // line 443
	float GetStandHullWidth() const;  // line 444
	float GetDuckHullHeight() const;  // line 445
	float GetDuckHullWidth() const;  // line 446
	void SetAirDuck( bool );  // line 448
	void UnDuck();  // line 449
	StickCameraState GetStickCameraState() const;  // line 451
	void SetQuaternionPunch( const Quaternion & );  // line 453
	void DecayQuaternionPunch();  // line 454
	void AddSurfacePaintPowerInfo( const BrushContact &, const char * );  // line 457
	void AddSurfacePaintPowerInfo( const trace_t &, const char * );  // line 458
	void SetEyeUpOffset( const Vector &, const Vector & );  // line 460
	void SetEyeOffset( const Vector &, const Vector & );  // line 461
	bool IsInTeamTauntIdle();  // line 463
	void SetHullHeight( float );  // line 465
	virtual void GetToolRecordingState( KeyValues * );  // line 467
	float GetAirTime();  // line 469
protected:
	virtual void ChooseActivePaintPowers( CUtlVector<PaintPowerInfo_t,CUtlMemory<PaintPowerInfo_t, int> > & );  // line 472
private:
	void DecayEyeOffset();  // line 475
	void DeterminePaintContacts();  // line 478
	void PredictPaintContacts( const Vector &, const Vector &, const Vector &, const Vector &, float, const char * );  // line 484
	void ChooseBestPaintPowersInRange( PaintPowerChoiceResultArray &, PaintPowerConstIter, PaintPowerConstIter, const PaintPowerChoiceCriteria_t & ) const;  // line 488
	virtual PaintPowerState ActivateSpeedPower( PaintPowerInfo_t & );  // line 491
	virtual PaintPowerState UseSpeedPower( PaintPowerInfo_t & );  // line 492
	virtual PaintPowerState DeactivateSpeedPower( PaintPowerInfo_t & );  // line 493
	virtual PaintPowerState ActivateBouncePower( PaintPowerInfo_t & );  // line 495
	virtual PaintPowerState UseBouncePower( PaintPowerInfo_t & );  // line 496
	virtual PaintPowerState DeactivateBouncePower( PaintPowerInfo_t & );  // line 497
	void PlayPaintSounds( const PaintPowerChoiceResultArray & );  // line 499
	void UpdatePaintedPower();  // line 500
	void UpdateAirInputScaleFadeIn();  // line 501
	void UpdateInAirState();  // line 502
	void CopyPaintPowerChoiceInfoToHudState( const PaintPowerChoiceResultArray & );  // line 503
	bool LateSuperJumpIsValid() const;  // line 504
	void RecomputeBoundsForOrientation();  // line 505
	void TryToChangeCollisionBounds( const Vector &, const Vector &, const Vector &, const Vector & );  // line 509
	float SpeedPaintAcceleration( float, float, float, float ) const;  // line 514
	bool RenderScreenSpacePaintEffect( IMatRenderContext * );  // line 516
	void InvalidatePaintEffects();  // line 517
	bool CheckToUseBouncePower( PaintPowerInfo_t & );  // line 519
	void UpdateNotSoStickyStick( PaintPowerInfo_t & );  // line 522
	void RotateUpVector( Vector &, Vector & );  // line 525
	void StartAutoRotate( StickCameraState, bool );  // line 526
	void CheckStickCameraState( const Vector &, const Vector &, Vector & );  // line 527
	void PostTeleportationStickCamFixup( const C_Portal_Base2D *, CMoveData * );  // line 528
	void SetOffTheWallProgress( float, Vector &, const Vector & );  // line 529
	void DrawJumpHelperDebug( PaintPowerConstIter, PaintPowerConstIter, float, bool, const PaintPowerInfo_t * ) const;  // line 532
	void ManageHeldObject();  // line 533
	PaintPowerInfo_t m_CachedJumpPower; // +0x2060  // line 536
	CUtlReference<CNewParticleEffect> m_PaintScreenSpaceEffect; // +0x208c  // line 537
	CUtlReference<CNewParticleEffect> m_PaintDripEffect; // +0x2098  // line 538
	CountdownTimer m_PaintScreenEffectCooldownTimer; // +0x20a4  // line 539
	Vector m_vInputVector; // +0x20b0  // line 540
	float m_flCachedJumpPowerTime; // +0x20bc  // line 541
	float m_flUsePostTeleportationBoxTime; // +0x20c0  // line 542
	float m_flSpeedDecelerationTime; // +0x20c4  // line 543
	float m_flPredictedJumpTime; // +0x20c8  // line 544
	bool m_bDoneStickInterp; // +0x20cc  // line 545
	bool m_bDoneCorrectPitch; // +0x20cd  // line 546
	bool m_bJumpWasPressedWhenForced; // +0x20ce  // line 547
	float m_flTimeSinceLastTouchedPower[3]; // +0x20d0  // line 549
	float m_flTimeLastTouchedGround; // +0x20dc  // line 551
	bool m_bDoneAirTauntHint; // +0x20e0  // line 552
	bool m_bWantsToSwapGuns; // +0x20e1  // line 554
	Vector m_vPrevGroundNormal; // +0x20e4  // line 556
	bool m_bToolMode_EyeHasPortalled_LastRecord; // +0x20f0  // line 558
public:
	CPortalPlayerShared m_Shared; // +0x20f4  // line 561
	void CreatePingPointer( Vector );  // line 564
	void DestroyPingPointer();  // line 565
	CUtlReference<CNewParticleEffect> m_FlingTrailEffect; // +0x2118  // line 566
	CUtlReference<CNewParticleEffect> m_PointLaser; // +0x2124  // line 567
	bool m_bFlingTrailActive; // +0x2130  // line 568
	bool m_bFlingTrailJustPortalled; // +0x2131  // line 569
	bool m_bFlingTrailPrePortalled; // +0x2132  // line 570
};
