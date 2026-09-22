//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Small portable consumer of the structured tool-process capability.
//
//=============================================================================//

#ifndef PLATFORM_TOOL_PROCESS_CLIENT_H
#define PLATFORM_TOOL_PROCESS_CLIENT_H

#include "platform/contracts/tool_process.h"

namespace platform
{

// Application roots inject one provider. The client preserves the provider's
// structured result while refusing to report a normal completion if the call
// leaked a live process or failed its cleanup guarantee.
class ToolProcessClient
{
public:
	explicit ToolProcessClient( IToolProcessProvider &provider ) noexcept : m_Provider( provider )
	{
	}

	[[nodiscard]] ToolProcessResult Run( const ToolProcessRequest &request );

private:
	IToolProcessProvider &m_Provider;
};

} // namespace platform

#endif // PLATFORM_TOOL_PROCESS_CLIENT_H
