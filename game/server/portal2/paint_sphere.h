//========= Portal 2 reconstruction ============================================//
//
// Purpose: paint_sphere: paints the brush surfaces inside a sphere with one
//          paint power when its Paint input fires.
//
// Written for the Portal 2 port from consumers, FGD data and related Valve code
// (no original source, pseudocode or debug data exists). Members follow the
// retail Portal 2 server's CPaintSphere datadesc. Not original Valve source;
// the repository's provenance and distribution warning applies.
//
// Server-only entity; nothing is networked.
//
//=============================================================================//

#ifndef PAINT_SPHERE_H
#define PAINT_SPHERE_H
#ifdef _WIN32
#pragma once
#endif

#include "paint_enum.h"

class CPaintSphere : public CLogicalEntity
{
public:
	DECLARE_CLASS( CPaintSphere, CLogicalEntity );
	DECLARE_DATADESC();

	CPaintSphere( void );

	void Paint( inputdata_t &inputdata );

private:
	int		m_nPaintType;		// PaintPowerType; the FGD's "Erase" choice is NO_POWER
	float	m_flSphereRadius;
	float	m_flAlphaPercent;	// Paint density, 0..1
};

#endif // PAINT_SPHERE_H
