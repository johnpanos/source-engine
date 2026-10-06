//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Drives the keybindings of the second local split-screen player from its own
//			controller. The input system keeps its frozen button codes (joystick 0 only) and
//			exposes each controller through IGamepadSlots (public/inputsystem/igamepadslots.h);
//			this turns slot N's button changes into the bound commands, run as that player.
//
//			Slot 0's controller still arrives as ordinary key events (Key_Event).
//
//=============================================================================//

#include "client_pch.h"
#include "inputsystem/igamepadslots.h"
#include "inputsystem/iinputsystem.h"
#include "keys.h"
#include "cmd.h"
#include "sys_dll.h"

#include "tier0/memdbgon.h"

namespace
{
const int kTriggerThreshold = 16384;	// of 32767: a trigger counts as a button past this

struct PadButton_t
{
	std::uint32_t	m_nMask;
	ButtonCode_t	m_Code;
};

const PadButton_t s_Buttons[] =
{
	{ gamepads::A, KEY_XBUTTON_A },
	{ gamepads::B, KEY_XBUTTON_B },
	{ gamepads::X, KEY_XBUTTON_X },
	{ gamepads::Y, KEY_XBUTTON_Y },
	{ gamepads::Back, KEY_XBUTTON_BACK },
	{ gamepads::Start, KEY_XBUTTON_START },
	{ gamepads::LeftStick, KEY_XBUTTON_STICK1 },
	{ gamepads::RightStick, KEY_XBUTTON_STICK2 },
	{ gamepads::LeftShoulder, KEY_XBUTTON_LEFT_SHOULDER },
	{ gamepads::RightShoulder, KEY_XBUTTON_RIGHT_SHOULDER },
	{ gamepads::Up, KEY_XBUTTON_UP },
	{ gamepads::Down, KEY_XBUTTON_DOWN },
	{ gamepads::Left, KEY_XBUTTON_LEFT },
	{ gamepads::Right, KEY_XBUTTON_RIGHT },
};

// The pad state last seen per slot, plus a generation so a replaced controller starts neutral.
struct PadMemory_t
{
	std::uint64_t	m_nGeneration = 0;
	std::uint32_t	m_nButtons = 0;
	bool			m_bLeftTrigger = false;
	bool			m_bRightTrigger = false;
};
PadMemory_t s_Memory[ gamepads::kSlotCount ];

IGamepadSlots *GetGamepadSlots()
{
	static IGamepadSlots *s_pSlots = NULL;
	static bool s_bLooked = false;
	if ( !s_bLooked && g_pInputSystem )
	{
		s_bLooked = true;
		s_pSlots = static_cast< IGamepadSlots * >( g_pInputSystem->QueryInterface( GAMEPAD_SLOTS_INTERFACE_VERSION ) );
	}
	return s_pSlots;
}

// Runs the binding of `code` for the local player in `nSlot`, as a key press or release would.
// Button commands ("+jump") get the key number as a parameter, as the key handler gives them.
void RunBinding( int nSlot, ButtonCode_t code, bool bDown )
{
	const char *pBinding = Key_BindingForKey( code );
	if ( !pBinding || !pBinding[ 0 ] )
		return;

	ACTIVE_SPLITSCREEN_PLAYER_GUARD( nSlot );

	// Queued on that player's command buffer (the active slot picks it), so it runs as them
	char szCommand[ 1024 ];
	if ( pBinding[ 0 ] == '+' )
	{
		V_snprintf( szCommand, sizeof( szCommand ), "%s%s %i\n", bDown ? "+" : "-", pBinding + 1, (int)code );
	}
	else if ( bDown )
	{
		V_snprintf( szCommand, sizeof( szCommand ), "%s\n", pBinding );
	}
	else
	{
		return;
	}

	Cbuf_AddText( szCommand );
}

void ApplyEdge( int nSlot, ButtonCode_t code, bool bWasDown, bool bIsDown )
{
	if ( bWasDown != bIsDown )
	{
		RunBinding( nSlot, code, bIsDown );
	}
}
} // namespace

//-----------------------------------------------------------------------------
// Once per frame, after the input system has polled: the other local players' pad edges.
//-----------------------------------------------------------------------------
void CL_PollSplitScreenGamepads()
{
	IGamepadSlots *pSlots = GetGamepadSlots();
	if ( !pSlots )
		return;

	for ( int nSlot = 1; nSlot < gamepads::kSlotCount; ++nSlot )
	{
		PadMemory_t &mem = s_Memory[ nSlot ];

		gamepads::State state;
		if ( !pSlots->GetGamepadState( nSlot, state ) )
			continue;

		// A removed or replaced controller releases everything it held
		bool bPresent = state.connected && splitscreen->IsValidSplitScreenSlot( nSlot );
		if ( !bPresent || state.generation != mem.m_nGeneration )
		{
			for ( int i = 0; i < ARRAYSIZE( s_Buttons ); ++i )
			{
				ApplyEdge( nSlot, s_Buttons[ i ].m_Code, ( mem.m_nButtons & s_Buttons[ i ].m_nMask ) != 0, false );
			}
			ApplyEdge( nSlot, KEY_XBUTTON_LTRIGGER, mem.m_bLeftTrigger, false );
			ApplyEdge( nSlot, KEY_XBUTTON_RTRIGGER, mem.m_bRightTrigger, false );

			mem = PadMemory_t();
			mem.m_nGeneration = state.generation;
			if ( !bPresent )
				continue;
		}

		for ( int i = 0; i < ARRAYSIZE( s_Buttons ); ++i )
		{
			ApplyEdge( nSlot, s_Buttons[ i ].m_Code,
				( mem.m_nButtons & s_Buttons[ i ].m_nMask ) != 0, ( state.buttons & s_Buttons[ i ].m_nMask ) != 0 );
		}

		const bool bLeftTrigger = state.axes[ gamepads::LeftTrigger ] > kTriggerThreshold;
		const bool bRightTrigger = state.axes[ gamepads::RightTrigger ] > kTriggerThreshold;
		ApplyEdge( nSlot, KEY_XBUTTON_LTRIGGER, mem.m_bLeftTrigger, bLeftTrigger );
		ApplyEdge( nSlot, KEY_XBUTTON_RTRIGGER, mem.m_bRightTrigger, bRightTrigger );

		mem.m_nButtons = state.buttons;
		mem.m_bLeftTrigger = bLeftTrigger;
		mem.m_bRightTrigger = bRightTrigger;
	}
}

//-----------------------------------------------------------------------------
// Purpose: What the input system reports for each controller slot, and what the bridge has seen
//-----------------------------------------------------------------------------
CON_COMMAND( ss_gamepad_status, "Lists the controller slots (split-screen players) and their state." )
{
	IGamepadSlots *pSlots = GetGamepadSlots();
	if ( !pSlots )
	{
		Msg( "The input system has no controller slots.\n" );
		return;
	}

	for ( int nSlot = 0; nSlot < gamepads::kSlotCount; ++nSlot )
	{
		gamepads::State state;
		if ( !pSlots->GetGamepadState( nSlot, state ) )
		{
			Msg( "slot %d: unavailable\n", nSlot );
			continue;
		}
		Msg( "slot %d: %s, generation %llu, buttons 0x%x, left stick %d %d, triggers %d %d, local player %s\n", nSlot,
			state.connected ? "connected" : "no controller", (unsigned long long)state.generation, state.buttons,
			state.axes[ gamepads::LeftX ], state.axes[ gamepads::LeftY ], state.axes[ gamepads::LeftTrigger ],
			state.axes[ gamepads::RightTrigger ], splitscreen->IsValidSplitScreenSlot( nSlot ) ? "yes" : "no" );
	}
}
