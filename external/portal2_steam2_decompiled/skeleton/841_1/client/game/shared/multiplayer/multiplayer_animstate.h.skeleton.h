// DWARF declaration skeleton for game/shared/multiplayer/multiplayer_animstate.h
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/shared/multiplayer/multiplayer_animstate.h:21
enum PlayerAnimEvent_t
{
	PLAYERANIMEVENT_ATTACK_PRIMARY = 0,
	PLAYERANIMEVENT_ATTACK_SECONDARY = 1,
	PLAYERANIMEVENT_ATTACK_GRENADE = 2,
	PLAYERANIMEVENT_RELOAD = 3,
	PLAYERANIMEVENT_RELOAD_LOOP = 4,
	PLAYERANIMEVENT_RELOAD_END = 5,
	PLAYERANIMEVENT_JUMP = 6,
	PLAYERANIMEVENT_SWIM = 7,
	PLAYERANIMEVENT_DIE = 8,
	PLAYERANIMEVENT_FLINCH_CHEST = 9,
	PLAYERANIMEVENT_FLINCH_HEAD = 10,
	PLAYERANIMEVENT_FLINCH_LEFTARM = 11,
	PLAYERANIMEVENT_FLINCH_RIGHTARM = 12,
	PLAYERANIMEVENT_FLINCH_LEFTLEG = 13,
	PLAYERANIMEVENT_FLINCH_RIGHTLEG = 14,
	PLAYERANIMEVENT_DOUBLEJUMP = 15,
	PLAYERANIMEVENT_CANCEL = 16,
	PLAYERANIMEVENT_SPAWN = 17,
	PLAYERANIMEVENT_SNAP_YAW = 18,
	PLAYERANIMEVENT_CUSTOM = 19,
	PLAYERANIMEVENT_CUSTOM_GESTURE = 20,
	PLAYERANIMEVENT_CUSTOM_SEQUENCE = 21,
	PLAYERANIMEVENT_CUSTOM_GESTURE_SEQUENCE = 22,
	PLAYERANIMEVENT_ATTACK_PRE = 23,
	PLAYERANIMEVENT_ATTACK_POST = 24,
	PLAYERANIMEVENT_GRENADE1_DRAW = 25,
	PLAYERANIMEVENT_GRENADE2_DRAW = 26,
	PLAYERANIMEVENT_GRENADE1_THROW = 27,
	PLAYERANIMEVENT_GRENADE2_THROW = 28,
	PLAYERANIMEVENT_VOICE_COMMAND_GESTURE = 29,
	PLAYERANIMEVENT_COUNT = 30,
};

// game/shared/multiplayer/multiplayer_animstate.h:81 sizeof=0x10 (i386)
struct GestureSlot_t
{
public:
	int m_iGestureSlot; // +0x0  // line 82
	$_295 m_iActivity; // +0x4  // line 83
	bool m_bAutoKill; // +0x8  // line 84
	bool m_bActive; // +0x9  // line 85
	C_AnimationLayer *m_pAnimLayer; // +0xc  // line 86
};

// game/shared/multiplayer/multiplayer_animstate.h:96 sizeof=0x1c (i386)
struct MultiPlayerPoseData_t
{
public:
	int m_iMoveX; // +0x0  // line 97
	int m_iMoveY; // +0x4  // line 98
	int m_iAimYaw; // +0x8  // line 99
	int m_iAimPitch; // +0xc  // line 100
	int m_iBodyHeight; // +0x10  // line 101
	float m_flEstimateYaw; // +0x14  // line 103
	float m_flLastAimTurnTime; // +0x18  // line 104
	void Init();  // line 106
};

// game/shared/multiplayer/multiplayer_animstate.h:119 sizeof=0x18 (i386)
struct DebugPlayerAnimData_t
{
public:
	float m_flSpeed; // +0x0  // line 120
	float m_flAimPitch; // +0x4  // line 121
	float m_flAimYaw; // +0x8  // line 122
	float m_flBodyHeight; // +0xc  // line 123
	Vector2D m_vecMoveYaw; // +0x10  // line 124
	void Init();  // line 126
};

// game/shared/multiplayer/multiplayer_animstate.h:137 sizeof=0x10 (i386)
struct MultiPlayerMovementData_t
{
public:
	float m_flWalkSpeed; // +0x0  // line 139
	float m_flRunSpeed; // +0x4  // line 140
	float m_flSprintSpeed; // +0x8  // line 141
	float m_flBodyYawRate; // +0xc  // line 142
};

// game/shared/multiplayer/multiplayer_animstate.h:155 sizeof=0xe0 (i386)
struct CMultiPlayerAnimState
{
public:
	int (**_vptr$CMultiPlayerAnimState)(); // +0x0  // line 0
	CMultiPlayerAnimState();  // line 161
	CMultiPlayerAnimState( C_BasePlayer *, MultiPlayerMovementData_t & );  // line 162
	~CMultiPlayerAnimState();  // line 163
	virtual void ClearAnimationState();  // line 167
	virtual void DoAnimationEvent( PlayerAnimEvent_t, int );  // line 168
	virtual $_295 CalcMainActivity();  // line 169
	virtual void Update( float, float );  // line 170
	virtual void Release();  // line 171
	const QAngle &GetRenderAngles();  // line 173
	virtual $_295 TranslateActivity( $_295 );  // line 175
	virtual void SetRunSpeed( float );  // line 177
	virtual void SetWalkSpeed( float );  // line 178
	virtual void SetSprintSpeed( float );  // line 179
	virtual void ShowDebugInfo();  // line 182
	virtual void DebugShowAnimState( int );  // line 183
	$_295 GetCurrentMainActivity();  // line 185
	void OnNewModel();  // line 187
	void ResetGestureSlots();  // line 190
	void ResetGestureSlot( int );  // line 191
	void AddVCDSequenceToGestureSlot( int, int, bool );  // line 192
	bool m_bForceAimYaw; // +0x4  // line 195
protected:
	virtual void Init( C_BasePlayer *, MultiPlayerMovementData_t & );  // line 199
	C_BasePlayer *GetBasePlayer();  // line 200
	virtual int SelectWeightedSequence( $_295 );  // line 203
	virtual void RestartMainSequence();  // line 204
	void GetOuterAbsVelocity( Vector & );  // line 206
	float GetOuterXYSpeed();  // line 207
	virtual bool HandleJumping( $_295 & );  // line 209
	virtual bool HandleDucking( $_295 & );  // line 210
	virtual bool HandleMoving( $_295 & );  // line 211
	virtual bool HandleSwimming( $_295 & );  // line 212
	virtual bool HandleDying( $_295 & );  // line 213
	CUtlVector<GestureSlot_t,CUtlMemory<GestureSlot_t, int> > m_aGestureSlots; // +0x8  // line 216
	bool InitGestureSlots();  // line 217
	void ShutdownGestureSlots();  // line 218
	bool IsGestureSlotActive( int );  // line 219
	bool IsGestureSlotPlaying( int, $_295 );  // line 220
	void AddToGestureSlot( int, $_295, bool );  // line 221
	void RestartGesture( int, $_295, bool );  // line 222
	void ComputeGestureSequence( CStudioHdr * );  // line 223
	void UpdateGestureLayer( CStudioHdr *, GestureSlot_t * );  // line 224
	void DebugGestureInfo();  // line 225
	void RunGestureSlotAnimEventsToCompletion( GestureSlot_t * );  // line 228
	virtual void PlayFlinchGesture( $_295 );  // line 231
	virtual float CalcMovementPlaybackRate( bool * );  // line 233
	virtual bool SetupPoseParameters( CStudioHdr * );  // line 236
	virtual void ComputePoseParam_MoveYaw( CStudioHdr * );  // line 237
	virtual void ComputePoseParam_AimPitch( CStudioHdr * );  // line 238
	virtual void ComputePoseParam_AimYaw( CStudioHdr * );  // line 239
	void ComputePoseParam_BodyHeight( CStudioHdr * );  // line 240
	virtual void EstimateYaw();  // line 241
	void ConvergeYawAngles( float, float, float, float & );  // line 242
	virtual float GetCurrentMaxGroundSpeed();  // line 244
	virtual void ComputeSequences( CStudioHdr * );  // line 245
	void ComputeMainSequence();  // line 246
	void UpdateInterpolators();  // line 247
	void ResetGroundSpeed();  // line 248
	float GetInterpolatedGroundSpeed();  // line 249
	void ComputeFireSequence();  // line 251
	void ComputeDeployedSequence();  // line 252
	bool ShouldUpdateAnimState();  // line 254
	void DebugShowAnimStateForPlayer( bool );  // line 256
	void DebugShowEyeYaw();  // line 257
	void DebugShowActivity( $_295 );  // line 263
	C_BasePlayer *m_pPlayer; // +0x1c  // line 269
	QAngle m_angRender; // +0x20  // line 271
	bool m_bPoseParameterInit; // +0x2c  // line 274
	MultiPlayerPoseData_t m_PoseParameterData; // +0x30  // line 275
	DebugPlayerAnimData_t m_DebugAnimData; // +0x4c  // line 276
	bool m_bCurrentFeetYawInitialized; // +0x64  // line 278
	float m_flLastAnimationStateClearTime; // +0x68  // line 279
	float m_flEyeYaw; // +0x6c  // line 281
	float m_flEyePitch; // +0x70  // line 282
	float m_flGoalFeetYaw; // +0x74  // line 283
	float m_flCurrentFeetYaw; // +0x78  // line 284
	float m_flLastAimTurnTime; // +0x7c  // line 285
	MultiPlayerMovementData_t m_MovementData; // +0x80  // line 287
	bool m_bJumping; // +0x90  // line 290
	float m_flJumpStartTime; // +0x94  // line 291
	bool m_bFirstJumpFrame; // +0x98  // line 292
	bool m_bInSwim; // +0x99  // line 295
	bool m_bFirstSwimFrame; // +0x9a  // line 296
	bool m_bDying; // +0x9b  // line 299
	bool m_bFirstDyingFrame; // +0x9c  // line 300
	$_295 m_eCurrentMainSequenceActivity; // +0xa0  // line 303
	int m_nSpecificMainSequence; // +0xa4  // line 306
	CHandle<C_BaseCombatWeapon> m_hActiveWeapon; // +0xa8  // line 309
	float m_flLastGroundSpeedUpdateTime; // +0xac  // line 313
	CInterpolatedVar<float> m_iv_flMaxGroundSpeed; // +0xb0  // line 314
	float m_flMaxGroundSpeed; // +0xdc  // line 316
};
