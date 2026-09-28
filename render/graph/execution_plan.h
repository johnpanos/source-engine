//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: What the serial and pooled executors share (private to
//			render.graph): resolving resources to device handles, the
//			transitions as recorded (a pooled resource's first transition
//			starts from the usage it was left in), recording one pass, and
//			finishing an execution.
//
//=============================================================================//

#ifndef RENDER_GRAPH_EXECUTION_PLAN_H
#define RENDER_GRAPH_EXECUTION_PLAN_H

#include "render/graph/executor.h"

#include <vector>

namespace render::graph::detail
{

struct Plan
{
	std::vector<device::TextureId> textures; // per resource
	std::vector<device::BufferId> buffers;
	std::vector<TransientPool::Entry> physical; // per physical resource
	std::vector<bool> fromPool;
	std::vector<std::vector<device::ResourceUsage>> before; // per compiled pass, per transition
	std::vector<device::ResourceUsage> finalBefore;
	std::vector<device::ResourceUsage> exitUsage; // per physical resource
	std::uint32_t created = 0;
	std::uint32_t reused = 0;
};

foundation::Expected<Plan, device::DeviceError> Prepare(
    const CompiledGraph &graph, device::IRenderDevice2 &device, TransientPool *pool );
void RecordPass( device::CommandEncoder &encoder, const CompiledGraph &graph, const Plan &plan,
    std::size_t index );
void RecordFinal( device::CommandEncoder &encoder, const CompiledGraph &graph, const Plan &plan );
// After a submission (token valid) or a failure (token absent): returns the
// physical resources to the pool, or releases them behind the token.
void Finish( const CompiledGraph &graph, Plan &plan, device::IRenderDevice2 &device,
    TransientPool *pool, const device::CompletionToken *token );
std::uint32_t TransitionCount( const CompiledGraph &graph );

} // namespace render::graph::detail

#endif // RENDER_GRAPH_EXECUTION_PLAN_H
