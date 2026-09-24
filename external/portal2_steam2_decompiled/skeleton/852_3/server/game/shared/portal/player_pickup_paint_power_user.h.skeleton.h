// DWARF declaration skeleton for game/shared/portal/player_pickup_paint_power_user.h
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/shared/portal/player_pickup_paint_power_user.h:17 sizeof=0x884 (i386)
struct PlayerPickupPaintPowerUser<CPhysicsProp> : public PropPaintPowerUser<CPhysicsProp>
{
public:
	virtual void OnPhysGunPickup( CBasePlayer *, PhysGunPickup_t );  // line 28
};

// game/shared/portal/player_pickup_paint_power_user.h:17 sizeof=0x10f0 (i386)
struct PlayerPickupPaintPowerUser<CNPC_FloorTurret> : public PropPaintPowerUser<CNPC_FloorTurret>
{
public:
	virtual void OnPhysGunPickup( CBasePlayer *, PhysGunPickup_t );  // line 28
};

// game/shared/portal/player_pickup_paint_power_user.h:17 (declaration)
~PlayerPickupPaintPowerUser();

// game/shared/portal/player_pickup_paint_power_user.h:17 @0x6f2ed0 _ZN26PlayerPickupPaintPowerUserI12CPhysicsPropED0Ev
PlayerPickupPaintPowerUser<CPhysicsProp>::~PlayerPickupPaintPowerUser()
{
	// inlined PropPaintPowerUser<CPhysicsProp>::~PropPaintPowerUser() at line 17
}

// game/shared/portal/player_pickup_paint_power_user.h:17 @0x6f2f80 _ZN26PlayerPickupPaintPowerUserI12CPhysicsPropED1Ev
PlayerPickupPaintPowerUser<CPhysicsProp>::~PlayerPickupPaintPowerUser()
{
	// inlined PropPaintPowerUser<CPhysicsProp>::~PropPaintPowerUser() at line 17
}

// game/shared/portal/player_pickup_paint_power_user.h:17 (declaration)
void PlayerPickupPaintPowerUser();

// game/shared/portal/player_pickup_paint_power_user.h:28 @0x6f0420 _ZN26PlayerPickupPaintPowerUserI12CPhysicsPropE15OnPhysGunPickupEP11CBasePlayer15PhysGunPickup_t
void PlayerPickupPaintPowerUser<CPhysicsProp>::OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
}
