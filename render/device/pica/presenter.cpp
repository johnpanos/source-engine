//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.pica's top-screen presenter (RFC 0026 P4,
//			provider.h's PresentTopScreen). The top screen is a 240 x 400
//			framebuffer scanned out a quarter turn; the device draws the
//			source's region onto it as one textured quad laid out turned,
//			and citro3d's frame end transfers the framebuffer to the screen
//			(the display transfer of the legacy renderer's proven setup) and
//			swaps it at the next vertical blank.
//
//=============================================================================//

#if defined( __3DS__ )

#include "pica_device.h"

#include <cstring>

extern "C" const unsigned char present_shbin[];
extern "C" const unsigned present_shbin_size;

namespace render::device::pica
{
namespace
{

constexpr int kScreenWidth = 400; // the screen's own orientation
constexpr int kScreenHeight = 240;

foundation::Unexpected<DeviceError> Fail( DeviceStatus status )
{
	return foundation::MakeUnexpected( DeviceError{ status, DeviceOperation::kSubmit, 0 } );
}

// The tiled RGBA8 framebuffer to the screen's scanout format, unscaled and
// untiled (materialsystem/shaderapicore/pica_renderer.cpp, proven on the
// 3DS and in Azahar).
u32 DisplayTransferFlags()
{
	GX_TRANSFER_FORMAT out = GX_TRANSFER_FMT_RGB8;
	switch ( gfxGetScreenFormat( GFX_TOP ) )
	{
	case GSP_RGBA8_OES:
		out = GX_TRANSFER_FMT_RGBA8;
		break;
	case GSP_BGR8_OES:
		out = GX_TRANSFER_FMT_RGB8;
		break;
	case GSP_RGB565_OES:
		out = GX_TRANSFER_FMT_RGB565;
		break;
	case GSP_RGB5_A1_OES:
		out = GX_TRANSFER_FMT_RGB5A1;
		break;
	case GSP_RGBA4_OES:
		out = GX_TRANSFER_FMT_RGBA4;
		break;
	}
	return GX_TRANSFER_FLIP_VERT( 0 ) | GX_TRANSFER_OUT_TILED( 0 ) | GX_TRANSFER_RAW_COPY( 0 ) |
	       GX_TRANSFER_IN_FORMAT( GX_TRANSFER_FMT_RGBA8 ) | GX_TRANSFER_OUT_FORMAT( out ) |
	       GX_TRANSFER_SCALING( GX_TRANSFER_SCALE_NO );
}

bool PresentProgram()
{
	SharedContext &shared = Shared();
	if ( shared.presentDvlb )
		return true;
	// DVLB_ParseFile keeps pointers into the words it is given.
	static std::vector<u32> words;
	words.resize( ( present_shbin_size + 3 ) / 4 );
	std::memcpy( words.data(), present_shbin, present_shbin_size );
	shared.presentDvlb = DVLB_ParseFile( words.data(), present_shbin_size );
	if ( !shared.presentDvlb )
		return false;
	shaderProgramInit( &shared.presentProgram );
	shaderProgramSetVsh( &shared.presentProgram, &shared.presentDvlb->DVLE[0] );
	return true;
}

} // namespace

DeviceResult<void> PicaDevice::PresentTopScreen(
    TextureId source, std::uint32_t width, std::uint32_t height )
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost );
	const auto found = m_Textures.find( source.value );
	if ( found == m_Textures.end() || found->second.released )
		return Fail( DeviceStatus::kInvalidHandle );
	const TextureRecord &texture = found->second;
	if ( texture.usage != ResourceUsage::kSampled || texture.layout.sampledLevels == 0 ||
	     width == 0 || height == 0 || width > texture.desc.width || height > texture.desc.height )
		return Fail( DeviceStatus::kInvalidState );
	if ( !PresentProgram() )
		return Fail( DeviceStatus::kInternal );
	if ( !m_Screen )
	{
		m_Screen = C3D_RenderTargetCreate(
		    kScreenHeight, kScreenWidth, GPU_RB_RGBA8, C3D_DEPTHTYPE( -1 ) );
		m_PresentQuad = static_cast<float *>( linearAlloc( 4 * 4 * sizeof( float ) ) );
		if ( !m_Screen || !m_PresentQuad )
			return Fail( DeviceStatus::kOutOfMemory );
		C3D_RenderTargetSetOutput( m_Screen, GFX_TOP, GFX_LEFT, DisplayTransferFlags() );
	}

	// Corners in the framebuffer's clip space, which is the screen turned: the
	// screen's x runs down the framebuffer's -y and its y along the
	// framebuffer's x (x' = y, y' = -x, as the legacy renderer's ConvertClip).
	// Texture coordinates are PVS1's (u, 1 - v) of the source's region.
	const float u = float( width ) / float( texture.desc.width );
	const float v = float( height ) / float( texture.desc.height );
	const float quad[4][4] = { { 1, 1, 0, 1 }, // screen top-left
	    { 1, -1, u, 1 },                       // screen top-right
	    { -1, 1, 0, 1 - v },                   // screen bottom-left
	    { -1, -1, u, 1 - v } };                // screen bottom-right
	std::memcpy( m_PresentQuad, quad, sizeof( quad ) );
	GSPGPU_FlushDataCache( m_PresentQuad, sizeof( quad ) );

	OpenFrame();
	C3D_FrameDrawOn( m_Screen );
	m_GpuUses.insert( source.value );
	BindProgram( Shared().presentProgram );
	C3D_AttrInfo *attributes = C3D_GetAttrInfo();
	AttrInfo_Init( attributes );
	AttrInfo_AddLoader( attributes, 0, GPU_FLOAT, 2 );
	AttrInfo_AddLoader( attributes, 1, GPU_FLOAT, 2 );
	C3D_BufInfo *buffers = C3D_GetBufInfo();
	BufInfo_Init( buffers );
	BufInfo_Add( buffers, m_PresentQuad, 4 * sizeof( float ), 2, 0x10 );

	C3D_TexEnv *env = C3D_GetTexEnv( 0 );
	C3D_TexEnvInit( env );
	C3D_TexEnvSrc( env, C3D_Both, GPU_TEXTURE0, GPU_TEXTURE0, GPU_TEXTURE0 );
	C3D_TexEnvFunc( env, C3D_Both, GPU_REPLACE );
	for ( int i = 1; i < int( kMaxCombinerStages ); ++i )
		C3D_TexEnvInit( C3D_GetTexEnv( i ) );
	C3D_TexEnvBufUpdate( C3D_RGB, 0 );
	C3D_TexEnvBufUpdate( C3D_Alpha, 0 );
	C3D_Tex &unit = Shared().units[0];
	unit = texture.sampled;
	unit.param = GPU_TEXTURE_MODE( GPU_TEX_2D ) | GPU_TEXTURE_MAG_FILTER( GPU_LINEAR ) |
	             GPU_TEXTURE_MIN_FILTER( GPU_LINEAR ) | GPU_TEXTURE_WRAP_S( GPU_CLAMP_TO_EDGE ) |
	             GPU_TEXTURE_WRAP_T( GPU_CLAMP_TO_EDGE );
	unit.maxLevel = 0;
	C3D_TexBind( 0, &unit );
	C3D_TexBind( 1, &Shared().placeholder );
	C3D_TexBind( 2, &Shared().placeholder );

	C3D_AlphaTest( false, GPU_ALWAYS, 0 );
	C3D_DepthTest( false, GPU_ALWAYS, GPU_WRITE_COLOR );
	C3D_StencilTest( false, GPU_ALWAYS, 0, 0, 0 );
	C3D_AlphaBlend( GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_ONE, GPU_ZERO, GPU_ONE, GPU_ZERO );
	C3D_CullFace( GPU_CULL_NONE );
	C3D_SetScissor( GPU_SCISSOR_DISABLE, 0, 0, 0, 0 );
	C3D_SetViewport( 0, 0, kScreenHeight, kScreenWidth );
	C3D_DrawArrays( GPU_TRIANGLE_STRIP, 0, 4 );
	// The frame end runs the draw, then the transfer to the screen.
	Drain();
	return {};
}

} // namespace render::device::pica

#endif // __3DS__
