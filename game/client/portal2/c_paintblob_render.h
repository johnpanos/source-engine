//========= Portal 2 reconstruction ============================================//
//
// Purpose: Paint blob rendering (blobulator isosurface and fast spheres).
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#ifndef C_PAINTBLOB_RENDER_H
#define C_PAINTBLOB_RENDER_H

#ifdef _WIN32
#pragma once
#endif

#include "utlvector.h"
#include "mathlib/vector4d.h"
#include "blobulator/Implicit/ImpParticle.h"

class IMaterial;
class IClientRenderable;

// World transforms of the portals touching a group of blobs
typedef CUtlVector< matrix3x4_t > PortalMatrixList_t;

// The particles handed to the blob renderer for one draw
typedef CUtlVector< ImpParticleWithFourInterpolants, CUtlMemoryAligned< ImpParticleWithFourInterpolants, 16 > > BlobParticles_t;

extern BlobParticles_t g_PaintblobRenderParticles;

namespace NPaintRenderer
{
	// Draws the particles in g_PaintblobRenderParticles with the given render mode (BlobRenderMode_t)
	void Paintblob_Draw( int renderMode, IClientRenderable *pClientRenderable, const Vector &vecRenderOrigin, IMaterial *pMaterial, float flCubeWidth, const PortalMatrixList_t &portalMatrixList, bool bSurfaceNoParticleCull );

	// Computes the ambient light cube at vecRenderOrigin, keeping the previous
	// colors when the new sample is invalid
	void Paintblob_SafeLightCubeUpdate( const Vector &vecRenderOrigin, Vector4D *cachedCubeColours );
}

#endif // C_PAINTBLOB_RENDER_H
