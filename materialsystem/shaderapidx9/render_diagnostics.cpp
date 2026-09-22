//========= Copyright Valve Corporation, All rights reserved. ============//
#include "render_diagnostics.h"

#include <cstdlib>
#include <limits>

namespace renderdiagnostics
{
namespace
{
void String( std::FILE *out, const char *value )
{
	std::fputc( '\"', out );
	for ( const unsigned char *p = reinterpret_cast<const unsigned char *>( value ? value : "" );
	    *p; ++p )
	{
		if ( *p == '\"' || *p == '\\' )
			std::fprintf( out, "\\%c", *p );
		else if ( *p < 32 )
			std::fprintf( out, "\\u%04x", *p );
		else if ( *p >= 128 )
		{
			unsigned length = *p >= 0xc2 && *p <= 0xdf   ? 2
			                  : *p >= 0xe0 && *p <= 0xef ? 3
			                  : *p >= 0xf0 && *p <= 0xf4 ? 4
			                                             : 0;
			bool valid = length != 0;
			for ( unsigned i = 1; valid && i < length; ++i )
			{
				// Stop at the first NUL; never probe further into a truncated sequence.
				valid = p[i] >= 0x80 && p[i] <= 0xbf;
			}
			if ( valid )
			{
				valid = !( ( *p == 0xe0 && p[1] < 0xa0 ) || ( *p == 0xed && p[1] >= 0xa0 ) ||
				           ( *p == 0xf0 && p[1] < 0x90 ) || ( *p == 0xf4 && p[1] >= 0x90 ) );
			}
			if ( valid )
			{
				std::fwrite( p, 1, length, out );
				p += length - 1;
			}
			else
			{
				// JSON surrogateescape preserves one malformed input byte without
				// conflating it with valid Unicode or a literal backslash escape.
				std::fprintf( out, "\\udc%02x", *p );
			}
		}
		else
			std::fputc( *p, out );
	}
	std::fputc( '\"', out );
}
unsigned Setting( const char *name, unsigned fallback, unsigned maximum )
{
	const char *text = std::getenv( name );
	if ( !text || !*text )
		return fallback;
	char *end = nullptr;
	unsigned long value = std::strtoul( text, &end, 10 );
	return !*end && value <= maximum ? static_cast<unsigned>( value ) : fallback;
}
}

Capture::Capture( const char *path, unsigned firstFrame, unsigned frameCount, unsigned maxDraws )
    : m_FirstFrame( firstFrame ), m_FrameCount( frameCount ), m_MaxDraws( maxDraws )
{
	if ( !path || !*path )
		return;
	m_Output = std::fopen( path, "w" );
	if ( !m_Output )
	{
		std::fprintf( stderr, "Render diagnostics: cannot open %s\n", path );
		return;
	}
	std::fprintf( m_Output,
	    "{\"event\":\"capture_begin\",\"schema\":\"source-render-trace/v1\","
	    "\"first_frame\":%u,\"frame_count\":%u,\"max_draws\":%u}\n",
	    m_FirstFrame, m_FrameCount, m_MaxDraws );
}
Capture::~Capture()
{
	if ( m_Output )
	{
		std::fprintf( m_Output,
		    "{\"event\":\"capture_end\",\"frames\":%u,"
		    "\"draw_details\":%u,\"shader_failures\":%u}\n",
		    m_Frame, m_Details, m_Failures );
		std::fclose( m_Output );
	}
}
bool Capture::WantsDrawState() const
{
	if ( !Enabled() )
		return false;
	std::lock_guard<std::mutex> lock( m_Mutex );
	return m_Frame >= m_FirstFrame && m_Frame - m_FirstFrame < m_FrameCount &&
	       m_Details < m_MaxDraws;
}
void Capture::BeginFrame()
{
	if ( !Enabled() )
		return;
	std::lock_guard<std::mutex> lock( m_Mutex );
	++m_Frame;
	m_Draws = m_Primitives = m_Errors = m_MissingShaders = m_Skipped = 0;
}
void Capture::EndFrame()
{
	if ( !Enabled() )
		return;
	std::lock_guard<std::mutex> lock( m_Mutex );
	std::fprintf( m_Output,
	    "{\"event\":\"frame\",\"frame\":%u,\"draws\":%u,"
	    "\"primitives\":%u,\"draw_errors\":%u,\"failed_shader_requests\":%u,\"skipped\":%u}\n",
	    m_Frame, m_Draws, m_Primitives, m_Errors, m_MissingShaders, m_Skipped );
	std::fflush( m_Output );
}
void Capture::RequestShader(
    bool pixel, const char *name, int staticIndex, int dynamicIndex, bool failed )
{
	if ( !Enabled() )
		return;
	std::lock_guard<std::mutex> lock( m_Mutex );
	Shader &shader = pixel ? m_Pixel : m_Vertex;
	shader.name = name ? name : "";
	shader.staticIndex = staticIndex;
	shader.dynamicIndex = dynamicIndex;
	shader.failed = failed;
}
void Capture::ShaderFailure( const char *name, int staticIndex, int dynamicCount )
{
	if ( !Enabled() )
		return;
	std::lock_guard<std::mutex> lock( m_Mutex );
	++m_Failures;
	if ( m_Failures > 256 )
		return;
	std::fprintf( m_Output, "{\"event\":\"shader_failure\",\"frame\":%u,\"name\":", m_Frame );
	String( m_Output, name );
	std::fprintf(
	    m_Output, ",\"static_index\":%d,\"dynamic_count\":%d}\n", staticIndex, dynamicCount );
}
void Capture::WriteShader( const char *stage, const Shader &shader )
{
	std::fprintf( m_Output, ",\"%s_requested\":{\"name\":", stage );
	String( m_Output, shader.name.c_str() );
	std::fprintf( m_Output, ",\"static_index\":%d,\"dynamic_index\":%d,\"failed\":%s}",
	    shader.staticIndex, shader.dynamicIndex, shader.failed ? "true" : "false" );
}
void Capture::Draw( const DrawState &state )
{
	if ( !Enabled() )
		return;
	std::lock_guard<std::mutex> lock( m_Mutex );
	++m_Draws;
	m_Primitives += state.primitives > 0 ? state.primitives : 0;
	m_Errors += state.result < 0;
	m_MissingShaders += m_Vertex.failed || m_Pixel.failed;
	if ( !state.stateAvailable || m_Frame < m_FirstFrame ||
	     m_Frame - m_FirstFrame >= m_FrameCount || m_Details >= m_MaxDraws )
		return;
	++m_Details;
	std::fprintf( m_Output, "{\"event\":\"draw\",\"frame\":%u,\"material\":", m_Frame );
	String( m_Output, state.material );
	std::fputs( ",\"kind\":", m_Output );
	String( m_Output, state.kind );
	std::fprintf( m_Output,
	    ",\"primitives\":%d,\"result\":%d,\"viewport\":[%u,%u],\"target_size\":[%u,%u],"
	    "\"target_format\":%u,\"target\":%llu,\"vs_bound\":%llu,\"ps_bound\":%llu,"
	    "\"texture_mask\":%u,\"depth_test\":%u,\"depth_write\":%u,\"color_write\":%u,"
	    "\"cull\":%u,\"blend\":%u,\"query_errors\":%u",
	    state.primitives, state.result, state.viewportWidth, state.viewportHeight,
	    state.targetWidth, state.targetHeight, state.targetFormat,
	    static_cast<unsigned long long>( state.target ),
	    static_cast<unsigned long long>( state.vertexShader ),
	    static_cast<unsigned long long>( state.pixelShader ), state.textureMask, state.depthTest,
	    state.depthWrite, state.colorWrite, state.cull, state.blend, state.queryErrors );
	WriteShader( "vs", m_Vertex );
	WriteShader( "ps", m_Pixel );
	std::fputs( "}\n", m_Output );
}
void Capture::Skipped( const char *material, const char *reason )
{
	if ( !Enabled() )
		return;
	std::lock_guard<std::mutex> lock( m_Mutex );
	++m_Skipped;
	if ( m_Frame < m_FirstFrame || m_Frame - m_FirstFrame >= m_FrameCount || m_Skipped > 256 )
		return;
	std::fprintf( m_Output, "{\"event\":\"skipped\",\"frame\":%u,\"material\":", m_Frame );
	String( m_Output, material );
	std::fputs( ",\"reason\":", m_Output );
	String( m_Output, reason );
	std::fputs( "}\n", m_Output );
}
void Capture::Present( std::int32_t result, bool cropped )
{
	if ( !Enabled() )
		return;
	std::lock_guard<std::mutex> lock( m_Mutex );
	std::fprintf( m_Output,
	    "{\"event\":\"present\",\"frame\":%u,\"result\":%d,\"cropped\":%s}\n",
	    m_Frame, result, cropped ? "true" : "false" );
}
Capture &Current()
{
	static Capture capture( std::getenv( "SOURCE_RENDER_TRACE" ),
	    Setting( "SOURCE_RENDER_TRACE_FIRST", 200, 100000 ),
	    Setting( "SOURCE_RENDER_TRACE_FRAMES", 4, 100 ),
	    Setting( "SOURCE_RENDER_TRACE_DRAWS", 20000, 100000 ) );
	return capture;
}
} // namespace renderdiagnostics
