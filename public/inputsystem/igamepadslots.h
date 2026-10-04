//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Independent gamepad snapshots without changing legacy button codes.
//===========================================================================//

#ifndef IGAMEPADSLOTS_H
#define IGAMEPADSLOTS_H

#include <cstdint>

#define GAMEPAD_SLOTS_INTERFACE_VERSION "InputGamepadSlots001"

namespace gamepads
{
constexpr int kSlotCount = 2;
enum Axis
{
	LeftX,
	LeftY,
	RightX,
	RightY,
	LeftTrigger,
	RightTrigger,
	AxisCount
};
enum Button : std::uint32_t
{
	A = 1u << 0,
	B = 1u << 1,
	X = 1u << 2,
	Y = 1u << 3,
	Back = 1u << 4,
	Start = 1u << 5,
	LeftStick = 1u << 6,
	RightStick = 1u << 7,
	LeftShoulder = 1u << 8,
	RightShoulder = 1u << 9,
	Up = 1u << 10,
	Down = 1u << 11,
	Left = 1u << 12,
	Right = 1u << 13
};
struct State
{
	bool connected = false;
	// Changes on attach/removal, including reconnect into the same slot. Never
	// use a slot number alone to identify a device across a lifetime boundary.
	std::uint64_t generation = 0;
	std::uint32_t buttons = 0;
	// Sticks: [-32768,32767], triggers: [0,32767], before gameplay dead zones.
	int axes[AxisCount] = {};
};
}

// Borrow from IInputSystem::QueryInterface. The input system owns devices and
// outlives the borrower. Calls and SDL event pumping stay on the input sequence
// (the SDL main thread). Snapshots are values; reads never consume events.
// Slots retain their numbers while connected, even when the other is removed.
// A replacement occupies the lowest empty slot; unsupported/excess devices are
// ignored. Invalid slots fail without modifying the output. Disconnected slots
// succeed with neutral input and their current generation. ResetInputState
// neutralizes both slots. The frozen IInputSystem remains a slot-0 projection.
class IGamepadSlots
{
public:
	virtual bool GetGamepadState( int slot, gamepads::State &state ) const = 0;
	virtual void SetGamepadRumble( int slot, float left, float right ) = 0;
};

#endif // IGAMEPADSLOTS_H
