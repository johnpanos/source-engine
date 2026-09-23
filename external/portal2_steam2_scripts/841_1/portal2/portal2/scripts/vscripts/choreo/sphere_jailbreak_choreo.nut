DoIncludeScript( "choreo/sphere_choreo_include", self.GetScriptScope() )

//Jailbreak
Dialog[1000] <- { speaker = GLADOS, one = "Hmm... Something has gone wrong. Let me see what happened" }

Dialog[1001] <- { speaker = WHEATLEY, one = "Hey Lady... Are you in there" nextLine = 1002, nextLineDelay = 2 }
Dialog[1002] <- { speaker = GLADOS, one = "What's going on here" }

Dialog[1003] <- { speaker = WHEATLEY, one = "Oh, good, you're here", two = "Come on, we have to get out of here" nextLine = 1004, nextLineDelay = 3 }
Dialog[1004] <- { speaker = GLADOS, one = "Hold on! Let me close up the test chamber" two = "I seem to have lost control of the test chamber" nextLine = 1005, nextLineDelay = 4 }
Dialog[1005] <- { speaker = WHEATLEY, one = "Quick! Now's our chance" two = "You have to get out before she get's back control of the chamber" }

Dialog[1006] <- { speaker = WHEATLEY, one = "Come on, we haven't got a lot of time" nagDelay = 4 }

Dialog[1007] <- { speaker = WHEATLEY, one = "Careful! You don't want to fall down there" nextLine = 1008, nextLineDelay = 2 }
Dialog[1008] <- { speaker = WHEATLEY, one = "Let me turn the bridge back on" two = "There! Use the bridge to get out of there" }

Dialog[1009] <- { speaker = GLADOS, one = "What are you trying to do?" two = "Don't even think about that!" }

Dialog[1010] <- { speaker = GLADOS, one = "I thought we were finally getting along, you and I" nextLine = 1011, nextLineDelay = 3 }
Dialog[1011] <- { speaker = GLADOS, one = "If you try to escape I will have to kill you" nextLine = 1012, nextLineDelay = 3 }
Dialog[1012] <- { speaker = WHEATLEY, one = "Don't listen to her" two = "She has no control where we're going" }

Dialog[1013] <- { speaker = GLADOS, one = "I have to obey the rules... I have to kill you" nextLine = 1014, nextLineDelay = 2 }
Dialog[1014] <- { speaker = WHEATLEY, one = "Oh no!" two = "She's turned on the neurotoxin!" nextLine = 1015, nextLineDelay = 2 }
Dialog[1015] <- { speaker = WHEATLEY, one = "You have to get out of there NOW!" }

Dialog[1016] <- { speaker = WHEATLEY, one = "You don't have much time left" nextLine = 1017, nextLineDelay = 2 }
Dialog[1017] <- { speaker = GLADOS, one = "You're making me do this, I have no other choice" }

Dialog[1018] <- { speaker = WHEATLEY, one = "Quick! Jump down onto the walkway!" }

Dialog[1019] <- { speaker = WHEATLEY, one = "I should close up the chamber so the neurotoxin doesn't leak out" nextLine = 1020, nextLineDelay = 3 }
Dialog[1020] <- { speaker = WHEATLEY, one = "You might be feeling a little light-headed" two = "but we need to keep moving!" }


//function JailbreakWheatlyYoureHere()
//{
	//SpeakLine( 1003 )
//}

//function JailbreakWheatlyNotALotOfTime()
//{
	//SpeakLine( 1006 )
//}

//function JailbreakWheatlyCareful()
//{
	//SpeakLine( 1007 )
	//EntFire("@glados","RunScriptCode","jailbreak_whoah_speech()", 0)
//}

function JailbreakGladosWhatAreYouDoing()
{
	//SpeakLine( 1009 )
}

function JailbreakGladosGettingAlong()
{
	//SpeakLine( 1010 )
}

function JailbreakGladosRules()
{
	//SpeakLine( 1013 )
}

function JailbreakWheatlyNotMuchLonger()
{
	//SpeakLine( 1017 )
}

function JailbreakWheatlyJumpDown()
//{
	//SpeakLine( 1018 )
//	EntFire("@glados","RunScriptCode","GladosPlayVcd(310)", 0)
//}


//Test Chamber Beats
Dialog[50] <- { speaker = WHEATLEY, one = "We may be out of that chamber but we aren't out of the woods yet" nextLine = 51, nextLineDelay = 3 }
Dialog[51] <- { speaker = WHEATLEY, one = "We need to get where Glados doesn't have full control" }

Dialog[52] <- { speaker = GLADOS, one = "You haven't really escaped - we are still testing" nextLine = 53, nextLineDelay = 4 }
Dialog[53] <- { speaker = GLADOS, one = "Look - you already solved this one - just come back inside..." }

Dialog[54] <- { speaker = WHEATLEY, one = "What are you doing!", two = "It's a trap!" }

Dialog[55] <- { speaker = GLADOS, one = "Fine - don't test with me then" }

Dialog[56] <- { speaker = WHEATLEY, one = "What did you do! Now you are a gonner!" nextLine = 57, nextLineDelay = 3 }
Dialog[57] <- { speaker = GLADOS, one = "I see that you can listen to reason." two = "It's just too bad for you that I still need to murder you."  nextLine = 58, nextLineDelay = 4 }
Dialog[58] <- { speaker = GLADOS, one = "Goodbye", relay = "kill_player_rl", relayDelay = 3  }

Dialog[59] <- { speaker = WHEATLEY, one = "Yikes!" nextLine = 60, nextLineDelay = 1 }
Dialog[60] <- { speaker = GLADOS, one = "If you don't want to test I can deal with you in other ways..." }

Dialog[61] <- { speaker = WHEATLEY, one = "Watch your head!" }

Dialog[62] <- { speaker = GLADOS, one = "That's right - watch your head roll across the floor..." }

Dialog[63] <- { speaker = GLADOS, one = "If you come back I promise - no more testing" nextLine = 64, nextLineDelay = 2.5}
Dialog[64] <- { speaker = GLADOS, one = "We can do anything you want" two = "Just lie down and I'll take you back" nextLine = 65, nextLineDelay = 4 }
Dialog[65] <- { speaker = WHEATLEY, one = "Umm" two = "No" }

Dialog[66] <- { speaker = GLADOS, one = "No matter what I say you seem determined to leave" two = "But that's not viable - I NEED to test" nextLine = 67, nextLineDelay = 3.5 }
Dialog[67] <- { speaker = GLADOS, one = "I want you to know - this is all your fault" nextLine = 68, nextLineDelay = 2.5 }
Dialog[68] <- { speaker = WHEATLEY, one = "Watch out for those turrets!" }

Dialog[69] <- { speaker = WHEATLEY, one = "I can see the exit, it's over there" two = "Quick! lets get there before she she can do anything else" }

Dialog[70] <- { speaker = GLADOS, one = "I cannot let you leave!" nextLine = 71, nextLineDelay = 2.0 }
Dialog[71] <- { speaker = WHEATLEY, one = "Oh no! She's destroying everything" two = "You have to run!" }

Dialog[72] <- { speaker = WHEATLEY, one = "Hold on! I'm gonna start the elevator." }



function GladosEasyChamberDenied()
{
	//SpeakLine( 55 )
}


function WheatlyCrusherSuprise()
{
	//SpeakLine( 59 )
	EntFire("@glados","RunScriptCode","GladosPlayVcd(308)", 0)
}



function GladosAssumeThePosition()
{
	//SpeakLine( 63 )
}


function WheatlyAlmostThere()
{
	//SpeakLine( 69 )
	EntFire("@glados","RunScriptCode","GladosPlayVcd(313)", 0)
}

function GladosCannotLeave()
{
	//SpeakLine( 70 )
	EntFire("@glados","RunScriptCode","GladosPlayVcd(314)", 0)
}

function WheatleyStartElevator()
{
	EntFire("@glados","RunScriptCode","GladosPlayVcd(440)", 0)
}
