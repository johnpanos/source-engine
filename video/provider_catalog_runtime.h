//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef VIDEO_PROVIDER_CATALOG_RUNTIME_H
#define VIDEO_PROVIDER_CATALOG_RUNTIME_H

#include "video/provider_catalog.h"
#include "videosubsystem.h"

// Owns the lifecycle of borrowed linked providers. Required failures roll back
// previously connected providers; optional failures are explicitly unavailable.
// Shutdown and disconnect happen in reverse catalog order. No callbacks survive
// Disconnect; the caller destroys all video materials/recorders first.
class CVideoProviderSet
{
public:
	CVideoProviderSet();
	bool Configure( const VideoProviderCatalog &catalog );
	bool Connect( CreateInterfaceFn factory, IVideoCommonServices *common );
	bool Init();
	void Shutdown();
	void Disconnect();
	IVideoSubSystem *Get( int system ) const;
	int Count() const;
	const char *FailedProvider() const { return m_FailedProvider; }

private:
	VideoProviderDescriptor m_Providers[VideoSystem::VIDEO_SYSTEM_COUNT];
	IVideoSubSystem *m_Systems[VideoSystem::VIDEO_SYSTEM_COUNT];
	bool m_Started[VideoSystem::VIDEO_SYSTEM_COUNT];
	int m_ProviderCount;
	bool m_Connected;
	const char *m_FailedProvider;
};

#endif // VIDEO_PROVIDER_CATALOG_RUNTIME_H
