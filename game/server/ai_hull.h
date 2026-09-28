//========= Copyright Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#ifndef AI_HULL_H
#define AI_HULL_H
#pragma once

class Vector;

#ifdef FSTOP
// The F-Stop blob hull's world: like MASK_NPCWORLDSTATIC but without grates,
// so a fluid flows through them (CS:GO public/bspflags.h).
#define MASK_NPCWORLDSTATIC_FLUID	(CONTENTS_SOLID|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP)
#endif

//=========================================================
// Link Properties. These hulls must correspond to the hulls
// in AI_Hull.cpp!
//=========================================================
enum Hull_t
{
	HULL_HUMAN,				// Combine, Stalker, Zombie...
	HULL_SMALL_CENTERED,	// Scanner
	HULL_WIDE_HUMAN,		// Vortigaunt
	HULL_TINY,				// Headcrab
	HULL_WIDE_SHORT,		// Bullsquid
	HULL_MEDIUM,			// Cremator
	HULL_TINY_CENTERED,		// Manhack 
	HULL_LARGE,				// Antlion Guard
	HULL_LARGE_CENTERED,	// Mortar Synth
	HULL_MEDIUM_TALL,		// Hunter
#ifdef FSTOP
	HULL_TINY_FLUID,		// Blob (F-Stop npc_surface); its links pass grates
#endif
//--------------------------------------------
	NUM_HULLS,
	HULL_NONE				// No Hull (appears after num hulls as we don't want to count it)
};

enum Hull_Bits_t
{
	bits_HUMAN_HULL				=	0x00000001,
	bits_SMALL_CENTERED_HULL	=	0x00000002,
	bits_WIDE_HUMAN_HULL		=	0x00000004,
	bits_TINY_HULL				=	0x00000008,
	bits_WIDE_SHORT_HULL		=	0x00000010,
	bits_MEDIUM_HULL			=	0x00000020,
	bits_TINY_CENTERED_HULL		=	0x00000040,
	bits_LARGE_HULL				=	0x00000080,
	bits_LARGE_CENTERED_HULL	=	0x00000100,
	bits_MEDIUM_TALL_HULL		=	0x00000200,
#ifdef FSTOP
	bits_TINY_FLUID_HULL		=	0x00000400,
	bits_HULL_BITS_MASK			=	0x000006ff,
#else
	bits_HULL_BITS_MASK			=	0x000002ff,
#endif
};

inline int HullToBit( Hull_t hull )
{
	return ( 1 << hull );
}



//=============================================================================
//	>> CAI_Hull
//=============================================================================
namespace NAI_Hull
{
	const Vector &Mins(int id);
	const Vector &Maxs(int id);

	const Vector &SmallMins(int id);
	const Vector &SmallMaxs(int id);

	float		Length(int id);
	float		Width(int id);
	float		Height(int id);

	int			Bits(int id);
 
	const char*	Name(int id);

#ifdef FSTOP
	// The contents node-graph link tests trace this hull against (CS:GO
	// ai_hull): MASK_NPCWORLDSTATIC for every hull but the blob's.
	unsigned int TraceMask(int id);
#endif

	Hull_t		LookupId(const char *szName);
};

#endif // AI_HULL_H
