//========= Portal 2 reconstruction ============================================//
//
// Purpose: Button whose clickable and visible area is a resource-defined polygon
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#ifndef POLYGONBUTTON_H
#define POLYGONBUTTON_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Button.h>
#include "vgui/ISurface.h"

class CPolygonButton : public vgui::Button
{
	DECLARE_CLASS_SIMPLE( CPolygonButton, vgui::Button );

public:

	CPolygonButton( vgui::Panel *parent, const char *panelName );

	virtual void ApplySettings( KeyValues *data );

	// Hit-tests against the hotspot polygon instead of the panel's bounding box.
	// (The hotspot is expected to be convex and wound clockwise in screen space.)
	virtual vgui::VPANEL IsWithinTraverse( int x, int y, bool traversePopups );

	// Scales the unscaled [0..1] hotspot points to the current panel size and
	// recomputes the visible hotspot's bounds.
	virtual void PerformLayout( void );

	virtual void ComputeAlignment( int &tx0, int &ty0, int &tx1, int &ty1 );
	virtual void PaintBackground( void );
	virtual void PaintBorder( void );
	virtual void ApplySchemeSettings( vgui::IScheme *scheme );

	virtual void UpdateHotspots( KeyValues *data );

protected:
	int m_nWhiteMaterial;

	CUtlVector< Vector2D > m_unscaledHotspotPoints;
	CUtlVector< Vector2D > m_unscaledVisibleHotspotPoints;
	vgui::Vertex_t *m_hotspotPoints;
	int m_numHotspotPoints;
	vgui::Vertex_t *m_visibleHotspotPoints;
	int m_numVisibleHotspotPoints;

	Vector2D m_hotspotMins;
	Vector2D m_hotspotMaxs;

	// Reconstruction note: both 2010 builds use the implicit destructor, which only
	// releases the two vectors; the scaled vertex arrays are not freed there.
};

#endif // POLYGONBUTTON_H
