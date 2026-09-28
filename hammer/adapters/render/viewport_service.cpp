//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer/adapters/render/viewport_service.h.
//
//=============================================================================//

#include "viewport_service.h"

#include <atomic>
#include <future>
#include <utility>

namespace hammer::render_adapter
{

// Shared by the service and its in-flight tasks. 'renderer' belongs to the
// render sequence; 'alive' is cleared by the destructor on the owner's thread
// and read on both sequences.
struct ViewportService::State
{
	State( ::render::device::IRenderDevice2 &d, platform::ITaskRunner &r,
	    platform::ISequencedTaskRunner &p, std::unique_ptr<IMaterialTextures> t )
	    : device( d ), render( r ), reply( p ), textures( std::move( t ) )
	{
	}

	::render::device::IRenderDevice2 &device;
	platform::ITaskRunner &render;
	platform::ISequencedTaskRunner &reply;
	std::unique_ptr<IMaterialTextures> textures; // the render sequence's; outlives the renderer
	std::unique_ptr<ViewportRenderer> renderer;
	std::atomic<bool> alive{ true };
};

namespace
{

using State = ViewportService::State;

void Reply( const std::shared_ptr<State> &state, ViewportService::Result result,
    ViewportService::Done done )
{
	// If the reply runner is gone, so is the host: nothing to tell.
	(void)state->reply.PostTask(
	    [state, result = std::move( result ), done = std::move( done )]() mutable
	    {
		    if ( state->alive && done )
		    {
			    done( std::move( result ) );
		    }
	    } );
}

void Poll( const std::shared_ptr<State> &state, ViewportRenderer::Ticket ticket,
    ViewportService::Done done )
{
	if ( !state->alive || !state->renderer )
	{
		return;
	}
	auto taken = state->renderer->Take( ticket );
	if ( !taken )
	{
		Reply( state, foundation::MakeUnexpected( taken.Error() ), std::move( done ) );
		return;
	}
	if ( taken.Value() )
	{
		Reply( state, std::move( *taken.Value() ), std::move( done ) );
		return;
	}
	// Still on the GPU: look again shortly, without blocking the sequence.
	(void)state->render.PostDelayedTask(
	    [state, ticket, done = std::move( done )]() mutable
	    {
		    Poll( state, ticket, std::move( done ) );
	    },
	    ViewportService::kPollNanoseconds );
}

} // namespace

ViewportService::ViewportService( ::render::device::IRenderDevice2 &device,
    platform::ITaskRunner &render, platform::ISequencedTaskRunner &reply,
    std::unique_ptr<IMaterialTextures> textures )
    : m_State( std::make_shared<State>( device, render, reply, std::move( textures ) ) )
{
}

ViewportService::~ViewportService()
{
	m_State->alive = false;
	// The renderer (and the frames it waits for) goes on its own sequence;
	// the device may be destroyed as soon as this returns.
	std::shared_ptr<std::promise<void>> gone = std::make_shared<std::promise<void>>();
	std::future<void> done = gone->get_future();
	std::shared_ptr<State> state = m_State;
	if ( m_State->render.PostTask(
	         [state, gone]
	         {
		         state->renderer.reset();
		         state->textures.reset();
		         gone->set_value();
	         } ) == platform::PostResult::kAccepted )
	{
		done.wait();
	}
	else
	{
		// The render runner is already shut down, so nothing else runs there.
		m_State->renderer.reset();
		m_State->textures.reset();
	}
}

bool ViewportService::SetMaterialSource( std::unique_ptr<IMaterialTextures> textures )
{
	std::shared_ptr<State> state = m_State;
	auto shared = std::make_shared<std::unique_ptr<IMaterialTextures>>( std::move( textures ) );
	return state->render.PostTask(
	           [state, shared]
	           {
		           // The renderer borrows the source, so it goes first; the next
		           // job makes a new one, which restages the scene.
		           state->renderer.reset();
		           state->textures = std::move( *shared );
	           } ) == platform::PostResult::kAccepted;
}

bool ViewportService::Submit( std::shared_ptr<const viewport::RenderSnapshot> scene,
    std::uint64_t key, ViewJob job, Done done )
{
	std::shared_ptr<State> state = m_State;
	return state->render.PostTask(
	           [state, scene = std::move( scene ), key, job = std::move( job ),
	               done = std::move( done )]() mutable
	           {
		           if ( !state->alive )
		           {
			           return;
		           }
		           if ( !state->renderer )
		           {
			           auto made = ViewportRenderer::Create( state->device, state->textures.get() );
			           if ( !made )
			           {
				           Reply( state, foundation::MakeUnexpected( made.Error() ),
				               std::move( done ) );
				           return;
			           }
			           state->renderer = std::move( made ).Value();
		           }
		           if ( !scene )
		           {
			           Reply( state, foundation::MakeUnexpected( ViewportStatus::kInvalidView ),
			               std::move( done ) );
			           return;
		           }
		           if ( auto staged = state->renderer->SetScene( *scene, key ); !staged )
		           {
			           Reply(
			               state, foundation::MakeUnexpected( staged.Error() ), std::move( done ) );
			           return;
		           }
		           ViewRequest request;
		           request.kind = job.kind;
		           request.camera2D = job.camera2D ? &*job.camera2D : nullptr;
		           request.camera3D = job.camera3D ? &*job.camera3D : nullptr;
		           request.grid = std::move( job.grid );
		           request.overlay = std::move( job.overlay );
		           request.pixelWidth = job.pixelWidth;
		           request.pixelHeight = job.pixelHeight;
		           auto ticket = state->renderer->Render( request );
		           if ( !ticket )
		           {
			           Reply(
			               state, foundation::MakeUnexpected( ticket.Error() ), std::move( done ) );
			           return;
		           }
		           Poll( state, ticket.Value(), std::move( done ) );
	           } ) == platform::PostResult::kAccepted;
}

} // namespace hammer::render_adapter
