// Class layouts from /home/john/Downloads/portal2-steam2-research/852_3/portal2/bin/server.dylib.dSYM/Contents/Resources/DWARF/server.dylib
// Unit game/server/player.cpp; i386 offsets; reconstruction aid.

// game/server/player.h:208
// game/server/player.h:208 sizeof=0x1230 (i386)
struct CBasePlayer : public CBaseCombatCharacter
{
protected:
	static edict_t *s_PlayerEdict;  // line 214
public:
	virtual ScriptClassDesc_t *GetScriptDesc();  // line 219
	CBasePlayer();  // line 221
	virtual ~CBasePlayer();  // line 222
	void StartUserMessageThrottling( const char **, int );  // line 224
	void FinishUserMessageThrottling();  // line 225
	bool ShouldThrottleUserMessage( const char * );  // line 227
	IPlayerInfo *GetPlayerInfo();  // line 231
	IBotController *GetBotController();  // line 232
	virtual void SetModel( const char * );  // line 234
	void SetBodyPitch( float );  // line 235
	virtual void UpdateOnRemove();  // line 237
	static CBasePlayer *CreatePlayer( const char *, edict_t * );  // line 239
	virtual void CreateViewModel( int );  // line 241
	CBaseViewModel *GetViewModel( int );  // line 242
	void HideViewModels();  // line 243
	void DestroyViewModels();  // line 244
	CPlayerState *PlayerData();  // line 246
	virtual int RequiredEdictIndex();  // line 248
	void LockPlayerInPlace();  // line 250
	void UnlockPlayer();  // line 251
	virtual void DrawDebugGeometryOverlays();  // line 253
	virtual void SetupVisibility( CBaseEntity *, unsigned char *, int );  // line 256
	virtual int UpdateTransmitState();  // line 257
	virtual int ShouldTransmit( const CCheckTransmitInfo * );  // line 258
	virtual bool WantsLagCompensationOnEntity( const CBaseEntity *, const CUserCmd *, const CBitVec<2048> * ) const;  // line 263
	virtual void Spawn();  // line 265
	virtual void Activate();  // line 266
	virtual void SharedSpawn();  // line 267
	virtual void ForceRespawn();  // line 268
	virtual void InitialSpawn();  // line 270
	virtual void InitHUD();  // line 271
	virtual void ShowViewPortPanel( const char *, bool, KeyValues * );  // line 272
	virtual const char *GetPlayerModelName();  // line 274
	virtual void PlayerDeathThink();  // line 276
	virtual void Jump();  // line 278
	virtual void Duck();  // line 279
	virtual const char *GetTracerType();  // line 281
	virtual void MakeTracer( const Vector &, const trace_t &, int );  // line 282
	virtual void DoImpactEffect( trace_t &, int );  // line 283
	void AddToPlayerSimulationList( CBaseEntity * );  // line 286
	void RemoveFromPlayerSimulationList( CBaseEntity * );  // line 287
	void SimulatePlayerSimulatedEntities();  // line 288
	void ClearPlayerSimulationList();  // line 289
	virtual void PhysicsSimulate();  // line 293
	void ForceSimulation();  // line 296
	virtual unsigned int PhysicsSolidMaskForEntity() const;  // line 298
	virtual void PreThink();  // line 300
	virtual void PostThink();  // line 301
	virtual int TakeHealth( float, int );  // line 302
	virtual void TraceAttack( const CTakeDamageInfo &, const Vector &, trace_t * );  // line 303
	bool ShouldTakeDamageInCommentaryMode( const CTakeDamageInfo & );  // line 304
	virtual int OnTakeDamage( const CTakeDamageInfo & );  // line 305
	virtual void DamageEffect( float, int );  // line 306
	virtual void OnDamagedByExplosion( const CTakeDamageInfo & );  // line 308
	void PauseBonusProgress( bool );  // line 310
	void SetBonusProgress( int );  // line 311
	void SetBonusChallenge( int );  // line 312
	int GetBonusProgress() const;  // line 314
	int GetBonusChallenge() const;  // line 315
	virtual Vector EyePosition();  // line 317
	virtual const QAngle &EyeAngles();  // line 318
	void EyePositionAndVectors( Vector *, Vector *, Vector *, Vector * );  // line 319
	virtual const QAngle &LocalEyeAngles();  // line 320
	void EyeVectors( Vector *, Vector *, Vector * );  // line 321
	void CacheVehicleView();  // line 322
	void SnapEyeAngles( const QAngle & );  // line 325
	virtual QAngle BodyAngles();  // line 327
	virtual Vector BodyTarget( const Vector &, bool );  // line 328
	virtual bool ShouldFadeOnDeath();  // line 329
	virtual const impactdamagetable_t &GetPhysicsImpactDamageTable();  // line 331
	virtual int OnTakeDamage_Alive( const CTakeDamageInfo & );  // line 332
	virtual void Event_Killed( const CTakeDamageInfo & );  // line 333
	virtual void Event_KilledOther( CBaseEntity *, const CTakeDamageInfo & );  // line 335
	virtual void Event_Dying();  // line 337
	bool IsHLTV() const;  // line 339
	bool IsReplay() const;  // line 340
	virtual bool IsPlayer() const;  // line 341
	virtual bool IsNetClient() const;  // line 342
	virtual bool IsFakeClient() const;  // line 345
	int GetClientIndex();  // line 348
	virtual const char *GetPlayerName() const;  // line 351
	void SetPlayerName( const char * );  // line 352
	virtual const char *GetCharacterDisplayName();  // line 354
	int GetUserID() const;  // line 356
	const char *GetNetworkIDString();  // line 357
	virtual const Vector GetPlayerMins() const;  // line 358
	virtual const Vector GetPlayerMaxs() const;  // line 359
	virtual void UpdateCollisionBounds();  // line 361
	void VelocityPunch( const Vector & );  // line 363
	void ViewPunch( const QAngle & );  // line 364
	void ViewPunchReset( float );  // line 365
	void ShowViewModel( bool );  // line 366
	void ShowCrosshair( bool );  // line 367
	bool ScriptIsPlayerNoclipping();  // line 369
	virtual void NoClipStateChanged();  // line 370
	void CalcView( Vector &, QAngle &, float &, float &, float & );  // line 373
	void SmoothViewOnStairs( Vector & );  // line 376
	virtual float CalcRoll( const QAngle &, const Vector &, float, float );  // line 377
	void CalcViewRoll( QAngle & );  // line 378
	virtual void CalcViewBob( Vector & );  // line 379
	virtual int Save( ISave & );  // line 381
	virtual int Restore( IRestore & );  // line 382
	virtual bool ShouldSavePhysics();  // line 383
	virtual void OnRestore();  // line 384
	virtual void PackDeadPlayerItems();  // line 386
	virtual void RemoveAllItems( bool );  // line 387
	bool IsDead() const;  // line 388
	bool HasPhysicsFlag( unsigned int );  // line 393
	virtual CBaseCombatCharacter *ActivePlayerCombatCharacter();  // line 395
	virtual Vector Weapon_ShootPosition();  // line 398
	virtual bool Weapon_CanUse( CBaseCombatWeapon * );  // line 399
	virtual void Weapon_Equip( CBaseCombatWeapon * );  // line 400
	virtual void Weapon_Drop( CBaseCombatWeapon *, const Vector *, const Vector * );  // line 401
	virtual bool Weapon_Switch( CBaseCombatWeapon *, int );  // line 402
	virtual void Weapon_SetLast( CBaseCombatWeapon * );  // line 403
	virtual bool Weapon_ShouldSetLast( CBaseCombatWeapon *, CBaseCombatWeapon * );  // line 404
	virtual bool Weapon_ShouldSelectItem( CBaseCombatWeapon * );  // line 405
	void Weapon_DropSlot( int );  // line 406
	CBaseCombatWeapon *Weapon_GetLast();  // line 407
	virtual bool HasUnlockableWeapons( int );  // line 409
	bool HasUnlockedWpn( int );  // line 410
	bool HasAnyAmmoOfType( int );  // line 411
	virtual void UpdateClientData();  // line 414
	virtual void UpdateBattery();  // line 415
	virtual void RumbleEffect( unsigned char, unsigned char, unsigned char );  // line 416
	virtual int ObjectCaps();  // line 419
	virtual void Precache();  // line 420
	bool IsOnLadder();  // line 421
	virtual void ExitLadder();  // line 422
	virtual surfacedata_t *GetLadderSurface( const Vector & );  // line 423
	virtual void SetFlashlightEnabled( bool );  // line 426
	virtual int FlashlightIsOn();  // line 427
	virtual bool FlashlightTurnOn( bool );  // line 428
	virtual void FlashlightTurnOff( bool );  // line 429
	virtual bool IsIlluminatedByFlashlight( CBaseEntity *, float * );  // line 430
	virtual void UpdatePlayerSound();  // line 432
	virtual void UpdateStepSound( surfacedata_t *, const Vector &, const Vector & );  // line 433
	virtual void PlayStepSound( Vector &, surfacedata_t *, float, bool );  // line 434
	virtual void GetStepSoundVelocities( float *, float * );  // line 435
	virtual void SetStepSoundTime( stepsoundtimes_t, bool );  // line 436
	virtual void DeathSound( const CTakeDamageInfo & );  // line 437
	const Vector &GetMovementCollisionNormal() const;  // line 438
	const Vector &GetGroundNormal() const;  // line 439
	virtual CBaseEntity *GetSoundscapeListener();  // line 442
	virtual Class_T Classify();  // line 444
	virtual void SetAnimation( PLAYER_ANIM );  // line 445
	virtual void OnMainActivityComplete( $_178, $_178 );  // line 446
	virtual void OnMainActivityInterrupted( $_178, $_178 );  // line 447
	void SetWeaponAnimType( const char * );  // line 448
	virtual void ImpulseCommands();  // line 451
	virtual void CheatImpulseCommands( int );  // line 452
	virtual bool ClientCommand( const CCommand & );  // line 453
	void NotifySinglePlayerGameEnding();  // line 455
	bool IsSinglePlayerGameEnding();  // line 456
	virtual bool StartObserverMode( int );  // line 459
	virtual void StopObserverMode();  // line 460
	virtual bool ModeWantsSpectatorGUI( int );  // line 461
	virtual bool SetObserverMode( int );  // line 462
	virtual int GetObserverMode();  // line 463
	virtual bool SetObserverTarget( CBaseEntity * );  // line 464
	virtual void ObserverUse( bool );  // line 465
	virtual CBaseEntity *GetObserverTarget();  // line 466
	virtual CBaseEntity *FindNextObserverTarget( bool );  // line 467
	virtual int GetNextObserverSearchStartPoint( bool );  // line 468
	virtual bool PassesObserverFilter( const CBaseEntity * );  // line 469
	virtual bool IsValidObserverTarget( CBaseEntity * );  // line 470
	virtual void CheckObserverSettings();  // line 471
	virtual void JumptoPosition( const Vector &, const QAngle & );  // line 472
	virtual void ForceObserverMode( int );  // line 473
	virtual void ResetObserverMode();  // line 474
	virtual void ValidateCurrentObserverTarget();  // line 475
	virtual void AttemptToExitFreezeCam();  // line 476
	virtual bool StartReplayMode( float, float, int );  // line 478
	virtual void StopReplayMode();  // line 479
	virtual int GetDelayTicks();  // line 480
	virtual int GetReplayEntity();  // line 481
	CLogicPlayerProxy *GetPlayerProxy();  // line 483
	void FirePlayerProxyOutput( const char *, variant_t, CBaseEntity *, CBaseEntity * );  // line 484
	virtual void CreateCorpse();  // line 486
	virtual CBaseEntity *EntSelectSpawnPoint();  // line 487
	virtual bool IsInAVehicle() const;  // line 490
	bool CanEnterVehicle( IServerVehicle *, int );  // line 491
	virtual bool GetInVehicle( IServerVehicle *, int );  // line 492
	virtual void LeaveVehicle( const Vector &, const QAngle & );  // line 493
	int GetVehicleAnalogControlBias();  // line 494
	void SetVehicleAnalogControlBias( int );  // line 495
	virtual void OnVehicleStart();  // line 498
	virtual void OnVehicleEnd( Vector & );  // line 499
	virtual IServerVehicle *GetVehicle();  // line 500
	virtual CBaseEntity *GetVehicleEntity();  // line 501
	bool UsingStandardWeaponsInVehicle();  // line 502
	void AddPoints( int, bool );  // line 504
	void AddPointsToTeam( int, bool );  // line 505
	virtual bool BumpWeapon( CBaseCombatWeapon * );  // line 506
	virtual bool RemovePlayerItem( CBaseCombatWeapon * );  // line 507
	CBaseEntity *HasNamedPlayerItem( const char * );  // line 508
	bool HasWeapons();  // line 509
	virtual void SelectLastItem();  // line 510
	virtual void SelectItem( const char *, int );  // line 511
	void ItemPreFrame();  // line 512
	virtual void ItemPostFrame();  // line 513
	virtual CBaseEntity *GiveNamedItem( const char *, int, bool );  // line 514
	void EnableControl( bool );  // line 515
	virtual void CheckTrainUpdate();  // line 516
	void AbortReload();  // line 517
	void SendAmmoUpdate();  // line 519
	void WaterMove();  // line 521
	float GetWaterJumpTime() const;  // line 522
	void SetWaterJumpTime( float );  // line 523
	float GetSwimSoundTime() const;  // line 524
	void SetSwimSoundTime( float );  // line 525
	virtual void SetPlayerUnderwater( bool );  // line 527
	void UpdateUnderwaterState();  // line 528
	bool IsPlayerUnderwater();  // line 529
	virtual bool CanBreatheUnderwater() const;  // line 531
	virtual bool CanRecoverCurrentDrowningDamage() const;  // line 532
	virtual void PlayerUse();  // line 533
	virtual void PlayUseDenySound();  // line 534
	virtual CBaseEntity *FindUseEntity();  // line 536
	virtual bool IsUseableEntity( CBaseEntity *, unsigned int );  // line 537
	bool ClearUseEntity();  // line 538
	CBaseEntity *DoubleCheckUseNPC( CBaseEntity *, const Vector &, const Vector & );  // line 539
	static bool CanPickupObject( CBaseEntity *, float, float );  // line 544
	virtual void PickupObject( CBaseEntity *, bool );  // line 545
	virtual void ForceDropOfCarriedPhysObjects( CBaseEntity * );  // line 546
	virtual float GetHeldObjectMass( IPhysicsObject * );  // line 547
	void CheckSuitUpdate();  // line 549
	void SetSuitUpdate( char *, int, int );  // line 550
	virtual void UpdateGeigerCounter();  // line 551
	void CheckTimeBasedDamage();  // line 552
	void ResetAutoaim();  // line 554
	virtual Vector GetAutoaimVector( float );  // line 556
	virtual Vector GetAutoaimVector( float, float );  // line 557
	virtual Vector GetAutoaimVector( float, float, float, AimResults * );  // line 558
	virtual void GetAutoaimVector( autoaim_params_t & );  // line 559
	virtual bool ShouldAutoaim();  // line 561
	void SetTargetInfo( Vector &, float );  // line 562
	void SetViewEntity( CBaseEntity * );  // line 564
	CBaseEntity *GetViewEntity();  // line 565
	virtual void ForceClientDllUpdate();  // line 567
	void DeathMessage( CBaseEntity * );  // line 569
	virtual void ProcessUsercmds( CUserCmd *, int, int, int, bool );  // line 572
	bool HasQueuedUsercmds() const;  // line 573
	void AvoidPhysicsProps( CUserCmd * );  // line 575
	virtual void PlayerRunCommand( CUserCmd *, IMoveHelper * );  // line 579
	void RunNullCommand();  // line 580
	virtual void ChangeTeam( int );  // line 583
	virtual void ChangeTeam( int, bool, bool );  // line 584
	virtual bool CanHearAndReadChatFrom( CBasePlayer * );  // line 587
	virtual bool CanSpeak();  // line 588
	audioparams_t &GetAudioParams();  // line 590
	virtual void ModifyOrAppendPlayerCriteria( ResponseRules::CriteriaSet & );  // line 592
	const QAngle &GetPunchAngle();  // line 594
	void SetPunchAngle( const QAngle & );  // line 595
	void SetPunchAngle( int, float );  // line 596
	virtual void DoMuzzleFlash();  // line 598
	virtual CNavArea *GetLastKnownArea() const;  // line 600
	const char *GetLastKnownPlaceName() const;  // line 601
	virtual void CheckChatText( char *, int );  // line 603
	virtual void CreateRagdollEntity();  // line 605
	virtual void HandleAnimEvent( animevent_t * );  // line 607
	CBaseEntity *GetTonemapController() const;  // line 609
	virtual bool ShouldAnnounceAchievement();  // line 614
	bool IsSplitScreenPartner( CBasePlayer * );  // line 617
	void SetSplitScreenPlayer( bool, CBasePlayer * );  // line 618
	bool IsSplitScreenPlayer() const;  // line 619
	CBasePlayer *GetSplitScreenPlayerOwner();  // line 620
	bool IsSplitScreenUserOnEdict( edict_t * );  // line 621
	int GetSplitScreenPlayerSlot();  // line 622
	void AddSplitScreenPlayer( CBasePlayer * );  // line 624
	void RemoveSplitScreenPlayer( CBasePlayer * );  // line 625
	CUtlVector<CHandle<CBasePlayer>,CUtlMemory<CHandle<CBasePlayer>, int> > &GetSplitScreenPlayers();  // line 626
	virtual bool EnsureSplitScreenTeam();  // line 629
	virtual void ForceChangeTeam( int );  // line 631
	void UpdateFXVolume();  // line 633
	void SetupVPhysicsShadow( const Vector &, const Vector &, CPhysCollide *, const char *, CPhysCollide *, const char * );  // line 636
	IPhysicsPlayerController *GetPhysicsController();  // line 637
	virtual void VPhysicsCollision( int, gamevcollisionevent_t * );  // line 638
	virtual void VPhysicsUpdate( IPhysicsObject * );  // line 639
	virtual void VPhysicsShadowUpdate( IPhysicsObject * );  // line 640
	virtual bool IsFollowingPhysics();  // line 641
	bool IsRideablePhysics( IPhysicsObject * );  // line 642
	IPhysicsObject *GetGroundVPhysics();  // line 643
	virtual void Touch( CBaseEntity * );  // line 645
	void SetTouchedPhysics( bool );  // line 646
	bool TouchedPhysics();  // line 647
	virtual Vector GetSmoothedVelocity();  // line 648
	virtual void InitVCollision( const Vector &, const Vector & );  // line 650
	virtual void VPhysicsDestroyObject();  // line 651
	void SetVCollisionState( const Vector &, const Vector &, int );  // line 652
	void PostThinkVPhysics();  // line 653
	virtual void UpdatePhysicsShadowToCurrentPosition();  // line 654
	void UpdatePhysicsShadowToPosition( const Vector & );  // line 655
	void UpdateVPhysicsPosition( const Vector &, const Vector &, float );  // line 656
	virtual CHintSystem *Hints();  // line 659
	bool ShouldShowHints();  // line 660
	void SetShowHints( bool );  // line 661
	bool HintMessage( int, bool );  // line 662
	void HintMessage( const char * );  // line 663
	void StartHintTimer( int );  // line 664
	void StopHintTimer( int );  // line 665
	void RemoveHintTimer( int );  // line 666
	int FragCount() const;  // line 669
	int DeathCount() const;  // line 670
	bool IsConnected() const;  // line 671
	bool IsDisconnecting() const;  // line 672
	bool IsSuitEquipped() const;  // line 673
	int ArmorValue() const;  // line 674
	bool HUDNeedsRestart() const;  // line 675
	float MaxSpeed() const;  // line 676
	$_178 GetActivity() const;  // line 677
	void SetActivity( $_178 );  // line 678
	bool IsPlayerLockedInPlace() const;  // line 679
	bool IsObserver() const;  // line 680
	bool IsOnTarget() const;  // line 681
	float MuzzleFlashTime() const;  // line 682
	float PlayerDrownTime() const;  // line 683
	void SetPlayerLocked( int );  // line 685
	int GetPlayerLocked();  // line 686
	int GetObserverMode() const;  // line 688
	CBaseEntity *GetObserverTarget() const;  // line 689
	virtual bool IsReadyToPlay();  // line 692
	virtual bool IsReadyToSpawn();  // line 693
	virtual bool ShouldGainInstantSpawn();  // line 694
	virtual void ResetPerRoundStats();  // line 695
	void AllowInstantSpawn();  // line 696
	virtual void ResetScores();  // line 698
	void ResetFragCount();  // line 699
	void IncrementFragCount( int );  // line 700
	void ResetDeathCount();  // line 702
	void IncrementDeathCount( int );  // line 703
	void SetArmorValue( int );  // line 705
	void IncrementArmorValue( int, int );  // line 706
	void SetConnected( PlayerConnectedState );  // line 708
	virtual void EquipSuit( bool );  // line 709
	virtual void RemoveSuit();  // line 710
	void SetMaxSpeed( float );  // line 711
	void NotifyNearbyRadiationSource( float );  // line 713
	void SetAnimationExtension( const char * );  // line 715
	void SetAdditionalPVSOrigin( const Vector & );  // line 717
	void SetCameraPVSOrigin( const Vector & );  // line 718
	void SetMuzzleFlashTime( float );  // line 719
	void SetUseEntity( CBaseEntity * );  // line 720
	virtual CBaseEntity *GetUseEntity();  // line 721
	virtual CBaseEntity *GetPotentialUseEntity();  // line 722
	void SetPhysicsFlag( int, bool );  // line 725
	void AllowImmediateDecalPainting();  // line 727
	virtual void CommitSuicide( bool, bool );  // line 730
	virtual void CommitSuicide( const Vector &, bool, bool );  // line 731
	void ForceOrigin( const Vector & );  // line 734
	void SetTimeBase( float );  // line 737
	float GetTimeBase() const;  // line 738
	void SetLastUserCommand( const CUserCmd & );  // line 739
	const CUserCmd *GetLastUserCommand();  // line 740
	virtual bool IsBot() const;  // line 741
	bool IsPredictingWeapons() const;  // line 743
	int CurrentCommandNumber() const;  // line 744
	const CUserCmd *GetCurrentUserCommand() const;  // line 745
	int GetFOV();  // line 747
	int GetDefaultFOV() const;  // line 748
	int GetFOVForNetworking();  // line 749
	bool SetFOV( CBaseEntity *, int, float, int );  // line 750
	void SetDefaultFOV( int );  // line 751
	CBaseEntity *GetFOVOwner();  // line 752
	float GetFOVDistanceAdjustFactor();  // line 753
	float GetFOVDistanceAdjustFactorForNetworking();  // line 754
	int GetImpulse() const;  // line 756
	void ActivateMovementConstraint( CBaseEntity *, const Vector &, float, float, float, bool );  // line 759
	void DeactivateMovementConstraint();  // line 760
	void NotePlayerTalked();  // line 763
	float LastTimePlayerTalked();  // line 764
	void DisableButtons( int );  // line 766
	void EnableButtons( int );  // line 767
	void ForceButtons( int );  // line 768
	void UnforceButtons( int );  // line 769
	void InputSetHealth( inputdata_t & );  // line 774
	void InputSetHUDVisibility( inputdata_t & );  // line 775
	surfacedata_t *GetSurfaceData() const;  // line 777
	void SetLadderNormal( Vector );  // line 778
	void ClearImpulse();  // line 780
	virtual CAI_Expresser *GetExpresser();  // line 783
	void IncrementEFNoInterpParity();  // line 785
	int GetEFNoInterpParity() const;  // line 786
private:
	int GetCommandContextCount() const;  // line 791
	CCommandContext *GetCommandContext( int );  // line 792
	CCommandContext *AllocCommandContext();  // line 793
	void RemoveCommandContext( int );  // line 794
	void RemoveAllCommandContexts();  // line 795
	CCommandContext *RemoveAllCommandContextsExceptNewest();  // line 796
	void ReplaceContextCommands( CCommandContext *, CUserCmd *, int );  // line 797
	int DetermineSimulationTicks();  // line 799
	void AdjustPlayerTimeBase( int );  // line 800
public:
	int m_StuckLast; // +0x8dc  // line 805
	CBasePlayer::NetworkVar_m_Local m_Local; // +0x8e0  // line 811
	static int GetOffset_m_Local();  // line 811
	CBasePlayer::NetworkVar_m_PlayerFog m_PlayerFog; // +0xad4  // line 813
	static int GetOffset_m_PlayerFog();  // line 813
	void InitFogController();  // line 814
	void InputSetFogController( inputdata_t & );  // line 815
	void OnTonemapTriggerStartTouch( CTonemapTrigger * );  // line 817
	void OnTonemapTriggerEndTouch( CTonemapTrigger * );  // line 818
	CUtlVector<CHandle<CTonemapTrigger>,CUtlMemory<CHandle<CTonemapTrigger>, int> > m_hTriggerTonemapList; // +0xb10  // line 819
	CNetworkHandle( CPostProcessController, m_hPostProcessCtrl ); // +0xb24  // line 821
	CNetworkHandle( CColorCorrection, m_hColorCorrectionCtrl ); // +0xb28  // line 822
	void InitPostProcessController();  // line 823
	void InputSetPostProcessController( inputdata_t & );  // line 824
	void InitColorCorrectionController();  // line 825
	void InputSetColorCorrectionController( inputdata_t & );  // line 826
	CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> > m_hTriggerSoundscapeList; // +0xb2c  // line 831
	CBasePlayer::NetworkVar_pl pl; // +0xb40  // line 834
	static int GetOffset_pl();  // line 834
	int m_nButtons; // +0xb74  // line 851
	int m_afButtonPressed; // +0xb78  // line 852
	int m_afButtonReleased; // +0xb7c  // line 853
	int m_afButtonLast; // +0xb80  // line 854
	int m_afButtonDisabled; // +0xb84  // line 855
	int m_afButtonForced; // +0xb88  // line 856
	CNetworkVar( bool, m_fOnTarget ); // +0xb8c  // line 858
	char m_szAnimExtension[32]; // +0xb8d  // line 860
	int m_nUpdateRate; // +0xbb0  // line 862
	float m_fLerpTime; // +0xbb4  // line 863
	bool m_bLagCompensation; // +0xbb8  // line 864
	bool m_bPredictWeapons; // +0xbb9  // line 865
	float GetDeathTime();  // line 867
	void ClearZoomOwner();  // line 869
	void SetPreviouslyPredictedOrigin( const Vector & );  // line 871
	const Vector &GetPreviouslyPredictedOrigin() const;  // line 872
	float GetFOVTime();  // line 873
	void AdjustDrownDmg( int );  // line 875
private:
	$_178 m_Activity; // +0xbbc  // line 879
protected:
	virtual void CalcPlayerView( Vector &, QAngle &, float & );  // line 883
	void CalcVehicleView( IServerVehicle *, Vector &, QAngle &, float &, float &, float & );  // line 885
	void CalcObserverView( Vector &, QAngle &, float & );  // line 886
	void CalcViewModelView( const Vector &, const QAngle & );  // line 887
	Vector m_vecAdditionalPVSOrigin; // +0xbc0  // line 893
	Vector m_vecCameraPVSOrigin; // +0xbcc  // line 895
	CNetworkHandle( CBaseEntity, m_hUseEntity ); // +0xbd8  // line 897
	int m_iTrain; // +0xbdc  // line 899
	float m_iRespawnFrames; // +0xbe0  // line 901
	CNetworkVar( unsigned int, m_afPhysicsFlags ); // +0xbe4  // line 902
	CNetworkHandle( CBaseEntity, m_hVehicle ); // +0xbe8  // line 905
	int m_iVehicleAnalogBias; // +0xbec  // line 907
	void UpdateButtonState( int );  // line 909
	bool m_bPauseBonusProgress; // +0xbf0  // line 911
	CNetworkVar( int, m_iBonusProgress ); // +0xbf4  // line 912
	CNetworkVar( int, m_iBonusChallenge ); // +0xbf8  // line 913
	int m_lastDamageAmount; // +0xbfc  // line 915
	float m_fTimeLastHurt; // +0xc00  // line 916
	Vector m_DmgOrigin; // +0xc04  // line 918
	float m_DmgTake; // +0xc10  // line 919
	float m_DmgSave; // +0xc14  // line 920
	int m_bitsDamageType; // +0xc18  // line 921
	int m_bitsHUDDamage; // +0xc1c  // line 922
	CNetworkVar( float, m_flDeathTime ); // +0xc20  // line 924
	float m_flDeathAnimTime; // +0xc24  // line 925
	CNetworkVar( int, m_iObserverMode ); // +0xc28  // line 927
	CNetworkVar( int, m_iFOV ); // +0xc2c  // line 928
	CNetworkVar( int, m_iDefaultFOV ); // +0xc30  // line 929
	CNetworkVar( int, m_iFOVStart ); // +0xc34  // line 930
	CNetworkVar( float, m_flFOVTime ); // +0xc38  // line 931
	int m_iObserverLastMode; // +0xc3c  // line 933
	CNetworkHandle( CBaseEntity, m_hObserverTarget ); // +0xc40  // line 934
	bool m_bForcedObserverMode; // +0xc44  // line 935
	CNetworkHandle( CBaseEntity, m_hZoomOwner ); // +0xc48  // line 937
	float m_tbdPrev; // +0xc4c  // line 940
	int m_idrowndmg; // +0xc50  // line 941
	int m_idrownrestored; // +0xc54  // line 942
	int m_nPoisonDmg; // +0xc58  // line 943
	int m_nPoisonRestored; // +0xc5c  // line 944
	uint8 m_rgbTimeBasedDamage[8]; // +0xc60  // line 946
	CNetworkVar( int, m_vphysicsCollisionState ); // +0xc68  // line 949
	virtual int SpawnArmorValue() const;  // line 951
	float m_fNextSuicideTime; // +0xc6c  // line 953
	int m_iSuicideCustomKillFlags; // +0xc70  // line 954
	float m_fDelay; // +0xc74  // line 957
	float m_fReplayEnd; // +0xc78  // line 958
	int m_iReplayEntity; // +0xc7c  // line 959
	virtual void UpdateTonemapController();  // line 961
	CNetworkHandle( CBaseEntity, m_hTonemapController ); // +0xc80  // line 962
private:
	void HandleFuncTrain();  // line 965
	CUtlVector<CCommandContext,CUtlMemory<CCommandContext, int> > m_CommandContext; // +0xc84  // line 969
protected:
	IPhysicsPlayerController *m_pPhysicsController; // +0xc98  // line 973
	IPhysicsObject *m_pShadowStand; // +0xc9c  // line 974
	IPhysicsObject *m_pShadowCrouch; // +0xca0  // line 975
	Vector m_oldOrigin; // +0xca4  // line 976
	Vector m_vecSmoothedVelocity; // +0xcb0  // line 977
	bool m_touchedPhysObject; // +0xcbc  // line 978
	bool m_bPhysicsWasFrozen; // +0xcbd  // line 979
private:
	int m_iPlayerSound; // +0xcc0  // line 983
	int m_iTargetVolume; // +0xcc4  // line 984
	int m_rgItems[5]; // +0xcc8  // line 986
	float m_flSwimTime; // +0xcdc  // line 989
	float m_flDuckTime; // +0xce0  // line 990
	float m_flDuckJumpTime; // +0xce4  // line 991
	float m_flSuitUpdate; // +0xce8  // line 993
	int m_rgSuitPlayList[4]; // +0xcec  // line 994
	int m_iSuitPlayNext; // +0xcfc  // line 995
	int m_rgiSuitNoRepeat[32]; // +0xd00  // line 996
	float m_rgflSuitNoRepeatTime[32]; // +0xd80  // line 997
	float m_flgeigerRange; // +0xe00  // line 999
	float m_flgeigerDelay; // +0xe04  // line 1000
	int m_igeigerRangePrev; // +0xe08  // line 1001
	bool m_fInitHUD; // +0xe0c  // line 1003
	bool m_fGameHUDInitialized; // +0xe0d  // line 1004
	bool m_fWeapon; // +0xe0e  // line 1005
	int m_iUpdateTime; // +0xe10  // line 1007
	int m_iClientBattery; // +0xe14  // line 1008
	QAngle m_vecAutoAim; // +0xe18  // line 1011
	int m_iFrags; // +0xe24  // line 1013
	int m_iDeaths; // +0xe28  // line 1014
	float m_flNextDecalTime; // +0xe2c  // line 1016
	PlayerConnectedState m_iConnected; // +0xe30  // line 1022
	CNetworkVar( int, m_ArmorValue ); // +0xe34  // line 1026
	float m_AirFinished; // +0xe38  // line 1027
	float m_PainFinished; // +0xe3c  // line 1028
	int m_iPlayerLocked; // +0xe40  // line 1031
	CSimpleSimTimer m_AutoaimTimer; // +0xe44  // line 1033
protected:
	CBasePlayer::NetworkVar_m_hViewModel m_hViewModel; // +0xe48  // line 1038
	CUserCmd m_LastCmd; // +0xe50  // line 1041
	CUserCmd *m_pCurrentCommand; // +0xeac  // line 1042
	float m_flStepSoundTime; // +0xeb0  // line 1044
	bool m_bAllowInstantSpawn; // +0xeb4  // line 1046
private:
	CNetworkVar( float, m_flMaxspeed ); // +0xeb8  // line 1051
	CNetworkVar( int, m_ladderSurfaceProps ); // +0xebc  // line 1052
	CNetworkVector( Vector, m_vecLadderNormal ); // +0xec0  // line 1053
protected:
	float m_flWaterJumpTime; // +0xecc  // line 1057
	Vector m_vecWaterJumpVel; // +0xed0  // line 1058
	int m_nImpulse; // +0xedc  // line 1059
	float m_flSwimSoundTime; // +0xee0  // line 1060
private:
	float m_flFlashTime; // +0xee4  // line 1064
	int m_nDrownDmgRate; // +0xee8  // line 1065
	int m_nNumCrouches; // +0xeec  // line 1067
	bool m_bDuckToggled; // +0xef0  // line 1068
public:
	bool GetToggledDuckState();  // line 1071
	void ToggleDuck();  // line 1072
	float GetStickDist();  // line 1073
	float m_flForwardMove; // +0xef4  // line 1075
	float m_flSideMove; // +0xef8  // line 1076
	int m_nNumCrateHudHints; // +0xefc  // line 1077
private:
	Vector m_vForcedOrigin; // +0xf00  // line 1082
	bool m_bForceOrigin; // +0xf0c  // line 1083
	CNetworkVar( int, m_nTickBase ); // +0xf10  // line 1086
	bool m_bGamePaused; // +0xf14  // line 1088
	float m_fLastPlayerTalkTime; // +0xf18  // line 1089
	CNetworkVar( CHandle<CBaseCombatWeapon>, m_hLastWeapon ); // +0xf1c  // line 1091
	CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> > m_SimulatedByThisPlayer; // +0xf20  // line 1094
	float m_flOldPlayerZ; // +0xf34  // line 1097
	float m_flOldPlayerViewOffsetZ; // +0xf38  // line 1098
	bool m_bPlayerUnderwater; // +0xf3c  // line 1100
	CNetworkHandle( CBaseEntity, m_hViewEntity ); // +0xf40  // line 1102
	CNetworkHandle( CBaseEntity, m_hConstraintEntity ); // +0xf44  // line 1105
	CNetworkVector( Vector, m_vecConstraintCenter ); // +0xf48  // line 1106
	CNetworkVar( float, m_flConstraintRadius ); // +0xf54  // line 1107
	CNetworkVar( float, m_flConstraintWidth ); // +0xf58  // line 1108
	CNetworkVar( float, m_flConstraintSpeedFactor ); // +0xf5c  // line 1109
	CNetworkVar( bool, m_bConstraintPastRadius ); // +0xf60  // line 1110
	char m_szNetname[32]; // +0xf61  // line 1119
protected:
	bool IsDucked() const;  // line 1133
	bool IsDucking() const;  // line 1134
	float GetStepSize() const;  // line 1135
	CNetworkVar( float, m_flLaggedMovementValue ); // +0xf84  // line 1137
	Vector m_vNewVPhysicsPosition; // +0xf88  // line 1144
	Vector m_vNewVPhysicsVelocity; // +0xf94  // line 1145
	Vector m_vecVehicleViewOrigin; // +0xfa0  // line 1148
	QAngle m_vecVehicleViewAngles; // +0xfac  // line 1149
	float m_flVehicleViewFOV; // +0xfb8  // line 1150
	int m_nVehicleViewSavedFrame; // +0xfbc  // line 1151
	Vector m_vecPreviouslyPredictedOrigin; // +0xfc0  // line 1153
	int m_nBodyPitchPoseParam; // +0xfcc  // line 1154
	CNavArea *m_lastNavArea; // +0xfd0  // line 1157
	CBasePlayer::NetworkVar_m_szLastPlaceName m_szLastPlaceName; // +0xfd4  // line 1158
	char m_szNetworkIDString[64]; // +0xfe6  // line 1160
	CPlayerInfo m_PlayerInfo; // +0x1028  // line 1161
	int m_surfaceProps; // +0x1034  // line 1164
	surfacedata_t *m_pSurfaceData; // +0x1038  // line 1165
	float m_surfaceFriction; // +0x103c  // line 1166
	char m_chTextureType; // +0x1040  // line 1167
	char m_chPreviousTextureType; // +0x1041  // line 1168
	bool m_bSinglePlayerGameEnding; // +0x1042  // line 1170
	CNetworkVar( int, m_ubEFNoInterpParity ); // +0x1044  // line 1172
	EHANDLE m_hPlayerProxy; // +0x1048  // line 1174
public:
	float GetLaggedMovementValue();  // line 1178
	void SetLaggedMovementValue( float );  // line 1179
	bool IsAutoKickDisabled() const;  // line 1181
	void DisableAutoKick( bool );  // line 1182
	void DumpPerfToRecipient( CBasePlayer *, int );  // line 1184
	virtual CBaseEntity *FindEntityClassForward( char * );  // line 1187
	virtual CBaseEntity *FindEntityForward( bool );  // line 1188
	virtual CBaseEntity *FindPickerEntityClass( char * );  // line 1189
	virtual CBaseEntity *FindPickerEntity();  // line 1190
	virtual CAI_Node *FindPickerAINode( int );  // line 1191
	virtual CAI_Link *FindPickerAILink();  // line 1192
	void PrepareForFullUpdate();  // line 1195
	virtual void OnSpeak( CBasePlayer *, const char *, float );  // line 1197
	virtual void OnVoiceTransmit();  // line 1200
private:
	bool m_autoKickDisabled; // +0x104c  // line 1204
public:
	// game/server/player.h:1207 sizeof=0xb8 (i386)
	struct StepSoundCache_t
	{
	public:
		StepSoundCache_t();  // line 1208
		CSoundParameters m_SoundParameters; // +0x0  // line 1209
		short unsigned int m_usSoundNameIndex; // +0xb4  // line 1210
	};  // line 1207
private:
	CBasePlayer::StepSoundCache_t m_StepSoundCache[2]; // +0x1050  // line 1213
	CUtlLinkedList<CPlayerSimInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerSimInfo, short unsigned int>, short unsigned int> > m_vecPlayerSimInfo; // +0x11c0  // line 1215
	CUtlLinkedList<CPlayerCmdInfo,short unsigned int,false,short unsigned int,CUtlMemory<UtlLinkedListElem_t<CPlayerCmdInfo, short unsigned int>, short unsigned int> > m_vecPlayerCmdInfo; // +0x11dc  // line 1216
	Vector m_movementCollisionNormal; // +0x11f8  // line 1219
	Vector m_groundNormal; // +0x1204  // line 1220
	CHandle<CBaseCombatCharacter> m_stuckCharacter; // +0x1210  // line 1221
	bool m_bSplitScreenPlayer; // +0x1214  // line 1224
	CHandle<CBasePlayer> m_hSplitOwner; // +0x1218  // line 1225
	CUtlVector<CHandle<CBasePlayer>,CUtlMemory<CHandle<CBasePlayer>, int> > m_hSplitScreenPlayers; // +0x121c  // line 1227
	float GetAutoaimScore( const Vector &, const Vector &, const Vector &, CBaseEntity *, float, CBaseCombatWeapon * );  // line 1230
	QAngle AutoaimDeflection( Vector &, autoaim_params_t & );  // line 1231
public:
	virtual unsigned int PlayerSolidMask( bool ) const;  // line 1234
};

// public/PlayerState.h:22
// public/PlayerState.h:22 sizeof=0x34 (i386)
struct CPlayerState
{
public:
	int (**_vptr$CPlayerState)(); // +0x0  // line 0
	virtual ~CPlayerState();  // line 29
	CNetworkVar( bool, deadflag ); // +0x4  // line 32
	QAngle v_angle; // +0x8  // line 34
	string_t netname; // +0x14  // line 40
	int fixangle; // +0x18  // line 42
	QAngle anglechange; // +0x1c  // line 44
	bool hltv; // +0x28  // line 46
	bool replay; // +0x29  // line 47
	int frags; // +0x2c  // line 48
	int deaths; // +0x30  // line 49
};

// game/server/basecombatcharacter.h:123
// game/server/basecombatcharacter.h:123 sizeof=0x8dc (i386)
struct CBaseCombatCharacter : public CBaseFlex
{
public:
	CBaseCombatCharacter();  // line 127
	virtual ~CBaseCombatCharacter();  // line 128
	virtual void Spawn();  // line 136
	virtual void Precache();  // line 143
	virtual int Restore( IRestore & );  // line 145
	virtual const impactdamagetable_t &GetPhysicsImpactDamageTable();  // line 147
	virtual int TakeHealth( float, int );  // line 149
	void CauseDeath( const CTakeDamageInfo & );  // line 150
	virtual bool FVisible( CBaseEntity *, int, CBaseEntity ** );  // line 152
	virtual bool FVisible( const Vector &, int, CBaseEntity ** );  // line 153
	static void ResetVisibilityCache( CBaseCombatCharacter * );  // line 154
	virtual bool FVisibleThroughPortal( const CPortal_Base2D *, CBaseEntity *, int, CBaseEntity ** );  // line 157
	virtual bool FInViewCone( CBaseEntity * );  // line 160
	virtual bool FInViewCone( const Vector & );  // line 161
	virtual CPortal_Base2D *FInViewConeThroughPortal( CBaseEntity * );  // line 164
	virtual CPortal_Base2D *FInViewConeThroughPortal( const Vector & );  // line 165
	virtual bool FInAimCone( CBaseEntity * );  // line 168
	virtual bool FInAimCone( const Vector & );  // line 169
	virtual bool ShouldShootMissTarget( CBaseCombatCharacter * );  // line 171
	virtual CBaseEntity *FindMissTarget();  // line 172
	bool DispatchInteraction( int, void *, CBaseCombatCharacter * );  // line 175
	virtual bool HandleInteraction( int, void *, CBaseCombatCharacter * );  // line 176
	virtual QAngle BodyAngles();  // line 178
	virtual Vector BodyDirection2D();  // line 179
	virtual Vector BodyDirection3D();  // line 180
	virtual Vector HeadDirection2D();  // line 181
	virtual Vector HeadDirection3D();  // line 182
	virtual Vector EyeDirection2D();  // line 183
	virtual Vector EyeDirection3D();  // line 184
	virtual void SetTransmit( CCheckTransmitInfo *, bool );  // line 186
	void OnFogTriggerStartTouch( CBaseEntity * );  // line 192
	void OnFogTriggerEndTouch( CBaseEntity * );  // line 193
	CBaseEntity *GetFogTrigger();  // line 194
	virtual bool IsHiddenByFog( const Vector & ) const;  // line 196
	virtual bool IsHiddenByFog( CBaseEntity * ) const;  // line 197
	virtual bool IsHiddenByFog( float ) const;  // line 198
	virtual float GetFogObscuredRatio( const Vector & ) const;  // line 199
	virtual float GetFogObscuredRatio( CBaseEntity * ) const;  // line 200
	virtual float GetFogObscuredRatio( float ) const;  // line 201
	virtual bool GetFogParams( fogparams_t * ) const;  // line 202
	// game/server/basecombatcharacter.h:207
	enum FieldOfViewCheckType
	{
		USE_FOV = 0,
		DISREGARD_FOV = 1,
	};  // line 207
	bool IsAbleToSee( const CBaseEntity *, CBaseCombatCharacter::FieldOfViewCheckType );  // line 210
	bool IsAbleToSee( CBaseCombatCharacter *, CBaseCombatCharacter::FieldOfViewCheckType );  // line 211
	virtual bool IsLookingTowards( const CBaseEntity *, float ) const;  // line 213
	virtual bool IsLookingTowards( const Vector &, float ) const;  // line 214
	virtual bool IsInFieldOfView( CBaseEntity * ) const;  // line 216
	virtual bool IsInFieldOfView( const Vector & ) const;  // line 217
	// game/server/basecombatcharacter.h:219
	enum LineOfSightCheckType
	{
		IGNORE_NOTHING = 0,
		IGNORE_ACTORS = 1,
	};  // line 219
	virtual bool IsLineOfSightClear( CBaseEntity *, CBaseCombatCharacter::LineOfSightCheckType ) const;  // line 224
	virtual bool IsLineOfSightClear( const Vector &, CBaseCombatCharacter::LineOfSightCheckType, CBaseEntity * ) const;  // line 225
	void PlayFootstepSound( const Vector &, bool, bool, bool, bool );  // line 230
	virtual void OnFootstep( const Vector &, bool, bool, bool, bool );  // line 231
	virtual int GiveAmmo( int, int, bool );  // line 236
	int GiveAmmo( int, const char *, bool );  // line 237
	void RemoveAmmo( int, int );  // line 238
	void RemoveAmmo( int, const char * );  // line 239
	void RemoveAllAmmo();  // line 240
	int GetAmmoCount( int ) const;  // line 241
	int GetAmmoCount( char * ) const;  // line 242
	virtual $_178 NPC_TranslateActivity( $_178 );  // line 244
	CBaseCombatWeapon *Weapon_Create( const char * );  // line 249
	virtual $_178 Weapon_TranslateActivity( $_178, bool * );  // line 250
	void Weapon_SetActivity( $_178, float );  // line 251
	virtual void Weapon_FrameUpdate();  // line 252
	virtual void Weapon_HandleAnimEvent( animevent_t * );  // line 253
	virtual CBaseCombatWeapon *Weapon_OwnsThisType( const char *, int ) const;  // line 254
	virtual int Weapon_GetSlot( const char *, int ) const;  // line 255
	virtual bool Weapon_CanUse( CBaseCombatWeapon * );  // line 256
	virtual void Weapon_Equip( CBaseCombatWeapon * );  // line 257
	virtual bool Weapon_EquipAmmoOnly( CBaseCombatWeapon * );  // line 258
	bool Weapon_Detach( CBaseCombatWeapon * );  // line 259
	virtual void Weapon_Drop( CBaseCombatWeapon *, const Vector *, const Vector * );  // line 260
	virtual bool Weapon_Switch( CBaseCombatWeapon *, int );  // line 261
	virtual Vector Weapon_ShootPosition();  // line 262
	bool Weapon_IsOnGround( CBaseCombatWeapon * );  // line 263
	CBaseEntity *Weapon_FindUsable( const Vector & );  // line 264
	virtual bool Weapon_CanSwitchTo( CBaseCombatWeapon * );  // line 265
	virtual bool Weapon_SlotOccupied( CBaseCombatWeapon * );  // line 266
	virtual CBaseCombatWeapon *Weapon_GetSlot( int ) const;  // line 267
	CBaseCombatWeapon *Weapon_GetWpnForAmmo( int );  // line 268
	void Weapon_DropAll( bool );  // line 272
	virtual bool AddPlayerItem( CBaseCombatWeapon * );  // line 274
	virtual bool RemovePlayerItem( CBaseCombatWeapon * );  // line 275
	virtual bool CanBecomeServerRagdoll();  // line 277
	virtual int OnTakeDamage( const CTakeDamageInfo & );  // line 283
	virtual int OnTakeDamage_Alive( const CTakeDamageInfo & );  // line 286
	virtual int OnTakeDamage_Dying( const CTakeDamageInfo & );  // line 287
	virtual int OnTakeDamage_Dead( const CTakeDamageInfo & );  // line 288
	virtual void OnFriendDamaged( CBaseCombatCharacter *, CBaseEntity * );  // line 290
	virtual void NotifyFriendsOfDamage( CBaseEntity * );  // line 291
	virtual void OnPlayerKilledOther( CBaseEntity *, const CTakeDamageInfo & );  // line 293
	virtual Vector CalcDeathForceVector( const CTakeDamageInfo & );  // line 296
	virtual int BloodColor();  // line 298
	virtual $_178 GetDeathActivity();  // line 299
	virtual bool CorpseGib( const CTakeDamageInfo & );  // line 301
	virtual void CorpseFade();  // line 302
	virtual bool HasHumanGibs();  // line 303
	virtual bool HasAlienGibs();  // line 304
	virtual bool ShouldGib( const CTakeDamageInfo & );  // line 305
	float GetDamageAccumulator();  // line 307
	int GetDamageCount();  // line 308
	virtual void Event_Killed( const CTakeDamageInfo & );  // line 311
	virtual bool ShouldDropActiveWeaponWhenKilled();  // line 312
	void InputKilledNPC( inputdata_t & );  // line 315
	virtual void OnKilledNPC( CBaseCombatCharacter * );  // line 316
	virtual bool Event_Gibbed( const CTakeDamageInfo & );  // line 321
	virtual void Event_Dying();  // line 323
	virtual bool BecomeRagdoll( const CTakeDamageInfo &, const Vector & );  // line 326
	virtual void FixupBurningServerRagdoll( CBaseEntity * );  // line 327
	virtual bool BecomeRagdollBoogie( CBaseEntity *, const Vector &, float, int );  // line 329
	CBaseEntity *FindHealthItem( const Vector &, const Vector & );  // line 331
	virtual CBaseEntity *CheckTraceHullAttack( float, const Vector &, const Vector &, float, int, float, bool );  // line 334
	virtual CBaseEntity *CheckTraceHullAttack( const Vector &, const Vector &, const Vector &, const Vector &, float, int, float, bool );  // line 335
	virtual CBaseCombatCharacter *MyCombatCharacterPointer();  // line 337
	virtual void VPhysicsShadowCollision( int, gamevcollisionevent_t * );  // line 340
	virtual void VPhysicsUpdate( IPhysicsObject * );  // line 341
	float CalculatePhysicsStressDamage( vphysics_objectstress_t *, IPhysicsObject * );  // line 342
	void ApplyStressDamage( IPhysicsObject *, bool );  // line 343
	virtual void PushawayTouch( CBaseEntity * );  // line 345
	void SetImpactEnergyScale( float );  // line 347
	virtual void UpdateOnRemove();  // line 349
	virtual Disposition_t IRelationType( CBaseEntity * );  // line 351
	virtual int IRelationPriority( CBaseEntity * );  // line 352
	virtual void SetLightingOriginRelative( CBaseEntity * );  // line 354
protected:
	Relationship_t *FindEntityRelationship( CBaseEntity * );  // line 357
public:
	virtual bool IsInAVehicle() const;  // line 362
	virtual IServerVehicle *GetVehicle();  // line 363
	virtual CBaseEntity *GetVehicleEntity();  // line 364
	virtual bool ExitVehicle();  // line 365
	void SetBloodColor( int );  // line 368
	CBaseCombatWeapon *GetActiveWeapon() const;  // line 371
	int WeaponCount() const;  // line 372
	CBaseCombatWeapon *GetWeapon( int ) const;  // line 373
	bool RemoveWeapon( CBaseCombatWeapon * );  // line 374
	void RemoveAllWeapons();  // line 375
	WeaponProficiency_t GetCurrentWeaponProficiency();  // line 376
	void SetCurrentWeaponProficiency( WeaponProficiency_t );  // line 377
	virtual WeaponProficiency_t CalcWeaponProficiency( CBaseCombatWeapon * );  // line 378
	virtual Vector GetAttackSpread( CBaseCombatWeapon *, CBaseEntity * );  // line 379
	virtual float GetSpreadBias( CBaseCombatWeapon *, CBaseEntity * );  // line 380
	virtual void DoMuzzleFlash();  // line 381
	static void InitInteractionSystem();  // line 384
	static void SetDefaultFactionRelationship( int, int, Disposition_t, int );  // line 387
	Disposition_t GetFactionRelationshipDisposition( int );  // line 388
	static void AllocateDefaultRelationships();  // line 389
	static void AllocateDefaultFactionRelationships();  // line 390
	static void SetDefaultRelationship( Class_T, Class_T, Disposition_t, int );  // line 391
	Disposition_t GetDefaultRelationshipDisposition( Class_T );  // line 392
	virtual void AddEntityRelationship( CBaseEntity *, Disposition_t, int );  // line 393
	virtual bool RemoveEntityRelationship( CBaseEntity * );  // line 394
	virtual void AddClassRelationship( Class_T, Disposition_t, int );  // line 395
	virtual void AddFactionRelationship( int, Disposition_t, int );  // line 396
	static int GetNumFactions();  // line 399
	static CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> > *GetEntitiesInFaction( int );  // line 400
	int GetFaction() const;  // line 401
	virtual void ChangeFaction( int );  // line 402
	Hull_t GetHullType() const;  // line 405
	void SetHullType( Hull_t );  // line 406
	void SetAmmoCount( int, int );  // line 411
	void SetActiveWeapon( CBaseCombatWeapon * );  // line 415
	void ClearActiveWeapon();  // line 416
	virtual void OnChangeActiveWeapon( CBaseCombatWeapon *, CBaseCombatWeapon * );  // line 417
	bool SwitchToNextBestWeapon( CBaseCombatWeapon * );  // line 420
	void SetRelationshipString( string_t );  // line 423
	float GetNextAttack() const;  // line 425
	void SetNextAttack( float );  // line 426
	bool m_bForceServerRagdoll; // +0x708  // line 428
	bool IsAllowedToPickupWeapons();  // line 431
	void SetPreventWeaponPickup( bool );  // line 432
	bool m_bPreventWeaponPickup; // +0x709  // line 433
	virtual CNavArea *GetLastKnownArea() const;  // line 435
	virtual bool IsAreaTraversable( const CNavArea * ) const;  // line 436
	virtual void ClearLastKnownArea();  // line 437
	virtual void UpdateLastKnownArea();  // line 438
	virtual void OnNavAreaChanged( CNavArea *, CNavArea * );  // line 439
	virtual void OnNavAreaRemoved( CNavArea * );  // line 440
	virtual void OnPursuedBy( INextBot * );  // line 445
	int LastHitGroup() const;  // line 450
protected:
	void SetLastHitGroup( int );  // line 452
public:
	CNetworkVar( float, m_flNextAttack ); // +0x70c  // line 455
private:
	Hull_t m_eHull; // +0x710  // line 458
protected:
	int m_bloodColor; // +0x714  // line 461
	float m_flFieldOfView; // +0x718  // line 466
	Vector m_HackedGunPos; // +0x71c  // line 467
	string_t m_RelationshipString; // +0x728  // line 468
	float m_impactEnergyScale; // +0x72c  // line 469
	uint8 m_weaponIDToIndex[13]; // +0x730  // line 471
public:
	static int GetInteractionID();  // line 474
protected:
	bool ComputeLOS( const Vector &, const Vector & ) const;  // line 478
private:
	bool ComputeTargetIsInDarkness( const Vector &, CNavArea *, const Vector & ) const;  // line 480
protected:
	void ThrowDirForWeaponStrip( CBaseCombatWeapon *, const Vector &, Vector * );  // line 484
	void DropWeaponForWeaponStrip( CBaseCombatWeapon *, const Vector &, const QAngle &, float );  // line 485
	static int m_lastInteraction;  // line 490
	static Relationship_t **m_DefaultRelationship;  // line 491
	static Relationship_t **m_FactionRelationship;  // line 492
	static CUtlVector<CUtlVector<CHandle<CBaseEntity>, CUtlMemory<CHandle<CBaseEntity>, int> >,CUtlMemory<CUtlVector<CHandle<CBaseEntity>, CUtlMemory<CHandle<CBaseEntity>, int> >, int> > m_aFactions;  // line 493
	int m_LastHitGroup; // +0x740  // line 496
	float m_flDamageAccumulator; // +0x744  // line 497
	int m_iDamageCount; // +0x748  // line 498
	WeaponProficiency_t m_CurrentWeaponProficiency; // +0x74c  // line 502
	CUtlVector<Relationship_t,CUtlMemory<Relationship_t, int> > m_Relationship; // +0x750  // line 507
	int m_nFaction; // +0x764  // line 508
	CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> > m_hTriggerFogList; // +0x768  // line 512
	EHANDLE m_hLastFogTrigger; // +0x77c  // line 513
	CBaseCombatCharacter::NetworkVar_m_iAmmo m_iAmmo; // +0x780  // line 516
	CBaseCombatCharacter::NetworkVar_m_hMyWeapons m_hMyWeapons; // +0x800  // line 519
	CNetworkHandle( CBaseCombatWeapon, m_hActiveWeapon ); // +0x8c0  // line 520
	CNavArea *m_lastNavArea; // +0x8c4  // line 523
	CAI_MoveMonitor m_NavAreaUpdateMonitor; // +0x8c8  // line 524
	int m_registeredNavTeam; // +0x8d8  // line 525
};

// CSetActiveSplitScreenPlayerGuard: not found as a complete record in this unit

// game/server/playerlocaldata.h:25
// game/server/playerlocaldata.h:25 sizeof=0x1f4 (i386)
struct CPlayerLocalData
{
public:
	int (**_vptr$CPlayerLocalData)(); // +0x0  // line 0
	CPlayerLocalData();  // line 33
	void UpdateAreaBits( CBasePlayer *, unsigned char * );  // line 35
	CPlayerLocalData::NetworkVar_m_chAreaBits m_chAreaBits; // +0x4  // line 40
	CPlayerLocalData::NetworkVar_m_chAreaPortalBits m_chAreaPortalBits; // +0x24  // line 41
	CNetworkVar( int, m_iHideHUD ); // +0x3c  // line 43
	CNetworkVar( float, m_flFOVRate ); // +0x40  // line 44
	Vector m_vecOverViewpoint; // +0x44  // line 46
	CNetworkVar( bool, m_bDucked ); // +0x50  // line 49
	CNetworkVar( bool, m_bDucking ); // +0x51  // line 51
	CNetworkVar( bool, m_bInDuckJump ); // +0x52  // line 53
	CNetworkVar( int, m_nDuckTimeMsecs ); // +0x54  // line 55
	CNetworkVar( int, m_nDuckJumpTimeMsecs ); // +0x58  // line 56
	CNetworkVar( int, m_nJumpTimeMsecs ); // +0x5c  // line 58
	int m_nStepside; // +0x60  // line 60
	CNetworkVar( float, m_flFallVelocity ); // +0x64  // line 62
	int m_nOldButtons; // +0x68  // line 64
	CSkyCamera *m_pOldSkyCamera; // +0x6c  // line 65
	CNetworkQAngle( QAngle, m_vecPunchAngle ); // +0x70  // line 69
	CNetworkQAngle( QAngle, m_vecPunchAngleVel ); // +0x7c  // line 70
	CNetworkVar( bool, m_bDrawViewmodel ); // +0x88  // line 72
	CNetworkVar( bool, m_bWearingSuit ); // +0x89  // line 75
	CNetworkVar( bool, m_bPoisoned ); // +0x8a  // line 76
	CNetworkVar( float, m_flStepSize ); // +0x8c  // line 77
	CNetworkVar( bool, m_bAllowAutoMovement ); // +0x90  // line 78
	CNetworkVar( bool, m_bAutoAimTarget ); // +0x91  // line 81
	CPlayerLocalData::NetworkVar_m_skybox3d m_skybox3d; // +0x94  // line 84
	static int GetOffset_m_skybox3d();  // line 84
	CPlayerLocalData::NetworkVar_m_PlayerFog m_PlayerFog; // +0xf8  // line 86
	static int GetOffset_m_PlayerFog();  // line 86
	fogparams_t m_fog; // +0x134  // line 87
	CPlayerLocalData::NetworkVar_m_audio m_audio; // +0x180  // line 89
	static int GetOffset_m_audio();  // line 89
	CNetworkVar( bool, m_bSlowMovement ); // +0x1f0  // line 91
};
