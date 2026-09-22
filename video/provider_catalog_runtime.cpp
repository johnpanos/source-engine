//========= Copyright Valve Corporation, All rights reserved. ============//
#include "provider_catalog_runtime.h"

#include <cstring>

CVideoProviderSet::CVideoProviderSet()
    : m_ProviderCount( 0 ), m_Connected( false ), m_FailedProvider( nullptr )
{
	for ( int i = 0; i < VideoSystem::VIDEO_SYSTEM_COUNT; ++i )
	{
		m_Systems[i] = nullptr;
		m_Started[i] = false;
	}
}

CVideoProviderSet::~CVideoProviderSet()
{
	Disconnect();
}

bool CVideoProviderSet::Configure( const VideoProviderCatalog &catalog )
{
	if ( m_Connected || catalog.count < 0 || catalog.count >= VideoSystem::VIDEO_SYSTEM_COUNT ||
	     ( catalog.count && !catalog.providers ) )
		return false;

	// Validate the entire selection before copying it or invoking any factory.
	for ( int i = 0; i < catalog.count; ++i )
	{
		const VideoProviderDescriptor &provider = catalog.providers[i];
		if ( !provider.name || !provider.name[0] || !provider.create ||
		     provider.system < VideoSystem::VIDEO_SYSTEM_FIRST ||
		     provider.system >= VideoSystem::VIDEO_SYSTEM_COUNT )
			return false;
		for ( int j = 0; j < i; ++j )
		{
			if ( provider.system == catalog.providers[j].system ||
			     std::strcmp( provider.name, catalog.providers[j].name ) == 0 )
				return false;
		}
	}
	for ( int i = 0; i < catalog.count; ++i )
		m_Providers[i] = catalog.providers[i];
	m_ProviderCount = catalog.count;
	m_FailedProvider = nullptr;
	return true;
}

bool CVideoProviderSet::Connect( CreateInterfaceFn factory, IVideoCommonServices *common )
{
	if ( m_Connected )
		return false;
	m_Connected = true;
	m_FailedProvider = nullptr;
	for ( int i = 0; i < m_ProviderCount; ++i )
	{
		const VideoProviderDescriptor &provider = m_Providers[i];
		IVideoSubSystem *system = provider.create();
		bool available = false;
		if ( system && system->GetSystemID() == provider.system )
		{
			if ( system->Connect( factory ) )
			{
				available = system->InitializeVideoSystem( common );
				if ( !available )
					system->ShutdownVideoSystem();
			}
			if ( !available )
				system->Disconnect();
		}
		if ( available )
			m_Systems[provider.system] = system;
		else
		{
			m_FailedProvider = provider.name;
			if ( provider.required )
			{
				Disconnect();
				return false;
			}
		}
	}
	return true;
}

bool CVideoProviderSet::Init()
{
	if ( !m_Connected )
		return false;
	for ( int i = 0; i < m_ProviderCount; ++i )
	{
		const VideoProviderDescriptor &provider = m_Providers[i];
		IVideoSubSystem *system = m_Systems[provider.system];
		if ( !system || m_Started[provider.system] )
			continue;
		if ( system->Init() != INIT_OK )
		{
			m_FailedProvider = provider.name;
			system->Shutdown();
			system->ShutdownVideoSystem();
			system->Disconnect();
			m_Systems[provider.system] = nullptr;
			if ( provider.required )
			{
				Disconnect();
				return false;
			}
		}
		else
			m_Started[provider.system] = true;
	}
	return true;
}

void CVideoProviderSet::Shutdown()
{
	for ( int i = m_ProviderCount - 1; i >= 0; --i )
	{
		int system = m_Providers[i].system;
		if ( m_Started[system] )
		{
			m_Systems[system]->Shutdown();
			m_Started[system] = false;
		}
	}
}

void CVideoProviderSet::Disconnect()
{
	Shutdown();
	for ( int i = m_ProviderCount - 1; i >= 0; --i )
	{
		int system = m_Providers[i].system;
		if ( m_Systems[system] )
		{
			m_Systems[system]->ShutdownVideoSystem();
			m_Systems[system]->Disconnect();
			m_Systems[system] = nullptr;
		}
	}
	m_Connected = false;
}

IVideoSubSystem *CVideoProviderSet::Get( int system ) const
{
	return system >= VideoSystem::VIDEO_SYSTEM_FIRST && system < VideoSystem::VIDEO_SYSTEM_COUNT
	           ? m_Systems[system]
	           : nullptr;
}

int CVideoProviderSet::Count() const
{
	int count = 0;
	for ( int i = VideoSystem::VIDEO_SYSTEM_FIRST; i < VideoSystem::VIDEO_SYSTEM_COUNT; ++i )
		if ( m_Systems[i] )
			++count;
	return count;
}
