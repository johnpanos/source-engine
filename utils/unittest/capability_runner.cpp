//========= Copyright Valve Corporation, All rights reserved. ============//
#include "testing/runner.h"
#include "platform/composition.h"

namespace testing
{
namespace
{
using foundation::Expected;
using platform::ProviderError;

struct IReporting
{
	static constexpr std::string_view kCapabilityName = "test-reporting";
	virtual ~IReporting() = default;
	virtual ITestReport &Report() = 0;
};

class ReportProvider final : public platform::IProviderLifecycle, public IReporting
{
public:
	explicit ReportProvider( ITestReport &report ) : m_Report( report ) {}
	ITestReport &Report() override { return m_Report; }
	Expected<void, ProviderError> Connect() override { return {}; }
	Expected<void, ProviderError> Initialize() override { return {}; }
	void Shutdown() noexcept override {}
	void Disconnect() noexcept override {}

private:
	ITestReport &m_Report;
};

struct ITestExecution
{
	static constexpr std::string_view kCapabilityName = "test-execution";
	virtual ~ITestExecution() = default;
	virtual int Run() = 0;
};

class ExecutionProvider final : public platform::IProviderLifecycle, public ITestExecution
{
public:
	ExecutionProvider( std::span<const Suite> suites, IReporting &reporting )
	    : m_Suites( suites ), m_Report( reporting.Report() )
	{
	}
	Expected<void, ProviderError> Connect() override { return {}; }
	Expected<void, ProviderError> Initialize() override
	{
		if ( m_Suites.empty() )
			return Invalid( "no required suites selected" );
		for ( std::size_t i = 0; i < m_Suites.size(); ++i )
		{
			if ( m_Suites[i].name.empty() || !m_Suites[i].run )
				return Invalid( "missing suite name or callback" );
			for ( std::size_t j = 0; j < i; ++j )
				if ( m_Suites[i].name == m_Suites[j].name )
					return Invalid( "duplicate required suite" );
		}
		return {};
	}
	void Shutdown() noexcept override {}
	void Disconnect() noexcept override {}
	int Run() override
	{
		bool passed = true;
		for ( const Suite &suite : m_Suites )
		{
			const TestResult result = suite.run();
			m_Report.Result( suite.name, result );
			passed = result.Passed() && passed;
		}
		return passed ? 0 : 1;
	}

private:
	static Expected<void, ProviderError> Invalid( const char *detail )
	{
		return foundation::MakeUnexpected(
		    ProviderError{ platform::ProviderErrorCode::InitializationFailed, detail } );
	}
	std::span<const Suite> m_Suites;
	ITestReport &m_Report;
};
}

int RunLinkedSuites( std::span<const Suite> suites, ITestReport &report )
{
	platform::ApplicationComposition composition;
	auto execution = composition.AddProvider(
	    platform::ProviderDescriptor::Define<ExecutionProvider, ITestExecution>(
	        "linked-test-executor", { platform::DependencyDescriptor::Required<IReporting>() },
	        [suites]( const platform::DependencyView &deps )
	            -> Expected<std::unique_ptr<ExecutionProvider>, ProviderError>
	        {
		        return std::make_unique<ExecutionProvider>( suites, *deps.Get<IReporting>() );
	        } ) );
	auto reporting = composition.AddProvider(
	    platform::ProviderDescriptor::Define<ReportProvider, IReporting>( "test-reporter", {},
	        [&report]( const platform::DependencyView & )
	            -> Expected<std::unique_ptr<ReportProvider>, ProviderError>
	        {
		        return std::make_unique<ReportProvider>( report );
	        } ) );
	if ( !execution || !reporting )
	{
		report.Error( "provider registration failed" );
		return 1;
	}
	auto started = composition.Start();
	if ( !started )
	{
		report.Error( started.Error().detail );
		return 1;
	}
	return composition.Find<ITestExecution>()->Run();
}
}
