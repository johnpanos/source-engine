//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.graph.v1 trace (RFC 0016): what compilation decided, in a
//			form tests, the independent model and evidence records compare.
//
//=============================================================================//

#ifndef RENDER_GRAPH_TRACE_H
#define RENDER_GRAPH_TRACE_H

#include "render/device/usage.h"

#include <cstdint>
#include <string>
#include <vector>

namespace render::graph
{

struct TraceTransition
{
	std::uint32_t pass = 0; // compiled index; UINT32_MAX for the final transitions
	std::uint32_t resource = 0;
	device::ResourceUsage before = device::ResourceUsage::kUndefined;
	device::ResourceUsage after = device::ResourceUsage::kUndefined;

	friend bool operator==( const TraceTransition &, const TraceTransition & ) = default;
};

// A cross-queue wait: compiled pass `consumer` starts after compiled pass
// `producer`, on the other queue, has completed.
struct TraceWait
{
	std::uint32_t consumer = 0;
	std::uint32_t producer = 0;

	friend bool operator==( const TraceWait &, const TraceWait & ) = default;
};

struct GraphTrace
{
	std::vector<std::uint32_t> kept;   // declaration indices, in execution order
	std::vector<std::uint32_t> culled; // declaration indices
	std::vector<TraceTransition> transitions;
	// Transients that share one physical resource, each list in first-use
	// order (resource indices); a transient alone is a set of one.
	std::vector<std::vector<std::uint32_t>> aliasSets;
	// Cross-queue waits, in consumer order; empty on one queue.
	std::vector<TraceWait> waits;

	// One line per decision, stable for comparison and logs.
	std::string ToString() const;
};

} // namespace render::graph

#endif // RENDER_GRAPH_TRACE_H
