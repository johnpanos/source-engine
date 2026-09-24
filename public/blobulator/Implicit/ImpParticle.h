#ifndef IMPPARTICLE_H
#define IMPPARTICLE_H

#include "mathlib/vector.h"

struct ImpParticleWithFourInterpolants
{
	Vector m_vPos;
	float m_flInterp[4];
};

#endif
