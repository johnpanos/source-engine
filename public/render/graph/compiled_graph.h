//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.graph.v1 compilation (RFC 0016). CompileGraph validates a
//			built graph and derives what executors need: the kept passes in
//			order, the usage transition before each access, the transitions
//			that return imported resources to their final usage, and each
//			transient's first and last pass.
//
//=============================================================================//

#ifndef RENDER_GRAPH_COMPILED_GRAPH_H
#define RENDER_GRAPH_COMPILED_GRAPH_H

#include "foundation/expected.h"
#include "render/graph/graph_builder.h"
#include "render/graph/trace.h"

#include <cstdint>
#include <vector>

namespace render::graph
{

enum class GraphStatus : std::uint8_t
{
	kReadBeforeWrite = 1, // a transient read before any pass wrote it
	kConflictingAccess,   // one pass uses a resource in two different usages
	kInvalidResource,     // a reference the builder never returned
	kUsageKindMismatch,   // a write usage declared as a read, or the reverse
	kNoExecute,           // a kept pass without an execute function
	kQueueKindMismatch    // a non-compute pass asked for the async compute queue
};

struct GraphError
{
	GraphStatus status = GraphStatus::kInvalidResource;
	std::uint32_t pass = 0;     // declaration index of the offending pass
	std::uint32_t resource = 0; // index of the offending resource
};

const char *DescribeGraphStatus( GraphStatus status );

struct Transition
{
	ResourceRef resource;
	device::ResourceUsage before = device::ResourceUsage::kUndefined;
	device::ResourceUsage after = device::ResourceUsage::kUndefined;
};

struct CompiledPass
{
	std::uint32_t declaration = 0; // index in GraphBuilder::Passes()
	std::vector<Transition> transitions;
	Queue queue = Queue::kGraphics; // where it runs (kGraphics unless options allow)
	// The latest compiled pass on the other queue that must complete before
	// this one starts (a timeline wait), or UINT32_MAX. Queues execute in
	// compiled order, so one wait covers every earlier pass on that queue.
	std::uint32_t waitFor = UINT32_MAX;
	// Resources whose previous access was on the other queue: their
	// ownership moves to this pass's queue (a queue-family transfer where
	// the queues are distinct families).
	std::vector<ResourceRef> acquires;
};

struct Lifetime
{
	ResourceRef resource;
	std::uint32_t first = 0; // compiled pass indices
	std::uint32_t last = 0;
	device::UsageSet usages; // what the transient is created with
};

// One device resource that backs one or more transients. Transients with
// the same description (usages aside) and disjoint lifetimes share one: the
// later one's first access is a write, and its transition starts from the
// state the earlier one left, so the reuse is ordered like any other access.
// Aliasing memory between different descriptions is a device capability
// (kTransientAliasing) that this compiler does not use.
struct PhysicalResource
{
	bool isTexture = true;
	device::TextureDesc texture;
	device::BufferDesc buffer;
	device::UsageSet usages; // the union over its members
	std::uint32_t first = 0; // compiled pass indices
	std::uint32_t last = 0;
	std::vector<std::uint32_t> members; // transient resource indices, first-use order
};

class CompiledGraph
{
public:
	std::vector<ResourceDecl> resources;
	std::vector<PassDecl> passes; // declarations, including culled ones
	std::vector<CompiledPass> order;
	std::vector<Transition> finalTransitions;
	std::vector<Lifetime> transients;
	std::vector<PhysicalResource> physical;
	// Per resource: its PhysicalResource, or -1 (imported, or never used).
	std::vector<std::int32_t> physicalOf;
	GraphTrace trace;
};

struct CompileOptions
{
	// The device has a compute queue separate from graphics
	// (device::Capability::kAsyncCompute). Without it every pass runs on
	// kGraphics and the compiled graph has no waits.
	bool asyncCompute = false;
};

foundation::Expected<CompiledGraph, GraphError> CompileGraph(
    GraphBuilder &&builder, const CompileOptions &options = {} );

} // namespace render::graph

#endif // RENDER_GRAPH_COMPILED_GRAPH_H
