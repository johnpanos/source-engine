//========= Copyright Valve Corporation, All rights reserved. ============//
// Private FSR/Vulkan bridge; no native handles cross the temporal port.
#include "vulkan_device.h"
#include "render/device/vulkan/fsr.h"
#include <cmath>
#include <algorithm>
#include <deque>
#ifdef RENDER_FSR411
#include "fsr411.h"
#endif
namespace render::device::vulkan
{
#ifdef RENDER_FSR411
namespace
{
// A history is shared only by its view provider and outstanding GPU work.
// Submission payloads retain it through completion, on the device sequence.
struct FsrHistory
{
	std::unique_ptr<Fsr411::Upscaler> upscaler;
	std::deque<CompletionToken> slots;
	TemporalExtent render, output;
	bool pending = false, invalid = true;
};
struct FsrWork final : ComputeInterop
{
	std::shared_ptr<FsrHistory> history;
	Fsr411::Frame frame;
	bool pending = true;
	explicit FsrWork( std::shared_ptr<FsrHistory> state ) : history( std::move( state ) ) {}

	void Barrier( VkCommandBuffer cmd, bool enter )
	{
		const Fsr411::Image resources[] = { frame.color, frame.depth, frame.motion };
		std::array<VkImageMemoryBarrier2, 3> barriers{};
		for ( unsigned i = 0; i < 3; ++i )
		{
			auto &barrier = barriers[i];
			const auto &scope = ScopeOf( ResourceUsage::kSampled );
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
			barrier.srcStageMask = enter ? scope.stages : VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
			barrier.dstStageMask = enter ? VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT : scope.stages;
			barrier.srcAccessMask = enter ? scope.access : VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
			barrier.dstAccessMask = enter ? VK_ACCESS_2_SHADER_SAMPLED_READ_BIT : scope.access;
			barrier.oldLayout = enter ? scope.layout : VK_IMAGE_LAYOUT_GENERAL;
			barrier.newLayout = enter ? VK_IMAGE_LAYOUT_GENERAL : scope.layout;
			barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = resources[i].image;
			barrier.subresourceRange = {
			    i == 1 ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		}
		VkDependencyInfo dependency{};
		dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependency.imageMemoryBarrierCount = barriers.size();
		dependency.pImageMemoryBarriers = barriers.data();
		vkCmdPipelineBarrier2( cmd, &dependency );
	}
	bool Record( VkCommandBuffer cmd ) override
	{
		if ( !history->upscaler )
			return false;
		frame.cmdbuf = cmd;
		frame.reset = frame.reset || history->invalid;
		Barrier( cmd, true );
		if ( !history->upscaler->Record( frame ) )
		{
			std::fprintf( stderr, "FSR 4.1.1: %s\n", history->upscaler->Error().c_str() );
			history->invalid = true;
			return false;
		}
		Barrier( cmd, false );
		return true;
	}
	void Submitted( CompletionToken token ) override
	{
		history->slots.push_back( token );
		pending = history->pending = false;
		history->invalid = false;
	}
	void Aborted() override
	{
		if ( pending )
		{
			pending = history->pending = false;
			history->invalid = true;
		}
	}
	void DeviceDestroyed() override
	{
		history->upscaler.reset();
		history->slots.clear();
		pending = history->pending = false;
		history->invalid = true;
	}
};
}
class FsrProvider final : public ITemporalUpscaler
{
public:
	FsrProvider( VulkanDevice &device, const std::string &assets )
	    : m_Device( device ), m_Assets( assets ), m_History( std::make_shared<FsrHistory>() )
	{
	}

	std::string Description() const override { return "FSR 4.1.1 INT8 experimental / 46a56a6"; }
	DeviceResult<void> Record( CommandEncoder &encoder, const TemporalDispatch &frame ) override
	{
		auto *backend = dynamic_cast<VulkanEncoder *>( encoder.Backend() );
		auto fail = [&]( DeviceStatus status ) -> DeviceResult<void>
		{
			if ( backend )
				backend->SetError();
			return Fail( status, DeviceOperation::kSubmit );
		};
		if ( !backend || &backend->Device() != &m_Device || m_History->pending )
			return fail( DeviceStatus::kInvalidState );
		if ( !frame.render.width || !frame.render.height ||
		     frame.render.width > frame.output.width || frame.render.height > frame.output.height ||
		     frame.output.width > 3840 || frame.output.height > 2160 ||
		     !std::isfinite( frame.jitterX ) || !std::isfinite( frame.jitterY ) ||
		     !std::isfinite( frame.preExposure ) || frame.preExposure <= 0 ||
		     !std::isfinite( frame.deltaMilliseconds ) || frame.deltaMilliseconds <= 0 )
			return fail( DeviceStatus::kInvalidDescription );
		const TextureId ids[] = {
		    frame.images.color, frame.images.depth, frame.images.motion, frame.images.output };
		const Format formats[] = {
		    Format::kRGBA16Float, Format::kD32Float, Format::kRG16Float, Format::kRGBA16Float };
		Fsr411::Image native[4];
		for ( unsigned i = 0; i < 4; ++i )
		{
			for ( unsigned j = 0; j < i; ++j )
				if ( ids[i] == ids[j] )
					return fail( DeviceStatus::kInvalidDescription );
			const auto *texture = m_Device.LiveTexture( ids[i].value );
			const auto extent = i == 3 ? frame.output : frame.render;
			if ( !texture || texture->desc.format != formats[i] || texture->desc.sampleCount != 1 ||
			     texture->desc.width != extent.width || texture->desc.height != extent.height ||
			     texture->desc.mipLevels != 1 || texture->layers != 1 )
				return fail( DeviceStatus::kInvalidDescription );
			native[i] = { texture->image, texture->view, extent.width, extent.height };
		}
		// Reset/resize gets new storage. Old submissions retain the old
		// context; no idle wait or fixed number of frames proves retirement.
		if ( !m_History->upscaler || m_History->invalid || m_History->render != frame.render ||
		     m_History->output != frame.output ||
		     ( frame.reset && !m_Device.m_Options.sensitivity.fsrRetainResetHistory ) )
		{
			m_History = std::make_shared<FsrHistory>();
			m_History->render = frame.render;
			m_History->output = frame.output;
			m_History->upscaler = std::make_unique<Fsr411::Upscaler>( m_Device.m_Adapter.physical,
			    m_Device.m_Device, m_Assets, m_Device.FsrMixedFloatDot() );
		}
		while ( !m_History->slots.empty() && m_Device.IsComplete( m_History->slots.front() ) )
			m_History->slots.pop_front();
		if ( m_History->slots.size() >= Fsr411::kFramesInFlight )
			return fail( DeviceStatus::kInvalidState );
		auto work = std::make_shared<FsrWork>( m_History );
		auto &f = work->frame;
		f.color = native[0];
		f.depth = native[1];
		f.motion = native[2];
		f.output = native[3];
		f.render_width = frame.render.width;
		f.render_height = frame.render.height;
		f.jitter[0] = frame.jitterX;
		f.jitter[1] = frame.jitterY;
		f.pre_exposure = frame.preExposure;
		f.reset = frame.reset;
		work->images = { { ids[0], ResourceUsage::kSampled }, { ids[1], ResourceUsage::kSampled },
		    { ids[2], ResourceUsage::kSampled }, { ids[3], ResourceUsage::kStorageWrite } };
		m_History->pending = true;
		std::erase_if( m_Device.m_ComputeInterop,
		    []( const auto &entry )
		    {
			    return entry.expired();
		    } );
		m_Device.m_ComputeInterop.push_back( work );
		m_LastWork = work; // idle histories remain registered for device recovery
		backend->Compute( std::move( work ) );
		return {};
	}

private:
	VulkanDevice &m_Device;
	std::string m_Assets;
	std::shared_ptr<FsrHistory> m_History;
	std::shared_ptr<FsrWork> m_LastWork;
};
#endif
DeviceResult<std::unique_ptr<ITemporalUpscaler>> CreateFsr411(
    IRenderDevice2 &device, const std::string &assets )
{
#ifdef RENDER_FSR411
	auto *native = dynamic_cast<VulkanDevice *>( &device );
	if ( native && native->FsrEnabled() )
		return std::unique_ptr<ITemporalUpscaler>( new FsrProvider( *native, assets ) );
#else
	(void)device;
	(void)assets;
#endif
	return Fail( DeviceStatus::kUnsupported, DeviceOperation::kCreateDevice );
}
}
