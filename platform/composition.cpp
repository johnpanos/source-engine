//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Typed capability composition and deterministic provider lifecycle.
//
//=============================================================================//

#include "platform/composition.h"

#include <algorithm>
#include <limits>

namespace platform
{

namespace
{

constexpr std::size_t kInvalidProvider = std::numeric_limits<std::size_t>::max();

CompositionError MakeError( CompositionErrorCode code, std::string_view provider = {},
    std::string_view capability = {}, std::string_view detail = {} )
{
	return { code, std::string( provider ), std::string( capability ), std::string( detail ) };
}

} // namespace

ApplicationComposition::~ApplicationComposition()
{
	(void)Stop();
}

foundation::Expected<void, CompositionError> ApplicationComposition::AddProvider(
    ProviderDescriptor descriptor )
{
	if ( m_State != State::Stopped || !m_Instances.empty() )
		return foundation::MakeUnexpected( MakeError( CompositionErrorCode::Busy ) );
	m_Descriptors.push_back( std::move( descriptor ) );
	return {};
}

foundation::Expected<std::vector<std::size_t>, CompositionError>
ApplicationComposition::BuildStartOrder() const
{
	for ( std::size_t provider = 0; provider < m_Descriptors.size(); ++provider )
	{
		const auto &descriptor = m_Descriptors[provider];
		if ( descriptor.Name().empty() )
			return foundation::MakeUnexpected(
			    MakeError( CompositionErrorCode::InvalidDescriptor ) );
		const auto caps = descriptor.Capabilities();
		for ( std::size_t i = 0; i < caps.size(); ++i )
		{
			if ( caps[i].Name().empty() )
				return foundation::MakeUnexpected(
				    MakeError( CompositionErrorCode::InvalidDescriptor, descriptor.Name() ) );
			for ( std::size_t j = 0; j < i; ++j )
				if ( caps[i] == caps[j] )
					return foundation::MakeUnexpected(
					    MakeError( CompositionErrorCode::DuplicateCapability, descriptor.Name(),
					        caps[i].Name() ) );
		}
		const auto deps = descriptor.Dependencies();
		for ( std::size_t i = 0; i < deps.size(); ++i )
			for ( std::size_t j = 0; j < i; ++j )
				if ( deps[i].capability == deps[j].capability )
					return foundation::MakeUnexpected(
					    MakeError( CompositionErrorCode::DuplicateDependency, descriptor.Name(),
					        deps[i].capability.Name() ) );
		for ( std::size_t other = provider + 1; other < m_Descriptors.size(); ++other )
		{
			if ( m_Descriptors[provider].Name() == m_Descriptors[other].Name() )
			{
				return foundation::MakeUnexpected( MakeError(
				    CompositionErrorCode::DuplicateProviderName, m_Descriptors[other].Name() ) );
			}
		}

		for ( CapabilityKey capability : m_Descriptors[provider].Capabilities() )
		{
			for ( std::size_t other = provider + 1; other < m_Descriptors.size(); ++other )
			{
				for ( CapabilityKey otherCapability : m_Descriptors[other].Capabilities() )
				{
					if ( capability == otherCapability )
					{
						return foundation::MakeUnexpected(
						    MakeError( CompositionErrorCode::DuplicateCapability,
						        m_Descriptors[other].Name(), capability.Name() ) );
					}
				}
			}
		}
	}

	std::vector<std::vector<std::size_t>> dependents( m_Descriptors.size() );
	std::vector<std::size_t> incoming( m_Descriptors.size(), 0 );

	for ( std::size_t consumer = 0; consumer < m_Descriptors.size(); ++consumer )
	{
		for ( const DependencyDescriptor &dependency : m_Descriptors[consumer].Dependencies() )
		{
			const std::size_t provider = FindCapabilityProvider( dependency.capability );
			if ( provider == kInvalidProvider )
			{
				if ( dependency.requirement == DependencyRequirement::Required )
				{
					return foundation::MakeUnexpected(
					    MakeError( CompositionErrorCode::MissingRequiredCapability,
					        m_Descriptors[consumer].Name(), dependency.capability.Name() ) );
				}
				continue;
			}

			if ( std::find( dependents[provider].begin(), dependents[provider].end(), consumer ) ==
			     dependents[provider].end() )
			{
				dependents[provider].push_back( consumer );
				++incoming[consumer];
			}
		}
	}

	std::vector<std::size_t> order;
	order.reserve( m_Descriptors.size() );
	std::vector<bool> emitted( m_Descriptors.size(), false );

	while ( order.size() != m_Descriptors.size() )
	{
		std::size_t next = kInvalidProvider;
		for ( std::size_t candidate = 0; candidate < m_Descriptors.size(); ++candidate )
		{
			if ( !emitted[candidate] && incoming[candidate] == 0 )
			{
				next = candidate;
				break;
			}
		}

		if ( next == kInvalidProvider )
		{
			std::string_view provider;
			for ( std::size_t candidate = 0; candidate < m_Descriptors.size(); ++candidate )
			{
				if ( !emitted[candidate] )
				{
					provider = m_Descriptors[candidate].Name();
					break;
				}
			}
			return foundation::MakeUnexpected(
			    MakeError( CompositionErrorCode::DependencyCycle, provider ) );
		}

		emitted[next] = true;
		order.push_back( next );
		for ( std::size_t dependent : dependents[next] )
			--incoming[dependent];
	}

	return order;
}

foundation::Expected<void, CompositionError> ApplicationComposition::Start()
{
	auto connected = Connect();
	if ( !connected )
		return connected;
	return Initialize();
}

foundation::Expected<void, CompositionError> ApplicationComposition::Connect()
{
	if ( m_State != State::Stopped )
		return foundation::MakeUnexpected(
		    MakeError( m_State == State::Running ? CompositionErrorCode::AlreadyRunning
		                                         : CompositionErrorCode::Busy ) );

	auto order = BuildStartOrder();
	if ( !order )
		return foundation::MakeUnexpected( std::move( order ).Error() );

	m_State = State::Starting;
	m_StartOrder = std::move( order ).Value();
	m_Instances.clear();
	m_Instances.resize( m_Descriptors.size() );
	m_ConnectedCount = 0;
	m_InitializedCount = 0;

	for ( std::size_t providerIndex : m_StartOrder )
	{
		std::vector<detail::ResolvedDependency> dependencies;
		dependencies.reserve( m_Descriptors[providerIndex].Dependencies().size() );
		for ( const DependencyDescriptor &dependency : m_Descriptors[providerIndex].Dependencies() )
		{
			dependencies.push_back(
			    { dependency.capability, FindCapability( dependency.capability ) } );
		}

		DependencyView view( dependencies );
		auto instance = m_Descriptors[providerIndex].m_Factory( view );
		if ( !instance )
		{
			const ProviderError error = std::move( instance ).Error();
			DestroyInstances();
			m_State = State::Stopped;
			return foundation::MakeUnexpected( MakeError( CompositionErrorCode::ConstructionFailed,
			    m_Descriptors[providerIndex].Name(), {}, error.detail ) );
		}

		m_Instances[providerIndex] =
		    std::make_unique<detail::ProviderInstance>( std::move( instance ).Value() );
	}

	for ( std::size_t providerIndex : m_StartOrder )
	{
		++m_ConnectedCount;
		auto connected = m_Instances[providerIndex]->Lifecycle().Connect();
		if ( !connected )
		{
			const ProviderError error = std::move( connected ).Error();
			Rollback( 0, m_ConnectedCount );
			return foundation::MakeUnexpected( MakeError( CompositionErrorCode::ConnectFailed,
			    m_Descriptors[providerIndex].Name(), {}, error.detail ) );
		}
	}
	m_State = State::Connected;
	return {};
}

foundation::Expected<void, CompositionError> ApplicationComposition::Initialize()
{
	if ( m_State != State::Connected )
		return foundation::MakeUnexpected( MakeError( CompositionErrorCode::Busy ) );

	for ( std::size_t providerIndex : m_StartOrder )
	{
		++m_InitializedCount;
		auto initialized = m_Instances[providerIndex]->Lifecycle().Initialize();
		if ( !initialized )
		{
			const ProviderError error = std::move( initialized ).Error();
			Rollback( m_InitializedCount, m_ConnectedCount );
			return foundation::MakeUnexpected(
			    MakeError( CompositionErrorCode::InitializationFailed,
			        m_Descriptors[providerIndex].Name(), {}, error.detail ) );
		}
	}

	m_State = State::Running;
	return {};
}

foundation::Expected<void, CompositionError> ApplicationComposition::Stop() noexcept
{
	if ( m_State == State::Starting || m_State == State::Stopping )
		return foundation::MakeUnexpected( MakeError( CompositionErrorCode::Busy ) );
	if ( m_State == State::Stopped && m_Instances.empty() )
		return {};

	Shutdown();
	Disconnect();
	return {};
}

void ApplicationComposition::Shutdown() noexcept
{
	if ( m_State != State::Running )
		return;
	m_State = State::Stopping;
	while ( m_InitializedCount > 0 )
	{
		--m_InitializedCount;
		m_Instances[m_StartOrder[m_InitializedCount]]->Lifecycle().Shutdown();
	}
	m_State = State::Connected;
}

void ApplicationComposition::Disconnect() noexcept
{
	if ( m_State == State::Running )
		Shutdown();
	if ( m_State == State::Connected )
		Rollback( 0, m_ConnectedCount );
}

bool ApplicationComposition::IsRunning() const noexcept
{
	return m_State == State::Running;
}

void ApplicationComposition::Rollback(
    std::size_t initializedCount, std::size_t connectedCount ) noexcept
{
	m_State = State::Stopping;
	while ( initializedCount > 0 )
	{
		--initializedCount;
		const std::size_t providerIndex = m_StartOrder[initializedCount];
		m_Instances[providerIndex]->Lifecycle().Shutdown();
	}

	while ( connectedCount > 0 )
	{
		--connectedCount;
		const std::size_t providerIndex = m_StartOrder[connectedCount];
		m_Instances[providerIndex]->Lifecycle().Disconnect();
	}

	DestroyInstances();
	m_ConnectedCount = 0;
	m_InitializedCount = 0;
	m_State = State::Stopped;
}

void ApplicationComposition::DestroyInstances() noexcept
{
	for ( auto provider = m_StartOrder.rbegin(); provider != m_StartOrder.rend(); ++provider )
		m_Instances[*provider].reset();
	m_Instances.clear();
}

void *ApplicationComposition::FindCapability( CapabilityKey capability ) const noexcept
{
	const std::size_t provider = FindCapabilityProvider( capability );
	if ( provider == kInvalidProvider || provider >= m_Instances.size() || !m_Instances[provider] )
		return nullptr;

	for ( const detail::CapabilityExport &entry : m_Instances[provider]->Exports() )
	{
		if ( entry.capability == capability )
			return entry.instance;
	}
	return nullptr;
}

std::size_t ApplicationComposition::FindCapabilityProvider(
    CapabilityKey capability ) const noexcept
{
	for ( std::size_t provider = 0; provider < m_Descriptors.size(); ++provider )
	{
		for ( CapabilityKey provided : m_Descriptors[provider].Capabilities() )
		{
			if ( provided == capability )
				return provider;
		}
	}
	return kInvalidProvider;
}

} // namespace platform
