//SceneTableLookup
//MAKE SURE THE INDEXES ON THIS ARRAY ARE NEGATIVE!!!!!
	SceneTableLookup[-1] <- "-1_01" //Test Scene
	SceneTableLookup[-2] <- "-2_01" //Wheatly Elevator Scene - Pre Crash
	SceneTableLookup[-3] <- "-3_01" //Wheatly Elevator Scene - Post Crash

/*
Properties:
	The following properties are evaluated for EVERY vcd in a scene:
		[required] vcd 
				This is the actual scene entity. Must take the form
				 	vcd = CreateSceneEntity("my_fully_pathed_scene.vcd")
		[required] next 
				SceneTable index of next vcd in scene or null for last vcd in scene.
		[required] char
				Character who is speaking in this vcd. Valid characters:
					glados
					wheatley
					announcer
					turret
		[optional] postdelay
					Number of seconds to wait after finishing this vcd before firing the next VCD in the scene.
					If postdelay < 0 the next vcd starts abs(postdelay) seconds after this vcd STARTS. This lets one character talk over another character.
					Postdelay can take the format postdelay=[x,y], in which case postdelay evaluates to RandomFloat(x,y)
		[optional] predelay
					Number of seconds to wait before starting this vcd.
					Predelay can take the format predelay=[x,y], in which case predelay evaluates to RandomFloat(x,y)
		[optional] talkover
					If talkover exists, this vcd will not silence and currently playing vcds. This should always be set for postdelay<0.

		[optional] fires
					Each vcd in a scene can trigger an arbitrary number of ent_fires. The fires array defines these ent_fires for each vcd.
					By default, these fire at the end of the associated vcd. If fireatstart exists, however, the entry will fire at the START
					of the vcd, and delay defines the number of seconds to wait from the START of the vcd.
					ex:
						fires=
						[
							{entity="entity1",input="Trigger",parameter="",delay=2,fireatstart=true},
							{entity="entity2",input="RunScriptCode",parameter="DoMyThing(1)",delay=1.6}
						]	
						
						2 seconds after the START of the vcd, entity1 is sent "Trigger".
						1.6 seconds after the END of the vcd, entity2 is sent "RunScriptCode" with a parameter of DoMyThing(1)


	The following properties are evaluated only for the FIRST vcd in a scene:

		[optional] noDingOn
				By default, a "ding" noise is played at the beginning and end of a scene (this is the default because glados.nut was originally
				designed just for glados vo. If noDingOn exists, the starting ding is suppressed.
		[optional] noDingOff
				By default, a "ding" noise is played at the beginning and end of a scene (this is the default because glados.nut was originally
				designed just for glados vo. If noDingOff exists, the ending ding is suppressed.
		[optional] queue
				If queue exists, this scene will queue up if a scene is currently playing.
		[optional] queuetimeout
				A scene will be removed from the queue if it waits in the queue > queuetimeout seconds. This is only evaluated if queue exists.
		[optional] queueforcesecs
				If a scene has waited in the queue > queueforcesecs, it will stop whatever scene is playing and start. This is only evaluated if queue exists.
		[optional] skipifbusy
				The scene will only play if no other scene is playing.
*/


//SCENE: TestTwo
	if (curMapName=="sp_intro_03")
	{
		//Oh no, don't. Anyway, back to you two imbeciles killing me:
		SceneTable["-1_01"] <- 
		{
			vcd = CreateSceneEntity("scenes/npc/glados/gladosbattle_pre16.vcd")
			char = "glados"
			postdelay = [0.0,5.0]
			predelay = 0.0
			next = "-1_02"
			noDingOff = true
			noDingOn = true
			queue = true
			queuetimeout = 10
		}
		//Wait here. Don't go anywhere. I'll be back.
		SceneTable["-1_02"] <- 
		{	
			vcd = CreateSceneEntity("scenes/npc/glados/gladosbattle_pre18.vcd")
			char = "glados"
			next = null
			fires=
			[
				{entity="entity1",input="Trigger",parameter="",delay=2,fireatstart=true},
				{entity="entity2",input="RunScriptCode",parameter="DoMyThing(1)",delay=1.6}
			]		
		}
	}	


	if (curMapName=="sp_column_blocker")
	{
		//Hey, partner. I knew you'd be comin' through this shaft, so I talked my way onto this nanobot work crew over here that's rebuildin' it. They're REALLY small, so they got tiny little brains. But there's a billion of 'em, so it's only a matter of time
		SceneTable["-2_01"] <- 
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/NanobotInto01.vcd")
			char = "wheatley"
			postdelay = 0.2
			predelay = 0.0
			next = "-2_02"
			noDingOff = true
			noDingOn = true
		}
		//Hold on... I'm on BREAK, Jerry.
		SceneTable["-2_02"] <- 
		{	
			vcd = CreateSceneEntity("scenes/npc/sphere03/NanobotOnBreak01.vcd")
			char = "wheatley"
			next = "-2_03"
		}
		//Anyway, look, we're real close to bustin' out. So just hang in there for five more chambers.
		SceneTable["-2_03"] <- 
		{	
			vcd = CreateSceneEntity("scenes/npc/sphere03/NanobotRealClose01.vcd")
			char = "wheatley"
			next = null
		}
		
		//Ow!
		SceneTable["-3_01"] <- 
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/NanobotOw01.vcd")
			char = "wheatley"
			postdelay = 1.0
			predelay = 0.0
			next = "-3_02"
			noDingOff = true
			noDingOn = true
		}
		//What? You can't fire me!
		SceneTable["-3_02"] <- 
		{	
			vcd = CreateSceneEntity("scenes/npc/sphere03/NanobotFired01.vcd")
			char = "wheatley"
			next = "-3_03"
		}
		//Well, JERRY -- maybe your prejudiced worksite could have accommodated a nanobot of my size! You'll be hearing from my lawyer! Thanks for the HATE CRIME, Jerry!
		SceneTable["-3_03"] <- 
		{	
			vcd = CreateSceneEntity("scenes/npc/sphere03/NanobotLawyers03.vcd")
			postdelay = 0.5
			char = "wheatley"
			next=null
			//next = "-3_04"
		}
		//We're not actually going to sue them, I just don't want them to report this. I don't even HAVE a lawyer. In fact, if I EVER retain counsel, I will DIE. Oh, I gotta go. I'll see you soon.
		//SceneTable["-3_04"] <- 
		//{	
		//	vcd = CreateSceneEntity("scenes/npc/sphere03/NanobotLawyers02.vcd")
		//	char = "wheatley"
		//	next = "-3_05"
		//}



//		NanobotOw01.wav	Ow!
		
//		NanobotFired01.wav	What? You can't fire me!
//		NanobotLawyers01.wav	Well, JERRY -- maybe your prejudiced worksite could have accommodated a nanobot of my size! You'll be hearing from my lawyer! Thanks for the HATE CRIME, Jerry!
//		NanobotLawyers02.wav	We're not actually going to sue them, I just don't want them to report this. I don't even HAVE a lawyer. In fact, if I EVER retain counsel, I will DIE. Oh, I gotta go. I'll see you soon.

	}	


	
// ============================================================================
// Drop a companion cube out of a dropper in sp_laser_lift_pit_flings
// if the player gets into an unwinnable situation
// NOTE: This is called EVERY TIME the cube is lost.
//		 The very first time a cube is dropped it will be a companion cube
// ============================================================================
function sp_laser_lift_pit_flings_cube_lost()
{
	printl("*** CUBE LOST!  Spawning new box...")
	EntFire( "drop_new_box_relay", "trigger", 0, 1 )	
}


// ============================================================================
// Called when sp_laser_lift_pit_flings companion cube is picked up
// ============================================================================
function sp_laser_lift_pit_flings_companion_cube_dissolved()
{
		printl("***DISSOLVING cube_dropper_box!")
		
		printl("*** HACK!  vscript dropping cube_dropper_box. Remove this once dissolving a carried object no longer crashes the game!")
		EntFire( "cube_dropper_box", "disablemotion", 0, 0, 0 )
		EntFire( "cube_dropper_box", "enablemotion", 0, 0.1, 0 )
		
		// spawn a new box whenever ready
		sp_laser_lift_pit_flings_cube_lost();
}