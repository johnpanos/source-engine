//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.d3d12: HLSL programs (compiled by the pinned DXC at
//			pipeline creation; tools/render/d3d12_lane.py owns the artifacts'
//			form), root signatures from the bind-group layouts, pipeline state
//			objects, and bind groups' descriptor tables.
//
//=============================================================================//

#include "d3d12_device.h"

#include <windows.h>
#include <dxcapi.h>

#include <cstring>
#include <sstream>

namespace render::device::d3d12
{

namespace
{

constexpr std::uint32_t kDrawConstantsRegister = 0;
constexpr std::uint32_t kVertexInfoRegister = 1;
constexpr std::uint32_t kConstantsSpace = 15;
constexpr const char *kSpecializationLine = "// render.device.d3d12 specialization ";

D3D12_COMPARISON_FUNC Compare( CompareOp op )
{
	switch ( op )
	{
	case CompareOp::kNever:
		return D3D12_COMPARISON_FUNC_NEVER;
	case CompareOp::kLess:
		return D3D12_COMPARISON_FUNC_LESS;
	case CompareOp::kEqual:
		return D3D12_COMPARISON_FUNC_EQUAL;
	case CompareOp::kLessEqual:
		return D3D12_COMPARISON_FUNC_LESS_EQUAL;
	case CompareOp::kGreater:
		return D3D12_COMPARISON_FUNC_GREATER;
	case CompareOp::kNotEqual:
		return D3D12_COMPARISON_FUNC_NOT_EQUAL;
	case CompareOp::kGreaterEqual:
		return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
	case CompareOp::kAlways:
		break;
	}
	return D3D12_COMPARISON_FUNC_ALWAYS;
}

D3D12_STENCIL_OP Stencil( StencilOp op )
{
	switch ( op )
	{
	case StencilOp::kZero:
		return D3D12_STENCIL_OP_ZERO;
	case StencilOp::kReplace:
		return D3D12_STENCIL_OP_REPLACE;
	case StencilOp::kIncrementClamp:
		return D3D12_STENCIL_OP_INCR_SAT;
	case StencilOp::kDecrementClamp:
		return D3D12_STENCIL_OP_DECR_SAT;
	case StencilOp::kInvert:
		return D3D12_STENCIL_OP_INVERT;
	case StencilOp::kIncrementWrap:
		return D3D12_STENCIL_OP_INCR;
	case StencilOp::kDecrementWrap:
		return D3D12_STENCIL_OP_DECR;
	case StencilOp::kKeep:
		break;
	}
	return D3D12_STENCIL_OP_KEEP;
}

D3D12_DESCRIPTOR_RANGE_TYPE RangeType( BindingKind kind )
{
	switch ( kind )
	{
	case BindingKind::kUniformBuffer:
		return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	case BindingKind::kSampledTexture:
		return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	case BindingKind::kStorageBuffer:
	case BindingKind::kStorageTexture:
		return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	case BindingKind::kSampler:
		break;
	}
	return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
}

DXGI_FORMAT VertexFormatOf( VertexFormat format )
{
	switch ( format )
	{
	case VertexFormat::kFloat2:
		return DXGI_FORMAT_R32G32_FLOAT;
	case VertexFormat::kFloat3:
		return DXGI_FORMAT_R32G32B32_FLOAT;
	case VertexFormat::kFloat4:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case VertexFormat::kUnorm8x4:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	return DXGI_FORMAT_UNKNOWN;
}

// The blend equations, as the GL adapter's (render/device/gl/execute.cpp).
D3D12_RENDER_TARGET_BLEND_DESC Blend( BlendMode mode, std::uint8_t mask )
{
	D3D12_RENDER_TARGET_BLEND_DESC blend{};
	blend.RenderTargetWriteMask = mask;
	blend.BlendOp = D3D12_BLEND_OP_ADD;
	blend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blend.LogicOp = D3D12_LOGIC_OP_NOOP;
	auto set = [&]( D3D12_BLEND src, D3D12_BLEND dst, D3D12_BLEND srcA, D3D12_BLEND dstA )
	{
		blend.BlendEnable = TRUE;
		blend.SrcBlend = src;
		blend.DestBlend = dst;
		blend.SrcBlendAlpha = srcA;
		blend.DestBlendAlpha = dstA;
	};
	switch ( mode )
	{
	case BlendMode::kOpaque:
		blend.SrcBlend = blend.SrcBlendAlpha = D3D12_BLEND_ONE;
		blend.DestBlend = blend.DestBlendAlpha = D3D12_BLEND_ZERO;
		break;
	case BlendMode::kAlpha:
		set( D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND_ONE,
		    D3D12_BLEND_INV_SRC_ALPHA );
		break;
	case BlendMode::kPremultiplied:
		set( D3D12_BLEND_ONE, D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND_ONE,
		    D3D12_BLEND_INV_SRC_ALPHA );
		break;
	case BlendMode::kAdditive:
		set( D3D12_BLEND_ONE, D3D12_BLEND_ONE, D3D12_BLEND_ONE, D3D12_BLEND_ONE );
		break;
	case BlendMode::kTransmittance:
		set( D3D12_BLEND_ONE, D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_ZERO, D3D12_BLEND_ONE );
		break;
	case BlendMode::kModulate2x:
		set( D3D12_BLEND_DEST_COLOR, D3D12_BLEND_SRC_COLOR, D3D12_BLEND_ZERO, D3D12_BLEND_ONE );
		break;
	case BlendMode::kAlphaAdditive:
		set( D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_ONE, D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_ONE );
		break;
	}
	return blend;
}

const wchar_t *Profile( ShaderStage stage )
{
	switch ( stage )
	{
	case ShaderStage::kVertex:
		return L"vs_6_6";
	case ShaderStage::kFragment:
		return L"ps_6_6";
	case ShaderStage::kCompute:
		break;
	}
	return L"cs_6_6";
}

std::wstring Wide( std::string_view text )
{
	return std::wstring( text.begin(), text.end() );
}

// "<id> <type>" of each specialization line the artifact declares.
std::vector<std::pair<std::uint32_t, std::string>> DeclaredConstants( std::string_view text )
{
	std::vector<std::pair<std::uint32_t, std::string>> declared;
	const std::string_view marker = kSpecializationLine;
	for ( std::size_t at = text.find( marker ); at != std::string_view::npos;
	      at = text.find( marker, at + 1 ) )
	{
		const std::size_t end = text.find( '\n', at );
		std::istringstream line( std::string( text.substr( at + marker.size(),
		    end == std::string_view::npos ? std::string_view::npos : end - at - marker.size() ) ) );
		std::uint32_t id = 0;
		std::string type;
		if ( line >> id >> type )
			declared.emplace_back( id, type );
	}
	return declared;
}

// A constant's value as an HLSL literal of its declared type.
std::string Literal( std::string_view type, std::uint32_t value )
{
	if ( type == "float" )
	{
		char text[32];
		std::snprintf( text, sizeof( text ), "asfloat(0x%08xu)", value );
		return text;
	}
	if ( type == "bool" )
		return value ? "true" : "false";
	if ( type == "int" )
		return std::to_string( static_cast<std::int32_t>( value ) );
	return std::to_string( value ) + "u";
}

} // namespace

// Descriptor ranges ----------------------------------------------------------------

std::optional<std::uint32_t> DescriptorRanges::Allocate( std::uint32_t count )
{
	if ( count == 0 )
		return 0u;
	for ( auto it = m_Free.begin(); it != m_Free.end(); ++it )
	{
		if ( it->count < count )
			continue;
		const std::uint32_t base = it->base;
		it->base += count;
		it->count -= count;
		if ( it->count == 0 )
			m_Free.erase( it );
		return base;
	}
	return std::nullopt;
}

void DescriptorRanges::Free( std::uint32_t base, std::uint32_t count )
{
	if ( count == 0 )
		return;
	auto it = m_Free.begin();
	while ( it != m_Free.end() && it->base < base )
		++it;
	it = m_Free.insert( it, { base, count } );
	if ( it + 1 != m_Free.end() && it->base + it->count == ( it + 1 )->base )
	{
		it->count += ( it + 1 )->count;
		m_Free.erase( it + 1 );
	}
	if ( it != m_Free.begin() && ( it - 1 )->base + ( it - 1 )->count == it->base )
	{
		( it - 1 )->count += it->count;
		m_Free.erase( it );
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE D3d12Device::ViewCpu( std::uint32_t index ) const
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_ViewHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += SIZE_T( index ) * m_ViewStride;
	return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE D3d12Device::SamplerCpu( std::uint32_t index ) const
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_SamplerHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += SIZE_T( index ) * m_SamplerStride;
	return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE D3d12Device::ViewGpu( std::uint32_t index ) const
{
	D3D12_GPU_DESCRIPTOR_HANDLE handle = m_ViewHeap->GetGPUDescriptorHandleForHeapStart();
	handle.ptr += UINT64( index ) * m_ViewStride;
	return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE D3d12Device::SamplerGpu( std::uint32_t index ) const
{
	D3D12_GPU_DESCRIPTOR_HANDLE handle = m_SamplerHeap->GetGPUDescriptorHandleForHeapStart();
	handle.ptr += UINT64( index ) * m_SamplerStride;
	return handle;
}

// The compiler ---------------------------------------------------------------------

DeviceResult<void> D3d12Device::LoadCompiler()
{
	// DXC is an import of the executable (dxcompiler.dll beside it), not a
	// loader site: a missing DLL fails the process start, never composition.
	if ( m_Compiler )
		return {};
	IDxcCompiler3 *compiler = nullptr;
	const HRESULT result = DxcCreateInstance(
	    CLSID_DxcCompiler, __uuidof( IDxcCompiler3 ), reinterpret_cast<void **>( &compiler ) );
	if ( FAILED( result ) || !compiler )
		return Fail( DeviceStatus::kUnavailable, DeviceOperation::kCreateDevice, result );
	*m_Compiler.Put() = compiler;
	return {};
}

DeviceResult<std::vector<std::byte>> D3d12Device::CompileStage( const ShaderArtifactView &stage,
    std::span<const SpecializationConstant> constants, std::int32_t &nativeCode )
{
	const DeviceOperation op = DeviceOperation::kCreatePipeline;
	const std::string_view text( reinterpret_cast<const char *>( stage.code.data() ),
	    stage.code.size() );
	// Each declared constant this stage is given becomes its macro; one it
	// does not declare is ignored (D20).
	std::vector<std::wstring> defines;
	std::string key = std::to_string( static_cast<int>( stage.stage ) ) + ":" +
	                  std::string( stage.entryPoint ) + ":";
	for ( const auto &[id, type] : DeclaredConstants( text ) )
	{
		for ( const SpecializationConstant &constant : constants )
		{
			if ( constant.stage != stage.stage || constant.id != id )
				continue;
			const std::string define =
			    "SPIRV_CROSS_CONSTANT_ID_" + std::to_string( id ) + "=" + Literal( type, constant.value );
			key += define + ";";
			defines.push_back( Wide( define ) );
		}
	}
	key += text;
	if ( const auto found = m_Programs.find( key ); found != m_Programs.end() )
		return found->second;

	auto *compiler = reinterpret_cast<IDxcCompiler3 *>( m_Compiler.Get() );
	const std::wstring entry = Wide( stage.entryPoint );
	// -Gis (strict IEEE): a specialization constant is a define DXC folds, so
	// without it the specialized and generic programs round differently (on
	// NVIDIA, render.family.pbr's specialized.identical-pixels); SPIR-V
	// specialization keeps both bit-identical, and so must this.
	std::vector<LPCWSTR> args = { L"-T", Profile( stage.stage ), L"-E", entry.c_str(), L"-O3",
		L"-Gis", L"-Qstrip_debug" };
	for ( const std::wstring &define : defines )
	{
		args.push_back( L"-D" );
		args.push_back( define.c_str() );
	}
	DxcBuffer source{ text.data(), text.size(), DXC_CP_UTF8 };
	IDxcResult *compiled = nullptr;
	HRESULT result = compiler->Compile( &source, args.data(), static_cast<UINT32>( args.size() ),
	    nullptr, __uuidof( IDxcResult ), reinterpret_cast<void **>( &compiled ) );
	if ( FAILED( result ) || !compiled )
		return Fail( DeviceStatus::kInternal, op, result );
	HRESULT status = S_OK;
	compiled->GetStatus( &status );
	if ( FAILED( status ) )
	{
		IDxcBlobEncoding *errors = nullptr;
		if ( SUCCEEDED( compiled->GetErrorBuffer( &errors ) ) && errors )
		{
			std::fprintf( stderr, "render.device.d3d12: DXC: %.*s\n",
			    static_cast<int>( errors->GetBufferSize() ),
			    static_cast<const char *>( errors->GetBufferPointer() ) );
			errors->Release();
		}
		compiled->Release();
		nativeCode = status;
		return Fail( DeviceStatus::kInvalidDescription, op, status );
	}
	IDxcBlob *object = nullptr;
	compiled->GetResult( &object );
	std::vector<std::byte> bytes;
	if ( object )
	{
		const auto *data = static_cast<const std::byte *>( object->GetBufferPointer() );
		bytes.assign( data, data + object->GetBufferSize() );
		object->Release();
	}
	compiled->Release();
	if ( bytes.empty() )
		return Fail( DeviceStatus::kInternal, op );
	m_Programs.emplace( std::move( key ), bytes );
	return bytes;
}

// Bind groups ------------------------------------------------------------------------

DeviceResult<void> D3d12Device::WriteGroupDescriptors( BindGroupRecord &group )
{
	const DeviceOperation op = DeviceOperation::kCreateBindGroup;
	std::uint32_t views = 0;
	std::uint32_t samplers = 0;
	for ( const BindingDesc &binding : group.bindings )
		( binding.kind == BindingKind::kSampler ? samplers : views ) += binding.count;
	const std::optional<std::uint32_t> viewBase = m_ViewRanges.Allocate( views );
	if ( !viewBase )
		return Fail( DeviceStatus::kOutOfMemory, op );
	const std::optional<std::uint32_t> samplerBase = m_SamplerRanges.Allocate( samplers );
	if ( !samplerBase )
	{
		m_ViewRanges.Free( *viewBase, views );
		return Fail( DeviceStatus::kOutOfMemory, op );
	}
	group.viewBase = *viewBase;
	group.viewCount = views;
	group.samplerBase = *samplerBase;
	group.samplerCount = samplers;

	// Each binding's first slot in its table, in layout order.
	std::uint32_t viewAt = 0;
	std::uint32_t samplerAt = 0;
	for ( const BindingDesc &binding : group.bindings )
	{
		const bool sampler = binding.kind == BindingKind::kSampler;
		std::uint32_t &at = sampler ? samplerAt : viewAt;
		for ( std::uint32_t i = 0; i < binding.count; ++i )
		{
			const std::uint32_t slot = at + i;
			const BindGroupEntry *entry = nullptr;
			for ( const BindGroupEntry &candidate : group.entries )
			{
				if ( candidate.binding == binding.binding + i )
					entry = &candidate;
			}
			switch ( binding.kind )
			{
			case BindingKind::kUniformBuffer:
			{
				D3D12_CONSTANT_BUFFER_VIEW_DESC view{};
				if ( entry && entry->buffer.IsValid() )
				{
					const BufferRecord &b = *LiveBuffer( entry->buffer.value );
					const std::uint64_t size =
					    entry->size ? entry->size : b.desc.size - entry->offset;
					view.BufferLocation = b.resource->GetGPUVirtualAddress() + entry->offset;
					view.SizeInBytes = static_cast<UINT>(
					    ( size + D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1 ) &
					    ~std::uint64_t( D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1 ) );
				}
				m_Device->CreateConstantBufferView(
				    view.BufferLocation ? &view : nullptr, ViewCpu( group.viewBase + slot ) );
				break;
			}
			case BindingKind::kStorageBuffer:
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC view{};
				view.Format = DXGI_FORMAT_R32_TYPELESS;
				view.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
				view.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
				ID3D12Resource *resource = nullptr;
				if ( entry && entry->buffer.IsValid() )
				{
					const BufferRecord &b = *LiveBuffer( entry->buffer.value );
					const std::uint64_t size =
					    entry->size ? entry->size : b.desc.size - entry->offset;
					resource = b.resource.Get();
					view.Buffer.FirstElement = entry->offset / 4;
					view.Buffer.NumElements = static_cast<UINT>( size / 4 );
				}
				else
				{
					view.Buffer.NumElements = 1;
				}
				m_Device->CreateUnorderedAccessView(
				    resource, nullptr, &view, ViewCpu( group.viewBase + slot ) );
				break;
			}
			case BindingKind::kSampledTexture:
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC view{};
				view.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				ID3D12Resource *resource = nullptr;
				view.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				view.Texture2D.MipLevels = 1;
				if ( entry && entry->texture.IsValid() )
				{
					const TextureRecord &t = *LiveTexture( entry->texture.value );
					resource = t.resource.Get();
					view.Format = FormatOf( t.desc.format ).view;
					const UINT mips = t.desc.mipLevels;
					if ( t.desc.sampleCount > 1 )
					{
						view.ViewDimension = t.layers > 1 ? D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY
						                                  : D3D12_SRV_DIMENSION_TEXTURE2DMS;
						view.Texture2DMSArray.ArraySize = t.layers;
					}
					else if ( t.desc.dimension == TextureDimension::k3D )
					{
						view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
						view.Texture3D.MipLevels = mips;
					}
					else if ( t.desc.dimension == TextureDimension::kCube && t.layers > 6 )
					{
						view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
						view.TextureCubeArray.MipLevels = mips;
						view.TextureCubeArray.NumCubes = t.layers / 6;
					}
					else if ( t.desc.dimension == TextureDimension::kCube )
					{
						view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
						view.TextureCube.MipLevels = mips;
					}
					else if ( t.layers > 1 )
					{
						view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
						view.Texture2DArray.MipLevels = mips;
						view.Texture2DArray.ArraySize = t.layers;
					}
					else
					{
						view.Texture2D.MipLevels = mips;
					}
				}
				m_Device->CreateShaderResourceView(
				    resource, &view, ViewCpu( group.viewBase + slot ) );
				break;
			}
			case BindingKind::kStorageTexture:
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC view{};
				ID3D12Resource *resource = nullptr;
				view.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				view.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				if ( entry && entry->texture.IsValid() )
				{
					const TextureRecord &t = *LiveTexture( entry->texture.value );
					resource = t.resource.Get();
					view.Format = FormatOf( t.desc.format ).view;
					if ( t.desc.dimension == TextureDimension::k3D )
					{
						view.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
						view.Texture3D.WSize = UINT( -1 );
					}
					else if ( t.layers > 1 )
					{
						view.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
						view.Texture2DArray.ArraySize = t.layers;
					}
				}
				m_Device->CreateUnorderedAccessView(
				    resource, nullptr, &view, ViewCpu( group.viewBase + slot ) );
				break;
			}
			case BindingKind::kSampler:
			{
				D3D12_SAMPLER_DESC view{};
				SamplerDesc desc;
				if ( entry && entry->sampler.IsValid() )
					desc = m_Samplers.at( entry->sampler.value ).desc;
				const bool minLinear = desc.minFilter == Filter::kLinear;
				const bool magLinear = desc.magFilter == Filter::kLinear;
				const bool mipLinear = desc.mipFilter == Filter::kLinear;
				const UINT reduction = desc.comparison ? D3D12_FILTER_REDUCTION_TYPE_COMPARISON
				                                       : D3D12_FILTER_REDUCTION_TYPE_STANDARD;
				if ( desc.maxAnisotropy > 1 )
					view.Filter = D3D12_ENCODE_ANISOTROPIC_FILTER( reduction );
				else
					view.Filter = D3D12_ENCODE_BASIC_FILTER(
					    minLinear ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT,
					    magLinear ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT,
					    mipLinear ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT, reduction );
				const D3D12_TEXTURE_ADDRESS_MODE address =
				    desc.address == AddressMode::kClampToEdge    ? D3D12_TEXTURE_ADDRESS_MODE_CLAMP
				    : desc.address == AddressMode::kMirroredRepeat ? D3D12_TEXTURE_ADDRESS_MODE_MIRROR
				                                                   : D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				view.AddressU = view.AddressV = view.AddressW = address;
				view.MaxAnisotropy = std::max( 1u, desc.maxAnisotropy );
				view.ComparisonFunc = Compare( desc.comparison.value_or( CompareOp::kNever ) );
				view.MaxLOD = D3D12_FLOAT32_MAX;
				m_Device->CreateSampler( &view, SamplerCpu( group.samplerBase + slot ) );
				break;
			}
			}
		}
		at += binding.count;
	}
	return {};
}

// Pipelines ----------------------------------------------------------------------------

DeviceResult<PipelineId> D3d12Device::CreatePipeline( const PipelineDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreatePipeline;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	auto valid = ValidatePipeline( desc, m_Facts,
	    [this]( BindGroupLayoutId id )
	    {
		    return FindLayout( id );
	    } );
	if ( !valid )
		return foundation::MakeUnexpected( valid.Error() );

	// What D3D12 needs beyond the shared rules: one stage of each kind, the
	// stages of the pipeline's kind and a supported sample count.
	const ShaderArtifactView *stages[3] = {};
	for ( const ShaderArtifactView &stage : desc.stages )
	{
		const auto index = static_cast<std::size_t>( stage.stage );
		if ( index >= 3 || stages[index] )
			return Fail( DeviceStatus::kInvalidDescription, op );
		stages[index] = &stage;
	}
	const bool compute = desc.kind == PipelineKind::kCompute;
	if ( compute ? ( !stages[2] || stages[0] || stages[1] ) : ( !stages[0] || stages[2] ) )
		return Fail( DeviceStatus::kInvalidDescription, op );
	if ( compute && !m_Facts.capabilities.Has( Capability::kCompute ) )
		return Fail( DeviceStatus::kUnsupported, op );
	if ( desc.raster.fill == FillMode::kLines &&
	     !m_Facts.capabilities.Has( Capability::kFillModeLines ) )
		return Fail( DeviceStatus::kUnsupported, op );
	if ( !compute && desc.sampleCount > 1 && !( m_Facts.limits.sampleCounts & desc.sampleCount ) )
		return Fail( DeviceStatus::kUnsupported, op );

	PipelineRecord record;
	record.kind = desc.kind;
	record.colorFormats.assign( desc.colorFormats.begin(), desc.colorFormats.end() );
	record.depthFormat = desc.depthFormat;
	record.sampleCount = desc.sampleCount;
	record.drawConstantBytes = desc.drawConstantBytes;
	record.vertexBuffers = static_cast<std::uint32_t>( desc.vertex.buffers.size() );
	if ( record.vertexBuffers > kMaxVertexSlots )
		return Fail( DeviceStatus::kUnsupported, op );
	for ( std::uint32_t slot = 0; slot < record.vertexBuffers; ++slot )
		record.strides[slot] = desc.vertex.buffers[slot].stride;

	// The root signature: per role, a view table and a sampler table in the
	// layout's binding order (register = binding, space = role), then the
	// draw constants and the base vertex and instance as root constants.
	std::vector<D3D12_ROOT_PARAMETER> params;
	std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> ranges( kMaxBindGroups * 2 );
	for ( std::size_t role = 0; role < desc.layouts.size(); ++role )
	{
		const BindGroupLayoutId layoutId = desc.layouts[role];
		const std::optional<LayoutView> layout = FindLayout( layoutId );
		if ( !layout )
			continue;
		const auto r = static_cast<std::uint32_t>( layout->role );
		record.layouts[r] = layoutId;
		record.layoutHasBindings[r] = !layout->bindings.empty();
		std::vector<D3D12_DESCRIPTOR_RANGE> &views = ranges[r * 2];
		std::vector<D3D12_DESCRIPTOR_RANGE> &samplers = ranges[r * 2 + 1];
		std::uint32_t viewAt = 0;
		std::uint32_t samplerAt = 0;
		for ( const BindingDesc &binding : layout->bindings )
		{
			D3D12_DESCRIPTOR_RANGE range{};
			range.RangeType = RangeType( binding.kind );
			range.NumDescriptors = binding.count;
			range.BaseShaderRegister = binding.binding;
			range.RegisterSpace = r;
			if ( binding.kind == BindingKind::kSampler )
			{
				range.OffsetInDescriptorsFromTableStart = samplerAt;
				samplerAt += binding.count;
				samplers.push_back( range );
			}
			else
			{
				range.OffsetInDescriptorsFromTableStart = viewAt;
				viewAt += binding.count;
				views.push_back( range );
			}
		}
	}
	for ( std::uint32_t r = 0; r < kMaxBindGroups; ++r )
	{
		for ( std::uint32_t kind = 0; kind < 2; ++kind )
		{
			const std::vector<D3D12_DESCRIPTOR_RANGE> &table = ranges[r * 2 + kind];
			if ( table.empty() )
				continue;
			D3D12_ROOT_PARAMETER param{};
			param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			param.DescriptorTable.NumDescriptorRanges = static_cast<UINT>( table.size() );
			param.DescriptorTable.pDescriptorRanges = table.data();
			param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			( kind == 0 ? record.params.views : record.params.samplers )[r] =
			    static_cast<std::int32_t>( params.size() );
			params.push_back( param );
		}
	}
	if ( desc.drawConstantBytes > 0 )
	{
		D3D12_ROOT_PARAMETER param{};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		param.Constants.ShaderRegister = kDrawConstantsRegister;
		param.Constants.RegisterSpace = kConstantsSpace;
		param.Constants.Num32BitValues = ( desc.drawConstantBytes + 3 ) / 4;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		record.params.drawConstants = static_cast<std::int32_t>( params.size() );
		params.push_back( param );
	}
	if ( !compute )
	{
		D3D12_ROOT_PARAMETER param{};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		param.Constants.ShaderRegister = kVertexInfoRegister;
		param.Constants.RegisterSpace = kConstantsSpace;
		param.Constants.Num32BitValues = 2;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		record.params.vertexInfo = static_cast<std::int32_t>( params.size() );
		params.push_back( param );
	}
	D3D12_ROOT_SIGNATURE_DESC rootDesc{};
	rootDesc.NumParameters = static_cast<UINT>( params.size() );
	rootDesc.pParameters = params.data();
	if ( !compute && !desc.vertex.attributes.empty() )
		rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	Com<ID3DBlob> serialized;
	Com<ID3DBlob> errors;
	HRESULT result = D3D12SerializeRootSignature(
	    &rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, serialized.Put(), errors.Put() );
	if ( FAILED( result ) )
		return Fail( DeviceStatus::kInternal, op, result );
	result = m_Device->CreateRootSignature( 0, serialized->GetBufferPointer(),
	    serialized->GetBufferSize(), IID_ID3D12RootSignature, record.root.PutVoid() );
	if ( FAILED( result ) )
		return Fail( DeviceStatus::kInternal, op, result );

	std::vector<std::byte> code[3];
	for ( std::size_t i = 0; i < 3; ++i )
	{
		if ( !stages[i] )
			continue;
		std::int32_t nativeCode = 0;
		auto compiled = CompileStage( *stages[i], desc.constants, nativeCode );
		if ( !compiled )
			return foundation::MakeUnexpected( compiled.Error() );
		code[i] = std::move( compiled ).Value();
	}

	if ( compute )
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC state{};
		state.pRootSignature = record.root.Get();
		state.CS = { code[2].data(), code[2].size() };
		result = m_Device->CreateComputePipelineState(
		    &state, IID_ID3D12PipelineState, record.state.PutVoid() );
	}
	else
	{
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputs;
		for ( const VertexAttribute &attribute : desc.vertex.attributes )
		{
			if ( attribute.bufferSlot >= record.vertexBuffers )
				return Fail( DeviceStatus::kInvalidDescription, op );
			const bool instance = desc.vertex.buffers[attribute.bufferSlot].perInstance;
			D3D12_INPUT_ELEMENT_DESC input{};
			input.SemanticName = "TEXCOORD";
			input.SemanticIndex = attribute.location;
			input.Format = VertexFormatOf( attribute.format );
			input.InputSlot = attribute.bufferSlot;
			input.AlignedByteOffset = attribute.offset;
			input.InputSlotClass = instance ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA
			                                : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			input.InstanceDataStepRate = instance ? 1 : 0;
			inputs.push_back( input );
		}
		D3D12_GRAPHICS_PIPELINE_STATE_DESC state{};
		state.pRootSignature = record.root.Get();
		state.VS = { code[0].data(), code[0].size() };
		if ( stages[1] )
			state.PS = { code[1].data(), code[1].size() };
		state.InputLayout = { inputs.data(), static_cast<UINT>( inputs.size() ) };
		state.BlendState.AlphaToCoverageEnable = desc.raster.alphaToCoverage;
		state.BlendState.IndependentBlendEnable = TRUE;
		for ( std::size_t i = 0; i < desc.colorFormats.size() && i < 8; ++i )
		{
			const BlendMode mode = desc.blends.empty() ? BlendMode::kOpaque : desc.blends[i];
			const std::uint8_t mask =
			    desc.colorWriteMasks.empty() ? kColorWriteAll : desc.colorWriteMasks[i];
			state.BlendState.RenderTarget[i] = Blend( mode, mask );
			state.RTVFormats[i] = FormatOf( desc.colorFormats[i] ).attachment;
		}
		state.NumRenderTargets = static_cast<UINT>( std::min<std::size_t>( desc.colorFormats.size(), 8 ) );
		state.SampleMask = UINT_MAX;
		D3D12_RASTERIZER_DESC &raster = state.RasterizerState;
		// D38: kLines draws triangle edges (D3D12's wireframe fill).
		raster.FillMode = desc.raster.fill == FillMode::kLines ? D3D12_FILL_MODE_WIREFRAME
		                                                        : D3D12_FILL_MODE_SOLID;
		raster.CullMode = desc.raster.cull == CullMode::kBack    ? D3D12_CULL_MODE_BACK
		                  : desc.raster.cull == CullMode::kFront ? D3D12_CULL_MODE_FRONT
		                                                         : D3D12_CULL_MODE_NONE;
		raster.FrontCounterClockwise = desc.raster.frontCounterClockwise;
		raster.DepthBias = static_cast<INT>( desc.raster.depthBiasConstant );
		raster.SlopeScaledDepthBias = desc.raster.depthBiasSlope;
		raster.DepthClipEnable = TRUE;
		D3D12_DEPTH_STENCIL_DESC &depth = state.DepthStencilState;
		const DepthStencilState &ds = desc.depthStencil;
		depth.DepthEnable = ds.depthTest || ds.depthWrite;
		depth.DepthFunc = ds.depthTest ? Compare( ds.compare ) : D3D12_COMPARISON_FUNC_ALWAYS;
		depth.DepthWriteMask = ds.depthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
		depth.StencilEnable = ds.stencil.enabled;
		depth.StencilReadMask = ds.stencil.readMask;
		depth.StencilWriteMask = ds.stencil.writeMask;
		depth.FrontFace = { Stencil( ds.stencil.fail ), Stencil( ds.stencil.depthFail ),
			Stencil( ds.stencil.pass ), Compare( ds.stencil.compare ) };
		depth.BackFace = depth.FrontFace;
		record.stencilReference = ds.stencil.reference;
		state.DSVFormat = desc.depthFormat == Format::kUnknown
		                      ? DXGI_FORMAT_UNKNOWN
		                      : FormatOf( desc.depthFormat ).attachment;
		state.SampleDesc.Count = desc.sampleCount;
		switch ( desc.topology )
		{
		case PrimitiveTopology::kTriangleList:
			state.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			record.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			break;
		case PrimitiveTopology::kTriangleStrip:
			state.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			record.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			break;
		case PrimitiveTopology::kLineList:
			state.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
			record.topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
			break;
		case PrimitiveTopology::kPointList:
			state.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
			record.topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
			break;
		}
		result = m_Device->CreateGraphicsPipelineState(
		    &state, IID_ID3D12PipelineState, record.state.PutVoid() );
	}
	if ( FAILED( result ) )
		return Fail( result == E_OUTOFMEMORY ? DeviceStatus::kOutOfMemory : DeviceStatus::kInternal,
		    op, result );
	SetName( record.state.Get(), desc.debugName );
	const PipelineId id{ ++m_NextId };
	m_Pipelines.emplace( id.value, std::move( record ) );
	return id;
}

} // namespace render::device::d3d12
