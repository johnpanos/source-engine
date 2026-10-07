//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.pica's descriptor and creation (RFC 0026).
//
//			Only the 3DS build has a device (pica_device.h); elsewhere
//			creation fails with kUnavailable, naming the adapter.
//
//=============================================================================//

#include "device.h"

#include "render/device/pica/provider.h"

#if defined( __3DS__ )
#include "pica_device.h"
#endif

namespace render::device::pica
{
namespace
{

DeviceResult<std::unique_ptr<IRenderDevice2>> CreateFromRequest( const DeviceRequest &request )
{
	if ( auto missing = FirstMissing( AdapterFacts().capabilities, request.required ) )
		return foundation::Unexpected<DeviceError>( DeviceError{ DeviceStatus::kUnsupported,
		    DeviceOperation::kCreateDevice, static_cast<std::int32_t>( *missing ) } );
	PicaAdapterOptions options;
	options.guardLinearMemory = request.validation;
	return Create( options );
}

} // namespace

const DeviceFacts &AdapterFacts()
{
	static const DeviceFacts facts = []
	{
		DeviceFacts value;
		value.diagnosticBackend = "pica";
		value.adapterName = "PICA200";
		// RFC 0026 decision 4: ETC1 (D40) alone of the optional capabilities.
		value.capabilities = { Capability::kTextureCompressionETC1 };
		value.limits.maxBindGroups = kMaxBindGroups;
		value.limits.maxTextureDimension2D = 1024;
		value.limits.maxColorAttachments = 1;
		value.limits.maxVertexBuffers = 12;
		value.limits.uniformBufferAlignment = 16;
		value.limits.sampleCounts = 1;
		value.artifactFormat = ArtifactFormat::kPica;
		return value;
	}();
	return facts;
}

const DeviceProviderDescriptor &Describe()
{
	static const DeviceProviderDescriptor descriptor{ "pica", &CreateFromRequest };
	return descriptor;
}

DeviceResult<std::unique_ptr<IRenderDevice2>> Create( const PicaAdapterOptions &options )
{
	if ( options.commandListBytes < 4096 )
		return foundation::Unexpected<DeviceError>(
		    DeviceError{ DeviceStatus::kInvalidDescription, DeviceOperation::kCreateDevice } );
#if defined( __3DS__ )
	EnableGuardBands( options.guardLinearMemory );
	auto device = std::make_unique<PicaDevice>( options );
	if ( auto initialized = device->Initialize(); !initialized )
		return foundation::MakeUnexpected( initialized.Error() );
	return std::unique_ptr<IRenderDevice2>( std::move( device ) );
#else
	return foundation::Unexpected<DeviceError>(
	    DeviceError{ DeviceStatus::kUnavailable, DeviceOperation::kCreateDevice } );
#endif
}

DeviceResult<void> PresentTopScreen(
    IRenderDevice2 &device, TextureId source, std::uint32_t width, std::uint32_t height )
{
#if defined( __3DS__ )
	if ( auto *pica = dynamic_cast<PicaDevice *>( &device ) )
		return pica->PresentTopScreen( source, width, height );
#else
	(void)device;
	(void)source;
	(void)width;
	(void)height;
#endif
	return foundation::Unexpected<DeviceError>(
	    DeviceError{ DeviceStatus::kUnsupported, DeviceOperation::kSubmit } );
}

std::span<std::byte> MapUploadBuffer( IRenderDevice2 &device, BufferId buffer )
{
#if defined( __3DS__ )
	if ( auto *pica = dynamic_cast<PicaDevice *>( &device ) )
		return pica->MapUploadBuffer( buffer );
#else
	(void)device;
	(void)buffer;
#endif
	return {};
}

void FlushUploadBuffer(
    IRenderDevice2 &device, BufferId buffer, std::uint64_t offset, std::uint64_t size )
{
#if defined( __3DS__ )
	if ( auto *pica = dynamic_cast<PicaDevice *>( &device ) )
		pica->FlushUploadBuffer( buffer, offset, size );
#else
	(void)device;
	(void)buffer;
	(void)offset;
	(void)size;
#endif
}

bool SimulateDeviceLoss( IRenderDevice2 &device )
{
#if defined( __3DS__ )
	if ( auto *pica = dynamic_cast<PicaDevice *>( &device ) )
	{
		pica->SimulateLoss();
		return true;
	}
#else
	(void)device;
#endif
	return false;
}

} // namespace render::device::pica
