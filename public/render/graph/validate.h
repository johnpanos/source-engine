//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.graph.v1 compiled-graph validation (RFC 0016 K2). Checks
//			a CompiledGraph against its own declarations, independently of how
//			the compiler produced it:
//
//			- every access finds its resource in the access's usage, reached by
//			  recorded transitions, and a write after another pass's write has
//			  its own transition (kMissingTransition);
//			- transients sharing a physical resource have disjoint lifetimes
//			  and one shape (kOverlappingAliases);
//			- every side-effect pass is kept (kCulledSideEffect);
//			- kept passes that touch a resource one of them writes run in
//			  declaration order (kReorderedDependency);
//			- a transient's first access in execution order writes it
//			  (kUndefinedRead).
//
//=============================================================================//

#ifndef RENDER_GRAPH_VALIDATE_H
#define RENDER_GRAPH_VALIDATE_H

#include "render/graph/compiled_graph.h"

#include <cstdint>
#include <vector>

namespace render::graph
{

enum class ViolationKind : std::uint8_t
{
	kMissingTransition,
	kOverlappingAliases,
	kCulledSideEffect,
	kReorderedDependency,
	kUndefinedRead
};

struct GraphViolation
{
	ViolationKind kind = ViolationKind::kMissingTransition;
	std::uint32_t pass = 0;     // declaration index, where one applies
	std::uint32_t resource = 0; // resource index, where one applies
};

const char *DescribeViolation( ViolationKind kind );
std::vector<GraphViolation> ValidateCompiledGraph( const CompiledGraph &graph );

} // namespace render::graph

#endif // RENDER_GRAPH_VALIDATE_H
