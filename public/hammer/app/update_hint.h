//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Independent document-change aggregation model (RFC 0002, hammer.app).
//			This is the reusable core of the legacy MFC `CUpdateHint`: when the
//			document is mutated, callers collect which nodes changed and how
//			(a notify code), plus the world-space region affected, and hand that
//			aggregate to the view-update step. The strict model owns only the
//			aggregation POLICY -- bucketing changed nodes by notify code with a
//			bounded number of codes, and unioning the affected bounds region -- so
//			it can be tested headlessly and driven by either UI sibling.
//
//			It composes the two already-extracted strict libraries: it references
//			nodes by `hammer::scene::NodeHandle` (not `CMapClass *`) and
//			accumulates the affected region as a `hammer::geometry::AxisAlignedBox`
//			(not `BoundBox`). No MFC, tier0, PCH, or GPU dependency. The
//			`CMapClass`/`BoundBox` marshalling stays in the MFC adapter shell.
//
//=============================================================================//

#ifndef HAMMER_APP_UPDATE_HINT_H
#define HAMMER_APP_UPDATE_HINT_H

#include "hammer/geometry/aabb.h"
#include "hammer/scene/handle.h"

#include <vector>

namespace hammer::app
{

// The maximum number of distinct notify codes an update hint can carry, preserved
// from the legacy `CUpdateHint::MAX_NOTIFY_CODES`. A change touching more than this
// many distinct codes is a caller error; the extra codes are rejected rather than
// silently overflowing (see PostUpdate).
inline constexpr int kMaxNotifyCodes = 16;

//-----------------------------------------------------------------------------
// Purpose: Aggregates the nodes changed by a document mutation, grouped by notify
//			code, together with the affected world-space region.
//-----------------------------------------------------------------------------
class UpdateHint
{
public:
	// One notify-code bucket: the code plus the nodes reported under it, in the
	// order they were reported.
	struct NotifyBucket
	{
		int code = 0;
		std::vector<scene::NodeHandle> nodes;
	};

	UpdateHint();

	// Clears all buckets and resets the affected region to the invalid/empty box,
	// so the next PreUpdate/PostUpdate seeds it exactly. Equivalent to the legacy
	// CUpdateHint::Reset.
	void Reset();

	// Accumulates a pre-change region without recording a node or code. Used to
	// capture the extent an object occupied BEFORE it was modified/moved, so the
	// affected region covers both the old and new positions.
	void PreUpdate( const geometry::AxisAlignedBox &affectedBounds );

	// Records that `node` changed with notification `notifyCode`, and unions
	// `affectedBounds` into the region. Nodes reported with the same code share a
	// bucket. Returns false and records nothing if `notifyCode` is a new code and
	// all kMaxNotifyCodes buckets are already in use (capacity limit); an existing
	// code always succeeds.
	bool PostUpdate( scene::NodeHandle node, int notifyCode,
					 const geometry::AxisAlignedBox &affectedBounds );

	// The union of every region passed to PreUpdate/PostUpdate since the last
	// Reset. Invalid (IsValidBox() == false) when nothing has been reported.
	const geometry::AxisAlignedBox &UpdateRegion() const { return m_updateRegion; }

	// Number of distinct notify codes currently held (0..kMaxNotifyCodes).
	int NotifyCodeCount() const { return static_cast<int>( m_buckets.size() ); }

	// The notify code / nodes of bucket `bucketIndex` in [0, NotifyCodeCount()).
	int NotifyCodeAt( int bucketIndex ) const { return m_buckets[bucketIndex].code; }
	const std::vector<scene::NodeHandle> &NodesAt( int bucketIndex ) const
	{
		return m_buckets[bucketIndex].nodes;
	}

private:
	// Returns the bucket for `code`, allocating one if room; nullptr when a new
	// code is requested and all buckets are in use.
	NotifyBucket *BucketFor( int code );

	std::vector<NotifyBucket> m_buckets;
	geometry::AxisAlignedBox m_updateRegion;
};

} // namespace hammer::app

#endif // HAMMER_APP_UPDATE_HINT_H
