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
	Entity,   // a click in a 2D view places a point entity of the active class
	Material, // a click on a brush applies the active material to all its faces
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

// One authored key/value on a placed entity beyond its classname and origin
// (e.g. "targetname", "angles"). Kept as an ordered list so a loaded entity
// round-trips its full keyvalue set, not just the two fields the editor knows.
struct EntityProperty
{
	std::string key;
	std::string value;
};

// One placed point entity (e.g. info_player_start, weapon_portalgun). 'origin' is
// the world position the Entity tool snapped the placement click to; 'properties'
// carries every other authored key so the entity round-trips faithfully. Brush
// entities are a later migration; this slice owns point entities.
struct MapEntity
{
	int id = 0;
	std::string classname;
	geometry::Vec3d origin;
	std::vector<EntityProperty> properties;
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

	// The classname the Entity tool places (default "info_player_start").
	void SetEntityClass( const std::string &classname );
	const std::string &EntityClass() const { return m_entityClass; }

	// The material the Material tool applies and that new Block brushes receive.
	void SetActiveMaterial( const std::string &material );
	const std::string &ActiveMaterial() const { return m_defaultMaterial; }

	// --- Normalized pointer input (world coords in the view's two axes) ------
	// Camera-view input is ignored here (navigation is the presenter's concern).
	// 'additive' (Ctrl-click in MFC) toggles the clicked brush in/out of a multi-
	// selection instead of replacing it; the default is single-select.
	void PointerDown( ViewId view, double u, double v, bool additive = false );
	void PointerDrag( ViewId view, double u, double v );
	void PointerUp( ViewId view, double u, double v );

	// --- 3D-view click selection --------------------------------------------
	// Ray-picks the nearest brush a world-space ray enters (AABB broad phase, then a
	// precise ray/convex-polytope test on the brush's real faces) and makes it the
	// selection, so the camera view can click-select the same way a 2D click does.
	// 'additive' (Ctrl-click) toggles the hit brush in/out of the multi-selection; a
	// non-additive ray that hits nothing clears the selection, mirroring an empty 2D
	// click. Mutates only the selection (no document change, so no undo step).
	// Returns true if a brush was hit. 'dir' need not be normalized.
	bool PickByRay(
	    const geometry::Vec3d &origin, const geometry::Vec3d &dir, bool additive = false );

	// Translates the selected brush rigidly by a world delta (every face plane and
	// the cached bound shift together), recording one undo unit -- the keyboard
	// "nudge" of the selection (arrow keys in a 2D view). Returns true if a brush
	// was selected and moved. No-op with nothing selected.
	bool MoveSelectionBy( double dx, double dy, double dz );

	// --- Commands (menu / keyboard / test) ----------------------------------
	// Commits the pending Block rectangle into a brush. Returns true if created.
	bool Commit();
	// Removes the selected brush or entity. Returns true if one was deleted.
	bool DeleteSelection();
	// Applies the active material to every face of the selected brush. Returns true
	// (recording one history unit) iff a brush was selected and its faces changed.
	// The Material tool's click routes through the same retexture path.
	bool ApplyActiveMaterialToSelection();
	// Sets/updates key='value' on the placed entity 'entityId', adding the key where
	// absent. 'classname'/'origin' are edited through their own owners, not here.
	// Records one history unit iff the value actually changed. Returns whether it did.
	bool SetEntityProperty( int entityId, const std::string &key, const std::string &value );

	// --- Exact domain operations ----------------------------------------------
	// The same authority the pointer gestures use, addressed by value so a
	// command script, a UI-driven test or an MCP client can author a map without
	// synthesizing gestures. Each successful call is one undo unit.
	//
	// Adds an axis-aligned world brush with the active material and selects it.
	// Returns its id, or nothing for a degenerate box (any extent <= 0).
	std::optional<int> CreateBlock( const geometry::Vec3d &mins, const geometry::Vec3d &maxs );
	// Places a point entity of 'classname' at 'origin' and selects it.
	std::optional<int> PlaceEntity( const std::string &classname, const geometry::Vec3d &origin );
	bool SetEntityOrigin( int entityId, const geometry::Vec3d &origin );
	// Worldspawn keyvalues other than classname/mapversion (e.g. "skyname"). A new
	// map starts with skyname "sky_day01_01" so it compiles without a leak into
	// an unset sky.
	bool SetWorldProperty( const std::string &key, const std::string &value );
	const std::vector<EntityProperty> &WorldProperties() const { return m_worldProperties; }
	bool Undo();
	bool Redo();
	bool CanUndo() const { return m_history.CanUndo(); }
	bool CanRedo() const { return m_history.CanRedo(); }

	// --- State for the presenter / tests ------------------------------------
	const std::vector<MapBrush> &Brushes() const { return m_brushes; }
	const std::vector<MapEntity> &Entities() const { return m_entities; }
	// The primary (last-picked) selected brush, if any. With a multi-selection this
	// is the most-recently clicked member; Selections() returns the whole set.
	std::optional<int> Selection() const { return m_selection; }
	// Every selected brush id (the primary plus any Ctrl-added extras), in no
	// guaranteed order. Empty when nothing is selected.
	std::vector<int> Selections() const;
	bool IsSelected( int id ) const;
	std::size_t SelectionCount() const { return m_selection ? m_extraSelected.size() + 1 : 0; }
	std::optional<int> SelectedEntity() const { return m_entitySelection; }
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
		std::vector<MapEntity> entities;
		std::vector<EntityProperty> worldProperties;
		int nextId = 1;
	};

	static void ViewAxes( ViewId view, int &uAxis, int &vAxis, int &freeAxis );
	void ResetHistory();
	void PushSnapshot();           // records current brushes+entities as a history unit
	void LoadSnapshotAtPosition(); // restores brushes+entities from the current slot
	MapBrush *FindBrush( int id );
	const MapBrush *FindBrush( int id ) const;
	MapEntity *FindEntity( int id );
	// Updates the brush selection for a click on brush 'id': non-additive replaces
	// the whole selection with 'id'; additive (Ctrl) toggles 'id' in/out of the
	// multi-selection, keeping the most-recent member primary. Clears any entity
	// selection. Selection-only (no history).
	void ApplyBrushPick( int id, bool additive );
	void ClearBrushSelection(); // primary + extras
	// Retextures every face of brush 'id' with the active material; returns true iff
	// the brush existed and any face material changed (caller records the history).
	bool RetextureBrush( int id );
	// Assembles a box AABB from a pending rectangle in its view.
	void PendingToAabb( geometry::Vec3d &mins, geometry::Vec3d &maxs ) const;

	std::vector<MapBrush> m_brushes;
	std::vector<MapEntity> m_entities;
	// Loaded displacement (dispinfo terrain) surfaces, carried for display only
	// (not editable in this slice); BuildScene() emits them for the viewport.
	std::vector<geometry::DisplacementMesh> m_displacements;
	int m_nextId = 1;
	std::optional<int> m_selection;       // primary (last-picked) selected brush id
	std::vector<int> m_extraSelected;     // additional selected brushes (Ctrl-click)
	std::optional<int> m_entitySelection; // selected entity id, if any

	Tool m_tool = Tool::Select;
	int m_grid = 64;
	double m_blockDepth = 128.0;
	std::string m_defaultMaterial = "DEV/DEV_MEASUREGENERIC01B";
	std::string m_entityClass = "info_player_start";

	Pending m_pending;
	// Selection-drag bookkeeping.
	bool m_dragging = false;
	double m_dragStartU = 0.0;
	double m_dragStartV = 0.0;
	std::vector<MapBrush> m_dragOrigs; // originals of all brushes being drag-moved
	bool m_dragMoved = false;

	std::vector<EntityProperty> m_worldProperties;

	DocumentHistory m_history;
	std::vector<DocState> m_snapshots;
};

} // namespace hammer::app

#endif // HAMMER_APP_EDITOR_CONTROLLER_H
