//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The port's shared description rules (RFC 0016). Every adapter
//			applies them before it creates anything, so each rule has one owner
//			and the shared suite's bad adapters show what skipping one breaks.
//
//=============================================================================//

#ifndef RENDER_DEVICE_VALIDATION_H
#define RENDER_DEVICE_VALIDATION_H

#include "render/device/bind_group.h"
#include "render/device/device.h"
#include "render/device/pipeline.h"
#include "render/device/resources.h"

#include <functional>
#include <optional>
#include <span>

namespace render::device
{

DeviceResult<void> ValidateBuffer( const BufferDesc &desc );
DeviceResult<void> ValidateTexture( const TextureDesc &desc, const Limits &limits );
DeviceResult<void> ValidateBindGroupLayout( const BindGroupLayoutDesc &desc );

// What an adapter knows about one of its layouts.
struct LayoutView
{
	BindGroupRole role = BindGroupRole::kFrame;
	std::span<const BindingDesc> bindings;
};

using LayoutLookup = std::function<std::optional<LayoutView>( BindGroupLayoutId )>;

// Group count, artifact format, stage set, and that every reflected binding
// is declared by the layout of its group with the same kind.
DeviceResult<void> ValidatePipeline(
    const PipelineDesc &desc, const DeviceFacts &facts, const LayoutLookup &lookup );

// Each entry names a binding the layout declares, with a resource of its kind.
DeviceResult<void> ValidateBindGroup( const BindGroupDesc &desc, const LayoutView &layout );

// D16: which words of the bound pipeline's draw-constant block an encoder has
// written since the pipeline was bound. Binding a pipeline leaves the block
// undefined; a draw or dispatch needs every word of it written. Both adapters
// keep one per encoder while validating.
class DrawConstantCoverage
{
public:
	void Bind( std::uint32_t pipelineBytes )
	{
		m_Bytes = pipelineBytes;
		m_Written = 0;
	}
	// False for an empty, unaligned or out-of-block write.
	bool Write( std::uint32_t offset, std::size_t size );
	// Every word of the block written (true for a pipeline without one).
	bool Ready() const;

private:
	std::uint32_t m_Bytes = 0;
	std::uint32_t m_Written = 0; // one bit per 4-byte word
};

} // namespace render::device

#endif // RENDER_DEVICE_VALIDATION_H
