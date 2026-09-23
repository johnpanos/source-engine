/*
Properties:
	The following properties are evaluated for EVERY vcd in  a scene:
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
				
	Nag/Idle tags:
		-Every vcd in a nag/idle block should have "next" set to null (unless they are part of a "nag chain" - see idleindex/idleunder below)
		-You can stop a nag cycle by calling @glados.GladosStopNag() or @glados.WheatleyStopNag()
		
		The *FIRST* vcd in a nag block should have:
			idle=true
			idlerandom=true 					[optional] if you want the vcds to play in random order, otherwise they play in order by "idleorderingroup"
			idlerepeat=true 					[optional] if you want the list to repeat after exhausting the list once
			idlerandomonrepeat=true 	[optional] if you want the list to play in order the first time and then randomize on repeats, mutually exclusive with idlerandom & idlerepeat
			idleminsecs=0.300 				minimum # of seconds between nags
			idlemaxsecs=1.500					maximum # of seconds between nags		
			
		*Every* vcd should have:
			idlegroup 				- a string that is the same for all vcds in the nag cycle block		
			idleorderingroup	- an integer ordering this vcd in the block		
			idlemaxplays			- [optional] once a vcd plays this many times, retire it. [default is never retire]
			idlerarity				- [optional] Every time the nag list is rebuilt (happens the first time the list is built, plus every time the list is exhausted) this vcd has idlerarity% chance to be included
			idleindex					- [optional] a unique id within the nag block used to group a set of vcds that should be played together as one "nag". This should be set only on the first vcd of the group
			idleunder					- [optional] points to idleindex for members of a nag group other that the first one
			
		//Example nag/idle cycle

		//TestIdle
		//Oops.
			//SceneTable["sp_intro_03TestIdle01"] <- {vcd=CreateSceneEntity("scenes/npc/glados/gladosbattle_xfer03.vcd"),postdelay=0.1,next=null,char="glados",noDingOff=true,noDingOn=true,idle=true,idlerandom=true,idlerepeat=true,idleminsecs=0.300,idlemaxsecs=1.500,idlegroup="sp_intro_03testidle",idleorderingroup=1}
		//That's funny, I don't feel corrupt. In fact, I feel pretty good.
			//SceneTable["sp_intro_03TestIdle02"] <- {vcd=CreateSceneEntity("scenes/npc/glados/gladosbattle_xfer04.vcd"),postdelay=0.1,next=null,char="glados",noDingOff=true,noDingOn=true,idlegroup="sp_intro_03testidle",idleorderingroup=2}
		//Core transfer?
			//SceneTable["sp_intro_03TestIdle03"] <- {vcd=CreateSceneEntity("scenes/npc/glados/gladosbattle_xfer05.vcd"),postdelay=0.1,next="sp_intro_03TestIdle0301",char="glados",noDingOff=true,noDingOn=true,idlegroup="sp_intro_03testidle",idleorderingroup=3,idleindex=1}
		//NAG GROUP: Oh, you are kidding me.
			//SceneTable["sp_intro_03TestIdle0301"] <- {vcd=CreateSceneEntity("scenes/npc/glados/gladosbattle_xfer06.vcd"),postdelay=0.1,next=null,char="glados",noDingOff=true,noDingOn=true,idlegroup="sp_intro_03testidle",idleorderingroup=4,idleunder=1}
		//No!
			//SceneTable["sp_intro_03TestIdle05"] <- {vcd=CreateSceneEntity("scenes/npc/glados/gladosbattle_xfer07.vcd"),postdelay=0.1,next=null,char="glados",noDingOff=true,noDingOn=true,idlegroup="sp_intro_03testidle",idleorderingroup=5}
		//Yes! You little worm!
			//SceneTable["sp_intro_03TestIdle06"] <- {vcd=CreateSceneEntity("scenes/npc/glados/gladosbattle_xfer10.vcd"),postdelay=0.1,next=null,char="glados",noDingOff=true,noDingOn=true,idlegroup="sp_intro_03testidle",idleorderingroup=6}
		//Don't do it.
			//SceneTable["sp_intro_03TestIdle07"] <- {vcd=CreateSceneEntity("scenes/npc/glados/gladosbattle_xfer12.vcd"),postdelay=0.1,next=null,char="glados",noDingOff=true,noDingOn=true,idlegroup="sp_intro_03testidle",idleorderingroup=7}
		//Not so fast!
			//SceneTable["sp_intro_03TestIdle08"] <- {vcd=CreateSceneEntity("scenes/npc/glados/gladosbattle_xfer14.vcd"),postdelay=0.1,next=null,char="glados",noDingOff=true,noDingOn=true,idlegroup="sp_intro_03testidle",idleorderingroup=8}
*/



//SceneTableLookup

//MAKE SURE THE INDEXES ON THIS ARRAY ARE NEGATIVE!!!!!

SceneTableLookup[-3000] <- "-3000_01" // find potatos for first time

SceneTableLookup[-3001] <- "-3001_01" // pick up potatos for first time

SceneTableLookup[-3002] <- "-3002_01" // discovering 'Caroline' in portrait in office


	//=================================================================
        //Called when player enters room Potatos is sitting in
	//=================================================================
	if (curMapName=="sp_a3_transition01")
	{
		//Oh. It's you. Go away.
		SceneTable["-3000_01"] <-
		{
			vcd=CreateSceneEntity("scenes/npc/glados/potatos_meetup02.vcd"),
			postdelay=4.0,
			next="-3000_02",
			char="glados",
			noDingOff=true,
			noDingOn=true,
		}	
		//Come to gloat?
		SceneTable["-3000_02"] <-
		{
			vcd=CreateSceneEntity("scenes/npc/glados/potatos_meetup03.vcd"),
			postdelay=[0.8,1.4],
			next="-3000_03",
			char="glados",
			noDingOff=true,
			noDingOn=true,
		}
		//Go on. Get a goooood lonnnnng look.
		SceneTable["-3000_03"] <-
		{
			vcd=CreateSceneEntity("scenes/npc/glados/potatos_meetup04.vcd"),
			postdelay=[0.8,1.4],
			next="-3000_04",
			char="glados",
			noDingOff=true,
			noDingOn=true,
		}
		//Go on. Get a big fat eyeful. With your big fat eyes.
		SceneTable["-3000_04"] <-
		{
			vcd=CreateSceneEntity("scenes/npc/glados/potatos_meetup05.vcd"),
			postdelay=[0.8,1.4],
			next="-3000_05",
			char="glados",
			noDingOff=true,
			noDingOn=true,
		}
		//That's right. A potato just called your eyes fat.
		SceneTable["-3000_05"] <-
		{
			vcd=CreateSceneEntity("scenes/npc/glados/potatos_meetup06.vcd"),
			postdelay=[0.8,1.4],
			next="-3000_06",
			char="glados",
			noDingOff=true,
			noDingOn=true,
			idlegroup="sp_a3_transition01_find_potatos_nag",
			idleorderingroup=4
		}
		//Now your fat eyes have seen everything.
		SceneTable["-3000_06"] <-
		{
			vcd=CreateSceneEntity("scenes/npc/glados/potatos_meetup07.vcd"),
			postdelay=[0.8,1.4],
			next="-3000_07",
			char="glados",
			noDingOff=true,
			noDingOn=true,
		}
		//In case you were wondering: Yes. I'm still a potato. Go away.
		SceneTable["-3000_07"] <-
		{
			vcd=CreateSceneEntity("scenes/npc/glados/potatos_meetup08.vcd"),
			postdelay=[0.8,1.4],
			next="-3000_08",
			char="glados",
			noDingOff=true,
			noDingOn=true,
		}
		//Wait. Why DID you trundle over here? You're not HUNGRY, are you? It's hard to see. What do you have in your hand? Knowing you it's a deep fryer.
		SceneTable["-3000_08"] <-
		{
			vcd=CreateSceneEntity("scenes/npc/glados/potatos_meetup10.vcd"),
			postdelay=[0.8,1.4],
			next="-3000_09",
			char="glados",
			noDingOff=true,
			noDingOn=true,
		}
		//Stay back.
		SceneTable["-3000_09"] <-
		{
			vcd=CreateSceneEntity("scenes/npc/glados/potatos_meetup11.vcd"),
			postdelay=0.0,
			next=null,
			char="glados",
			noDingOff=true,
			noDingOn=true,
		}
	
	//=================================================================
        //Called when player picks up Potatos for the first time
	//=================================================================

		//What are you doing? Put me back this instant.
		SceneTable["-3001_01"] <-
		{
			vcd=CreateSceneEntity("scenes/npc/glados/potatos_postpickup05.vcd"),
			char="glados"
			postdelay=0.0,
			predelay = 0.3
			next = "-3001_02"
			noDingOff = true
			noDingOn = true
		}
		//Put me back. Put me back. Put–[Robot voice] Battery low. Powering down.
		SceneTable["-3001_02"] <-
		{
			vcd=CreateSceneEntity("scenes/npc/glados/potatos_postpickup08.vcd"),
			char="glados"
			postdelay=0.0,
			predelay = 0.0
			next = "null"
		}
	}

	//=================================================================
        //Called when discovering 'Caroline' in portrait in office
	//=================================================================
	if (curMapName=="sp_a3_speed_ramp")
	{
		//Caroline ... Her name was Caroline. I remember her. Why do I remember her?
		SceneTable["-3002_01"] <-
		{
			vcd=CreateSceneEntity("scenes/npc/glados/potatos_familiarportrait01.vcd"),
			char="glados"
			postdelay=10.0,
			predelay = 0.3
			next = "-3002_02"
			noDingOff = true
			noDingOn = true
		}
		//Caroline...
		SceneTable["-3002_02"] <-
		{
			vcd=CreateSceneEntity("scenes/npc/glados/potatos_familiarportrait02.vcd"),
			char="glados"
			postdelay=0.0,
			predelay = 0.0
			next = "null"
		}
	}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

function sp_a3_transition01_find_potatos()
{
	GladosPlayVcd( -3000 )
}
function sp_a3_transition01_pickup_potatos()
{
	GladosPlayVcd( -3001 )
}
function sp_a3_speed_ramp_caroline()
{
	GladosPlayVcd( -3002 )
}
