//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: GPU time of labeled sections (RFC 0014 D4); see pass_timers.h.
//
//=============================================================================//

#include "render/graph/pass_timers.h"

#include <algorithm>
#include <cstring>

namespace render::graph
{

namespace
{

// Frames recorded but not read yet, at most: a device that never completes
// leaves later frames untimed rather than growing.
constexpr std::size_t kMaxFrames = 8;

} // namespace

GpuPassTimers::GpuPassTimers( device::IRenderDevice2 &device, std::uint32_t maxTimestamps )
    : m_Device( device ), m_MaxChunks( std::max( 1u, ( maxTimestamps + kChunk - 1 ) / kChunk ) ),
      m_Supported( device.Facts().capabilities.Has( device::Capability::kTimestamps ) &&
                   device.Facts().timestampPeriodNs > 0.0 )
{
}

GpuPassTimers::~GpuPassTimers()
{
	for ( const std::unique_ptr<Frame> &frame : m_Frames )
	{
		for ( device::BufferId buffer : frame->chunks )
			(void)m_Device.Release( buffer, m_Latest );
	}
}

void GpuPassTimers::Track( device::CompletionToken token )
{
	if ( token.NamesSubmission() &&
	     ( !m_Latest.NamesSubmission() || token.epoch > m_Latest.epoch ||
	         ( token.epoch == m_Latest.epoch && token.value > m_Latest.value ) ) )
		m_Latest = token;
}

void GpuPassTimers::Finish( device::CompletionToken done )
{
	if ( !m_Recording )
		return;
	m_Recording->waiting = true;
	m_Recording->hasDone = true;
	m_Recording->done = done;
	m_Recording = nullptr;
	m_Attached.clear();
}

void GpuPassTimers::BeginFrame( std::uint64_t frame, device::CompletionToken submitted )
{
	if ( !m_Supported )
		return;
	std::lock_guard<std::mutex> lock( m_Lock );
	Track( submitted );
	if ( m_Recording && m_Recording->frame == frame )
		return;
	// The frame recorded before this one was submitted before `submitted`.
	Finish( submitted );
	CollectLocked();
	for ( const std::unique_ptr<Frame> &candidate : m_Frames )
	{
		if ( !candidate->waiting )
		{
			m_Recording = candidate.get();
			break;
		}
	}
	if ( !m_Recording && m_Frames.size() < kMaxFrames )
	{
		m_Frames.push_back( std::make_unique<Frame>() );
		m_Recording = m_Frames.back().get();
	}
	if ( !m_Recording )
		return;
	m_Recording->frame = frame;
	m_Recording->chunksUsed = 0;
	m_Recording->hasDone = false;
	m_Recording->sections.clear();
	m_Recording->overflowed = 0;
}

void GpuPassTimers::EndFrame( device::CompletionToken token )
{
	if ( !m_Supported )
		return;
	std::lock_guard<std::mutex> lock( m_Lock );
	Track( token );
	Finish( token );
}

void GpuPassTimers::Collect()
{
	if ( !m_Supported )
		return;
	std::lock_guard<std::mutex> lock( m_Lock );
	CollectLocked();
}

void GpuPassTimers::CollectLocked()
{
	for ( const std::unique_ptr<Frame> &frame : m_Frames )
	{
		if ( frame->waiting && frame->hasDone && m_Device.IsComplete( frame->done ) )
		{
			Read( *frame );
			frame->waiting = false;
		}
	}
}

void GpuPassTimers::Read( Frame &frame )
{
	std::vector<std::uint64_t> ticks( std::size_t( frame.chunksUsed ) * kChunk );
	for ( std::uint32_t c = 0; c < frame.chunksUsed; ++c )
	{
		const auto out = std::as_writable_bytes( std::span( ticks ).subspan( c * kChunk, kChunk ) );
		if ( !m_Device.ReadBuffer( frame.chunks[c], 0, out ) )
			return; // the frame is lost, not reported
	}
	PassTimerReport latest;
	latest.frames = 1;
	latest.lastFrame = frame.frame;
	latest.overflowed = frame.overflowed;
	const double toMs = m_Device.Facts().timestampPeriodNs * 1e-6;
	for ( const Section &section : frame.sections )
	{
		if ( !section.closed || section.end >= ticks.size() || section.begin >= ticks.size() )
			continue;
		const std::uint64_t begin = ticks[section.begin];
		const std::uint64_t end = ticks[section.end];
		const double ms = end > begin ? double( end - begin ) * toMs : 0.0;
		for ( PassTimerReport *report : { &m_Report, &latest } )
		{
			auto found = std::find_if( report->passes.begin(), report->passes.end(),
			    [&]( const PassTime &time )
			    {
				    return time.depth == section.depth && time.name == section.name;
			    } );
			if ( found == report->passes.end() )
			{
				report->passes.push_back( { section.name, section.depth, 0.0, 0 } );
				found = std::prev( report->passes.end() );
			}
			found->milliseconds += ms;
			found->cpuMilliseconds += section.cpuMilliseconds;
			++found->count;
		}
	}
	if ( latest.lastFrame >= m_LatestReport.lastFrame )
		m_LatestReport = std::move( latest );
	m_Report.lastFrame = std::max( m_Report.lastFrame, frame.frame );
	++m_Report.frames;
}

PassTimerReport GpuPassTimers::Take()
{
	std::lock_guard<std::mutex> lock( m_Lock );
	CollectLocked();
	PassTimerReport report = std::move( m_Report );
	m_Report = PassTimerReport();
	return report;
}

PassTimerReport GpuPassTimers::Latest()
{
	std::lock_guard<std::mutex> lock( m_Lock );
	CollectLocked();
	return m_LatestReport;
}

void GpuPassTimers::Attach( device::CommandEncoder &encoder )
{
	if ( !m_Supported )
		return;
	encoder.SetLabelObserver( this );
}

void GpuPassTimers::Detach( device::CommandEncoder &encoder )
{
	if ( encoder.LabelObserver() == this )
		encoder.SetLabelObserver( nullptr );
	std::lock_guard<std::mutex> lock( m_Lock );
	m_Attached.erase( &encoder );
}

bool GpuPassTimers::Write(
    device::CommandEncoder &encoder, Attached &attached, std::uint32_t *index )
{
	Frame &frame = *m_Recording;
	if ( attached.slot == kChunk )
	{
		if ( frame.chunksUsed == m_MaxChunks )
		{
			++m_Report.overflowed;
			++frame.overflowed;
			return false;
		}
		if ( frame.chunksUsed == frame.chunks.size() )
		{
			device::BufferDesc desc;
			desc.size = kChunk * sizeof( std::uint64_t );
			desc.usages = { device::ResourceUsage::kCopyDestination };
			desc.memory = device::MemoryKind::kReadback;
			desc.debugName = "gpu pass timers";
			auto buffer = m_Device.CreateBuffer( desc );
			if ( !buffer )
			{
				++m_Report.overflowed;
				++frame.overflowed;
				return false;
			}
			frame.chunks.push_back( buffer.Value() );
		}
		attached.chunk = frame.chunksUsed++;
		attached.slot = 0;
		// The chunk is this encoder's alone for the frame.
		encoder.TransitionBuffer( frame.chunks[attached.chunk], device::ResourceUsage::kUndefined,
		    device::ResourceUsage::kCopyDestination );
	}
	encoder.WriteTimestamp(
	    frame.chunks[attached.chunk], std::uint64_t( attached.slot ) * sizeof( std::uint64_t ) );
	*index = attached.chunk * kChunk + attached.slot++;
	return true;
}

void GpuPassTimers::OnBeginLabel( device::CommandEncoder &encoder, std::string_view label )
{
	std::lock_guard<std::mutex> lock( m_Lock );
	if ( !m_Recording )
		return;
	Attached &attached = m_Attached[&encoder];
	std::uint32_t index = 0;
	if ( !Write( encoder, attached, &index ) )
	{
		attached.open.push_back( ~0u );
		return;
	}
	Section section;
	section.cpuBegin = std::chrono::steady_clock::now();
	section.name = std::string( label );
	section.depth = std::uint32_t( attached.open.size() );
	section.begin = index;
	attached.open.push_back( std::uint32_t( m_Recording->sections.size() ) );
	m_Recording->sections.push_back( std::move( section ) );
}

void GpuPassTimers::OnEndLabel( device::CommandEncoder &encoder )
{
	std::lock_guard<std::mutex> lock( m_Lock );
	const auto found = m_Attached.find( &encoder );
	if ( !m_Recording || found == m_Attached.end() || found->second.open.empty() )
		return;
	Attached &attached = found->second;
	const std::uint32_t open = attached.open.back();
	attached.open.pop_back();
	std::uint32_t index = 0;
	if ( open == ~0u || open >= m_Recording->sections.size() ||
	     !Write( encoder, attached, &index ) )
		return;
	m_Recording->sections[open].cpuMilliseconds = std::chrono::duration<double, std::milli>(
	    std::chrono::steady_clock::now() - m_Recording->sections[open].cpuBegin )
	                                                  .count();
	m_Recording->sections[open].end = index;
	m_Recording->sections[open].closed = true;
}

} // namespace render::graph
