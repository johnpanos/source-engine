//========= Portal 2 port ======================================================//
//
// Purpose: Blobulator tile renderer bookkeeping.
//
// Clean-room implementation of the blobulator interface the Portal 2 paint
// blob renderer uses (not Valve's blobulator library). In the retail library
// every in-flight tiler owns a tile renderer whose triangles occupy a shared
// fragment vertex buffer; the paint renderer asks how many are acquired to
// decide whether a deferred (two-pass) draw still fits. Here each tiler keeps
// its own triangles, and the factory reports the tilers that are acquired and
// not yet returned, which is the same quantity.
//
//=============================================================================//

#ifndef BLOBULATOR_IMPRENDERER_H
#define BLOBULATOR_IMPRENDERER_H
#ifdef _WIN32
#pragma once
#endif

#include "tier0/threadtools.h"

class ImpRendererFactory
{
public:
	static ImpRendererFactory *s_pInstance;

	int GetNumberOfAcquiredTileRenderers() const { return m_nAcquired; }

	// Called by ImpTilerFactory when a tiler is handed out / returned.
	void OnTileRendererAcquired() { ++m_nAcquired; }
	void OnTileRendererReleased()
	{
		Assert( m_nAcquired > 0 );
		--m_nAcquired;
	}

	ImpRendererFactory() : m_nAcquired( 0 ) {}

private:
	CInterlockedInt m_nAcquired;
};

#endif // BLOBULATOR_IMPRENDERER_H
