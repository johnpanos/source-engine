//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Dedicated product composition behind the preserved IAppSystem ABI.
//
//=============================================================================

#ifndef APPFRAMEWORK_DEDICATED_COMPOSITION_BRIDGE_H
#define APPFRAMEWORK_DEDICATED_COMPOSITION_BRIDGE_H

#include "appframework/IAppSystem.h"

class IDedicatedExportsService;
using DedicatedExportsServiceFactory = IDedicatedExportsService *(*)();
using DedicatedExportsServiceBinder = void ( * )( IDedicatedExportsService * );

struct DedicatedComposition;

DedicatedComposition *DedicatedComposition_Create( CreateInterfaceFn factory );
bool DedicatedComposition_AddSystem(
    DedicatedComposition *composition, IAppSystem *system, const char *interfaceName );
bool DedicatedComposition_AddExports( DedicatedComposition *composition, IAppSystem *legacyView,
    const char *interfaceName, DedicatedExportsServiceFactory factory,
    DedicatedExportsServiceBinder bind );
bool DedicatedComposition_OwnModule(
    DedicatedComposition *composition, CSysModule *module, void ( *unload )( CSysModule * ) );
IAppSystem *DedicatedComposition_LegacyBridge( DedicatedComposition *composition );
const char *DedicatedComposition_Error( const DedicatedComposition *composition );
void DedicatedComposition_Destroy( DedicatedComposition *composition );

#endif // APPFRAMEWORK_DEDICATED_COMPOSITION_BRIDGE_H
