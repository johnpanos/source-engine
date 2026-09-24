//========= Portal 2 port ======================================================//
//
// Purpose: Blobulator metaball particles.
//
// Clean-room implementation of the blobulator interface the Portal 2 paint
// blob renderer uses (not Valve's blobulator library). Layout follows the 2010
// retail DWARF declarations (common/blobulator/Implicit/ImpParticle.h and the
// interpolant corner types in UserFunctions.h).
//
// center.w and scale both hold the particle's field scale: the radius scale of
// its field (1 = the tiler's render and cutoff radii). The renderer rewrites
// center with SetWSIMD() to move a particle while keeping its scale.
//
//=============================================================================//

#ifndef BLOBULATOR_IMPPARTICLE_H
#define BLOBULATOR_IMPPARTICLE_H
#ifdef _WIN32
#pragma once
#endif

#include "blobulator/Point3D.h"

struct ALIGN16 ImpParticle
{
	Point3D center;
	float scale;

	ImpParticle() : scale( 1.0f ) {}

	void setFieldScale( float flScale )
	{
		scale = flScale;
		center[3] = flScale;
	}

	// 1 / scale^2: multiplies squared distances into the unit field's space.
	float GetFieldRScaleSq() const { return ( scale > 0.0f ) ? 1.0f / ( scale * scale ) : 0.0f; }
} ALIGN16_POST;

// interpolants1: per-particle color (rgb) blended over the surface.
struct ALIGN16 ImpParticleWithOneInterpolant : public ImpParticle
{
	Point3D interpolants1;
} ALIGN16_POST;

struct ALIGN16 ImpParticleWithTwoInterpolants : public ImpParticleWithOneInterpolant
{
	Point3D interpolants2;
} ALIGN16_POST;

// interpolants2-4: a per-particle frame (the renderer sets outward direction,
// tangent and bitangent) blended into the surface tangent frame.
struct ALIGN16 ImpParticleWithFourInterpolants : public ImpParticleWithTwoInterpolants
{
	Point3D interpolants3;
	Point3D interpolants4;
} ALIGN16_POST;

#endif // BLOBULATOR_IMPPARTICLE_H
