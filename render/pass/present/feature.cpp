//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.present (RFC 0016); see feature.h.
//
//=============================================================================//

#include "render/pass/present/feature.h"

namespace render::pass::present
{

namespace
{

class PresentFeature final : public frame::IRenderFeature
{
public:
	explicit PresentFeature( const PresentOptions &options ) : m_Options( options ) {}

	const char *Name() const override { return "present"; }
	frame::FeatureRequirements Requirements() const override { return {}; }

	void AddPasses( frame::FeatureContext &context ) override
	{
		const graph::ResourceRef target = context.target;
		const std::uint32_t width =
		    context.frame.target.IsValid() ? context.frame.targetDesc.width : context.frame.width;
		const std::uint32_t height =
		    context.frame.target.IsValid() ? context.frame.targetDesc.height : context.frame.height;
		const device::ClearColor clear = m_Options.clear;
		context.graph.AddPass( "present", graph::PassKind::kRender )
		    .Write( target, device::ResourceUsage::kColorAttachment )
		    .SideEffect()
		    .Execute(
		        [target, width, height, clear]( graph::RecordContext &record )
		        {
			        device::ColorAttachment color[1];
			        color[0].texture = record.Texture( target );
			        color[0].load = device::LoadOp::kLoad;
			        color[0].clear = clear;
			        device::RenderingDesc rendering;
			        rendering.colors = color;
			        rendering.width = width;
			        rendering.height = height;
			        record.Encoder().BeginRendering( rendering );
			        record.Encoder().EndRendering();
		        } );
	}

private:
	PresentOptions m_Options;
};

} // namespace

std::unique_ptr<frame::IRenderFeature> CreatePresentFeature( const PresentOptions &options )
{
	return std::make_unique<PresentFeature>( options );
}

} // namespace render::pass::present
