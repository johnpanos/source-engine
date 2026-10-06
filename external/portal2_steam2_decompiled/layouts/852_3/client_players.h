// Class layouts from Steam2 852_3 client
// Unit game/client/c_baseplayer.cpp; i386 offsets; reconstruction aid.

// game/client/c_baseplayer.h:71
// game/client/c_baseplayer.h:71 sizeof=0x1a54 (i386)
struct C_BasePlayer : public C_BaseCombatCharacter
{
public:
	static typedescription_t m_PredDesc[];  // line 75
	C_BasePlayer();  // line 78
	virtual ~C_BasePlayer();  // line 79
	virtual void Spawn();  // line 81
	virtual void SharedSpawn();  // line 82
	virtual void UpdateOnRemove();  // line 83
	virtual Class_T Classify();  // line 84
	virtual void OnPreDataChanged( DataUpdateType_t );  // line 87
	virtual void OnDataChanged( DataUpdateType_t );  // line 88
	virtual void PreDataUpdate( DataUpdateType_t );  // line 90
	virtual void PostDataUpdate( DataUpdateType_t );  // line 91
	virtual void ReceiveMessage( int, bf_read & );  // line 93
	virtual void OnRestore();  // line 95
	virtual void MakeTracer( const Vector &, const trace_t &, int );  // line 97
	virtual void GetToolRecordingState( KeyValues * );  // line 99
	bool ClearUseEntity();  // line 102
	void SetAnimationExtension( const char * );  // line 105
	C_BaseViewModel *GetViewModel( int );  // line 107
	virtual C_BaseCombatWeapon *GetActiveWeapon() const;  // line 108
	virtual const char *GetTracerType();  // line 109
	virtual void CalcView( Vector &, QAngle &, float &, float &, float & );  // line 112
	virtual void CalcViewModelView( const Vector &, const QAngle & );  // line 113
	void SmoothViewOnStairs( Vector & );  // line 117
	virtual float CalcRoll( const QAngle &, const Vector &, float, float );  // line 118
	void CalcViewRoll( QAngle & );  // line 119
	virtual void CalcViewBob( Vector & );  // line 120
	void CreateWaterEffects();  // line 121
	virtual void SetPlayerUnderwater( bool );  // line 123
	void UpdateUnderwaterState();  // line 124
	bool IsPlayerUnderwater();  // line 125
	virtual C_BaseCombatCharacter *ActivePlayerCombatCharacter();  // line 127
	virtual Vector Weapon_ShootPosition();  // line 129
	virtual bool Weapon_CanUse( C_BaseCombatWeapon * );  // line 130
	virtual void Weapon_DropPrimary();  // line 131
	virtual Vector GetAutoaimVector( float );  // line 133
	void SetSuitUpdate( char *, int, int );  // line 134
	virtual bool CreateMove( float, CUserCmd * );  // line 137
	virtual void AvoidPhysicsProps( CUserCmd * );  // line 138
	virtual void PlayerUse();  // line 140
	C_BaseEntity *FindUseEntity();  // line 141
	virtual bool IsUseableEntity( C_BaseEntity *, unsigned int );  // line 142
	virtual bool CanPickupObject( C_BaseEntity *, float, float );  // line 145
	virtual float GetHeldObjectMass( IPhysicsObject * );  // line 146
	virtual void ForceDropOfCarriedPhysObjects();  // line 147
	virtual bool IsPlayer() const;  // line 151
	virtual int GetHealth() const;  // line 152
	int GetBonusProgress() const;  // line 154
	int GetBonusChallenge() const;  // line 155
	virtual int GetObserverMode() const;  // line 158
	virtual C_BaseEntity *GetObserverTarget() const;  // line 159
	void SetObserverTarget( EHANDLE );  // line 160
	bool AudioStateIsUnderwater( Vector );  // line 162
	bool IsObserver() const;  // line 164
	bool IsHLTV() const;  // line 165
	void ResetObserverMode();  // line 169
	bool IsBot() const;  // line 170
	virtual Vector EyePosition();  // line 173
	virtual const QAngle &EyeAngles();  // line 174
	void EyePositionAndVectors( Vector *, Vector *, Vector *, Vector * );  // line 175
	virtual const QAngle &LocalEyeAngles();  // line 176
	virtual IRagdoll *GetRepresentativeRagdoll() const;  // line 180
	virtual void GetRagdollInitBoneArrays( matrix3x4a_t *, matrix3x4a_t *, matrix3x4a_t *, float );  // line 183
	void EyeVectors( Vector *, Vector *, Vector * );  // line 186
	void CacheVehicleView();  // line 187
	bool IsSuitEquipped();  // line 190
	virtual void TeamChange( int );  // line 193
	void Flashlight();  // line 196
	void UpdateFlashlight();  // line 197
	void TurnOffFlashlight();  // line 198
	virtual const char *GetFlashlightTextureName() const;  // line 199
	virtual float GetFlashlightFOV() const;  // line 200
	virtual float GetFlashlightFarZ() const;  // line 201
	virtual float GetFlashlightLinearAtten() const;  // line 202
	virtual bool CastsFlashlightShadows() const;  // line 203
	virtual void GetFlashlightOffset( const Vector &, const Vector &, const Vector &, Vector * ) const;  // line 204
	Vector m_vecFlashlightOrigin; // +0x12b0  // line 205
	Vector m_vecFlashlightForward; // +0x12bc  // line 206
	Vector m_vecFlashlightUp; // +0x12c8  // line 207
	Vector m_vecFlashlightRight; // +0x12d4  // line 208
	virtual bool IsAllowedToSwitchWeapons();  // line 211
	virtual C_BaseCombatWeapon *GetActiveWeaponForSelection();  // line 212
	virtual C_BaseAnimating *GetRenderedWeaponModel();  // line 217
	virtual bool IsOverridingViewmodel();  // line 219
	virtual int DrawOverriddenViewmodel( C_BaseViewModel *, int, const RenderableInstance_t & );  // line 220
	virtual float GetDefaultAnimSpeed();  // line 222
	void SetMaxSpeed( float );  // line 224
	float MaxSpeed() const;  // line 225
	virtual ShadowType_t ShadowCastType();  // line 228
	virtual bool ShouldReceiveProjectedTextures( int );  // line 230
	void CheckForLocalPlayer( int );  // line 237
	static bool IsLocalPlayer( const C_BaseEntity * );  // line 240
	bool IsLocalPlayer() const;  // line 242
	virtual void ThirdPersonSwitch( bool );  // line 245
	bool ShouldDrawLocalPlayer();  // line 246
	static C_BasePlayer *GetLocalPlayer( int );  // line 247
	static void SetRemoteSplitScreenPlayerViewsAreLocalPlayer( bool );  // line 248
	static bool HasAnyLocalPlayer();  // line 249
	static int GetSplitScreenSlotForPlayer( C_BaseEntity * );  // line 250
	void AddSplitScreenPlayer( C_BasePlayer * );  // line 252
	void RemoveSplitScreenPlayer( C_BasePlayer * );  // line 253
	CUtlVector<CHandle<C_BasePlayer>,CUtlMemory<CHandle<C_BasePlayer>, int> > &GetSplitScreenPlayers();  // line 254
	bool IsSplitScreenPartner( C_BasePlayer * );  // line 256
	bool IsSplitScreenPlayer() const;  // line 258
	int GetSplitScreenPlayerSlot();  // line 259
	virtual IClientModelRenderable *GetClientModelRenderable();  // line 261
	virtual bool PreRender( int );  // line 262
	int GetUserID() const;  // line 264
	virtual bool CanSetSoundMixer();  // line 265
	virtual C_BaseEntity *GetSoundscapeListener();  // line 268
	void AddToPlayerSimulationList( C_BaseEntity * );  // line 272
	void SimulatePlayerSimulatedEntities();  // line 273
	void RemoveFromPlayerSimulationList( C_BaseEntity * );  // line 274
	void ClearPlayerSimulationList();  // line 275
	virtual void PhysicsSimulate();  // line 278
	virtual void VPhysicsShadowUpdate( IPhysicsObject * );  // line 279
	virtual bool IsFollowingPhysics();  // line 280
	bool IsRideablePhysics( IPhysicsObject * );  // line 281
	IPhysicsObject *GetGroundVPhysics();  // line 282
	void UpdatePhysicsShadowToCurrentPosition();  // line 283
	void UpdateVPhysicsPosition( const Vector &, const Vector &, float );  // line 284
	void UpdatePhysicsShadowToPosition( const Vector & );  // line 285
	void PostThinkVPhysics();  // line 286
	void SetTouchedPhysics( bool );  // line 287
	bool TouchedPhysics();  // line 288
	void SetPhysicsFlag( int, bool );  // line 289
	bool HasPhysicsFlag( unsigned int );  // line 290
	void SetVCollisionState( const Vector &, const Vector &, int );  // line 291
	virtual unsigned int PhysicsSolidMaskForEntity() const;  // line 292
	virtual C_BasePlayer *GetPredictionOwner();  // line 296
	virtual void PreThink();  // line 298
	virtual void PostThink();  // line 299
	virtual void ItemPreFrame();  // line 301
	virtual void ItemPostFrame();  // line 302
	virtual void AbortReload();  // line 303
	virtual void SelectLastItem();  // line 305
	virtual void Weapon_SetLast( C_BaseCombatWeapon * );  // line 306
	virtual bool Weapon_ShouldSetLast( C_BaseCombatWeapon *, C_BaseCombatWeapon * );  // line 307
	virtual bool Weapon_ShouldSelectItem( C_BaseCombatWeapon * );  // line 308
	virtual bool Weapon_Switch( C_BaseCombatWeapon *, int );  // line 309
	virtual C_BaseCombatWeapon *GetLastWeapon();  // line 310
	void ResetAutoaim();  // line 311
	virtual void SelectItem( const char *, int );  // line 312
	virtual void UpdateClientData();  // line 314
	virtual float GetFOV() const;  // line 316
	virtual int GetDefaultFOV() const;  // line 317
	virtual bool IsZoomed();  // line 318
	bool SetFOV( C_BaseEntity *, int, float, int );  // line 319
	void ClearZoomOwner();  // line 320
	float GetFOVDistanceAdjustFactor();  // line 322
	virtual void ViewPunch( const QAngle & );  // line 324
	void ViewPunchReset( float );  // line 325
	void UpdateButtonState( int );  // line 327
	int GetImpulse() const;  // line 328
	virtual bool Simulate();  // line 330
	virtual bool ShouldInterpolate();  // line 332
	virtual bool ShouldDraw();  // line 334
	virtual int DrawModel( int, const RenderableInstance_t & );  // line 335
	virtual const char *GetPlayerModelName();  // line 336
	virtual void OverrideView( CViewSetup * );  // line 339
	C_BaseEntity *GetViewEntity() const;  // line 341
	const char *GetPlayerName();  // line 344
	virtual const Vector GetPlayerMins() const;  // line 345
	virtual const Vector GetPlayerMaxs() const;  // line 346
	virtual void UpdateCollisionBounds();  // line 348
	bool IsPlayerDead();  // line 351
	bool IsPoisoned();  // line 352
	virtual C_BaseEntity *GetUseEntity() const;  // line 354
	virtual C_BaseEntity *GetPotentialUseEntity() const;  // line 355
	IClientVehicle *GetVehicle();  // line 358
	const IClientVehicle *GetVehicle() const;  // line 359
	bool IsInAVehicle() const;  // line 361
	virtual void SetVehicleRole( int );  // line 362
	void LeaveVehicle();  // line 363
	bool UsingStandardWeaponsInVehicle();  // line 365
	virtual void SetAnimation( PLAYER_ANIM );  // line 367
	float GetTimeBase() const;  // line 369
	float GetFinalPredictedTime() const;  // line 370
	bool IsInVGuiInputMode() const;  // line 372
	bool IsInViewModelVGuiInputMode() const;  // line 373
	C_CommandContext *GetCommandContext();  // line 375
	int CurrentCommandNumber() const;  // line 378
	const CUserCmd *GetCurrentUserCommand() const;  // line 379
	const CUserCmd *GetLastUserCommand();  // line 380
	virtual const QAngle &GetPunchAngle();  // line 382
	void SetPunchAngle( const QAngle & );  // line 383
	float GetWaterJumpTime() const;  // line 385
	void SetWaterJumpTime( float );  // line 386
	float GetSwimSoundTime() const;  // line 387
	void SetSwimSoundTime( float );  // line 388
	float GetDeathTime();  // line 390
	void SetPreviouslyPredictedOrigin( const Vector & );  // line 392
	const Vector &GetPreviouslyPredictedOrigin() const;  // line 393
	virtual float GetMinFOV() const;  // line 396
	virtual void DoMuzzleFlash();  // line 398
	virtual void PlayPlayerJingle();  // line 399
	virtual void UpdateStepSound( surfacedata_t *, const Vector &, const Vector & );  // line 401
	virtual void PlayStepSound( Vector &, surfacedata_t *, float, bool );  // line 402
	virtual surfacedata_t *GetFootstepSurface( const Vector &, const char * );  // line 403
	virtual void GetStepSoundVelocities( float *, float * );  // line 404
	virtual void SetStepSoundTime( stepsoundtimes_t, bool );  // line 405
	void NotePredictionError( const Vector & );  // line 410
	void GetPredictionErrorSmoothingVector( Vector & );  // line 413
	virtual void ExitLadder();  // line 415
	surfacedata_t *GetLadderSurface( const Vector & );  // line 416
	void ForceButtons( int );  // line 418
	void UnforceButtons( int );  // line 419
	void SetLadderNormal( Vector );  // line 422
	const Vector &GetLadderNormal() const;  // line 423
	int GetLadderSurfaceProps() const;  // line 424
	virtual CHintSystem *Hints();  // line 427
	bool ShouldShowHints();  // line 428
	bool HintMessage( int, bool, bool );  // line 429
	void HintMessage( const char * );  // line 430
	virtual IMaterial *GetHeadLabelMaterial();  // line 432
	virtual fogparams_t *GetFogParams();  // line 435
	void FogControllerChanged( bool );  // line 436
	void UpdateFogController();  // line 437
	void UpdateFogBlend();  // line 438
	C_PostProcessController *GetActivePostProcessController() const;  // line 440
	C_ColorCorrection *GetActiveColorCorrection() const;  // line 441
	void IncrementEFNoInterpParity();  // line 443
	int GetEFNoInterpParity() const;  // line 444
	float GetFOVTime();  // line 446
	PlayerRenderMode_t GetPlayerRenderMode( int );  // line 448
	virtual void OnAchievementAchieved( int );  // line 450
protected:
	fogparams_t m_CurrentFog; // +0x12e0  // line 453
	EHANDLE m_hOldFogController; // +0x132c  // line 454
public:
	static void RecvProxy_LocalVelocityX( const CRecvProxyData *, void *, void * );  // line 458
	static void RecvProxy_LocalVelocityY( const CRecvProxyData *, void *, void * );  // line 459
	static void RecvProxy_LocalVelocityZ( const CRecvProxyData *, void *, void * );  // line 460
	static void RecvProxy_ObserverTarget( const CRecvProxyData *, void *, void * );  // line 462
	static void RecvProxy_ObserverMode( const CRecvProxyData *, void *, void * );  // line 463
	static void RecvProxy_LocalOriginXY( const CRecvProxyData *, void *, void * );  // line 465
	static void RecvProxy_LocalOriginZ( const CRecvProxyData *, void *, void * );  // line 466
	static void RecvProxy_NonLocalOriginXY( const CRecvProxyData *, void *, void * );  // line 467
	static void RecvProxy_NonLocalOriginZ( const CRecvProxyData *, void *, void * );  // line 468
	static void RecvProxy_NonLocalCellOriginXY( const CRecvProxyData *, void *, void * );  // line 469
	static void RecvProxy_NonLocalCellOriginZ( const CRecvProxyData *, void *, void * );  // line 470
	virtual bool ShouldRegenerateOriginFromCellBits() const;  // line 472
	void SetUseEntity( C_BaseEntity * );  // line 474
	int m_StuckLast; // +0x1330  // line 477
	C_BasePlayer::NetworkVar_m_Local m_Local; // +0x1334  // line 480
	static int GetOffset_m_Local();  // line 480
	EHANDLE m_hTonemapController; // +0x14f8  // line 482
	CPlayerState pl; // +0x14fc  // line 485
	int m_iFOV; // +0x1510  // line 492
	int m_iFOVStart; // +0x1514  // line 493
	int m_afButtonLast; // +0x1518  // line 494
	int m_afButtonPressed; // +0x151c  // line 495
	int m_afButtonReleased; // +0x1520  // line 496
	int m_nButtons; // +0x1524  // line 497
protected:
	int m_nImpulse; // +0x1528  // line 499
	CNetworkVar( int, m_ladderSurfaceProps ); // +0x152c  // line 500
	int m_flPhysics; // +0x1530  // line 501
public:
	float m_flFOVTime; // +0x1534  // line 503
private:
	float m_flWaterJumpTime; // +0x1538  // line 505
	float m_flSwimSoundTime; // +0x153c  // line 506
protected:
	float m_flStepSoundTime; // +0x1540  // line 508
	float m_surfaceFriction; // +0x1544  // line 509
private:
	CNetworkVector( Vector, m_vecLadderNormal ); // +0x1548  // line 511
public:
	char m_szAnimExtension[32]; // +0x1554  // line 515
private:
	int m_nOldTickBase; // +0x1574  // line 517
	int m_iBonusProgress; // +0x1578  // line 519
	int m_iBonusChallenge; // +0x157c  // line 520
	float m_flMaxspeed; // +0x1580  // line 523
public:
	EHANDLE m_hZoomOwner; // +0x1584  // line 527
protected:
	IPhysicsPlayerController *m_pPhysicsController; // +0x1588  // line 530
	IPhysicsObject *m_pShadowStand; // +0x158c  // line 531
	IPhysicsObject *m_pShadowCrouch; // +0x1590  // line 532
	int m_vphysicsCollisionState; // +0x1594  // line 533
	Vector m_oldOrigin; // +0x1598  // line 534
	bool m_touchedPhysObject; // +0x15a4  // line 535
	bool m_bPhysicsWasFrozen; // +0x15a5  // line 536
	Vector m_vNewVPhysicsPosition; // +0x15a8  // line 537
	Vector m_vNewVPhysicsVelocity; // +0x15b4  // line 538
	CUserCmd m_LastCmd; // +0x15c0  // line 539
	unsigned int m_afPhysicsFlags; // +0x161c  // line 541
	EHANDLE m_hVehicle; // +0x1620  // line 542
	CHandle<C_BaseCombatWeapon> m_hLastWeapon; // +0x1624  // line 544
	CHandle<C_BaseViewModel> m_hViewModel[2]; // +0x1628  // line 546
public:
	bool m_fOnTarget; // +0x1630  // line 550
	EHANDLE m_hUseEntity; // +0x1634  // line 553
	int m_iDefaultFOV; // +0x1638  // line 559
	int m_afButtonForced; // +0x163c  // line 561
	CUserCmd *m_pCurrentCommand; // +0x1640  // line 564
	EHANDLE m_hViewEntity; // +0x1644  // line 566
	EHANDLE m_hConstraintEntity; // +0x1648  // line 569
	Vector m_vecConstraintCenter; // +0x164c  // line 570
	float m_flConstraintRadius; // +0x1658  // line 571
	float m_flConstraintWidth; // +0x165c  // line 572
	float m_flConstraintSpeedFactor; // +0x1660  // line 573
	bool m_bConstraintPastRadius; // +0x1664  // line 574
protected:
	virtual void CalcPlayerView( Vector &, QAngle &, float & );  // line 578
	void CalcVehicleView( IClientVehicle *, Vector &, QAngle &, float &, float &, float & );  // line 580
	virtual void CalcObserverView( Vector &, QAngle &, float & );  // line 581
	virtual Vector GetChaseCamViewOffset( C_BaseEntity * );  // line 582
	void CalcChaseCamView( Vector &, QAngle &, float & );  // line 583
	void CalcInEyeCamView( Vector &, QAngle &, float & );  // line 584
	virtual void CalcDeathCamView( Vector &, QAngle &, float & );  // line 585
	virtual void CalcRoamingView( Vector &, QAngle &, float & );  // line 586
	virtual void CalcFreezeCamView( Vector &, QAngle &, float & );  // line 587
	void DetermineVguiInputMode( CUserCmd * );  // line 590
	virtual void SetLocalViewAngles( const QAngle & );  // line 593
	virtual void SetViewAngles( const QAngle & );  // line 594
	surfacedata_t *GetGroundSurface();  // line 597
	bool JustEnteredVehicle();  // line 601
	int m_iObserverMode; // +0x1668  // line 604
	EHANDLE m_hObserverTarget; // +0x166c  // line 605
	float m_flObserverChaseDistance; // +0x1670  // line 606
	Vector m_vecFreezeFrameStart; // +0x1674  // line 607
	float m_flFreezeFrameStartTime; // +0x1680  // line 608
	float m_flFreezeFrameDistance; // +0x1684  // line 609
	bool m_bWasFreezeFraming; // +0x1688  // line 610
	float m_flDeathTime; // +0x168c  // line 611
private:
	C_BasePlayer &operator=( const C_BasePlayer & );  // line 616
	C_BasePlayer( const C_BasePlayer & );  // line 617
	EHANDLE m_hOldVehicle; // +0x1690  // line 620
	CInterpolatedVar<Vector> m_iv_vecViewOffset; // +0x1694  // line 622
	Vector m_vecWaterJumpVel; // +0x16c0  // line 625
protected:
	QAngle m_vecOldViewAngles; // +0x16cc  // line 629
private:
	bool m_bWasFrozen; // +0x16d8  // line 632
	int m_nTickBase; // +0x16dc  // line 634
	int m_nFinalPredictedTick; // +0x16e0  // line 635
	EHANDLE m_pCurrentVguiScreen; // +0x16e4  // line 637
	bool m_bFlashlightEnabled[2]; // +0x16e8  // line 641
	CUtlVector<CHandle<C_BaseEntity>,CUtlMemory<CHandle<C_BaseEntity>, int> > m_SimulatedByThisPlayer; // +0x16ec  // line 644
	float m_flOldPlayerZ; // +0x1700  // line 647
	float m_flOldPlayerViewOffsetZ; // +0x1704  // line 648
	Vector m_vecVehicleViewOrigin; // +0x1708  // line 650
	QAngle m_vecVehicleViewAngles; // +0x1714  // line 651
	float m_flVehicleViewFOV; // +0x1720  // line 652
	int m_nVehicleViewSavedFrame; // +0x1724  // line 653
	int m_iOldAmmo[32]; // +0x1728  // line 656
	C_CommandContext m_CommandContext; // +0x17a8  // line 658
	float m_flWaterSurfaceZ; // +0x180c  // line 661
	bool m_bResampleWaterSurface; // +0x1810  // line 662
	TimedEvent m_tWaterParticleTimer; // +0x1814  // line 663
	CSmartPtr<WaterDebrisEffect,CRefCountAccessor> m_pWaterEmitter; // +0x181c  // line 664
	bool m_bPlayerUnderwater; // +0x1820  // line 666
	float GetStepSize() const;  // line 682
	float m_flNextAvoidanceTime; // +0x1824  // line 684
	float m_flAvoidanceRight; // +0x1828  // line 685
	float m_flAvoidanceForward; // +0x182c  // line 686
	float m_flAvoidanceDotForward; // +0x1830  // line 687
	float m_flAvoidanceDotRight; // +0x1834  // line 688
protected:
	virtual bool IsDucked() const;  // line 691
	virtual bool IsDucking() const;  // line 692
	virtual float GetFallVelocity();  // line 693
	void ForceSetupBonesAtTimeFakeInterpolation( matrix3x4a_t *, float );  // line 694
	float m_flLaggedMovementValue; // +0x1838  // line 696
	Vector m_vecPredictionError; // +0x183c  // line 701
	float m_flPredictionErrorTime; // +0x1848  // line 702
	Vector m_vecPreviouslyPredictedOrigin; // +0x184c  // line 704
	char m_szLastPlaceName[18]; // +0x1858  // line 706
	int m_surfaceProps; // +0x186c  // line 709
	surfacedata_t *m_pSurfaceData; // +0x1870  // line 710
	char m_chTextureType; // +0x1874  // line 711
	bool m_bSentFreezeFrame; // +0x1875  // line 713
	float m_flFreezeZOffset; // +0x1878  // line 714
	uint8 m_ubEFNoInterpParity; // +0x187c  // line 715
	uint8 m_ubOldEFNoInterpParity; // +0x187d  // line 716
	CUtlVector<CHandle<C_BasePlayer>,CUtlMemory<CHandle<C_BasePlayer>, int> > m_hSplitScreenPlayers; // +0x1880  // line 719
	int m_nSplitScreenSlot; // +0x1894  // line 720
	CHandle<C_BasePlayer> m_hSplitOwner; // +0x1898  // line 721
	bool m_bIsLocalPlayer; // +0x189c  // line 722
public:
	// game/client/c_baseplayer.h:727 sizeof=0xb8 (i386)
	struct StepSoundCache_t
	{
	public:
		StepSoundCache_t();  // line 728
		CSoundParameters m_SoundParameters; // +0x0  // line 729
		short unsigned int m_usSoundNameIndex; // +0xb4  // line 730
	};  // line 727
private:
	C_BasePlayer::StepSoundCache_t m_StepSoundCache[2]; // +0x18a0  // line 733
public:
	const char *GetLastKnownPlaceName() const;  // line 737
	float GetLaggedMovementValue();  // line 739
	bool ShouldGoSouth( Vector, Vector );  // line 740
	void SetOldPlayerZ( float );  // line 742
	const fogplayerparams_t &GetPlayerFog() const;  // line 744
private:
	CNetworkHandle( C_PostProcessController, m_hPostProcessCtrl ); // +0x1a10  // line 749
	CNetworkHandle( C_ColorCorrection, m_hColorCorrectionCtrl ); // +0x1a14  // line 750
	fogplayerparams_t m_PlayerFog; // +0x1a18  // line 753
};

// game/client/cdll_client_int.h:176
// game/client/cdll_client_int.h:176 sizeof=0x20 (i386)
struct CSetActiveSplitScreenPlayerGuard : public CVGuiScreenSizeSplitScreenPlayerGuard
{
public:
	CSetActiveSplitScreenPlayerGuard( const char *, int, int, int, bool );  // line 178
	CSetActiveSplitScreenPlayerGuard( const char *, int, C_BaseEntity *, int, bool );  // line 179
	~CSetActiveSplitScreenPlayerGuard();  // line 180
private:
	bool m_bChanged; // +0xc  // line 182
	const char *m_pchContext; // +0x10  // line 183
	int m_nLine; // +0x14  // line 184
	int m_nSaveSlot; // +0x18  // line 185
	bool m_bSaveGetLocalPlayerAllowed; // +0x1c  // line 186
};

// game/shared/usercmd.h:65
// game/shared/usercmd.h:65 sizeof=0x5c (i386)
struct CUserCmd
{
public:
	int (**_vptr$CUserCmd)(); // +0x0  // line 0
	CUserCmd();  // line 67
	virtual ~CUserCmd();  // line 72
	void Reset();  // line 74
	CUserCmd &operator=( const CUserCmd & );  // line 129
	CUserCmd( const CUserCmd & );  // line 187
	CRC32_t GetChecksum() const;  // line 192
	int command_number; // +0x4  // line 233
	int tick_count; // +0x8  // line 236
	QAngle viewangles; // +0xc  // line 239
	float forwardmove; // +0x18  // line 242
	float sidemove; // +0x1c  // line 244
	float upmove; // +0x20  // line 246
	int buttons; // +0x24  // line 248
	uint8 impulse; // +0x28  // line 250
	int weaponselect; // +0x2c  // line 252
	int weaponsubtype; // +0x30  // line 253
	int random_seed; // +0x34  // line 255
	short int mousedx; // +0x38  // line 257
	short int mousedy; // +0x3a  // line 258
	bool hasbeenpredicted; // +0x3c  // line 261
	short int player_held_entity; // +0x3e  // line 274
	short int held_entity_was_grabbed_through_portal; // +0x40  // line 278
	QAngle headangles; // +0x44  // line 281
	Vector headoffset; // +0x50  // line 282
};

// CInput: not found as a complete record in this unit

// game/client/c_basecombatcharacter.h:24
// game/client/c_basecombatcharacter.h:24 sizeof=0x12b0 (i386)
struct C_BaseCombatCharacter : public C_BaseFlex
{
public:
	static typedescription_t m_PredDesc[];  // line 28
	C_BaseCombatCharacter();  // line 30
	virtual ~C_BaseCombatCharacter();  // line 31
	virtual bool IsBaseCombatCharacter();  // line 33
	virtual C_BaseCombatCharacter *MyCombatCharacterPointer();  // line 34
	// game/client/c_basecombatcharacter.h:39
	enum FieldOfViewCheckType
	{
		USE_FOV = 0,
		DISREGARD_FOV = 1,
	};  // line 39
	bool IsAbleToSee( const C_BaseEntity *, C_BaseCombatCharacter::FieldOfViewCheckType );  // line 40
	bool IsAbleToSee( C_BaseCombatCharacter *, C_BaseCombatCharacter::FieldOfViewCheckType );  // line 41
	virtual bool IsLookingTowards( const C_BaseEntity *, float ) const;  // line 43
	virtual bool IsLookingTowards( const Vector &, float ) const;  // line 44
	virtual bool IsInFieldOfView( C_BaseEntity * ) const;  // line 46
	virtual bool IsInFieldOfView( const Vector & ) const;  // line 47
	// game/client/c_basecombatcharacter.h:49
	enum LineOfSightCheckType
	{
		IGNORE_NOTHING = 0,
		IGNORE_ACTORS = 1,
	};  // line 49
	virtual bool IsLineOfSightClear( C_BaseEntity *, C_BaseCombatCharacter::LineOfSightCheckType ) const;  // line 54
	virtual bool IsLineOfSightClear( const Vector &, C_BaseCombatCharacter::LineOfSightCheckType, C_BaseEntity * ) const;  // line 55
	void RemoveAmmo( int, int );  // line 60
	void RemoveAmmo( int, const char * );  // line 61
	void RemoveAllAmmo();  // line 62
	int GetAmmoCount( int ) const;  // line 63
	int GetAmmoCount( char * ) const;  // line 64
	virtual C_BaseCombatWeapon *Weapon_OwnsThisType( const char *, int ) const;  // line 66
	virtual int Weapon_GetSlot( const char *, int ) const;  // line 67
	virtual bool Weapon_Switch( C_BaseCombatWeapon *, int );  // line 68
	virtual bool Weapon_CanSwitchTo( C_BaseCombatWeapon * );  // line 69
	bool SwitchToNextBestWeapon( C_BaseCombatWeapon * );  // line 72
	virtual C_BaseCombatWeapon *GetActiveWeapon() const;  // line 74
	int WeaponCount() const;  // line 75
	virtual C_BaseCombatWeapon *GetWeapon( int ) const;  // line 76
	void SetAmmoCount( int, int );  // line 79
	float GetNextAttack() const;  // line 81
	void SetNextAttack( float );  // line 82
	virtual int BloodColor();  // line 84
	void SetBloodColor( int );  // line 87
	virtual void DoMuzzleFlash();  // line 89
	float m_flNextAttack; // +0x1164  // line 94
private:
	bool ComputeLOS( const Vector &, const Vector & ) const;  // line 97
	C_BaseCombatCharacter::NetworkVar_m_iAmmo m_iAmmo; // +0x1168  // line 100
	CHandle<C_BaseCombatWeapon> m_hMyWeapons[48]; // +0x11e8  // line 101
	CHandle<C_BaseCombatWeapon> m_hActiveWeapon; // +0x12a8  // line 102
protected:
	int m_bloodColor; // +0x12ac  // line 108
private:
	C_BaseCombatCharacter( const C_BaseCombatCharacter & );  // line 116
};

// game/client/c_baseentity.h:187
// game/client/c_baseentity.h:187 sizeof=0xa5c (i386)
struct C_BaseEntity : public IClientEntity
{
public:
	static typedescription_t m_PredDesc[];  // line 197
	virtual ScriptClassDesc_t *GetScriptDesc();  // line 199
	C_BaseEntity();  // line 201
protected:
	virtual ~C_BaseEntity();  // line 205
public:
	static C_BaseEntity *CreatePredictedEntityByName( const char *, const char *, int, bool );  // line 208
	static void UpdateVisibilityAllEntities();  // line 209
	virtual void FireBullets( const FireBulletsInfo_t & );  // line 212
	virtual bool ShouldDrawUnderwaterBulletBubbles();  // line 213
	virtual bool ShouldDrawWaterImpacts();  // line 214
	virtual bool HandleShotImpactingWater( const FireBulletsInfo_t &, const Vector &, ITraceFilter *, Vector * );  // line 216
	virtual ITraceFilter *GetBeamTraceFilter();  // line 217
	virtual void DispatchTraceAttack( const CTakeDamageInfo &, const Vector &, trace_t * );  // line 218
	virtual void TraceAttack( const CTakeDamageInfo &, const Vector &, trace_t * );  // line 219
	virtual void DoImpactEffect( trace_t &, int );  // line 220
	virtual void MakeTracer( const Vector &, const trace_t &, int );  // line 221
	virtual int GetTracerAttachment();  // line 222
	void ComputeTracerStartPosition( const Vector &, Vector * );  // line 223
	void TraceBleed( float, const Vector &, trace_t *, int );  // line 224
	virtual int BloodColor();  // line 225
	virtual const char *GetTracerType();  // line 226
	virtual void TakeDamage( const CTakeDamageInfo & );  // line 229
	virtual void Spawn();  // line 231
	virtual void SpawnClientEntity();  // line 232
	virtual void Precache();  // line 233
	virtual void Activate();  // line 234
	void ParseMapData( CEntityMapData * );  // line 236
	virtual void OnParseMapDataFinished();  // line 237
	virtual bool KeyValue( const char *, const char * );  // line 238
	virtual bool KeyValue( const char *, float );  // line 239
	virtual bool KeyValue( const char *, int );  // line 240
	virtual bool KeyValue( const char *, const Vector & );  // line 241
	virtual bool GetKeyValue( const char *, char *, int );  // line 242
	void SetBlocksLOS( bool );  // line 246
	bool BlocksLOS();  // line 247
	void SetAIWalkable( bool );  // line 248
	bool IsAIWalkable();  // line 249
	virtual void InitSharedVars();  // line 252
	void Interp_SetupMappings( VarMapping_t * );  // line 254
	int Interp_Interpolate( VarMapping_t *, float );  // line 257
	void Interp_RestoreToLastNetworked( VarMapping_t * );  // line 259
	void Interp_UpdateInterpolationAmounts( VarMapping_t * );  // line 260
	void Interp_HierarchyUpdateInterpolationAmounts();  // line 261
	virtual bool Init( int, int );  // line 264
	void Term();  // line 267
	static void *operator new( size_t );  // line 270
	static void *operator new []( size_t );  // line 271
	static void *operator new( size_t, int, const char *, int );  // line 272
	static void *operator new []( size_t, int, const char *, int );  // line 273
	static void operator delete( void * );  // line 274
	static void operator delete( void *, int, const char *, int );  // line 275
	virtual IClientUnknown *GetIClientUnknown();  // line 278
	virtual C_BaseAnimating *GetBaseAnimating();  // line 279
	virtual void SetClassname( const char * );  // line 280
	virtual Class_T Classify();  // line 282
	string_t m_iClassname; // +0x10  // line 284
	HSCRIPT GetScriptInstance();  // line 286
	HSCRIPT m_hScriptInstance; // +0x14  // line 288
	string_t m_iszScriptId; // +0x18  // line 289
	virtual void SetRefEHandle( const CBaseHandle & );  // line 294
	virtual const CBaseHandle &GetRefEHandle() const;  // line 295
	void SetToolHandle( HTOOLHANDLE );  // line 297
	HTOOLHANDLE GetToolHandle() const;  // line 298
	void EnableInToolView( bool );  // line 300
	bool IsEnabledInToolView() const;  // line 301
	void SetToolRecording( bool );  // line 303
	bool IsToolRecording() const;  // line 304
	bool HasRecordedThisFrame() const;  // line 305
	virtual void RecordToolMessage();  // line 306
	virtual void OnToolStartRecording();  // line 307
	void DontRecordInTools();  // line 310
	bool ShouldRecordInTools() const;  // line 311
protected:
	virtual void Release();  // line 314
public:
	virtual ICollideable *GetCollideable();  // line 318
	virtual IClientNetworkable *GetClientNetworkable();  // line 319
	virtual IClientRenderable *GetClientRenderable();  // line 320
	virtual IClientEntity *GetIClientEntity();  // line 321
	virtual C_BaseEntity *GetBaseEntity();  // line 322
	virtual IClientThinkable *GetClientThinkable();  // line 323
	virtual IClientModelRenderable *GetClientModelRenderable();  // line 324
	virtual IClientAlphaProperty *GetClientAlphaProperty();  // line 325
	virtual const Vector &GetRenderOrigin();  // line 334
	virtual const QAngle &GetRenderAngles();  // line 335
	virtual Vector GetObserverCamOrigin();  // line 336
	virtual const matrix3x4_t &RenderableToWorldTransform();  // line 337
	virtual int GetRenderFlags();  // line 338
	virtual const model_t *GetModel() const;  // line 339
	virtual int DrawModel( int, const RenderableInstance_t & );  // line 340
	virtual bool LODTest();  // line 341
	virtual void GetRenderBounds( Vector &, Vector & );  // line 342
	virtual IPVSNotify *GetPVSNotifyInterface();  // line 343
	virtual void GetRenderBoundsWorldspace( Vector &, Vector & );  // line 344
	virtual void GetShadowRenderBounds( Vector &, Vector &, ShadowType_t );  // line 346
	virtual void GetColorModulation( float * );  // line 349
	virtual void OnThreadedDrawSetup();  // line 351
	virtual bool TestCollision( const Ray_t &, unsigned int, trace_t & );  // line 353
	virtual bool TestHitboxes( const Ray_t &, unsigned int, trace_t & );  // line 354
	C_BaseEntity *GetOwnerEntity() const;  // line 357
	void SetOwnerEntity( C_BaseEntity * );  // line 358
	C_BaseEntity *GetEffectEntity() const;  // line 360
	void SetEffectEntity( C_BaseEntity * );  // line 361
	bool IsAbleToHaveFireEffect() const;  // line 363
	virtual float GetAttackDamageScale();  // line 367
	virtual void NotifyShouldTransmit( ShouldTransmitState_t );  // line 371
	virtual void PreDataUpdate( DataUpdateType_t );  // line 374
	virtual void PostDataUpdate( DataUpdateType_t );  // line 375
	virtual void ValidateModelIndex();  // line 377
	virtual void SetDormant( bool );  // line 380
	virtual bool IsDormant();  // line 381
	virtual void OnSetDormant( bool );  // line 383
	virtual void SetDestroyedOnRecreateEntities();  // line 387
	virtual int GetEFlags() const;  // line 389
	virtual void SetEFlags( int );  // line 390
	void AddEFlags( int );  // line 391
	void RemoveEFlags( int );  // line 392
	bool IsEFlagSet( int ) const;  // line 393
	bool IsMarkedForDeletion();  // line 396
	virtual int entindex() const;  // line 398
	int GetSoundSourceIndex() const;  // line 402
	virtual void ReceiveMessage( int, bf_read & );  // line 405
	virtual void *GetDataTableBasePtr();  // line 407
	virtual void ClientThink();  // line 412
	virtual ClientThinkHandle_t GetThinkHandle();  // line 414
	virtual void SetThinkHandle( ClientThinkHandle_t );  // line 415
	void AddVar( void *, IInterpolatedVar *, int, bool );  // line 420
	void RemoveVar( void *, bool );  // line 421
	VarMapping_t *GetVarMapping();  // line 422
	VarMapping_t m_VarMap; // +0x1c  // line 424
	CCollisionProperty *CollisionProp();  // line 429
	const CCollisionProperty *CollisionProp() const;  // line 430
	CParticleProperty *ParticleProp();  // line 431
	const CParticleProperty *ParticleProp() const;  // line 432
	CClientAlphaProperty *AlphaProp();  // line 433
	const CClientAlphaProperty *AlphaProp() const;  // line 434
	bool IsFloating();  // line 437
	virtual bool ShouldSavePhysics();  // line 439
	virtual void OnSave();  // line 442
	virtual void OnRestore();  // line 443
	virtual int ObjectCaps();  // line 445
	virtual int Save( ISave & );  // line 447
	virtual int Restore( IRestore & );  // line 448
private:
	int SaveDataDescBlock( ISave &, datamap_t * );  // line 452
	int RestoreDataDescBlock( IRestore &, datamap_t * );  // line 453
public:
	virtual bool CreateVPhysics();  // line 458
	IPhysicsObject *VPhysicsInitStatic();  // line 462
	IPhysicsObject *VPhysicsInitNormal( SolidType_t, int, bool, solid_t * );  // line 465
	IPhysicsObject *VPhysicsInitShadow( bool, bool, solid_t * );  // line 469
private:
	bool VPhysicsInitSetup();  // line 473
public:
	void VPhysicsSetObject( IPhysicsObject * );  // line 476
	void VPhysicsSwapObject( IPhysicsObject * );  // line 477
	virtual void VPhysicsDestroyObject();  // line 479
	virtual void VPhysicsUpdate( IPhysicsObject * );  // line 482
	virtual void VPhysicsShadowUpdate( IPhysicsObject * );  // line 483
	IPhysicsObject *VPhysicsGetObject() const;  // line 484
	virtual int VPhysicsGetObjectList( IPhysicsObject **, int );  // line 485
	virtual bool VPhysicsIsFlesh();  // line 486
	virtual void VPhysicsCompensateForPredictionErrors( const uint8 * );  // line 487
	virtual bool SetupBones( matrix3x4a_t *, int, int, float );  // line 491
	virtual void SetupWeights( const matrix3x4_t *, int, float *, float * );  // line 492
	virtual bool UsesFlexDelayedWeights();  // line 493
	virtual void DoAnimationEvents();  // line 494
	virtual const Vector &GetAbsOrigin() const;  // line 496
	virtual const QAngle &GetAbsAngles() const;  // line 497
	Vector Forward() const;  // line 498
	Vector Left() const;  // line 499
	Vector Up() const;  // line 500
	const Vector &GetNetworkOrigin() const;  // line 502
	const QAngle &GetNetworkAngles() const;  // line 503
	void SetNetworkOrigin( const Vector & );  // line 505
	void SetNetworkAngles( const QAngle & );  // line 506
	const Vector &GetLocalOrigin() const;  // line 508
	void SetLocalOrigin( const Vector & );  // line 509
	vec_t GetLocalOriginDim( int ) const;  // line 510
	void SetLocalOriginDim( int, vec_t );  // line 511
	const QAngle &GetLocalAngles() const;  // line 513
	void SetLocalAngles( const QAngle & );  // line 514
	vec_t GetLocalAnglesDim( int ) const;  // line 515
	void SetLocalAnglesDim( int, vec_t );  // line 516
	virtual const Vector &GetPrevLocalOrigin() const;  // line 518
	virtual const QAngle &GetPrevLocalAngles() const;  // line 519
	virtual void Teleport( const Vector *, const QAngle *, const Vector * );  // line 523
	void SetLocalTransform( const matrix3x4_t & );  // line 525
	void SetModelName( string_t );  // line 527
	string_t GetModelName() const;  // line 528
	int GetModelIndex() const;  // line 530
	void SetModelIndex( int );  // line 531
	virtual const Vector &WorldAlignMins() const;  // line 538
	virtual const Vector &WorldAlignMaxs() const;  // line 539
	void SetCollisionBounds( const Vector &, const Vector & );  // line 546
	virtual const Vector &WorldSpaceCenter() const;  // line 549
	const Vector &WorldAlignSize() const;  // line 552
	bool IsPointSized() const;  // line 553
	float BoundingRadius() const;  // line 558
	virtual void ComputeWorldSpaceSurroundingBox( Vector *, Vector * );  // line 561
	matrix3x4_t &EntityToWorldTransform();  // line 564
	const matrix3x4_t &EntityToWorldTransform() const;  // line 565
	void EntityToWorldSpace( const Vector &, Vector * ) const;  // line 568
	void WorldToEntitySpace( const Vector &, Vector * ) const;  // line 569
	matrix3x4_t &GetParentToWorldTransform( matrix3x4_t & );  // line 576
	void GetVectors( Vector *, Vector *, Vector * ) const;  // line 578
	void SetAbsOrigin( const Vector & );  // line 581
	void SetAbsAngles( const QAngle & );  // line 582
	void AddFlag( int );  // line 584
	void RemoveFlag( int );  // line 585
	void ToggleFlag( int );  // line 586
	int GetFlags() const;  // line 587
	void ClearFlags();  // line 588
	void SetDistanceFade( float, float );  // line 590
	void SetGlobalFadeScale( float );  // line 591
	float GetMinFadeDist() const;  // line 592
	float GetMaxFadeDist() const;  // line 593
	float GetGlobalFadeScale() const;  // line 594
	MoveType_t GetMoveType() const;  // line 596
	MoveCollide_t GetMoveCollide() const;  // line 597
	virtual SolidType_t GetSolid() const;  // line 598
	virtual int GetSolidFlags() const;  // line 600
	bool IsSolidFlagSet( int ) const;  // line 601
	void SetSolidFlags( int );  // line 602
	void AddSolidFlags( int );  // line 603
	void RemoveSolidFlags( int );  // line 604
	bool IsSolid() const;  // line 605
	virtual CMouthInfo *GetMouth();  // line 607
	virtual bool GetSoundSpatialization( SpatializationInfo_t & );  // line 611
	virtual int LookupAttachment( const char * );  // line 614
	virtual bool GetAttachment( int, matrix3x4_t & );  // line 615
	virtual bool GetAttachment( int, Vector & );  // line 616
	virtual bool GetAttachment( int, Vector &, QAngle & );  // line 617
	virtual bool GetAttachmentVelocity( int, Vector &, Quaternion & );  // line 618
	virtual void InvalidateAttachments();  // line 619
	virtual C_Team *GetTeam();  // line 622
	virtual int GetTeamNumber() const;  // line 623
	virtual void ChangeTeam( int );  // line 624
	virtual int GetRenderTeamNumber();  // line 625
	virtual bool InSameTeam( C_BaseEntity * );  // line 626
	virtual bool InLocalTeam();  // line 627
	virtual bool IsValidIDTarget();  // line 630
	virtual char *GetIDString();  // line 631
	void EmitSound( const char *, float, float * );  // line 634
	void EmitSound( const char *, HSOUNDSCRIPTHANDLE &, float, float * );  // line 635
	void StopSound( const char * );  // line 636
	void StopSound( const char *, HSOUNDSCRIPTHANDLE & );  // line 637
	void GenderExpandString( const char *, char *, int );  // line 638
	static float GetSoundDuration( const char *, const char * );  // line 640
	static bool GetParametersForSound( const char *, CSoundParameters &, const char * );  // line 642
	static bool GetParametersForSound( const char *, HSOUNDSCRIPTHANDLE &, CSoundParameters &, const char * );  // line 643
	static void EmitSound( IRecipientFilter &, int, const char *, const Vector *, float, float * );  // line 645
	static void EmitSound( IRecipientFilter &, int, const char *, HSOUNDSCRIPTHANDLE &, const Vector *, float, float * );  // line 646
	static void StopSound( int, const char * );  // line 647
	static soundlevel_t LookupSoundLevel( const char * );  // line 648
	static soundlevel_t LookupSoundLevel( const char *, HSOUNDSCRIPTHANDLE & );  // line 649
	static void EmitSound( IRecipientFilter &, int, const EmitSound_t & );  // line 651
	static void EmitSound( IRecipientFilter &, int, const EmitSound_t &, HSOUNDSCRIPTHANDLE & );  // line 652
	static void StopSound( int, int, const char *, bool );  // line 654
	static void EmitAmbientSound( int, const Vector &, const char *, int, float, float * );  // line 656
	static HSOUNDSCRIPTHANDLE PrecacheScriptSound( const char * );  // line 659
	static void PrefetchScriptSound( const char * );  // line 660
	static void RemoveRecipientsIfNotCloseCaptioning( C_RecipientFilter & );  // line 664
	static void EmitCloseCaption( IRecipientFilter &, int, const char *, CUtlVector<Vector,CUtlMemory<Vector, int> > &, float, bool );  // line 665
	static void MarkAimEntsDirty();  // line 668
	static void CalcAimEntPositions();  // line 669
	static bool IsPrecacheAllowed();  // line 671
	static void SetAllowPrecache( bool );  // line 672
	static bool m_bAllowPrecache;  // line 674
	static bool IsSimulatingOnAlternateTicks();  // line 676
	static bool sm_bAccurateTriggerBboxChecks;  // line 679
	virtual void UpdatePartitionListEntry();  // line 684
	virtual bool InitializeAsClientEntity( const char *, bool );  // line 688
	virtual bool Simulate();  // line 697
	virtual void OnDataChanged( DataUpdateType_t );  // line 703
	virtual void OnPreDataChanged( DataUpdateType_t );  // line 706
	bool IsStandable() const;  // line 708
	bool IsBSPModel() const;  // line 709
	virtual IClientVehicle *GetClientVehicle();  // line 713
	virtual void GetAimEntOrigin( IClientEntity *, Vector *, QAngle * );  // line 716
	virtual const Vector &GetOldOrigin();  // line 719
	C_BaseEntity *GetMoveParent() const;  // line 722
	C_BaseEntity *GetRootMoveParent();  // line 723
	C_BaseEntity *FirstMoveChild() const;  // line 724
	C_BaseEntity *NextMovePeer() const;  // line 725
	ClientEntityHandle_t GetClientHandle() const;  // line 727
	bool IsServerEntity();  // line 728
	void RenderWithViewModels( bool );  // line 730
	bool IsRenderingWithViewModels() const;  // line 731
	void DisableCachedRenderBounds( bool );  // line 732
	bool IsCachedRenderBoundsDisabled() const;  // line 733
	virtual RenderableTranslucencyType_t ComputeTranslucencyType();  // line 738
	virtual uint8 OverrideAlphaModulation( uint8 );  // line 739
	virtual uint8 OverrideShadowAlphaModulation( uint8 );  // line 740
	void OnTranslucencyTypeChanged();  // line 743
	void OnSplitscreenRenderingChanged();  // line 746
	virtual void GetToolRecordingState( KeyValues * );  // line 748
	virtual void CleanupToolRecordingState( KeyValues * );  // line 749
	virtual CollideType_t GetCollideType();  // line 753
	virtual bool ShouldDraw();  // line 755
	bool IsVisible() const;  // line 756
	bool IsVisibleToAnyPlayer() const;  // line 757
	void UpdateVisibility();  // line 758
	virtual bool IsSelfAnimating();  // line 763
	virtual void OnLatchInterpolatedVariables( int );  // line 766
	void OnStoreLastNetworkedValue();  // line 768
	virtual CStudioHdr *OnNewModel();  // line 771
	virtual void OnNewParticleEffect( const char *, CNewParticleEffect * );  // line 772
	virtual void OnParticleEffectDeleted( CNewParticleEffect * );  // line 773
	bool IsSimulatedEveryTick() const;  // line 775
	bool IsAnimatedEveryTick() const;  // line 776
	void SetSimulatedEveryTick( bool );  // line 777
	void SetAnimatedEveryTick( bool );  // line 778
	void Interp_Reset( VarMapping_t * );  // line 780
	virtual void ResetLatched();  // line 781
	float GetInterpolationAmount( int );  // line 783
	float GetLastChangeTime( int );  // line 784
	virtual bool Interpolate( float );  // line 787
	bool Teleported();  // line 790
	virtual bool IsSubModel();  // line 792
	virtual bool CreateLightEffects();  // line 794
	void AddToAimEntsList();  // line 796
	void RemoveFromAimEntsList();  // line 797
	virtual void Clear();  // line 800
	virtual int DrawBrushModel( bool, bool, bool );  // line 802
	virtual float GetTextureAnimationStartTime();  // line 805
	virtual void TextureAnimationWrapped();  // line 807
	virtual void SetNextClientThink( float );  // line 810
	virtual void SetHealth( int );  // line 813
	virtual int GetHealth() const;  // line 814
	virtual int GetMaxHealth() const;  // line 815
	float HealthFraction() const;  // line 818
	virtual ShadowType_t ShadowCastType();  // line 821
	virtual bool ShouldRenderInFastReflections();  // line 823
	virtual bool ShouldReceiveProjectedTextures( int );  // line 826
	virtual bool IsShadowDirty();  // line 829
	virtual void MarkShadowDirty( bool );  // line 830
	virtual IClientRenderable *GetShadowParent();  // line 831
	virtual IClientRenderable *FirstShadowChild();  // line 832
	virtual IClientRenderable *NextShadowPeer();  // line 833
	void AddToLeafSystem();  // line 836
	void AddToLeafSystem( bool );  // line 837
	void RemoveFromLeafSystem();  // line 839
	virtual void AddDecal( const Vector &, const Vector &, const Vector &, int, int, bool, trace_t &, int );  // line 843
	void RemoveAllDecals();  // line 845
	bool IsBrushModel() const;  // line 848
	float ProxyRandomValue() const;  // line 851
	float SpawnTime() const;  // line 854
	virtual bool IsClientCreated() const;  // line 856
	virtual void UpdateOnRemove();  // line 858
	virtual void SUB_Remove();  // line 860
	void CheckInitPredictable( const char * );  // line 864
	virtual C_BasePlayer *GetPredictionOwner();  // line 865
	void AllocateIntermediateData();  // line 867
	void DestroyIntermediateData();  // line 868
	void ShiftIntermediateDataForward( int, int );  // line 869
	void ShiftFirstPredictedIntermediateDataForward( int );  // line 870
	void *GetPredictedFrame( int );  // line 872
	void *GetFirstPredictedFrame( int );  // line 873
	void *GetOriginalNetworkDataObject();  // line 874
	bool IsIntermediateDataAllocated() const;  // line 875
	virtual void InitPredictable( C_BasePlayer * );  // line 877
	void ShutdownPredictable();  // line 878
	int GetSplitUserPlayerPredictionSlot();  // line 879
	virtual void SetPredictable( bool );  // line 881
	bool GetPredictable() const;  // line 882
	void PreEntityPacketReceived( int );  // line 883
	void PostEntityPacketReceived();  // line 884
	bool PostNetworkDataReceived( int );  // line 885
	bool GetPredictionEligible() const;  // line 886
	void SetPredictionEligible( bool );  // line 887
	void SaveData( const char *, int, int );  // line 894
	void RestoreData( const char *, int, int );  // line 895
	void OnPostRestoreData();  // line 900
	virtual const char *DamageDecal( int, int );  // line 902
	virtual void DecalTrace( trace_t *, const char * );  // line 903
	virtual void ImpactTrace( trace_t *, int, char * );  // line 904
	struct <anonymous> m_pfnThink; // +0x38  // line 908
	virtual void Think();  // line 909
	void PhysicsDispatchThink( BASEPTR );  // line 919
	void ClearBBoxVisualization();  // line 929
	void ToggleBBoxVisualization( int );  // line 930
	void DrawBBoxVisualizations();  // line 931
	virtual bool PreRender( int );  // line 933
	bool IsViewEntity() const;  // line 935
	void SetSize( const Vector &, const Vector & );  // line 939
	const char *GetClassname();  // line 940
	const char *GetDebugName();  // line 941
	virtual const char *GetPlayerName() const;  // line 942
	static int PrecacheModel( const char * );  // line 943
	static bool PrecacheSound( const char * );  // line 944
	static void PrefetchSound( const char * );  // line 945
	void Remove();  // line 946
	const char *GetSignifierName();  // line 949
	unsigned char GetParentAttachment() const;  // line 956
	bool HasDataObjectType( int ) const;  // line 959
	void AddDataObjectType( int );  // line 960
	void RemoveDataObjectType( int );  // line 961
	void *GetDataObject( int );  // line 963
	void *CreateDataObject( int );  // line 964
	void DestroyDataObject( int );  // line 965
	void DestroyAllDataObjects();  // line 966
	virtual void EstimateAbsVelocity( Vector & );  // line 969
	void SetPlayerSimulated( C_BasePlayer * );  // line 973
	bool IsPlayerSimulated() const;  // line 974
	C_BasePlayer *GetSimulatingPlayer();  // line 975
	void UnsetPlayerSimulated();  // line 976
	virtual bool CanBePoweredUp();  // line 980
	virtual bool AttemptToPowerup( int, float, float, C_BaseEntity *, CDamageModifier * );  // line 981
	void SetCheckUntouch( bool );  // line 983
	bool GetCheckUntouch() const;  // line 984
	virtual bool IsCurrentlyTouching() const;  // line 986
	virtual void StartTouch( C_BaseEntity * );  // line 988
	virtual void Touch( C_BaseEntity * );  // line 989
	virtual void EndTouch( C_BaseEntity * );  // line 990
	struct <anonymous> m_pfnTouch; // +0x40  // line 992
	void PhysicsStep();  // line 994
protected:
	static bool sm_bDisableTouchFuncs;  // line 997
public:
	touchlink_t *PhysicsMarkEntityAsTouched( C_BaseEntity * );  // line 1000
	void PhysicsTouch( C_BaseEntity * );  // line 1001
	void PhysicsStartTouch( C_BaseEntity * );  // line 1002
	static const trace_t &GetTouchTrace();  // line 1005
	void PhysicsImpact( C_BaseEntity *, trace_t & );  // line 1008
	void PhysicsMarkEntitiesAsTouching( C_BaseEntity *, trace_t & );  // line 1009
	void PhysicsMarkEntitiesAsTouchingEventDriven( C_BaseEntity *, trace_t & );  // line 1010
	void PhysicsTouchTriggers( const Vector * );  // line 1011
	static void PhysicsRemoveTouchedList( C_BaseEntity * );  // line 1014
	static void PhysicsNotifyOtherOfUntouch( C_BaseEntity *, C_BaseEntity * );  // line 1015
	static void PhysicsRemoveToucher( C_BaseEntity *, touchlink_t * );  // line 1016
	groundlink_t *AddEntityToGroundList( C_BaseEntity * );  // line 1018
	void PhysicsStartGroundContact( C_BaseEntity * );  // line 1019
	static void PhysicsNotifyOtherOfGroundRemoval( C_BaseEntity *, C_BaseEntity * );  // line 1021
	static void PhysicsRemoveGround( C_BaseEntity *, groundlink_t * );  // line 1022
	static void PhysicsRemoveGroundList( C_BaseEntity * );  // line 1023
	void StartGroundContact( C_BaseEntity * );  // line 1025
	void EndGroundContact( C_BaseEntity * );  // line 1026
	void SetGroundChangeTime( float );  // line 1028
	float GetGroundChangeTime();  // line 1029
	void WakeRestingObjects();  // line 1032
	bool HasNPCsOnIt();  // line 1033
	bool PhysicsCheckWater();  // line 1035
	void PhysicsCheckVelocity();  // line 1036
	void PhysicsAddHalfGravity( float );  // line 1037
	void PhysicsAddGravityMove( Vector & );  // line 1038
	virtual unsigned int PhysicsSolidMaskForEntity() const;  // line 1040
	void SetGroundEntity( C_BaseEntity * );  // line 1042
	C_BaseEntity *GetGroundEntity();  // line 1043
	C_BaseEntity *GetGroundEntity() const;  // line 1044
	void PhysicsPushEntity( const Vector &, trace_t * );  // line 1046
	void PhysicsCheckWaterTransition();  // line 1047
	void PerformFlyCollisionResolution( trace_t &, Vector & );  // line 1050
	void ResolveFlyCollisionBounce( trace_t &, Vector &, float );  // line 1051
	void ResolveFlyCollisionSlide( trace_t &, Vector & );  // line 1052
	void ResolveFlyCollisionCustom( trace_t &, Vector & );  // line 1053
	void PhysicsCheckForEntityUntouch();  // line 1055
	void CreateShadow();  // line 1058
	void DestroyShadow();  // line 1061
	// game/client/c_baseentity.h:1065
	enum thinkmethods_t
	{
		THINK_FIRE_ALL_FUNCTIONS = 0,
		THINK_FIRE_BASE_ONLY = 1,
		THINK_FIRE_ALL_BUT_BASE = 2,
	};  // line 1065
	void SetParent( C_BaseEntity *, int );  // line 1077
	bool PhysicsRunThink( C_BaseEntity::thinkmethods_t );  // line 1079
	bool PhysicsRunSpecificThink( int, BASEPTR );  // line 1080
	virtual void PhysicsSimulate();  // line 1082
	virtual bool IsAlive();  // line 1083
	bool IsInWorld();  // line 1085
	bool IsWorld() const;  // line 1087
	virtual bool ShouldRegenerateOriginFromCellBits() const;  // line 1090
	virtual bool IsPlayer() const;  // line 1092
	virtual bool IsBaseCombatCharacter();  // line 1093
	virtual C_BaseCombatCharacter *MyCombatCharacterPointer();  // line 1094
	virtual bool IsNPC();  // line 1095
	C_AI_BaseNPC *MyNPCPointer();  // line 1096
	virtual bool IsSprite() const;  // line 1098
	virtual bool IsProp() const;  // line 1099
	virtual bool IsBaseObject() const;  // line 1102
	virtual bool IsBaseCombatWeapon() const;  // line 1103
	virtual C_BaseCombatWeapon *MyCombatWeaponPointer();  // line 1104
	virtual bool ShouldDrawForSplitScreenUser( int );  // line 1107
	void SetBlurState( bool );  // line 1108
	virtual bool IsBlurred();  // line 1109
	virtual bool IsBaseTrain() const;  // line 1111
	virtual Vector EyePosition();  // line 1114
	virtual const QAngle &EyeAngles();  // line 1115
	virtual const QAngle &LocalEyeAngles();  // line 1116
	virtual Vector EarPosition();  // line 1119
	Vector EyePosition() const;  // line 1121
	const QAngle &EyeAngles() const;  // line 1122
	const QAngle &LocalEyeAngles() const;  // line 1123
	Vector EarPosition() const;  // line 1124
	virtual bool ShouldCollide( int, int ) const;  // line 1127
	void SetFriction( float );  // line 1130
	void SetGravity( float );  // line 1132
	float GetGravity() const;  // line 1133
	void SetModelByIndex( int );  // line 1136
	bool SetModel( const char * );  // line 1140
	void SetModelPointer( const model_t * );  // line 1142
	void SetMoveType( MoveType_t, MoveCollide_t );  // line 1145
	void SetMoveCollide( MoveCollide_t );  // line 1146
	void SetSolid( SolidType_t );  // line 1147
	void SetLocalVelocity( const Vector & );  // line 1151
	void SetAbsVelocity( const Vector & );  // line 1152
	const Vector &GetLocalVelocity() const;  // line 1153
	const Vector &GetAbsVelocity() const;  // line 1154
	void ApplyLocalVelocityImpulse( const Vector & );  // line 1156
	void ApplyAbsVelocityImpulse( const Vector & );  // line 1157
	void ApplyLocalAngularVelocityImpulse( const AngularImpulse & );  // line 1158
	void SetLocalAngularVelocity( const QAngle & );  // line 1162
	const QAngle &GetLocalAngularVelocity() const;  // line 1163
	const Vector &GetBaseVelocity() const;  // line 1168
	void SetBaseVelocity( const Vector & );  // line 1169
	virtual const Vector &GetViewOffset() const;  // line 1171
	virtual void SetViewOffset( const Vector & );  // line 1172
	virtual void GetGroundVelocityToApply( Vector & );  // line 1174
	const Vector &GetEyeOffset() const;  // line 1177
	void SetEyeOffset( const Vector & );  // line 1178
	const QAngle &GetEyeAngleOffset() const;  // line 1180
	void SetEyeAngleOffset( const QAngle & );  // line 1181
	void InvalidatePhysicsRecursive( int );  // line 1185
	ClientRenderHandle_t GetRenderHandle() const;  // line 1187
	void SetRemovalFlag( bool );  // line 1189
	bool HasSpawnFlags( int ) const;  // line 1191
	bool IsEffectActive( int ) const;  // line 1194
	void AddEffects( int );  // line 1195
	void RemoveEffects( int );  // line 1196
	int GetEffects() const;  // line 1197
	void ClearEffects();  // line 1198
	void SetEffects( int );  // line 1199
	void ComputeAbsPosition( const Vector &, Vector * );  // line 1202
	void ComputeAbsDirection( const Vector &, Vector * );  // line 1205
	void FollowEntity( C_BaseEntity *, bool );  // line 1208
	void StopFollowingEntity();  // line 1209
	bool IsFollowingEntity();  // line 1210
	C_BaseEntity *GetFollowedEntity();  // line 1211
	virtual int GetBody();  // line 1214
	virtual int GetSkin();  // line 1215
	const Vector &ScriptGetForward();  // line 1217
	const Vector &ScriptGetLeft();  // line 1218
	const Vector &ScriptGetUp();  // line 1219
	void NetworkStateManualMode( bool );  // line 1223
	void NetworkStateSetUpdateInterval( float );  // line 1226
	void NetworkStateForceUpdate();  // line 1227
	int RegisterThinkContext( const char * );  // line 1230
	BASEPTR ThinkSet( BASEPTR, float, const char * );  // line 1231
	void SetNextThink( float, const char * );  // line 1232
	float GetNextThink( const char * );  // line 1233
	float GetLastThink( const char * );  // line 1234
	int GetNextThinkTick( const char * );  // line 1235
	int GetLastThinkTick( const char * );  // line 1236
	void CheckHasThinkFunction( bool );  // line 1239
	void CheckHasGamePhysicsSimulation();  // line 1240
	bool WillThink();  // line 1241
	bool WillSimulateGamePhysics();  // line 1242
	int GetFirstThinkTick();  // line 1243
	float GetAnimTime() const;  // line 1245
	void SetAnimTime( float );  // line 1246
	float GetSimulationTime() const;  // line 1248
	void SetSimulationTime( float );  // line 1249
	float GetCreateTime();  // line 1251
	void SetCreateTime( float );  // line 1252
	int GetCreationTick() const;  // line 1254
	virtual ModelInstanceHandle_t GetModelInstance();  // line 1269
	void SetModelInstance( ModelInstanceHandle_t );  // line 1270
	bool SnatchModelInstance( C_BaseEntity * );  // line 1271
	virtual ClientShadowHandle_t GetShadowHandle() const;  // line 1272
	virtual ClientRenderHandle_t &RenderHandle();  // line 1273
	virtual void CreateModelInstance();  // line 1275
	void MoveToLastReceivedPosition( bool );  // line 1278
protected:
	void DestroyModelInstance();  // line 1282
	static void ProcessTeleportList();  // line 1285
	static void ProcessInterpolatedList();  // line 1286
	static void CheckInterpolatedVarParanoidMeasurement();  // line 1287
	virtual bool ShouldInterpolate();  // line 1290
	void MarkMessageReceived();  // line 1293
	float GetLastMessageTime() const;  // line 1296
	int PhysicsClipVelocity( const Vector &, const Vector &, Vector &, float );  // line 1299
	int BaseInterpolatePart1( float &, Vector &, QAngle &, int & );  // line 1311
	void BaseInterpolatePart2( Vector &, QAngle &, int );  // line 1312
public:
	static int GetPredictionRandomSeed();  // line 1317
	static void SetPredictionRandomSeed( const CUserCmd * );  // line 1318
	static C_BasePlayer *GetPredictionPlayer();  // line 1319
	static void SetPredictionPlayer( C_BasePlayer * );  // line 1320
	static void CheckCLInterpChanged();  // line 1321
	int GetCollisionGroup() const;  // line 1324
	void SetCollisionGroup( int );  // line 1325
	void CollisionRulesChanged();  // line 1326
	static C_BaseEntity *Instance( int );  // line 1328
	static C_BaseEntity *Instance( IClientEntity * );  // line 1330
	static C_BaseEntity *Instance( CBaseHandle );  // line 1331
	static bool IsServer();  // line 1333
	static bool IsClient();  // line 1334
	static const char *GetDLLType();  // line 1335
	static void SetAbsQueriesValid( bool );  // line 1336
	static bool IsAbsQueriesValid();  // line 1337
	static void PushEnableAbsRecomputations( bool );  // line 1340
	static void PopEnableAbsRecomputations();  // line 1341
	static void EnableAbsRecomputations( bool );  // line 1345
	static bool IsAbsRecomputationsEnabled();  // line 1347
	static void PreRenderEntities( int );  // line 1349
	static void PurgeRemovedEntities();  // line 1350
	static void SimulateEntities();  // line 1351
	virtual void BoneMergeFastCullBloat( Vector &, Vector &, const Vector &, const Vector & ) const;  // line 1354
	const color24 GetRenderColor() const;  // line 1358
	uint8 GetRenderColorR() const;  // line 1359
	uint8 GetRenderColorG() const;  // line 1360
	uint8 GetRenderColorB() const;  // line 1361
	uint8 GetRenderAlpha() const;  // line 1362
	void SetRenderColor( uint8, uint8, uint8 );  // line 1363
	void SetRenderColorR( uint8 );  // line 1364
	void SetRenderColorG( uint8 );  // line 1365
	void SetRenderColorB( uint8 );  // line 1366
	void SetRenderAlpha( uint8 );  // line 1367
	void SetRenderMode( RenderMode_t, bool );  // line 1369
	RenderMode_t GetRenderMode() const;  // line 1370
	void SetRenderFX( RenderFx_t, float, float );  // line 1372
	RenderFx_t GetRenderFX() const;  // line 1373
	bool SetCellBits( int );  // line 1376
	static void RecvProxy_CellBits( const CRecvProxyData *, void *, void * );  // line 1378
	static void RecvProxy_CellX( const CRecvProxyData *, void *, void * );  // line 1379
	static void RecvProxy_CellY( const CRecvProxyData *, void *, void * );  // line 1380
	static void RecvProxy_CellZ( const CRecvProxyData *, void *, void * );  // line 1381
	static void RecvProxy_CellOrigin( const CRecvProxyData *, void *, void * );  // line 1382
	static void RecvProxy_CellOriginXY( const CRecvProxyData *, void *, void * );  // line 1383
	static void RecvProxy_CellOriginZ( const CRecvProxyData *, void *, void * );  // line 1384
	const char *GetEntityName();  // line 1386
	int index; // +0x48  // line 1391
	short unsigned int m_EntClientFlags; // +0x4c  // line 1394
private:
	const model_t *model; // +0x50  // line 1399
	CNetworkColor32( color32_s, m_clrRender ); // +0x54  // line 1400
protected:
	int m_cellbits; // +0x58  // line 1404
	int m_cellwidth; // +0x5c  // line 1405
	int m_cellX; // +0x60  // line 1406
	int m_cellY; // +0x64  // line 1407
	int m_cellZ; // +0x68  // line 1408
	Vector m_vecCellOrigin; // +0x6c  // line 1409
private:
	Vector m_vecAbsVelocity; // +0x78  // line 1413
	Vector m_vecAbsOrigin; // +0x84  // line 1414
	Vector m_vecOrigin; // +0x90  // line 1415
	QAngle m_vecAngVelocity; // +0x9c  // line 1417
	QAngle m_angAbsRotation; // +0xa8  // line 1418
	QAngle m_angRotation; // +0xb4  // line 1419
	float m_flGravity; // +0xc0  // line 1421
	float m_flProxyRandomValue; // +0xc4  // line 1423
	int m_iEFlags; // +0xc8  // line 1425
	unsigned char m_nWaterType; // +0xcc  // line 1427
	bool m_bDormant; // +0xcd  // line 1430
	int m_fEffects; // +0xd0  // line 1435
public:
	int m_iTeamNum; // +0xd4  // line 1438
	int m_nNextThinkTick; // +0xd8  // line 1439
	int m_iHealth; // +0xdc  // line 1440
private:
	int m_fFlags; // +0xe0  // line 1442
protected:
	Vector m_vecViewOffset; // +0xe4  // line 1445
private:
	Vector m_vecVelocity; // +0xf0  // line 1448
	Vector m_vecBaseVelocity; // +0xfc  // line 1449
	QAngle m_angNetworkAngles; // +0x108  // line 1451
	Vector m_vecNetworkOrigin; // +0x114  // line 1454
	float m_flFriction; // +0x120  // line 1457
	CHandle<C_BaseEntity> m_hNetworkMoveParent; // +0x124  // line 1460
	EHANDLE m_hOwnerEntity; // +0x128  // line 1462
	EHANDLE m_hGroundEntity; // +0x12c  // line 1463
	char m_iName[260]; // +0x130  // line 1465
	char m_iSignifierName[260]; // +0x234  // line 1468
public:
	short int m_nModelIndex; // +0x338  // line 1473
private:
	unsigned char m_nRenderFX; // +0x33a  // line 1475
	unsigned char m_nRenderMode; // +0x33b  // line 1476
	unsigned char m_MoveType; // +0x33c  // line 1477
	unsigned char m_MoveCollide; // +0x33d  // line 1478
	unsigned char m_nWaterLevel; // +0x33e  // line 1479
public:
	char m_lifeState; // +0x33f  // line 1482
	float m_flAnimTime; // +0x340  // line 1489
	float m_flOldAnimTime; // +0x344  // line 1490
	float m_flSimulationTime; // +0x348  // line 1492
	float m_flOldSimulationTime; // +0x34c  // line 1493
	float m_flCreateTime; // +0x350  // line 1495
private:
	unsigned char m_nOldRenderMode; // +0x354  // line 1498
public:
	ClientRenderHandle_t m_hRender; // +0x356  // line 1503
	CBitVec<2> m_VisibilityBits; // +0x358  // line 1504
	bool m_bReadyToDraw; // +0x35c  // line 1507
	bool m_bClientSideRagdoll; // +0x35d  // line 1508
	static bool IsInterpolationEnabled();  // line 1511
	int m_nLastThinkTick; // +0x360  // line 1516
	char m_takedamage; // +0x364  // line 1519
	float m_flSpeed; // +0x368  // line 1525
	int touchStamp; // +0x36c  // line 1534
	virtual bool OnPredictedEntityRemove( bool, C_BaseEntity * );  // line 1539
	bool IsDormantPredictable() const;  // line 1541
	bool BecameDormantThisPacket() const;  // line 1542
	void SetDormantPredictable( bool );  // line 1543
	int GetWaterLevel() const;  // line 1545
	void SetWaterLevel( int );  // line 1546
	int GetWaterType() const;  // line 1547
	void SetWaterType( int );  // line 1548
	float GetElasticity() const;  // line 1550
	int GetTextureFrameIndex();  // line 1552
	void SetTextureFrameIndex( int );  // line 1553
	virtual bool GetShadowCastDistance( float *, ShadowType_t ) const;  // line 1555
	virtual bool GetShadowCastDirection( Vector *, ShadowType_t ) const;  // line 1556
	virtual C_BaseEntity *GetShadowUseOtherEntity() const;  // line 1557
	virtual void SetShadowUseOtherEntity( C_BaseEntity * );  // line 1558
	CInterpolatedVar<QAngle> &GetRotationInterpolator();  // line 1560
	CInterpolatedVar<Vector> &GetOriginInterpolator();  // line 1561
	virtual bool AddRagdollToFadeQueue();  // line 1562
	void MarkRenderHandleDirty();  // line 1565
	void HierarchyUpdateMoveParent();  // line 1568
	void SetCPULevels( int, int );  // line 1570
	void SetGPULevels( int, int );  // line 1571
	int GetMinCPULevel() const;  // line 1572
	int GetMaxCPULevel() const;  // line 1573
	int GetMinGPULevel() const;  // line 1574
	int GetMaxGPULevel() const;  // line 1575
	int GetServerObjectCaps();  // line 1579
protected:
	CBaseHandle m_RefEHandle; // +0x370  // line 1586
private:
	bool m_bEnabledInToolView; // +0x374  // line 1591
	bool m_bToolRecording; // +0x375  // line 1592
	HTOOLHANDLE m_ToolHandle; // +0x378  // line 1593
	int m_nLastRecordedFrame; // +0x37c  // line 1594
	bool m_bRecordInTools; // +0x380  // line 1595
protected:
	IPhysicsObject *m_pPhysicsObject; // +0x384  // line 1600
	bool m_bPredictionEligible; // +0x388  // line 1603
	int m_nSimulationTick; // +0x38c  // line 1606
	int GetIndexForThinkContext( const char * );  // line 1609
	CUtlVector<thinkfunc_t,CUtlMemory<thinkfunc_t, int> > m_aThinkFunctions; // +0x390  // line 1610
	int m_iCurrentThinkContext; // +0x3a4  // line 1611
	Vector m_vecEyeOffset; // +0x3a8  // line 1615
	QAngle m_EyeAngleOffset; // +0x3b4  // line 1616
	int m_spawnflags; // +0x3c0  // line 1619
	virtual int GetStudioBody();  // line 1622
	bool IsParentChanging();  // line 1624
	int m_iObjectCapsCache; // +0x3c4  // line 1629
private:
	bool InitializeAsClientEntityByIndex( int, bool );  // line 1636
	static void InterpolateServerEntities();  // line 1640
	static void AddVisibleEntities();  // line 1643
	static void ToolRecordEntities();  // line 1646
	void UpdateBaseVelocity();  // line 1649
	void PhysicsPusher();  // line 1652
	void PhysicsNone();  // line 1653
	void PhysicsNoclip();  // line 1654
	void PhysicsParent();  // line 1655
	void PhysicsStepRunTimestep( float );  // line 1656
	void PhysicsToss();  // line 1657
	void PhysicsCustom();  // line 1658
	void PhysicsRigidChild();  // line 1661
	void CalcAbsolutePosition();  // line 1664
	void CalcAbsoluteVelocity();  // line 1665
	void SimulateAngles( float );  // line 1668
	virtual void PerformCustomPhysics( Vector *, Vector *, QAngle *, QAngle * );  // line 1671
	void AddStudioDecal( const Ray_t &, int, int, bool, trace_t &, int );  // line 1674
	void AddBrushModelDecal( const Ray_t &, const Vector &, int, bool, trace_t & );  // line 1675
	void ComputePackedOffsets();  // line 1677
	int GetIntermediateDataSize();  // line 1678
	void UnlinkChild( C_BaseEntity *, C_BaseEntity * );  // line 1680
	void LinkChild( C_BaseEntity *, C_BaseEntity * );  // line 1681
	void HierarchySetParent( C_BaseEntity * );  // line 1682
	void UnlinkFromHierarchy();  // line 1683
	void UpdateWaterState();  // line 1686
	void PhysicsCheckSweep( const Vector &, const Vector &, trace_t * );  // line 1689
	void MoveToAimEnt();  // line 1692
	void SetNextThink( int, float );  // line 1695
	void SetLastThink( int, float );  // line 1696
	float GetNextThink( int ) const;  // line 1697
	int GetNextThinkTick( int ) const;  // line 1698
	void CleanUpAlphaProperty();  // line 1700
	bool m_bDormantPredictable; // +0x3c8  // line 1704
	int m_nIncomingPacketEntityBecameDormant; // +0x3cc  // line 1707
	float m_flSpawnTime; // +0x3d0  // line 1711
	float m_flLastMessageTime; // +0x3d4  // line 1714
	ModelInstanceHandle_t m_ModelInstance; // +0x3d8  // line 1719
	ClientShadowHandle_t m_ShadowHandle; // +0x3da  // line 1722
	CBitVec<2> m_ShadowBits; // +0x3dc  // line 1723
	float m_fadeMinDist; // +0x3e0  // line 1726
	float m_fadeMaxDist; // +0x3e4  // line 1727
	float m_flFadeScale; // +0x3e8  // line 1728
	ClientThinkHandle_t m_hThink; // +0x3ec  // line 1730
	unsigned char m_iParentAttachment; // +0x3f0  // line 1732
	unsigned char m_iOldParentAttachment; // +0x3f1  // line 1733
	bool m_bPredictable; // +0x3f2  // line 1738
	bool m_bRenderWithViewModels; // +0x3f3  // line 1739
	bool m_bDisableCachedRenderBounds; // +0x3f4  // line 1740
	int m_nSplitUserPlayerPredictionSlot; // +0x3f8  // line 1741
	CHandle<C_BaseEntity> m_pMoveParent; // +0x3fc  // line 1744
	CHandle<C_BaseEntity> m_pMoveChild; // +0x400  // line 1745
	CHandle<C_BaseEntity> m_pMovePeer; // +0x404  // line 1746
	CHandle<C_BaseEntity> m_pMovePrevPeer; // +0x408  // line 1747
	CHandle<C_BaseEntity> m_hOldMoveParent; // +0x40c  // line 1748
	string_t m_ModelName; // +0x410  // line 1750
	C_BaseEntity::NetworkVar_m_Collision m_Collision; // +0x414  // line 1752
	static int GetOffset_m_Collision();  // line 1752
	C_BaseEntity::NetworkVar_m_Particles m_Particles; // +0x470  // line 1753
	static int GetOffset_m_Particles();  // line 1753
	CClientAlphaProperty *m_pClientAlphaProperty; // +0x490  // line 1754
	float m_flElasticity; // +0x494  // line 1757
	float m_flShadowCastDistance; // +0x498  // line 1759
	EHANDLE m_ShadowDirUseOtherEntity; // +0x49c  // line 1760
	float m_flGroundChangeTime; // +0x4a0  // line 1762
	Vector m_vecOldOrigin; // +0x4a4  // line 1767
	QAngle m_vecOldAngRotation; // +0x4b0  // line 1768
	CInterpolatedVar<Vector> m_iv_vecOrigin; // +0x4bc  // line 1771
	CInterpolatedVar<QAngle> m_iv_angRotation; // +0x4e8  // line 1772
	matrix3x4_t m_rgflCoordinateFrame; // +0x514  // line 1775
	int m_CollisionGroup; // +0x544  // line 1778
	uint8 *m_pIntermediateData[150]; // +0x548  // line 1782
	uint8 *m_pIntermediateData_FirstPredicted[151]; // +0x7a0  // line 1783
	uint8 *m_pOriginalData; // +0x9fc  // line 1784
	int m_nIntermediateDataCount; // +0xa00  // line 1785
	int m_nIntermediateData_FirstPredictedShiftMarker; // +0xa04  // line 1786
	bool m_bEverHadPredictionErrorsForThisCommand; // +0xa08  // line 1787
	bool m_bIsPlayerSimulated; // +0xa09  // line 1789
	CNetworkVar( bool, m_bSimulatedEveryTick ); // +0xa0a  // line 1792
	CNetworkVar( bool, m_bAnimatedEveryTick ); // +0xa0b  // line 1793
	CNetworkVar( bool, m_bAlternateSorting ); // +0xa0c  // line 1794
	unsigned char m_nMinCPULevel; // +0xa0d  // line 1796
	unsigned char m_nMaxCPULevel; // +0xa0e  // line 1797
	unsigned char m_nMinGPULevel; // +0xa0f  // line 1798
	unsigned char m_nMaxGPULevel; // +0xa10  // line 1799
	unsigned char m_iTextureFrameIndex; // +0xa11  // line 1802
	unsigned char m_fBBoxVisFlags; // +0xa12  // line 1805
	bool m_bIsValidIKAttachment; // +0xa13  // line 1807
	int m_DataChangeEventRef; // +0xa14  // line 1811
	CHandle<C_BasePlayer> m_hPlayerSimulationOwner; // +0xa18  // line 1815
	EHANDLE m_hEffectEntity; // +0xa1c  // line 1819
	static int m_nPredictionRandomSeed;  // line 1824
	static C_BasePlayer *m_pPredictionPlayer;  // line 1825
	static bool s_bAbsQueriesValid;  // line 1826
	static bool s_bAbsRecomputationEnabled;  // line 1827
	static bool s_bInterpolate;  // line 1829
	int m_fDataObjectTypes; // +0xa20  // line 1831
	AimEntsListHandle_t m_AimEntsListHandle; // +0xa24  // line 1833
	int m_nCreationTick; // +0xa28  // line 1834
public:
	float m_fRenderingClipPlane[4]; // +0xa2c  // line 1838
	bool m_bEnableRenderingClipPlane; // +0xa3c  // line 1839
	virtual float *GetRenderClipPlane();  // line 1840
protected:
	void AddToEntityList( entity_list_ids_t );  // line 1844
	void RemoveFromEntityList( entity_list_ids_t );  // line 1845
	short unsigned int m_ListEntry[5]; // +0xa3e  // line 1846
	CThreadFastMutex m_CalcAbsolutePositionMutex; // +0xa48  // line 1848
	CThreadFastMutex m_CalcAbsoluteVelocityMutex; // +0xa50  // line 1849
private:
	bool m_bIsBlurred; // +0xa58  // line 1852
};
