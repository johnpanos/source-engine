//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Bounded, thread-safe ILogSink. Console text may be printed from any engine
// thread; the server drains it on its own sequence. When the bound is reached
// new lines are dropped and counted (log notifications are diagnostics, never
// required work). A simple mutex is used: RFC 0006 prefers it until a lock-free
// queue has test and performance evidence.
//
//=============================================================================//

#ifndef DEBUGAPI_CORE_LOG_COLLECTOR_H
#define DEBUGAPI_CORE_LOG_COLLECTOR_H

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "debugapi/debugapi_host.h"

namespace debugapi
{

struct LogLine
{
	std::string text;
	bool developer = false;
	uint32_t rgba = 0;
};

class LogCollector final : public ILogSink
{
public:
	explicit LogCollector( size_t maxBytes ) : m_MaxBytes( maxBytes ) {}

	void Append( std::string_view text, bool developer, uint32_t rgba ) noexcept override;

	// Moves out everything collected so far and the number of lines dropped
	// since the previous drain.
	void Drain( std::vector<LogLine> &lines, uint32_t &dropped );

private:
	std::mutex m_Mutex;
	std::vector<LogLine> m_Lines;
	size_t m_Bytes = 0;
	size_t m_MaxBytes;
	uint32_t m_Dropped = 0;
};

} // namespace debugapi

#endif // DEBUGAPI_CORE_LOG_COLLECTOR_H
