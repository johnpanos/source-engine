//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef VIDEO_PROVIDER_CATALOG_H
#define VIDEO_PROVIDER_CATALOG_H

#include "video/ivideoservices.h"

class IVideoSubSystem;

// Factories return linked-module-owned instances. The composition copies descriptors
// before Connect; the modules, names and factories must outlive all video objects.
struct VideoProviderDescriptor
{
	const char *name;
	VideoSystem_t system;
	IVideoSubSystem *( *create )();
	bool required;
};

struct VideoProviderCatalog
{
	const VideoProviderDescriptor *providers;
	int count;
};

extern "C"
{
	const VideoProviderCatalog *VideoServices_GetBuiltProviders();
	IVideoServices *VideoServices_CreateWithProviders( const VideoProviderCatalog *catalog );
	IVideoSubSystem *VideoBink_Create();
	IVideoSubSystem *VideoQuickTime_Create();
	IVideoSubSystem *VideoWebM_Create();
}

#endif // VIDEO_PROVIDER_CATALOG_H
