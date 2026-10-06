// Class layouts from /home/john/Downloads/portal2-steam2-research/852_3/portal2/bin/server.dylib.dSYM/Contents/Resources/DWARF/server.dylib
// Unit game/server/portal/portal_player.cpp; i386 offsets; reconstruction aid.

// game/server/portal/portal_player.h:52
// game/server/portal/portal_player.h:52 sizeof=0x17a0 (i386)
struct CPortal_Player : public PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer> >
{
public:
	CPortal_Player();  // line 56
	virtual ~CPortal_Player();  // line 57
	static CPortal_Player *CreatePlayer( const char *, edict_t * );  // line 59
	virtual void Precache();  // line 68
	virtual void CreateSounds();  // line 69
	virtual void StopLoopingSounds();  // line 70
	virtual void Spawn();  // line 71
	virtual void SharedSpawn();  // line 72
	virtual void OnRestore();  // line 73
	virtual int Restore( IRestore & );  // line 74
	virtual void Activate();  // line 75
	virtual void Touch( CBaseEntity * );  // line 77
	virtual void InitialSpawn();  // line 80
	void SetPlacingPhoto( bool );  // line 82
	void OnPhotoAdded( int );  // line 83
	void OnPhotoRemoved( int );  // line 84
	void SetSelectedPhoto( int );  // line 85
	int GetSelectedPhoto();  // line 86
	void ClearPhotos();  // line 87
	void StripPhotos( bool );  // line 88
	void FlashDenyIndicator( float, unsigned char );  // line 89
	void FlashInventory( float, unsigned char );  // line 90
	void Flash( float, const Vector & );  // line 91
	void ControlHelperAnimate( unsigned char, bool );  // line 92
	void UpdateLocatorEntityIndices( int *, int );  // line 93
	virtual bool ShouldCollide( int, int ) const;  // line 96
	virtual float PlayScene( const char *, float, AI_Response *, IRecipientFilter * );  // line 97
	virtual void NotifySystemEvent( CBaseEntity *, notify_system_event_t, const notify_system_event_params_t & );  // line 98
	virtual void PostThink();  // line 100
	virtual void PreThink();  // line 101
	void SwapThink();  // line 102
	virtual void PlayerDeathThink();  // line 103
	void UpdatePortalPlaneSounds();  // line 105
	void UpdateWooshSounds();  // line 106
	$_178 TranslateActivity( $_178, bool * );  // line 108
	virtual void Teleport( const Vector *, const QAngle *, const Vector * );  // line 109
	$_178 TranslateTeamActivity( $_178 );  // line 111
	virtual void SetAnimation( PLAYER_ANIM );  // line 113
	virtual void PlayerRunCommand( CUserCmd *, IMoveHelper * );  // line 115
	virtual bool ClientCommand( const CCommand & );  // line 117
	virtual void CreateViewModel( int );  // line 118
	virtual bool BecomeRagdollOnClient( const Vector & );  // line 119
	virtual int OnTakeDamage( const CTakeDamageInfo & );  // line 120
	virtual int OnTakeDamage_Alive( const CTakeDamageInfo & );  // line 121
	virtual void Break( CBaseEntity *, const CTakeDamageInfo & );  // line 122
	virtual bool WantsLagCompensationOnEntity( const CBasePlayer *, const CUserCmd *, const CBitVec<2048> * ) const;  // line 123
	virtual void FireBullets( const FireBulletsInfo_t & );  // line 124
	virtual bool Weapon_Switch( CBaseCombatWeapon *, int );  // line 126
	virtual Vector Weapon_ShootPosition();  // line 127
	virtual bool BumpWeapon( CBaseCombatWeapon * );  // line 128
	virtual CBaseCombatWeapon *Weapon_OwnsThisType( const char *, int ) const;  // line 129
	virtual void Weapon_Equip( CBaseCombatWeapon * );  // line 130
	virtual void SelectItem( const char *, int );  // line 131
	virtual void ShutdownUseEntity();  // line 134
	virtual bool ShouldDropActiveWeaponWhenKilled();  // line 135
	virtual Vector EyeDirection3D();  // line 137
	virtual Vector EyeDirection2D();  // line 138
	virtual const Vector &WorldSpaceCenter() const;  // line 139
	virtual void Event_Killed( const CTakeDamageInfo & );  // line 143
	virtual void Jump();  // line 144
	void UnDuck();  // line 146
	bool UseFoundEntity( CBaseEntity * );  // line 147
	virtual void PlayerUse();  // line 151
	virtual void GetStepSoundVelocities( float *, float * );  // line 153
	virtual void PlayStepSound( Vector &, surfacedata_t *, float, bool );  // line 154
	virtual void UpdateOnRemove();  // line 155
	virtual void OnSave( IEntitySaveUtils * );  // line 157
	virtual void SetupVisibility( CBaseEntity *, unsigned char *, int );  // line 159
	virtual void UpdatePortalViewAreaBits( unsigned char *, int );  // line 160
	virtual void ItemPostFrame();  // line 161
	bool ValidatePlayerModel( const char * );  // line 163
	void ParseScriptedInteractions();  // line 165
	void AddScriptedInteraction( ScriptedNPCInteraction_t * );  // line 166
	CUtlVector<ScriptedNPCInteraction_t,CUtlMemory<ScriptedNPCInteraction_t, int> > *GetScriptedInteractions();  // line 167
	void FireConcept( const char * );  // line 169
	bool IsTaunting();  // line 170
	int GetTeamTauntState() const;  // line 171
	void SetTeamTauntState( int );  // line 172
	Vector GetTeamTauntPosition();  // line 173
	QAngle GetTeamTauntAngles();  // line 174
	QAngle GetAnimEyeAngles();  // line 176
	virtual Vector GetAttackSpread( CBaseCombatWeapon *, CBaseEntity * );  // line 178
	virtual void CheatImpulseCommands( int );  // line 180
	void CreateRagdollEntity( const CTakeDamageInfo & );  // line 181
	void GiveAllItems();  // line 182
	void GiveDefaultItems();  // line 183
	void NoteWeaponFired();  // line 185
	void ResetAnimation();  // line 187
	void SetPlayerModel();  // line 189
	int GetPlayerModelType();  // line 191
	virtual void ForceDuckThisFrame();  // line 193
	void DoAnimationEvent( PlayerAnimEvent_t, int );  // line 197
	virtual void SetupBones( matrix3x4a_t *, int );  // line 198
	virtual void PickupObject( CBaseEntity *, bool );  // line 201
	virtual void ForceDropOfCarriedPhysObjects( CBaseEntity * );  // line 202
	void ToggleHeldObjectOnOppositeSideOfPortal();  // line 204
	void SetHeldObjectOnOppositeSideOfPortal( bool );  // line 205
	bool IsHeldObjectOnOppositeSideOfPortal();  // line 206
	CPortal_Base2D *GetHeldObjectPortal();  // line 207
	void SetHeldObjectPortal( CPortal_Base2D * );  // line 208
	void SetUsingVMGrabState( bool );  // line 209
	bool IsUsingVMGrab();  // line 210
	bool WantsVMGrab();  // line 211
	void UpdateVMGrab( CBaseEntity * );  // line 212
	bool IsForcingDrop();  // line 213
	EHANDLE m_hGrabbedEntity; // +0x1364  // line 218
	EHANDLE m_hPortalThroughWhichGrabOccured; // +0x1368  // line 219
	bool m_bForcingDrop; // +0x136c  // line 220
	CNetworkVar( bool, m_bUseVMGrab ); // +0x136d  // line 221
	CNetworkVar( bool, m_bUsingVMGrabState ); // +0x136e  // line 222
	CNetworkHandle( CBaseEntity, m_hAttachedObject ); // +0x1370  // line 226
	CNetworkVar( QAngle, m_vecCarriedObjectAngles ); // +0x1374  // line 227
	void SetUseKeyCooldownTime( float );  // line 229
	void SetStuckOnPortalCollisionObject();  // line 231
	CWeaponPortalBase *GetActivePortalWeapon() const;  // line 233
	void IncrementPortalsPlaced( bool );  // line 235
	void IncrementStepsTaken();  // line 236
	void UpdateSecondsTaken();  // line 237
	void ResetThisLevelStats();  // line 238
	int NumPortalsPlaced() const;  // line 239
	int NumStepsTaken() const;  // line 240
	float NumSecondsTaken() const;  // line 241
	bool IsHoldingEntity( CBaseEntity * );  // line 243
	virtual float GetHeldObjectMass( IPhysicsObject * );  // line 244
	void SetNeuroToxinDamageTime( float );  // line 246
	void IncNumCamerasDetatched();  // line 248
	int GetNumCamerasDetatched() const;  // line 249
	void MarkClientCheckPVSDirty();  // line 251
	void SetIsHoldingObject( bool );  // line 253
	virtual void ApplyPortalTeleportation( const CPortal_Base2D *, CMoveData * );  // line 255
	Vector m_vecTotalBulletForce; // +0x1380  // line 257
	bool m_bSilentDropAndPickup; // +0x138c  // line 259
	CNetworkHandle( CBaseEntity, m_hRagdoll ); // +0x1390  // line 262
	void SetAirControlSupressionTime( float );  // line 268
	bool IsSuppressingAirControl();  // line 269
	virtual bool TestHitboxes( const Ray_t &, unsigned int, trace_t & );  // line 271
	virtual void ModifyOrAppendCriteria( ResponseRules::CriteriaSet & );  // line 272
	float GetImplicitVerticalStepSpeed() const;  // line 274
	void SetImplicitVerticalStepSpeed( float );  // line 275
	const CPortalPlayerLocalData &GetPortalPlayerLocalData() const;  // line 277
	virtual Vector EyePosition();  // line 279
	void PlayCoopPingEffect();  // line 288
	CNetworkVar( bool, m_bPitchReorientation ); // +0x1394  // line 290
	CNetworkHandle( CPortal_Base2D, m_hPortalEnvironment ); // +0x1398  // line 291
	CNetworkHandle( CFunc_LiquidPortal, m_hSurroundingLiquidPortal ); // +0x139c  // line 292
	virtual CBaseEntity *EntSelectSpawnPoint();  // line 296
	void PickTeam();  // line 297
	static void ClientDisconnected( edict_t * );  // line 298
	virtual void ChangeTeam( int );  // line 299
	virtual int FlashlightIsOn();  // line 301
	virtual bool FlashlightTurnOn( bool );  // line 302
	virtual void FlashlightTurnOff( bool );  // line 303
	void SetInTractorBeam( bool );  // line 305
protected:
	Vector m_vWorldSpaceCenterHolder; // +0x13a0  // line 310
	CPortal_Player::NetworkVar_m_PortalLocal m_PortalLocal; // +0x13ac  // line 311
	static int GetOffset_m_PortalLocal();  // line 311
private:
	virtual const char *GetPlayerModelName();  // line 315
	virtual void PlayUseDenySound();  // line 316
	void Taunt( const char * );  // line 317
	bool SolveTeamTauntPositionAndAngles( CPortal_Player * );  // line 318
	void StartTaunt();  // line 319
	bool FindRemoteTauntViewpoint( Vector *, QAngle * );  // line 320
	CSoundPatch *m_pWooshSound; // +0x1550  // line 326
	CNetworkQAngle( QAngle, m_angEyeAngles ); // +0x1554  // line 328
	CPortalPlayerAnimState *m_PlayerAnimState; // +0x1560  // line 330
	int m_iLastWeaponFireUsercmd; // +0x1564  // line 332
	CNetworkVar( int, m_iSpawnInterpCounter ); // +0x1568  // line 333
	CNetworkVar( int, m_iPlayerSoundType ); // +0x156c  // line 334
	CUtlVector<ScriptedNPCInteraction_t,CUtlMemory<ScriptedNPCInteraction_t, int> > m_ScriptedInteractions; // +0x1570  // line 336
	CNetworkVar( bool, m_bTauntRemoteView ); // +0x1584  // line 338
	CNetworkVar( Vector, m_vecRemoteViewOrigin ); // +0x1588  // line 339
	CNetworkVar( QAngle, m_vecRemoteViewAngles ); // +0x1594  // line 340
	CNetworkVar( float, m_fTauntCameraDistance ); // +0x15a0  // line 341
	CNetworkVar( int, m_nTeamTauntState ); // +0x15a4  // line 342
	CNetworkVector( Vector, m_vTauntPosition ); // +0x15a8  // line 343
	CNetworkQAngle( QAngle, m_vTauntAngles ); // +0x15b4  // line 344
	char m_szTauntForce[64]; // +0x15c0  // line 346
	QAngle m_angTauntCamera; // +0x1600  // line 348
	CNetworkVar( bool, m_bHeldObjectOnOppositeSideOfPortal ); // +0x160c  // line 350
	CNetworkHandle( CPortal_Base2D, m_pHeldObjectPortal ); // +0x1610  // line 351
	bool m_bIntersectingPortalPlane; // +0x1614  // line 357
	bool m_bStuckOnPortalCollisionObject; // +0x1615  // line 358
	bool m_bPlayUseDenySound; // +0x1616  // line 359
	float m_fNeuroToxinDamageTime; // +0x1618  // line 361
	PortalPlayerStatistics_t m_StatsThisLevel; // +0x161c  // line 363
	float m_fTimeLastNumSecondsUpdate; // +0x1628  // line 364
	int m_iNumCamerasDetatched; // +0x162c  // line 366
	QAngle m_qPrePortalledViewAngles; // +0x1630  // line 368
	bool m_bFixEyeAnglesFromPortalling; // +0x163c  // line 369
	VMatrix m_matLastPortalled; // +0x1640  // line 370
	float m_flLastPingTime; // +0x1680  // line 373
	bool m_bClientCheckPVSDirty; // +0x1684  // line 377
	float m_flUseKeyCooldownTime; // +0x1688  // line 379
	CNetworkVar( bool, m_bIsHoldingSomething ); // +0x168c  // line 381
	float m_flImplicitVerticalStepSpeed; // +0x1690  // line 383
	CNetworkVar( bool, m_iSpawnCounter ); // +0x1694  // line 388
public:
	bool IsReorienting() const;  // line 391
	Vector GetPaintGunShootPosition();  // line 392
	bool IsPressingJumpKey() const;  // line 394
	bool IsHoldingJumpKey() const;  // line 395
	bool IsTryingToSuperJump( const PaintPowerInfo_t * ) const;  // line 396
	void SetJumpedThisFrame( bool );  // line 397
	bool JumpedThisFrame() const;  // line 398
	InAirState GetInAirState() const;  // line 399
	bool WantsToSwapGuns();  // line 401
	void SetWantsToSwapGuns( bool );  // line 402
	bool IsUsingPostTeleportationBox() const;  // line 404
	const Vector &GetInputVector() const;  // line 406
	void SetInputVector( const Vector & );  // line 407
	virtual Vector BodyTarget( const Vector &, bool );  // line 409
	const Vector &GetPrevGroundNormal() const;  // line 411
	void SetPrevGroundNormal( const Vector & );  // line 412
	virtual PaintPowerType GetPaintPowerAtPoint( const Vector & ) const;  // line 414
	virtual void Paint( PaintPowerType, const Vector & );  // line 415
	virtual void CleansePaint();  // line 416
	void Reorient( QAngle & );  // line 418
	float GetReorientationProgress() const;  // line 419
	bool IsDoneReorienting() const;  // line 420
	virtual const Vector GetPlayerMins() const;  // line 421
	virtual const Vector GetPlayerMaxs() const;  // line 422
	const Vector &GetHullMins() const;  // line 423
	const Vector &GetHullMaxs() const;  // line 424
	const Vector &GetStandHullMins() const;  // line 425
	const Vector &GetStandHullMaxs() const;  // line 426
	const Vector &GetDuckHullMins() const;  // line 427
	const Vector &GetDuckHullMaxs() const;  // line 428
	float GetHullHeight() const;  // line 430
	float GetHullWidth() const;  // line 431
	float GetStandHullHeight() const;  // line 432
	float GetStandHullWidth() const;  // line 433
	float GetDuckHullHeight() const;  // line 434
	float GetDuckHullWidth() const;  // line 435
	virtual void UpdateCollisionBounds();  // line 437
	virtual void InitVCollision( const Vector &, const Vector & );  // line 438
	bool PlayGesture( const char * );  // line 440
	void SetAirDuck( bool );  // line 442
	StickCameraState GetStickCameraState() const;  // line 444
	void SetQuaternionPunch( const Quaternion & );  // line 445
	void DecayQuaternionPunch();  // line 446
	void AddSurfacePaintPowerInfo( const BrushContact &, const char * );  // line 449
	void AddSurfacePaintPowerInfo( const trace_t &, const char * );  // line 450
	void SetEyeUpOffset( const Vector &, const Vector & );  // line 452
	void SetEyeOffset( const Vector &, const Vector & );  // line 453
	void GivePlayerPaintGun( bool, bool );  // line 455
	void GivePlayerPortalGun( bool, bool );  // line 456
	void ResetBounceCount();  // line 458
	CPortal_Player::NetworkVar_m_Shared m_Shared; // +0x1698  // line 461
	static int GetOffset_m_Shared();  // line 461
	void SetHullHeight( float );  // line 463
protected:
	virtual void ChooseActivePaintPowers( CUtlVector<PaintPowerInfo_t,CUtlMemory<PaintPowerInfo_t, int> > & );  // line 466
private:
	void DecayEyeOffset();  // line 469
	void GivePortalPlayerItems();  // line 471
	void DeterminePaintContacts();  // line 474
	void PredictPaintContacts( const Vector &, const Vector &, const Vector &, const Vector &, float, const char * );  // line 480
	void ChooseBestPaintPowersInRange( PaintPowerChoiceResultArray &, PaintPowerConstIter, PaintPowerConstIter, const PaintPowerChoiceCriteria_t & ) const;  // line 484
	virtual PaintPowerState ActivateSpeedPower( PaintPowerInfo_t & );  // line 487
	virtual PaintPowerState UseSpeedPower( PaintPowerInfo_t & );  // line 488
	virtual PaintPowerState DeactivateSpeedPower( PaintPowerInfo_t & );  // line 489
	virtual PaintPowerState ActivateBouncePower( PaintPowerInfo_t & );  // line 491
	virtual PaintPowerState UseBouncePower( PaintPowerInfo_t & );  // line 492
	virtual PaintPowerState DeactivateBouncePower( PaintPowerInfo_t & );  // line 493
	void PlayPaintSounds( const PaintPowerChoiceResultArray & );  // line 495
	void UpdatePaintedPower();  // line 496
	void UpdateAirInputScaleFadeIn();  // line 497
	void UpdateInAirState();  // line 498
	void CopyPaintPowerChoiceInfoToHudState( const PaintPowerChoiceResultArray & );  // line 499
	bool LateSuperJumpIsValid() const;  // line 500
	void RecomputeBoundsForOrientation();  // line 501
	void TryToChangeCollisionBounds( const Vector &, const Vector &, const Vector &, const Vector & );  // line 505
	float SpeedPaintAcceleration( float, float, float, float ) const;  // line 510
	bool CheckToUseBouncePower( PaintPowerInfo_t & );  // line 512
	void UpdateNotSoStickyStick( PaintPowerInfo_t & );  // line 515
	void RotateUpVector( Vector &, Vector & );  // line 518
	void StartAutoRotate( StickCameraState, bool );  // line 519
	void CheckStickCameraState( const Vector &, const Vector &, Vector & );  // line 520
	void PostTeleportationStickCamFixup( const CPortal_Base2D *, CMoveData * );  // line 521
	void SetOffTheWallProgress( float, Vector &, const Vector & );  // line 522
	void DrawJumpHelperDebug( PaintPowerConstIter, PaintPowerConstIter, float, bool, const PaintPowerInfo_t * ) const;  // line 525
	PaintPowerInfo_t m_CachedJumpPower; // +0x1714  // line 528
	Vector m_vInputVector; // +0x1740  // line 529
	float m_flCachedJumpPowerTime; // +0x174c  // line 530
	float m_flUsePostTeleportationBoxTime; // +0x1750  // line 531
	float m_flSpeedDecelerationTime; // +0x1754  // line 532
	float m_flPredictedJumpTime; // +0x1758  // line 533
	bool m_bJumpWasPressedWhenForced; // +0x175c  // line 534
	int m_nBounceCount; // +0x1760  // line 535
	float m_flLastSuppressedBounceTime; // +0x1764  // line 536
	float m_flTimeSinceLastTouchedPower[3]; // +0x1768  // line 537
	CNetworkVar( bool, m_bWantsToSwapGuns ); // +0x1774  // line 540
	bool m_bSendSwapProximityFailEvent; // +0x1775  // line 541
	CBaseCombatWeapon *m_pWeaponWhenDead; // +0x1778  // line 543
	PlayerGunType m_PlayerGunType; // +0x177c  // line 544
	bool m_bSpawnFromDeath; // +0x1780  // line 545
	Vector m_vPrevGroundNormal; // +0x1784  // line 547
	Vector m_vGravity; // +0x1790  // line 549
	CNetworkVar( float, m_flHullHeight ); // +0x179c  // line 551
};
