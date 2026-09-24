//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Bounded, thread-safe ILogSink.
//
//=============================================================================//

#include "log_collector.h"

#include <utility>

namespace debugapi
{

void LogCollector::Append( std::string_view text, bool developer, uint32_t rgba ) noexcept
{
	if ( text.empty() )
		return;
	try
	{
		std::lock_guard<std::mutex> lock( m_Mutex );
		if ( m_Bytes + text.size() > m_MaxBytes )
		{
			++m_Dropped;
			return;
		}
		m_Lines.push_back( LogLine{ std::string( text ), developer, rgba } );
		m_Bytes += text.size();
	}
	catch ( ... )
	{
		// Allocation failure: the line is lost, like any dropped diagnostic.
	}
}

void LogCollector::Drain( std::vector<LogLine> &lines, uint32_t &dropped )
{
	std::lock_guard<std::mutex> lock( m_Mutex );
	lines = std::exchange( m_Lines, {} );
	dropped = std::exchange( m_Dropped, 0 );
	m_Bytes = 0;
}

} // namespace debugapi
