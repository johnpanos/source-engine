//========= Copyright Valve Corporation, All rights reserved. ============//
#include "core_temporal.h"
#include <cstdio>
#include <fstream>
#include <vector>
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
	ReleaseCapture( m_Capture && m_Capture->submitted ? m_Capture->token : m_Last );
	ReleaseImages();
}
bool CoreTemporal::Record( CommandEncoder &encoder, const legacy::CorePassTarget &target,
    TextureId motion, const TemporalRequest &request )
{
	CollectCapture();
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
	{
		std::fprintf( stderr, "FSR targets: render %ux%u at %d,%d, output %ux%u, target %ux%u\n",
		    request.render.width, request.render.height, request.x, request.y, request.output.width,
		    request.output.height, target.width, target.height );
		return fail( "targets" );
	}
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
		RecordCapture( encoder, request, dispatch.reset, target.frame );
		if ( !m_Frame )
			std::fprintf( stderr, "FSR game: %ux%u -> %ux%u, before post/HUD\n",
			    request.render.width, request.render.height, request.output.width,
			    request.output.height );
		m_Generation = request.generation;
		m_Frame = target.frame;
	}
	return success;
}

// Diagnostic only: one bounded readback, using the exact images sent to FSR.
void CoreTemporal::RecordCapture( CommandEncoder &encoder, const TemporalRequest &request,
    bool historyReset, std::uint64_t frame )
{
	if ( request.capturePrefix.empty() || request.capturePrefix == m_LastCapturePrefix )
		return;
	m_LastCapturePrefix = request.capturePrefix;
	if ( std::ifstream( request.capturePrefix + ".json" ).good() )
	{
		std::fprintf( stderr, "FSR capture: destination exists; use a unique prefix\n" );
		return;
	}
	if ( m_Capture )
	{
		std::fprintf( stderr, "FSR capture: previous capture is still pending\n" );
		return;
	}
	Capture capture;
	capture.request = request;
	capture.historyReset = historyReset;
	capture.frame = frame;
	const TextureId images[] = { m_Images.color, m_Images.depth, m_Images.motion, m_Images.output };
	const unsigned pixelBytes[] = { 8, 4, 4, 8 };
	for ( unsigned i = 0; i < 4; ++i )
	{
		const auto extent = i == 3 ? request.output : request.render;
		capture.sizes[i] = std::uint64_t( extent.width ) * extent.height * pixelBytes[i];
		BufferDesc desc;
		desc.size = capture.sizes[i];
		desc.memory = MemoryKind::kReadback;
		desc.usages = { ResourceUsage::kCopyDestination };
		desc.debugName = "game temporal capture";
		auto buffer = m_Device.CreateBuffer( desc );
		if ( !buffer )
		{
			for ( auto allocated : capture.buffers )
				if ( allocated.IsValid() )
					(void)m_Device.Release( allocated, {} );
			std::fprintf( stderr, "FSR capture: allocation failed\n" );
			return;
		}
		capture.buffers[i] = buffer.Value();
	}
	for ( unsigned i = 0; i < 4; ++i )
	{
		const auto extent = i == 3 ? request.output : request.render;
		encoder.TransitionBuffer(
		    capture.buffers[i], ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.TransitionTexture( images[i], ResourceUsage::kSampled, ResourceUsage::kCopySource );
		TextureBufferCopy copy;
		copy.width = extent.width;
		copy.height = extent.height;
		encoder.CopyTextureToBuffer( images[i], capture.buffers[i], copy );
		encoder.TransitionTexture( images[i], ResourceUsage::kCopySource, ResourceUsage::kSampled );
	}
	m_Capture = std::move( capture );
}

void CoreTemporal::ReleaseCapture( CompletionToken token )
{
	if ( !m_Capture )
		return;
	for ( auto buffer : m_Capture->buffers )
		(void)m_Device.Release( buffer, token );
	m_Capture.reset();
}

void CoreTemporal::CaptureSubmitted( CompletionToken token, bool success )
{
	if ( !m_Capture || m_Capture->submitted )
		return;
	if ( !success )
	{
		std::fprintf( stderr, "FSR capture: frame submission failed; no capture published\n" );
		ReleaseCapture( m_Last );
		return;
	}
	m_Capture->token = token;
	m_Capture->submitted = true;
}

void CoreTemporal::CollectCapture()
{
	if ( !m_Capture || !m_Capture->submitted || !m_Device.IsComplete( m_Capture->token ) )
		return;
	const auto &capture = *m_Capture;
	const auto &request = capture.request;
	const char *suffixes[] = {
	    ".color.rgba16f", ".depth.r32f", ".motion.rg16f", ".output.rgba16f" };
	bool success = true;
	for ( unsigned i = 0; i < 4 && success; ++i )
	{
		std::vector<std::byte> bytes( capture.sizes[i] );
		success = m_Device.ReadBuffer( capture.buffers[i], 0, bytes ).HasValue();
		if ( !success )
			break;
		std::ofstream file( request.capturePrefix + suffixes[i], std::ios::binary );
		file.write( reinterpret_cast<const char *>( bytes.data() ), bytes.size() );
		file.close();
		success = !file.fail();
	}
	if ( success )
	{
		std::ofstream file( request.capturePrefix + ".json" );
		file.precision( 9 );
		file << "{\"render\":[" << request.render.width << ',' << request.render.height
		     << "],\"output\":[" << request.output.width << ',' << request.output.height
		     << "],\"jitter\":[" << request.jitterX << ',' << request.jitterY
		     << "],\"generation\":" << request.generation << ",\"native_frame\":" << capture.frame
		     << ",\"history_reset\":" << ( capture.historyReset ? "true" : "false" )
		     << ",\"motion_targets\":" << request.motionTargets
		     << ",\"camera_valid\":" << ( request.cameraValid ? "true" : "false" );
		const auto matrix = [&]( const char *name, const std::array<float, 16> &values )
		{
			file << ",\"" << name << "\":[";
			for ( unsigned i = 0; i < 16; ++i )
				file << ( i ? "," : "" ) << values[i];
			file << ']';
		};
		matrix( "current_to_clip", request.currentToClip );
		matrix( "previous_to_clip", request.previousToClip );
		file << "}\n";
		file.close();
		success = !file.fail();
	}
	std::fprintf( stderr, "FSR capture: %s (%s)\n", success ? "saved" : "failed",
	    request.capturePrefix.c_str() );
	ReleaseCapture( capture.token );
}
}
