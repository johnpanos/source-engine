//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The world's render scene and its culling capture (RFC 0016 K5); see
// render_core_world.h.
//
//=============================================================================//

#include "render_core_world.h"

#include "filesystem.h"
#include "filesystem_engine.h"
#include "gl_model_private.h"
#include "gl_rmain.h"
#include "host.h"
#include "render_core_host.h"
#include "render/composition/render_core_world.h"
#include "staticpropmgr.h"
#include "tier0/dbg.h"
#include "tier1/convar.h"
#include "tier1/utlbuffer.h"
#include "tier1/utlstring.h"

#include <algorithm>
#include <iterator>
#include <vector>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace
{

// One world list of a 3D view: its leaves and planes, and the props the view
// drew after it (a view may build several, as portal recursion does).
struct ViewRecord
{
	std::vector<int> leaves;
	float planes[FRUSTUM_NUMPLANES * 4] = {};
	float origin[3] = {};
	bool forcedLeaf = false;
	bool visOverride = false;
	bool waterReflection = false;
	std::vector<int> props;
};

struct CaptureState
{
	CUtlString file;
	CUtlString tag;
	bool pending = false;
	bool capturing = false;
	CUtlBuffer records{ 0, 0, CUtlBuffer::TEXT_BUFFER };
	int views = 0;
	std::vector<std::vector<ViewRecord>> stack; // per open 3D view, its world lists
};

CaptureState &Capture()
{
	static CaptureState s_State;
	return s_State;
}

// The box the legacy traversal (R_RecursiveWorldNode) last tests for a leaf:
// its own, unless it lies in a subtree marked too small to cull (contents -2,
// MarkSmallNode), whose top node's parent is then the last one tested.
void CullBox( const mleaf_t &leaf, Vector &center, Vector &halfDiagonal )
{
	center = leaf.m_vecCenter;
	halfDiagonal = leaf.m_vecHalfDiagonal;
	const mnode_t *top = NULL;
	for ( const mnode_t *node = leaf.parent; node; node = node->parent )
	{
		if ( node->contents == -2 )
			top = node;
	}
	if ( top && top->parent )
	{
		center = top->parent->m_vecCenter;
		halfDiagonal = top->parent->m_vecHalfDiagonal;
	}
}

// R_CullNodeInternal's test against every plane of a frustum: whether the
// box lies wholly behind one of them.
bool OutsideFrustum( const Frustum_t &frustum, const Vector &center, const Vector &halfDiagonal )
{
	for ( int i = 0; i < FRUSTUM_NUMPLANES; ++i )
	{
		const cplane_t *plane = frustum.GetPlane( i );
		const float centerDotNormal = DotProduct( center, plane->normal ) - plane->dist;
		const float halfDotAbsNormal = DotProduct( halfDiagonal, frustum.GetAbsNormal( i ) );
		if ( centerDotNormal + halfDotAbsNormal < 0.0f )
			return true;
	}
	return false;
}

void AppendList( CUtlBuffer &out, const char *name, const std::vector<int> &values )
{
	out.Printf( ",\"%s\":[", name );
	for ( size_t i = 0; i < values.size(); ++i )
		out.Printf( "%s%d", i ? "," : "", values[i] );
	out.PutChar( ']' );
}

} // namespace

void RenderCoreWorld_LevelInit()
{
	worldbrushdata_t *brush = host_state.worldbrush;
	if ( !brush || !brush->leafs )
		return;
	std::vector<float> boxes;
	std::vector<int> codes;
	for ( int leaf = 0; leaf < brush->numleafs; ++leaf )
	{
		const mleaf_t &data = brush->leafs[leaf];
		if ( data.contents & CONTENTS_SOLID )
			continue;
		Vector center, half;
		CullBox( data, center, half );
		const float box[6] = { center.x - half.x, center.y - half.y, center.z - half.z,
		    center.x + half.x, center.y + half.y, center.z + half.z };
		boxes.insert( boxes.end(), box, box + 6 );
		codes.push_back( leaf );
	}
	// The static props are the core's own scene, committed with its models
	// (IRenderCoreWorld::SetStaticProps).
	if ( IRenderCoreWorld *world = RenderCoreHost_World(); world && !codes.empty() )
		world->SetWorldLeaves( boxes.data(), codes.data(), unsigned( codes.size() ) );
}

bool RenderCoreWorld_Capturing()
{
	return Capture().capturing;
}

void RenderCoreWorld_BeginFrame()
{
	CaptureState &capture = Capture();
	capture.stack.clear();
	if ( capture.pending )
	{
		capture.pending = false;
		capture.capturing = true;
		capture.views = 0;
	}
}

void RenderCoreWorld_EndFrame()
{
	CaptureState &capture = Capture();
	if ( !capture.capturing )
		return;
	capture.capturing = false;
	capture.stack.clear();
	if ( !g_pFileSystem->WriteFile( capture.file.Get(), "DEFAULT_WRITE_PATH", capture.records ) )
		Warning( "r_core_cull_capture: cannot write %s\n", capture.file.Get() );
	else
		Msg( "r_core_cull_capture: %d views in %s\n", capture.views, capture.file.Get() );
	capture.records.Clear();
}

static int s_nViewDepth = 0;

int RenderCoreWorld_ViewDepth()
{
	return s_nViewDepth;
}

void RenderCoreWorld_ViewBegin()
{
	++s_nViewDepth;
	CaptureState &capture = Capture();
	if ( capture.capturing )
		capture.stack.emplace_back();
}

void RenderCoreWorld_OnWorldList( const unsigned short *pLeaves, int nLeafCount,
    const float *pOrigin, bool bForcedLeaf, bool bVisOverride, bool bWaterReflection )
{
	CaptureState &capture = Capture();
	if ( !capture.capturing || capture.stack.empty() )
		return;
	capture.stack.back().emplace_back();
	ViewRecord &view = capture.stack.back().back();
	view.leaves.assign( pLeaves, pLeaves + MAX( 0, nLeafCount ) );
	for ( int i = 0; i < FRUSTUM_NUMPLANES; ++i )
	{
		// The legacy view frustum, as R_RecursiveWorldNode culls with it.
		const cplane_t *plane = g_Frustum.GetPlane( i );
		view.planes[i * 4 + 0] = plane->normal.x;
		view.planes[i * 4 + 1] = plane->normal.y;
		view.planes[i * 4 + 2] = plane->normal.z;
		view.planes[i * 4 + 3] = plane->dist;
	}
	view.origin[0] = pOrigin[0];
	view.origin[1] = pOrigin[1];
	view.origin[2] = pOrigin[2];
	view.forcedLeaf = bForcedLeaf;
	view.visOverride = bVisOverride;
	view.waterReflection = bWaterReflection;
}

void RenderCoreWorld_OnStaticPropsDrawn( const int *pProps, int nCount )
{
	CaptureState &capture = Capture();
	if ( capture.capturing && !capture.stack.empty() && !capture.stack.back().empty() )
	{
		std::vector<int> &props = capture.stack.back().back().props;
		props.insert( props.end(), pProps, pProps + nCount );
	}
}

namespace
{

void WriteViewRecord( const ViewRecord &view )
{
	CaptureState &capture = Capture();
	worldbrushdata_t *brush = host_state.worldbrush;
	if ( !brush )
		return;
	std::vector<int> legacyLeaves = view.leaves;
	std::sort( legacyLeaves.begin(), legacyLeaves.end() );
	std::vector<int> legacyProps = view.props;
	std::sort( legacyProps.begin(), legacyProps.end() );
	legacyProps.erase( std::unique( legacyProps.begin(), legacyProps.end() ), legacyProps.end() );
	const int propCount = StaticPropMgr_CorePropCount();
	std::vector<unsigned char> visibleLeaf( brush->numleafs, 0 ),
	    visibleProp( MAX( 1, propCount ), 0 );
	for ( int leaf : legacyLeaves )
	{
		if ( leaf >= 0 && leaf < brush->numleafs )
			visibleLeaf[leaf] = 1;
	}
	for ( int prop : legacyProps )
	{
		if ( prop >= 0 && prop < propCount )
			visibleProp[prop] = 1;
	}

	// The core's culling, with the view's planes and the legacy visibility
	// (the leaves and props of its scenes: what DrawView draws them from).
	std::vector<int> coreLeaves( MAX( 1, brush->numleafs ) ), coreProps( MAX( 1, propCount ) );
	IRenderCoreWorld::ViewCull cull{ view.planes, visibleLeaf.data(), unsigned( brush->numleafs ),
	    visibleProp.data(), unsigned( propCount ), coreLeaves.data(), coreProps.data() };
	IRenderCoreWorld *world = RenderCoreHost_World();
	const bool live = world && world->CullView( cull );
	coreLeaves.resize( cull.drawnLeafCount );
	coreProps.resize( cull.drawnPropCount );
	const int frustumCulled = live ? int( cull.frustumCulled ) : -1;
	const int providerCulled = live ? int( cull.providerCulled ) : -1;
	const int pooledEqual = live ? cull.pooledEqual : -1;
	std::sort( coreLeaves.begin(), coreLeaves.end() );
	std::sort( coreProps.begin(), coreProps.end() );
	auto difference = []( const std::vector<int> &a, const std::vector<int> &b )
	{
		std::vector<int> out;
		std::set_difference( a.begin(), a.end(), b.begin(), b.end(), std::back_inserter( out ) );
		return out;
	};
	const std::vector<int> missing = difference( legacyLeaves, coreLeaves );
	const std::vector<int> extra = difference( coreLeaves, legacyLeaves );
	const std::vector<int> missingProps = difference( legacyProps, coreProps );
	const std::vector<int> extraProps = difference( coreProps, legacyProps );

	// g_Frustum now holds the enclosing view's planes; judge the missing
	// items against the planes this view culled with.
	Frustum_t viewFrustum;
	for ( int i = 0; i < FRUSTUM_NUMPLANES; ++i )
	{
		const Vector normal( view.planes[i * 4], view.planes[i * 4 + 1], view.planes[i * 4 + 2] );
		viewFrustum.SetPlane( i, PLANE_ANYZ, normal, view.planes[i * 4 + 3] );
	}

	CUtlBuffer &out = capture.records;
	out.Printf( "{\"schema\":\"source-core-culling/v2\",\"tag\":\"%s\",\"view\":%d,"
	            "\"origin\":[%.1f,%.1f,%.1f],\"live\":%s,\"forced_leaf\":%s,"
	            "\"vis_override\":%s,\"water_reflection\":%s,\"instances\":%d,"
	            "\"legacy\":%d,\"core\":%d,\"legacy_props\":%d,\"core_props\":%d,"
	            "\"frustum_culled\":%d,\"provider_culled\":%d,\"pooled_equal\":%d",
	    capture.tag.Get(), capture.views++, view.origin[0], view.origin[1], view.origin[2],
	    live ? "true" : "false", view.forcedLeaf ? "true" : "false",
	    view.visOverride ? "true" : "false", view.waterReflection ? "true" : "false",
	    int( brush->numleafs ) + propCount, int( legacyLeaves.size() ), int( coreLeaves.size() ),
	    int( legacyProps.size() ), int( coreProps.size() ), frustumCulled, providerCulled,
	    pooledEqual );
	AppendList( out, "missing", missing );
	AppendList( out, "extra", extra );
	AppendList( out, "missing_props", missingProps );
	AppendList( out, "extra_props", extraProps );
	// Each missing leaf's area, and whether legacy's own test puts its box
	// outside this view's frustum (legacy tests a leaf in an area it sees
	// through an area portal against that area's frustum instead).
	out.PutString( ",\"missing_detail\":[" );
	for ( size_t i = 0; i < missing.size(); ++i )
	{
		const mleaf_t &leaf = brush->leafs[missing[i]];
		Vector center, half;
		CullBox( leaf, center, half );
		out.Printf( "%s{\"leaf\":%d,\"area\":%d,\"outside_view\":%s}", i ? "," : "", missing[i],
		    int( leaf.area ), OutsideFrustum( viewFrustum, center, half ) ? "true" : "false" );
	}
	out.PutString( "],\"missing_props_detail\":[" );
	for ( size_t i = 0; i < missingProps.size(); ++i )
	{
		Vector mins, maxs;
		StaticPropMgr_CorePropBounds( missingProps[i], mins, maxs );
		const Vector center = ( mins + maxs ) * 0.5f, half = ( maxs - mins ) * 0.5f;
		out.Printf( "%s{\"prop\":%d,\"outside_view\":%s}", i ? "," : "", missingProps[i],
		    OutsideFrustum( viewFrustum, center, half ) ? "true" : "false" );
	}
	out.PutString( "]}\n" );
}

} // namespace

void RenderCoreWorld_ViewEnd()
{
	s_nViewDepth = MAX( 0, s_nViewDepth - 1 );
	CaptureState &capture = Capture();
	if ( !capture.capturing || capture.stack.empty() )
		return;
	std::vector<ViewRecord> lists = std::move( capture.stack.back() );
	capture.stack.pop_back();
	for ( ViewRecord &view : lists )
		WriteViewRecord( view );
}

CON_COMMAND( r_core_cull_capture,
    "r_core_cull_capture <file> [tag]: records, for every world view of the next frame, the "
    "render core's culled world leaves against the legacy visible leaves (RFC 0016 K5)" )
{
	if ( args.ArgC() < 2 )
	{
		Warning( "usage: r_core_cull_capture <file> [tag]\n" );
		return;
	}
	CaptureState &capture = Capture();
	capture.file = args[1];
	capture.tag = args.ArgC() > 2 ? args[2] : "";
	capture.records.Clear();
	capture.pending = true;
}
