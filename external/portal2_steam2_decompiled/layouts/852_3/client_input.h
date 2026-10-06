// Class layouts from /home/john/Downloads/portal2-steam2-research/852_3/portal2/bin/client.dylib.dSYM/Contents/Resources/DWARF/client.dylib
// Unit game/client/in_main.cpp; i386 offsets; reconstruction aid.

// game/client/input.h:52
// game/client/input.h:52 sizeof=0x1f0 (i386)
struct CInput : public IInput
{
public:
	CInput();  // line 55
	~CInput();  // line 56
	virtual void Init_All();  // line 58
	virtual void Shutdown_All();  // line 59
	virtual int GetButtonBits( bool );  // line 60
	virtual void CreateMove( int, float, bool );  // line 61
	virtual void ExtraMouseSample( float, bool );  // line 62
	virtual bool WriteUsercmdDeltaToBuffer( int, bf_write *, int, int, bool );  // line 63
	virtual void EncodeUserCmdToBuffer( int, bf_write &, int );  // line 64
	virtual void DecodeUserCmdFromBuffer( int, bf_read &, int );  // line 65
	virtual CUserCmd *GetUserCmd( int, int );  // line 67
	virtual void MakeWeaponSelection( C_BaseCombatWeapon * );  // line 69
	virtual float KeyState( kbutton_t * );  // line 71
	virtual int KeyEvent( int, ButtonCode_t, const char * );  // line 72
	virtual kbutton_t *FindKey( const char * );  // line 73
	virtual void ControllerCommands();  // line 75
	virtual void Joystick_Advanced( bool );  // line 76
	virtual void Joystick_SetSampleTime( float );  // line 77
	virtual void IN_SetSampleTime( float );  // line 78
	virtual void AccumulateMouse( int );  // line 80
	virtual void ActivateMouse();  // line 81
	virtual void DeactivateMouse();  // line 82
	virtual void ClearStates();  // line 84
	virtual float GetLookSpring();  // line 85
	virtual void GetFullscreenMousePos( int *, int *, int *, int * );  // line 87
	virtual void SetFullscreenMousePos( int, int );  // line 88
	virtual void ResetMouse();  // line 89
	virtual float GetLastForwardMove();  // line 92
	virtual void ClearInputButton( int );  // line 93
	virtual void CAM_Think();  // line 95
	virtual int CAM_IsThirdPerson( int );  // line 96
	virtual void CAM_GetCameraOffset( Vector & );  // line 97
	virtual void CAM_ToThirdPerson();  // line 98
	virtual void CAM_ToFirstPerson();  // line 99
	virtual void CAM_ToThirdPersonShoulder();  // line 100
	virtual void CAM_StartMouseMove();  // line 101
	virtual void CAM_EndMouseMove();  // line 102
	virtual void CAM_StartDistance();  // line 103
	virtual void CAM_EndDistance();  // line 104
	virtual int CAM_InterceptingMouse();  // line 105
	virtual void CAM_Command( int );  // line 106
	virtual void CAM_ToOrthographic();  // line 109
	virtual bool CAM_IsOrthographic() const;  // line 110
	virtual void CAM_OrthographicSize( float &, float & ) const;  // line 111
	virtual void LevelInit();  // line 117
	virtual void CAM_SetCameraThirdData( CameraThirdData_t *, const QAngle & );  // line 119
	virtual void CAM_CameraThirdThink();  // line 120
	virtual void CheckPaused( CUserCmd * );  // line 122
	virtual void CheckSplitScreenMimic( int, CUserCmd *, CUserCmd * );  // line 123
protected:
	virtual void Init_Camera();  // line 128
	void Init_Keyboard();  // line 129
	void Init_Mouse();  // line 130
	void Shutdown_Keyboard();  // line 131
	void AddKeyButton( const char *, kbutton_t * );  // line 133
	void ScaleMovements( CUserCmd * );  // line 135
	void ComputeForwardMove( int, CUserCmd * );  // line 136
	void ComputeUpwardMove( int, CUserCmd * );  // line 137
	void ComputeSideMove( int, CUserCmd * );  // line 138
	void AdjustAngles( int, float );  // line 139
	void ClampAngles( QAngle & );  // line 140
	void AdjustPitch( int, float, QAngle & );  // line 141
	void AdjustYaw( int, float, QAngle & );  // line 142
	float DetermineKeySpeed( int, float );  // line 143
	void GetAccumulatedMouseDeltasAndResetAccumulators( int, float *, float * );  // line 144
	void GetMouseDelta( int, float, float, float *, float * );  // line 145
	void ScaleMouse( int, float *, float * );  // line 146
	virtual void ApplyMouse( int, QAngle &, CUserCmd *, float, float );  // line 147
	void MouseMove( int, CUserCmd * );  // line 148
	void ControllerMove( int, float, CUserCmd * );  // line 151
	float ScaleAxisValue( float, float );  // line 152
	virtual void JoyStickMove( float, CUserCmd * );  // line 153
	virtual bool ControllerModeActive();  // line 155
	virtual bool JoyStickActive();  // line 156
	virtual void JoyStickSampleAxes( float &, float &, float &, float &, bool &, bool & );  // line 157
	virtual void JoyStickThirdPersonPlatformer( CUserCmd *, float &, float &, float &, float & );  // line 158
	virtual void JoyStickTurn( CUserCmd *, float &, float &, float, bool, bool );  // line 159
	virtual void JoyStickForwardSideControl( float, float, float &, float & );  // line 160
	virtual void JoyStickApplyMovement( CUserCmd *, float, float );  // line 161
	void GetMousePos( int &, int & );  // line 164
	void SetMousePos( int, int );  // line 165
	virtual void GetWindowCenter( int &, int & );  // line 166
	void CheckMouseAcclerationVars();  // line 168
	void ValidateUserCmd( CUserCmd *, int );  // line 170
	void TrackIRMove( float, CUserCmd * );  // line 173
	void Init_TrackIR();  // line 174
	void Shutdown_TrackIR();  // line 175
	bool m_fTrackIRAvailable; // +0x4  // line 177
public:
	// game/client/input.h:182 sizeof=0xc (i386)
	struct $_313
	{
	public:
		unsigned int AxisFlags; // +0x0  // line 183
		unsigned int AxisMap; // +0x4  // line 184
		unsigned int ControlMap; // +0x8  // line 185
	};  // line 182
protected:
	void DescribeJoystickAxis( int, const char *, CInput::$_313 * );  // line 188
	const char *DescribeAxis( int );  // line 189
public:
	// game/client/input.h:208
	enum $_316
	{
		MOUSE_ACCEL_THRESHHOLD1 = 0,
		MOUSE_ACCEL_THRESHHOLD2 = 1,
		MOUSE_SPEED_FACTOR = 2,
		NUM_MOUSE_PARAMS = 3,
	};  // line 208
protected:
	bool m_fMouseInitialized; // +0x5  // line 218
	bool m_fMouseActive; // +0x6  // line 220
	bool m_fJoystickAdvancedInit; // +0x7  // line 222
public:
	// game/client/input.h:226 sizeof=0xdc (i386)
	struct PerUserInput_t
	{
	public:
		PerUserInput_t();  // line 227
		float m_flAccumulatedMouseXMovement; // +0x0  // line 279
		float m_flAccumulatedMouseYMovement; // +0x4  // line 280
		float m_flPreviousMouseXPosition; // +0x8  // line 281
		float m_flPreviousMouseYPosition; // +0xc  // line 282
		float m_flRemainingJoystickSampleTime; // +0x10  // line 283
		float m_flKeyboardSampleTime; // +0x14  // line 284
		float m_flSpinFrameTime; // +0x18  // line 286
		float m_flSpinRate; // +0x1c  // line 287
		float m_flLastYawAngle; // +0x20  // line 288
		CInput::$_313 m_rgAxes[6]; // +0x24  // line 291
		bool m_fCameraInterceptingMouse; // +0x6c  // line 294
		bool m_fCameraInThirdPerson; // +0x6d  // line 296
		bool m_fCameraMovingWithMouse; // +0x6e  // line 298
		Vector m_vecCameraOffset; // +0x70  // line 300
		bool m_fCameraDistanceMove; // +0x7c  // line 302
		int m_nCameraOldX; // +0x80  // line 304
		int m_nCameraOldY; // +0x84  // line 305
		int m_nCameraX; // +0x88  // line 306
		int m_nCameraY; // +0x8c  // line 307
		bool m_CameraIsOrthographic; // +0x90  // line 310
		QAngle m_angPreviousViewAngles; // +0x94  // line 312
		QAngle m_angPreviousViewAnglesTilt; // +0xa0  // line 313
		float m_flLastForwardMove; // +0xac  // line 315
		int m_nClearInputState; // +0xb0  // line 317
		CUserCmd *m_pCommands; // +0xb4  // line 319
		CVerifiedUserCmd *m_pVerifiedCommands; // +0xb8  // line 320
		CHandle<C_BaseCombatWeapon> m_hSelectedWeapon; // +0xbc  // line 323
		CameraThirdData_t *m_pCameraThirdData; // +0xc0  // line 329
		int m_nCamCommand; // +0xc4  // line 330
		float m_flPreviousJoystickForwardMove; // +0xc8  // line 335
		float m_flPreviousJoystickSideMove; // +0xcc  // line 336
		float m_flPreviousJoystickYaw; // +0xd0  // line 337
		float m_flPreviousJoystickPitch; // +0xd4  // line 338
		bool m_bPreviousJoystickUseAbsoluteYaw; // +0xd8  // line 339
		bool m_bPreviousJoystickUseAbsolutePitch; // +0xd9  // line 340
	};  // line 226
protected:
	CInput::PerUserInput_t &GetPerUser( int );  // line 343
	const CInput::PerUserInput_t &GetPerUser( int ) const;  // line 344
	bool m_fRestoreSPI; // +0x8  // line 347
	int m_rgOrigMouseParms[3]; // +0xc  // line 349
	int m_rgNewMouseParms[3]; // +0x18  // line 351
	bool m_rgCheckMouseParam[3]; // +0x24  // line 352
	bool m_fMouseParmsValid; // +0x27  // line 354
	CKeyboardKey *m_pKeys; // +0x28  // line 356
	CInput::PerUserInput_t m_PerUser[2]; // +0x2c  // line 358
	InputContextHandle_t m_hInputContext; // +0x1e4  // line 360
	CThreadFastMutex m_IKContactPointMutex; // +0x1e8  // line 361
};

// game/client/kbutton.h:17
// game/client/kbutton.h:17 sizeof=0x18 (i386)
struct kbutton_t
{
public:
	// game/client/kbutton.h:19 sizeof=0xc (i386)
	struct Split_t
	{
	public:
		int down[2]; // +0x0  // line 21
		int state; // +0x8  // line 23
	};  // line 19
	kbutton_t::Split_t &GetPerUser( int );  // line 26
	kbutton_t::Split_t m_PerUser[2]; // +0x0  // line 28
};
