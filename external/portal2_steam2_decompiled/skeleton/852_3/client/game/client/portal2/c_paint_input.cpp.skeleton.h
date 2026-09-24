// DWARF declaration skeleton for game/client/portal2/c_paint_input.cpp
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x75b80 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
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

// game/client/portal2/c_paint_input.cpp:66 @0x528a70 _ZN13C_Paint_Input10ApplyMouseEiR6QAngleP8CUserCmdff
void C_Paint_Input::ApplyMouse( int nSlot, QAngle &viewangles, CUserCmd *cmd, float mouse_x, float mouse_y )
{
	C_Portal_Player *pPlayer;  // line 71
	Vector vForward;  // line 75
	Vector vRight;  // line 75
	Vector vUp;  // line 75
	Vector vPlayerUp;  // line 76
	Vector vStickNormal;  // line 77
	bool bShouldDampInput;  // line 81
	QAngle newAngles;  // line 203
	QAngle viewOffset;  // line 203
	// inlined QAngle::operator+=() at line 207
	// inlined QAngle::QAngle() at line 203
	// inlined C_Portal_Player::GetLocalPortalPlayer() at line 71
	{
		Vector vAngle;  // line 121
		// inlined CThirdPersonManager::GetCameraOffsetAngles() at line 121
		// inlined CThirdPersonManager::SetCameraOffsetAngles() at line 123
	}
	{
		float flProgress;  // line 84
		// inlined ConVar::GetFloat() at line 85
	}
	{
		float pitch;  // line 177
		VMatrix rotMatrix;  // line 181
		// inlined DotProduct() at line 177
		// inlined ConVar::GetFloat() at line 179
		// inlined CrossProduct() at line 184
		// inlined Vector::operator=() at line 184
		// inlined VMatrix::operator*() at line 185
		// inlined Vector::operator=() at line 185
	}
	{
		Vector vAngle;  // line 161
		float flPitchLimitDown;  // line 162
		float flPitchLimitUp;  // line 163
		// inlined CThirdPersonManager::SetCameraOffsetAngles() at line 172
		// inlined Vector::operator[]() at line 170
		// inlined CThirdPersonManager::GetCameraOffsetAngles() at line 161
	}
	{
		VMatrix rotMatrix;  // line 130
		float angle;  // line 131
		// inlined VMatrix::operator*() at line 133
		// inlined Vector::operator=() at line 133
	}
}
