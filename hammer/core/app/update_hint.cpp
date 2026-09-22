//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer::app::UpdateHint (RFC 0002). See
//			public/hammer/app/update_hint.h. Reusable core of the legacy MFC
//			CUpdateHint aggregation, built on the strict geometry and scene
//			libraries with no MFC/tier0/PCH/GPU dependency.
//
//=============================================================================//

#include "hammer/app/update_hint.h"

namespace hammer::app
{

UpdateHint::UpdateHint()
{
	Reset();
}

void UpdateHint::Reset()
{
	m_buckets.clear();
	m_updateRegion.ResetBounds();
}

void UpdateHint::PreUpdate( const geometry::AxisAlignedBox &affectedBounds )
{
	m_updateRegion.UpdateBounds( affectedBounds );
}

UpdateHint::NotifyBucket *UpdateHint::BucketFor( int code )
{
	for ( NotifyBucket &bucket : m_buckets )
	{
		if ( bucket.code == code )
		{
			return &bucket;
		}
	}

	// New code: allocate a bucket only if we are under the capacity limit.
	if ( static_cast<int>( m_buckets.size() ) >= kMaxNotifyCodes )
	{
		return nullptr;
	}

	m_buckets.push_back( NotifyBucket{ code, {} } );
	return &m_buckets.back();
}

bool UpdateHint::PostUpdate( scene::NodeHandle node, int notifyCode,
							 const geometry::AxisAlignedBox &affectedBounds )
{
	NotifyBucket *bucket = BucketFor( notifyCode );
	if ( bucket == nullptr )
	{
		// Capacity exceeded by a new code: record nothing, matching the legacy
		// reject-and-return path (which never touched bounds or the object list).
		return false;
	}

	bucket->nodes.push_back( node );
	m_updateRegion.UpdateBounds( affectedBounds );
	return true;
}

} // namespace hammer::app
