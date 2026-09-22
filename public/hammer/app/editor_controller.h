//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Headless editor interaction authority for the Hammer shell (RFC 0002,
//			hammer.app — the H3/H4 "one selection/mutation/history owner"). It owns
//			the editable map and the core Hammer UX flows -- the Block tool (drag a
//			rectangle in a 2D view, commit an extruded brush), the Selection tool
//			(pick, move, delete), grid snapping, and undo/redo -- expressed over
//			*normalized* input, with NO GTK/GDK/GPU dependency. The GTK host is a
//			thin presenter that translates gestures into these calls; the same
//			calls are what the conformance UI tests drive with simulated input, so
//			the tested logic and the shipped logic are identical.
//
//			A brush is stored as its set of half-space planes: the Block tool makes
//			an axis-aligned box, while a VMF loaded here keeps each solid's real side
//			planes, so arbitrary convex brush shapes are preserved, rendered, and
//			round-tripped -- not collapsed to a bounding box. An axis-aligned bound
//			is cached alongside for picking and grid math only.
//
//=============================================================================//

#ifndef HAMMER_APP_EDITOR_CONTROLLER_H
#define HAMMER_APP_EDITOR_CONTROLLER_H

#include "hammer/app/document_history.h"
#include "hammer/geometry/brush.h"

#include <optional>
#include <string>
#include <vector>

namespace hammer::app
{

enum class Tool
{
	Select,
	Block,
};

// The four viewports. The camera view is navigation-only (no editing input); the
// three orthographic views map to the world-axis pair the pointer edits.
enum class ViewId
{
	Camera,
	Top,   // X / Y (free axis Z)
	Front, // X / Z (free axis Y)
	Side,  // Y / Z (free axis X)
};

// One editable brush. Its true shape is the convex intersection of 'planes' (the
// Block tool makes an axis-aligned box; a loaded VMF brush keeps its real side
// planes, so arbitrary brush shapes are preserved and rendered, not collapsed to a
// box). 'materials' is parallel to 'planes' when known. 'mins'/'maxs' are a cached
// axis-aligned bound used for picking, grid math and framing, not the shape.
struct MapBrush
{
	int id = 0;
	std::vector<geometry::Plane> planes;
	std::vector<std::string> materials;
	geometry::Vec3d mins;
	geometry::Vec3d maxs;
};

// The synthetic solid id BuildScene() gives the in-progress Block-tool rectangle,
// so a presenter can draw it distinctly. Real brushes have positive ids.
constexpr int kPendingSolidId = -1;

class EditorController
{
public:
	EditorController();

	// --- Document lifecycle -------------------------------------------------
	void NewMap();
	// Reconstructs editable boxes from a VMF's solids (each solid -> its AABB).
	bool LoadVmf( const std::string &vmfText, std::string &error );
	// Serialises the current map to a VMF document (versioninfo + world solids).
	std::string ToVmf() const;

	bool IsModified() const { return m_history.IsModified(); }
	void MarkSaved() { m_history.MarkSaved(); }

	// --- Tool / grid state --------------------------------------------------
	void SetTool( Tool tool );
	Tool CurrentTool() const { return m_tool; }
	void SetGridSize( int grid );
	int GridSize() const { return m_grid; }
	// The extrusion depth (on a 2D view's free axis) new Block brushes get.
	void SetBlockDepth( double depth ) { m_blockDepth = depth > 1.0 ? depth : 1.0; }
	double BlockDepth() const { return m_blockDepth; }

	// --- Normalized pointer input (world coords in the view's two axes) ------
	// Camera-view input is ignored here (navigation is the presenter's concern).
	void PointerDown( ViewId view, double u, double v );
	void PointerDrag( ViewId view, double u, double v );
	void PointerUp( ViewId view, double u, double v );

	// --- Commands (menu / keyboard / test) ----------------------------------
	// Commits the pending Block rectangle into a brush. Returns true if created.
	bool Commit();
	// Removes the selected brush. Returns true if one was deleted.
	bool DeleteSelection();
	bool Undo();
	bool Redo();
	bool CanUndo() const { return m_history.CanUndo(); }
	bool CanRedo() const { return m_history.CanRedo(); }

	// --- State for the presenter / tests ------------------------------------
	const std::vector<MapBrush> &Brushes() const { return m_brushes; }
	std::optional<int> Selection() const { return m_selection; }
	bool HasPending() const { return m_pending.active; }
	// The pending box as a full 3D AABB (with the free-axis extrusion), if any.
	bool PendingBox( geometry::Vec3d &mins, geometry::Vec3d &maxs ) const;

	// The renderable scene: every brush plus the pending box (id kPendingSolidId),
	// built through the shared geometry bridge so rendering reuses one path.
	geometry::WorldScene BuildScene() const;

	// Grid-snaps a single world coordinate through the shared rounding owner.
	double Snap( double value ) const;

private:
	struct Pending
	{
		bool active = false;
		ViewId view = ViewId::Top;
		double u0 = 0.0;
		double v0 = 0.0;
		double u1 = 0.0;
		double v1 = 0.0;
	};

	struct DocState
	{
		std::vector<MapBrush> brushes;
		int nextId = 1;
	};

	static void ViewAxes( ViewId view, int &uAxis, int &vAxis, int &freeAxis );
	void ResetHistory();
	void PushSnapshot();           // records current brushes as a new history unit
	void LoadSnapshotAtPosition(); // restores brushes from the current history slot
	MapBrush *FindBrush( int id );
	const MapBrush *FindBrush( int id ) const;
	// Assembles a box AABB from a pending rectangle in its view.
	void PendingToAabb( geometry::Vec3d &mins, geometry::Vec3d &maxs ) const;

	std::vector<MapBrush> m_brushes;
	int m_nextId = 1;
	std::optional<int> m_selection;

	Tool m_tool = Tool::Select;
	int m_grid = 64;
	double m_blockDepth = 128.0;
	std::string m_defaultMaterial = "DEV/DEV_MEASUREGENERIC01B";

	Pending m_pending;
	// Selection-drag bookkeeping.
	bool m_dragging = false;
	double m_dragStartU = 0.0;
	double m_dragStartV = 0.0;
	MapBrush m_dragOrig;
	bool m_dragMoved = false;

	DocumentHistory m_history;
	std::vector<DocState> m_snapshots;
};

} // namespace hammer::app

#endif // HAMMER_APP_EDITOR_CONTROLLER_H
