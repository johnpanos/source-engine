//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Gyro aiming policy: how the rotation IInputSystem reports becomes
//          view rotation. It follows what a player sees from Steam Input's
//          gyro camera on a Steam Deck:
//          - Turning uses player space by default: the direction comes from
//            rotation about world up, and the amount from the device's own yaw
//            and roll together. Turning the body or the device turns the view
//            the same way however far the screen is tilted back.
//          - Tightening scales very slow rotation down smoothly (hand tremor,
//            sensor noise) instead of cutting it off with a deadzone.
//
//          Pure functions over the screen axes of
//          IInputSystem::GetGyroAccumulators and GetGyroUp: pitch about
//          screen-right, yaw about screen-up, roll about the screen normal,
//          counter-clockwise positive (positive yaw turns left).
//
//          Tested by unittests/inputtest (input.gyro).
//
//===========================================================================//

#ifndef GYRO_AIM_H
#define GYRO_AIM_H
#ifdef _WIN32
#pragma once
#endif

#include <math.h>

namespace gyroaim
{

// Values of touch_gyro_axis.
enum ETurnAxis
{
	TURN_YAW = 0,      // turn the device about screen-up
	TURN_ROLL,         // steer it like a wheel
	TURN_YAW_AND_ROLL, // both, added
	TURN_PLAYER_SPACE, // about world up (the default)
	TURN_AXIS_COUNT
};

// How far player space lets the world-up component exceed itself before the
// device's combined yaw and roll caps it: a 45 degree buffer, so tilting the
// device while turning does not scale the turn.
const float kPlayerSpaceRelax = 1.41f;

// The turn (positive left) for rotation[3] in screen axes. up is the direction
// away from gravity in the same axes, or NULL when unknown; player space then
// falls back to yaw.
inline float Turn( int nAxis, const float rotation[3], const float *up )
{
	switch ( nAxis )
	{
	case TURN_ROLL:
		return rotation[2];
	case TURN_YAW_AND_ROLL:
		return rotation[1] + rotation[2];
	case TURN_PLAYER_SPACE:
	{
		const float flUpLength = up ? sqrtf( up[0] * up[0] + up[1] * up[1] + up[2] * up[2] ) : 0.f;
		if ( !( flUpLength > 1e-6f ) )
			return rotation[1];
		// Rotation about world up, leaving pitch out: pitching the device
		// never turns the view.
		const float flWorldYaw = ( rotation[1] * up[1] + rotation[2] * up[2] ) / flUpLength;
		const float flLocal = sqrtf( rotation[1] * rotation[1] + rotation[2] * rotation[2] );
		const float flTurn = fminf( fabsf( flWorldYaw ) * kPlayerSpaceRelax, flLocal );
		return flWorldYaw < 0.f ? -flTurn : flTurn;
	}
	case TURN_YAW:
	default:
		return rotation[1];
	}
}

// Scales (turn, pitch), rotated over flSeconds, by speed / flThreshold while
// the speed is below flThreshold (radians per second), which leaves faster
// rotation untouched and has no step at the threshold. A threshold or an
// interval of 0 or less leaves the rotation unchanged.
inline void Tighten( float &turn, float &pitch, float flSeconds, float flThreshold )
{
	if ( !( flSeconds > 0.f ) || !( flThreshold > 0.f ) )
		return;
	const float flSpeed = sqrtf( turn * turn + pitch * pitch ) / flSeconds;
	if ( flSpeed >= flThreshold )
		return;
	const float flScale = flSpeed / flThreshold;
	turn *= flScale;
	pitch *= flScale;
}

} // namespace gyroaim

#endif // GYRO_AIM_H
