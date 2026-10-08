//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.d3d12: submission. Validate checks one encoder's list
//			against the port's rules and the usage state (the shared
//			recording::Validate); Replay records the accepted lists into one
//			D3D12 command list.
//
//=============================================================================//

#include "d3d12_device.h"

#include <cstring>

namespace render::device::d3d12
{

// Replay ------------------------------------------------------------------------

namespace
{

constexpr std::uint64_t kPitchAlignment = D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
constexpr std::uint64_t kPlacementAlignment = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;

std::uint64_t AlignUp( std::uint64_t value, std::uint64_t alignment )
{
	return ( value + alignment - 1 ) / alignment * alignment;
}

// A copy region in D3D12 terms: whole blocks, the tight row the port's buffer
// holds and the pitched row D3D12's footprint needs.
struct Region
{
	std::uint32_t width = 0;  // texels, whole blocks
	std::uint32_t height = 0; // texels, whole blocks
	std::uint32_t rows = 0;   // block rows
	std::uint64_t rowBytes = 0;
	std::uint64_t pitch = 0;
};

Region RegionOf( Format format, std::uint32_t width, std::uint32_t height )
{
	const FormatBlock block = BlockOf( format );
	Region r;
	const std::uint32_t across = ( width + block.width - 1 ) / block.width;
	r.rows = ( height + block.height - 1 ) / block.height;
	r.width = across * block.width;
	r.height = r.rows * block.height;
	r.rowBytes = std::uint64_t( across ) * block.bytes;
	r.pitch = AlignUp( r.rowBytes, kPitchAlignment );
	return r;
}

} // namespace

class Replay
{
public:
	Replay( D3d12Device &device, ID3D12GraphicsCommandList *list,
	    std::vector<Com<ID3D12Resource>> &scratch )
	    : m_D( device ), m_L( list ), m_Scratch( scratch )
	{
		ID3D12DescriptorHeap *heaps[] = { m_D.ViewHeap(), m_D.SamplerHeap() };
		m_L->SetDescriptorHeaps( 2, heaps );
	}

	DeviceResult<void> Run( const std::vector<Command> &commands )
	{
		for ( const Command &command : commands )
		{
			if ( auto done = Execute( command ); !done )
				return done;
		}
		return {};
	}

private:
	void Transition( ID3D12Resource *resource, D3D12_RESOURCE_STATES &state,
	    D3D12_RESOURCE_STATES target )
	{
		if ( state == target )
			return;
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = resource;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = state;
		barrier.Transition.StateAfter = target;
		m_L->ResourceBarrier( 1, &barrier );
		state = target;
	}
	void Buffer( BufferRecord &b, D3D12_RESOURCE_STATES target )
	{
		if ( !b.fixedState )
			Transition( b.resource.Get(), b.state, target );
	}
	void Texture( TextureRecord &t, D3D12_RESOURCE_STATES target )
	{
		Transition( t.resource.Get(), t.state, target );
	}

	// A buffer of this submission, freed when it completes.
	DeviceResult<ID3D12Resource *> Scratch(
	    std::uint64_t size, D3D12_HEAP_TYPE heap, D3D12_RESOURCE_STATES state )
	{
		auto made = m_D.CommittedBuffer( size, heap, state );
		if ( !made )
			return foundation::MakeUnexpected( made.Error() );
		m_Scratch.push_back( std::move( made ).Value() );
		return m_Scratch.back().Get();
	}

	BufferRecord &B( std::uint64_t id ) { return *m_D.ExistingBuffer( id ); }
	TextureRecord &T( std::uint64_t id ) { return *m_D.ExistingTexture( id ); }

	D3D12_CPU_DESCRIPTOR_HANDLE Rtv( std::uint32_t slot ) const
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_D.m_RtvHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += SIZE_T( slot ) * m_D.m_RtvStride;
		return handle;
	}
	D3D12_CPU_DESCRIPTOR_HANDLE Dsv() const
	{
		return m_D.m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
	}

	// RTV and DSV descriptors are read when a clear or OMSetRenderTargets is
	// recorded, so the scratch slots are reused at once.
	void MakeRtv( const TextureRecord &t, std::uint32_t mip, std::uint32_t layer, std::uint32_t slot )
	{
		D3D12_RENDER_TARGET_VIEW_DESC view{};
		view.Format = FormatOf( t.desc.format ).attachment;
		if ( t.desc.dimension == TextureDimension::k3D )
		{
			view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
			view.Texture3D.MipSlice = mip;
			view.Texture3D.WSize = UINT( -1 );
		}
		else if ( t.desc.sampleCount > 1 )
		{
			view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
			view.Texture2DMSArray.FirstArraySlice = layer;
			view.Texture2DMSArray.ArraySize = 1;
		}
		else
		{
			view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			view.Texture2DArray.MipSlice = mip;
			view.Texture2DArray.FirstArraySlice = layer;
			view.Texture2DArray.ArraySize = 1;
		}
		m_D.m_Device->CreateRenderTargetView( t.resource.Get(), &view, Rtv( slot ) );
	}
	void MakeDsv( const TextureRecord &t, std::uint32_t mip, std::uint32_t layer, bool readOnly )
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC view{};
		view.Format = FormatOf( t.desc.format ).attachment;
		if ( readOnly )
			view.Flags = HasStencil( t.desc.format )
			                 ? D3D12_DSV_FLAG_READ_ONLY_DEPTH | D3D12_DSV_FLAG_READ_ONLY_STENCIL
			                 : D3D12_DSV_FLAG_READ_ONLY_DEPTH;
		if ( t.desc.sampleCount > 1 )
		{
			view.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
			view.Texture2DMSArray.FirstArraySlice = layer;
			view.Texture2DMSArray.ArraySize = 1;
		}
		else
		{
			view.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			view.Texture2DArray.MipSlice = mip;
			view.Texture2DArray.FirstArraySlice = layer;
			view.Texture2DArray.ArraySize = 1;
		}
		m_D.m_Device->CreateDepthStencilView( t.resource.Get(), &view, Dsv() );
	}

	D3D12_TEXTURE_COPY_LOCATION TextureLocation(
	    const TextureRecord &t, std::uint32_t mip, std::uint32_t layer ) const
	{
		D3D12_TEXTURE_COPY_LOCATION location{};
		location.pResource = t.resource.Get();
		location.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		location.SubresourceIndex = t.Subresource( mip, layer );
		return location;
	}
	static D3D12_TEXTURE_COPY_LOCATION Footprint( ID3D12Resource *buffer, std::uint64_t offset,
	    Format format, const Region &region )
	{
		D3D12_TEXTURE_COPY_LOCATION location{};
		location.pResource = buffer;
		location.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		location.PlacedFootprint.Offset = offset;
		location.PlacedFootprint.Footprint.Format = FormatOf( format ).copy;
		location.PlacedFootprint.Footprint.Width = region.width;
		location.PlacedFootprint.Footprint.Height = region.height;
		location.PlacedFootprint.Footprint.Depth = 1;
		location.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>( region.pitch );
		return location;
	}
	static D3D12_BOX Box( const TextureBufferCopy &copy, const Region &region )
	{
		return { copy.x, copy.y, 0, copy.x + region.width, copy.y + region.height, 1 };
	}
	// The port's tight layout is D3D12's footprint exactly when each row is
	// already pitched and the offset placed.
	static bool Direct( const Region &region, std::uint64_t offset )
	{
		return ( region.rows == 1 || region.rowBytes == region.pitch ) &&
		       region.rowBytes % kPitchAlignment == 0 && offset % kPlacementAlignment == 0;
	}

	DeviceResult<void> Execute( const Command &command )
	{
		switch ( command.op )
		{
		case Op::kTransitionTexture:
		{
			TextureRecord &t = T( command.a );
			Texture( t, StateOf( command.after, IsDepthFormat( t.desc.format ) ) );
			break;
		}
		case Op::kTransitionBuffer:
			Buffer( B( command.a ), StateOf( command.after, false ) );
			break;
		case Op::kClearTexture:
			ClearTexture( command );
			break;
		case Op::kWriteBuffer:
		{
			BufferRecord &b = B( command.a );
			Buffer( b, D3D12_RESOURCE_STATE_COPY_DEST );
			ID3D12Resource *source = m_D.m_Ring.Get();
			std::uint64_t at = command.ringOffset;
			if ( !command.fromRing )
			{
				auto staged = Scratch( command.bytes.size(), D3D12_HEAP_TYPE_UPLOAD,
				    D3D12_RESOURCE_STATE_GENERIC_READ );
				if ( !staged )
					return foundation::MakeUnexpected( staged.Error() );
				source = staged.Value();
				void *mapped = nullptr;
				const D3D12_RANGE none{ 0, 0 };
				if ( FAILED( source->Map( 0, &none, &mapped ) ) )
					return Fail( DeviceStatus::kInternal, DeviceOperation::kSubmit );
				std::memcpy( mapped, command.bytes.data(), command.bytes.size() );
				source->Unmap( 0, nullptr );
				at = 0;
			}
			m_L->CopyBufferRegion( b.resource.Get(), command.copy.destinationOffset, source, at,
			    command.copy.size );
			break;
		}
		case Op::kCopyBuffer:
		{
			BufferRecord &src = B( command.a );
			BufferRecord &dst = B( command.b );
			Buffer( src, D3D12_RESOURCE_STATE_COPY_SOURCE );
			Buffer( dst, D3D12_RESOURCE_STATE_COPY_DEST );
			m_L->CopyBufferRegion( dst.resource.Get(), command.copy.destinationOffset,
			    src.resource.Get(), command.copy.sourceOffset, command.copy.size );
			break;
		}
		case Op::kCopyTextureToBuffer:
			return CopyTextureToBuffer( command );
		case Op::kCopyBufferToTexture:
			return CopyBufferToTexture( command );
		case Op::kCopyTexture:
		{
			TextureRecord &src = T( command.a );
			TextureRecord &dst = T( command.b );
			Texture( src, D3D12_RESOURCE_STATE_COPY_SOURCE );
			Texture( dst, D3D12_RESOURCE_STATE_COPY_DEST );
			const TextureBufferCopy &copy = command.textureCopy;
			const D3D12_TEXTURE_COPY_LOCATION from = TextureLocation( src, copy.mip, copy.layer );
			const D3D12_TEXTURE_COPY_LOCATION to = TextureLocation( dst, copy.mip, copy.layer );
			const D3D12_BOX box{ copy.x, copy.y, 0, copy.x + copy.width, copy.y + copy.height, 1 };
			m_L->CopyTextureRegion( &to, copy.x, copy.y, 0, &from, &box );
			break;
		}
		case Op::kBeginRendering:
			BeginRendering( command );
			break;
		case Op::kEndRendering:
			EndRendering();
			break;
		case Op::kSetViewport:
		{
			const Viewport &v = command.viewport;
			const D3D12_VIEWPORT viewport{ v.x, v.y, v.width, v.height, v.minDepth, v.maxDepth };
			m_L->RSSetViewports( 1, &viewport );
			break;
		}
		case Op::kBeginOcclusionQuery:
		case Op::kEndOcclusionQuery:
			break; // D43 is unclaimed: refused at Submit
		case Op::kClearRegion:
			ClearRegion( command.region );
			break;
		case Op::kWriteTimestamp:
		{
			BufferRecord &b = B( command.a );
			const UINT slot = m_D.m_NextTimestamp++ % m_D.m_TimestampSlots;
			m_L->EndQuery( m_D.m_Timestamps.Get(), D3D12_QUERY_TYPE_TIMESTAMP, slot );
			m_L->ResolveQueryData( m_D.m_Timestamps.Get(), D3D12_QUERY_TYPE_TIMESTAMP, slot, 1,
			    b.resource.Get(), command.offset );
			break;
		}
		case Op::kBeginLabel:
		case Op::kEndLabel:
			break;
		case Op::kSetPipeline:
		{
			m_Pipeline = &m_D.m_Pipelines.at( command.a );
			const bool compute = m_Pipeline->kind == PipelineKind::kCompute;
			m_L->SetPipelineState( m_Pipeline->state.Get() );
			if ( compute )
				m_L->SetComputeRootSignature( m_Pipeline->root.Get() );
			else
			{
				m_L->SetGraphicsRootSignature( m_Pipeline->root.Get() );
				m_L->IASetPrimitiveTopology( m_Pipeline->topology );
				m_L->OMSetStencilRef( m_Pipeline->stencilReference );
			}
			// A new root signature drops every root argument.
			m_GroupsDirty = true;
			m_VertexDirty = true;
			break;
		}
		case Op::kSetBindGroup:
			m_Groups[command.slot] = command.a;
			m_GroupsDirty = true;
			break;
		case Op::kSetVertexBuffer:
			m_Vertex[command.slot] = { command.a, command.offset };
			m_VertexDirty = true;
			break;
		case Op::kSetIndexBuffer:
		{
			const BufferRecord &b = B( command.a );
			const D3D12_INDEX_BUFFER_VIEW view{ b.resource->GetGPUVirtualAddress() + command.offset,
				static_cast<UINT>( b.desc.size - command.offset ),
				command.indexFormat == IndexFormat::kUint16 ? DXGI_FORMAT_R16_UINT
				                                            : DXGI_FORMAT_R32_UINT };
			m_L->IASetIndexBuffer( &view );
			break;
		}
		case Op::kSetDrawConstants:
			std::memcpy( m_Constants + command.offset, command.bytes.data(), command.bytes.size() );
			break;
		case Op::kDraw:
			Prepare( 0, command.firstInstance );
			m_L->DrawInstanced( command.count, command.instances, command.first, command.firstInstance );
			break;
		case Op::kDrawIndexed:
			Prepare( 0, command.firstInstance );
			m_L->DrawIndexedInstanced( command.count, command.instances, command.first,
			    command.vertexOffset, command.firstInstance );
			break;
		case Op::kDrawIndexedIndirect:
		case Op::kDrawIndexedIndirectCount:
		{
			Prepare( 0, 0 );
			BufferRecord &records = B( command.a );
			ID3D12CommandSignature *signature = m_D.IndirectSignature( command.first );
			if ( !signature )
				return Fail( DeviceStatus::kInternal, DeviceOperation::kSubmit );
			ID3D12Resource *count = nullptr;
			UINT64 countOffset = 0;
			if ( command.op == Op::kDrawIndexedIndirectCount )
			{
				count = B( command.b ).resource.Get();
				countOffset = command.copy.destinationOffset;
			}
			m_L->ExecuteIndirect( signature, command.count, records.resource.Get(), command.offset,
			    count, countOffset );
			break;
		}
		case Op::kDispatch:
		{
			Prepare( 0, 0 );
			m_L->Dispatch( command.count, command.first, command.firstInstance );
			// Storage writes are visible to the next command, as the port's
			// storage usage promises (the GL adapter's glMemoryBarrier).
			D3D12_RESOURCE_BARRIER barrier{};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			m_L->ResourceBarrier( 1, &barrier );
			break;
		}
		}
		return {};
	}

	void ClearTexture( const Command &command )
	{
		TextureRecord &t = T( command.a );
		const D3D12_RESOURCE_STATES restore = t.state;
		const SubresourceRange &r = command.range;
		const std::uint32_t mips = std::min( r.baseMip + r.mipCount, t.desc.mipLevels );
		const std::uint32_t layers = t.desc.dimension == TextureDimension::k3D
		                                 ? r.baseLayer + 1
		                                 : std::min( r.baseLayer + r.layerCount, t.layers );
		const bool depth = IsDepthFormat( t.desc.format );
		Texture( t, depth ? D3D12_RESOURCE_STATE_DEPTH_WRITE : D3D12_RESOURCE_STATE_RENDER_TARGET );
		const float color[4] = { command.color.r, command.color.g, command.color.b,
			command.color.a };
		for ( std::uint32_t mip = r.baseMip; mip < mips; ++mip )
		{
			for ( std::uint32_t layer = r.baseLayer; layer < layers; ++layer )
			{
				if ( depth )
				{
					MakeDsv( t, mip, layer, false );
					m_L->ClearDepthStencilView( Dsv(),
					    D3D12_CLEAR_FLAG_DEPTH |
					        ( HasStencil( t.desc.format ) ? D3D12_CLEAR_FLAG_STENCIL
					                                      : D3D12_CLEAR_FLAGS( 0 ) ),
					    command.color.r, 0, 0, nullptr );
				}
				else
				{
					// An sRGB view encodes the port's linear color, as a GL
					// texture clear of linear values does.
					MakeRtv( t, mip, layer, 0 );
					m_L->ClearRenderTargetView( Rtv( 0 ), color, 0, nullptr );
				}
			}
		}
		Texture( t, restore );
	}

	DeviceResult<void> CopyTextureToBuffer( const Command &command )
	{
		TextureRecord &t = T( command.a );
		BufferRecord &b = B( command.b );
		const TextureBufferCopy &copy = command.textureCopy;
		Texture( t, D3D12_RESOURCE_STATE_COPY_SOURCE );
		Buffer( b, D3D12_RESOURCE_STATE_COPY_DEST );
		const Region region = RegionOf( t.desc.format, copy.width, copy.height );
		const D3D12_TEXTURE_COPY_LOCATION from = TextureLocation( t, copy.mip, copy.layer );
		const D3D12_BOX box = Box( copy, region );
		if ( Direct( region, copy.bufferOffset ) )
		{
			const D3D12_TEXTURE_COPY_LOCATION to =
			    Footprint( b.resource.Get(), copy.bufferOffset, t.desc.format, region );
			m_L->CopyTextureRegion( &to, 0, 0, 0, &from, &box );
			return {};
		}
		auto scratch = Scratch( region.pitch * region.rows, D3D12_HEAP_TYPE_DEFAULT,
		    D3D12_RESOURCE_STATE_COPY_DEST );
		if ( !scratch )
			return foundation::MakeUnexpected( scratch.Error() );
		const D3D12_TEXTURE_COPY_LOCATION to = Footprint( scratch.Value(), 0, t.desc.format, region );
		m_L->CopyTextureRegion( &to, 0, 0, 0, &from, &box );
		D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COPY_DEST;
		Transition( scratch.Value(), state, D3D12_RESOURCE_STATE_COPY_SOURCE );
		for ( std::uint32_t row = 0; row < region.rows; ++row )
			m_L->CopyBufferRegion( b.resource.Get(), copy.bufferOffset + row * region.rowBytes,
			    scratch.Value(), row * region.pitch, region.rowBytes );
		return {};
	}

	DeviceResult<void> CopyBufferToTexture( const Command &command )
	{
		BufferRecord &b = B( command.a );
		TextureRecord &t = T( command.b );
		const TextureBufferCopy &copy = command.textureCopy;
		Buffer( b, D3D12_RESOURCE_STATE_COPY_SOURCE );
		Texture( t, D3D12_RESOURCE_STATE_COPY_DEST );
		const Region region = RegionOf( t.desc.format, copy.width, copy.height );
		const D3D12_TEXTURE_COPY_LOCATION to = TextureLocation( t, copy.mip, copy.layer );
		const D3D12_BOX box{ 0, 0, 0, region.width, region.height, 1 };
		if ( Direct( region, copy.bufferOffset ) )
		{
			const D3D12_TEXTURE_COPY_LOCATION from =
			    Footprint( b.resource.Get(), copy.bufferOffset, t.desc.format, region );
			m_L->CopyTextureRegion( &to, copy.x, copy.y, 0, &from, &box );
			return {};
		}
		auto scratch = Scratch( region.pitch * region.rows, D3D12_HEAP_TYPE_DEFAULT,
		    D3D12_RESOURCE_STATE_COPY_DEST );
		if ( !scratch )
			return foundation::MakeUnexpected( scratch.Error() );
		for ( std::uint32_t row = 0; row < region.rows; ++row )
			m_L->CopyBufferRegion( scratch.Value(), row * region.pitch, b.resource.Get(),
			    copy.bufferOffset + row * region.rowBytes, region.rowBytes );
		D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COPY_DEST;
		Transition( scratch.Value(), state, D3D12_RESOURCE_STATE_COPY_SOURCE );
		const D3D12_TEXTURE_COPY_LOCATION from =
		    Footprint( scratch.Value(), 0, t.desc.format, region );
		m_L->CopyTextureRegion( &to, copy.x, copy.y, 0, &from, &box );
		return {};
	}

	void BeginRendering( const Command &command )
	{
		D3D12_CPU_DESCRIPTOR_HANDLE colors[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
		const auto count = static_cast<std::uint32_t>( command.colors.size() );
		m_Resolves.clear();
		for ( std::uint32_t i = 0; i < count; ++i )
		{
			const ColorAttachment &color = command.colors[i];
			TextureRecord &t = T( color.texture.value );
			MakeRtv( t, 0, 0, i );
			colors[i] = Rtv( i );
			if ( color.load == LoadOp::kClear )
			{
				const float clear[4] = { color.clear.r, color.clear.g, color.clear.b,
					color.clear.a };
				m_L->ClearRenderTargetView( colors[i], clear, 0, nullptr );
			}
			if ( color.resolve.IsValid() )
				m_Resolves.push_back( { color.texture.value, color.resolve.value } );
		}
		const D3D12_CPU_DESCRIPTOR_HANDLE depth = Dsv();
		if ( command.depth )
		{
			TextureRecord &t = T( command.depth->texture.value );
			const bool readOnly = !( t.state & D3D12_RESOURCE_STATE_DEPTH_WRITE );
			MakeDsv( t, 0, 0, readOnly );
			if ( command.depth->load == LoadOp::kClear )
				m_L->ClearDepthStencilView( depth,
				    D3D12_CLEAR_FLAG_DEPTH | ( HasStencil( t.desc.format )
				                                   ? D3D12_CLEAR_FLAG_STENCIL
				                                   : D3D12_CLEAR_FLAGS( 0 ) ),
				    command.depth->clearDepth, 0, 0, nullptr );
		}
		m_L->OMSetRenderTargets( count, colors, FALSE, command.depth ? &depth : nullptr );
		m_RenderWidth = command.width;
		m_RenderHeight = command.height;
		m_RenderDepthStencil =
		    command.depth && HasStencil( T( command.depth->texture.value ).desc.format );
		const D3D12_VIEWPORT viewport{ 0.0f, 0.0f, float( command.width ), float( command.height ),
			0.0f, 1.0f };
		const D3D12_RECT scissor{ 0, 0, LONG( command.width ), LONG( command.height ) };
		m_L->RSSetViewports( 1, &viewport );
		m_L->RSSetScissorRects( 1, &scissor );
	}

	// D44: the views' rectangle clears, clipped to the render area.
	void ClearRegion( const render::device::ClearRegion &r )
	{
		const LONG x1 =
		    LONG( std::min<std::uint64_t>( std::uint64_t( r.x ) + r.width, m_RenderWidth ) );
		const LONG y1 =
		    LONG( std::min<std::uint64_t>( std::uint64_t( r.y ) + r.height, m_RenderHeight ) );
		if ( LONG( r.x ) >= x1 || LONG( r.y ) >= y1 )
			return;
		const D3D12_RECT rect{ LONG( r.x ), LONG( r.y ), x1, y1 };
		if ( r.color )
		{
			const float value[4] = {
			    r.colorValue.r, r.colorValue.g, r.colorValue.b, r.colorValue.a };
			m_L->ClearRenderTargetView( Rtv( 0 ), value, 1, &rect );
		}
		const D3D12_CLEAR_FLAGS flags = D3D12_CLEAR_FLAGS(
		    ( r.depth ? D3D12_CLEAR_FLAG_DEPTH : 0 ) |
		    ( r.stencil && m_RenderDepthStencil ? D3D12_CLEAR_FLAG_STENCIL : 0 ) );
		if ( flags )
			m_L->ClearDepthStencilView( Dsv(), flags, r.depthValue, r.stencilValue, 1, &rect );
	}

	void EndRendering()
	{
		for ( const auto &[sourceId, targetId] : m_Resolves )
		{
			TextureRecord &source = T( sourceId );
			TextureRecord &target = T( targetId );
			const D3D12_RESOURCE_STATES restore = source.state;
			Texture( source, D3D12_RESOURCE_STATE_RESOLVE_SOURCE );
			Texture( target, D3D12_RESOURCE_STATE_RESOLVE_DEST );
			m_L->ResolveSubresource( target.resource.Get(), 0, source.resource.Get(), 0,
			    FormatOf( source.desc.format ).attachment );
			Texture( source, restore );
		}
		m_Resolves.clear();
	}

	// Root arguments and vertex buffers before a draw or dispatch.
	void Prepare( std::int32_t baseVertex, std::uint32_t baseInstance )
	{
		const PipelineRecord &p = *m_Pipeline;
		const bool compute = p.kind == PipelineKind::kCompute;
		if ( m_GroupsDirty )
		{
			for ( std::uint32_t role = 0; role < kMaxBindGroups; ++role )
			{
				const auto found = m_D.m_BindGroups.find( m_Groups[role] );
				if ( found == m_D.m_BindGroups.end() )
					continue;
				const BindGroupRecord &group = found->second;
				if ( p.params.views[role] >= 0 )
				{
					const auto table = m_D.ViewGpu( group.viewBase );
					if ( compute )
						m_L->SetComputeRootDescriptorTable( p.params.views[role], table );
					else
						m_L->SetGraphicsRootDescriptorTable( p.params.views[role], table );
				}
				if ( p.params.samplers[role] >= 0 )
				{
					const auto table = m_D.SamplerGpu( group.samplerBase );
					if ( compute )
						m_L->SetComputeRootDescriptorTable( p.params.samplers[role], table );
					else
						m_L->SetGraphicsRootDescriptorTable( p.params.samplers[role], table );
				}
			}
			m_GroupsDirty = false;
		}
		if ( p.params.drawConstants >= 0 )
		{
			const UINT values = ( p.drawConstantBytes + 3 ) / 4;
			if ( compute )
				m_L->SetComputeRoot32BitConstants( p.params.drawConstants, values, m_Constants, 0 );
			else
				m_L->SetGraphicsRoot32BitConstants( p.params.drawConstants, values, m_Constants, 0 );
		}
		if ( compute )
			return;
		// SV_VertexID already counts the base vertex; SV_InstanceID leaves out
		// the first instance, which SPIRV-Cross adds from here.
		const std::int32_t info[2] = { baseVertex, static_cast<std::int32_t>( baseInstance ) };
		m_L->SetGraphicsRoot32BitConstants( p.params.vertexInfo, 2, info, 0 );
		if ( m_VertexDirty )
		{
			D3D12_VERTEX_BUFFER_VIEW views[kMaxVertexSlots]{};
			for ( std::uint32_t slot = 0; slot < p.vertexBuffers; ++slot )
			{
				const BufferRecord &b = B( m_Vertex[slot].first );
				views[slot].BufferLocation =
				    b.resource->GetGPUVirtualAddress() + m_Vertex[slot].second;
				views[slot].SizeInBytes = static_cast<UINT>( b.desc.size - m_Vertex[slot].second );
				views[slot].StrideInBytes = p.strides[slot];
			}
			if ( p.vertexBuffers )
				m_L->IASetVertexBuffers( 0, p.vertexBuffers, views );
			m_VertexDirty = false;
		}
	}

	D3d12Device &m_D;
	ID3D12GraphicsCommandList *m_L;
	const PipelineRecord *m_Pipeline = nullptr;
	std::array<std::uint64_t, kMaxBindGroups> m_Groups{};
	std::array<std::pair<std::uint64_t, std::uint64_t>, kMaxVertexSlots> m_Vertex{};
	std::byte m_Constants[kMaxDrawConstantBytes] = {};
	bool m_GroupsDirty = true;
	bool m_VertexDirty = true;
	std::vector<Com<ID3D12Resource>> &m_Scratch;
	std::uint32_t m_RenderWidth = 0; // the open rendering's extent (D44)
	std::uint32_t m_RenderHeight = 0;
	bool m_RenderDepthStencil = false; // its depth attachment has stencil
	std::vector<std::pair<std::uint64_t, std::uint64_t>> m_Resolves;
};

DeviceResult<void> D3d12Device::Issue(
    std::vector<RecordingEncoder *> &encoders, CompletionToken token )
{
	const DeviceOperation op = DeviceOperation::kSubmit;
	InFlight flight;
	flight.value = token.value;
	if ( !m_FreeAllocators.empty() )
	{
		flight.allocator = std::move( m_FreeAllocators.back() );
		m_FreeAllocators.pop_back();
	}
	else
	{
		const HRESULT result = m_Device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT,
		    IID_ID3D12CommandAllocator, flight.allocator.PutVoid() );
		if ( FAILED( result ) )
			return Fail( DeviceStatus::kOutOfMemory, op, result );
	}
	HRESULT result = m_Device->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT,
	    flight.allocator.Get(), nullptr, IID_ID3D12GraphicsCommandList, flight.list.PutVoid() );
	if ( FAILED( result ) )
		return Fail( DeviceStatus::kOutOfMemory, op, result );
	// Every resource a held submission names must still exist when it is
	// issued; one freed early (a release that did not wait for its token) is
	// a lost submission, never a dangling record.
	for ( RecordingEncoder *encoder : encoders )
	{
		for ( const Command &command : encoder->Commands() )
		{
			auto present = [this]( std::uint64_t id )
			{
				return id == 0 || ExistingBuffer( id ) || ExistingTexture( id ) ||
				       m_Pipelines.count( id ) || m_BindGroups.count( id );
			};
			bool ok = present( command.a ) && present( command.b );
			for ( const ColorAttachment &color : command.colors )
				ok &= present( color.texture.value ) && present( color.resolve.value );
			if ( command.depth )
				ok &= present( command.depth->texture.value );
			if ( !ok )
				return Fail( DeviceStatus::kDeviceLost, op );
		}
	}
	Replay replay( *this, flight.list.Get(), flight.scratch );
	for ( RecordingEncoder *encoder : encoders )
	{
		if ( auto ran = replay.Run( encoder->Commands() ); !ran )
		{
			flight.list->Close();
			return ran;
		}
	}
	result = flight.list->Close();
	if ( FAILED( result ) )
		return Fail( DeviceStatus::kInternal, op, result );
	ID3D12CommandList *lists[] = { flight.list.Get() };
	m_Queue->ExecuteCommandLists( 1, lists );
	result = m_Queue->Signal( m_Fence.Get(), token.value );
	if ( FAILED( result ) )
		return Fail( DeviceStatus::kDeviceLost, op, result );
	m_InFlight.push_back( std::move( flight ) );
	CountDebugMessages();
	return {};
}

} // namespace render::device::d3d12
