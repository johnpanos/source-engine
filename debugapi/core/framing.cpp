//========= Copyright Valve Corporation, All rights reserved. ============//
//
// debugapi.framing.v1 providers: newline-delimited and Content-Length.
//
//=============================================================================//

#include "debugapi/debugapi_framing.h"

#include <charconv>
#include <utility>

namespace debugapi
{
namespace
{

constexpr size_t kMaxHeaderBytes = 4096;

bool IsBlank( std::string_view text )
{
	return text.find_first_not_of( " \t\r" ) == std::string_view::npos;
}

class NewlineDecoder final : public IFrameDecoder
{
public:
	explicit NewlineDecoder( size_t maxFrameBytes ) : m_MaxFrameBytes( maxFrameBytes ) {}

	FrameStatus Feed(
	    std::string_view bytes, std::vector<std::string> &frames, std::string &error ) override
	{
		if ( m_Fatal )
			return FrameStatus::Fatal;
		while ( !bytes.empty() )
		{
			const size_t newline = bytes.find( '\n' );
			const std::string_view chunk = bytes.substr( 0, newline );
			if ( m_Partial.size() + chunk.size() > m_MaxFrameBytes + 1 )
				return Fail( error );
			m_Partial.append( chunk );
			if ( newline == std::string_view::npos )
				break;
			bytes.remove_prefix( newline + 1 );
			if ( !m_Partial.empty() && m_Partial.back() == '\r' )
				m_Partial.pop_back();
			if ( m_Partial.size() > m_MaxFrameBytes )
				return Fail( error );
			if ( !IsBlank( m_Partial ) )
				frames.push_back( std::move( m_Partial ) );
			m_Partial.clear();
		}
		return FrameStatus::Open;
	}

private:
	FrameStatus Fail( std::string &error )
	{
		m_Fatal = true;
		m_Partial.clear();
		error = "frame exceeds " + std::to_string( m_MaxFrameBytes ) + " bytes";
		return FrameStatus::Fatal;
	}

	size_t m_MaxFrameBytes;
	std::string m_Partial;
	bool m_Fatal = false;
};

std::unique_ptr<IFrameDecoder> CreateNewlineDecoder( size_t maxFrameBytes )
{
	return std::make_unique<NewlineDecoder>( maxFrameBytes );
}

void EncodeNewline( std::string_view payload, std::string &out )
{
	out.append( payload );
	out.push_back( '\n' );
}

class ContentLengthDecoder final : public IFrameDecoder
{
public:
	explicit ContentLengthDecoder( size_t maxFrameBytes ) : m_MaxFrameBytes( maxFrameBytes ) {}

	FrameStatus Feed(
	    std::string_view bytes, std::vector<std::string> &frames, std::string &error ) override
	{
		if ( m_Fatal )
			return FrameStatus::Fatal;
		m_Buffer.append( bytes );
		size_t offset = 0;
		for ( ;; )
		{
			if ( !m_HaveLength )
			{
				const size_t end = m_Buffer.find( "\r\n\r\n", offset );
				if ( end == std::string::npos )
				{
					if ( m_Buffer.size() - offset > kMaxHeaderBytes )
						return Fail( error, "header block exceeds 4096 bytes" );
					break;
				}
				if ( end - offset > kMaxHeaderBytes )
					return Fail( error, "header block exceeds 4096 bytes" );
				std::string message;
				if ( !ParseHeaders(
				         std::string_view( m_Buffer ).substr( offset, end - offset ), message ) )
					return Fail( error, message );
				offset = end + 4;
				m_HaveLength = true;
			}
			if ( m_Buffer.size() - offset < m_Length )
				break;
			frames.push_back( m_Buffer.substr( offset, m_Length ) );
			offset += m_Length;
			m_HaveLength = false;
		}
		m_Buffer.erase( 0, offset );
		return FrameStatus::Open;
	}

private:
	bool ParseHeaders( std::string_view block, std::string &message )
	{
		bool found = false;
		while ( !block.empty() )
		{
			const size_t end = block.find( "\r\n" );
			const std::string_view line = block.substr( 0, end );
			block = end == std::string_view::npos ? std::string_view() : block.substr( end + 2 );
			const size_t colon = line.find( ':' );
			if ( colon == std::string_view::npos || colon == 0 )
			{
				message = "malformed header line";
				return false;
			}
			if ( !EqualsIgnoreCase( line.substr( 0, colon ), "Content-Length" ) )
				continue;
			std::string_view value = line.substr( colon + 1 );
			while ( !value.empty() && ( value.front() == ' ' || value.front() == '\t' ) )
				value.remove_prefix( 1 );
			while ( !value.empty() && ( value.back() == ' ' || value.back() == '\t' ) )
				value.remove_suffix( 1 );
			size_t length = 0;
			const auto [ptr, ec] =
			    std::from_chars( value.data(), value.data() + value.size(), length );
			if ( found || value.empty() || ec != std::errc() || ptr != value.data() + value.size() )
			{
				message = "invalid or duplicate Content-Length";
				return false;
			}
			if ( length > m_MaxFrameBytes )
			{
				message = "frame exceeds " + std::to_string( m_MaxFrameBytes ) + " bytes";
				return false;
			}
			m_Length = length;
			found = true;
		}
		if ( !found )
			message = "missing Content-Length header";
		return found;
	}

	static bool EqualsIgnoreCase( std::string_view a, std::string_view b )
	{
		if ( a.size() != b.size() )
			return false;
		for ( size_t i = 0; i < a.size(); ++i )
		{
			const char x = ( a[i] >= 'A' && a[i] <= 'Z' ) ? char( a[i] - 'A' + 'a' ) : a[i];
			const char y = ( b[i] >= 'A' && b[i] <= 'Z' ) ? char( b[i] - 'A' + 'a' ) : b[i];
			if ( x != y )
				return false;
		}
		return true;
	}

	FrameStatus Fail( std::string &error, std::string message )
	{
		m_Fatal = true;
		m_Buffer.clear();
		error = std::move( message );
		return FrameStatus::Fatal;
	}

	size_t m_MaxFrameBytes;
	std::string m_Buffer;
	size_t m_Length = 0;
	bool m_HaveLength = false;
	bool m_Fatal = false;
};

std::unique_ptr<IFrameDecoder> CreateContentLengthDecoder( size_t maxFrameBytes )
{
	return std::make_unique<ContentLengthDecoder>( maxFrameBytes );
}

void EncodeContentLength( std::string_view payload, std::string &out )
{
	out.append( "Content-Length: " );
	out.append( std::to_string( payload.size() ) );
	out.append( "\r\n\r\n" );
	out.append( payload );
}

} // namespace

const FramingProvider &NewlineFraming()
{
	static const FramingProvider provider{ "newline", &CreateNewlineDecoder, &EncodeNewline };
	return provider;
}

const FramingProvider &ContentLengthFraming()
{
	static const FramingProvider provider{
	    "content-length", &CreateContentLengthDecoder, &EncodeContentLength };
	return provider;
}

} // namespace debugapi
