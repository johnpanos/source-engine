//========= Portal 2 reconstruction ============================================//
//
// Purpose: Smoothed histogram panel used by the leaderboard graphs
//          (SplineGraphPanel, "FriendGraph" / "GlobalGraph" in
//          resource/ui/portal_leaderboard_graph_panel.res).
//
// Reconstructed from the retail class and convar names (SplineGraphPanel,
// cl_splinegraph_bucket) and the .res layout; the drawing is not original
// Valve source. The repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef SPLINEGRAPHPANEL_H
#define SPLINEGRAPHPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>
#include "tier1/utlvector.h"

//-----------------------------------------------------------------------------
// Draws the distribution of a set of samples between a minimum and maximum as
// a filled curve: the samples are counted into cl_splinegraph_bucket buckets,
// normalized to the tallest bucket, and a Catmull-Rom spline through the
// bucket centers is filled down to the bottom edge in the foreground color.
//-----------------------------------------------------------------------------
class SplineGraphPanel : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( SplineGraphPanel, vgui::Panel );

public:
	SplineGraphPanel( vgui::Panel *pParent, const char *pName );

	// Replace the samples and the value range they are bucketed over.
	void SetData( const CUtlVector< float > &samples, float flMin, float flMax );
	void ClearData();
	bool HasData() const { return m_Samples.Count() > 0; }

	virtual void Paint();

private:
	void DrawFilledQuad( float x0, float y0, float x1, float y1, float flBottom );

	CUtlVector< float > m_Samples;
	float m_flMin;
	float m_flMax;
	int m_nWhiteTexture;
};

#endif // SPLINEGRAPHPANEL_H
