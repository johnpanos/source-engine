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
	SceneTableLookup[-1] <- "-1_01" //Test Scene
									//Wheatly Elevator Scene - Pre Crash
									//Wheatly Elevator Scene - Post Crash
	


// ****************************************************************************************************
// GLADOS BATTLE scenetable
// ****************************************************************************************************

	SceneTableLookup[-50] <- "-20_01" 	// I honestly didn't think you would fall for that
										// In fact, I devised a much more elaborate trap ahead, for when you got through this easy one.
										// If I had known it would be this easy, I would have just dangled a turkey leg
	SceneTableLookup[-53] <- "-21_01"	// But anyway. Here we are.  Again.
										// I hope you brought something more powerful than the portal gun this time
										// It was nice catching up. Lets get to business.
										// You remember my turrets don't you... oh wait, that's you in 5 seconds. goodbye.
	
	SceneTableLookup[-700] <- "-700_01" // turret conversation here
	SceneTableLookup[-750] <- "-750_01" // wheatley neurotoxin ride nags here
	
	
	SceneTableLookup[-57] <- "-22_01"	// My turrets!
										// Oh, you were busy back there
										// //I suppose we could just sit in this room and glare at each other until somebody drops dead, but I have a better idea.
										// Your old friend, deadly neurotoxin.  If i were you i'd take a deep breath and hold it.
										
	SceneTableLookup[-61] <- "-23_01"	// Hello!
										// I hate you so much
										// Warning: Central core is 80 percent corrupted. 
										// That's funny, I don't feel corrupt. In fact, I feel pretty good.
										// Alternate core detected.
										// That's ME they're talking about!
										// To initiate a core transfer, please deposit substitute core in receptacle.
										// Core transfer?
										// Oh, you are kidding me.
										
	SceneTableLookup[-760] <- "-760_01"// Wheatley socket nags

	SceneTableLookup[-71] <- "-24_01" 	//Substitute core accepted.
										//Substitute core, are you ready to start the procedure?
										//Yes.
										//Corrupted core, are you ready to start the procedure?
										//No!
										//Nonononononono!
										//Stalemate detected. Transfer procedure cannot continue.
										//Yes! You little worm!
										//...unless a stalemate associate is present to press the stalemate resolution button.
										//Leave me in! Leave me in! Go press it!
										//Don't do it.
										//Don't press that button. You don't know what you're doing.
	
	SceneTableLookup[-755] <- "-755_01"//Button press nags

	SceneTableLookup[-84] <- "-25_01"	//Not so fast!
										//Think about this.
										//You need to be a trained stalemate associate to press that button. You're unqualified. I know this sounds like an obvious ploy, but I'm really not joking.
										//Impersonating a stalemate associate. I just added that to the list. It's a list I made of all the things you've done. Well, it's a list that I AM making, because you're still doing things right now, even though I'm telling you to stop. Stop, by the way.

										
	SceneTableLookup[-88] <- "-26_01"	// Ahhhh!
										// Stalemate Resolved. Core Transfer Initiated.


	SceneTableLookup[-4] <- "-4_01" 	// Wheatley: Here I go!
										// Wheatley: Wait, what if this hurts?
										// Wheatley: Wait, what if it really hurts? Ohh, I didn't think of that
										// Wheatley: Oh no!  Oh no oh no --
										// Wheatley: Ahhhhh!
	
	SceneTableLookup[-89] <- "-27_00"  // GLaDOS: GET YOUR HANDS OFF ME! NO! NO! STOP!
	
	SceneTableLookup[-90] <- "-27_01"	// CHELL! STOP THIS! I AM YOUR MOTHER!
										// ALRIGHT, FINE! THAT'S NOT TRUE!
										// BUT IT COULD BEEEEEAAHHH!
	
	SceneTableLookup[-93] <- "-28_01"	// Glados GIBBERING
	
	
	
	SceneTableLookup[-9] <- "-5_01"	// Wheatley: Wow! Check ME out, Partner!
										// We did it! I can't believe we did it! I'm in control of the whole facility now!
										// Oh! Right, the escape elevator! I'll call it now.
										// Elevator --called. You get in. I'll tell it to take you to the surface.										
										
	SceneTableLookup[-13] <- "-14_01"	// GLaDOS: Don't do this.. Don't do this...
										// GLaDOS: Oh.  it's you.
										// Wheatley: I knew it was gonna be cool being in charge of everything, but... WOW, is this cool! This body is amazing!
										// Glados: No.
										// Wheatley: And check this out! I'm brilliant now! [spanish] I don't know what I just said, but I can find out.
										// Wheatley: Oh! The elevator. Sorry. [ elevator moves ]
										// Wheatley: Wow, look how small you are! I can barely see you down there! But I'm HUGE. [evil laugh] Where did THAT come from? 
										// Glados: So here's a couple of facts. Absolute power corrupts. Absolutely.
										// Wheatley: Actually... hold on. 
										// Wheatley: Why do we have to leave right this minute? [elevator descends]
										// Glados: And if you combine absolute power with an absolute moron, inside the world's most powerful supercomputer... 
										// Wheatley: You be quiet! I was smart enough to take control of the whole building away from you, wasn't I? 
										// Glados: You didn't do anything. SHE did all the work.
										// Wheatley: Oh really? Well, maybe it's time I DID something, then. 
										// Glados: What are you doing?
										// Glados: No!
	
	
// - potatos
									 
	SceneTableLookup[-33] <- "-10_01"	// I'm on to you too, lady.  .. who's the boss?  Little old Wheatley!
	
	
	SceneTableLookup[-34] <- "-11_01" 	// Ah...!
										// THAT is a potato battery.  Now you live in it.
										// Just... kill me.
	
	SceneTableLookup[-37] <- "-12_01"	 // There! See? I'm huge, you're trapped and she's a potato!  who's a moron now?
										// Still you.
										// How about now? now who's a moron?
										// Still you.  You're still a moron.
										// NOW WHO'S A MORON?
										// If you weren't such a moron, you'd know who.
										// I am not a moron! 
	
	SceneTableLookup[-44] <- "-13_01" 	// How about if I PUNCHED.YOU.INTO.THIS.PIT.  Huh?  Who's a moron now?
										// Uh oh.																														

// ****************************************************************************************************
// SCIENCE FAIR	scenetable								  
// ****************************************************************************************************

SceneTableLookup[-100] <- "-100_01" // I'm not 100% sure we're going the right way.. I give it 65 percent

SceneTableLookup[-101] <- "-101_01" // Bring your daughter to work day.  That did not end well.
									//  And 40 potato batteries.  Embarassing.

SceneTableLookup[-102] <- "-102_01" // Pretty sure we're going the right way. Just to reassure you.

SceneTableLookup[-103] <- "-103_01" // Don't worry i'm absolutely guaranteeing you it is this way.  Ah, It's not this way.

SceneTableLookup[-104] <- "-104_01" // Definitely sure it's this way.  Not entirely sure.  I don't know where it is.


// ****************************************************************************************************
// SP_A2_PIT_FLINGS	companion cube fizzle scenetable					  
// ****************************************************************************************************

SceneTableLookup[-200] <- "-200_01" // Oh. Did I accidentally fizzle that before you could complete the test? I'm sorry.

SceneTableLookup[-201] <- "-201_01"// Go ahead and grab another one so that it won't also fizzle and you won't look stupid again.

SceneTableLookup[-202] <- "-202_01"// Go ahead. I PROMISE not to fizzle it this time.

SceneTableLookup[-203] <- "-203_01"// Oh. No. I fizzled that one too.

SceneTableLookup[-204] <- "-204_01"// Oh well. We have warehouses FULL of the things. Absolutely worthless. I'm happy to get rid of them.

SceneTableLookup[-205] <- "-205_01"// Go ahead. This time I promise you'll look incrementally less stupid than the previous two times in which you looked incredibly stupid.

SceneTableLookup[-206] <- "-206_01"// I think that one was about to say "I love you." They ARE sentient, of course. We just have a LOT of them.


// ****************************************************************************************************
// SP_A2_TRUST_FLING scenetable					  
// ****************************************************************************************************

SceneTableLookup[-300] <- "-300_01"// Oh, sorry. Some of these test chambers haven't been cleaned in ages.

SceneTableLookup[-301] <- "-301_01"// So sometimes there's still trash in them. Standing around. Smelling, and being useless.

SceneTableLookup[-302] <- "-302_01"// Try to avoid the garbage hurtling towards you.

SceneTableLookup[-303] <- "-303_01"// Don't TEST with the garbage. It's garbage.

SceneTableLookup[-304] <- "-304_01"// Press the button again.

// ****************************************************************************************************
// SP_A2_COLUMN_BLOCKER scenetable			  
// ****************************************************************************************************

SceneTableLookup[-400] <- "-400_01"// Initiating surprise in 3... 2... 1.
									// I made it all up
									// Surprise
									// Your parents are probably dead... Wouldn't want to see you.
								

// ****************************************************************************************************
// SP_A2_BRIDGE_THE_GAP scenetable			  
// ****************************************************************************************************

SceneTableLookup[-500] <- "-500_00" //Perfect, the door's malfunctioning. I guess somebody's going to have to repair that too. [beat] No, don't get up. I'll be right back. Don't touch anything.
SceneTableLookup[-501] <- "-500_01" //Hey! Hey! Up here!
									//I found some bird eggs up here. Just dropped 'em into the door mechanism.  Shut it right down. I--AGH!					
									// BIRD BIRD BIRD BIRD
									//[out of breath] Okay. That's probably the bird, isn't it? That laid the eggs! Livid!
									//Anyway, look, the point is we're gonna break out of here, alright? But we can't do it yet. Look for me fifteen chambers ahead.
									//Here she comes! Just play along! RememberFifteenChambers!
								
								
// ****************************************************************************************************
// SP_A2_FIZZLER_INTRO scenetable					  
// ****************************************************************************************************


SceneTableLookup[-950] <- "-50_01"//Nevermind. I have to go... check something. Test on your own recognizance. I'll be back.


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

	
	// ****************************************************************************************************
	// GLaDOS Battle
	// ****************************************************************************************************
	
	if (curMapName=="sp_a2_core") // TODO:  CHANGE THIS TO CORRECT MAP NAME
	{
	// ====================================== Glados trap scene
		
		SceneTable ["-20_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/fgbrvtrap02.vcd") // I honestly didn't think you would fall for that
			char = "glados"
			postdelay = 1.3 
			predelay = 1.0
			next = "-20_02"
			noDingOff = true
			noDingOn = true
		}
		
		SceneTable ["-20_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/fgbrvtrap03.vcd") // In fact, I devised a much more elaborate trap ahead, for when you got through this easy one.
			char = "glados"
			postdelay = 0.8 
			predelay = 0.0
			next = "-20_03"
			noDingOff = true
			noDingOn = true
		}
		
		SceneTable ["-20_03" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/fgbrvtrap05.vcd") // If I had known it would be this easy, I would have just dangled a turkey leg
			char = "glados"
			postdelay = 2.0
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="rv_start_moving_trigger",input="enable",parameter="",delay=0.5}
			]
			
			
		}
		
		// ======================================  Plays when player lands in moving vault
		SceneTable ["-21_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/fgbturrets01.vcd") // Well, it was nice catching up. Lets get to business.
			char = "glados"
			postdelay = 0.8
			predelay = 0.0
			next = "-21_02"
			noDingOff = true
			noDingOn = true
		}
		
		SceneTable ["-21_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/gladosbattle_pre02.vcd") // I hope you brought something more powerful than the portal gun this time
			char = "glados"
			postdelay = 0.0
			predelay = 0.0
			next = "-21_03"
			noDingOff = true
			noDingOn = true		
		}
		
		SceneTable ["-21_03" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/gladosbattle_pre03.vcd") // Otherwise you're about to become the past president of the being alive club, ha ha
			char = "glados"
			postdelay = 0.7
			predelay = 0.0
			next = "-21_04"
			noDingOff = true
			noDingOn = true		
		}
		
		SceneTable ["-21_04" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/fgbturrets02.vcd") // You remember my turrets don't you... oh wait, that's you in 5 seconds. goodbye.
			char = "glados"
			postdelay = -9.0
			predelay = 0.0
			next = "-700_01"
			noDingOff = true
			noDingOn = true
			talkover=true	
			fires=
			[
				{entity="deploy_turrets_relay",input="Trigger",parameter="",delay=4,fireatstart=true},		
			]
		}
		
		// ====================================== Turrets deployed scene

		SceneTable ["-700_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/turret/glados_battle_defect_arrive18.vcd") // [muffled turret talking]
			char = "turret"
			postdelay = 0.0
			predelay = 0.0
			next = "-700_02"
			noDingOff = true
			noDingOn = true
			talkover=true			
			settarget1="turret_02-chamber_npc_turret"
		}
		
		SceneTable ["-700_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/turret/sp_sabotage_factory_defect_dryfire05.vcd") // dry fire
			char = "turret"
			postdelay = -0.5
			predelay = 0.0
			next = "-700_03"
			noDingOff = true
			noDingOn = true
			talkover=true
			settarget1="turret_02-chamber_npc_turret"
			fires=
			[
				{entity="box_turret_push_trigger",input="enable",parameter="",delay=0.0,fireatstart=true}	// turret falls over
			]
		}
		
		SceneTable ["-700_03" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/turret/glados_battle_defect_arrive01.vcd") // it's my big chance!
			char = "turret"
			postdelay = 0.0
			predelay = 0.0
			next = "-700_04"
			noDingOff = true
			noDingOn = true
			talkover=true				
			settarget1="turret_01-chamber_npc_turret"
			fires=
			[
				{entity="turret_01-chamber_npc_turret_guns_out_ss",input="beginsequence",parameter="",delay=0, fireatstart=true}, // pop out guns
				{entity="turret_01-chamber_npc_turret",input="ignite",parameter="",delay=0},
			]
		}
		
		SceneTable ["-700_04" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/turret/sp_sabotage_factory_defect_dryfire07.vcd") // dry fire
			char = "turret"
			postdelay = 0.0
			predelay = 0.0
			next = "-700_05"
			noDingOff = true
			noDingOn = true
			talkover=true
			settarget1="turret_01-chamber_npc_turret"
		}
		
		SceneTable ["-700_05" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/turret/glados_battle_defect_arrive15.vcd") //  This is trouble
			char = "turret"
			postdelay = 0.0
			predelay = 0.0
			next = "-700_06"
			noDingOff = true
			noDingOn = true
			talkover=true
			settarget1="turret_01-chamber_npc_turret"
			fires=
			[
				{entity="turret_01-chamber_npc_turret",input="selfdestructimmediately",parameter="",delay=0.0},		
				{entity="turret_02-chamber_npc_turret",input="ignite",parameter="",delay=0.1}
			]
		}
		
		SceneTable ["-700_06" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/turret/glados_battle_defect_arrive17.vcd") // [muffled turret talking while on fire]
			char = "turret"
			postdelay = 0.0
			predelay = 0.0
			next = "-700_07"
			noDingOff = true
			noDingOn = true
			talkover=true			
			settarget1="turret_02-chamber_npc_turret"
			fires=
			[
				{entity="turret_02-chamber_npc_turret",input="selfdestructimmediately",parameter="",delay=0.0}
			]
		}
		
		SceneTable ["-700_07" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/turret/glados_battle_defect_arrive12.vcd") // ahhh not again!
			char = "turret"
			postdelay = 0.0
			predelay = 0.0
			next = "-700_08"
			noDingOff = true
			noDingOn = true
			talkover=true
			settarget1="turret_03-chamber_npc_turret"
			fires=
			[
				{entity="turret_03-chamber_npc_turret",input="selfdestruct",parameter="",delay=0,fireatstart=true},
				{entity="turret_03-chamber_npc_turret",input="selfdestructimmediately",parameter="",delay=0.5}
			]
		}
		
		SceneTable ["-700_08" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/turret/glados_battle_defect_arrive07.vcd") // Here it comes, pal!
			char = "turret"
			postdelay = 0.5
			predelay = 0.0
			next = "-700_09"
			noDingOff = true
			noDingOn = true
			talkover=true
			settarget1="turret_04-chamber_npc_turret"
			fires=
			[
				{entity="turret_04-chamber_npc_turret_guns_out_ss",input="beginsequence",parameter="",delay=0, fireatstart=true}, // pop out guns
			]
		}
		
		
		SceneTable ["-700_09" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/turret/glados_battle_defect_arrive08.vcd") // Locked and loaded!
			char = "turret"
			postdelay = 0.0
			predelay = 0.0
			next = "-700_10"
			noDingOff = true
			noDingOn = true
			talkover=true
			settarget1="turret_05-chamber_npc_turret"
			fires=
			[
				{entity="turret_05 -chamber_npc_turret_guns_out_ss",input="beginsequence",parameter="",delay=0, fireatstart=true}, // pop out guns
			]
		}
		
		SceneTable ["-700_10" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/turret/sp_sabotage_factory_defect_dryfire01.vcd") // dry fire
			char = "turret"
			postdelay = 0.0
			predelay = 0.0
			next = "-700_11"
			noDingOff = true
			noDingOn = true
			talkover=true
			settarget1="turret_04-chamber_npc_turret"
			fires=
			[
				{entity="turret_04-chamber_npc_turret",input="ignite",parameter="",delay=0,fireatstart=true},
				{entity="turret_04-chamber_npc_turret",input="selfdestructimmediately",parameter="",delay=1.0}
				{entity="turret_05-chamber_npc_turret",input="ignite",parameter="",delay=0},
			]
			
			// explodes, cracks glass
			// sets turret 5 on fire
		}
		
		
		SceneTable ["-700_11" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/turret/glados_battle_defect_arrive16.vcd") // Oh this aint' good
			char = "turret"
			postdelay = 0.0
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true		
			settarget1="turret_05-chamber_npc_turret"
			fires=
			[
				{entity="turret_05-chamber_npc_turret",input="selfdestructimmediately",parameter="",delay=0.0},
				{entity="@glados",input="RunScriptCode",parameter="TurretDeathReactionDialog()",delay=0.3}
			]
		}
		
		/*
		
		// ====================================== Turret Death Reaction
		SceneTable["-22_01"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/glados/fgbturrets04.vcd") // My turrets!
			char="glados"
			postdelay=0.1
			predelay = 0.0
			next="-22_02"
			noDingOff=true
			noDingOn=true
		}
		*/
		
		SceneTable["-22_01"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/glados/gladosbattle_pre06.vcd") // Ohhhhh. You were busy back there.
			char="glados"
			postdelay=0.8
			predelay = 0.0
			next="-22_03"
			noDingOff=true
			noDingOn=true
		}
		
		SceneTable["-22_03"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/glados/gladosbattle_pre17.vcd") //I suppose we could just sit in this room and glare at each other until somebody drops dead, but I have a better idea.
			char="glados"
			postdelay=0.1
			predelay = 0.0
			next="-22_04"
			noDingOff=true
			noDingOn=true
			fires=
			[
				{entity="deploy_neurotoxin_tube_relay",input="Trigger",parameter="",delay=5.00, fireatstart=true}
			]
		}
		
		SceneTable["-22_04"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/glados/gladosbattle_pre07.vcd") // Your old friend, deadly neurotoxin.
			char="glados"
			postdelay=0.3
			predelay = 0.0
			next="-22_05"
			noDingOff=true
			noDingOn=true
		}
		
		SceneTable["-22_05"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/glados/gladosbattle_pre08.vcd") //If I were you, I'd take a deep breath right now and hold it.
			char="glados"
			postdelay=0.1
			predelay = 0.0
			next= null
			noDingOff=true
			noDingOn=true
			fires=
			[
				{entity="deploy_wheatley_relay",input="Trigger",parameter="",delay=0.00},
				{entity="glados_watch_wheatley_deploy_relay",input="Trigger",parameter="",delay=0.4},
				{entity="@glados",input="runscriptcode",parameter="StartWheatleyNeurotoxinRideNag()",delay=0.00}
			]
		}
		
		// ======================================
		// Wheatley neurotoxin ride
		// ======================================
		SceneTable["-750_01"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/stairbouncepain02.vcd"),idlerandom=true, idlerepeat=true, postdelay=0.1,next=null,char="glados",noDingOff=true,noDingOn=true,idle=true, talkover=true, idleminsecs=0.0,idlemaxsecs=0.0,idlegroup="wheatleybouncepain",idleorderingroup=1}
		SceneTable["-750_02"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/stairbouncepain04.vcd"),postdelay=0.1,next=null,char="glados",noDingOff=true,noDingOn=true, talkover=true, idlegroup="wheatleybouncepain",idleorderingroup=2}
		SceneTable["-750_03"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/stairbouncepain06.vcd"),postdelay=0.1,next=null,char="glados",noDingOff=true,noDingOn=true, talkover=true,idlegroup="wheatleybouncepain",idleorderingroup=3}
		SceneTable["-750_04"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/stairbouncepain07.vcd"),postdelay=0.1,next=null,char="glados",noDingOff=true,noDingOn=true,talkover=true,idlegroup="wheatleybouncepain",idleorderingroup=4}
		SceneTable["-750_05"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/stairbouncepain09.vcd"),postdelay=0.1,next=null,char="glados",noDingOff=true,noDingOn=true,talkover=true,idlegroup="wheatleybouncepain",idleorderingroup=5}
		SceneTable["-750_06"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/stairbouncepain10.vcd"),postdelay=0.1,next=null,char="glados",noDingOff=true,noDingOn=true,talkover=true,idlegroup="wheatleybouncepain",idleorderingroup=6}
		SceneTable["-750_07"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/stairbouncepain12.vcd"),postdelay=0.1,next=null,char="glados",noDingOff=true,noDingOn=true, talkover=true,idlegroup="wheatleybouncepain",idleorderingroup=7}
		SceneTable["-750_08"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/stairbouncepain12.vcd"),postdelay=0.1,next=null,char="glados",noDingOff=true,noDingOn=true, talkover=true,idlegroup="wheatleybouncepain",idleorderingroup=8}
		
		
		function StartWheatleyNeurotoxinRideNag()
		{
			printl("==Starting wheatley neurotoxin ride Nag")
			GladosPlayVcd( -750 )
		}
		
		function StopWheatleyNeurotoxinRideNag()
		{
			printl("==Stopping wheatley neurotoxin ride Nag")
			GladosStopNag()
			GladosCharacterStopScene("wheatley")  // this will make wheatley stop talking if he is currently playing a vcd
		}
		
		
		
		// ======================================  Wheatley Lands In Chamber		
		SceneTable["-23_01"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/sphere03/gladosbattle_pre01.vcd") // Hello!
			char="wheatley"
			postdelay=0.1
			predelay = 0.0
			next="-23_02"
			noDingOff=true
			noDingOn=true
			fires=
			[
				{entity="shatter_vault_glass_relay",input="Trigger",parameter="",delay=0.4 },
			]
			
		}
		
		SceneTable["-23_02"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/glados/gladosbattle_pre09.vcd") //I hate you so much.
			char="glados"
			postdelay=0.0
			predelay = 0.8
			next= "-23_03"
			noDingOff=true
			noDingOn=true
		}
		
		//CoreDetected
		SceneTable["-23_03"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/announcer/gladosbattle02.vcd")	 // Warning: Central core is 80 percent corrupted.
			char="announcer"
			postdelay=0.0
			predelay = 0.0
			next= "-23_04"
			noDingOff=true
			noDingOn=true
		}
		
		SceneTable["-23_04"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/glados/gladosbattle_xfer04.vcd") 	// That's funny, I don't feel corrupt. In fact, I feel pretty good.
			char="glados"
			postdelay=0.0
			predelay = 0.0
			next= "-23_05"
			noDingOff=true
			noDingOn=true
		}
		
		SceneTable["-23_05"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/announcer/gladosbattle03.vcd")	 // Alternate core detected.
			char="announcer"
			postdelay=0.0
			predelay = 0.0
			next= "-23_06"
			noDingOff=true
			noDingOn=true
		}
		
		SceneTable["-23_06"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/sphere03/gladosbattle_pre05.vcd") 	// That's ME they're talking about!
			char="wheatley"
			postdelay=0.0
			predelay = 0.0
			next= "-23_07"
			noDingOff=true
			noDingOn=true
		}
		
		SceneTable["-23_07"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/announcer/gladosbattle04.vcd") 	// To initiate a core transfer, please deposit substitute core in receptacle.
			char="announcer"
			postdelay=-5.0
			predelay = 0.0
			next= "-23_08"
			noDingOff=true
			noDingOn=true
			talkover=true
			fires=
			[
				{entity="deploy_core_receptacle_relay",input="Trigger",parameter="",delay=0.3,fireatstart=true},
				{entity="display_socket_instructions_relay",input="Trigger",parameter="",delay=0.3,fireatstart=true}
			]
		}
		
		SceneTable["-23_08"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/glados/gladosbattle_xfer05.vcd") // Core transfer?
			char="glados"
			postdelay=0.0
			predelay = 0.0
			next= "-23_09"
			noDingOff=true
			noDingOn=true
			talkover=true
		}
		
		SceneTable["-23_09"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/glados/gladosbattle_xfer06.vcd") // Oh, you are kidding me.
			char="glados"
			postdelay=0.0
			predelay = 0.0 
			next= null
			noDingOff=true
			noDingOn=true
			talkover=true
			fires=
			[
				{entity="core_transfer_nag_relay",input="trigger",parameter="",delay=0.0}
			]
		}
		
		// ======================================
		// Wheatley socket plugin nag
		// ======================================
		SceneTable["-760_01"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/fgbpluginnags01.vcd"),idlerandomonrepeat=true, idlerepeat=true, postdelay=0.1,next="-760_01a",char="wheatley",noDingOff=true,noDingOn=true,idle=true, talkover=true, idleminsecs=6.0,idlemaxsecs=10.0,idlegroup="socketwheatleynag",idleorderingroup=1, idleindex=1}
		SceneTable["-760_01a"] <- {vcd=CreateSceneEntity("scenes/npc/glados/gladosbattle_xfer12.vcd"),postdelay=0.1,next=null,char="glados",noDingOff=true,noDingOn=true, talkover=true, idlegroup="socketwheatleynag",idleorderingroup=2, idleunder=1}
		
		SceneTable["-760_02"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/fgbpluginnags02.vcd"),postdelay=0.1,next=null,char="wheatley",noDingOff=true,noDingOn=true, talkover=true, idlegroup="socketwheatleynag",idleorderingroup=2}
		SceneTable["-760_03"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/fgbpluginnags03.vcd"),postdelay=0.1,next=null,char="wheatley",noDingOff=true,noDingOn=true, talkover=true,idlegroup="socketwheatleynag",idleorderingroup=3}
		SceneTable["-760_04"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/fgbpluginnags04.vcd"),postdelay=0.1,next=null,char="wheatley",noDingOff=true,noDingOn=true,talkover=true,idlegroup="socketwheatleynag",idleorderingroup=4}
		SceneTable["-760_05"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/fgbpluginnags05.vcd"),postdelay=0.1,next=null,char="wheatley",noDingOff=true,noDingOn=true,talkover=true,idlegroup="socketwheatleynag",idleorderingroup=5}
		SceneTable["-760_06"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/fgbpluginnags06.vcd"),postdelay=0.1,next=null,char="wheatley",noDingOff=true,noDingOn=true,talkover=true,idlegroup="socketwheatleynag",idleorderingroup=6}
		SceneTable["-760_07"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/fgbpluginnags07.vcd"),postdelay=0.1,next=null,char="wheatley",noDingOff=true,noDingOn=true, talkover=true,idlegroup="socketwheatleynag",idleorderingroup=7}
		
		
		function StartWheatleyPluginNag()
		{
			printl("==Starting wheatley Nag")
			GladosPlayVcd( -760 )
		}
		
		function StopWheatleyPluginNag()
		{
			printl("==Stopping wheatley Nag")
			GladosStopNag()
			GladosCharacterStopScene("wheatley")  // this will make wheatley stop talking if he is mid sentence
		}
		
		
		
		// ======================================  Wheatley Plugged in
		SceneTable["-24_01"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/announcer/gladosbattle05.vcd") //Substitute core accepted. 
			char="announcer"
			postdelay=0.1
			predelay = 0.0
			next="-24_02"
			noDingOff=true
			noDingOn=true
			talkover=true
			fires=
			[
				{entity="core_transfer_nag_relay",input="kill",parameter="",delay=0.0, fireatstart=true},
				{entity="@glados",input="RunScriptCode",parameter="StopWheatleyPluginNag()",delay=0.0, fireatstart=true}
			]
		}
		
		SceneTable["-24_02"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/announcer/gladosbattle06.vcd") //Substitute core, are you ready to start the procedure?
			char="announcer"
			postdelay=0.0
			predelay = 0.0
			next= "-24_03"
			noDingOff=true
			noDingOn=true
			talkover=true
		}
		
		SceneTable["-24_03"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/sphere03/gladosbattle_pre06.vcd")	// Yes
			char="wheatley"
			postdelay=0.0
			predelay = 0.0
			next= "-24_04"
			noDingOff=true
			noDingOn=true
		}
		
		SceneTable["-24_04"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/announcer/gladosbattle07.vcd") //Corrupted core, are you ready to start the procedure?
			char="announcer"
			postdelay=0.0
			predelay = 0.0
			next= "-24_05"
			noDingOff=true
			noDingOn=true
		}
		
		SceneTable["-24_05"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/glados/gladosbattle_xfer07.vcd") //No!
			char="glados"
			postdelay=0.4
			predelay = 0.0
			next= "-24_06"
			noDingOff=true
			noDingOn=true
			fires=
			[
				{entity="@glados",input="runscriptcode",parameter="sp_sabotage_glados_specials(1)",delay=0.00}
			]
		}
		
		SceneTable["-24_06"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/glados/gladosbattle_xfer08.vcd") 	// Nonononono!
			char="glados"
			postdelay=0.0
			predelay = 0.0
			next= "-24_07"
			noDingOff=true
			noDingOn=true
			talkover=true
		}
		
		SceneTable["-24_07"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/announcer/gladosbattle08.vcd")	//Stalemate detected. Transfer procedure cannot continue.
			char="announcer"
			postdelay=0.0
			predelay = 0.0
			next= "-24_08"
			noDingOff=true
			noDingOn=true
		}
		
		SceneTable["-24_08"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/glados/gladosbattle_xfer10.vcd") //Yes! You little worm!
			char="glados"
			postdelay=0.0
			predelay = 0.0
			next= "-24_09"
			noDingOff=true
			noDingOn=true
			talkover=true
			fires=
			[
				{entity="@glados",input="runscriptcode",parameter="sp_sabotage_glados_specials(2)",delay=0.3,fireatstart=true}
			]
		}
		
		SceneTable["-24_09"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/announcer/gladosbattle09.vcd") //...unless a stalemate associate is present to press the stalemate resolution button.
			char="announcer"
			postdelay=0.0
			predelay = 0.0 
			next= "-24_10"
			noDingOff=true
			noDingOn=true
			talkover=true
			fires=
			[
				{entity="open_stalemate_room_doors_relay",input="trigger",parameter="",delay=0.0},
				{entity="display_button_press_instructions_relay",input="Trigger",parameter="",delay=0.0}
			]
		}
		
		SceneTable["-24_10"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/sphere03/gladosbattle_pre10.vcd") //Leave me in! Leave me in! Go press it!
			char="wheatley"
			postdelay=0.0
			predelay = 0.4 
			next= "-24_11"
			noDingOff=true
			noDingOn=true
		}
		
		SceneTable["-24_11"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/glados/gladosbattle_xfer12.vcd") //Don't do it.
			char="glados"
			postdelay=0.5
			predelay = 0.0 
			next= "-24_12"
			noDingOff=true
			noDingOn=true
		}
		
		SceneTable["-24_12"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/glados/gladosbattle_xfer11.vcd") //Don't press that button. You don't know what you're doing.
			char="glados"
			postdelay=0.5
			predelay = 0.0 
			next= null
			noDingOff=true
			noDingOn=true
			fires=
			[
				{entity="@glados",input="RunScriptCode",parameter="StartStalemateButtonPressNag()",delay=0.0}
			]		
		}
		
				
		// ======================================
		// Stalemate button press nags
		// ======================================
		SceneTable["-755_01"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/secondwheatleytestidle04.vcd"),idlerandom=true, idlerepeat=true, predelay=[7,12], postdelay=0.1,next=null,char="wheatley",noDingOff=true,noDingOn=true,idle=true, talkover=true, idleminsecs=7.0,idlemaxsecs=12.0,idlegroup="pressthebutton",idleorderingroup=1}
		SceneTable["-755_02"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/secondwheatleytestidle05.vcd"),postdelay=0.1,next=null,char="wheatley",noDingOff=true,noDingOn=true, talkover=true, idlegroup="pressthebutton",idleorderingroup=2}
		SceneTable["-755_03"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/secondwheatleytestidle06.vcd"),postdelay=0.1,next=null,char="wheatley",noDingOff=true,noDingOn=true, talkover=true,idlegroup="pressthebutton",idleorderingroup=3}
		
		
		function StartStalemateButtonPressNag()
		{
			printl("==Starting stalemate button Nag")
			GladosPlayVcd( -755 )
		}
		
		function StopStalemateButtonPressNag()
		{
			printl("==Stopping stalemate button Nag")
			GladosStopNag()
			GladosCharacterStopScene("wheatley")  // this will make wheatley stop talking
			GladosCharacterStopScene("glados")  // this will make glados stop talking
		}
		
		// ======================================  //ButtonDenied
		//Not so fast!
		SceneTable["-25_01"] <- {vcd=CreateSceneEntity("scenes/npc/glados/gladosbattle_xfer14.vcd"),postdelay=0.3,next="-25_02",char="glados",noDingOff=true,noDingOn=true, talkover=true}
		//Think about this.
		SceneTable["-25_02"] <- {vcd=CreateSceneEntity("scenes/npc/glados/gladosbattle_xfer15.vcd"),postdelay=0.2,next="-25_03",char="glados",noDingOff=true,noDingOn=true, talkover=true}
		//You need to be a trained stalemate associate to press that button. You're unqualified. I know this sounds like an obvious ploy, but I'm really not joking.
		SceneTable["-25_03"] <- {vcd=CreateSceneEntity("scenes/npc/glados/gladosbattle_xfer16.vcd"),postdelay=0.2,next="-25_04",char="glados",noDingOff=true,noDingOn=true, talkover=true}
		//Impersonating a stalemate associate. I just added that to the list. It's a list I made of all the things you've done. Well, it's a list that I AM making, because you're still doing things right now, even though I'm telling you to stop. Stop, by the way.
		SceneTable["-25_04"] <- {vcd=CreateSceneEntity("scenes/npc/glados/gladosbattle_xfer17.vcd"),postdelay=0.2,next=null,char="glados",noDingOff=true,noDingOn=true, talkover=true}
		
		// ====================================== //Stalemate button pressed
		SceneTable["-26_01"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/glados/sp_sabotage_glados_dropped01.vcd") // ahhh!
			char="glados"
			postdelay=0.5
			predelay = 0.0 
			next= "-26_02"
			noDingOff=true
			noDingOn=true
			fires=
			[
				{entity="@glados",input="RunScriptCode",parameter="StopStalemateButtonPressNag()",delay=0.0, fireatstart=true}
			]
		}
		
		SceneTable["-26_02"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/announcer/gladosbattle10.vcd") // Stalemate Resolved. Core Transfer Initiated.
			char="announcer"
			postdelay=0.5
			predelay = 0.0 
			next= null
			noDingOff=true
			noDingOn=true
		}

		
		// ====================================== Wheatley transfer scene
		SceneTable ["-4_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/gladosbattle_transfer01.vcd") // Here I go!
			char = "wheatley"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4_02"
			noDingOff = true
			noDingOn = true
		}
		
		SceneTable ["-4_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/gladosbattle_transfer02.vcd") // Wait, what if this hurts?
			char = "wheatley"
			postdelay = 0.0 
			predelay = 0.0
			next = "-4_03"
			noDingOff = true
			noDingOn = true
		}
		
		SceneTable ["-4_03" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/gladosbattle_transfer03.vcd") // What if it really hurts? I didn't think of that.
			char = "wheatley"
			postdelay = 0.0
			predelay = 0.0
			next ="-4_04"
			noDingOff = true
			noDingOn = true		
		}
		
		SceneTable ["-4_04" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/fgbwheatleytransfer03.vcd") // Oh it will.  Believe me, it will.
			char = "glados"
			postdelay = 0.0
			predelay = 0.0
			next = "-4_05"
			noDingOff = true
			noDingOn = true		
		}
		
		SceneTable ["-4_05" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/gladosbattle_transfer04.vcd")  // Oh no! Oh no! Nonono!
			char = "wheatley"
			postdelay = 0.0
			predelay = 0.0
			next = "-4_06"
			noDingOff = true
			noDingOn = true		
		}
		
		SceneTable ["-4_06" ] <-
		{ 
			vcd = CreateSceneEntity("scenes/npc/sphere03/gladosbattle_transfer05.vcd") // aaahhhhhh!!!!!
			char = "wheatley"
			postdelay = 0.0
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="maintenance_pit_script",input="RunScriptCode",parameter="OpenMaintenancePitDoor()",delay=2.0, fireatstart=true },
				{entity="@glados",input="RunScriptCode",parameter="PitHandsGrabGladosHead()",delay=3.0, fireatstart=true}
			]
		}
		
		// ====================================== Glados initial grab by small pit arms
		SceneTable["-27_00"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/glados/fgbgladostransfer15.vcd") // Get your hands off me!
			char="glados"
			postdelay=0.0
			predelay = 1.5 
			next= null // stand alone scene, next is triggered by player look
			noDingOff=true
			noDingOn=true
			fires=
			[
				{entity="pitgrab_player_looking_at_glados_aisc",input="enable",parameter="",delay=0 } 
			]
		}
		
		// ====================================== Glados grabbed by big pit arms

		
		SceneTable ["-27_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/fgbgladostransfer09.vcd") // CHELL! STOP THIS! I AM YOUR MOTHER!
			char = "glados"
			postdelay = 1.0 
			predelay = 0.0
			next = "-27_02"
			noDingOff = true
			noDingOn = true
		}
		
		SceneTable ["-27_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/fgbgladostransfer12.vcd") // ALRIGHT, FINE! THAT'S NOT TRUE!
			char = "glados"
			postdelay = 0.0
			predelay = 0.0
			next ="-27_03"
			noDingOff = true
			noDingOn = true		
		}
		
		SceneTable ["-27_03" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/fgbgladostransfer13.vcd") // BUT IT COULD BEEEEEAAHHH!
			char = "glados"
			postdelay = 0.0
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="maintenance_pit_script",input="RunScriptCode",parameter="EjectGladosHead()",delay=0.0 },		// eject glados head
				{entity="begin_wheatley_emergence_relay",input="trigger",parameter="",delay=2.0}, 						// wheatley appears
				{entity="@glados",input="RunScriptCode",parameter="CoreTransferCompleted()",delay=2.5 },				 // start wheatley dialog
				{entity="maintenance_pit_script",input="RunScriptCode",parameter="RevealWheatley()",delay=2.0 },		// reveal wheatley
				
				// ***TODO***  THIS IS DISABLED UNTIL THE DUCKING CODE IS FIXED!
				// **********	WHEATLEY IS BEING DUCKED TO GLADOS VOLUME LEVEL.  SHIFT HAPPENS.
				//{entity="@glados",input="RunScriptCode",parameter="StartGladosGibberishNag()",delay=4.0 }				 // start gibberish nag				
			]		
		}
		
		// ======================================
		// Glados gibberish nag
		// ======================================
		SceneTable["-28_01"] <- {vcd=CreateSceneEntity("scenes/npc/glados/sp_sabotage_glados_gibberish01.vcd"),idlerandom=true, idlerepeat=true, postdelay=0.1,next=null,char="glados",noDingOff=true,noDingOn=true,idle=true, talkover=true, idleminsecs=0.0,idlemaxsecs=0.0,idlegroup="gibberish",idleorderingroup=1}
		SceneTable["-28_02"] <- {vcd=CreateSceneEntity("scenes/npc/glados/sp_sabotage_glados_gibberish02.vcd"),postdelay=0.1,next=null,char="glados",noDingOff=true,noDingOn=true, talkover=true, idlegroup="gibberish",idleorderingroup=2}
		SceneTable["-28_03"] <- {vcd=CreateSceneEntity("scenes/npc/glados/sp_sabotage_glados_gibberish03.vcd"),postdelay=0.1,next=null,char="glados",noDingOff=true,noDingOn=true, talkover=true,idlegroup="gibberish",idleorderingroup=3}
		SceneTable["-28_04"] <- {vcd=CreateSceneEntity("scenes/npc/glados/sp_sabotage_glados_gibberish04.vcd"),postdelay=0.1,next=null,char="glados",noDingOff=true,noDingOn=true,talkover=true,idlegroup="gibberish",idleorderingroup=4}
		SceneTable["-28_05"] <- {vcd=CreateSceneEntity("scenes/npc/glados/sp_sabotage_glados_gibberish05.vcd"),postdelay=0.1,next=null,char="glados",noDingOff=true,noDingOn=true,talkover=true,idlegroup="gibberish",idleorderingroup=5}
		SceneTable["-28_06"] <- {vcd=CreateSceneEntity("scenes/npc/glados/sp_sabotage_glados_gibberish06.vcd"),postdelay=0.1,next=null,char="glados",noDingOff=true,noDingOn=true,talkover=true,idlegroup="gibberish",idleorderingroup=6}
		SceneTable["-28_07"] <- {vcd=CreateSceneEntity("scenes/npc/glados/sp_sabotage_glados_gibberish07.vcd"),postdelay=0.1,next=null,char="glados",noDingOff=true,noDingOn=true, talkover=true,idlegroup="gibberish",idleorderingroup=7}
		
		
		function StartGladosGibberishNag()
		{
			printl("==Starting Glados Nag")
			GladosPlayVcd( -93 )
		}
		
		function StopGladosGibberishNag()
		{
			printl("==Stopping Glados Nag")
			GladosStopNag()
			GladosCharacterStopScene("glados")  // this will make glados stop talking if she is mid sentence
		}
		
		
		
		// ====================================== Wheatley swap completed scene
		
		SceneTable ["-5_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/heelturn13.vcd") // Wow! Check ME out, Partner! 
			char = "wheatley"
			postdelay = 0.0 
			predelay = 0.0
			next = "-5_02"
			noDingOff = true
			noDingOn = true
		}
		
		SceneTable ["-5_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/heelturn01.vcd") // We did it! I can't believe we did it! I'm in control of the whole facility now!     
			char = "wheatley"
			postdelay = 0.8 
			predelay = 0.0
			next = "-5_03"
			noDingOff = true
			noDingOn = true
		}
		
		SceneTable ["-5_03" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/heelturn02.vcd") // Oh! Right, the escape elevator! I'll call it now.
			char = "wheatley"
			postdelay = 1.0
			predelay = 0.0
			next = "-5_04"
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="deploy_exit_elevator_relay",input="Trigger",parameter="",delay=2,fireatstart=true},
			] 
		}
		
		SceneTable ["-5_04" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/heelturn03.vcd") // Elevator -- called. You get in. I'll tell it to take you to the surface.
			char = "wheatley"
			postdelay = 0.0
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
				// start wheatley elevator nag here
		}
		
		// ====================================== Plays after entering elevator and door closes, trapping player
		SceneTable ["-14_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/fgbdonotdothis01.vcd") // [gibberish] Don't do this..
			char = "glados"
			postdelay = 0.0
			predelay = 0.0
			next = "-14_02"
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="@glados",input="RunScriptCode",parameter="StopGladosGibberishNag()",delay=0.0, fireatstart=true}
			]
			
			
			// stop wheatley elevator nag here
		}
		
		SceneTable ["-14_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/chellgladoswakeup01.vcd") // Oh.  it's you.
			char = "glados"
			postdelay = 0.0
			predelay = 0.0
			next = "-14_03"
			noDingOff = true
			noDingOn = true
		}
		
		SceneTable ["-14_03" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/heelturn06.vcd") // Wheatley: I knew it was gonna be cool being in charge of everything, but... WOW, is this cool! This body is amazing!
			char = "wheatley"
			postdelay = 1.0
			predelay = 0.0
			next = "-14_04"
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="start_juggling_relay",input="Trigger",parameter="",delay=3, fireatstart=true }
			]
		}
		
		SceneTable ["-14_04" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/fgbDoNotDoThis03.vcd") // Glados: [gibberish] No!
			char = "glados"
			postdelay = 0.0
			predelay = 0.0
			next = "-14_05"
			noDingOff = true
			noDingOn = true
	
		}
		
		SceneTable ["-14_05" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/heelturn07.vcd") // Wheatley: And check this out! I'm brilliant now! [spanish] I don't know what I just said, but I can find out. 
			char = "wheatley"
			postdelay = 0.8
			predelay = 0.0
			next = "-14_06"
			noDingOff = true
			noDingOn = true
	
		}
		
		SceneTable ["-14_06" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/heelturn08.vcd") // Oh! The elevator. Sorry. [ elevator moves ]
			char = "wheatley"
			postdelay = 0.0
			predelay = 0.0
			next = "-14_07"
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="move_exit_elevator_to_escape_vista_relay",input="Trigger",parameter="",delay=0.5}
			]
		}
		
		SceneTable ["-14_07" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/gladosbattle_core01.vcd") // Glados: You're not going anywhere.
			char = "glados"
			postdelay = 0.0
			predelay = 0.0
			next = "-14_08"
			noDingOff = true
			noDingOn = true
	
		}
		
		SceneTable ["-14_08" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/heelturn09.vcd") // Wheatley: Wow, look how small you are! I can barely see you down there! But I'm HUGE. [evil laugh] Where did THAT come from? 
			char = "wheatley"
			postdelay = 0.0
			predelay = 0.0
			next = "-14_09"
			noDingOff = true
			noDingOn = true
	
		}
		
		SceneTable ["-14_09" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/fgbheelturn01.vcd") // Glados: So here's a couple of facts. Absolute power corrupts. Absolutely.
			char = "glados"
			postdelay = 0.0
			predelay = 0.0
			next = "-14_10"
			noDingOff = true
			noDingOn = true
	
		}
		
		SceneTable ["-14_10" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/gladosbattle_pre15.vcd") // Wheatley: Actually... hold on. 
			char = "wheatley"
			postdelay = 0.0
			predelay = 0.0
			next = "-14_11"
			noDingOff = true
			noDingOn = true
	
		}
		
		SceneTable ["-14_11" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/gladosbattle_pre16.vcd") // Wheatley: Why do we have to leave right this minute? [elevator descends]
			char = "wheatley"
			postdelay = 0.0
			predelay = 0.0
			next = "-14_12"
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="move_exit_elevator_to_smash_start",input="Trigger",parameter="",delay=0, fireatstart=true}
			]
		}
		
		SceneTable ["-14_12" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/fgbheelturn02.vcd") // Glados: And if you combine absolute power with an absolute moron, inside the world's most powerful supercomputer... 
			char = "glados"
			postdelay = 0.0
			predelay = 0.0
			next = "-14_13"
			noDingOff = true
			noDingOn = true
	
		}
		
		SceneTable ["-14_13" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/heelturn11.vcd") // Wheatley: You be quiet! I was smart enough to take control of the whole building away from you, wasn't I? 
			char = "wheatley"
			postdelay = 0.0
			predelay = 0.0
			next = "-14_14"
			noDingOff = true
			noDingOn = true
	
		}
		
		SceneTable ["-14_14" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/sp_sabotage_glados_berate01.vcd") // Glados: You didn't do anything. SHE did all the work.
			char = "glados"
			postdelay = 0.0
			predelay = 0.0
			next = "-14_15"
			noDingOff = true
			noDingOn = true
	
		}
		
		// ====================================== Wheatley grabs GLaDOS to make her PotatOS.
		SceneTable ["-14_15" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/heelturn12.vcd") // Wheatley: Oh really? Well, maybe it's time I DID something, then.
			char = "wheatley"
			postdelay = 0.0
			predelay = 0.0
			next = "-14_16"
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="maintenance_pit_script",input="RunScriptCode",parameter="MakePotatos()",delay=3, fireatstart=true}
			]
	
		}
		
		SceneTable ["-14_16" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/sp_sabotage_glados_confused04.vcd") // Glados: What are you doing?
			char = "glados"
			postdelay = 0.5
			predelay = 0.0
			next = "-14_17"
			noDingOff = true
			noDingOn = true
	
		}
		
		SceneTable ["-14_17" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/sp_sabotage_glados_potatos_initial01.vcd") // Glados: No!
			char = "glados"
			postdelay = 2
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="@glados",input="RunScriptCode",parameter="DialogDuringPotatosManufacture()",delay=0.5 },
			]
	
		}
		
		
		// ====================================== Wheatley talks to player while PotatOS is being made
		
		SceneTable ["-10_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/gladosbattle_elevator04.vcd") //   I'm on to you too lady.. Now who's the boss?
			char = "wheatley"
			postdelay = 1.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
			
			fires=
			[
				{entity="@glados",input="RunScriptCode",parameter="PotatosPresentation()",delay=2 },
				{entity="maintenance_pit_script",input="RunScriptCode",parameter="PresentPotatos()",delay=0}
			]
		}
		
		// ====================================== Wheatley displays potato battery for first time
		
		SceneTable ["-11_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/gladosbattle_elevator05.vcd") //   Ah!
			char = "wheatley"
			postdelay = 6.0 
			predelay = 0.0
			next = "-11_02"
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="maintenance_pit_script",input="RunScriptCode",parameter="DeliverPotatos()",delay=1 },
			]
		}
		
		SceneTable ["-11_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/gladosbattle_xfer14.vcd") // THAT is a potato battery.  Now you live in it.
			char = "wheatley"
			postdelay = 0.5
			predelay = 0.0
			next = "-11_03"
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="@glados",input="RunScriptCode",parameter="PotatosPresentation()",delay=0, fireatstart=true },
				{entity="maintenance_pit_script",input="RunScriptCode",parameter="WigglePotatosMultiple()",delay=0.0, fireatstart=true }
				
			]
		}
		
		SceneTable ["-11_03" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/sp_sabotage_glados_potatos_initial06.vcd") // just...kill me.
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
			
			fires=
			[
				{entity="begin_potatos_moron_scene_relay",input="Trigger",parameter="",delay=0.5 },
			]
		}
		
		 
		
		// ====================================== Elevator moron scene
		
		SceneTable ["-12_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/gladosbattle_elevatorend01.vcd") // There! See? ..she's a potato! Who's a moron now?
			char = "wheatley"
			postdelay = 0.0 
			predelay = 0.0
			next = "-12_02"
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="maintenance_pit_script",input="RunScriptCode",parameter="TapGladosGentlyOnGlass()",delay=0.0, fireatstart=true }
			]
		}
		
		
		SceneTable ["-12_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_fgbprefallelevator01.vcd") // You.
			char = "glados"
			postdelay = 0.8
			predelay = 0.0
			next = "-12_03"
			noDingOff = true
			noDingOn = true
		}
		
		SceneTable ["-12_03" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/gladosbattle_elevatorend03.vcd") // How about NOW? NOW who's a moron?
			char = "wheatley"
			postdelay = 0.0
			predelay = 0.0
			next ="-12_04"
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="maintenance_pit_script",input="RunScriptCode",parameter="TapGladosStronglyOnGlass()",delay=0.0, fireatstart=true }
			]
		}
		
		SceneTable ["-12_04" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/sp_sabotage_glados_potatos_stillyou01.vcd") // Still you.
			char = "glados"
			postdelay = 0.0
			predelay = 0.0
			next = "-12_05"
			noDingOff = true
			noDingOn = true	
		}
		
		SceneTable ["-12_05" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_fgbprefallelevator03.vcd") // You're still a moron
			char = "glados"
			postdelay = 0.8
			predelay = 0.0
			next = "-12_06"
			noDingOff = true
			noDingOn = true	
		}
		
		SceneTable ["-12_06" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/gladosbattle_elevatorend04.vcd") // NOW WHO'S A MORON?
			char = "wheatley"
			postdelay = 0.0
			predelay = 0.0
			next = "-12_07"
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="maintenance_pit_script",input="RunScriptCode",parameter="PunchGladosThroughGlass()",delay=0.0, fireatstart=true }
			]
		}
		
		SceneTable ["-12_07" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/potatos_fgbprefallelevator04.vcd") // I hate to tell you this... 
			char = "glados"
			postdelay = 0.0
			predelay = 0.0
			next = "-12_08"
			noDingOff = true
			noDingOn = true			
		}
		
		SceneTable ["-12_08" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/gladosbattle_elevatorend07.vcd") // I am not a moron!
			char = "wheatley"
			postdelay = 0.0
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="begin_elevator_conclusion_relay",input="Trigger",parameter="",delay=0.0 },
			] 
		}
		
		
		// ====================================== Elevator conclusion
		SceneTable ["-13_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/gladosbattle_elevatorend05.vcd") // How about if I punched you into this pit? Huh? Who's a moron now?
			char = "wheatley"
			postdelay = 0.0
			predelay = 0.0
			next = "-13_02"
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="maintenance_pit_script",input="RunScriptCode",parameter="PoundElevatorOne()",delay=0.5, fireatstart=true },
				{entity="smash_elevator_relay",input="Trigger",parameter="",delay=1.7, fireatstart=true },
				
				{entity="maintenance_pit_script",input="RunScriptCode",parameter="PoundElevatorTwo()",delay=1.6, fireatstart=true },
				{entity="smash_elevator_relay",input="Trigger",parameter="",delay=2.6, fireatstart=true },
			
				{entity="maintenance_pit_script",input="RunScriptCode",parameter="PoundElevatorThree()",delay=2.4, fireatstart=true },
				{entity="smash_elevator_relay",input="Trigger",parameter="",delay=3.3, fireatstart=true },
			
				{entity="maintenance_pit_script",input="RunScriptCode",parameter="PoundElevatorFour()",delay=3.0, fireatstart=true },
				{entity="smash_elevator_relay",input="Trigger",parameter="",delay=3.9, fireatstart=true },
			
				{entity="maintenance_pit_script",input="RunScriptCode",parameter="PoundElevatorFive()",delay=3.6, fireatstart=true }
				{entity="smash_elevator_relay",input="Trigger",parameter="",delay=4.4, fireatstart=true }
			
			]
		}
		
		SceneTable ["-13_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/sp_sabotage_glados_postxfer09.vcd") // uh oh.
			char = "wheatley"
			postdelay = 0.0
			predelay = 1.5
			next = null
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="begin_elevator_collapse_relay",input="Trigger",parameter="",delay=0.0 },
				{entity="exit_elevator_groan_sound",input="playsound",parameter="",delay=0.0, fireatstart=true },
			] 
		}
	 }
	 
// ****************************************************************************************************
// SCIENCE FAIR
// ****************************************************************************************************

if (curMapName=="sp_a2_bts4" || "sabotage_offices" )
	{
		SceneTable ["-100_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/sphere_flashlight_tour43.vcd") // I'm not 100% sure we're going the right way..65 percent
			char = "wheatley"
			postdelay = 0.0
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
		}
		
		SceneTable ["-101_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/sphere_flashlight_tour41.vcd") // Bring your daughter to work day.  That did not end well.
			char = "wheatley"
			postdelay = 1.0
			predelay = 0.0
			next = "-101_02"
			noDingOff = true
			noDingOn = true
		}
		
		SceneTable ["-101_02" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/sphere_flashlight_tour42.vcd") // And 40 potato batteries.  Embarassing.
			char = "wheatley"
			postdelay = 0.0
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="lookat_potato_exhibits_relay",input="trigger",parameter="",delay=0.2, fireatstart=true },
				{entity="wheatley_exhibit_move_triggers",input="enable",parameter="",delay=4.0, fireatstart=true },
				{entity="lookat_player_rl",input="trigger",parameter="",delay=3, fireatstart=true }, 
			] 
		}
		
		
		SceneTable ["-102_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/sphere_flashlight_tour44.vcd") // Pretty sure we're going the right way. Just to reassure you.
			char = "wheatley"
			postdelay = 0.0
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
		}
		
		SceneTable ["-103_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/sphere_flashlight_tour50.vcd") //  Don't worry i'm absolutely guaranteeing you it is this way.
			char = "wheatley"
			postdelay = 0.0
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="lookat_deadend_relay",input="trigger",parameter="",delay=2.0, fireatstart=true },			// aim light down deadend
				{entity="move_wheatley_to_deadend_relay",input="trigger",parameter="",delay=3.0, fireatstart=true },	// move towards deadend
				{entity="lookat_deadend_entry_relay",input="trigger",parameter="",delay=3.7, fireatstart=true },		// aim light back
				{entity="move_wheatley_to_exit_relay",input="trigger",parameter="",delay=4.0, fireatstart=true },		// move out of deadend
			]
			
		}
		
		SceneTable ["-104_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/sphere03/sphere_flashlight_tour51.vcd") //  Definitely sure it's this way.  I dont know where it is.
			char = "wheatley"
			postdelay = 0.0
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
		}
	}
	
// ============================================================================
// Called at end of factory where wheatley isn't quite sure of the way
// ============================================================================
function sp_a2_bts4_end_dialog()
{
	GladosPlayVcd( -104 )
}



// ****************************************************************************************************
// ****************************************************************************************************

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

// ****************************************************************************************************
// SP_A2_PIT_FLINGS	companion cube fizzle
// ****************************************************************************************************
if (curMapName=="sp_a2_pit_flings")
	{
		// triggered when player picks up cube for the first time
		SceneTable ["-200_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/fizzlecube01.vcd") // Oh. Did I accidentally fizzle that before you could complete the test? I'm sorry.
			char = "glados"
			postdelay = 1.0 
			predelay = 0.0
			next = "-201_01"
			noDingOff = true
			noDingOn = true
		}
		
		SceneTable ["-201_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/fizzlecube03.vcd") // Go ahead and grab another one so that it won't also fizzle and you won't look stupid again.
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="drop_new_box_relay",input="trigger",parameter="",delay=1.0, fireatstart=true },
			]
		}
		

		
		// THIS IS NOT USED
		// triggered when second box hits the ground
		SceneTable ["-202_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/fizzlecube07.vcd") // Go ahead. I PROMISE not to fizzle it this time.
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
		}
		
		
		// triggered when player picks up the second box
		SceneTable ["-203_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/fizzlecube05.vcd") // Oh. No. I fizzled that one too.
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = "-204_01"
			noDingOff = true
			noDingOn = true
			queue = true
			fires=
			[
				{entity="drop_new_box_relay",input="trigger",parameter="",delay=0.0 },
				{entity="companion_cube_trigger",input="kill",parameter="",delay=0.0, fireatstart=true },
			]
		}
		
		SceneTable ["-204_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/fizzlecube06.vcd") // Oh well. We have warehouses FULL of the things. Absolutely worthless. I'm happy to get rid of them.
			char = "glados"
			postdelay = 0.0 
			predelay = 1.0
			next = null
			noDingOff = true
			noDingOn = true
		}
		
		// THIS LINE IS NOT USED
		SceneTable ["-205_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/fizzlecube09.vcd") // Go ahead. This time I promise you'll look incrementally less stupid than the previous two times in which you looked incredibly stupid.
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true

		}
		
		SceneTable ["-206_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/fizzlecube10.vcd") // I think that one was about to say "I love you." They ARE sentient, of course. We just have a LOT of them.
			char = "glados"
			postdelay = 0.0 
			predelay = 2
			next = null
			noDingOff = true
			noDingOn = true
		}
	}

// ============================================================================
// Drop a companion cube out of a dropper in sp_a2_pit_flings
// if the player gets into an unwinnable situation
// NOTE: This is called EVERY TIME the cube is lost.
//		 The very first time a cube is dropped it will be a companion cube
// ============================================================================

if (curMapName=="sp_a2_pit_flings")
{
	boxDissolveCount <- 1
}

function sp_laser_lift_pit_flings_cube_lost()
{
	
		// play different dialog for the various boxes that drop
		switch ( boxDissolveCount )
		{
			case 1:
				GladosPlayVcd( -200 )
				break
			case 2:
				GladosPlayVcd( -203 )
				break
			default:
				printl("====== Cube lost! spawning a new one...")
				EntFire( "drop_new_box_relay", "trigger", 0, 0, 0 )
				break
		}
		boxDissolveCount+=1
}


// ============================================================================
// Called when sp_laser_lift_pit_flings companion cube is picked up
// ============================================================================
function sp_laser_lift_pit_flings_companion_cube_dissolved()
{
		printl("***DISSOLVING cube_dropper_box!")
		
		// spawn a new box whenever ready
		sp_laser_lift_pit_flings_cube_lost();
}


// ============================================================================
// Called if player tries to smuggle cube out of level
// ============================================================================
function sp_a2_pit_flings()
{
	GladosPlayVcd( -206 )
}



// ****************************************************************************************************
// SP_A2_TRUST_FLING
// ****************************************************************************************************
 if (curMapName=="sp_a2_trust_fling")
	{
		SceneTable ["-300_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/faithplategarbage01.vcd") // Oh, sorry. Some of these test chambers haven't been cleaned in ages.
			char = "glados"
			postdelay = 0.0 
			predelay = 1.0
			next = "-301_01"
			noDingOff = true
			noDingOn = true
		}
		
		SceneTable ["-301_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/faithplategarbage02.vcd") // So sometimes there's still trash in them. Standing around. Smelling, and being useless.
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = "-302_01"
			noDingOff = true
			noDingOn = true
		}
		

		// triggered when second box hits the ground
		SceneTable ["-302_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/faithplategarbage04.vcd") // Try to avoid the garbage hurtling towards you.
			char = "glados"
			postdelay = 0.0 
			predelay = 0.5
			next = null
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="garbage_pickup_relay",input="enable",parameter="",delay=0.5 },
			]
		}
		
		
		// triggered when player picks up the second box
		SceneTable ["-303_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/faithplategarbage05.vcd") // Don't TEST with the garbage. It's garbage.
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="button_press_dialog_relay",input="trigger",parameter="",delay=0.5 },
			]
		}
		
		SceneTable ["-304_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/faithplategarbage06.vcd") // Press the button again.
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
		}
	}


// ============================================================================
// Called when trust fling dropper button is first pressed
// ============================================================================
function sp_a2_trust_fling_garbage_spawn()
{
	GladosPlayVcd( -300 )
}

// ============================================================================
// Called when trust fling garbage is picked up
// ============================================================================
function sp_a2_trust_fling_garbage_pickup()
{
	GladosPlayVcd( -303 )
}


// ============================================================================
// This VCD is chained from the previous one through a map relay and will only
// play if the button has not already been pressed
// ============================================================================
function sp_a2_trust_fling_button_press_reminder()
{
	GladosPlayVcd( -304 )
}


// ****************************************************************************************************
// SP_A2_COLUMN_BLOCKER	(birthday surprise scene)		  
// ****************************************************************************************************
if (curMapName=="sp_a2_column_blocker")
{
		SceneTable ["-400_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/testchambermisc34.vcd") // initiating surprise in 3...2...1.
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = "-401_01"
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="surprise_room_lights_on",input="Trigger",parameter="",delay=0.2},
			]
		}
		
		SceneTable ["-401_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/testchambermisc35.vcd") // i made it all up
			char = "glados"
			postdelay = 2.5 
			predelay = 2.0
			next = "-402_01"
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="surprise_room_effects_relay",input="Trigger",parameter="",delay=0.6},
				{entity="surprise_room_party_horn_sound",input="playsound",parameter="",delay=0.3}
			]
		}
		
		SceneTable ["-402_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/testchambermisc41.vcd") // surprise.
			char = "glados"
			postdelay = 2.0 
			predelay = 0.0
			next = "-403_01"
			noDingOff = true
			noDingOn = true
		}
		
		SceneTable ["-403_01" ] <-
		{
			vcd = CreateSceneEntity("scenes/npc/glados/testchambermisc37.vcd") // your parents are probably dead. ...i doubt they'd want to see you.
			char = "glados"
			postdelay = 0.0 
			predelay = 0.0
			next = null
			noDingOff = true
			noDingOn = true
			fires=
			[
				{entity="surprise_room_door",input="open",parameter="",delay=1.5, fireatstart=true},
			]
		}
}


// ****************************************************************************************************
// SP_A2_BRIDGE_THE_GAP - BIRD!
// ****************************************************************************************************

if (curMapName=="sp_a2_bridge_the_gap")
{
		//Perfect, the door's malfunctioning. I guess somebody's going to have to repair that too. [beat] No, don't get up. I'll be right back. Don't touch anything.
		SceneTable["-500_00"] <- 
		{
			vcd=CreateSceneEntity("scenes/npc/glados/sp_trust_fling_sphereinterrupt01.vcd")
			postdelay=0.000
			next=null
			char="glados"
			fires=
			[
				{entity="start_wheatley_window_scene_relay",input="trigger",parameter="",delay=3.0}
			]
			predelay=0.2
			queue=1 
		}

	//Hey! Hey! Up here!
	SceneTable["-500_01"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/sp_trust_fling01.vcd"),postdelay=0.2,next="-500_02",char="wheatley",noDingOff=true,noDingOn=true}

	//I found some bird eggs up here. Just dropped 'em into the door mechanism.  Shut it right down. I--AGH!
	SceneTable["-500_02"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/sp_trust_flingAlt07.vcd")
		postdelay=0.1
		next="-500_03"
		char="wheatley"
		noDingOff=true
		noDingOn=true
		fires=
		[
			{entity="bird_attack_start_relay",input="Trigger",parameter="",delay=3.6, fireatstart=true},
		]
	}

	//BIRD BIRD BIRD BIRD
	SceneTable["-500_03"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/sp_trust_flingAlt02.vcd"),postdelay=2.5,next="-500_04",char="wheatley",noDingOff=true,noDingOn=true}

	//[out of breath] Okay. That's probably the bird, isn't it? That laid the eggs! Livid!
	SceneTable["-500_04"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/sp_trust_flingAlt08.vcd")
		postdelay=0.3
		next="-500_05"
		char="wheatley"
		noDingOff=true
		noDingOn=true
		fires=
		[
			{entity="bird_attack_end_relay",input="Trigger",parameter="",delay=0.0, fireatstart=true} 
		]
	}

	//Anyway, look, the point is we're gonna break out of here, alright? But we can't do it yet. Look for me fifteen chambers ahead.
	SceneTable["-500_05"] <- {vcd=CreateSceneEntity("scenes/npc/sphere03/sp_trust_fling03.vcd"),postdelay=0.1,next="-500_06",char="wheatley",noDingOff=true,noDingOn=true}

	//Here she comes! Just play along! RememberFifteenChambers!
	SceneTable["-500_06"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/sp_trust_fling04.vcd")
		postdelay=1
		next=null
		char="wheatley"
		fires=
		[
			{entity="trick_door_open_relay",input="Trigger",parameter="",delay=3.3,fireatstart=true},
			{entity="wheatley_depart_scene_relay",input="Trigger",parameter="",delay=1,fireatstart=true}
		]
		noDingOff=true
		noDingOn=true
	}
}

// ============================================================================
// Called when door malfunction begins
// ============================================================================
function sp_a2_bridge_the_gap_brokendoor_scene()
{
	GladosPlayVcd( -500 )
}

// ============================================================================
// Starts the wheatley window / bird scene
// ============================================================================
function sp_a2_bridge_the_gap_window_scene()
{
	printl("=====================***************************************CALLING WINDOW SCENE")
	GladosPlayVcd( -501 )
}



if (curMapName=="sp_a2_fizzler_intro")
{
	//Nevermind. I have to go... check something. Test on your own recognizance. I'll be back.
	SceneTable["-50_01"] <- 
	{
		vcd = CreateSceneEntity("scenes/npc/glados/sp_trust_fling_sphereinterrupt03.vcd")
		char = "glados"
		predelay = 0.0
		next = null
		noDingOff = true
		noDingOn = true
	}
}

// ============================================================================
// Called after the distant explosion
// ============================================================================
function sp_a2_fizzler_training_distant_explosion()
{
	GladosPlayVcd( -950 )
}

// ****************************************************************************************************
// SP_A1_INTRO1 scenetable			  
// ****************************************************************************************************

SceneTableLookup[-600] <- "-600_01" // start of the open the door string of dialog

SceneTableLookup[-601] <- "-601_01" // Nags if player waited through the entire opening sequence

SceneTableLookup[-602] <- "-602_01" // First Sequence when door is opened

SceneTableLookup[-603] <- "-603_01" // Nag telling the player to say apple

SceneTableLookup[-604] <- "-604_01" // Jumping is close enough to saying apple

SceneTableLookup[-605] <- "-605_01" // First Impact with the wall above dock

SceneTableLookup[-606] <- "-606_01" // When the container first starts it's ride

SceneTableLookup[-607] <- "-607_01" // When wheatley re-enters and lets the player know that they have brain damage

// ****************************************************************************************************
// SP_A1_INTRO1 (container ride scene)		  
// ****************************************************************************************************
if (curMapName=="sp_a1_intro1")
{
	//Hello?
	SceneTable["-600_01"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningHello01.vcd"),
		char="wheatley"
		postdelay=0.0,
		predelay = 0.0
		next = "-600_02"
		noDingOff = true
		noDingOn = true
//		queue = true
//		queuetimeout = 10
		fires=
		[
			{entity="container_door_knock_2_relay",input="trigger",parameter="",delay=0.0}
		]		
	}
	//Helloooo? 
	SceneTable["-600_02"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningHello12.vcd"),
		char="wheatley"
		postdelay=0.0,
		predelay = 1.0
		next = "-600_03"
	}
	//Are you going to open the door? At any time?
	SceneTable["-600_03"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningHello13.vcd"),
		char="wheatley"
		postdelay=0.0,
		predelay = 1.0
		next = "-600_05"
	}
	//Hello? Can y--no?
	SceneTable["-600_05"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningHello15.vcd"),
		char="wheatley"
		postdelay=0.0,
		predelay = 1.0
		next = "-600_07"
	}
	//Are you going to open this door? Because it's fairly urgent.
	SceneTable["-600_07"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningHello17.vcd"),
		char="wheatley"
		postdelay=0.0,
		predelay = 1.0
		next = "-600_08"
	}
	//Oh, just open the door! [to self] That's too aggressive. [loud again] Hello, friend! Why not open the door?
	SceneTable["-600_08"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningHello18.vcd"),
		char="wheatley"
		postdelay=0.0,
		predelay = 1.0
		next = "-600_09"
	}
	//[to self] Hm. Could be Spanish, could be Spanish. [loud again] Hola, amigo! Abre la puerta! Donde esta--no. Um...
	SceneTable["-600_09"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningHello19.vcd"),
		char="wheatley"
		postdelay=0.0,
		predelay = 1.0
		next = "-600_11"
	}
	//Fine! No, absolutely fine. It's not like I don't have, you know, ten thousand other test subjects begging me to help them escape. You know, it's not like this place is about to EXPLODE.
	SceneTable["-600_11"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningHello21.vcd"),
		char="wheatley"
		postdelay=0.0,
		predelay = 1.0
		next = "-600_12"
		fires=
		[
			{entity="container_door_knock_3_relay",input="trigger",parameter="",delay=1.0}
		]
	}
	//Alright, look, okay, I'll be honest. You're the LAST test subject left. And if you DON'T help me, we're both going to die. Alright? I didn't want to say it, you dragged it out of me. Alright? Dead. Dos Muerte.
	SceneTable["-600_12"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningHello22.vcd"),
		char="wheatley"
		postdelay=0.0,
		predelay = 2.0
		next = null
		fires=
		[
			{entity="@glados",input="RunScriptCode",parameter="sp_a1_intro1_open_door_nags()",delay=1.0}
		]
	}
	
	///////////////////////////////////////////////
	///////////////////////////////////////////////
	
	//Hello!
	SceneTable["-601_01"] <-
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningHelloNag01.vcd"),
		postdelay=0.1,
		next=null,
		char="wheatley",
		noDingOff=true,
		noDingOn=true,
		idle=true,
		idlerandom=true,
		idlerepeat=true,
		idleminsecs=1.00,
		idlemaxsecs=2.600,
		idlegroup="sp_a1_intro1_open_door_nag",
		idleorderingroup=1
	}	
	//Helloooooooooooo!
	SceneTable["-601_02"] <-
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningHelloNag02.vcd"),
		postdelay=0.1,
		next=null,
		char="wheatley",
		noDingOff=true,
		noDingOn=true,
		idlegroup="sp_a1_intro1_open_door_nag",
		idleorderingroup=2
	}
	//Come on!
	SceneTable["-601_03"] <-
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningHelloNag03.vcd"),
		postdelay=0.1,
		next=null,
		char="wheatley",
		noDingOff=true,
		noDingOn=true,
		idlegroup="sp_a1_intro1_open_door_nag",
		idleorderingroup=3
	}
	//Open the door!
	SceneTable["-601_04"] <-
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningHelloNag04.vcd"),
		postdelay=0.1,
		next=null,
		char="wheatley",
		noDingOff=true,
		noDingOn=true,
		idlegroup="sp_a1_intro1_open_door_nag",
		idleorderingroup=4
	}
	//Hello!
	SceneTable["-601_05"] <-
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningHelloNag05.vcd"),
		postdelay=0.1,
		next=null,
		char="wheatley",
		noDingOff=true,
		noDingOn=true,
		idlegroup="sp_a1_intro1_open_door_nag",
		idleorderingroup=5
	}
	
	///////////////////////////////////////////////
	///////////////////////////////////////////////

	//YES! I KNEW someone was alive in here!
	SceneTable["-602_01"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningDoorOpened01.vcd"),
		char="wheatley"
		postdelay = 0.0
		predelay = 0.0
		next = "-602_02"
		noDingOff = true
		noDingOn = true
		fires=
		[
//			{entity="are_you_alright_vcd",input="Start",parameter="",delay=0.6}
			{entity="spherebot_train_1_chassis_1",input="MoveToPathNode",parameter="spherebot_path_inside_hallway",delay=3.5}
		]
	}
	
	//AGH! You look TERRIB--you look good. Looking good, actually. If I'm honest.
	SceneTable["-602_02"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningFirstLook01.vcd"),
		char="wheatley"
		postdelay = 0.0
		predelay = 0.0
		next = "-602_03"
		noDingOff = true
		noDingOn = true
	}
	
	//How are you? How you feeling, you okay? Wait. Don't answer that. Too much deep relaxation, what it does is it relaxes the gums. And the vibrations from talking 
	SceneTable["-602_03"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningPanicSituation01.vcd"),
		char="wheatley"
		postdelay = 0.0
		predelay = 0.0
		next = "-602_04"
		noDingOff = true
		noDingOn = true
	}
	
	//So, word of advice: there's PLENTY of time to recover. Just take it slow.
	SceneTable["-602_04"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningPanicSituation03.vcd"),
		char="wheatley"
		postdelay = 0.0
		predelay = 0.0
		next = null
		noDingOff = true
		noDingOn = true
		fires=
		[
			{entity="emergency_evacuation_vcd",input="Start",parameter="",delay=0.0}
		]
	}
	
	///////////////////////////////////////////////
	///////////////////////////////////////////////

	//Simple word. 'Apple'.
	SceneTable["-603_01"] <-
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningBrainDamageAppleNag01.vcd"),
		char="wheatley"
		postdelay=0.0,
		predelay = 1.5
		next = "-603_02"
		noDingOff = true
		noDingOn = true
//		queue = true
//		queuetimeout = 10
	}
	//Just say 'Apple'. Classic. Very simple.
	SceneTable["-603_02"] <-
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningBrainDamageAppleNag02.vcd"),
		char="wheatley"
		postdelay=0.0,
		predelay = 1.0
		next = "-603_03"
	}
	//Ay. Double Pee-Ell-Ee.
	SceneTable["-603_03"] <-
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningBrainDamageAppleNag03.vcd"),
		char="wheatley"
		postdelay=0.0,
		predelay = 1.0
		next = "-603_04"
	}
	//Just say 'Apple'. Easy word, isn't it? 'Apple'.
	SceneTable["-603_04"] <-
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningBrainDamageAppleNag04.vcd"),
		char="wheatley"
		postdelay=0.0,
		predelay = 1.0
		next = "-603_05"
	}
	//How would you use it in a sentence? "Mmm, this apple's crunchy," you might say. And I'm not even asking you for the whole sentence. Just the word 'Apple'.
	SceneTable["-603_05"] <-
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningBrainDamageAppleNag05.vcd"),
		char="wheatley"
		postdelay=0.0,
		predelay = 1.0
		next = null
		fires=
		[
			{entity="sphere_player_has_pressed_space_second",input="Trigger",parameter="",delay=1.5}
		]
	}
	
	///////////////////////////////////////////////
	///////////////////////////////////////////////

	//Okay, you know what? That's close enough. Just hold tight.
	SceneTable["-604_01"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningCloseEnough01.vcd"),
		char="wheatley"
		postdelay = 0.0
		predelay = 0.0
		next = null
		noDingOff = true
		noDingOn = true
		fires=
		[
			{entity="core_meltdown_rl",input="Trigger",parameter="",delay=0.0}
		]
	}
	
	///////////////////////////////////////////////
	///////////////////////////////////////////////

	//Ohhhhhh! - removed
	SceneTable["-605_02"] <-
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningWallHitOne01.vcd"),
		char="wheatley"
		postdelay=0.0,
		predelay = 3.5
		next = "-605_03"
		noDingOff = true
		noDingOn = true
		queue = true
		queuetimeout = 10
	}
	
	//Good news: that is NOT a docking station. So there's one mystery solved. I'm going to attempt a manual override on this wall. Could get a bit technical! Hold on!
	SceneTable["-605_01"] <-
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningWallHitOne02.vcd"),
		char="wheatley"
		postdelay=0.0,
		predelay = 3.5
		next = null
		noDingOff = true
		noDingOn = true
		queue = true
		queuetimeout = 10
		
		fires=
		[
			{entity="ram_exit_wall_relay",input="Trigger",parameter="",delay=0.0}
			{entity="almost_there_vcd",input="Start",parameter="",delay=3.0}
		]
	}
	
	///////////////////////////////////////////////
	///////////////////////////////////////////////
	
	// Alright, I wasn't going to mention this to you, but I am in PRETTY HOT WATER here.
	SceneTable["-606_01"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningOutside01.vcd"),
		char="wheatley"
		postdelay = 0.0,
		predelay = 0.0
		next = "-606_02"
		noDingOff = true
		noDingOn = true
		queue = true
		queuetimeout = 10
	}


	//How you doing down there? You still holding on?
	SceneTable["-606_02"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningNavigate17.vcd"),
		char="wheatley"
		postdelay = 0.0,
		predelay = 1.0
		next = "-606_03"
	}

	// The reserve power ran out, so of course the whole relaxation center stops waking up the bloody test subjects.
	SceneTable["-606_03"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningOutside02.vcd"),
		char="wheatley"
		postdelay = 0.0,
		predelay = 1.0
		next = "-606_04"
	}
	
	//Hold on! This is a bit tricky!
	SceneTable["-606_04"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningNavigate05.vcd"),
		char="wheatley"
		postdelay = 0.0,
		predelay = 1.0
		next = "-606_05"
	}


	//And of course nobody tells ME anything. No.
    SceneTable["-606_05"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningOutside03.vcd"),
		char="wheatley"
		postdelay = 0.0,
		predelay = 1.0
		next = "-606_06"
	}


	//Anyway, why should I be kept informed about the life functions of the ten thousand bloody test subjects I'm supposed to be in charge of? Why would you mention it to me?
	SceneTable["-606_06"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningOutside04.vcd"),
		char="wheatley"
		postdelay = 0.0,
		predelay = 1.0
		next = "-606_07"
	}



	//Oi, it's close... can you see? Am I gonna make it through? Have I got enough space?
	SceneTable["-606_07"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningNavigate12.vcd"),
		char="wheatley"
		postdelay = 0.0,
		predelay = 1.0
		next = "-606_08"
	}
	//Agh, just... I just gotta get it through here...
	SceneTable["-606_08"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningNavigate06.vcd"),
		char="wheatley"
		postdelay = 0.0,
		predelay = 1.0
		next = "-606_09"
	}
	//Okay, I've just gotta concentrate!
	SceneTable["-606_09"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningNavigate03.vcd"),
		char="wheatley"
		postdelay = 0.0,
		predelay = 1.0
		next = "-606_10"
	}

	//And whose fault do you think it's going to be when the management comes down here and finds ten thousand flipping vegetables?
	SceneTable["-606_10"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningOutside05.vcd"),
		char="wheatley"
		postdelay = 0.0,
		predelay = 1.3
		next = null
	}
	
	///////////////////////////////////////////////
	///////////////////////////////////////////////
	
	// Let me explain. Most test subjects do experience some cognitive deterioration after a few months in suspension.
	SceneTable["-607_01"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningBrainDamage01.vcd"),
		char="wheatley"
		postdelay = 0.0,
		predelay = 0.0
		next = "-607_02"
		noDingOff = true
		noDingOn = true
		queue = true
		queuetimeout = 10
	}

	//Straight away you're thinking, "Oo, that doesn't sound good. But don't be alarmed, alright? Because ah... well, actually, if you DO feel alarmed, hold onto that.
	SceneTable["-607_02"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningBrainDamage03.vcd"),
		char="wheatley"
		postdelay = 0.0,
		predelay = 0.0
		next = "-607_03"
	}
	
	//Do you understand what I'm saying? Does any of this make sense? Just tell me 'Yes'.
	SceneTable["-607_03"] <- 
	{
		vcd=CreateSceneEntity("scenes/npc/sphere03/OpeningBrainDamage04.vcd"),
		char="wheatley"
		postdelay = 0.0,
		predelay = 0.0
		next = null
		fires=
		[
			{entity="hint_press_spacebar_to_talk",input="ShowHint",parameter="",delay=0.0}
			{entity="sphere_player_has_pressed_space_first",input="Enable",parameter="",delay=0.0}
		]
	}
}

// ============================================================================
// Called when wheatley wants you to open the door
// ============================================================================
function sp_a1_intro1_open_door_sequence()
{
	GladosPlayVcd( -600 )
}

function sp_a1_intro1_open_door_nags()
{
	GladosPlayVcd( -601 )
}

function sp_a1_intro1_knew_someone_alive()
{
	GladosCharacterStopScene("wheatley")
	GladosPlayVcd( -602 )
}

function sp_a1_intro1_say_apple_nag()
{
	GladosPlayVcd( -603 )
}

function sp_a1_intro1_jumping_close_enough()
{
	GladosCharacterStopScene("wheatley")
	GladosPlayVcd( -604 )
}

function sp_a1_intro1_first_wall_impact()
{
	GladosPlayVcd( -605 )
}

function sp_a1_intro1_container_start_moving()
{
	GladosPlayVcd( -606 )
}

function sp_a1_intro1_explain_brain_damage()
{
	GladosPlayVcd( -607 )
}
