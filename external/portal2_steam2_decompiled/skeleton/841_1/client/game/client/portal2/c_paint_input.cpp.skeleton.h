// DWARF declaration skeleton for game/client/portal2/c_paint_input.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x759e0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
}

// game/client/portal2/c_paint_input.cpp:50
ConVar m_yaw;

// game/client/portal2/c_paint_input.cpp:51
ConVar m_side;

// game/client/portal2/c_paint_input.cpp:52
static ConVar m_forward;

// game/client/portal2/c_paint_input.cpp:55
ConVar debug_mouse;

// game/client/portal2/c_paint_input.cpp:57
ConVar debug_pitch_limit;

// game/client/portal2/c_paint_input.cpp:65
ConVar stick_cam_input_suppression_threshhold;

// game/client/portal2/c_paint_input.cpp:66 @0x523640 _ZN13C_Paint_Input10ApplyMouseEiR6QAngleP8CUserCmdff
void C_Paint_Input::ApplyMouse( int nSlot, QAngle &viewangles, CUserCmd *cmd, float mouse_x, float mouse_y )
{
	C_Portal_Player *pPlayer;  // line 71
	Vector vForward;  // line 75
	Vector vRight;  // line 75
	Vector vUp;  // line 75
	Vector vPlayerUp;  // line 76
	Vector vStickNormal;  // line 77
	bool bShouldDampInput;  // line 81
	QAngle newAngles;  // line 182
	QAngle viewOffset;  // line 182
	// inlined QAngle::operator+=() at line 186
	// inlined QAngle::QAngle() at line 182
	// inlined C_Portal_Player::GetLocalPortalPlayer() at line 71
	{
		Vector vAngle;  // line 102
		// inlined CThirdPersonManager::GetCameraOffsetAngles() at line 102
		// inlined CThirdPersonManager::SetCameraOffsetAngles() at line 104
		// inlined C_Portal_Player::SetFaceTauntCameraEndAngles() at line 108
	}
	{
		float flProgress;  // line 84
		// inlined ConVar::GetFloat() at line 85
	}
	{
		float pitch;  // line 156
		VMatrix rotMatrix;  // line 160
		// inlined DotProduct() at line 156
		// inlined ConVar::GetFloat() at line 158
		// inlined CrossProduct() at line 163
		// inlined Vector::operator=() at line 163
		// inlined VMatrix::operator*() at line 164
		// inlined Vector::operator=() at line 164
	}
	{
		Vector vAngle;  // line 141
		float flPitchLimitDown;  // line 142
		float flPitchLimitUp;  // line 143
		// inlined CThirdPersonManager::SetCameraOffsetAngles() at line 151
		// inlined Vector::operator[]() at line 149
		// inlined CThirdPersonManager::GetCameraOffsetAngles() at line 141
	}
	{
		VMatrix rotMatrix;  // line 116
		float angle;  // line 117
		// inlined VMatrix::operator*() at line 119
		// inlined Vector::operator=() at line 119
	}
}

// game/client/portal2/c_paint_input.cpp:194 @0x523450 _ZN13C_Paint_Input12JoyStickTurnEP8CUserCmdRfS2_fbb
void C_Paint_Input::JoyStickTurn( CUserCmd *cmd, float &yaw, float &pitch, float frametime, bool bAbsoluteYaw, bool bAbsolutePitch )
{
	C_Portal_Player *pPlayer;  // line 201
	// inlined C_Portal_Player::GetLocalPortalPlayer() at line 201
	{
		Vector vAngle;  // line 221
		float flPitchLimitDown;  // line 222
		float flPitchLimitUp;  // line 223
		// inlined CThirdPersonManager::SetCameraOffsetAngles() at line 232
		// inlined Vector::operator[]() at line 230
		// inlined CThirdPersonManager::GetCameraOffsetAngles() at line 221
	}
	{
		Vector vAngle;  // line 209
		// inlined CThirdPersonManager::GetCameraOffsetAngles() at line 209
		// inlined CThirdPersonManager::SetCameraOffsetAngles() at line 211
		// inlined C_Portal_Player::SetFaceTauntCameraEndAngles() at line 215
	}
}
