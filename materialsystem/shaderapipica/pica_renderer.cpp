//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The 3DS fullbright renderer (see pica_renderer.h).
//
//=============================================================================//

#include "pica_renderer.h"

#include <3ds.h>
#include <citro3d.h>

#include <cstdio>
#include <cstring>
#include <new>

// The vertex shader binary, assembled from fullbright.v.pica by the build.
extern "C" const unsigned char fullbright_shbin[];
extern "C" const unsigned fullbright_shbin_size;

// The launch shell's bottom-screen log (launcher_main/n3ds_main.cpp).
extern "C" void N3ds_StartDebugConsole();

namespace pica
{

namespace
{

constexpr std::size_t kRingBytes = 3u * 1024u * 1024u; // per frame in flight
constexpr int kRingCount = 2;

// The top screen's display-transfer flags: the tiled RGBA8 render target to
// the linear scanout buffer in the screen's own format (gfxInitDefault makes
// it BGR8; SDL3's 3DS video driver makes it RGBA8), no scaling.
u32 DisplayTransferFlags()
{
	GX_TRANSFER_FORMAT out = GX_TRANSFER_FMT_RGB8;
	switch ( gfxGetScreenFormat( GFX_TOP ) )
	{
	case GSP_RGBA8_OES: out = GX_TRANSFER_FMT_RGBA8; break;
	case GSP_BGR8_OES: out = GX_TRANSFER_FMT_RGB8; break;
	case GSP_RGB565_OES: out = GX_TRANSFER_FMT_RGB565; break;
	case GSP_RGB5_A1_OES: out = GX_TRANSFER_FMT_RGB5A1; break;
	case GSP_RGBA4_OES: out = GX_TRANSFER_FMT_RGBA4; break;
	}
	return GX_TRANSFER_FLIP_VERT( 0 ) | GX_TRANSFER_OUT_TILED( 0 ) | GX_TRANSFER_RAW_COPY( 0 ) |
		GX_TRANSFER_IN_FORMAT( GX_TRANSFER_FMT_RGBA8 ) | GX_TRANSFER_OUT_FORMAT( out ) |
		GX_TRANSFER_SCALING( GX_TRANSFER_SCALE_NO );
}

struct State
{
	bool initialized = false;
	bool inFrame = false;
	C3D_RenderTarget *top = nullptr;
	DVLB_s *dvlb = nullptr;
	shaderProgram_s program;
	int uMvp = -1;
	int uTint = -1;
	std::uint8_t *ring[kRingCount] = {};
	std::size_t ringUsed = 0;
	int ringIndex = 0;
	Stats stats;
	std::size_t textureBytes = 0;
	Texture *boundTexture = nullptr;
	bool textureEnvTextured = false;
	bool textureEnvValid = false;
};

State g_state;

GPU_TESTFUNC PicaCompare( Compare compare, bool reverse )
{
	// Reversing swaps the order of the operands: a < b becomes a > b.
	switch ( compare )
	{
	case Compare::kNever: return GPU_NEVER;
	case Compare::kLess: return reverse ? GPU_GREATER : GPU_LESS;
	case Compare::kEqual: return GPU_EQUAL;
	case Compare::kLessEqual: return reverse ? GPU_GEQUAL : GPU_LEQUAL;
	case Compare::kGreater: return reverse ? GPU_LESS : GPU_GREATER;
	case Compare::kNotEqual: return GPU_NOTEQUAL;
	case Compare::kGreaterEqual: return reverse ? GPU_LEQUAL : GPU_GEQUAL;
	case Compare::kAlways: return GPU_ALWAYS;
	}
	return GPU_ALWAYS;
}

GPU_BLENDFACTOR PicaBlend( Blend blend )
{
	switch ( blend )
	{
	case Blend::kZero: return GPU_ZERO;
	case Blend::kOne: return GPU_ONE;
	case Blend::kSrcColor: return GPU_SRC_COLOR;
	case Blend::kOneMinusSrcColor: return GPU_ONE_MINUS_SRC_COLOR;
	case Blend::kDstColor: return GPU_DST_COLOR;
	case Blend::kOneMinusDstColor: return GPU_ONE_MINUS_DST_COLOR;
	case Blend::kSrcAlpha: return GPU_SRC_ALPHA;
	case Blend::kOneMinusSrcAlpha: return GPU_ONE_MINUS_SRC_ALPHA;
	case Blend::kDstAlpha: return GPU_DST_ALPHA;
	case Blend::kOneMinusDstAlpha: return GPU_ONE_MINUS_DST_ALPHA;
	case Blend::kSrcAlphaSaturate: return GPU_SRC_ALPHA_SATURATE;
	}
	return GPU_ONE;
}

void SetTextureEnv( bool textured )
{
	if ( g_state.textureEnvValid && g_state.textureEnvTextured == textured )
		return;
	C3D_TexEnv *env = C3D_GetTexEnv( 0 );
	C3D_TexEnvInit( env );
	if ( textured )
	{
		C3D_TexEnvSrc( env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR );
		C3D_TexEnvFunc( env, C3D_Both, GPU_MODULATE );
	}
	else
	{
		C3D_TexEnvSrc( env, C3D_Both, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR );
		C3D_TexEnvFunc( env, C3D_Both, GPU_REPLACE );
	}
	g_state.textureEnvTextured = textured;
	g_state.textureEnvValid = true;
}

} // namespace

void ConvertClip( const float m[16], float out[16] )
{
	// Column-vector rows of the D3D row-vector matrix: r[j][i] = m[i][j].
	float r[4][4];
	for ( int i = 0; i < 4; ++i )
		for ( int j = 0; j < 4; ++j )
			r[j][i] = m[i * 4 + j];
	for ( int i = 0; i < 4; ++i )
	{
		out[0 * 4 + i] = r[1][i];          // x' = y
		out[1 * 4 + i] = -r[0][i];         // y' = -x
		out[2 * 4 + i] = r[2][i] - r[3][i]; // z' = z - w
		out[3 * 4 + i] = r[3][i];          // w' = w
	}
}

Texture::Texture() : m_native( nullptr ), m_valid( false ), m_width( 0 ), m_height( 0 ), m_bytes( 0 ) {}

Texture::~Texture()
{
	Release();
}

void Texture::Release()
{
	if ( m_native )
	{
		C3D_Tex *tex = static_cast<C3D_Tex *>( m_native );
		if ( m_valid )
		{
			C3D_TexDelete( tex );
			g_state.textureBytes -= m_bytes;
		}
		delete tex;
		m_native = nullptr;
	}
	if ( g_state.boundTexture == this )
		g_state.boundTexture = nullptr;
	m_valid = false;
	m_bytes = 0;
}

bool Texture::Upload( TexFormat format, int width, int height, int levelCount,
	const std::uint8_t *const *levels )
{
	Release();
	if ( levelCount < 1 )
		return false;
	C3D_Tex *tex = new ( std::nothrow ) C3D_Tex;
	if ( !tex )
		return false;
	C3D_TexInitParams params = {};
	params.width = std::uint16_t( width );
	params.height = std::uint16_t( height );
	params.maxLevel = std::uint8_t( levelCount - 1 );
	params.format = GPU_TEXCOLOR( format );
	params.type = GPU_TEX_2D;
	params.onVram = false; // linear FCRAM: VRAM holds the render targets
	if ( !C3D_TexInitWithParams( tex, nullptr, params ) )
	{
		delete tex;
		return false;
	}
	std::size_t total = 0;
	for ( int level = 0; level < levelCount; ++level )
	{
		u32 size = 0;
		void *dst = C3D_TexGetImagePtr( tex, tex->data, level, &size );
		std::memcpy( dst, levels[level], size );
		total += size;
	}
	C3D_TexFlush( tex );
	C3D_TexSetFilter( tex, GPU_LINEAR, GPU_NEAREST );
	if ( levelCount > 1 )
		C3D_TexSetFilterMipmap( tex, GPU_LINEAR );
	C3D_TexSetWrap( tex, GPU_REPEAT, GPU_REPEAT );
	m_native = tex;
	m_valid = true;
	m_width = width;
	m_height = height;
	m_bytes = total;
	g_state.textureBytes += total;
	return true;
}

void Texture::SetWrap( bool wrapS, bool wrapT )
{
	if ( !m_valid )
		return;
	C3D_TexSetWrap( static_cast<C3D_Tex *>( m_native ), wrapS ? GPU_REPEAT : GPU_CLAMP_TO_EDGE,
		wrapT ? GPU_REPEAT : GPU_CLAMP_TO_EDGE );
}

bool Init()
{
	if ( g_state.initialized )
		return true;
	N3ds_StartDebugConsole();
	if ( !C3D_Init( C3D_DEFAULT_CMDBUF_SIZE * 4 ) )
		return false;
	g_state.top = C3D_RenderTargetCreate( 240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8 );
	if ( !g_state.top )
		return false;
	C3D_RenderTargetSetOutput( g_state.top, GFX_TOP, GFX_LEFT, DisplayTransferFlags() );

	g_state.dvlb = DVLB_ParseFile( (u32 *)fullbright_shbin, fullbright_shbin_size );
	shaderProgramInit( &g_state.program );
	shaderProgramSetVsh( &g_state.program, &g_state.dvlb->DVLE[0] );
	C3D_BindProgram( &g_state.program );
	g_state.uMvp = shaderInstanceGetUniformLocation( g_state.program.vertexShader, "mvp" );
	g_state.uTint = shaderInstanceGetUniformLocation( g_state.program.vertexShader, "tint" );

	C3D_AttrInfo *attr = C3D_GetAttrInfo();
	AttrInfo_Init( attr );
	AttrInfo_AddLoader( attr, 0, GPU_FLOAT, 3 );
	AttrInfo_AddLoader( attr, 1, GPU_UNSIGNED_BYTE, 4 );
	AttrInfo_AddLoader( attr, 2, GPU_FLOAT, 2 );

	for ( auto &ring : g_state.ring )
	{
		ring = static_cast<std::uint8_t *>( linearAlloc( kRingBytes ) );
		if ( !ring )
			return false;
	}
	g_state.initialized = true;
	return true;
}

void Shutdown()
{
	if ( !g_state.initialized )
		return;
	if ( g_state.inFrame )
		EndFrame();
	for ( auto &ring : g_state.ring )
	{
		linearFree( ring );
		ring = nullptr;
	}
	shaderProgramFree( &g_state.program );
	DVLB_Free( g_state.dvlb );
	C3D_RenderTargetDelete( g_state.top );
	C3D_Fini();
	g_state = State();
}

bool Initialized()
{
	return g_state.initialized;
}

bool InFrame()
{
	return g_state.inFrame;
}

void BeginFrame()
{
	if ( !g_state.initialized || g_state.inFrame )
		return;
	C3D_FrameBegin( C3D_FRAME_SYNCDRAW );
	C3D_FrameDrawOn( g_state.top );
	g_state.inFrame = true;
	g_state.ringIndex = ( g_state.ringIndex + 1 ) % kRingCount;
	g_state.ringUsed = 0;
	g_state.stats = Stats();
	g_state.textureEnvValid = false;
	SetViewport( 0, 0, kScreenWidth, kScreenHeight );
}

void EndFrame()
{
	if ( !g_state.inFrame )
		return;
	g_state.stats.textureBytes = g_state.textureBytes;
	C3D_FrameEnd( 0 );
	g_state.inFrame = false;
}

void Clear( bool color, bool depth, std::uint32_t rgba )
{
	if ( !g_state.inFrame )
		BeginFrame();
	C3D_ClearBits bits = C3D_ClearBits( ( color ? C3D_CLEAR_COLOR : 0 ) | ( depth ? C3D_CLEAR_DEPTH : 0 ) );
	if ( bits )
		C3D_RenderTargetClear( g_state.top, bits, rgba, 0 );
}

void SetViewport( int x, int y, int width, int height )
{
	// The framebuffer is the screen turned a quarter: screen x runs along the
	// framebuffer's y, screen y along its x from the far edge.
	const int fbX = 240 - ( y + height );
	const int fbY = 400 - ( x + width );
	C3D_SetViewport( u32( fbX < 0 ? 0 : fbX ), u32( fbY < 0 ? 0 : fbY ), u32( height ), u32( width ) );
}

void *AllocTransient( std::size_t bytes, std::size_t align )
{
	std::size_t at = ( g_state.ringUsed + align - 1 ) & ~( align - 1 );
	if ( at + bytes > kRingBytes )
	{
		++g_state.stats.ringOverflows;
		return nullptr;
	}
	g_state.ringUsed = at + bytes;
	return g_state.ring[g_state.ringIndex] + at;
}

bool IsLinear( const void *ptr )
{
	const u32 address = reinterpret_cast<u32>( ptr );
	return address >= OS_FCRAM_VADDR && address < OS_FCRAM_VADDR + OS_FCRAM_SIZE;
}

void *AllocLinear( std::size_t bytes )
{
	return linearAlloc( bytes );
}

void FreeLinear( void *ptr )
{
	if ( ptr )
		linearFree( ptr );
}

void FlushLinear( const void *ptr, std::size_t bytes )
{
	GSPGPU_FlushDataCache( ptr, bytes );
}

void Draw( const float clipFromObject[16], const DrawState &state, Texture *texture,
	const Vertex *vertices, int vertexCount, const std::uint16_t *indices, int indexCount,
	Primitive primitive )
{
	if ( !g_state.inFrame )
		BeginFrame();
	if ( vertexCount <= 0 || !vertices )
		return;

	float rows[16];
	ConvertClip( clipFromObject, rows );
	// citro3d's float vector uniforms are stored w z y x.
	C3D_FVec *mvp = C3D_FVUnifWritePtr( GPU_VERTEX_SHADER, g_state.uMvp, 4 );
	for ( int i = 0; i < 4; ++i )
		mvp[i] = FVec4_New( rows[i * 4 + 0], rows[i * 4 + 1], rows[i * 4 + 2], rows[i * 4 + 3] );
	C3D_FVUnifSet( GPU_VERTEX_SHADER, g_state.uTint, state.tint[0], state.tint[1], state.tint[2], state.tint[3] );

	C3D_BufInfo *buf = C3D_GetBufInfo();
	BufInfo_Init( buf );
	BufInfo_Add( buf, vertices, sizeof( Vertex ), 3, 0x210 );

	const bool textured = texture && texture->Valid();
	if ( textured )
		C3D_TexBind( 0, static_cast<C3D_Tex *>( texture->Native() ) );
	SetTextureEnv( textured );

	C3D_DepthTest( state.depthTest || state.depthWrite,
		state.depthTest ? PicaCompare( state.depthFunc, true ) : GPU_ALWAYS,
		GPU_WRITEMASK( ( state.colorWrite ? GPU_WRITE_COLOR : 0 ) |
			( state.alphaWrite ? GPU_WRITE_ALPHA : 0 ) | ( state.depthWrite ? GPU_WRITE_DEPTH : 0 ) ) );
	C3D_AlphaTest( state.alphaTest, PicaCompare( state.alphaFunc, false ), state.alphaRef );
	if ( state.blend )
		C3D_AlphaBlend( GPU_BLEND_ADD, GPU_BLEND_ADD, PicaBlend( state.src ), PicaBlend( state.dst ),
			PicaBlend( state.src ), PicaBlend( state.dst ) );
	else
		C3D_AlphaBlend( GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_ONE, GPU_ZERO, GPU_ONE, GPU_ZERO );
	// D3D culls counter-clockwise (back) faces; the quarter turn keeps the
	// winding, and the framebuffer's y axis points the other way.
	C3D_CullFace( state.cull ? GPU_CULL_FRONT_CCW : GPU_CULL_NONE );

	const GPU_Primitive_t prim = primitive == Primitive::kTriangleStrip ? GPU_TRIANGLE_STRIP :
		( primitive == Primitive::kTriangleFan ? GPU_TRIANGLE_FAN : GPU_TRIANGLES );
	if ( indices && indexCount > 0 )
	{
		C3D_DrawElements( prim, indexCount, C3D_UNSIGNED_SHORT, indices );
		g_state.stats.triangles += std::uint32_t( prim == GPU_TRIANGLES ? indexCount / 3 : indexCount - 2 );
	}
	else
	{
		C3D_DrawArrays( prim, 0, vertexCount );
		g_state.stats.triangles += std::uint32_t( prim == GPU_TRIANGLES ? vertexCount / 3 : vertexCount - 2 );
	}
	++g_state.stats.draws;
}

bool CaptureTopScreen( const char *path )
{
	u16 fbWidth = 0, fbHeight = 0;
	const u8 *fb = gfxGetFramebuffer( GFX_TOP, GFX_LEFT, &fbWidth, &fbHeight );
	const GSPGPU_FramebufferFormat format = gfxGetScreenFormat( GFX_TOP );
	const int bpp = gspGetBytesPerPixel( format );
	FILE *file = std::fopen( path, "wb" );
	if ( !file || !fb )
	{
		if ( file )
			std::fclose( file );
		return false;
	}
	// The scanout buffer is column-major: fbWidth (240) texels per screen
	// column, bottom row first; screen columns run left to right.
	std::fprintf( file, "P6\n%d %d\n255\n", int( fbHeight ), int( fbWidth ) );
	static std::uint8_t row[400 * 3];
	for ( int y = 0; y < fbWidth; ++y )
	{
		for ( int x = 0; x < fbHeight; ++x )
		{
			const u8 *p = fb + ( x * fbWidth + ( fbWidth - 1 - y ) ) * bpp;
			std::uint8_t r = 0, g = 0, b = 0;
			switch ( format )
			{
			case GSP_RGBA8_OES: r = p[3], g = p[2], b = p[1]; break;
			case GSP_BGR8_OES: r = p[2], g = p[1], b = p[0]; break;
			case GSP_RGB565_OES:
			{
				const u16 v = u16( p[0] | ( p[1] << 8 ) );
				r = u8( ( v >> 11 ) << 3 ), g = u8( ( ( v >> 5 ) & 63 ) << 2 ), b = u8( ( v & 31 ) << 3 );
				break;
			}
			default:
			{
				const u16 v = u16( p[0] | ( p[1] << 8 ) );
				r = u8( ( v >> 11 ) << 3 ), g = u8( ( ( v >> 6 ) & 31 ) << 3 ), b = u8( ( ( v >> 1 ) & 31 ) << 3 );
				break;
			}
			}
			row[x * 3 + 0] = r, row[x * 3 + 1] = g, row[x * 3 + 2] = b;
		}
		std::fwrite( row, 1, std::size_t( fbHeight ) * 3, file );
	}
	std::fclose( file );
	return true;
}

const Stats &FrameStats()
{
	return g_state.stats;
}

} // namespace pica
