DoIncludeScript( "choreo/sphere_choreo_include", self.GetScriptScope() )

//GladosPlayVcd indexes for original text
DialogVcd <- {}
DialogVcd[200] <- 384
DialogVcd[202] <- 385
DialogVcd[203] <- 400
DialogVcd[204] <- 386
DialogVcd[205] <- 387


//Jailbreak
Dialog[200] <- { speaker = WHEATLEY, one = "Good to see you made it" two = "What are you doing all the way over there?" nextLine = 201, nextLineDelay = 4}
Dialog[201] <- { speaker = WHEATLEY, one = "We need to figure out a way to destroy the toxin vaporators" two = "Make your way over to me" }

Dialog[202] <- { speaker = WHEATLEY, one = "That laser should do the trick" two = "Use it to destroy the vaporator!", nagDelay = 30 }

Dialog[203] <- { speaker = WHEATLEY, one = "You disabled the toxin!" two = "Now break that tube!" }

Dialog[204] <- { speaker = WHEATLEY, one = "We want to leave through that tube!" two = "But you must destroy the toxin first" }

Dialog[205] <- { speaker = WHEATLEY, one = "Jump in the tube! I'll be right behind you." }

function SpeakLineVcd( arg )
{
	if (arg in DialogVcd)
	{
		EntFire("@glados","RunScriptCode","GladosPlayVcd("+DialogVcd[arg]+")", 0.00)
	}
	else
	{
		SpeakLine( arg )
	}
}


function ToxinWheatleyGoodToSeeYou()
{
	SpeakLineVcd( 200 )
}

function ToxinWheatleyGetTheLaser()
{
	SpeakLineVcd( 202 )
}

function ToxinWheatleyToxinIsOff()
{
	SpeakLineVcd( 203 )
}

function ToxinWheatleyTurnToxinOff()
{
	SpeakLineVcd( 204 )
}

function ToxinWheatleyJumpInTheTube()
{
	SpeakLineVcd( 205 )
}