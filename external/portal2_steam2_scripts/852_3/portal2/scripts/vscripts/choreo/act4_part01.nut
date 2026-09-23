DoIncludeScript( "choreo/sphere_choreo_include", self.GetScriptScope() )


//**IGNORE THIS - ERIK WILL FILL THIS IN WHEN REAL DIALOG DROPS IN**
DialogVcd <- {}



//**OK YOU CAN STOP IGNORING THINGS STARTING HERE
//Single line test 
	Dialog[1000] <- { speaker = WHEATLEY, one = "This is a test..." }
//Multi line test
	Dialog[1001] <- { speaker = COMPUTER, one = "This is line 1." two = "This is line 2." }
//Multi line test linked to another line
	Dialog[1002] <- { speaker = WHEATLEY, one = "Line 1." two = "Line 2." nextLine = 1003, nextLineDelay = 4 }
	Dialog[1003] <- { speaker = WHEATLEY, one = "Linked line." }

	Dialog[2000] <- { speaker = WHEATLEY, one = "Thanks, that was fun! I'm going to kill you now. <paraphrased>" }
	Dialog[2001] <- { speaker = WHEATLEY, one = "Any last words?" }
	Dialog[2002] <- { speaker = WHEATLEY, one = "Ooookay then." }
	Dialog[2003] <- { speaker = WHEATLEY, one = "whoa! whoa! what are you doing?" }
	Dialog[2004] <- { speaker = WHEATLEY, one = "Gah! Get back here!" }
	Dialog[2005] <- { speaker = WHEATLEY, one = "Oh dear. This is bad." }
	Dialog[2006] <- { speaker = WHEATLEY, one = "Enjoy your last meal. Hope you like mashed potato!" }
	Dialog[2007] <- { speaker = WHEATLEY, one = "ok, too risky to toy with I guess... goodbye!" }


	Dialog[2100] <- { speaker = WHEATLEY, one = "Haha!!" nextLine = 2101, nextLineDelay = 4}
	Dialog[2101] <- { speaker = WHEATLEY, one = "Oh hell. Wrong batch." }

	Dialog[2110] <- { speaker = WHEATLEY, one = "Ok, got the turrets sorted. You're in for it now!" }

	Dialog[2200] <- { speaker = WHEATLEY, one = "Turn back! You're getting too close!" }
	Dialog[2201] <- { speaker = WHEATLEY, one = "Ok, seriously now. Last warning!" }



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



function Act4Test()
{
	SpeakLineVcd( 1000 )
}

function WheatleyBetrayalGoodbye()
{
	SpeakLineVcd ( 2000 )
}

function WheatleyBetrayalLastWords1()
{
	SpeakLineVcd ( 2001 )
}

function WheatleyBetrayalLastWords2()
{
	SpeakLineVcd ( 2002 )
}

function WheatleyBetrayalPaintReaction()
{
	SpeakLineVcd ( 2003 )
}

function WheatleyBetrayalEscape1()
{
	SpeakLineVcd ( 2004 )
}

function WheatleyBetrayalEscape2()
{
	SpeakLineVcd ( 2005 )
}

function WheatleyBetrayalLastMeal()
{
	SpeakLineVcd ( 2006 )
}

function WheatleyBetrayalAbortSpeech()
{
	SpeakLineVcd ( 2007 )
}

function WheatleyEscapeDefectiveTurretUnveil()
{
	SpeakLineVcd ( 2100 )
}

function WheatleyEscapeWorkingTurretUnveil()
{
	SpeakLineVcd ( 2110 )
}

function WheatleyEscapeConveyor()
{
	SpeakLineVcd ( 2200 )
}

function WheatleyEscapeConveyorTbeam()
{
	SpeakLineVcd ( 2201 )
}
