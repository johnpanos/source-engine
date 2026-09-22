//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Typed capability composition and deterministic provider lifecycle.
//
//=============================================================================//

#ifndef PLATFORM_COMPOSITION_H
#define PLATFORM_COMPOSITION_H

#include "foundation/expected.h"

#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace platform
{

template < typename Contract >
concept CapabilityContract = requires {
	{ Contract::kCapabilityName } -> std::convertible_to< std::string_view >;
};

class CapabilityKey
{
public:
	template < CapabilityContract Contract >
	static CapabilityKey For() noexcept
	{
		return CapabilityKey( &CapabilityToken< Contract >, Contract::kCapabilityName );
	}

	std::string_view Name() const noexcept { return m_Name; }

	friend bool operator==( CapabilityKey left, CapabilityKey right ) noexcept
	{
		return left.m_Token == right.m_Token;
	}

private:
	template < typename Contract >
	inline static const char CapabilityToken = 0;

	CapabilityKey( const void *token, std::string_view name ) noexcept : m_Token( token ), m_Name( name )
	{
	}

	const void *m_Token;
	std::string_view m_Name;
};

enum class DependencyRequirement
{
	Required,
	Optional,
};

struct DependencyDescriptor
{
	template < CapabilityContract Contract >
	static DependencyDescriptor Required() noexcept
	{
		return { CapabilityKey::For< Contract >(), DependencyRequirement::Required };
	}

	template < CapabilityContract Contract >
	static DependencyDescriptor Optional() noexcept
	{
		return { CapabilityKey::For< Contract >(), DependencyRequirement::Optional };
	}

	CapabilityKey capability;
	DependencyRequirement requirement;
};

enum class ProviderErrorCode
{
	ConstructionFailed,
	ConnectFailed,
	InitializationFailed,
	BridgeInstallFailed,
};

struct ProviderError
{
	ProviderErrorCode code;
	std::string detail;
};

enum class CompositionErrorCode
{
	Busy,
	InvalidDescriptor,
	DuplicateDependency,
	AlreadyRunning,
	NotRunning,
	DuplicateProviderName,
	DuplicateCapability,
	MissingRequiredCapability,
	DependencyCycle,
	ConstructionFailed,
	ConnectFailed,
	InitializationFailed,
	DuplicateLegacyBridge,
	LegacyBridgeInstallFailed,
};

struct CompositionError
{
	CompositionErrorCode code;
	std::string provider;
	std::string capability;
	std::string detail;
};

class IProviderLifecycle
{
public:
	virtual ~IProviderLifecycle() = default;

	virtual foundation::Expected< void, ProviderError > Connect() = 0;
	virtual foundation::Expected< void, ProviderError > Initialize() = 0;
	virtual void Shutdown() noexcept = 0;
	virtual void Disconnect() noexcept = 0;
};

namespace detail
{

struct ResolvedDependency
{
	CapabilityKey capability;
	void *instance;
};

struct CapabilityExport
{
	CapabilityKey capability;
	void *instance;
};

class ProviderInstance
{
public:
	ProviderInstance( ProviderInstance && ) noexcept = default;
	ProviderInstance &operator=( ProviderInstance && ) noexcept = default;

	ProviderInstance( const ProviderInstance & ) = delete;
	ProviderInstance &operator=( const ProviderInstance & ) = delete;

	template < typename Provider, CapabilityContract... Contracts >
	requires std::derived_from< Provider, IProviderLifecycle > &&
	         ( std::derived_from< Provider, Contracts > &&... )
	static ProviderInstance Own( std::unique_ptr< Provider > provider )
	{
		std::vector< CapabilityExport > exports;
		exports.reserve( sizeof...( Contracts ) );
		( exports.push_back(
		      { CapabilityKey::For< Contracts >(), static_cast< Contracts * >( provider.get() ) } ),
		  ... );
		return ProviderInstance( std::move( provider ), std::move( exports ) );
	}

	IProviderLifecycle &Lifecycle() noexcept { return *m_Owner; }
	std::span< const CapabilityExport > Exports() const noexcept { return m_Exports; }

private:
	ProviderInstance( std::unique_ptr< IProviderLifecycle > owner,
	                  std::vector< CapabilityExport > exports )
	    : m_Owner( std::move( owner ) ), m_Exports( std::move( exports ) )
	{
	}

	std::unique_ptr< IProviderLifecycle > m_Owner;
	std::vector< CapabilityExport > m_Exports;
};

} // namespace detail

class DependencyView
{
public:
	template < CapabilityContract Contract >
	Contract *Get() const noexcept
	{
		const CapabilityKey wanted = CapabilityKey::For< Contract >();
		for ( const detail::ResolvedDependency &dependency : m_Dependencies )
		{
			if ( dependency.capability == wanted )
				return static_cast< Contract * >( dependency.instance );
		}
		return nullptr;
	}

private:
	explicit DependencyView( std::span< const detail::ResolvedDependency > dependencies ) noexcept
	    : m_Dependencies( dependencies )
	{
	}

	std::span< const detail::ResolvedDependency > m_Dependencies;

	friend class ApplicationComposition;
};

class ProviderDescriptor
{
public:
	ProviderDescriptor( ProviderDescriptor && ) noexcept = default;
	ProviderDescriptor &operator=( ProviderDescriptor && ) noexcept = default;

	ProviderDescriptor( const ProviderDescriptor & ) = delete;
	ProviderDescriptor &operator=( const ProviderDescriptor & ) = delete;

	template < typename Provider, CapabilityContract... Contracts, typename Factory >
	requires std::derived_from< Provider, IProviderLifecycle > &&
	         ( std::derived_from< Provider, Contracts > &&... )
	static ProviderDescriptor Define( std::string name, std::vector< DependencyDescriptor > dependencies,
	                                  Factory factory )
	{
		std::vector< CapabilityKey > capabilities{ CapabilityKey::For< Contracts >()... };
		FactoryFunction erased =
		    [factory = std::move( factory )]( const DependencyView &view ) mutable
		    -> foundation::Expected< detail::ProviderInstance, ProviderError > {
			auto result = factory( view );
			if ( !result )
				return foundation::MakeUnexpected( std::move( result ).Error() );
			if ( !result.Value() )
				return foundation::MakeUnexpected( ProviderError{
				    ProviderErrorCode::ConstructionFailed, "factory returned a null provider" } );

			return detail::ProviderInstance::Own< Provider, Contracts... >(
			    std::move( result ).Value() );
		};

		return ProviderDescriptor( std::move( name ), std::move( capabilities ),
		                           std::move( dependencies ), std::move( erased ) );
	}

	std::string_view Name() const noexcept { return m_Name; }
	std::span< const CapabilityKey > Capabilities() const noexcept { return m_Capabilities; }
	std::span< const DependencyDescriptor > Dependencies() const noexcept { return m_Dependencies; }

private:
	using FactoryFunction = std::function< foundation::Expected< detail::ProviderInstance, ProviderError >(
	    const DependencyView & ) >;

	ProviderDescriptor( std::string name, std::vector< CapabilityKey > capabilities,
	                    std::vector< DependencyDescriptor > dependencies, FactoryFunction factory )
	    : m_Name( std::move( name ) ), m_Capabilities( std::move( capabilities ) ),
	      m_Dependencies( std::move( dependencies ) ), m_Factory( std::move( factory ) )
	{
	}

	std::string m_Name;
	std::vector< CapabilityKey > m_Capabilities;
	std::vector< DependencyDescriptor > m_Dependencies;
	FactoryFunction m_Factory;

	friend class ApplicationComposition;
};

class ApplicationComposition
{
public:
	ApplicationComposition() = default;
	~ApplicationComposition();

	ApplicationComposition( const ApplicationComposition & ) = delete;
	ApplicationComposition &operator=( const ApplicationComposition & ) = delete;
	ApplicationComposition( ApplicationComposition && ) = delete;
	ApplicationComposition &operator=( ApplicationComposition && ) = delete;

	[[nodiscard]] foundation::Expected< void, CompositionError > AddProvider( ProviderDescriptor descriptor );

	[[nodiscard]] foundation::Expected< void, CompositionError > Start();
	[[nodiscard]] foundation::Expected< void, CompositionError > Stop() noexcept;

	bool IsRunning() const noexcept;

	template < CapabilityContract Contract >
	Contract *Find() const noexcept
	{
		if ( !IsRunning() )
			return nullptr;
		return static_cast< Contract * >( FindCapability( CapabilityKey::For< Contract >() ) );
	}

private:
	enum class State
	{
		Stopped,
		Starting,
		Running,
		Stopping,
	};

	[[nodiscard]] foundation::Expected< std::vector< std::size_t >, CompositionError >
	BuildStartOrder() const;
	void Rollback( std::size_t initializedCount, std::size_t connectedCount ) noexcept;
	void DestroyInstances() noexcept;
	void *FindCapability( CapabilityKey capability ) const noexcept;
	std::size_t FindCapabilityProvider( CapabilityKey capability ) const noexcept;

	std::vector< ProviderDescriptor > m_Descriptors;
	std::vector< std::unique_ptr< detail::ProviderInstance > > m_Instances;
	std::vector< std::size_t > m_StartOrder;
	State m_State = State::Stopped;
	std::size_t m_ConnectedCount = 0;
	std::size_t m_InitializedCount = 0;
};

} // namespace platform

#endif // PLATFORM_COMPOSITION_H
