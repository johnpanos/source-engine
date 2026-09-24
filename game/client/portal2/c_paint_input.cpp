//========= Portal 2 reconstruction ============================================//
//
// Purpose: Paint (stick surface / taunt camera) aware client input.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "c_paint_input.h"
#include "kbutton.h"
#include "c_portal_player.h"
#include "cam_thirdperson.h"
#include "portal_util_shared.h"
#include "mathlib/vmatrix.h"
#include "tier1/convar_serverbounded.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar lookstrafe;
extern ConVar cl_pitchdown;
extern ConVar cl_pitchup;
extern ConVar_ServerBounded *m_pitch;

ConVar m_yaw( "m_yaw", "0.022", FCVAR_ARCHIVE, "Mouse yaw factor." );
ConVar m_side( "m_side", "0.8", FCVAR_ARCHIVE, "Mouse side factor." );
static ConVar m_forward( "m_forward", "1", FCVAR_ARCHIVE, "Mouse forward factor." );

// Reconstruction note: debug_mouse is registered by the original unit but has no reader in it.
ConVar debug_mouse( "debug_mouse", "20.f", 0 );

ConVar debug_pitch_limit( "debug_pitch_limit", "0.f", 0 );

// Progress of the stick camera reorientation below which vertical mouse input is suppressed.
ConVar stick_cam_input_suppression_threshhold( "stick_cam_input_suppression_threshhold", "0.8f", FCVAR_DEVELOPMENTONLY );

//-----------------------------------------------------------------------------
// Purpose: Applies mouse deltas relative to the player's current up vector
//-----------------------------------------------------------------------------
void C_Paint_Input::ApplyMouse( int nSlot, QAngle &viewangles, CUserCmd *cmd, float mouse_x, float mouse_y )
{
	C_Portal_Player *pPlayer = C_Portal_Player::GetLocalPortalPlayer();
	if ( !pPlayer )
		return;

	Vector vForward, vRight, vUp;
	Vector vPlayerUp = pPlayer->GetPortalPlayerLocalData().m_Up;
	Vector vStickNormal = pPlayer->GetPortalPlayerLocalData().m_StickNormal;
	AngleVectors( viewangles, &vForward, &vRight, &vUp );

	// Damp the vertical input while the camera is reorienting to a new surface
	bool bShouldDampInput = !pPlayer->IsDoneReorienting() &&
							pPlayer->GetPortalPlayerLocalData().m_nStickCameraState == STICK_CAMERA_SURFACE_TRANSITION;
	if ( bShouldDampInput )
	{
		float flProgress = pPlayer->GetReorientationProgress();
		const float flThreshold = stick_cam_input_suppression_threshhold.GetFloat();
		if ( flProgress < flThreshold )
		{
			flProgress = 0.0f;
		}
		else if ( flProgress < 1.0f )
		{
			flProgress = ( flProgress - flThreshold ) / ( 1.0f - flThreshold );
		}

		mouse_y *= flProgress;
	}

	// Yaw
	if ( !( in_strafe.GetPerUser( nSlot ).state & 1 ) && !lookstrafe.GetInt() )
	{
		if ( CAM_IsThirdPerson() && pPlayer->IsTaunting() )
		{
			// Taunting: the mouse orbits the taunt camera instead of turning the player
			if ( mouse_x != 0.0f && !pPlayer->IsInterpolatingTauntAngles() )
			{
				Vector vAngle = g_ThirdPersonManager.GetCameraOffsetAngles();
				vAngle[ YAW ] -= mouse_x * m_yaw.GetFloat();
				g_ThirdPersonManager.SetCameraOffsetAngles( vAngle );

				if ( fabs( vAngle[ YAW ] - pPlayer->GetTauntCamTargetYaw() ) > 30.0f )
				{
					pPlayer->SetFaceTauntCameraEndAngles( true );
				}
			}
		}
		else
		{
			// Turn around the player's up vector
			VMatrix rotMatrix;
			float angle = -m_yaw.GetFloat() * mouse_x;
			MatrixBuildRotationAboutAxis( rotMatrix, vPlayerUp, angle );
			vRight = rotMatrix * vRight;
		}
	}
	else
	{
		cmd->sidemove += m_side.GetFloat() * mouse_x;
	}

	// Pitch
	if ( !( in_strafe.GetPerUser( nSlot ).state & 1 ) )
	{
		if ( CAM_IsThirdPerson() && pPlayer->IsTaunting() )
		{
			if ( mouse_y != 0.0f && !pPlayer->IsInterpolatingTauntAngles() )
			{
				Vector vAngle = g_ThirdPersonManager.GetCameraOffsetAngles();
				float flPitchLimitDown = pPlayer->m_Shared.InCond( PORTAL_COND_DROWNING ) ? 5.0f : -15.0f;
				float flPitchLimitUp = 55.0f;

				vAngle[ PITCH ] = clamp( vAngle[ PITCH ] + m_pitch->GetFloat() * mouse_y, flPitchLimitDown, flPitchLimitUp );
				g_ThirdPersonManager.SetCameraOffsetAngles( vAngle );
			}
		}
		else
		{
			// Pitch is measured from the plane perpendicular to the player's up vector
			float pitch = 90.0f - RAD2DEG( acos( clamp( DotProduct( vForward, vPlayerUp ), -1.0f, 1.0f ) ) ) - m_pitch->GetFloat() * mouse_y;
			pitch = clamp( pitch, debug_pitch_limit.GetFloat() - cl_pitchup.GetFloat(), cl_pitchdown.GetFloat() - debug_pitch_limit.GetFloat() );

			VMatrix rotMatrix;
			MatrixBuildRotationAboutAxis( rotMatrix, vRight, pitch );

			vForward = CrossProduct( vPlayerUp, vRight );
			vForward = rotMatrix * vForward;
		}
	}
	else
	{
		cmd->forwardmove -= m_forward.GetFloat() * mouse_y;
	}

	// Convert the new basis back to view angles
	QAngle newAngles, viewOffset( 0.0f, 0.0f, 0.0f );
	VectorAngles( vForward, vPlayerUp, newAngles );
	UTIL_NormalizedAngleDiff( viewangles, newAngles, &viewOffset );
	viewangles += viewOffset;

	cmd->mousedx = (int)mouse_x;
	cmd->mousedy = (int)mouse_y;
}


//-----------------------------------------------------------------------------
// Purpose: Joystick turning also drives the taunt camera
//-----------------------------------------------------------------------------
void C_Paint_Input::JoyStickTurn( CUserCmd *cmd, float &yaw, float &pitch, float frametime, bool bAbsoluteYaw, bool bAbsolutePitch )
{
	CInput::JoyStickTurn( cmd, yaw, pitch, frametime, bAbsoluteYaw, bAbsolutePitch );

	C_Portal_Player *pPlayer = C_Portal_Player::GetLocalPortalPlayer();
	if ( pPlayer && CAM_IsThirdPerson() && pPlayer->IsTaunting() )
	{
		if ( yaw != 0.0f && !pPlayer->IsInterpolatingTauntAngles() )
		{
			Vector vAngle = g_ThirdPersonManager.GetCameraOffsetAngles();
			vAngle[ YAW ] -= yaw;
			g_ThirdPersonManager.SetCameraOffsetAngles( vAngle );

			if ( fabs( vAngle[ YAW ] - pPlayer->GetTauntCamTargetYaw() ) > 30.0f )
			{
				pPlayer->SetFaceTauntCameraEndAngles( true );
			}
		}

		if ( pitch != 0.0f && !pPlayer->IsInterpolatingTauntAngles() )
		{
			Vector vAngle = g_ThirdPersonManager.GetCameraOffsetAngles();
			float flPitchLimitDown = pPlayer->m_Shared.InCond( PORTAL_COND_DROWNING ) ? 5.0f : -15.0f;
			float flPitchLimitUp = 55.0f;

			vAngle[ PITCH ] = clamp( vAngle[ PITCH ] + pitch, flPitchLimitDown, flPitchLimitUp );
			g_ThirdPersonManager.SetCameraOffsetAngles( vAngle );
		}
	}
}
