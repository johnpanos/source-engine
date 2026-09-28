//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.frame.v1 features (RFC 0016). A feature adds its passes to
//			each frame's graph and declares the device capabilities it needs;
//			composition fails, naming the feature and capability, when the
//			device lacks one. Features never depend on each other or on the
//			renderer (CAP011 rule 2); they meet only in the graph.
//
//=============================================================================//

#ifndef RENDER_FRAME_FEATURE_H
#define RENDER_FRAME_FEATURE_H

#include "render/device/facts.h"
#include "render/frame/renderer.h"
#include "render/graph/executor.h"
#include "render/graph/graph_builder.h"

namespace render::frame
{

struct FeatureRequirements
{
	device::CapabilitySet required;
};

struct FeatureContext
{
	graph::GraphBuilder &graph;
	const FrameDesc &frame;
	graph::ResourceRef target; // the frame's color target
	std::uint64_t stagesMarked = 0;
	std::uint32_t views = 0;
};

class IRenderFeature
{
public:
	virtual ~IRenderFeature() = default;
	virtual const char *Name() const = 0;
	virtual FeatureRequirements Requirements() const = 0;
	virtual void AddPasses( FeatureContext &context ) = 0;
};

} // namespace render::frame

#endif // RENDER_FRAME_FEATURE_H
