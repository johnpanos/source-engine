//===== Copyright 1996-2009, Valve Corporation, All rights reserved. ======//
//
// Purpose: Portal 2 port: the matchmaking data record that IPlayerLocal
//          exposes. The CS:GO framework took it from
//          matchmaking/cstrike15/imatchext_cstrike15.h, whose title data
//          structures collide with Portal 2's; only this record is kept here,
//          as defined there.
//
//===========================================================================//

#ifndef MM_MATCHMAKINGDATA_H
#define MM_MATCHMAKINGDATA_H
#ifdef _WIN32
#pragma once
#endif

#define MM_AVG_CONST 100.0f

// MatchmakingDataMode
// We keep seperate matchmaking data for a different kinds of types.  Types that
// aren't specific to certain game play rules (like gungame progressive) should
// use the general type.
enum MatchmakingDataType
{
	MMDATA_TYPE_GENERAL = 0,
	MMDATA_TYPE_GGPROGRESSIVE,

	MMDATA_TYPE_COUNT,
};

// MatchmakingDataScope
// There are two kinds of scope for each MatchmakingDataType: Lifetime and Round.
// Lifetime is the user's liifetime matchmaking values serialized to/from the user's
// profile.
// Round is the user's current values accumulated during the current round of game
// play.  At the end of the round, the Round values are aggregated with the lifetime
// values according to the formulas specified in the MatchSystem resource file.
enum MatchmakingDataScope
{
	MMDATA_SCOPE_LIFETIME = 0,
	MMDATA_SCOPE_ROUND,
	
	MMDATA_SCOPE_COUNT,
};

#define MATCHMAKINGDATA_FIELD(name) short name [MMDATA_TYPE_COUNT][MMDATA_SCOPE_COUNT];

// MatchmakingData
// This is the data structure used for matchmaking.  Any fields addsed to this structure
// need to be added to all of the appropriate areas where we calculate averages and 
// serialize this data.
struct MatchmakingData
{
	MATCHMAKINGDATA_FIELD(mContribution);
	MATCHMAKINGDATA_FIELD(mMVPs);
	MATCHMAKINGDATA_FIELD(mKills);
	MATCHMAKINGDATA_FIELD(mDeaths);
	MATCHMAKINGDATA_FIELD(mHeadShots);
	MATCHMAKINGDATA_FIELD(mDamage);
	MATCHMAKINGDATA_FIELD(mShotsFired);
	MATCHMAKINGDATA_FIELD(mShotsHit);
	MATCHMAKINGDATA_FIELD(mDominations);
	MATCHMAKINGDATA_FIELD(mRoundsPlayed);
};

#undef MATCHMAKINGDATA_FIELD

#endif // MM_MATCHMAKINGDATA_H
