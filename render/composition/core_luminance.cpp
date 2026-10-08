//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition's luminance counts (RFC 0016
//			render.pass.luminance); see core_luminance.h.
//
//=============================================================================//

#include "core_luminance.h"

#include <algorithm>
#include <cstdio>
#include <limits>
#include <optional>

namespace render::composition
{

static_assert( ( pass::luminance::kLuminanceTag & legacy::kCorePassForwarded ) != 0 &&
                   ( ( pass::luminance::kLuminanceTag | pass::luminance::kLuminanceSerialMask ) &
                       ( legacy::kCorePassLegacyOff | legacy::kCorePassFrameEnd ) ) == 0,
    "luminance tags are forwarded tags, clear of the frontend's slot bits" );

namespace
{

// The backend's textures as stored (the legacy compare read them so).
class Textures final : public pass::luminance::ILuminanceTextures
{
public:
	explicit Textures( legacy::ICoreTextures &textures ) : m_Textures( textures ) {}
	device::TextureId Import( int key ) override { return m_Textures.Import( key, false ); }

private:
	legacy::ICoreTextures &m_Textures;
};

} // namespace

unsigned CoreLuminance::Queue(
    ITexture *texture, int x0, int y0, int x1, int y1, float minimum, float maximum, float scale )
{
	legacy::ICorePassSlots *slots = m_Frontend.CorePassSlots();
	if ( !slots || !m_Host || !m_Host->textureHandle || !texture )
		return 0;
	pass::luminance::Query query;
	query.texture = m_Host->textureHandle( texture );
	query.x0 = x0;
	query.y0 = y0;
	query.x1 = x1;
	query.y1 = y1;
	query.minimum = minimum;
	query.maximum = maximum;
	query.scale = scale;
	const std::uint32_t tag = m_Counter.Queue( query );
	if ( tag )
		slots->MarkSlot( tag );
	return tag;
}

int CoreLuminance::Result( unsigned id )
{
	const std::optional<std::int64_t> count = m_Counter.Result( id );
	if ( !count )
		return -1;
	if ( *count < 0 )
		return -2;
	return int( std::min<std::int64_t>( *count, std::numeric_limits<int>::max() ) );
}

void CoreLuminance::GetStats( RenderCoreLuminanceStats *out ) const
{
	if ( !out )
		return;
	const pass::luminance::LuminanceStats stats = m_Counter.Stats();
	*out = RenderCoreLuminanceStats{};
	out->queued = stats.queued;
	out->refused = stats.refused;
	out->recorded = stats.recorded;
	out->resolved = stats.resolved;
	out->failed = stats.failed;
	std::snprintf( out->lastFailure, sizeof( out->lastFailure ), "%s", stats.lastFailure.c_str() );
}

void CoreLuminance::RecordSlot(
    std::uint32_t tag, device::CommandEncoder &encoder, const legacy::CorePassTarget &target )
{
	std::optional<Textures> textures;
	if ( target.textures )
		textures.emplace( *target.textures );
	pass::luminance::LuminanceTarget luminance;
	luminance.device = target.device;
	luminance.textures = textures ? &*textures : nullptr;
	luminance.submitted = target.submitted;
	luminance.frame = target.frame;
	m_Device = target.device;
	m_Counter.Record( tag, encoder, luminance );
}

void CoreLuminance::FrameSubmitted( device::CompletionToken token, bool submitted )
{
	if ( submitted && m_Device )
		m_Counter.FrameSubmitted( *m_Device, token );
}

void CoreLuminance::ReleaseDevice( device::IRenderDevice2 &device )
{
	m_Counter.ReleaseDevice( device );
	if ( m_Device == &device )
		m_Device = nullptr;
}

} // namespace render::composition
