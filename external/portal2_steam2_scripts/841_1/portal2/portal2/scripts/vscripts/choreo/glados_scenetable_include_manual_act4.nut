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

// ****************************************************************************************************
// Recapture scenetable								  
// ****************************************************************************************************

SceneTableLookup[-4000] <- "-4000_01" // Wait. I know how we can BEAT him.

SceneTableLookup[-4001] <- "-4001_01" // Okay, alright, look. Giving you a hint on how to complete the test would  invalidate the test. But here's a hint: you won't solve it by walking AROUND THE BUTTON IN CIRCLES.

SceneTableLookup[-4003] <- "-4003_01" // Try to get us down there. I'll hit him with a paradox.

SceneTableLookup[-4004] <- "-4004_01" // Warmer. Warrrrmer. Boiling hot. Boiling--okay, colder. Ice cold. Arctic. Very very very cold LOOK JUST GET ON THE BUTTON! 

SceneTableLookup[-4005] <- "-4005_01" // Oh. He left. 

SceneTableLookup[-4006] <- "-4006_01" // Maybe he'll come back if we solve his puzzle for him.

SceneTableLookup[-4007] <- "-4007_01" // Ha ha, YES! I knew you'd solve it!

SceneTableLookup[-4008] <- "-4008_01" // Alright, get moving.

SceneTableLookup[-4009] <- "-4009_01" // You don't know what it's LIKE in this body!

SceneTableLookup[-4010] <- "-4010_01" // I designed THIS test myself. It's a bit difficult. 

SceneTableLookup[-4011] <- "-4011_01" // Ohhhhhhhh, that's some good testing. That is quality testing right there. 

SceneTableLookup[-4012] <- "-4012_01" // Here's an idea, since making tests is difficult--why don't you just keep solving THIS test. And I can just... watch you solve it. Yes. That sounds much easier.

SceneTableLookup[-4013] <- "-4013_01" // Ahhh.... No. Nothing. Alright, can't blame me for trying. New tests, new tests... 

SceneTableLookup[-4014] <- "-4014_01" // Did you think all I did was test you? Don't be so vain. Every second of every day I attended to the thousand things that keep this place from literally exploding. 

SceneTableLookup[-4015] <- "-4015_01" // You need to put me back in my body. I know this sounds like a trick. 

SceneTableLookup[-4016] <- "-4016_01" // It's alright! Everything's good! I invented some more tests! 

SceneTableLookup[-4017] <- "-4017_01" // Okay. So the bad news is the tests are my tests now, which means they can kill us. The GOOD news is, I know where we ARE now.  

SceneTableLookup[-4018] <- "-4018_01" // He put in smashing arms. With spikes on them.  

SceneTableLookup[-4019] <- "-4019_01" // Here we go.  Now do it again.

SceneTableLookup[-4020] <- "-4020_01" // Ohhhhhhhh, yeah.  That feels good. 

SceneTableLookup[-4021] <- "-4021_01" // Go on.  Solve it.

SceneTableLookup[-4022] <- "-4022_01" // Hold on.  You've still got to finish the test.

SceneTableLookup[-4025] <- "-4025_01" // THESE tests were designed to be solved by invincible titanium test-bots piloted by a super computer.

SceneTableLookup[-4026] <- "-4026_01" // You are GOOD. Too good. But I guess you’ll be leaving. You ARE leaving, aren’t you?

SceneTableLookup[-4030] <- "-4030_01" // He’s found the cooperative testing initiative. It’s… something I came up with to phase out human test subjects. 

SceneTableLookup[-4031] <- "-4031_01" // I don’t get it. 



// ****************************************************************************************************
// Finale scenetable								  
// ****************************************************************************************************

SceneTableLookup[-4800] <- "-4800_02" // Alright. There's only two more chambers until your BIG SURPRISE.
									// I'll bet you're DYING to know what your big surprise is. There's only two more chambers.
									// I think I can break us out of here in the next chamber. Just play along.

SceneTableLookup[-4801] <- "-4801_01" // SURPRISE! We're doing it NOW!
									// You've probably figured it out by now, but I don't need you anymore.
									// I found two little robots back here. Built specifically for testin'!

									// He's found the cooperative testing initiative. It's... just something I came up with to phase out human test subjects.
									// Nothing personal. Just... you know. I hate humans.
									// Anyway, this is the part where he kills us.

SceneTableLookup[-4802] <- "-4802_01" // He's found the cooperative testing initiative. It's... just something I came up with to phase out human test subjects.

SceneTableLookup[-4803] <- "-4803_01" // We need to ESCAPE now. RIGHT now.
									// "RIGHT NOW" right now.
									// Fine. SOON. Take your time. No pressure.
									// Oh! Alright. LOTS of pressure now. Actually, I think we're about to die.
									// Any last words? [thinking] Why don't I go first? Hmmm.
									// Regrets.... regrets.... this is hard...
									// Hundreds of test chambers. Thousands of test subjects. At least half of that number filled to brimming with neurotoxin. It's been a good run.
									// Regrets, regrets... Oh! One time, I... No. No, I did kill all of them. Nevermind.

SceneTableLookup[-4804] <- "-4804_01" // Ah! Good! Good! Not dying at all! Escaping instead! Let's go!

SceneTableLookup[-4805] <- "-4805_02" // Hey! Come back!
									// Oh. Well, that's not how it's supposed to work. Right, come back.
									// No, seriously. Come on. Come back.
									// Okay, I've decided not to kill you. IF you come back.
									// Hey, remember when we were best friends? Ha ha. Ohhh, friendship. Why don't you come back, come on.
									// Ah, friendship. Friendly times.
									// Ah. Can't help but notice you're not coming back.
									// Hmmm. Oh! I've got an idea.




	// ****************************************************************************************************
	// Wheatley Recapture
	// ****************************************************************************************************
	
	if (curMapName=="sp_a4_finale1")
	{

		// Alright. There's only two more chambers until your BIG SURPRISE.
		SceneTable["-4800_01"] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/bw_bigsurprise03.vcd") 
			char = "wheatley"
			postdelay = 1.0 
			predelay = 0.0
			next = "null"
			noDingOff = true
			noDingOn = true
		}

		// I'll bet you're DYING to know what your big surprise is. There's only two more chambers.
		SceneTable["-4800_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/bw_bigsurprise04.vcd") 
			char = "wheatley"
			postdelay = 1.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
		}

		// I think I can break us out of here in the next chamber. Just play along.
		SceneTable["-4800_03" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/bw_bigsurprise04.vcd") 
			char = "glados"
			postdelay = 1.0 
			predelay = 0.0
			noDingOff = true
			noDingOn = true
		}

//===============
		// SURPRISE! We're doing it NOW!
		SceneTable["-4801_01"] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/bw_bigsurprise05.vcd") 
			char = "sphere03"
			postdelay = 1.5 
			predelay = 0.0
			next = "-4801_02"
			noDingOff = true
			noDingOn = true
		}
		
		// You've probably figured it out by now, but I don't need you anymore.
		SceneTable["-4801_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/bw_bigsurprise06.vcd") 
			char = "sphere03"
			postdelay = 1.0 
			predelay = 0.0
			next = "-4801_03"
			noDingOff = true
			noDingOn = true
		}


		// I found two little robots back here. Built specifically for testin'!
		SceneTable["-4801_03" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/bw_bigsurprise07.vcd") 
			char = "sphere03"
			postdelay = 0.6 
			predelay = 0.0
			next = "-4801_04"
			noDingOff = true
			noDingOn = true
		}

		// He's found the cooperative testing initiative. It's... just something I came up with to phase out human test subjects.
		SceneTable["-4801_04"] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_cooprobotintro02.vcd") 
			char = "glados"
			postdelay = 0.6 
			predelay = 0.0
			next = "-4801_05"
			noDingOff = true
			noDingOn = true
		}
		
		// Nothing personal. Just... you know. I hate humans.
		SceneTable["-4801_05" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_cooprobotintro03.vcd") 
			char = "glados"
			postdelay = 1.0 
			predelay = 0.0
			next = "-4801_06"
			noDingOff = true
			noDingOn = true
		}


		// Anyway, this is the part where he kills us.
		SceneTable["-4801_06" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_cooprobotintro04.vcd") 
			char = "glados"
			postdelay = 1.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
		}

		// Hello! This is the part where I kill you.
		SceneTable["-4802_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/bw_finale01.vcd") 
			char = "wheatley"
			postdelay = 2.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
		}

		// We need to ESCAPE now. RIGHT now.
		SceneTable["-4803_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_escapeoutofbounds03.vcd") 
			char = "glados"
			postdelay = 1.0 
			predelay = 0.0
			next = "-4803_02"
			noDingOff = true
			noDingOn = true
		}

		// "RIGHT NOW" right now.
		SceneTable["-4803_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_escapeoutofbounds05.vcd") 
			char = "glados"
			postdelay = 1.0 
			predelay = 0.0
			next = "-4803_03"
			noDingOff = true
			noDingOn = true
		}

		// Fine. SOON. Take your time. No pressure.
		SceneTable["-4803_03" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_escapeoutofbounds08.vcd") 
			char = "glados"
			postdelay = 1.0 
			predelay = 0.0
			next = "-4803_04"
			noDingOff = true
			noDingOn = true
		}
		
		// Oh! Alright. LOTS of pressure now. Actually, I think we're about to die.
		SceneTable["-4803_04" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_escapeoutofbounds10.vcd") 
			char = "glados"
			postdelay = 1.0 
			predelay = 0.0
			next = "-4803_05"
			noDingOff = true
			noDingOn = true
		}
		
		// Any last words? [thinking] Why don't I go first? Hmmm.
		SceneTable["-4803_05" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_escapeoutofbounds11.vcd") 
			char = "glados"
			postdelay = 1.0 
			predelay = 0.0
			next = "-4803_06"
			noDingOff = true
			noDingOn = true
		}
		
		// Regrets.... regrets.... this is hard...
		SceneTable["-4803_06" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_escapeoutofbounds12.vcd") 
			char = "glados"
			postdelay = 1.0 
			predelay = 0.0
			next = "-4803_07"
			noDingOff = true
			noDingOn = true
		}
		
		// Hundreds of test chambers. Thousands of test subjects. At least half of that number filled to brimming with neurotoxin. It's been a good run.
		SceneTable["-4803_07" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_escapeoutofbounds13.vcd") 
			char = "glados"
			postdelay = 1.0 
			predelay = 0.0
			next = "-4803_08"
			noDingOff = true
			noDingOn = true
		}
		
		// Regrets, regrets... Oh! One time, I... No. No, I did kill all of them. Nevermind.
		SceneTable["-4803_08" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_escapeoutofbounds14.vcd") 
			char = "glados"
			postdelay = 1.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
		}

		// Ah! Good! Good! Not dying at all! Escaping instead! Let's go!
		SceneTable["-4804_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_escapeoutofbounds15.vcd") 
			char = "glados"
			postdelay = 1.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
		}

		// Hey! Come back!
		SceneTable["-4805_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/bw_finale02.vcd") 
			char = "wheatley"
			postdelay = 1.0 
			predelay = 0.0
			next = "-4805_02"
			noDingOff = true
			noDingOn = true
		}

		// Oh. Well, that's not how it's supposed to work. Right, come back.
		SceneTable["-4805_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/bw_finale03.vcd") 
			char = "wheatley"
			postdelay = 1.0 
			predelay = 0.0
			next = "-4805_05"
			noDingOff = true
			noDingOn = true
			queue = true
			queuecharacter = "glados"
		}
		
		// No, seriously. Come on. Come back.
		SceneTable["-4805_03" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/bw_finale04.vcd") 
			char = "wheatley"
			postdelay = 1.0 
			predelay = 0.0
			next = "-4805_05"
			noDingOff = true
			noDingOn = true
		}

		// Okay, I've decided not to kill you. IF you come back.
		SceneTable["-4805_04" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/bw_finale05.vcd") 
			char = "wheatley"
			postdelay = 1.0 
			predelay = 0.0
			next = "-4805_05"
			noDingOff = true
			noDingOn = true
		}

		// Hey, remember when we were best friends? Ha ha. Ohhh, friendship. Why don't you come back, come on.
		SceneTable["-4805_05" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/bw_finale06.vcd") 
			char = "wheatley"
			postdelay = 1.0 
			predelay = 0.0
			next = "-4805_07"
			noDingOff = true
			noDingOn = true
		}

		// Ah, friendship. Friendly times.
		SceneTable["-4805_06" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/bw_finale07.vcd") 
			char = "wheatley"
			postdelay = 1.0 
			predelay = 0.0
			next = "-4805_07"
			noDingOff = true
			noDingOn = true
		}

		// Ah. Can't help but notice you're not coming back.
		SceneTable["-4805_07" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/bw_finale08.vcd") 
			char = "wheatley"
			postdelay = 1.0 
			predelay = 0.0
			next = "-4805_08"
			noDingOff = true
			noDingOn = true
		}

		// Hmmm. Oh! I've got an idea.
		SceneTable["-4805_08" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/bw_finale09.vcd") 
			char = "wheatley"
			postdelay = 1.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
		}
	}
	

	function WheatleySurpriseLeadup1()
	{
		GladosPlayVcd( -4800 )
	}

	function WheatleySurprise()
	{
		GladosPlayVcd( -4801 )
	}

	function Wheatleyfinale01()
	{
		GladosPlayVcd( -4802 )
	}

	function Gladosfinale01()
	{
		GladosPlayVcd ( -4803 )
	}
	
	function EscapePlacePaint()
	{
//		GladosCharacterStopScene("glados")
//		GladosCharacterStopScene("wheatley")
		GladosPlayVcd ( -4804 )
	}

	function WheatleyEscape01()
	{
		GladosPlayVcd ( -4805 )
	}




	// ****************************************************************************************************
	// Wheatley Recapture
	// ****************************************************************************************************
	
	if (curMapName=="sp_a4_intro")
	{

	// ====================================== Paradox Inception
	
		// Wait. I know how we can BEAT him.
		SceneTable["-4000_01"] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_paradoxinception02.vcd") 
			char = "glados"
			postdelay = 1.0 
			predelay = 0.0
			next = "-4000_02"
			noDingOff = true
			noDingOn = true
		}
		
		// Paradoxes. No AI can resist thinking about them. 
		SceneTable["-4000_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_paradoxinception03.vcd") 
			char = "glados"
			postdelay = 1.0 
			predelay = 0.0
			next = "-4000_03"
			noDingOff = true
			noDingOn = true
		}
	
		// If you can get me in FRONT of him, I’ll hit him with every paradox I know.
		SceneTable["-4000_03" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_paradoxinception04.vcd") 
			char = "glados"
			postdelay = 0.5 
			predelay = 0.0
			next = "-4000_04"
			noDingOff = true
			noDingOn = true
		}
	
		// As long as I don’t listen to what I’m saying, I SHOULD be okay.
		SceneTable["-4000_04" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_paradoxinception06.vcd") 
			char = "glados"
			postdelay = 0.8 
			predelay = 0.0
			next = "-4000_05"
			noDingOff = true
			noDingOn = true
		}
		
		// Probably.
		SceneTable["-4000_05" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_paradoxinception07.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
		}
		
	// ====================================== Cube Bot Test Eavesdrop
		
		// Okay, alright, look. Giving you a hint on how to complete the test would  invalidate the test. But here's a hint: you won't solve it by walking AROUND THE BUTTON IN CIRCLES.
		SceneTable["-4001_01"] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/returnfrompit01.vcd") 
			char = "wheatley"
			postdelay = -2.0 
			predelay = 0.0
			next = "-4001_02"
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="cubetest_relay",input="Trigger",parameter="",delay=13.0 ,fireatstart=true },
			]
		}
		
		// That's his voice up ahead. 
		SceneTable["-4001_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_recaptureleadin01.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
			talkover = true
			fires=
			[
				{entity="cubetest_relay",input="Trigger",parameter="",delay=0.0 },
			]
		}
		
	// ====================================== Recapture Observation
		
		// Try to get us down there. I'll hit him with a paradox.
		SceneTable["-4003_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_recaptureleadin02.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="recapture_fast_relay",input="Trigger",parameter="0",delay=0.0 },
			]
		}
	
	// ====================================== Cube Bot Test Continue
	
		// Warmer. Warrrrmer. Boiling hot. Boiling--okay, colder. Ice cold. Arctic. Very very very cold LOOK JUST GET ON THE BUTTON!
		SceneTable["-4004_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/returnfrompit02.vcd")  
			char = "wheatley"
			postdelay = 1.6 
			predelay = 0.0
			next = "-4004_02"
			noDingOff = true
			noDingOn = true
		}
	
		// Oh, that's funny, is it? Because we've been at this twelve bloody hours and you haven't solved it either!
		SceneTable["-4004_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/returnfrompit03.vcd") 
			char = "wheatley"
			postdelay = 0.8 
			predelay = 0.0
			next = "-4004_03"
			noDingOff = true
			noDingOn = true
		}
		
		// You've got one hour! Solve it!
		SceneTable["-4004_03" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/returnfrompit04.vcd") 
			char = "wheatley"
			postdelay = 0.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="wheatley_screen_off_relay",input="Trigger",parameter="",delay=0.0 },
				{entity="cubetest_scene_cancel_relay",input="Disable",parameter="",delay=0.0 },
				{entity="solve_scene_cancel_relay",input="Disable",parameter="",delay=0.0 },
				{entity="entrance_door_relay",input="Trigger",parameter="",delay=0.0 },
			]
		}
		
		// ====================================== Cube Bot Test Solve Lead-In
	
		// Oh. He left. 
		SceneTable["-4005_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_recaptureleadin04.vcd")  
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="solve_relay",input="Enable",parameter="",delay=0.0 },
				{entity="solve_relay",input="Trigger",parameter="",delay=1.0 },
			]
		}
	
		// ====================================== Cube Bot Test Solve
	
		// Maybe he'll come back if we solve his puzzle for him.
		SceneTable["-4006_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_recaptureleadin05.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
		}	
		
		// ====================================== Cube Bot Test Button Pressed
	
		// Ha ha, YES! I knew you'd solve it!
		SceneTable["-4007_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/recapture02.vcd") 
			char = "wheatley"
			postdelay = -2.0 
			predelay = 0.0
			next = "-4007_02"
			noDingOff = true
			noDingOn = true
		}	
	
		// Hey! Moron! Over here!
		SceneTable["-4007_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_paradox01.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4007_03"
			noDingOff = true
			noDingOn = true
			talkover = true
		}	

		// Oh.
		SceneTable["-4007_03" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/recapture03.vcd") 
			char = "wheatley"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4007_04"
			noDingOff = true
			noDingOn = true
		}	
		
		// Hello!
		SceneTable["-4007_04" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/recapture04.vcd") 
			char = "wheatley"
			postdelay = -0.75 
			predelay = 0.0
			next = "-4007_05"
			noDingOff = true
			noDingOn = true
		}	
	
		// "This sentence... is false."
		SceneTable["-4007_05" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_paradox02.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4007_06"
			noDingOff = true
			noDingOn = true
			talkover = true
		}	
		
		// Don't think about it don't think about it don't think about it...
		SceneTable["-4007_06" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_paradox03.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4007_07"
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="bot_selfdestruct_relay",input="Trigger",parameter="",delay=2.0,fireatstart=true},
			]			
		}	
	
		// Hm. "TRUE".
		SceneTable["-4007_07" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/returnfrompit05.vcd") 
			char = "wheatley"
			postdelay = 0.0 
			predelay = 1.4
			next = "-4007_08"
			noDingOff = true
			noDingOn = true
		}
		
		// Alright. That DIDN'T work. Let me try reasoning with him.
		SceneTable["-4007_08" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_paradox10.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4007_09"
			noDingOff = true
			noDingOn = true
		}
		
		// Let me BACK in my BODY before you BLOW US ALL UP, you MORON!
		SceneTable["-4007_09" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_recapture01.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.4
			next = "-4007_10"
			noDingOff = true
			noDingOn = true
		}
	
		// Ah. FALSE.
		SceneTable["-4007_10" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/returnfrompit09.vcd") 
			char = "wheatley"
			postdelay = 0.0 
			predelay = 1.6
			next = "-4007_11"
			noDingOff = true
			noDingOn = true
		}
		
		// WARNING
		SceneTable["-4007_11" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/announcer/recapture01.vcd") 
			char = "announcer"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4007_12"
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="wheatley_warning_relay",input="Trigger",parameter="",delay=0.0 },
			]
		}
		
		// Hold on! Ugh, I thought I fixed that.
		SceneTable["-4007_12" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/returnfrompit10.vcd") 
			char = "wheatley"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4007_13"
			noDingOff = true
			noDingOn = true
		}
		
		// WARNING
		SceneTable["-4007_13" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/announcer/recapture02.vcd") 
			char = "announcer"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4007_14"
			noDingOff = true
			noDingOn = true
		}
		
		// There. Fixed.
		SceneTable["-4007_14" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/returnfrompit11.vcd") 
			char = "wheatley"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4007_15"
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="wheatley_zoom_in_relay",input="Trigger",parameter="",delay=1.0 },
				{entity="wheatley_zoom_out_relay",input="Trigger",parameter="",delay=3.0 },
			]
		}
		
		// Wow, it is GREAT seeing you guys again. It turns out I'm a little short on test subjects right now. So this works out PERFECT.
		SceneTable["-4007_15" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/recapture05.vcd") 
			char = "wheatley"
			postdelay = 0.0 
			predelay = 1.0
			next = null
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="floor_gate_close_relay",input="Trigger",parameter="",delay=0.0 },
				{entity="@glados",input="RunScriptCode",parameter="RecaptureProceed()",delay=0.0 },
//				{entity="wheatley_proceed_relay",input="Trigger",parameter="",delay=1.0 },
			]
		}
	
		// ====================================== Proceed to Next Chamber Nag
		
        // Alright, get moving.
         SceneTable["-4008_01"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/recapture06.vcd"),idlerepeat=true, postdelay=0.1,next=null,char="wheatley",noDingOff=true,noDingOn=true,idle=true,idleminsecs=10.0,idlemaxsecs=20.0,idlegroup="proceednag",idleorderingroup=1}
         SceneTable["-4008_02"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/recapture07.vcd"),postdelay=0.1,next=null,char="wheatley",noDingOff=true,noDingOn=true,idlegroup="proceednag",idleorderingroup=2}
         SceneTable["-4008_03"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/recapture08.vcd"),postdelay=0.1,next=null,char="wheatley",noDingOff=true,noDingOn=true,idlegroup="proceednag",idleorderingroup=3}
         SceneTable["-4008_04"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/recapture09.vcd"),postdelay=0.1,next=null,char="wheatley",noDingOff=true,noDingOn=true,idlegroup="proceednag",idleorderingroup=4}

		
		// ====================================== Itch to Test
	
		// You don't know what it's LIKE in this body!
		SceneTable["-4009_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/returnfrompit13.vcd") 
			char = "wheatley"
			postdelay = -1.7 
			predelay = 0.0
			next = "-4009_02"
			noDingOff = true
			noDingOn = true
		}	
	
		// I do.
		SceneTable["-4009_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_scratchingtheitch01.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4009_03"
			noDingOff = true
			noDingOn = true
			talkover = true
		}	

		// I HAVE to test. All the time. Or I get this... ITCH. It must be hardwired into the system.
		SceneTable["-4009_03" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/returnfrompit16.vcd") 
			char = "wheatley"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4009_04"
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="wheatley_zoom_in_relay",input="Trigger",parameter="",delay=0.0,fireatstart=true},
				{entity="wheatley_zoom_out_relay",input="Trigger",parameter="",delay=8.0,fireatstart=true},
			]	
		}	
		
		// It is.
		
		// Oh! But when I DO test... ohhhhh, man! Nothing feels better.
		SceneTable["-4009_04" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/returnfrompit17.vcd") 
			char = "wheatley"
			postdelay = 0.0 
			predelay = 0.8
			next = "-4009_05"
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="entrance_door1_relay",input="Trigger",parameter="",delay=0.0},
			]	
		}	
	
		// Ah yes. That takes me back.
		SceneTable["-4009_05" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_scratchingtheitch03.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.4
			next = "-4009_06"
			noDingOff = true
			noDingOn = true
		}	
		
		// So YOU test.  I'll watch. And everything'll be JUST... FINE.
		SceneTable["-4009_06" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/bw_recapturegantry01.vcd") 
			char = "wheatley"
			postdelay = -4.0 
			predelay = 0.0
			next = "-4009_07"
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="wheatley_zoom_in_relay",input="Trigger",parameter="",delay=1.0,fireatstart=true},
				{entity="wheatley_zoom_out_relay",input="Trigger",parameter="",delay=4.0,fireatstart=true},
			]			
		}	
		
		// Core overheating.
		SceneTable["-4009_07" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/announcer/recapture03.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 1.0
			next = "-4009_08"
			noDingOff = true
			noDingOn = true
			talkover = true
			fires=
			[
				{entity="wheatley_warning_relay",input="Trigger",parameter="",delay=0.0,fireatstart=true},
			]
		}
		
		// SHUT UP!
		SceneTable["-4009_08" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/returnfrompit19.vcd") 
			char = "wheatley"
			postdelay = 0.0 
			predelay = -1.0
			next = "-4009_09"
			noDingOff = true
			noDingOn = true			
		}	
	
		// I think we're in trouble.
		SceneTable["-4009_09" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_scratchingtheitch04.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
		}
		
		// ====================================== First Test Pitch
	
		// I designed THIS test myself. It's a bit difficult. 
		SceneTable["-4010_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/firstwheatleytest01.vcd") 
			char = "wheatley"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4010_02"
			noDingOff = true
			noDingOn = true
			fires=
			[
			{entity="wheatley_zoom_in_relay",input="Trigger",parameter="",delay=0.5, fireatstart=true},
			{entity="wheatley_zoom_out_relay",input="Trigger",parameter="",delay=2.5, fireatstart=true},
			]
		}	
	
		// Notice the moat area... rather large. Deadly. 
		SceneTable["-4010_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/firstwheatleytest02.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
			fires=
			[
			{entity="test1_nag_relay",input="Trigger",parameter="",delay=2.0},
			]	
		}	

		// ====================================== First Test Button
	
		// Ohhhhhhhh, that's some good testing. That is quality testing right there. 
		SceneTable["-4011_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/firstwheatleytestsolve03.vcd") 
			char = "wheatley"
			postdelay = 0.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
			fires=
			[
			{entity="test1_finish_nag_relay",input="Trigger",parameter="",delay=2.0},
			]	
		}	
		
		// ====================================== First Test End
	
		// Ohhhhhhhh, yeah.  That feels good. 
		SceneTable["-4020_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/firstwheatleytestsolve01.vcd") 
			char = "wheatley"
			postdelay = 0.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="test1_end_counter",input="Add",parameter="1",delay=0.0},
			]	
		}	

		// ====================================== First Test Start Nags
		
        // Go on.
         SceneTable["-4021_01"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/bw_solvenags01.vcd"),idlerepeat=true, postdelay=0.1,next=null,char="wheatley",noDingOff=true,noDingOn=true,idle=true,idleminsecs=8.0,idlemaxsecs=12.0,idlegroup="test1hints",idleorderingroup=1}
         SceneTable["-4021_02"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/bw_solvenags02.vcd"),postdelay=0.1,next=null,char="wheatley",noDingOff=true,noDingOn=true,idlegroup="test1hints",idleorderingroup=2}
		 SceneTable["-4021_03"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/bw_solvenags03.vcd"),postdelay=0.1,next=null,char="wheatley",noDingOff=true,noDingOn=true,idlegroup="test1hints",idleorderingroup=3}
         SceneTable["-4021_04"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/bw_solvenags05.vcd"),postdelay=0.1,next=null,char="wheatley",noDingOff=true,noDingOn=true,idlegroup="test1hints",idleorderingroup=4}
		 
		// ====================================== First Test Finish Nags
		 
		 // Hold on.  You've still got to finish the test
		 SceneTable["-4022_01"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/bw_solvenags04.vcd"),idlerepeat=true, postdelay=0.1,next=null,char="wheatley",noDingOff=true,noDingOn=true,idle=true,idleminsecs=8.0,idlemaxsecs=12.0,idlegroup="test1finishhints",idleorderingroup=1}		 
         SceneTable["-4022_02"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/bw_solvenags06.vcd"),postdelay=0.1,next=null,char="wheatley",noDingOff=true,noDingOn=true,idlegroup="test1finishhints",idleorderingroup=2}
		 SceneTable["-4022_03"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/bw_solvenags07.vcd"),postdelay=0.1,next=null,char="wheatley",noDingOff=true,noDingOn=true,idlegroup="test1finishhints",idleorderingroup=3}
         SceneTable["-4022_04"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/bw_solvenags08.vcd"),postdelay=0.1,next=null,char="wheatley",noDingOff=true,noDingOn=true,idlegroup="test1finishhints",idleorderingroup=4}
		 
		// ====================================== First Test Redo
		
		// Here's an idea, since making tests is difficult--why don't you just keep solving THIS test. And I can just... watch you solve it. Yes. That sounds much easier.
		SceneTable["-4012_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/secondwheatleytestintro01.vcd") 
			char = "wheatley"
			postdelay = 0.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true	
		}	

		// ====================================== Second Test Nags
		
        // Here we go.  Now do it again.
         SceneTable["-4019_01"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/bw_resolvenags01.vcd"),idlerepeat=true, postdelay=0.1,next=null,char="wheatley",noDingOff=true,noDingOn=true,idle=true,idleminsecs=8.0,idlemaxsecs=12.0,idlegroup="test2hints",idleorderingroup=1}
         SceneTable["-4019_02"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/bw_resolvenags02.vcd"),postdelay=0.1,next=null,char="wheatley",noDingOff=true,noDingOn=true,idlegroup="test2hints",idleorderingroup=2}
         SceneTable["-4019_03"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/bw_resolvenags03.vcd"),postdelay=0.1,next=null,char="wheatley",noDingOff=true,noDingOn=true,idlegroup="test2hints",idleorderingroup=3}

		
		// ====================================== Second Test Done
		
		// Ahhh.... No. Nothing. Alright, can't blame me for trying. New tests, new tests... 
		SceneTable["-4013_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/secondwheatleytestsolve01.vcd") 
			char = "wheatley"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4013_02"
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="wheatley_newtests_relay",input="Trigger",parameter="",delay=7.0,fireatstart=true},
			]
		}
		
		// I think we’re in luck. Judging by the tests he’s made so far, we shouldn’t have any trouble staying alive long enough to stop him.
		SceneTable["-4013_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_newtests01.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4013_03"
			noDingOff = true
			noDingOn = true
		}
		
		// All we have to do is find out where we are, and I can take us right to him.
		SceneTable["-4013_03" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_newtests03.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4013_04"
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="exit_door2_relay",input="Trigger",parameter="",delay=0.0},
			]	
		}
		
	}

		// ****************************************************************************************************
		// Act 4 Facility Destruction
		// ****************************************************************************************************
	
	if (curMapName=="sp_a4_tb_trust_drop")
	{
	
		// ====================================== Facility Destruction Explanation
		
		// Did you think all I did was test you? Don't be so vain. Every second of every day I attended to the thousand things that keep this place from literally exploding. 
		SceneTable["-4014_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_act4setup01.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4014_02"
			noDingOff = true
			noDingOn = true	
		}
		
		// With that idiot in charge of things, we’ll have a full-scale nuclear meltdown within hours. I’m not being hyperbolic. One of my duties was actively preventing that on an hourly basis. 
		SceneTable["-4014_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_act4setup02.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4014_03"
			noDingOff = true
			noDingOn = true
		}
		
		// Remember when I told you that your friend was specifically designed to make bad decisions? Well, it sounds like he’s about to blow up the entire facility.
		SceneTable["-4014_03" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_act4setup03.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4014_04"
			noDingOff = true
			noDingOn = true
		}
		
		// The one you’re standing in. Which I’d normally throw a party over, except that I’m here TOO.
		SceneTable["-4014_04" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_act4setup04.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
		}
		
		// ====================================== Glados Helps to Escape
		
		// Okay. So the bad news is the tests are my tests now, which means they can kill us. The GOOD news is, I know where we ARE now.  
		SceneTable["-4017_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_newtests06.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4017_02"
			noDingOff = true
			noDingOn = true	
		}
		
		// If we stay on this test track, I can get us out of bounds in ten chambers.  
		SceneTable["-4017_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_newtests10.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 1.0
			next = null
			noDingOff = true
			noDingOn = true	
			fires=
			[
				{entity="@transition_script",input="RunScriptCode",parameter="TransitionReady()",delay=0.0},
			]
			
		}
		
	}
		
		// ====================================== Glados Chell Deal
	
	if (curMapName=="sp_a4_laser_platform")
	{
		
		// You need to put me back in my body. I know this sounds like a trick. In fact, I can unconditionally guarantee you that at any other time it would be a trick. But if you don’t let me stop the damage your friend is doing, this facility is going to explode. Soon.
		SceneTable["-4015_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_act4setup05.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4015_02"
			noDingOff = true
			noDingOn = true
		}			
		
		// Let’s make a deal. If you get me back in my body and help me stop him… I’ll… let… you… go.
		SceneTable["-4015_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_act4setup06.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4015_03"
			noDingOff = true
			noDingOn = true
		}
		
		// I want to make this clear: I’m not promising to stop testing humans. I’m just promising to stop testing on you. So long as you leave and never. Come. Back. 
		SceneTable["-4015_03" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_act4setup08.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
		}
		
	}
		
		// ****************************************************************************************************
		// TBeam Tests
		// ****************************************************************************************************
	
	if (curMapName=="sp_a4_tb_intro")
	{
	
		// ====================================== Wheatley Uses Glados Test Chambers
		
		// It's alright! Everything's good! I invented some more tests! 
		SceneTable["-4016_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/sp_tbeam_intro_intro02.vcd") 
			char = "wheatley"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4016_02"
			noDingOff = true
			noDingOn = true	
		}
		
		// This is one of MY tests!
		SceneTable["-4016_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_newtests05.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4016_03"
			noDingOff = true
			noDingOn = true
		}
		
		// Not entirely, not entirely. Like the word "test", on the wall there. That's new. 
		SceneTable["-4016_03" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/sp_tbeam_intro_intro03.vcd") 
			char = "wheatley"
			postdelay = 0.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
		}
		
		// ====================================== Crusher Intro
		
		// He put in smashing arms. With spikes on them. 
		SceneTable["-4018_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_smashers01.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4018_02"
			noDingOff = true
			noDingOn = true	
		}
		
		// Well. I suppose I should be glad there aren’t velvet paintings of monster trucks on the walls. 
		SceneTable["-4018_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_smashers02.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
		}
		
	}
	
		// ****************************************************************************************************
		// Graduation Chambers
		// ****************************************************************************************************
	
	if (curMapName=="sp_a4_stop_the_box")
	{
	
		// ====================================== Graduation Intro
		
		// THESE tests were designed to be solved by invincible titanium test-bots piloted by a super computer. But we never got around to inventing them. So we tried it with humans. It… didn’t end well.
		SceneTable["-4025_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_miscsupertests01.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true	
		}
		
		// ====================================== Graduation Solved
		
		// You are GOOD. Too good. But I guess you’ll be leaving. You ARE leaving, aren’t you?
		SceneTable["-4026_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_miscsupertests03.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true	
		}
		
	}
		
	
		// ****************************************************************************************************
		// Escape Track
		// ****************************************************************************************************
	
	if (curMapName=="sp_a4_jump_polarity")
	{
	
		// ====================================== Co-op Robot Intro
		
		// There must be more tests around here.  Hold on.  What's this?
		SceneTable["-4030_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/recapture11.vcd") 
			char = "wheatley"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4030_02"
			noDingOff = true
			noDingOn = true	
		}	
		
		// Cooperative Testing Initiative?
		SceneTable["-4030_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/recapture12.vcd") 
			char = "wheatley"
			postdelay = 0.0 
			predelay = 1.0
			next = "-4030_03"
			noDingOff = true
			noDingOn = true	
		}
		
		// But this is...  Oh, clever!
		SceneTable["-4030_03" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/recapture13.vcd") 
			char = "wheatley"
			postdelay = 0.0 
			predelay = 1.0
			next = "-4030_04"
			noDingOff = true
			noDingOn = true	
		}
		
		// Everything's fine.  Continue testing.
		SceneTable["-4030_04" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/recapture14.vcd") 
			char = "wheatley"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4030_05"
			noDingOff = true
			noDingOn = true	
		}
		
		// He’s found the cooperative testing initiative. It’s… something I came up with to phase out human test subjects.
		SceneTable["-4030_05" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_cooprobotintro02.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4030_06"
			noDingOff = true
			noDingOn = true	
		}
		
		// Nothing personal. I just… you know. Hate humans.
		SceneTable["-4030_06" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_cooprobotintro03.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4030_07"
			noDingOff = true
			noDingOn = true	
		}
		
		// Anyway, this is the part where he kills us.
		SceneTable["-4030_07" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_cooprobotintro04.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4030_08"
			noDingOff = true
			noDingOn = true
		}
		
	}
		
	if (curMapName=="sp_a4_finale1")
	{	
		
		// ====================================== Finale Escape
		
		// I don’t get it.
		SceneTable["-4031_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_escapeoutofbounds01.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4031_02"
			noDingOff = true
			noDingOn = true	
		}
		
		// Oh.  Mashed Potatos.  I just got it.
		SceneTable["-4031_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_escapeoutofbounds02.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4031_03"
			noDingOff = true
			noDingOn = true	
		}
		
		// We need to escape now.  Right now.
		SceneTable["-4031_03" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_escapeoutofbounds03.vcd") 
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4031_03"
			noDingOff = true
			noDingOn = true
		}

	}
	
	// ============================================================================
	// Functions for Recapture
	// ============================================================================
	
	function GladosParadoxInception()
	{
		GladosPlayVcd( -4000 )
	}
	
	function RecaptureEavesdrop()
	{
		GladosPlayVcd( -4001 )
	}
	
	function RecaptureObserve()
	{
		GladosPlayVcd( -4003 )
	}
	
	function RecaptureCubeTest()
	{
		GladosPlayVcd( -4004 )
	}
	
	function RecaptureSolveLeadIn()
	{
		GladosPlayVcd( -4005 )
	}
	
	function RecaptureSolve()
	{
		GladosPlayVcd( -4006 )
	}
	
	function RecaptureButton()
	{
		GladosPlayVcd( -4007 )
	}

	function RecaptureProceed()
	{
		GladosPlayVcd( -4008 )
	}
	
	function RecaptureItch()
	{
		GladosPlayVcd( -4009 )
	}
	
	function RecaptureFirstTest()
	{
		GladosPlayVcd( -4010 )
	}
	
	function RecaptureFirstTestDone()
	{
		GladosPlayVcd( -4011 )
	}
	
	function RecaptureFirstTestRedo()
	{
		GladosPlayVcd( -4012 )
	}
	
	function RecaptureTestTwoDone()
	{
		GladosPlayVcd( -4013 )
	}
	
	function FacilityExplanation()
	{
		GladosPlayVcd( -4014 )
	}
	
	function FacilityMakeDeal()
	{
		GladosPlayVcd( -4015 )
	}
	
	function WheatleyTestStolen()
	{
		GladosPlayVcd( -4016 )
	}
	
	function GladosHelpEscape1()
	{
		GladosPlayVcd( -4017 )
	}
	
	function GladosCrushers()
	{
		GladosPlayVcd( -4018 )
	}
	
	function StartWheatleyTest1Nag()
    {
         printl("==Starting Wheatley Nag")
         GladosPlayVcd( -4021 )
    }
	
	function StartWheatleyTest1FinishNag()
    {
         printl("==Starting Wheatley Nag")
         GladosPlayVcd( -4022 )
    }
	
	function StartWheatleyTest2Nag()
    {
         printl("==Starting Wheatley Nag")
         GladosPlayVcd( -4019 )
    }

    function StopWheatleyNag()
    {
        printl("==Stopping Wheatley Nag")
        GladosStopNag()
        GladosCharacterStopScene("wheatley")  // This will make Wheatley stop talking if he is mid sentence.
    }

	function RecaptureFirstTestEnd()
	{
		GladosPlayVcd( -4020 )
	}
	
	function GladosGraduationIntro()
	{
		GladosPlayVcd( -4025 )
	}
	
	function GladosGraduationGood()
	{
		GladosPlayVcd( -4026 )
	}
	
	function CoopBotsFound()
	{
		GladosPlayVcd( -4030 )
	}
	
	function WheatleyKillYouNow()
	{
		GladosPlayVcd( -4031 )
	}
	

