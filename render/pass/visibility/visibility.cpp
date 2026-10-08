//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.visibility (RFC 0016 K8 sprites cohort); see
//			visibility.h.
//
//=============================================================================//

#include "render/pass/visibility/visibility.h"

#include "render/shaderlib/core_artifacts.h"
#include "render/shaderlib/pipeline_recipe.h"

#include <cstring>
#include <map>
#include <mutex>
#include <set>
#include <vector>

namespace render::pass::visibility
{

namespace
{

using namespace render::device;

constexpr const char *kVertex = "render/pass/visibility/visibility.vert";
constexpr const char *kFragment = "render/pass/visibility/visibility.frag";

// The legacy proxy's four triangles: apex, base corner, next base corner.
constexpr int kTriangles[4][3] = { { 0, 1, 2 }, { 0, 2, 3 }, { 0, 3, 4 }, { 0, 4, 1 } };

constexpr std::size_t kMaxKept = 4096;

struct Pending
{
	std::uint32_t serial = 0;
	BufferId vertices;
	BufferId readback;
	bool submitted = false;
	CompletionToken token;
};

// One target's pipelines: tested (LessEqual, no writes) and untested.
struct Variant
{
	Format colorFormat = Format::kUnknown;
	Format depthFormat = Format::kUnknown;
	std::uint32_t samples = 1;
	PipelineId tested;
	PipelineId untested;
	std::string failure;
};

} // namespace

struct VisibilityCounter::State
{
	mutable std::mutex lock;
	std::uint32_t nextSerial = 1;
	std::map<std::uint32_t, Query> queued;
	std::set<std::uint32_t> recorded;
	std::map<std::uint32_t, Counts> results;
	VisibilityStats stats;

	IRenderDevice2 *device = nullptr;
	std::vector<Variant> variants;
	std::vector<Pending> pending;

	void Fail( std::uint32_t serial, const std::string &why )
	{
		std::lock_guard<std::mutex> guard( lock );
		results[serial] = { -1, -1 };
		++stats.failed;
		stats.lastFailure = why;
	}

	Variant *VariantFor( const VisibilityTarget &target )
	{
		for ( Variant &v : variants )
		{
			if ( v.colorFormat == target.colorFormat && v.depthFormat == target.depthFormat &&
			     v.samples == target.samples )
				return &v;
		}
		Variant &v = variants.emplace_back();
		v.colorFormat = target.colorFormat;
		v.depthFormat = target.depthFormat;
		v.samples = target.samples;
		if ( !device->Facts().capabilities.Has( Capability::kOcclusionQueries ) )
		{
			v.failure = "the device counts no occlusion samples (clause D43)";
			return &v;
		}
		const VertexAttribute attributes[] = { { 0, VertexFormat::kFloat4, 0, 0 } };
		const VertexBufferLayout buffers[] = { { sizeof( float ) * 4, false } };
		static const std::uint8_t kNoWrites[] = { 0 };
		for ( const bool tested : { true, false } )
		{
			shaderlib::PipelineRecipe recipe = shaderlib::CoreRecipe( { kVertex, kFragment } );
			recipe.raster.cull = CullMode::kNone;
			recipe.colorFormats = { target.colorFormat };
			recipe.blends = { BlendMode::kOpaque };
			recipe.depthFormat = target.depthFormat;
			recipe.depthStencil = { tested, false, CompareOp::kLessEqual };
			recipe.sampleCount = target.samples;
			recipe.debugName =
			    tested ? "render.pass.visibility tested" : "render.pass.visibility possible";
			auto resolved = shaderlib::Resolve(
			    recipe, shaderlib::CoreArtifacts(), device->Facts().artifactFormat );
			if ( !resolved )
			{
				v.failure = "the visibility program has no artifact for the device";
				return &v;
			}
			PipelineDesc desc = resolved.Value().Desc();
			desc.vertex = { attributes, buffers };
			desc.colorWriteMasks = kNoWrites;
			auto pipeline = device->CreatePipeline( desc );
			if ( !pipeline )
			{
				v.failure = "a visibility pipeline was refused";
				return &v;
			}
			( tested ? v.tested : v.untested ) = pipeline.Value();
		}
		return &v;
	}

	void ReleasePending( const Pending &p, CompletionToken after )
	{
		for ( ResourceId resource : { ResourceId( p.vertices ), ResourceId( p.readback ) } )
		{
			if ( resource.value != 0 )
				(void)device->Release( resource, after );
		}
	}

	void ReleaseAll( CompletionToken after )
	{
		if ( !device )
			return;
		for ( const Pending &p : pending )
		{
			ReleasePending( p, after );
			std::lock_guard<std::mutex> guard( lock );
			results[p.serial] = { -1, -1 };
			++stats.failed;
			stats.lastFailure = "the device went before the counts were read";
		}
		pending.clear();
		for ( Variant &v : variants )
		{
			for ( PipelineId pipeline : { v.tested, v.untested } )
			{
				if ( pipeline.IsValid() )
					(void)device->Release( pipeline, after );
			}
		}
		variants.clear();
		device = nullptr;
	}
};

VisibilityCounter::VisibilityCounter() : m_State( std::make_unique<State>() )
{
}

// Without ReleaseDevice the device may be gone: the handles are dropped.
VisibilityCounter::~VisibilityCounter() = default;

std::uint32_t VisibilityCounter::Queue( const Query &query )
{
	State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	if ( !( query.viewport.width > 0.0f ) || !( query.viewport.height > 0.0f ) )
	{
		++s.stats.refused;
		s.stats.lastFailure = "a visibility query with an empty viewport";
		return 0;
	}
	const std::uint32_t serial = s.nextSerial;
	s.nextSerial = ( s.nextSerial + 1 ) & kVisibilitySerialMask;
	if ( s.nextSerial == 0 )
		s.nextSerial = 1;
	s.queued[serial] = query;
	++s.stats.queued;
	while ( s.queued.size() > kMaxKept )
	{
		s.results[s.queued.begin()->first] = { -1, -1 };
		++s.stats.failed;
		s.stats.lastFailure = "a visibility query's slot never recorded";
		s.queued.erase( s.queued.begin() );
	}
	while ( s.results.size() > kMaxKept )
		s.results.erase( s.results.begin() );
	return kVisibilityTag | serial;
}

std::optional<Counts> VisibilityCounter::Result( std::uint32_t tag )
{
	State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	const auto found = s.results.find( tag & kVisibilitySerialMask );
	if ( found == s.results.end() )
		return std::nullopt;
	const Counts counts = found->second;
	s.results.erase( found );
	return counts;
}

VisibilityStats VisibilityCounter::Stats() const
{
	std::lock_guard<std::mutex> guard( m_State->lock );
	return m_State->stats;
}

void VisibilityCounter::Record(
    std::uint32_t tag, CommandEncoder &encoder, const VisibilityTarget &target )
{
	State &s = *m_State;
	if ( !IsVisibilityTag( tag ) )
		return;
	const std::uint32_t serial = tag & kVisibilitySerialMask;
	Query query;
	{
		std::lock_guard<std::mutex> guard( s.lock );
		const auto found = s.queued.find( serial );
		if ( found == s.queued.end() )
		{
			if ( !s.recorded.count( serial ) )
			{
				++s.stats.failed;
				s.stats.lastFailure = "a visibility slot names no queued query";
			}
			return;
		}
		query = found->second;
		s.queued.erase( found );
		s.recorded.insert( serial );
		while ( s.recorded.size() > kMaxKept )
			s.recorded.erase( s.recorded.begin() );
	}
	if ( !target.device || !target.color.IsValid() || !target.depth.IsValid() ||
	     target.width == 0 || target.height == 0 )
		return s.Fail( serial, "a visibility slot's target is incomplete" );
	if ( s.device && s.device != target.device )
		s.ReleaseAll( CompletionToken() );
	s.device = target.device;
	Variant *variant = s.VariantFor( target );
	if ( !variant->failure.empty() )
		return s.Fail( serial, variant->failure );

	// The proxy's twelve vertices, each shifted half a pixel as the views
	// are (D3D9 pixel centers to the port's).
	float vertices[12][4];
	for ( int t = 0; t < 4; ++t )
	{
		for ( int c = 0; c < 3; ++c )
		{
			const float *p = query.points[kTriangles[t][c]];
			float *v = vertices[t * 3 + c];
			std::memcpy( v, p, sizeof( float ) * 4 );
			v[0] += p[3] / query.viewport.width;
			v[1] -= p[3] / query.viewport.height;
		}
	}
	Pending p;
	p.serial = serial;
	BufferDesc vertexDesc;
	vertexDesc.size = sizeof( vertices );
	vertexDesc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kVertex };
	vertexDesc.debugName = "render.pass.visibility proxy";
	BufferDesc readback;
	readback.size = 16;
	readback.usages = { ResourceUsage::kCopyDestination };
	readback.memory = MemoryKind::kReadback;
	readback.debugName = "render.pass.visibility counts";
	auto vertexMade = s.device->CreateBuffer( vertexDesc );
	auto readbackMade = s.device->CreateBuffer( readback );
	if ( vertexMade )
		p.vertices = vertexMade.Value();
	if ( readbackMade )
		p.readback = readbackMade.Value();
	if ( !vertexMade || !readbackMade )
	{
		s.ReleasePending( p, target.submitted );
		return s.Fail( serial, "the visibility proxy's buffers were refused" );
	}

	encoder.BeginLabel( "pixel visibility" );
	encoder.TransitionBuffer(
	    p.vertices, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	encoder.WriteBuffer( p.vertices, 0, std::as_bytes( std::span( vertices ) ) );
	encoder.TransitionBuffer( p.vertices, ResourceUsage::kCopyDestination, ResourceUsage::kVertex );
	encoder.TransitionBuffer(
	    p.readback, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	const ColorAttachment colors[] = { { target.color, LoadOp::kLoad, StoreOp::kStore, {}, {} } };
	RenderingDesc rendering;
	rendering.colors = colors;
	rendering.depth = DepthAttachment{ target.depth, LoadOp::kLoad, StoreOp::kStore, 1.0f };
	rendering.width = target.width;
	rendering.height = target.height;
	encoder.BeginRendering( rendering );
	encoder.SetViewport( query.viewport );
	encoder.SetVertexBuffer( 0, p.vertices, 0 );
	encoder.SetPipeline( variant->tested );
	encoder.BeginOcclusionQuery( p.readback, 0 );
	encoder.Draw( 12, 1, 0, 0 );
	encoder.EndOcclusionQuery();
	encoder.SetPipeline( variant->untested );
	encoder.BeginOcclusionQuery( p.readback, 8 );
	encoder.Draw( 12, 1, 0, 0 );
	encoder.EndOcclusionQuery();
	encoder.EndRendering();
	encoder.EndLabel();
	s.pending.push_back( p );
	std::lock_guard<std::mutex> guard( s.lock );
	++s.stats.recorded;
}

void VisibilityCounter::FrameSubmitted( IRenderDevice2 &device, CompletionToken token )
{
	State &s = *m_State;
	if ( s.device != &device )
		return;
	for ( Pending &p : s.pending )
	{
		if ( !p.submitted )
		{
			p.submitted = true;
			p.token = token;
		}
	}
	std::erase_if( s.pending,
	    [&]( const Pending &p )
	    {
		    if ( !p.submitted || !device.IsComplete( p.token ) )
			    return false;
		    std::uint64_t counts[2] = {};
		    const bool read = bool(
		        device.ReadBuffer( p.readback, 0, std::as_writable_bytes( std::span( counts ) ) ) );
		    s.ReleasePending( p, p.token );
		    std::lock_guard<std::mutex> guard( s.lock );
		    if ( read )
		    {
			    s.results[p.serial] = { std::int64_t( counts[0] ), std::int64_t( counts[1] ) };
			    ++s.stats.resolved;
		    }
		    else
		    {
			    s.results[p.serial] = { -1, -1 };
			    ++s.stats.failed;
			    s.stats.lastFailure = "the visibility counts' read back failed";
		    }
		    return true;
	    } );
}

void VisibilityCounter::ReleaseDevice( IRenderDevice2 &device )
{
	State &s = *m_State;
	if ( s.device != &device )
		return;
	s.ReleaseAll( CompletionToken() );
}

} // namespace render::pass::visibility
