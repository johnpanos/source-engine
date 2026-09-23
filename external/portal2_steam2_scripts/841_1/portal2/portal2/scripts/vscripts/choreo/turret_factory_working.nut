function PlayStuckSound()
{
	printl("=================I AM TURRET NAMED: " + self.GetName() )
	
	TurretVoManager.TurretStuck(self)
}

// -------------------------------------------------------------------
// Called when a functioning turret starts the self test and will pass
// -------------------------------------------------------------------
function FunctioningTurretPass()
{
	printl("=================TURRET NAME: " + self.GetName() )
	TurretVoManager.GoodTurretPass(self)
	//ScannerPassTurret()
}

// -------------------------------------------------------------------
// Called when a functioning turret starts the self test and will fail
// -------------------------------------------------------------------
function FunctioningTurretFail()
{
	TurretVoManager.GoodTurretTest(self)
	//ScannerRejectTurret()
}

// -------------------------------------------------------------------
// Called when a malfunctioning turret starts the self test and will pass
// -------------------------------------------------------------------
function MalfunctioningTurretPass()
{
	//self.EmitSound("turret.TurretStuckInTube01")
	TurretVoManager.DefectTurretPass(self)
	//ScannerPassTurret()
}

// -------------------------------------------------------------------
// Called when a malfunctioning turret starts the self test and will fail
// -------------------------------------------------------------------
function MalfunctioningTurretFail()
{
	//self.EmitSound("turret.TurretStuckInTubeGoodbye01")
	TurretVoManager.DefectTurretTest(self)
	//ScannerRejectTurret()
}

function MalfunctioningTurretFling()
{
	TurretVoManager.DefectTurretFail(self)
}

function FunctioningTurretFling()
{
	TurretVoManager.GoodTurretFail(self)
}

function grabbedDefect()
{
	printl("==========GRABBED A TURRET!")
	TurretVoManager.grabbedDefectTurret(self)
}

function MalfunctioningTurretSneakBy()
{
	printl("===STARTING SNEAK!!!")
	TurretVoManager.DefectTurretSneakBy(self)
}

// =======================================================================
// =======================================================================


// -------------------------------------------------------------------
// Precache sounds we will emit
// -------------------------------------------------------------------
function Precache()
{
	self.PrecacheSoundScript( "turret.TurretStuckInTube01" )
	self.PrecacheSoundScript( "turret.TurretStuckInTubeGoodbye01" )
}

// -------------------------------------------------------------------
// Allow turret to pass scanner
// -------------------------------------------------------------------
function ScannerPassFunctionTurret()
{
	EntFire( "accept_turret_relay", "trigger", 0, 4 )
}

// -------------------------------------------------------------------
// Catapults the turret away 
// -------------------------------------------------------------------
function ScannerRejectFunctioningTurret()
{
	EntFire( "reject_turret_relay", "trigger", 0, 4 )
}

// -------------------------------------------------------------------
// Allow turret to pass scanner
// -------------------------------------------------------------------
function ScannerPassMalfunctionTurret()
{
	EntFire( "accept_turret_relay", "trigger", 0, 4 )
}

// -------------------------------------------------------------------
// Catapults the turret away
// -------------------------------------------------------------------
function ScannerRejectMalfunctioningTurret()
{
	EntFire( "reject_turret_relay", "trigger", 0, 4 )
}



// =======================================================================
// =======================================================================

// -------------------------------------------------------------------
// Called when a functioning turret is in position to shoot a dummy
// -------------------------------------------------------------------
function FunctioningTurretReachedDummyShootPosition()
{
	TurretVoManager.GoodTurretShootPosition(self)
}

// -------------------------------------------------------------------
// Called when a malfunctioning turret is in position to shoot a dummy
// -------------------------------------------------------------------
function MalfunctioningTurretReachedDummyShootPosition()
{
	TurretVoManager.DefectTurretShootPosition(self)
}


// -------------------------------------------------------------------
// Spawn a turret to shoot at dummy
// -------------------------------------------------------------------
function SpawnDummyShootTurret()
{
	printl("***Spawning a turret to shoot at dummy!")
	
	// spawn a new turret
	//EntFire("dummyshoot_conveyor_1_spawn_rl", "trigger", 0, 2 )
	
	// send current train on down the line to exit
	//EntFire("dummyshoot_conveyor_1_advance_train_relay", "trigger", 0, 2 )
}