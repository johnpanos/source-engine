//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Dedicated product lifecycle with an IAppSystem compatibility view.
//
//=============================================================================

#include "platform/composition.h"

#include <array>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// Legacy Tier headers define min/max macros, so they follow all C++ headers.
#include "appframework/dedicated_composition_bridge.h"
#include "exports_service.h"

namespace
{

constexpr std::array<std::string_view, 16> kSlotNames = { "dedicated.system.00",
    "dedicated.system.01", "dedicated.system.02", "dedicated.system.03", "dedicated.system.04",
    "dedicated.system.05", "dedicated.system.06", "dedicated.system.07", "dedicated.system.08",
    "dedicated.system.09", "dedicated.system.10", "dedicated.system.11", "dedicated.system.12",
    "dedicated.system.13", "dedicated.system.14", "dedicated.system.15" };

template <std::size_t Index> struct SystemSlot
{
	static constexpr std::string_view kCapabilityName = kSlotNames[Index];
	virtual IAppSystem *LegacyView() const = 0;
};

// Module-owned Source services remain borrowed implementation details. The
// canonical composition owns their lifecycle, ordering, rollback and teardown;
// the IAppSystem bridge below exposes them only to frozen callers and mods.
template <std::size_t Index>
class SystemProvider final : public platform::IProviderLifecycle, public SystemSlot<Index>
{
public:
	SystemProvider( IAppSystem &system, CreateInterfaceFn factory )
	    : m_System( system ), m_Factory( factory )
	{
	}

	IAppSystem *LegacyView() const override { return &m_System; }

	foundation::Expected<void, platform::ProviderError> Connect() override
	{
		if ( !m_System.Connect( m_Factory ) )
			return foundation::MakeUnexpected( platform::ProviderError{
			    platform::ProviderErrorCode::ConnectFailed, "IAppSystem::Connect failed" } );
		return {};
	}

	foundation::Expected<void, platform::ProviderError> Initialize() override
	{
		if ( m_System.Init() != INIT_OK )
			return foundation::MakeUnexpected( platform::ProviderError{
			    platform::ProviderErrorCode::InitializationFailed, "IAppSystem::Init failed" } );
		return {};
	}

	void Shutdown() noexcept override { m_System.Shutdown(); }
	void Disconnect() noexcept override { m_System.Disconnect(); }

private:
	IAppSystem &m_System;
	CreateInterfaceFn m_Factory;
};

// The dedicated exports are a native composition-owned service. Their frozen
// IAppSystem object only forwards the mod ABI while this provider is connected.
class ExportsProvider final : public platform::IProviderLifecycle, public SystemSlot<0>
{
public:
	ExportsProvider( std::unique_ptr<IDedicatedExportsService> service, IAppSystem &legacyView,
	    DedicatedExportsServiceBinder bind )
	    : m_Service( std::move( service ) ), m_LegacyView( legacyView ), m_Bind( bind )
	{
	}

	IAppSystem *LegacyView() const override { return &m_LegacyView; }
	foundation::Expected<void, platform::ProviderError> Connect() override
	{
		m_Bind( m_Service.get() );
		return {};
	}
	foundation::Expected<void, platform::ProviderError> Initialize() override { return {}; }
	void Shutdown() noexcept override {}
	void Disconnect() noexcept override { m_Bind( nullptr ); }

private:
	std::unique_ptr<IDedicatedExportsService> m_Service;
	IAppSystem &m_LegacyView;
	DedicatedExportsServiceBinder m_Bind;
};

struct SystemRecord
{
	std::string interfaceName;
	IAppSystem *system;
	DedicatedExportsServiceFactory exportsFactory = nullptr;
	DedicatedExportsServiceBinder exportsBind = nullptr;
};

struct ModuleRecord
{
	CSysModule *module;
	void ( *unload )( CSysModule * );
};

} // namespace

struct DedicatedComposition
{
	explicit DedicatedComposition( CreateInterfaceFn factory ) : bridge( *this ), factory( factory )
	{
	}
	~DedicatedComposition()
	{
		(void)root.Stop();
		for ( auto module = modules.rbegin(); module != modules.rend(); ++module )
			module->unload( module->module );
	}

	class Bridge final : public IAppSystem
	{
	public:
		explicit Bridge( DedicatedComposition &owner ) : m_Owner( owner ) {}

		bool Connect( CreateInterfaceFn ) override { return m_Owner.Connect(); }
		void Disconnect() override { m_Owner.root.Disconnect(); }
		void *QueryInterface( const char *name ) override { return m_Owner.Lookup( name ); }
		InitReturnVal_t Init() override
		{
			auto result = m_Owner.root.Initialize();
			if ( result )
				return INIT_OK;
			m_Owner.SetError( result.Error() );
			return INIT_FAILED;
		}
		void Shutdown() override { m_Owner.root.Shutdown(); }

	private:
		DedicatedComposition &m_Owner;
	};

	bool Add( IAppSystem *system, const char *interfaceName )
	{
		if ( !system || !interfaceName || !*interfaceName || records.size() == kSlotNames.size() ||
		     configured )
		{
			error = "invalid or late dedicated system registration";
			return false;
		}
		for ( const SystemRecord &record : records )
		{
			if ( record.interfaceName == interfaceName )
			{
				error = "duplicate dedicated system interface";
				return false;
			}
		}
		records.push_back( { interfaceName, system } );
		return true;
	}

	bool AddExports( IAppSystem *legacyView, const char *interfaceName,
	    DedicatedExportsServiceFactory create, DedicatedExportsServiceBinder bind )
	{
		if ( !records.empty() || !legacyView || !interfaceName || !*interfaceName || !create ||
		     !bind || configured )
		{
			error = "invalid or late dedicated exports registration";
			return false;
		}
		records.push_back( { interfaceName, legacyView, create, bind } );
		return true;
	}

	bool OwnModule( CSysModule *module, void ( *unload )( CSysModule * ) )
	{
		if ( !module || !unload || configured )
		{
			error = "invalid or late dedicated module transfer";
			return false;
		}
		modules.push_back( { module, unload } );
		return true;
	}

	template <std::size_t Index>
	void *LookupSlot( std::size_t wanted, const char *name, bool exact ) const
	{
		if constexpr ( Index == kSlotNames.size() )
			return nullptr;
		else
		{
			if ( wanted != Index )
				return LookupSlot<Index + 1>( wanted, name, exact );
			auto *slot = root.Find<SystemSlot<Index>>();
			if ( !slot && !connecting )
				return nullptr;
			// Source Connect callbacks may query sibling interfaces. All legacy
			// instances are module-owned and registered before connection begins.
			IAppSystem *system = slot ? slot->LegacyView() : records[Index].system;
			return exact ? system : system->QueryInterface( name );
		}
	}

	void *Lookup( const char *name ) const
	{
		if ( !name )
			return nullptr;
		for ( std::size_t index = 0; index < records.size(); ++index )
		{
			if ( records[index].interfaceName == name )
				return LookupSlot<0>( index, name, true );
		}
		for ( std::size_t index = 0; index < records.size(); ++index )
		{
			if ( void *view = LookupSlot<0>( index, name, false ) )
				return view;
		}
		return nullptr;
	}

	bool Connect()
	{
		if ( !configured )
		{
			for ( std::size_t index = 0; index < records.size(); ++index )
			{
				if ( !AddSlot<0>( index ) )
					return false;
			}
			configured = true;
		}
		connecting = true;
		auto result = root.Connect();
		connecting = false;
		if ( result )
		{
			error.clear();
			return true;
		}
		SetError( result.Error() );
		return false;
	}

	template <std::size_t Index> bool AddSlot( std::size_t wanted )
	{
		if constexpr ( Index == kSlotNames.size() )
		{
			error = "too many dedicated systems";
			return false;
		}
		else
		{
			if ( wanted != Index )
				return AddSlot<Index + 1>( wanted );
			std::vector<platform::DependencyDescriptor> dependencies;
			if constexpr ( Index > 0 )
				dependencies.push_back(
				    platform::DependencyDescriptor::Required<SystemSlot<Index - 1>>() );
			if constexpr ( Index == 0 )
			{
				if ( records[Index].exportsFactory )
				{
					const SystemRecord &record = records[Index];
					auto descriptor =
					    platform::ProviderDescriptor::Define<ExportsProvider, SystemSlot<0>>(
					        record.interfaceName, {},
					        [create = record.exportsFactory, bind = record.exportsBind,
					            legacyView = record.system]( const platform::DependencyView & )
					            -> foundation::Expected<std::unique_ptr<ExportsProvider>,
					                platform::ProviderError>
					        {
						        std::unique_ptr<IDedicatedExportsService> service( create() );
						        if ( !service )
							        return foundation::MakeUnexpected( platform::ProviderError{
							            platform::ProviderErrorCode::ConstructionFailed,
							            "dedicated exports factory returned null" } );
						        return std::make_unique<ExportsProvider>(
						            std::move( service ), *legacyView, bind );
					        } );
					auto result = root.AddProvider( std::move( descriptor ) );
					if ( result )
						return true;
					SetError( result.Error() );
					return false;
				}
			}
			IAppSystem *system = records[Index].system;
			auto descriptor =
			    platform::ProviderDescriptor::Define<SystemProvider<Index>, SystemSlot<Index>>(
			        records[Index].interfaceName, std::move( dependencies ),
			        [system, selectedFactory = factory]( const platform::DependencyView & )
			            -> foundation::Expected<std::unique_ptr<SystemProvider<Index>>,
			                platform::ProviderError>
			        {
				        return std::make_unique<SystemProvider<Index>>( *system, selectedFactory );
			        } );
			auto result = root.AddProvider( std::move( descriptor ) );
			if ( result )
				return true;
			SetError( result.Error() );
			return false;
		}
	}

	void SetError( const platform::CompositionError &failure )
	{
		error = failure.provider;
		if ( !failure.detail.empty() )
		{
			error += ": ";
			error += failure.detail;
		}
	}

	platform::ApplicationComposition root;
	Bridge bridge;
	CreateInterfaceFn factory;
	std::vector<SystemRecord> records;
	std::vector<ModuleRecord> modules;
	std::string error;
	bool configured = false;
	bool connecting = false;
};

DedicatedComposition *DedicatedComposition_Create( CreateInterfaceFn factory )
{
	return factory ? new DedicatedComposition( factory ) : nullptr;
}

bool DedicatedComposition_AddSystem(
    DedicatedComposition *composition, IAppSystem *system, const char *interfaceName )
{
	return composition && composition->Add( system, interfaceName );
}

bool DedicatedComposition_AddExports( DedicatedComposition *composition, IAppSystem *legacyView,
    const char *interfaceName, DedicatedExportsServiceFactory factory,
    DedicatedExportsServiceBinder bind )
{
	return composition && composition->AddExports( legacyView, interfaceName, factory, bind );
}

bool DedicatedComposition_OwnModule(
    DedicatedComposition *composition, CSysModule *module, void ( *unload )( CSysModule * ) )
{
	return composition && composition->OwnModule( module, unload );
}

IAppSystem *DedicatedComposition_LegacyBridge( DedicatedComposition *composition )
{
	return composition ? &composition->bridge : nullptr;
}

const char *DedicatedComposition_Error( const DedicatedComposition *composition )
{
	return composition ? composition->error.c_str() : "missing dedicated composition";
}

void DedicatedComposition_Destroy( DedicatedComposition *composition )
{
	delete composition;
}
