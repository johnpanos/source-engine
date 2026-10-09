//========= Copyright Valve Corporation, All rights reserved. ============//
// Purpose: Exercise the installed SDL input provider with two virtual gamepads.
//===========================================================================//

#include <SDL3/SDL.h>
#include "appframework/ilaunchermgr.h"
#include "appframework/window_provider.h"
#include "inputsystem/provider_catalog.h"
#include "inputsystem/iinputsystem.h"
#include "inputsystem/igamepadslots.h"
#include "tier0/icommandline.h"
#include "tier1/convar.h"
#include "tier1/tier1.h"
#include "icvar.h"
#include "vstdlib/cvar.h"
#include <cstdio>
#include <cstring>

namespace
{
int checks = 0, failures = 0;
ILauncherMgr *manager = nullptr;
IInputSystem *input = nullptr;
// The client's controller switch (game/client/in_main.cpp), on as a player who
// uses a gamepad sets it; rumble is off without it.
ConVar joystick( "joystick", "1" );
struct Motors
{
	int calls = 0;
	Uint16 left = 0, right = 0;
};
void Check( bool ok, const char *description, int line )
{
	++checks;
	if ( !ok )
	{
		++failures;
		std::fprintf( stderr, "FAIL %d: %s (%s)\n", line, description, SDL_GetError() );
	}
}
#define CHECK( expression ) Check( ( expression ), #expression, __LINE__ )
void *Factory( const char *name, int *result )
{
	if ( !std::strcmp( name, SDLMGR_INTERFACE_VERSION ) )
		return manager;
	if ( !std::strcmp( name, INPUTSYSTEM_INTERFACE_VERSION ) )
		return input;
	return VStdLib_GetICVarFactory()( name, result );
}
bool SDLCALL Rumble( void *data, Uint16 left, Uint16 right )
{
	Motors &motors = *static_cast<Motors *>( data );
	++motors.calls;
	motors.left = left;
	motors.right = right;
	return true;
}
// Deliberately bad provider: the suite must reject shared player state and
// incorrectly targeted haptics, rather than blessing a slot-0-only adapter.
class CollapsingSlots : public IGamepadSlots
{
public:
	explicit CollapsingSlots( IGamepadSlots &provider ) : provider( provider ) {}
	bool GetGamepadState( int slot, gamepads::State &state ) const override
	{
		return provider.GetGamepadState( slot == 1 ? 0 : slot, state );
	}
	void SetGamepadRumble( int slot, float left, float right ) override
	{
		provider.SetGamepadRumble( slot == 1 ? 0 : slot, left, right );
	}

private:
	IGamepadSlots &provider;
};

SDL_JoystickID Attach( const char *name, Motors &motors )
{
	SDL_VirtualJoystickDesc description;
	SDL_INIT_INTERFACE( &description );
	description.type = SDL_JOYSTICK_TYPE_GAMEPAD;
	description.naxes = SDL_GAMEPAD_AXIS_COUNT;
	description.nbuttons = SDL_GAMEPAD_BUTTON_COUNT;
	description.axis_mask = ( 1u << SDL_GAMEPAD_AXIS_COUNT ) - 1;
	description.button_mask = ( 1u << SDL_GAMEPAD_BUTTON_COUNT ) - 1;
	description.name = name;
	description.userdata = &motors;
	description.Rumble = Rumble;
	SDL_JoystickID id = SDL_AttachVirtualJoystick( &description );
	CHECK( id != 0 );
	return id;
}
void Poll()
{
	SDL_UpdateJoysticks();
	input->PollInputState();
}
gamepads::State Read( IGamepadSlots &slots, int slot )
{
	gamepads::State state;
	CHECK( slots.GetGamepadState( slot, state ) );
	return state;
}
}

int main( int argc, char **argv )
{
	CommandLine()->CreateCmdLine( argc, argv );
	CHECK( SDL_Init( SDL_INIT_GAMEPAD ) );
	// Run with physical joysticks disabled through SDL hints (see README).
	// Virtual pads prove routing; they do not qualify controller hardware.
	manager = WindowProvider_Describe()->create();
	input = InputSystem_Describe()->create();
	CHECK( input->Connect( Factory ) );
	// The test's own ConVars register in the cvar system the input system uses.
	g_pCVar = static_cast<ICvar *>( Factory( CVAR_INTERFACE_VERSION, NULL ) );
	ConVar_Register( 0 );
	CHECK( input->Init() == INIT_OK );
	IGamepadSlots *slots =
	    static_cast<IGamepadSlots *>( input->QueryInterface( GAMEPAD_SLOTS_INTERFACE_VERSION ) );
	CHECK( slots != nullptr );
	if ( !slots )
		return 1;
	CollapsingSlots badProvider( *slots );
	if ( CommandLine()->FindParm( "-seed-slot-collapse" ) )
		slots = &badProvider;
	CHECK( !Read( *slots, 0 ).connected );
	CHECK( !Read( *slots, 1 ).connected );
	gamepads::State invalid;
	invalid.generation = 42;
	CHECK( !slots->GetGamepadState( -1, invalid ) && invalid.generation == 42 );
	CHECK( !slots->GetGamepadState( 2, invalid ) && invalid.generation == 42 );

	Motors motors[3];
	SDL_JoystickID first = Attach( "Atlas fixture", motors[0] );
	SDL_JoystickID second = Attach( "P-body fixture", motors[1] );
	Poll();
	SDL_Joystick *a = SDL_OpenJoystick( first );
	SDL_Joystick *b = SDL_OpenJoystick( second );
	CHECK( a && b );
	CHECK( Read( *slots, 0 ).connected );
	CHECK( Read( *slots, 1 ).connected );
	const uint64_t firstGeneration = Read( *slots, 0 ).generation;
	const uint64_t secondGeneration = Read( *slots, 1 ).generation;
	CHECK( input->GetJoystickCount() == 1 );

	CHECK( SDL_SetJoystickVirtualButton( a, SDL_GAMEPAD_BUTTON_SOUTH, true ) );
	CHECK( SDL_SetJoystickVirtualButton( b, SDL_GAMEPAD_BUTTON_EAST, true ) );
	CHECK( SDL_SetJoystickVirtualAxis( a, SDL_GAMEPAD_AXIS_LEFTX, 16000 ) );
	CHECK( SDL_SetJoystickVirtualAxis( b, SDL_GAMEPAD_AXIS_LEFTX, -24000 ) );
	CHECK( SDL_SetJoystickVirtualAxis( b, SDL_GAMEPAD_AXIS_LEFT_TRIGGER, 18000 ) );
	CHECK( SDL_SetJoystickVirtualAxis( b, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, 26000 ) );
	Poll();
	CHECK( Read( *slots, 0 ).buttons == gamepads::A );
	CHECK( Read( *slots, 1 ).buttons == gamepads::B );
	CHECK( Read( *slots, 0 ).axes[gamepads::LeftX] == 16000 );
	CHECK( Read( *slots, 1 ).axes[gamepads::LeftX] == -24000 );
	CHECK( Read( *slots, 1 ).axes[gamepads::LeftTrigger] !=
	       Read( *slots, 1 ).axes[gamepads::RightTrigger] );
	CHECK( input->IsButtonDown( KEY_XBUTTON_A ) );
	CHECK( !input->IsButtonDown( KEY_XBUTTON_B ) );
	CHECK( input->GetAnalogValue( JOYSTICK_AXIS( 0, JOY_AXIS_X ) ) == 16000 );
	CHECK( !input->IsButtonDown( KEY_XBUTTON_RTRIGGER ) );

	// Rumble uses separate leases and the same underlying device ownership.
	slots->SetGamepadRumble( 0, 0.6f, 0.2f );
	const int firstCalls = motors[0].calls;
	slots->SetGamepadRumble( 1, 0.1f, 0.8f );
	CHECK( motors[0].calls == firstCalls );
	CHECK( motors[0].left > motors[0].right );
	CHECK( motors[1].left < motors[1].right );
	input->StopRumble();
	CHECK( motors[0].left == 0 && motors[0].right == 0 );
	CHECK( motors[1].left == 0 && motors[1].right == 0 );

	// A third device must not displace either player.
	SDL_JoystickID extra = Attach( "Excess fixture", motors[2] );
	Poll();
	CHECK( Read( *slots, 0 ).generation == firstGeneration );
	CHECK( Read( *slots, 1 ).generation == secondGeneration );
	CHECK( SDL_DetachVirtualJoystick( extra ) );
	Poll();
	CHECK( Read( *slots, 1 ).buttons == gamepads::B );

	CHECK( SDL_DetachVirtualJoystick( first ) );
	SDL_CloseJoystick( a );
	Poll();
	CHECK( !Read( *slots, 0 ).connected );
	CHECK( Read( *slots, 0 ).buttons == 0 );
	CHECK( Read( *slots, 0 ).axes[gamepads::LeftX] == 0 );
	CHECK( !input->IsButtonDown( KEY_XBUTTON_A ) );
	CHECK( input->GetAnalogValue( JOYSTICK_AXIS( 0, JOY_AXIS_X ) ) == 0 );
	CHECK( Read( *slots, 1 ).generation == secondGeneration );
	CHECK( Read( *slots, 1 ).buttons == gamepads::B );

	SDL_JoystickID replacement = Attach( "Reconnect fixture", motors[0] );
	Poll();
	CHECK( Read( *slots, 0 ).connected );
	CHECK( Read( *slots, 0 ).generation > firstGeneration );
	CHECK( Read( *slots, 1 ).generation == secondGeneration );
	input->EnableInput( false );
	CHECK( Read( *slots, 1 ).buttons == 0 );
	CHECK( Read( *slots, 1 ).axes[gamepads::LeftX] == 0 );
	input->ResetInputState();
	input->EnableInput( true );
	CHECK( Read( *slots, 0 ).buttons == 0 );
	CHECK( Read( *slots, 1 ).buttons == 0 );
	CHECK( Read( *slots, 1 ).axes[gamepads::LeftX] == 0 );
	// Reinitialize while the second pad is held. State must be sampled at
	// open, not wait for a new axis/button transition after startup.
	CHECK( SDL_SetJoystickVirtualButton( b, SDL_GAMEPAD_BUTTON_NORTH, true ) );
	CHECK( SDL_SetJoystickVirtualAxis( b, SDL_GAMEPAD_AXIS_RIGHTX, 12000 ) );
	Poll();
	input->Shutdown();
	CHECK( !Read( *slots, 0 ).connected && !Read( *slots, 1 ).connected );
	CHECK( input->Init() == INIT_OK );
	CHECK( Read( *slots, 0 ).connected && Read( *slots, 1 ).connected );
	// SDL enumerates by instance ID, so the surviving older P-body pad now
	// takes slot 0 in this new input lifetime.
	CHECK( ( Read( *slots, 0 ).buttons & gamepads::Y ) != 0 );
	CHECK( Read( *slots, 0 ).axes[gamepads::RightX] == 12000 );
	CHECK( SDL_DetachVirtualJoystick( second ) );
	SDL_CloseJoystick( b );
	CHECK( SDL_DetachVirtualJoystick( replacement ) );
	Poll();
	CHECK( !Read( *slots, 0 ).connected && !Read( *slots, 1 ).connected );
	input->Shutdown();
	input->Disconnect();
	SDL_Quit();
	std::printf( "CONFORMANCE %d %d\n", checks, failures );
	return failures ? 1 : 0;
}
