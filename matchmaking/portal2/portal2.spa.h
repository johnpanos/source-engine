//===== Copyright 1996-2009, Valve Corporation, All rights reserved. ======//
//
// Purpose: Portal 2 port: the title's XLAST-generated constants
//          (portal2.spa.h), which the leaked source tree does not contain.
//
// The achievement, avatar award, game mode and single-player presence values
// are the ones the retail Portal 2 Linux matchmaking.so ships in its title
// tables. Not original Valve source; the repository's provenance and
// distribution warning applies.
//
//===========================================================================//

#ifndef PORTAL2_SPA_H
#define PORTAL2_SPA_H
#ifdef _WIN32
#pragma once
#endif

//
// Achievements (CMatchTitle::DescribeTitleAchievements)
//
#define ACHIEVEMENT_SURVIVE_CONTAINER_RIDE           1
#define ACHIEVEMENT_WAKE_UP                          2
#define ACHIEVEMENT_LASER                            3
#define ACHIEVEMENT_BRIDGE                           4
#define ACHIEVEMENT_BREAK_OUT                        5
#define ACHIEVEMENT_STALEMATE_ASSOCIATE              6
#define ACHIEVEMENT_ADDICTED_TO_SPUDS                7
#define ACHIEVEMENT_BLUE_GEL                         8
#define ACHIEVEMENT_ORANGE_GEL                       9
#define ACHIEVEMENT_WHITE_GEL                        10
#define ACHIEVEMENT_TRACTOR_BEAM                     11
#define ACHIEVEMENT_TRIVIAL_TEST                     12
#define ACHIEVEMENT_WHEATLEY_TRIES_TO                13
#define ACHIEVEMENT_SHOOT_THE_MOON                   14
#define ACHIEVEMENT_BOX_HOLE_IN_ONE                  15
#define ACHIEVEMENT_SPEED_RUN_LEVEL                  16
#define ACHIEVEMENT_COMPLIANT                        17
#define ACHIEVEMENT_SAVE_CUBE                        18
#define ACHIEVEMENT_LAUNCH_TURRET                    19
#define ACHIEVEMENT_CLEAN_UP                         20
#define ACHIEVEMENT_REENTER_TEST_CHAMBERS            21
#define ACHIEVEMENT_NOT_THE_DROID                    22
#define ACHIEVEMENT_SAVE_REDEMPTION_TURRET           23
#define ACHIEVEMENT_CATCH_CRAZY_BOX                  24
#define ACHIEVEMENT_NO_BOAT                          25
#define ACHIEVEMENT_A3_DOORS                         26
#define ACHIEVEMENT_PORTRAIT                         27
#define ACHIEVEMENT_DEFIANT                          28
#define ACHIEVEMENT_BREAK_MONITORS                   29
#define ACHIEVEMENT_HI_FIVE_YOUR_PARTNER             30
#define ACHIEVEMENT_TEAM_BUILDING                    31
#define ACHIEVEMENT_MASS_AND_VELOCITY                32
#define ACHIEVEMENT_HUG_NAME                         33
#define ACHIEVEMENT_EXCURSION_FUNNELS                34
#define ACHIEVEMENT_NEW_BLOOD                        35
#define ACHIEVEMENT_NICE_CATCH                       36
#define ACHIEVEMENT_TAUNTS                           37
#define ACHIEVEMENT_YOU_MONSTER                      38
#define ACHIEVEMENT_PARTNER_DROP                     39
#define ACHIEVEMENT_PARTY_OF_THREE                   40
#define ACHIEVEMENT_PORTAL_TAUNT                     41
#define ACHIEVEMENT_TEACHER                          42
#define ACHIEVEMENT_WITH_STYLE                       43
#define ACHIEVEMENT_LIMITED_PORTALS                  44
#define ACHIEVEMENT_FOUR_PORTALS                     45
#define ACHIEVEMENT_SPEED_RUN_COOP                   46
#define ACHIEVEMENT_STAYING_ALIVE                    47
#define ACHIEVEMENT_TAUNT_CAMERA                     48
#define ACHIEVEMENT_ROCK_CRUSHES_ROBOT               49
#define ACHIEVEMENT_SPREAD_THE_LOVE                  50
#define ACHIEVEMENT_SUMMER_SALE                      51

//
// Avatar awards (CMatchTitle::DescribeTitleAvatarAwards)
//
#define AVATARASSETAWARD_SHIRT1           1
#define AVATARASSETAWARD_SHIRT2           2
#define AVATARASSETAWARD_SHIRT3           3
#define AVATARASSETAWARD_HAT1             4
#define AVATARASSETAWARD_PROP1            5

//
// Game mode context (g_pcv_CONTEXT_GAME_MODE, see inc_gamemode.inc)
//
#define CONTEXT_GAME_MODE_SP				0
#define CONTEXT_GAME_MODE_COOP				1
#define CONTEXT_GAME_MODE_COOP_CHALLENGE	2
#define CONTEXT_GAME_MODE_COOP_COMMUNITY	3

//
// Single-player presence text context (g_pcv_CONTEXT_SP_PRESENCE_TEXT):
// the chapter number, 0 for maps outside the campaign.
//
#define CONTEXT_SP_PRESENCE_TEXT_DEFAULT	0
#define CONTEXT_SP_PRESENCE_TEXT_CH1        1
#define CONTEXT_SP_PRESENCE_TEXT_CH2        2
#define CONTEXT_SP_PRESENCE_TEXT_CH3        3
#define CONTEXT_SP_PRESENCE_TEXT_CH4        4
#define CONTEXT_SP_PRESENCE_TEXT_CH5        5
#define CONTEXT_SP_PRESENCE_TEXT_CH6        6
#define CONTEXT_SP_PRESENCE_TEXT_CH7        7
#define CONTEXT_SP_PRESENCE_TEXT_CH8        8
#define CONTEXT_SP_PRESENCE_TEXT_CH9        9
#define CONTEXT_SP_PRESENCE_TEXT_CH10       10

//
// Xbox LIVE context, property and query ids. The retail PC build compiles the
// code that sends them to Xbox LIVE out (SetAllUsersContext/Property and the
// X360 session search keys), so their values are not in any PC binary and
// were not recovered. They are distinct ids with no effect on this platform.
//
#define CONTEXT_PRESENCE_MAINMENU				1
#define CONTEXT_PRESENCE_SP						2
#define CONTEXT_PRESENCE_COOPMENU				3
#define CONTEXT_PRESENCE_COOPGAME				4
#define CONTEXT_PRESENCE_COOPGAME_TRACK			5

#define CONTEXT_SP_PRESENCE_TEXT				0x101
#define CONTEXT_COOP_PRESENCE_TRACK				0x102
#define CONTEXT_COOP_PRESENCE_WAITING			0x103
#define CONTEXT_COOP_PRESENCE_TAGLINE			0x104

#define CONTEXT_COOP_PRESENCE_TAGLINE_DEFAULT		0
#define CONTEXT_COOP_PRESENCE_TAGLINE_SPLITSCREEN	1
#define CONTEXT_COOP_PRESENCE_TAGLINE_SYSTEMLINK	2
#define CONTEXT_COOP_PRESENCE_TAGLINE_FRIEND		3
#define CONTEXT_COOP_PRESENCE_TAGLINE_QUICKMATCH	4

#define PROPERTY_MMVERSION						0x10000201
#define PROPERTY_INSTALLED_DLC1					0x10000202
#define PROPERTY_REQUIRED_DLC1					0x10000212
#define PROPERTY_NUMPLAYERS						0x10000221
#define PROPERTY_COOP_TRACK_CHAPTER				0x10000222
#define PROPERTY_COOP_TRACK_NUMCHAPTERS			0x10000223

#define SESSION_MATCH_QUERY_COOP				0

#endif // PORTAL2_SPA_H
