//========= Copyright Valve Corporation, All rights reserved. ============//
#include "core_temporal.h"
#include <cstdio>
namespace render::composition
{
using namespace device;
void CoreTemporal::ReleaseImages()
{
	for ( auto image : { m_Images.color, m_Images.depth, m_Images.motion, m_Images.output } )
		if ( image.IsValid() )
			(void)m_Device.Release( ResourceId( image ), m_Last );
	m_Images = {};
	m_Render = {};
	m_Display = {};
}
CoreTemporal::~CoreTemporal()
{
	ReleaseImages();
}
bool CoreTemporal::Record( CommandEncoder &encoder, const legacy::CorePassTarget &target,
    TextureId motion, const TemporalRequest &request )
{
	const auto fail = []( const char *stage )
	{
		std::fprintf( stderr, "FSR game: %s failed\n", stage );
		return false;
	};
	if ( !target.color.IsValid() || !target.depth.IsValid() || !motion.IsValid() ||
	     target.samples != 1 || !request.render.width || !request.render.height || request.x < 0 ||
	     request.y < 0 || request.x + request.render.width > target.width ||
	     request.y + request.render.height > target.height ||
	     request.output.width != target.width || request.output.height != target.height )
		return fail( "targets" );
	m_Last = target.submitted;
	const bool resized = m_Render != request.render || m_Display != request.output;
	if ( resized )
	{
		ReleaseImages();
		const Format formats[] = {
		    Format::kRGBA16Float, Format::kD32Float, Format::kRG16Float, Format::kRGBA16Float };
		TextureId *images[] = {
		    &m_Images.color, &m_Images.depth, &m_Images.motion, &m_Images.output };
		for ( unsigned i = 0; i < 4; ++i )
		{
			TextureDesc desc;
			desc.format = formats[i];
			auto extent = i == 3 ? request.output : request.render;
			desc.width = extent.width;
			desc.height = extent.height;
			desc.usages = { ResourceUsage::kSampled, ResourceUsage::kCopySource };
			desc.usages.Add( i == 3   ? ResourceUsage::kStorageWrite
			                 : i == 1 ? ResourceUsage::kDepthWrite
			                          : ResourceUsage::kColorAttachment );
			desc.debugName = "game temporal image";
			auto image = m_Device.CreateTexture( desc );
			if ( !image )
			{
				ReleaseImages();
				return fail( "allocation" );
			}
			*images[i] = image.Value();
			encoder.TransitionTexture(
			    image.Value(), ResourceUsage::kUndefined, ResourceUsage::kSampled );
		}
		m_Render = request.render;
		m_Display = request.output;
	}
	if ( !m_Output || m_Format != target.colorFormat )
	{
		auto output = pass::output::OutputRenderer::Create( m_Device, target.colorFormat );
		if ( !output )
			return fail( "output pipeline" );
		m_Output = std::move( output ).Value();
		m_Format = target.colorFormat;
	}
	m_Output->Collect( target.submitted );
	encoder.BeginLabel( "game temporal reconstruction" );
	TemporalImages sources{ target.color, target.depth, motion, {} };
	const bool decode =
	    target.colorFormat == Format::kBGRA8Unorm || target.colorFormat == Format::kRGBA8Unorm;
	if ( !m_Copy.Record( encoder, sources, m_Images, request.render, request.x, request.y, decode,
	         target.submitted ) )
	{
		encoder.EndLabel();
		return fail( "input copy" );
	}
	encoder.TransitionTexture(
	    m_Images.output, ResourceUsage::kSampled, ResourceUsage::kStorageWrite );
	TemporalDispatch dispatch;
	dispatch.images = m_Images;
	dispatch.render = request.render;
	dispatch.output = request.output;
	dispatch.jitterX = request.jitterX;
	dispatch.jitterY = request.jitterY;
	dispatch.deltaMilliseconds = request.deltaMilliseconds;
	dispatch.preExposure = target.outputScale;
	dispatch.reset = resized || m_Generation != request.generation || m_Frame + 1 != target.frame;
	if ( !m_Provider->Record( encoder, dispatch ) )
	{
		encoder.EndLabel();
		return fail( "provider recording" );
	}
	encoder.TransitionTexture(
	    m_Images.output, ResourceUsage::kStorageWrite, ResourceUsage::kSampled );
	pass::output::OutputDirectTargets output;
	output.scene = m_Images.output;
	output.sceneWidth = request.output.width;
	output.sceneHeight = request.output.height;
	output.target = target.color;
	output.width = request.output.width;
	output.height = request.output.height;
	pass::output::OutputParams params;
	params.toneMap = false;
	const bool success = m_Output->Record( encoder, output, params ).HasValue();
	encoder.EndLabel();
	if ( success )
	{
		if ( !m_Frame )
			std::fprintf( stderr, "FSR game: %ux%u -> %ux%u, before post/HUD\n",
			    request.render.width, request.render.height, request.output.width,
			    request.output.height );
		m_Generation = request.generation;
		m_Frame = target.frame;
	}
	return success;
}
}
