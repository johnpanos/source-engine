//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.v2.sensitivity (RFC 0016 K1): ten deliberately bad
//			adapters, each a decorator over the null adapter that breaks one
//			clause. The shared suite must fail each on the clause it breaks,
//			while the undecorated control passes.
//
//			Not yet covered by a bad adapter (they need a rasterizing adapter
//			and run in the GPU lane): a flipped Y convention or a -1 to 1 depth
//			range (D13), and a claimed capability that corrupts (aliasing).
//
//=============================================================================//

#include "device_conformance.h"
#include "render/device/null/provider.h"

#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>

namespace
{

using namespace render::device;

enum class Defect
{
	kAcceptsBadComparison,
	kNone,
	kReleasesEarly,       // D5: frees at once, ignoring the token
	kFactsChange,         // D1: facts differ after the first use
	kCompletesOutOfOrder, // D6: reports a later token complete before earlier ones
	kAcceptsStaleEpoch,   // D7: drops old-epoch waits instead of failing
	kSkipsLayoutCheck,    // D4: strips reflection before creating
	kLeaksOnFailure,      // D2: creates a buffer before rejecting a texture
	kTruncatesBindGroups, // D3: silently keeps the first four layouts
	kRecyclesUploads,     // D10: upload ranges retire at submission, not completion
	kSubmitsPartially,    // D8: drops erroring encoders and runs the rest
	kIgnoresUsageState,   // D12: accepts transitions from any usage
	kFillsDrawConstants,  // D16: zero-fills a bound pipeline's draw constants
	kAttachesBlocks,      // D19: creates a block-compressed attachment without it
	kDropsRegionOrigin,   // D22: copies every buffer-to-texture region to (0, 0)
	kDropsTimestamps      // D23: records no timestamp
};

// Draw-constant block sizes by pipeline, for kFillsDrawConstants.
using BlockSizes = std::shared_ptr<std::map<std::uint64_t, std::uint32_t>>;

// Wraps a null encoder; some defects rewrite what it records.
class BadEncoder final : public IEncoderBackend
{
public:
	BadEncoder( std::unique_ptr<IEncoderBackend> inner, Defect defect, BlockSizes blocks )
	    : m_Inner( std::move( inner ) ), m_Defect( defect ), m_Blocks( std::move( blocks ) )
	{
	}

	void TransitionTexture( TextureId texture, ResourceUsage before, ResourceUsage after,
	    const SubresourceRange &range ) override
	{
		if ( m_Defect == Defect::kIgnoresUsageState )
			before = ResourceUsage::kUndefined;
		m_Inner->TransitionTexture( texture, before, after, range );
	}
	void TransitionBuffer( BufferId buffer, ResourceUsage before, ResourceUsage after ) override
	{
		m_Inner->TransitionBuffer( buffer, before, after );
	}
	void ClearTexture(
	    TextureId texture, const ClearColor &color, const SubresourceRange &range ) override
	{
		if ( m_Defect == Defect::kIgnoresUsageState )
			m_Inner->TransitionTexture(
			    texture, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination, range );
		m_Inner->ClearTexture( texture, color, range );
	}
	void WriteBuffer(
	    BufferId buffer, std::uint64_t offset, std::span<const std::byte> bytes ) override
	{
		m_Inner->WriteBuffer( buffer, offset, bytes );
	}
	void CopyBuffer( BufferId source, BufferId destination, const BufferCopy &copy ) override
	{
		m_Inner->CopyBuffer( source, destination, copy );
	}
	void CopyTextureToBuffer(
	    TextureId source, BufferId destination, const TextureBufferCopy &copy ) override
	{
		m_Inner->CopyTextureToBuffer( source, destination, copy );
	}
	void CopyBufferToTexture(
	    BufferId source, TextureId destination, const TextureBufferCopy &copy ) override
	{
		TextureBufferCopy region = copy;
		if ( m_Defect == Defect::kDropsRegionOrigin )
			region.x = region.y = 0;
		m_Inner->CopyBufferToTexture( source, destination, region );
	}
	void BeginRendering( const RenderingDesc &desc ) override { m_Inner->BeginRendering( desc ); }
	void EndRendering() override { m_Inner->EndRendering(); }
	void SetPipeline( PipelineId pipeline ) override
	{
		m_Inner->SetPipeline( pipeline );
		// The defect: the block is treated as defined (zeros) once bound.
		const auto block = m_Blocks->find( pipeline.value );
		if ( m_Defect == Defect::kFillsDrawConstants && block != m_Blocks->end() &&
		     block->second > 0 )
		{
			static const std::byte kZeros[kMaxDrawConstantBytes] = {};
			m_Inner->SetDrawConstants( 0, std::span<const std::byte>( kZeros, block->second ) );
		}
	}
	void SetBindGroup( BindGroupRole role, BindGroupId group ) override
	{
		m_Inner->SetBindGroup( role, group );
	}
	void SetVertexBuffer( std::uint32_t slot, BufferId buffer, std::uint64_t offset ) override
	{
		m_Inner->SetVertexBuffer( slot, buffer, offset );
	}
	void SetIndexBuffer( BufferId buffer, std::uint64_t offset, IndexFormat format ) override
	{
		m_Inner->SetIndexBuffer( buffer, offset, format );
	}
	void SetViewport( const Viewport &viewport ) override { m_Inner->SetViewport( viewport ); }
	void Draw( std::uint32_t a, std::uint32_t b, std::uint32_t c, std::uint32_t d ) override
	{
		m_Inner->Draw( a, b, c, d );
	}
	void DrawIndexed( std::uint32_t a, std::uint32_t b, std::uint32_t c, std::int32_t d,
	    std::uint32_t e ) override
	{
		m_Inner->DrawIndexed( a, b, c, d, e );
	}
	void Dispatch( std::uint32_t x, std::uint32_t y, std::uint32_t z ) override
	{
		m_Inner->Dispatch( x, y, z );
	}
	void SetDrawConstants( std::uint32_t offset, std::span<const std::byte> bytes ) override
	{
		m_Inner->SetDrawConstants( offset, bytes );
	}
	void BeginLabel( std::string_view label ) override { m_Inner->BeginLabel( label ); }
	void EndLabel() override { m_Inner->EndLabel(); }
	void WriteTimestamp( BufferId buffer, std::uint64_t offset ) override
	{
		if ( m_Defect != Defect::kDropsTimestamps )
			m_Inner->WriteTimestamp( buffer, offset );
	}
	bool HasError() const override { return m_Inner->HasError(); }

	std::unique_ptr<IEncoderBackend> TakeInner() { return std::move( m_Inner ); }

private:
	std::unique_ptr<IEncoderBackend> m_Inner;
	Defect m_Defect;
	BlockSizes m_Blocks;
};

class BadDevice final : public IRenderDevice2
{
public:
	BadDevice( std::unique_ptr<IRenderDevice2> inner, Defect defect )
	    : m_Inner( std::move( inner ) ), m_Defect( defect ), m_Facts( m_Inner->Facts() )
	{
	}

	render::device::null::INullDeviceControl *Control()
	{
		return render::device::null::Control( *m_Inner );
	}

	const DeviceFacts &Facts() const override
	{
		if ( m_Defect == Defect::kFactsChange && m_Used )
			m_Facts.limits.maxColorAttachments = 1;
		return m_Facts;
	}
	DeviceState State() const override { return m_Inner->State(); }
	std::uint32_t Epoch() const override { return m_Inner->Epoch(); }

	DeviceResult<BufferId> CreateBuffer( const BufferDesc &desc ) override
	{
		m_Used = true;
		return m_Inner->CreateBuffer( desc );
	}
	DeviceResult<TextureId> CreateTexture( const TextureDesc &desc ) override
	{
		if ( m_Defect == Defect::kLeaksOnFailure )
		{
			BufferDesc scratch;
			scratch.size = 64;
			scratch.usages = { ResourceUsage::kCopyDestination };
			auto leaked = m_Inner->CreateBuffer( scratch );
			auto texture = m_Inner->CreateTexture( desc );
			if ( texture && leaked )
				(void)m_Inner->Release( leaked.Value(), {} );
			return texture;
		}
		if ( m_Defect == Defect::kAttachesBlocks && IsBlockCompressed( desc.format ) &&
		     desc.usages.Has( ResourceUsage::kColorAttachment ) )
		{
			TextureDesc sampled = desc;
			sampled.usages = { ResourceUsage::kSampled };
			return m_Inner->CreateTexture( sampled );
		}
		return m_Inner->CreateTexture( desc );
	}
	DeviceResult<SamplerId> CreateSampler( const SamplerDesc &desc ) override
	{
		SamplerDesc valid = desc;
		if ( m_Defect == Defect::kAcceptsBadComparison )
			valid.comparison = CompareOp::kAlways;
		return m_Inner->CreateSampler( valid );
	}
	DeviceResult<BindGroupLayoutId> CreateBindGroupLayout(
	    const BindGroupLayoutDesc &desc ) override
	{
		return m_Inner->CreateBindGroupLayout( desc );
	}
	DeviceResult<BindGroupId> CreateBindGroup( const BindGroupDesc &desc ) override
	{
		return m_Inner->CreateBindGroup( desc );
	}
	DeviceResult<PipelineId> CreatePipeline( const PipelineDesc &desc ) override
	{
		PipelineDesc changed = desc;
		std::vector<ShaderArtifactView> stages( desc.stages.begin(), desc.stages.end() );
		if ( m_Defect == Defect::kSkipsLayoutCheck )
		{
			for ( ShaderArtifactView &stage : stages )
				stage.bindings = {};
			changed.stages = stages;
		}
		if ( m_Defect == Defect::kTruncatesBindGroups && desc.layouts.size() > kMaxBindGroups )
			changed.layouts = desc.layouts.first( kMaxBindGroups );
		auto created = m_Inner->CreatePipeline( changed );
		if ( created )
			( *m_Blocks )[created.Value().value] = desc.drawConstantBytes;
		return created;
	}
	DeviceResult<void> Release( ResourceId resource, CompletionToken releaseAfter ) override
	{
		if ( m_Defect == Defect::kReleasesEarly )
			releaseAfter = {};
		return m_Inner->Release( resource, releaseAfter );
	}
	DeviceResult<CommandEncoder> BeginEncoder( QueueKind queue ) override
	{
		auto inner = m_Inner->BeginEncoder( queue );
		if ( !inner )
			return inner;
		return CommandEncoder( queue,
		    std::make_unique<BadEncoder>( inner.Value().TakeBackend(), m_Defect, m_Blocks ) );
	}
	DeviceResult<CompletionToken> Submit(
	    QueueKind queue, std::span<CommandEncoder> encoders, const SubmitWaits &waits ) override
	{
		std::vector<CommandEncoder> unwrapped;
		for ( CommandEncoder &encoder : encoders )
		{
			std::unique_ptr<IEncoderBackend> backend = encoder.TakeBackend();
			auto *bad = dynamic_cast<BadEncoder *>( backend.get() );
			if ( !bad )
				continue;
			if ( m_Defect == Defect::kSubmitsPartially && bad->HasError() )
				continue;
			unwrapped.emplace_back( queue, bad->TakeInner() );
		}
		std::vector<CompletionToken> kept;
		for ( const CompletionToken &token : waits.tokens )
		{
			if ( m_Defect != Defect::kAcceptsStaleEpoch || token.epoch >= m_Inner->Epoch() )
				kept.push_back( token );
		}
		return m_Inner->Submit( queue, unwrapped, SubmitWaits{ kept } );
	}
	bool IsComplete( CompletionToken token ) const override
	{
		if ( m_Defect == Defect::kCompletesOutOfOrder && token.NamesSubmission() )
			return token.value % 2 == 0 || m_Inner->IsComplete( token );
		return m_Inner->IsComplete( token );
	}
	std::size_t Poll() override { return m_Inner->Poll(); }
	DeviceResult<void> ReadBuffer(
	    BufferId buffer, std::uint64_t offset, std::span<std::byte> out ) override
	{
		return m_Inner->ReadBuffer( buffer, offset, out );
	}
	DeviceResult<void> WaitIdle() override { return m_Inner->WaitIdle(); }
	DeviceResult<void> Recover() override { return m_Inner->Recover(); }
	std::size_t LiveResourceCount() const override { return m_Inner->LiveResourceCount(); }

private:
	std::unique_ptr<IRenderDevice2> m_Inner;
	Defect m_Defect;
	BlockSizes m_Blocks = std::make_shared<std::map<std::uint64_t, std::uint32_t>>();
	mutable DeviceFacts m_Facts;
	bool m_Used = false;
};

rendertest::DeviceDriver Driver( Defect defect )
{
	rendertest::DeviceDriver driver;
	driver.name = "under-test";
	driver.create = [defect]() -> std::unique_ptr<IRenderDevice2>
	{
		render::device::null::NullOptions options;
		options.completion = render::device::null::CompletionMode::kManual;
		options.uploadRingBytes = 256 * 1024;
		// The recording adapter's own sensitivity knob: a decorator cannot
		// reach its ring.
		options.unsafeUploadReuse = defect == Defect::kRecyclesUploads;
		auto device = render::device::null::Create( options );
		if ( !device )
			return nullptr;
		return std::make_unique<BadDevice>( std::move( device ).Value(), defect );
	};
	driver.complete = []( IRenderDevice2 &device, CompletionToken token )
	{
		static_cast<BadDevice &>( device ).Control()->CompleteThrough( token.queue, token.value );
		return device.IsComplete( token );
	};
	driver.lose = []( IRenderDevice2 &device )
	{
		static_cast<BadDevice &>( device ).Control()->LoseDevice();
	};
	driver.holdsCompletion = true;
	return driver;
}

std::string FailuresOf( const rendertest::DeviceDriver &driver )
{
	char *buffer = nullptr;
	size_t size = 0;
	std::FILE *stream = open_memstream( &buffer, &size );
	{
		testing::Checks inner( stream );
		rendertest::RunDeviceConformance( inner, driver );
	}
	std::fclose( stream );
	std::string out( buffer ? buffer : "", size );
	std::free( buffer );
	return out;
}

} // namespace

int main()
{
	testing::Checks checks;
	{
		const std::string failures = FailuresOf( Driver( Defect::kNone ) );
		checks.That( failures.empty(), "control-passes" );
		if ( !failures.empty() )
			std::printf( "%s", failures.c_str() );
	}
	struct Case
	{
		Defect defect;
		const char *clause;
	};
	const Case cases[] = {
	    { Defect::kReleasesEarly, "under-test.D5 " },
	    { Defect::kFactsChange, "under-test.D1 " },
	    { Defect::kCompletesOutOfOrder, "under-test.D6 " },
	    { Defect::kAcceptsStaleEpoch, "under-test.D7 " },
	    { Defect::kSkipsLayoutCheck, "under-test.D4 " },
	    { Defect::kLeaksOnFailure, "under-test.D2 " },
	    { Defect::kTruncatesBindGroups, "under-test.D3 " },
	    { Defect::kRecyclesUploads, "under-test.D10 " },
	    { Defect::kSubmitsPartially, "under-test.D8 " },
	    { Defect::kIgnoresUsageState, "under-test.D12 " },
	    { Defect::kFillsDrawConstants, "under-test.D16 " },
	    { Defect::kAttachesBlocks, "under-test.D19 " },
	    { Defect::kDropsRegionOrigin, "under-test.D22 " },
	    { Defect::kDropsTimestamps, "under-test.D23 " },
	    { Defect::kAcceptsBadComparison, "under-test.D24 " },
	};
	for ( const Case &c : cases )
	{
		const std::string failures = FailuresOf( Driver( c.defect ) );
		checks.That(
		    failures.find( c.clause ) != std::string::npos, std::string( "detects " ) + c.clause );
	}
	return checks.Report();
}
