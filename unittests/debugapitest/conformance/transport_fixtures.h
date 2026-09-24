//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: transport fixtures (memory, Unix socket) for the shared suites.
//
//===========================================================================//

#ifndef DEBUGAPITEST_TRANSPORT_FIXTURES_H
#define DEBUGAPITEST_TRANSPORT_FIXTURES_H

#include <memory>
#include <string>

#include "transport_conformance.h"

namespace debugapitest
{

class MemoryFixture final : public ITransportFixture
{
public:
	const char *Name() const override { return "memory"; }
	std::unique_ptr<debugapi::ITransport> Create() override;
	std::unique_ptr<ITestPeer> Connect( debugapi::ITransport &transport ) override;
};

// Creates each transport at a fresh socket path in a private temporary
// directory, removed by the destructor.
class UnixSocketFixture final : public ITransportFixture
{
public:
	UnixSocketFixture();
	~UnixSocketFixture() override;

	const char *Name() const override { return "unix-socket"; }
	std::unique_ptr<debugapi::ITransport> Create() override;
	std::unique_ptr<ITestPeer> Connect( debugapi::ITransport &transport ) override;

	const std::string &Directory() const { return m_Directory; }
	std::string NextPath();

private:
	std::string m_Directory;
	std::string m_LastPath;
	int m_Counter = 0;
};

// A non-blocking client socket connected to `path`; a dead peer on failure.
std::unique_ptr<ITestPeer> ConnectUnixPeer( const std::string &path );

} // namespace debugapitest

#endif // DEBUGAPITEST_TRANSPORT_FIXTURES_H
