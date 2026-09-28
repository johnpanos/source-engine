//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.legacy-frame-executor (RFC 0016 K3): the legacy frame as
//			stage passes of the core's frame graph (frame_source.h), on the
//			null adapter.
//
//			A fake frame source records each stage as a buffer write whose
//			byte count names the stage. The null device's recorded commands
//			must show exactly the stages the source has, in LegacyFrameStage
//			order, in one submission, each inside a pass label, and Finish
//			must see that submission's token. A skipped frame records and
//			submits nothing; a failed Prepare fails the frame without
//			Finish; a stage that fails its recording fails the frame after
//			Finish. LastFramePasses counts the stage passes.
//
//			Seeded defect (sensitivity row): the test's own
//			RENDER_LEGACY_FRAME_EXECUTOR_SEEDED_REVERSED source declares its
//			stages in reverse order, which the order clause must catch.
//
//=============================================================================//

#include "render/device/null/provider.h"
#include "render/legacy/frame_source.h"
#include "testing/checks.h"

#include <array>
#include <cstddef>
#include <cstdio>
#include <vector>

namespace
{

using namespace render;
using legacy::LegacyFrameStage;

constexpr unsigned int kStages = static_cast<unsigned int>( LegacyFrameStage::kCount );

class FakeSource final : public legacy::ILegacyFrameSource
{
public:
	FakeSource( device::IRenderDevice2 &device, device::BufferId buffer )
	    : m_Device( device ), m_Buffer( buffer )
	{
	}

	device::IRenderDevice2 &Device() override { return m_Device; }
	bool Prepare( bool *skip ) override
	{
		++prepares;
		*skip = skipFrame;
		return prepareOk;
	}
	bool HasStage( LegacyFrameStage stage ) override
	{
		return present[static_cast<unsigned int>( stage )];
	}
	bool RecordStage( LegacyFrameStage stage, device::CommandEncoder &encoder ) override
	{
#if defined( RENDER_LEGACY_FRAME_EXECUTOR_SEEDED_REVERSED )
		stage = static_cast<LegacyFrameStage>( kStages - 1 - static_cast<unsigned int>( stage ) );
#endif
		const unsigned int index = static_cast<unsigned int>( stage );
		if ( !m_Written )
		{
			encoder.TransitionBuffer( m_Buffer, device::ResourceUsage::kUndefined,
			    device::ResourceUsage::kCopyDestination );
			m_Written = true;
		}
		static const std::byte kBytes[4 * kStages] = {};
		encoder.WriteBuffer( m_Buffer, 0, std::span<const std::byte>( kBytes, 4 * ( index + 1 ) ) );
		return index != failStage;
	}
	bool Finish( const device::CompletionToken &token, bool submitted ) override
	{
		++finishes;
		finishedSubmitted = submitted;
		finishedToken = token;
		return true;
	}

	std::array<bool, kStages> present = { true, true, true, true, true };
	bool skipFrame = false;
	bool prepareOk = true;
	unsigned int failStage = kStages;
	int prepares = 0;
	int finishes = 0;
	bool finishedSubmitted = false;
	device::CompletionToken finishedToken;

private:
	device::IRenderDevice2 &m_Device;
	device::BufferId m_Buffer;
	bool m_Written = false;
};

struct Observed
{
	std::vector<unsigned int> stages; // from the write sizes
	std::vector<std::uint64_t> submissions;
	int labels = 0;
};

Observed Observe( device::null::INullDeviceControl &control )
{
	Observed observed;
	for ( const device::null::RecordedCommand &command : control.Recorded() )
	{
		if ( command.op == device::null::RecordedOp::kWriteBuffer )
		{
			observed.stages.push_back( static_cast<unsigned int>( command.count / 4 - 1 ) );
			observed.submissions.push_back( command.submission );
		}
		else if ( command.op == device::null::RecordedOp::kBeginLabel )
		{
			++observed.labels;
		}
	}
	return observed;
}

} // namespace

int main()
{
	testing::Checks checks;
	device::null::NullOptions options;
	options.completion = device::null::CompletionMode::kManual;
	auto created = device::null::Create( options );
	if ( !checks.That( created.HasValue(), "device.null-device-is-created" ) )
		return checks.Report();
	std::unique_ptr<device::IRenderDevice2> dev = std::move( created ).Value();
	device::null::INullDeviceControl &control = *device::null::Control( *dev );
	legacy::ILegacyFrameExecutor &executor = legacy::LegacyFrameExecutor();

	auto run = [&]( FakeSource &source, bool *ok )
	{
		control.ClearRecorded();
		*ok = executor.RunFrame( source );
		control.CompleteAll();
		return Observe( control );
	};
	auto makeBuffer = [&]()
	{
		device::BufferDesc desc;
		desc.size = 64;
		desc.usages = device::UsageSet{ device::ResourceUsage::kCopyDestination };
		return dev->CreateBuffer( desc ).Value();
	};

	// Every stage.
	{
		FakeSource source( *dev, makeBuffer() );
		bool ok = false;
		const Observed seen = run( source, &ok );
		checks.That( ok, "frame.all-stages-run" );
		checks.Equal( seen.stages, std::vector<unsigned int>{ 0, 1, 2, 3, 4 },
		    "frame.stages-record-in-order" );
		checks.That( seen.submissions.size() == 5 && seen.submissions.front() != 0 &&
		                 seen.submissions.front() == seen.submissions.back(),
		    "frame.one-submission" );
		checks.That( seen.labels >= 5, "frame.each-stage-is-a-labelled-pass" );
		checks.Equal( executor.LastFramePasses(), 5u, "frame.pass-per-stage" );
		checks.That( source.finishes == 1 && source.finishedSubmitted &&
		                 source.finishedToken.value == seen.submissions.front(),
		    "frame.finish-sees-the-submission" );
	}
	// Without resolve and capture (single-sampled, no readback).
	{
		FakeSource source( *dev, makeBuffer() );
		source.present[static_cast<unsigned int>( LegacyFrameStage::kResolve )] = false;
		source.present[static_cast<unsigned int>( LegacyFrameStage::kCapture )] = false;
		bool ok = false;
		const Observed seen = run( source, &ok );
		checks.That( ok, "frame.partial-stages-run" );
		checks.Equal(
		    seen.stages, std::vector<unsigned int>{ 0, 1, 4 }, "frame.absent-stages-have-no-pass" );
		checks.Equal( executor.LastFramePasses(), 3u, "frame.pass-count-follows-the-stages" );
	}
	// A skipped frame.
	{
		FakeSource source( *dev, makeBuffer() );
		source.skipFrame = true;
		bool ok = false;
		const Observed seen = run( source, &ok );
		checks.That( ok && seen.stages.empty() && source.finishes == 0,
		    "skip.nothing-recorded-or-finished" );
	}
	// A failed prepare.
	{
		FakeSource source( *dev, makeBuffer() );
		source.prepareOk = false;
		bool ok = true;
		const Observed seen = run( source, &ok );
		checks.That(
		    !ok && seen.stages.empty() && source.finishes == 0, "prepare-failure.fails-the-frame" );
	}
	// A stage that fails its recording.
	{
		FakeSource source( *dev, makeBuffer() );
		source.failStage = static_cast<unsigned int>( LegacyFrameStage::kScene );
		bool ok = true;
		(void)run( source, &ok );
		checks.That( !ok && source.finishes == 1, "record-failure.fails-the-frame-after-finish" );
	}
	std::printf( "INFO legacy frame executor: %llu frame(s) run\n",
	    static_cast<unsigned long long>( executor.Frames() ) );
	(void)dev->WaitIdle();
	return checks.Report();
}
