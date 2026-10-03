//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.vulkan host interop (RFC 0016 K1): IHostDevice over
//			a VulkanDevice created in host mode. See host_device.h.
//
//=============================================================================//

#include "vulkan_device.h"

#include <cstdio>
#include <cstring>

namespace render::device::vulkan
{

namespace
{

class HostDevice final : public IHostDevice
{
public:
	explicit HostDevice( std::unique_ptr<VulkanDevice> device ) : m_Device( std::move( device ) ) {}

	const HostDeviceInfo &Info() const override { return m_Device->HostInfo(); }
	IRenderDevice2 &Port() override { return *m_Device; }

	void RecordNative( CommandEncoder &encoder, NativeRecord record, void *user ) override
	{
		if ( VulkanEncoder *backend = Backend( encoder ) )
			backend->Native( record, user );
	}
	void BeginSection( CommandEncoder &encoder ) override
	{
		if ( VulkanEncoder *backend = Backend( encoder ) )
			backend->BeginSection();
	}
	void EndSection( CommandEncoder &encoder ) override
	{
		if ( VulkanEncoder *backend = Backend( encoder ) )
			backend->EndSection();
	}
	bool RunSection( VkCommandBuffer cmd, std::uint32_t index ) override
	{
		return m_Device->RunSection( cmd, index );
	}
	void AddSubmitWait(
	    CommandEncoder &encoder, VkSemaphore semaphore, VkPipelineStageFlags2 stage ) override
	{
		if ( VulkanEncoder *backend = Backend( encoder ) )
			backend->AddWait( semaphore, stage );
	}
	void AddSubmitSignal( CommandEncoder &encoder, VkSemaphore semaphore ) override
	{
		if ( VulkanEncoder *backend = Backend( encoder ) )
			backend->AddSignal( semaphore );
	}
	bool ImportImage(
	    VkImage image, const TextureDesc &desc, ResourceUsage home, TextureId *texture ) override
	{
		auto imported = m_Device->ImportImage( image, desc, home );
		if ( !imported )
			return false;
		*texture = imported.Value();
		return true;
	}
	bool SubmitNative( NativeRecord record, void *user, VkSemaphore wait,
	    VkPipelineStageFlags2 waitStage, VkSemaphore signal, CompletionToken *token ) override
	{
		auto encoder = m_Device->BeginEncoder( QueueKind::kGraphics );
		if ( !encoder )
			return false;
		RecordNative( encoder.Value(), record, user );
		if ( wait != VK_NULL_HANDLE )
			AddSubmitWait( encoder.Value(), wait, waitStage );
		if ( signal != VK_NULL_HANDLE )
			AddSubmitSignal( encoder.Value(), signal );
		auto submitted = m_Device->Submit( QueueKind::kGraphics, { &encoder.Value(), 1 }, {} );
		if ( !submitted )
			return false;
		if ( token )
			*token = submitted.Value();
		return true;
	}

	VkResult CreateBuffer( const VkBufferCreateInfo &info, const HostMemory &memory,
	    VkBuffer *buffer, HostAllocation *allocation, void **mapped ) override
	{
		return m_Device->Memory().CreateBuffer( info, memory, buffer, allocation, mapped );
	}
	VkResult CreateImage( const VkImageCreateInfo &info, const HostMemory &memory, VkImage *image,
	    HostAllocation *allocation ) override
	{
		return m_Device->Memory().CreateImage( info, memory, image, allocation );
	}
	void DestroyBuffer( VkBuffer buffer, HostAllocation allocation ) override
	{
		m_Device->Memory().DestroyBuffer( buffer, allocation );
	}
	void DestroyImage( VkImage image, HostAllocation allocation ) override
	{
		m_Device->Memory().DestroyImage( image, allocation );
	}
	void Free( HostAllocation allocation ) override { m_Device->Memory().Free( allocation ); }
	VkResult Map( HostAllocation allocation, void **data ) override
	{
		return m_Device->Memory().Map( allocation, data );
	}
	void Unmap( HostAllocation allocation ) override { m_Device->Memory().Unmap( allocation ); }
	VkResult Invalidate(
	    HostAllocation allocation, VkDeviceSize offset, VkDeviceSize size ) override
	{
		return m_Device->Memory().Invalidate( allocation, offset, size );
	}
	VkMemoryPropertyFlags Properties( HostAllocation allocation ) const override
	{
		return m_Device->Memory().Properties( allocation );
	}

	VkSemaphore Timeline() const override { return m_Device->TimelineSemaphore(); }
	std::uint64_t NextSubmitValue() override { return m_Device->ReserveValue(); }
	void AbandonValue( std::uint64_t value ) override { m_Device->AbandonValue( value ); }
	std::uint64_t SubmittedValue() const override { return m_Device->SubmittedValue(); }
	std::uint64_t CompletedValue() override { return m_Device->Completed(); }
	VkResult WaitValue( std::uint64_t value, std::uint64_t timeoutNs ) override
	{
		return m_Device->WaitValue( value, timeoutNs );
	}

	void ReleaseBufferAfter(
	    std::uint64_t value, VkBuffer buffer, HostAllocation allocation ) override
	{
		VulkanDevice::HostRelease release;
		release.value = value;
		release.buffer = buffer;
		release.allocation = allocation;
		m_Device->ReleaseHostAfter( release );
	}
	void ReleaseImageAfter( std::uint64_t value, VkImage image, HostAllocation allocation ) override
	{
		VulkanDevice::HostRelease release;
		release.value = value;
		release.image = image;
		release.allocation = allocation;
		m_Device->ReleaseHostAfter( release );
	}
	void ReleaseAfter(
	    std::uint64_t value, void ( *destroy )( void *object ), void *object ) override
	{
		VulkanDevice::HostRelease release;
		release.value = value;
		release.destroy = destroy;
		release.object = object;
		m_Device->ReleaseHostAfter( release );
	}
	std::size_t Collect() override { return m_Device->CollectHost(); }
	std::size_t PendingReleases() const override { return m_Device->PendingHostReleases(); }

	void LockQueue() override { m_Device->QueueMutex().lock(); }
	void UnlockQueue() override { m_Device->QueueMutex().unlock(); }

	std::uint64_t LiveAllocations() const override { return m_Device->Memory().LiveAllocations(); }
	std::uint64_t LiveBytes() const override { return m_Device->Memory().LiveBytes(); }

private:
	// The port encoder's Vulkan backend, when it is one of this device's.
	VulkanEncoder *Backend( CommandEncoder &encoder )
	{
		auto *backend = dynamic_cast<VulkanEncoder *>( encoder.Backend() );
		return backend && &backend->Device() == m_Device.get() ? backend : nullptr;
	}

	std::unique_ptr<VulkanDevice> m_Device;
};

void Copy( char *error, std::size_t errorSize, const char *message )
{
	if ( error && errorSize > 0 )
		std::snprintf( error, errorSize, "%s", message ? message : "" );
}

// Creates a host device on instance (null: its own), or reports why not.
std::unique_ptr<IHostDevice> MakeDevice( const HostDeviceRequest &request,
    std::shared_ptr<InstanceHandle> instance, char *error, std::size_t errorSize )
{
	Copy( error, errorSize, "" );
	VulkanAdapterOptions options;
	options.fsr411 = request.fsr411;
	auto device = std::make_unique<VulkanDevice>( options, &request, std::move( instance ) );
	const DeviceResult<void> initialized = device->Initialize();
	if ( !initialized )
	{
		char message[256];
		std::snprintf( message, sizeof( message ),
		    "render.device.vulkan: %s (status %u, VkResult %d)",
		    device->FailureReason() ? device->FailureReason() : "the device could not be created",
		    static_cast<unsigned>( initialized.Error().status ),
		    static_cast<int>( initialized.Error().nativeCode ) );
		Copy( error, errorSize, message );
		// The surface is the host's only once creation succeeds.
		const HostDeviceInfo &info = device->HostInfo();
		if ( info.surface != VK_NULL_HANDLE && info.instance != VK_NULL_HANDLE )
			vkDestroySurfaceKHR( info.instance, info.surface, nullptr );
		return nullptr;
	}
	return std::make_unique<HostDevice>( std::move( device ) );
}

class HostInstance final : public IHostInstance
{
public:
	explicit HostInstance( std::shared_ptr<InstanceHandle> instance, bool fsr )
	    : m_Instance( std::move( instance ) ), m_Fsr( fsr )
	{
	}
	VkInstance Instance() const override { return m_Instance->instance; }
	std::unique_ptr<IHostDevice> CreateDevice(
	    const HostDeviceRequest &request, char *error, std::size_t errorSize ) override
	{
		auto selected = request;
		selected.fsr411 = m_Fsr;
		return MakeDevice( selected, m_Instance, error, errorSize );
	}

private:
	std::shared_ptr<InstanceHandle> m_Instance;
	bool m_Fsr;
};

class Factory final : public IHostDeviceFactory
{
	bool m_Fsr;

public:
	explicit Factory( bool fsr ) : m_Fsr( fsr ) {}
	std::unique_ptr<IHostDevice> Create(
	    const HostDeviceRequest &request, char *error, std::size_t errorSize ) const override
	{
		auto selected = request;
		selected.fsr411 = m_Fsr;
		return MakeDevice( selected, nullptr, error, errorSize );
	}

	std::unique_ptr<IHostInstance> CreateInstance(
	    const HostDeviceRequest &request, char *error, std::size_t errorSize ) const override
	{
		Copy( error, errorSize, "" );
		std::uint32_t apiVersion = 0;
		auto instance = CreateHostInstance( request, &apiVersion );
		if ( !instance )
		{
			Copy( error, errorSize,
			    "render.device.vulkan: the Vulkan instance could not be created" );
			return nullptr;
		}
		return std::make_unique<HostInstance>(
		    std::shared_ptr<InstanceHandle>( std::move( instance ).Value() ), m_Fsr );
	}
};

} // namespace

const IHostDeviceFactory &HostDeviceFactory( bool fsr411 )
{
	static const Factory normal( false ), temporal( true );
	return fsr411 ? temporal : normal;
}

} // namespace render::device::vulkan
