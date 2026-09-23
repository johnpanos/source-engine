//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Layout records of the types that cross the declared C++11 facades
//          of C++20 modules (RFC 0006 M0, toolchain.abi.v1). The same unit is
//          compiled once per dialect; consumers compare the records with each
//          other and with the frozen LP64 table. C++11-compatible.
//
//=============================================================================//

#ifndef TOOLCHAINTEST_BOUNDARY_LAYOUT_H
#define TOOLCHAINTEST_BOUNDARY_LAYOUT_H

enum
{
	kToolchainMaxFields = 12
};

struct ToolchainLayoutRecord
{
	const char *name;
	unsigned long size;
	unsigned long align;
	int fieldCount;
	unsigned long offsets[kToolchainMaxFields];
	long cplusplus; // dialect of the unit that produced the record
};

// Defined by boundary_layout_unit.cpp once per dialect unit.
const ToolchainLayoutRecord *ToolchainLayoutsLegacy( int *pCount );
const ToolchainLayoutRecord *ToolchainLayoutsCxx20( int *pCount );

#endif // TOOLCHAINTEST_BOUNDARY_LAYOUT_H
